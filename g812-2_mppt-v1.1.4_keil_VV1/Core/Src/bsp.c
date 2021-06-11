/*
 * bsp.c
 *
 *  Created on: 8 февр. 2016
 *      Author: Viacheslav Azarov
 *      Board Specific Peripherals
 ******************************************************************************
 *  This software developed by engineer Viacheslav Azarov with free tools
 *  from STMicroelectronics on personal wish employer Danil Rogalev and saves
 *  all rights all authors and contributors.
 ******************************************************************************
 */

#include "stm32f3xx_hal.h"
#include "adc.h"
#include "hrtim.h"
#include "tim.h"
#include "gpio.h"
#include "bsp.h"
#include "pb_charger.h"

//const uint16_t conf[50];

#ifdef  NUCLEO_F334R8
#define COMMON_LED1_PORT	GPIOA
#define COMMON_LED1_PIN		GPIO_PIN_5
#else
#define COMMON_LED1_PORT	GPIOB
#define COMMON_LED1_PIN		GPIO_PIN_4
#endif


/* Target assignments (0..4095) in ADC units */
//volatile  uint16_t BatCurrentLimit = FBC_VALUE2ADC(
//									  FBC_LI_BATTERY_CHARGE_CURRENT,
//									  FBC_BATTERY_CURRENT_RATIO);
//volatile  uint16_t BatVoltageLimit= FBC_VALUE2ADC(
//		  	  	  	  	 FBC_LI_BATTERY_CHARGE_VOLTAGE,
//									 FBC_BATTERY_VOLTAGE_RATIO);

volatile  uint16_t BatCurrentLimit=0;
volatile  uint16_t BatVoltageLimit=0;


/* Junction ADC conversion result buffers (0..4095) */
volatile  uint16_t Ref1Conversion = 0;
volatile  uint16_t Ref2Conversion = 0;
volatile  uint16_t VpanConversion = 0;
volatile  uint16_t VbatConversion = 0;
volatile  uint16_t IpanConversion = 0;
volatile  uint16_t IbatConversion = 0;

/* Regulators */
volatile   int32_t BridgeGearNumber = 0;
volatile  uint32_t DimmingDutyCycle = 0;

/* MPPT optimizer */
volatile  uint32_t LastPowerOfPanel = 0;
volatile  int16_t  LastStepOptimize = 0;
volatile  uint16_t PanelVoltageMPPT = FBC_VALUE2ADC(FBC_START_PANEL_VOLTAGE,
									  FBC_PANEL_VOLTAGE_RATIO);
/* Active dimming levels [%]*/
volatile  uint8_t  AlightStateLevel = BSP_LIGHTING_ALIGHT;
volatile  uint8_t  DimmedStateLevel = BSP_LIGHTING_DIMMED;

/* Auto decrement timeout counters */
volatile  uint32_t SysSharedTimeout = 0;
volatile  uint32_t SysBridgeTimeout = 0;

/* Presence detection timeout */
volatile  uint32_t PresenceDetector = 0;

static BSP_FBCState_TypeDef bridgestate= FBC_STATE_STOPPED;
BSP_DimState_TypeDef  __attribute__((section (".udata"))) udata_LightingState;
BSP_LoadState_TypeDef __attribute__((section (".udata"))) udata_ExtLoadState;

/* Private variables */
static volatile uint8_t slowblinkedleds=0, fastblinkedleds=0;
static volatile uint8_t dimmingtarget=0, dimmingactive=0;
static volatile uint32_t fastblinkcounter=0, slowblinkcounter=0, dimrollingcounter=0;
static volatile uint16_t presencefiltered=0;

/* Timers for waiting end of transient process */
extern volatile  uint32_t StoppingTimeout;


void BSP_Init(void)
{
	MX_GPIO_Init();
	MX_ADC1_Init();
	MX_ADC2_Init();
	MX_HRTIM1_Init();
	MX_TIM3_Init();
	MX_TIM6_Init();
	HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_DIFFERENTIAL_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_DIFFERENTIAL_ENDED);
	HAL_ADCEx_InjectedStart_IT(&hadc2);
	HAL_ADCEx_InjectedStart_IT(&hadc1);
	HAL_HRTIM_WaveformCounterStart_IT(&hhrtim1,HRTIM_TIMERID_MASTER|
				     HRTIM_TIMERID_TIMER_A|HRTIM_TIMERID_TIMER_C);
	HAL_TIM_PWM_Start_IT(&htim3,TIM_CHANNEL_1);
}

void BSP_Reset(void)
{
/*	  HAL_Delay(200);
	  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15|GPIO_PIN_13|GPIO_PIN_14,GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4,GPIO_PIN_RESET);
	  HAL_Delay(200);
	  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15|GPIO_PIN_13|GPIO_PIN_14,GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3|GPIO_PIN_4,GPIO_PIN_SET);
*/
	  NVIC_SystemReset();
 }

