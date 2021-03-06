/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  *  This software developed by engineer Viacheslav Azarov with free tools
  *  from STMicroelectronics on personal wish employer Danil Rogalev and saves
  *  all rights all authors and contributors.
  ******************************************************************************
  */

#include "stm32f3xx_hal.h"
//#include "stm32f3xx_hal_iwdg.h"
//#include "stm32f3xx_hal_crc.h"
#include "adc.h"
#include "hrtim.h"
#include "gpio.h"
#include "tim.h"
#include "ssd1306.h"
#include "bsp.h"
#include "smbus.h"
#include "print.h"
//#include "usart.h"
//#include "mb.h"
//#include "mbport.h"
//#include "conf_struct.h"
//#include "pb_charger.h"

// Understand without comments
#define SYS_I_WATCHDOG_TIMEOUT		5/1	/*s*/
#define SYS_I_WATCHDOG_WINDOW		1/2	/*s*/

// Simplest printing
#define prints(str) printstr(SSD1306_PutChar,str)
#define printi(val,len) printint(SSD1306_PutChar,val,len)

// Helpers
#define BOOLEAN		uint8_t
#define FALSE		0
#define TRUE		!FALSE

// Predicates
#define MORNING_DETECTED	  SunlightTrigger()
#define NIGHT_DETECTED		  !SunlightTrigger()
#define PRESENCE_DETECTED	(PresenceDetector != 0)
#define PRESENCE_ENDED		(PresenceDetector == 0)
#define SUNPOWER_DETECTED	(VpanConversion > FBC_VALUE2ADC(FBC_START_PANEL_VOLTAGE, \
															FBC_PANEL_VOLTAGE_RATIO))
#define CHARGER_REST_ENDED	(SysBridgeTimeout == 0)
#define TEMPERATURE_NORMAL	1//(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5) == GPIO_PIN_SET)
#define TEMPERATURE_HIGHER	0//(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5) == GPIO_PIN_RESET)

#define LIGHTING_ENABLED	(BSP_LDC_State() != DIM_STATE_EXTINGUISHED)
#define LIGHTING_DISABLED	(BSP_LDC_State() == DIM_STATE_EXTINGUISHED)
#define LIGHTING_DIMMED		(BSP_LDC_State() == DIM_STATE_DIM)
#define LIGHTING_ALIGHT		(BSP_LDC_State() == DIM_STATE_ALIGHT)
#define CHARGER_STOPPED		(BSP_FBC_State() == FBC_STATE_STOPPED)
#define CHARGER_WORKING		(BSP_FBC_State() == FBC_STATE_WORKING)
#define LOAD_DISCONNECTED	(BSP_ELC_State() == LOAD_STATE_DISCONNECTED)
#define LOAD_CONNECTED		(BSP_ELC_State() == LOAD_STATE_CONNECTED)
#define POWER_LIMITATION	(PowerLimitation != 0)
#define POWER_OPTIMIZE		(PowerLimitation == 0)
#define BATTAREY_ONCHARGE	(IbatConversion > FBC_VALUE2ADC(FBC_LI_BATTERY_CUTOFF_CURRENT, \
											  FBC_BATTERY_CURRENT_RATIO) + \
											  FBC_VALUE2ADC(FBC_LI_BATTERY_THRESHOLD_CURRENT, \
											  FBC_BATTERY_CURRENT_RATIO))
#define BATTAREY_CHARGED	(IbatConversion < FBC_VALUE2ADC(FBC_LI_BATTERY_CUTOFF_CURRENT, \
											  FBC_BATTERY_CURRENT_RATIO) - \
											  FBC_VALUE2ADC(FBC_LI_BATTERY_THRESHOLD_CURRENT, \
											  FBC_BATTERY_CURRENT_RATIO))
#define BATTERY_DISCHARGED  (VbatConversion < FBC_VALUE2ADC(FBC_LI_BATTERY_CUTOFF_VOLTAGE, \
											  FBC_BATTERY_VOLTAGE_RATIO) - \
											  FBC_VALUE2ADC(FBC_LI_BATTERY_THRESHOLD_VOLTAGE / 2, \
											  FBC_BATTERY_VOLTAGE_RATIO))
#define BATTERY_UNDERCHARGED (VbatConversion > FBC_VALUE2ADC(FBC_LI_BATTERY_CUTOFF_VOLTAGE, \
											  FBC_BATTERY_VOLTAGE_RATIO) + \
											  FBC_VALUE2ADC(FBC_LI_BATTERY_THRESHOLD_VOLTAGE / 2, \
											  FBC_BATTERY_VOLTAGE_RATIO))
												



// Actions
#define DISABLE_LIGHTING	BSP_LDC_SetState(DIM_STATE_EXTINGUISHED)
#define ENABLE_LIGHTING		{BSP_LDC_SetState(DIM_STATE_DIM); udata_LongTimeCounter = 0;}
#define ALIGHT_LIGHTING		BSP_LDC_SetState(DIM_STATE_ALIGHT)
#define DIM_LIGTHING		BSP_LDC_SetState(DIM_STATE_DIM)
#define STARTUP_CHARGER		BSP_FBC_SetState(FBC_STATE_WORKING)
#define DISCONNECT_LOAD		BSP_ELC_SetState(LOAD_STATE_DISCONNECTED)
#define CONNECT_LOAD		BSP_ELC_SetState(LOAD_STATE_CONNECTED)
#define LOW_CHARGE_CURRENT  {BatCurrentLimit = FBC_VALUE2ADC(FBC_LI_BATTERY_REDUCE_CURRENT, \
											   FBC_BATTERY_CURRENT_RATIO);}
#define HIGH_CHARGE_CURRENT  {BatCurrentLimit = FBC_VALUE2ADC(FBC_LI_BATTERY_CHARGE_CURRENT, \
											   FBC_BATTERY_CURRENT_RATIO);}

void SystemClock_Config(void);
static void MX_CRC_Init(void);
static void MX_IWDG_Init(void);

void BSP_ExecuteControl(void);
void BSP_EcecuteDisplay(void);
void BSP_ExecuteLEDshow(void);
void BSP_ExecuteCeremonial(void);
void BSP_ExecuteReadBMS(void);
void BSP_ExecuteWatchdogs(void);
void pbChargerFSM(void);

uint32_t LongTime(void);
uint32_t ROM_CRC32_code(void);
uint32_t ROM_CRC32_data(void);
BOOLEAN ROM_CRC32_Assigned(void);
BOOLEAN ROM_Integrity(void);
static BOOLEAN SunlightTrigger(void);
												 								 
uint8_t pbChargerEn=1;	
BSP_FBCState_TypeDef ch_state;

/**************************************************************************************
 * Special uninitialized segment used to storage state in stage reset
 */
uint32_t __attribute__((section (".udata"))) udata_LongTimeCounter;
uint8_t  __attribute__((section (".udata"))) udata_SunlightTrigger;
BSP_BATState_TypeDef
		 __attribute__((section (".udata"))) udata_BatteryTrigger;
extern BSP_DimState_TypeDef 				 udata_LightingState;
extern BSP_LoadState_TypeDef 				 udata_ExtLoadState;

// Resets counters
uint16_t __attribute__((section (".udata"))) udata_TemperatureResets;
uint16_t __attribute__((section (".udata"))) udata_OvervoltageResets;
uint16_t __attribute__((section (".udata"))) udata_OvercarrentResets;
uint16_t __attribute__((section (".udata"))) udata_PowergoodResets;
uint16_t __attribute__((section (".udata"))) udata_WatchdogResets;
uint16_t __attribute__((section (".udata"))) udata_ExternalResets;

/* Tiny segment in tail of ROM, consist CRC32 code of integrity.
 * Must be patched by special utility in production image of ROM*/