/* Interrupt handlers  */
void on_LED_SlowBlink(void)
{
	if (slowblinkedleds & BSP_LED1) HAL_GPIO_TogglePin(COMMON_LED1_PORT,COMMON_LED1_PIN);
	if (slowblinkedleds & BSP_LED2) HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_15);
#ifndef DEBUG
	if (slowblinkedleds & BSP_LED3) HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_13);
	if (slowblinkedleds & BSP_LED4) HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_14);
	if (slowblinkedleds & BSP_LED5) HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
#endif
}

void on_LED_FastBlink(void)
{
	if (fastblinkedleds & BSP_LED1) HAL_GPIO_TogglePin(COMMON_LED1_PORT,COMMON_LED1_PIN);
	if (fastblinkedleds & BSP_LED2) HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_15);
#ifndef DEBUG
	if (fastblinkedleds & BSP_LED3) HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_13);
	if (fastblinkedleds & BSP_LED4) HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_14);
	if (fastblinkedleds & BSP_LED5) HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);
#endif
}

void on_Dim_Rolling(void)
{
	TIM_OC_InitTypeDef sConfigOC;

	if (dimmingtarget == dimmingactive)
	{
	  if (dimmingactive == 0 && BSP_LDC_State()!=DIM_STATE_EXTINGUISHED)
	  {
	  // Shutdown LED lamp driver in low consumption mode
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
	  // Disable impulses
		sConfigOC.Pulse = 0;
		sConfigOC.OCMode = TIM_OCMODE_INACTIVE;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
		HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
		udata_LightingState = DIM_STATE_EXTINGUISHED;
	   }
	   return;
	}
	if (dimmingtarget  > dimmingactive) dimmingactive++;
								   else dimmingactive--;

	sConfigOC.Pulse = DimmingDutyCycle = FBC_DIMMING_WIDTH(dimmingactive);
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start_IT(&htim3,TIM_CHANNEL_1);
}

void BSP_LED_SetState( uint8_t leds, BSP_LEDState_TypeDef state)
{
	GPIO_PinState output;
	switch (state)
	{
	case LED_STATE_SLOW_BLINKING:
		 {slowblinkedleds |= leds; fastblinkedleds &= ~leds;} return;
	case LED_STATE_FAST_BLINKING:
		 {fastblinkedleds |= leds; slowblinkedleds &= ~leds;} return;
	case LED_STATE_ALIGHT: output = GPIO_PIN_SET; break;
	case LED_STATE_EXTINGUISHED: output = GPIO_PIN_RESET; break;
	default: return;
	}
	fastblinkedleds &= ~leds;
	slowblinkedleds &= ~leds;
	if (leds & BSP_LED1) HAL_GPIO_WritePin(COMMON_LED1_PORT,COMMON_LED1_PIN,output);
	if (leds & BSP_LED2) HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,output);
#ifndef DEBUG
	if (leds & BSP_LED3) HAL_GPIO_WritePin(GPIOA,GPIO_PIN_13,output);
	if (leds & BSP_LED4) HAL_GPIO_WritePin(GPIOA,GPIO_PIN_14,output);
	if (leds & BSP_LED5) HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,output);
#endif
}

BSP_LEDState_TypeDef BSP_LED_State(uint8_t leds)
{
	uint8_t result;
	if ((slowblinkedleds & leds) == leds) return LED_STATE_SLOW_BLINKING;
	if ((slowblinkedleds & leds) != 0)	  return LED_STATE_ALIGHT;
	if ((fastblinkedleds & leds) == leds) return LED_STATE_FAST_BLINKING;
	if ((fastblinkedleds & leds) != 0) 	  return LED_STATE_ALIGHT;
	result = !0;
	if (leds & BSP_LED1) result &= (HAL_GPIO_ReadPin(COMMON_LED1_PORT,COMMON_LED1_PIN)==GPIO_PIN_RESET);
	if (leds & BSP_LED2) result &= (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15)==GPIO_PIN_RESET);
#ifndef DEBUG
	if (leds & BSP_LED3) result &= (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_13)==GPIO_PIN_RESET);
	if (leds & BSP_LED4) result &= (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_14)==GPIO_PIN_RESET);
	if (leds & BSP_LED5) result &= (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3)==GPIO_PIN_RESET);
#endif
	if (result) return LED_STATE_EXTINGUISHED;
	return LED_STATE_ALIGHT;
}

void BSP_ELC_SetState(BSP_LoadState_TypeDef state)
{
	switch (state)
	{
	case LOAD_STATE_DISCONNECTED:	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET); break;
	case LOAD_STATE_CONNECTED:		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
	}
	udata_ExtLoadState = state;
}