const uint32_t __attribute__((section (".rom_crc"))) rom_crc_code[1] = {0};
const uint32_t __attribute__((section (".rom_crc"))) rom_crc_data[1] = {0};
const char __attribute__((section (".rom_crc"))) logo[] = "g812-2_mppt-1";
/**************************************************************************************/

extern uint32_t PowerLimitation; // Bridge mode indicator for LED display

CRC_HandleTypeDef  hcrc;
IWDG_HandleTypeDef hiwdg;


int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
//  MX_IWDG_Init();
//  HAL_IWDG_Start(&hiwdg);
//  HAL_IWDG_Refresh(&hiwdg);



 /* Configure the system clock */
  SystemClock_Config();

//  MX_CRC_Init();

//  if (!ROM_Integrity())	BSP_Reset(); //VV 28.05.21 todo eliminate freeze

  /* If PowerOn initialize damaged values in memory*/
//  if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)
//	||__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
//  {
//	  udata_LongTimeCounter = 0;
//	  udata_LightingState = DIM_STATE_EXTINGUISHED;
//	  udata_SunlightTrigger = TRUE;
//	  udata_BatteryTrigger = BAT_STATE_UNDERCHARGED;
//	  udata_ExtLoadState = LOAD_STATE_DISCONNECTED;

//// Reset statistics
//	  udata_TemperatureResets = 0;
//	  udata_OvervoltageResets = 0;
//	  udata_OvercarrentResets = 0;
//	  udata_PowergoodResets   = 0;
//	  udata_WatchdogResets    = 0;
//	  udata_ExternalResets    = 0;
//  }

//  if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) udata_WatchdogResets++;
//  if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) udata_ExternalResets++;

//  __HAL_RCC_CLEAR_RESET_FLAGS();

  /* Initialize all configured peripherals */
  BSP_Init();

  // Setup from mirror the previous states when been system reset
//  BSP_LDC_SetState(BSP_LDC_State());
//  BSP_ELC_SetState(BSP_ELC_State());

  // Reset Converter
  BSP_FBC_SetState(FBC_STATE_STOPPED);

//  // Initialize tiny display
//  HAL_Delay(100);
//  SSD1306_Init();

//  // Display checksum if ROM not been patched
//  if (!ROM_CRC32_Assigned())
//  {
//	  int32_t timer; uint32_t oldtick, newtick;

//	  SSD1306_SetPos(0,0);
//	  prints("Debug11s,missingCRC!\nCodeCRC="); printi((int)(ROM_CRC32_code()),11);
//	  prints("\nDataCRC=");						printi((int)(ROM_CRC32_data()),11);
//#ifdef DEBUG
//	  prints("\nSCF= "); printi(HAL_ADCEx_Calibration_GetValue(&hadc1,ADC_SINGLE_ENDED),5);
//	  prints(", "); 	 printi(HAL_ADCEx_Calibration_GetValue(&hadc2,ADC_SINGLE_ENDED),5);
//	  prints("\nDCF= "); printi(HAL_ADCEx_Calibration_GetValue(&hadc1,ADC_DIFFERENTIAL_ENDED),5);
//	  prints(", ");		 printi(HAL_ADCEx_Calibration_GetValue(&hadc2,ADC_DIFFERENTIAL_ENDED),5);
//#else
//	  prints("\nGood lack!");
//#endif
//// Simplest show
//	  timer = 11000; oldtick = HAL_GetTick();
//	 do
//	 {
//	  BSP_ExecuteWatchdogs();
//	  SSD1306_SetPos(5,0);
//#ifdef DEBUG
//	  prints("EVR= ");	printi(HAL_ADCEx_InjectedGetValue(&hadc1,2),5);
//	  prints(", ");		printi(HAL_ADCEx_InjectedGetValue(&hadc2,2),5);
//	  prints("\nVPB= "); printi(VpanConversion,5);
//	  prints(", ");		printi(VbatConversion,5);
//#endif
//	  prints("\nIt remains "); printi(timer/1000,3);prints(" s.");
//	  newtick = HAL_GetTick(); timer -= newtick - oldtick; oldtick = newtick;
//	 }
//	 while(timer > 0);
//  }
//  SSD1306_Clear();

//  MX_I2C1_SMBUS_Init();
	
//	eMBErrorCode eStatus;	
//	__disable_irq();		
//  eStatus = eMBInit( MB_RTU, 0x0A, 0, 115200UL, MB_PAR_NONE );
//  eStatus = eMBEnable();
//	__enable_irq();

  while (TRUE)
  {
		ch_state=BSP_FBC_State();
	  BSP_ExecuteControl();
//		ch_state=BSP_FBC_State();
//	  BSP_ExecuteLEDshow();
		if(pbChargerEn==0){
			BSP_ExecuteReadBMS();
		}
		else{
			pbChargerFSM();
		}
//	  BSP_EcecuteDisplay();
//	  BSP_ExecuteCeremonial();
//	  BSP_ExecuteWatchdogs();
//		eMBPoll();
  }
}
/*-------------------------------END MAIN---------------------------------------*/

/*****************************************************************************************
 *  Main logic of automatic control full system, al0 like styled for more understanding
 * */
void BSP_ExecuteControl(void)
{
// Simplest control rules for lighting
    if (LIGHTING_ENABLED  && MORNING_DETECTED)	DISABLE_LIGHTING;
    if (LIGHTING_DISABLED && NIGHT_DETECTED)	ENABLE_LIGHTING;
    if (PRESENCE_DETECTED && LIGHTING_DIMMED)	ALIGHT_LIGHTING;
    if (PRESENCE_ENDED    && LIGHTING_ALIGHT)	DIM_LIGTHING;
// Start condition for charger
    if (CHARGER_STOPPED && SUNPOWER_DETECTED &&
    	CHARGER_REST_ENDED && TEMPERATURE_NORMAL)	STARTUP_CHARGER;
// Sync switch external load with lighting
    if (LIGHTING_ENABLED  && LOAD_DISCONNECTED)	CONNECT_LOAD;
    if (LIGHTING_DISABLED && LOAD_CONNECTED)	DISCONNECT_LOAD;
// Battery two stage charging for Li-Ion
		if(pbChargerEn==0){
			if (BATTERY_DISCHARGED)		LOW_CHARGE_CURRENT;
			if (BATTERY_UNDERCHARGED)	HIGH_CHARGE_CURRENT;
		}
}

void BSP_ExecuteLEDshow(void)
{
// 1 LED rules
	if (CHARGER_STOPPED)   						BSP_LED_SetState(BSP_LED1,LED_STATE_EXTINGUISHED);
	if (CHARGER_WORKING && POWER_OPTIMIZE) 		BSP_LED_SetState(BSP_LED1,LED_STATE_SLOW_BLINKING);
	if (CHARGER_WORKING && POWER_LIMITATION)	BSP_LED_SetState(BSP_LED1,LED_STATE_ALIGHT);
// 2 LED rules
	if (LIGHTING_DISABLED)	BSP_LED_SetState(BSP_LED2,LED_STATE_EXTINGUISHED);
	if (LIGHTING_ALIGHT)	BSP_LED_SetState(BSP_LED2,LED_STATE_FAST_BLINKING);
	if (LIGHTING_DIMMED)	BSP_LED_SetState(BSP_LED2,LED_STATE_ALIGHT);
#ifndef DEBUG
// 3 LED rules Yellow
	if (LOAD_DISCONNECTED)	BSP_LED_SetState(BSP_LED3,LED_STATE_FAST_BLINKING);
	if (LOAD_CONNECTED)		BSP_LED_SetState(BSP_LED3,LED_STATE_SLOW_BLINKING);
// 4 LED rules Blue
	if (BATTAREY_CHARGED)		BSP_LED_SetState(BSP_LED4,LED_STATE_EXTINGUISHED);
	if (BATTAREY_ONCHARGE)		BSP_LED_SetState(BSP_LED4,LED_STATE_ALIGHT);
// 5 LED rules Red
	if (LOAD_CONNECTED || LIGHTING_ENABLED)		BSP_LED_SetState(BSP_LED5,LED_STATE_ALIGHT);
	if (LOAD_DISCONNECTED && LIGHTING_DISABLED)	BSP_LED_SetState(BSP_LED5,LED_STATE_EXTINGUISHED);
#endif
}