BSP_LoadState_TypeDef BSP_ELC_State(void)
{
	return udata_ExtLoadState;
}

void BSP_LDC_SetState(BSP_DimState_TypeDef state)
{
	switch (state)
	{
	case DIM_STATE_EXTINGUISHED: dimmingtarget = 0; return;
    case DIM_STATE_ALIGHT: dimmingtarget = AlightStateLevel; break;
    case DIM_STATE_DIM:    dimmingtarget = DimmedStateLevel; break;
    default:
    		return;
    }
   // Enable driver to work
    udata_LightingState = state;
    HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET);
}

BSP_DimState_TypeDef BSP_LDC_State(void)
{
	return udata_LightingState;
}

void BSP_FBC_SetState(BSP_FBCState_TypeDef state)
{
	switch (state) {
	case FBC_STATE_WORKING:  // Enable HRTIM outputs
	   {
		  HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
							  	  	  	  	  	  HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2);
		  StoppingTimeout = BSP_REGULATIONS_TIMEOUT(FBC_STOPPING_TRANSIENT_TIME);
	   } break;
	case FBC_STATE_STOPPED://	Disable HRTIM outputs
		HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2 |
						       	   	   	   	   HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2);
	     break;
	default: return;
	}
	bridgestate = state;
}

void BSP_FBC_Reset(void)
{
/* Recalculate and setup duty cycles correspond to
 * voltage ratio on input and output of converter.
 */
	BSP_FBC_SetGearNumber(); 
	BSP_FBC_CutGearNumber();
	BSP_FBC_SetDutyCycles();

/* Initialize variables of optimizer*/
	LastPowerOfPanel = 0;
	LastStepOptimize = 0;
	PanelVoltageMPPT = FBC_VALUE2ADC(FBC_START_PANEL_VOLTAGE,
									 FBC_PANEL_VOLTAGE_RATIO);
}


BSP_FBCState_TypeDef  BSP_FBC_State(void)
{
	return bridgestate;
}


/* Calculation duty cycle parameters */
#define DELTA   (FBC_BUCKBOOST_PERIOD * FBC_BUCKBOOST_DELTA)
#define DUTY1   (FBC_BUCKBOOST_PERIOD * FBC_BUCKBOOST_DUTY1)
#define DUTY2   (FBC_BUCKBOOST_PERIOD * FBC_BUCKBOOST_DUTY2)
#define ONE     FBC_BUCKBOOST_PERIOD
#define SCALE   FBC_FRACTIONS_SCALE

void BSP_FBC_CutGearNumber(void)
{ // Range limitation
  BridgeGearNumber = (BridgeGearNumber<0)?0:
		     (BridgeGearNumber>FBC_BUCKBOOST_PERIOD*2)?FBC_BUCKBOOST_PERIOD*2:
		      BridgeGearNumber;
}

void BSP_FBC_SetGearNumber(void)
{
    unsigned int k = SCALE * VbatConversion / VpanConversion;
    BridgeGearNumber = (k < SCALE*DUTY1/(ONE-DELTA))  ? k*(ONE-DELTA)/SCALE:
           (k < SCALE*DUTY1/(ONE-DUTY2))        ? ONE+DUTY1-DELTA-SCALE*DUTY1/k:
           (k < SCALE*(ONE-DELTA)/(ONE-DUTY2))  ? k*(ONE-DUTY2)/SCALE+(DUTY2-DELTA):
                                                  2*ONE-2*DELTA-SCALE*(ONE-DELTA)/k;
}

void BSP_FBC_SetDutyCycles(void)
{
    uint32_t gear, buck, boost, buckboost, buckend, adcpos;

    gear = BridgeGearNumber;

    /* Calculate two duty cycles based on single number */
    	  if (gear < DELTA)              {buck=DELTA; boost=DELTA;}
    else if (gear < DUTY1)              {buck=gear;  boost=DELTA;}
    else if (gear < DUTY1+DUTY2-DELTA)  {buck=DUTY1; boost=DELTA+gear-DUTY1;}
    else if (gear < ONE+DUTY2-2*DELTA)  {buck=gear-DUTY2+DELTA; boost=DUTY2;}
    else if (gear < 2*ONE-3*DELTA)      {buck=ONE-DELTA; boost=gear-ONE+2*DELTA;}
    else                                {buck=ONE-DELTA; boost=ONE-DELTA;}

    /* Setup ADC trigger in optimal time position */
    buckboost = buck - boost; buckend = FBC_BUCKBOOST_WIDTH(100) - buck;
    /* Simple sort intervals */
    adcpos = (boost > buckboost) ?
      		 (boost > buckend) ?  boost - FBC_CONVERSION_WIDTH:
      		  FBC_BUCKBOOST_WIDTH(100) - FBC_CONVERSION_WIDTH:
			 (buckboost > buckend) ? buck - FBC_CONVERSION_WIDTH:
              FBC_BUCKBOOST_WIDTH(100) - FBC_CONVERSION_WIDTH;

    /* Begin of critical section, disable preemption */
//     __disable_fault_irq(); //VV 09.06.21
		__disable_irq();
     hhrtim1.Instance->sMasterRegs.MCMP1R = adcpos;
     hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = buck;
     hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = boost;
     /* End of critical section, enable preemption*/
//     __enable_fault_irq();	//VV 09.06.21
		__enable_irq();
}