void BSP_EcecuteDisplay(void)
{
   static int newtime, oldtime = 0;
   SSD1306_SetPos(0,0);
   {
	  prints("MPPT(");
	  prints((CHARGER_STOPPED)?"#":(PowerLimitation)?"!":(LastStepOptimize>0)?"+":"-");
	  prints(") Vb/Vp%="); printi(100*VbatConversion/VpanConversion,5);
	  prints("\nVpan=");    printi(FBC_ADC2VALUE(VpanConversion,FBC_PANEL_VOLTAGE_RATIO),5);
	  prints("  Ip=");	    printi(FBC_ADC2VALUE(IpanConversion,FBC_PANEL_CURRENT_RATIO),5);
	  prints("\nVbat=");    printi(FBC_ADC2VALUE(VbatConversion,FBC_BATTERY_VOLTAGE_RATIO),5);
	  prints("  Ib=");       printi(FBC_ADC2VALUE(IbatConversion,FBC_BATTERY_CURRENT_RATIO),5);
#ifdef DISPLAY_RESET_STATISTICS
	  prints("\nOVR=");	printi(udata_OvervoltageResets,5);
	  prints(" OCR=");	printi(udata_OvercarrentResets,5);
	  prints("\nTCR=");	printi(udata_TemperatureResets,5);
	  prints(" PGR=");	printi(udata_PowergoodResets,5);
	  prints("\nWDR=");	printi(udata_WatchdogResets,5);
	  prints(" EXR=");	printi(udata_ExternalResets,5);
#else
	  prints("\nVbms=");	printi(SMBUS_getValue(SMBUS_Volt),5);
	  prints("  Er=");		printi(SMBUS_getCntErr(SMBUS_Volt),5);
	  prints("\nIbms=");	printi(SMBUS_getValue(SMBUS_Amp),5);
	  prints("  Er=");		printi(SMBUS_getCntErr(SMBUS_Amp),5);
	  prints("\nCbms=");	printi(SMBUS_getValue(SMBUS_RemCap),5);
	  prints("  Er=");		printi(SMBUS_getCntErr(SMBUS_RemCap),5);
	  prints("\nL%=");		printi(AlightStateLevel,3);
	  prints(", ");			printi(DimmedStateLevel,3);
	  prints(" * ");
	  prints(LIGHTING_DISABLED?"Off":LIGHTING_ALIGHT?"Bright":"Dim");
#endif
	  newtime = HAL_GetTick();
	  prints("\nCycleTime="); printi(newtime - oldtime,5);
	  oldtime = newtime;
	}
}

void BSP_ExecuteCeremonial(void)
{
	uint32_t time = LongTime();
	static uint8_t last_interval = LIGHTING_CEREMONIAL_STEPS; 

	for (uint8_t i = 0; i < LIGHTING_CEREMONIAL_STEPS; i++)
	  if (Ceremonial[i].time > time)	//todo min->ms
	  { // Reset lighting intensity if interval changed
		  if (i!=last_interval)
		  {
		    AlightStateLevel = Ceremonial[i].alight;
			DimmedStateLevel = Ceremonial[i].dim;
			BSP_LDC_SetState(BSP_LDC_State());
			last_interval = i;
		  }
		  return;
	  }
	if (last_interval == LIGHTING_CEREMONIAL_STEPS) return;
	  // Default values from conf_struct.h
	 AlightStateLevel = BSP_LIGHTING_ALIGHT;
	 DimmedStateLevel = BSP_LIGHTING_DIMMED;
	 BSP_LDC_SetState(BSP_LDC_State());
	 last_interval = LIGHTING_CEREMONIAL_STEPS;
}


void BSP_ExecuteReadBMS(void)
{
	  SMBUS_task_cycle();
}

void BSP_ExecuteWatchdogs(void)
{
//...Simplest way
	HAL_IWDG_Refresh(&hiwdg);
}

typedef enum {
	Initialization=1,
	Bulk,
	Absorption,
	Equalization,
	Float
}chStates;				
static uint8_t currentState=Initialization;	
extern TIM_HandleTypeDef htim6;
extern uint16_t minutesCnt;

uint16_t VbatConversion_volt;

void pbChargerFSM(void)
{
	switch(currentState){
		case Initialization:
			if(VbatConversion < FBC_VALUE2ADC(FBC_PB_BATTERY_MIN_VOLTAGE,FBC_BATTERY_VOLTAGE_RATIO)){
				VbatConversion_volt=FBC_ADC2VALUE(VbatConversion,FBC_BATTERY_VOLTAGE_RATIO);
				break;
			}
			else{
				currentState=Bulk;
				break;
			}
		case Bulk:
			BatCurrentLimit = FBC_VALUE2ADC(FBC_PB_BATTERY_BULK_CURRENT,FBC_BATTERY_CURRENT_RATIO);
			HAL_TIM_Base_Start_IT(&htim6);
			if(minutesCnt>=FBC_PB_BULK_TIMEOUT){
				HAL_TIM_Base_Stop_IT(&htim6);
				minutesCnt=0;
				currentState=Initialization;
				return;
			}
			if(VbatConversion>=FBC_VALUE2ADC(FBC_PB_BATTERY_MAX_VOLTAGE,FBC_BATTERY_VOLTAGE_RATIO)){
//				HAL_TIM_Base_Stop_IT(&htim6);
//				minutesCnt=0;
//				currentState=Absorption;
				return;
			}
			break;
		case Absorption:
			BatVoltageLimit=FBC_VALUE2ADC(FBC_PB_BATTERY_ABSORPTION_VOLTAGE,FBC_BATTERY_CURRENT_RATIO);
			HAL_TIM_Base_Start_IT(&htim6);
			if(minutesCnt>=FBC_PB_ABSORPTION_TIMEOUT){
				HAL_TIM_Base_Stop_IT(&htim6);
				minutesCnt=0;
				currentState=Equalization;
				return;
			}
			if(IbatConversion>=FBC_VALUE2ADC(FBC_PB_BATTERY_ABSORPTION_MIN_CURRENT,FBC_BATTERY_VOLTAGE_RATIO)){
				HAL_TIM_Base_Stop_IT(&htim6);
				minutesCnt=0;
				currentState=Float;
				return;
			}
			break;
		case Equalization:
			BatVoltageLimit=FBC_VALUE2ADC(FBC_PB_BATTERY_EQUALIZATION_VOLTAGE,FBC_BATTERY_CURRENT_RATIO);
			BatCurrentLimit = FBC_VALUE2ADC(FBC_PB_BATTERY_EQUALIZATION_CURRENT,FBC_BATTERY_CURRENT_RATIO);
			HAL_TIM_Base_Start_IT(&htim6);
			if(minutesCnt>=FBC_PB_EQUALIZATION_TIMEOUT){
				HAL_TIM_Base_Stop_IT(&htim6);
				minutesCnt=0;
				currentState=Float;
				return;
			}
			break;
		case Float:
			BatVoltageLimit=FBC_VALUE2ADC(FBC_PB_BATTERY_EQUALIZATION_VOLTAGE,FBC_BATTERY_CURRENT_RATIO);
			if(VbatConversion<FBC_VALUE2ADC(FBC_PB_BATTERY_FLOAT_REDUCE_VOLTAGE,FBC_BATTERY_VOLTAGE_RATIO))\
				currentState=Bulk;
			break;
		default:
			break;
	}
}