/*******************************************************************************
 * Callback functions defined in HAL for user implementation
 */

// In Nucleo board used user button as sensor
#ifdef NUCLEO_F334R8
  #define PRESENCE_DETECTOR_PORT	GPIOC
  #define PRESENCE_DETECTOR_PIN		GPIO_PIN_13
  #define PRESENCE_DETECTOR_POLARITY GPIO_PIN_RESET
#else
  #define PRESENCE_DETECTOR_PORT 	GPIOD
  #define PRESENCE_DETECTOR_PIN		GPIO_PIN_2
  #ifdef  BSP_PRESENCE_POLAITY_HIGH
	#define PRESENCE_DETECTOR_POLARITY GPIO_PIN_SET
  #else
    #define PRESENCE_DETECTOR_POLARITY GPIO_PIN_RESET
  #endif
#endif


void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
  if (!fastblinkcounter--) {
	  on_LED_FastBlink();
	  fastblinkcounter = 100 * FBC_DIMMING_FREQUENCY / (100 * LED_FAST_BLINK_FREQUENCY) /2 ;
  }
  if (!slowblinkcounter--) {
	  on_LED_SlowBlink();
	  slowblinkcounter = 100 * FBC_DIMMING_FREQUENCY / (100 * LED_SLOW_BLINK_FREQUENCY) / 2;
  }
  if (!dimrollingcounter--) {
	  on_Dim_Rolling();
      dimrollingcounter = 100 * FBC_DIMMING_FREQUENCY / (100 * BSP_DIMMING_VELOCITY);
  }
  
  /* Timeouts */
  if (SysSharedTimeout) SysSharedTimeout--;
  if (SysBridgeTimeout) SysBridgeTimeout--;
  if (PresenceDetector) PresenceDetector--;
  
  /* Presence detector filter */
  if (presencefiltered)  {
	  /* Signal released, stop detection and wait new interrupt*/
	  if (HAL_GPIO_ReadPin(PRESENCE_DETECTOR_PORT,PRESENCE_DETECTOR_PIN)!=PRESENCE_DETECTOR_POLARITY)
	  		  presencefiltered=0;
	  if (presencefiltered  > 1) presencefiltered--;
	  /* While signal continue to be active, renew presence timer */
	  if (presencefiltered == 1) PresenceDetector = FBC_DIMMING_FREQUENCY * BSP_LIGHTING_TIME;

  }
}

#ifdef USE_TEMPERATURE_SENSOR
extern uint16_t  udata_TemperatureResets;
#endif
extern uint16_t  udata_OvervoltageResets;
extern uint16_t  udata_OvercarrentResets;
extern uint16_t  udata_PowergoodResets;

void HAL_GPIO_EXTI_Callback(uint16_t Pin)
{
	switch (Pin)
	{
	  case PRESENCE_DETECTOR_PIN:// Presence signal detected
	    presencefiltered = FBC_DIMMING_FREQUENCY * BSP_PRESENCE_DETECTION_TIME;
		break;
#ifdef USE_TEMPERATURE_SENSOR
	  case GPIO_PIN_5: // Temperature reset
	    {
	    	udata_TemperatureResets++;
	    	// Stop converter immediately and reset
	    	BSP_FBC_SetState(FBC_STATE_STOPPED);
	    	BSP_Reset();
	    } break;
#endif
#ifndef NUCLEO_F334R8  // In Nucleo board this disabled
	  case GPIO_PIN_13: // Power good reset
	    {
	    	udata_PowergoodResets++;
	    	// Stop converter immediately and reset
	    	BSP_FBC_SetState(FBC_STATE_STOPPED);
	    	BSP_Reset();
	    } break;
#endif
	  case GPIO_PIN_14: // Over voltage reset
	    {
	    	udata_OvervoltageResets++;
	    	// Stop converter immediately and reset
	    	BSP_FBC_SetState(FBC_STATE_STOPPED);
	    	BSP_Reset();
	    } break;
	  case GPIO_PIN_15: // Over current reset and reset
	    {
	    	udata_OvercarrentResets++;
	    	// Stop converter immediately
	    	BSP_FBC_SetState(FBC_STATE_STOPPED);
	    	BSP_Reset();
	    } break;
	  default: return;
	}
}
/***************************************************************************************/