/******************************************************************************************/

uint32_t LongTime(void)
{
	static uint32_t oldtick = 0;

	uint32_t newtick = HAL_GetTick();
	udata_LongTimeCounter += newtick - oldtick;
	oldtick = newtick;

	return udata_LongTimeCounter;
}

static BOOLEAN SunlightTrigger(void)
{
  static enum {morning, unstable, night} light = unstable;
  static uint32_t time = 0, oldtick = 0;

  uint32_t  newtick;

// Timer based on HAL Tick
  newtick = HAL_GetTick();
  if (time <= BSP_SUNLIGHT_DETECTION_TIME * 1000) {time += newtick - oldtick;}
  else if (light != unstable )
	  	  	  	  	  udata_SunlightTrigger = light == morning;
  oldtick = newtick;

// Solar light estimation in long time, when converter stopped
  if (SysBridgeTimeout == 0)
  {
   // Light metering
	 if (VpanConversion > FBC_VALUE2ADC(BSP_SUNLIGHT_MORNING,FBC_PANEL_VOLTAGE_RATIO))
	 {
	   if (light != morning) {time = 0; light = morning;};
	 }
	  else
       if (VpanConversion < FBC_VALUE2ADC(BSP_SUNLIGHT_NIGHT,FBC_PANEL_VOLTAGE_RATIO))
       {
          if (light != night) {time = 0; light = night;};
       }
       else
     	  {time = 0; light = unstable;};
  }

  return udata_SunlightTrigger;
}

int __io_putchar(int ch)
{
	SSD1306_PutChar(ch); return 0;
}

//extern char _srom_crc, _erom_crc; VV 27.05.21
char _srom_crc, _erom_crc;
uint32_t ROM_CRC32_code(void)
{
  return HAL_CRC_Accumulate(&hcrc, (uint32_t *) &_srom_crc, ((uint32_t)&_erom_crc - (uint32_t)&_srom_crc)/4);
}

//extern char _sidata, _sdata, _edata; VV 27.05.21
char _sidata, _sdata, _edata;
uint32_t ROM_CRC32_data(void)
{
  return HAL_CRC_Accumulate(&hcrc, (uint32_t *)&_sidata, ((uint32_t)&_edata - (uint32_t)&_sdata)/4);
}

BOOLEAN ROM_CRC32_Assigned(void)
{
	return rom_crc_code[1] || rom_crc_data[1];
}

BOOLEAN ROM_Integrity(void)
{
  return !ROM_CRC32_Assigned() ||
	(ROM_CRC32_code() == rom_crc_code[1] && ROM_CRC32_data() == rom_crc_data[1]);
}

/* IWDG init function */
void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Window = 40000 * SYS_I_WATCHDOG_WINDOW  / 256;
  hiwdg.Init.Reload = 40000 * SYS_I_WATCHDOG_TIMEOUT / 256;
  HAL_IWDG_Init(&hiwdg);

}

/* CRC init function */
void MX_CRC_Init(void)
{

  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLED;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
  HAL_CRC_Init(&hcrc);

}
void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc)
{
  if(hcrc->Instance==CRC)  __CRC_CLK_ENABLE();
}
/** System Clock Configuration**/

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
	
#define USE_HSE_RESONATOR

#ifdef USE_HSE_RESONATOR
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
#else
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
#endif
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_HRTIM1;
  PeriphClkInit.Hrtim1ClockSelection = RCC_HRTIM1CLK_PLLCLK;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

//	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_HRTIM1|RCC_PERIPHCLK_ADC12;
//  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
//  PeriphClkInit.Hrtim1ClockSelection = RCC_HRTIM1CLK_PLLCLK;
//	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
