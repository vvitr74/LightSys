/**
  ******************************************************************************
  * @file    stm32f3xx_it.c
  * @brief   Interrupt Service Routines.
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
/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_tim.h"
#include "stm32f3xx_hal_smbus.h"
#include "stm32f3xx_hal_crc.h"
#include "stm32f3xx.h"
#include "stm32f3xx_it.h"
#include "bsp.h"
#include <stdlib.h>

/* Timers for waiting end of transient process */
volatile  uint32_t StoppingTimeout = 0;
volatile  uint32_t OverloadTimeout = 0;

/* Timers for display converter modes */
volatile  uint32_t PowerLimitation = 0;


// ADC noise filtering
static uint32_t vpanaverage = 0,	vbataverage = 0,
				ipanaverage = 0,	ibataverage = 0,
				ref1average = 0,	ref2average = 0;
static uint16_t averagecount = 0;

/* External variables --------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern HRTIM_HandleTypeDef hhrtim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern SMBUS_HandleTypeDef hsmbus1;

extern BSP_FBCState_TypeDef ch_state;

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

/******************************************************************************/
/* STM32F3xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f3xx.s).                    */
/******************************************************************************/
/**
* @brief This function handles ADC1 and ADC2 interrupts.
*/

uint32_t adc_data=0;
uint32_t adc_volt=0;
void ADC1_2_IRQHandler(void)
{
	if (__HAL_ADC_GET_FLAG(&hadc1,ADC_FLAG_JEOS) &&
	    __HAL_ADC_GET_FLAG(&hadc2,ADC_FLAG_JEOS))
	{
	  __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_JEOS);
	  __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_JEOS);
		
		adc_data=hadc1.Instance->JDR3;
		adc_volt=FBC_ADC2VALUE(adc_data, FBC_PANEL_VOLTAGE_RATIO);
//		adc_volt=FBC_ADC2VALUE(adc_data, 1);

	  vpanaverage += hadc1.Instance->JDR1;
	  ref1average += hadc1.Instance->JDR2;
	  vbataverage += hadc1.Instance->JDR3;
	  ipanaverage += hadc2.Instance->JDR1;
	  ref2average += hadc2.Instance->JDR2;
	  ibataverage += hadc2.Instance->JDR3;
	  averagecount--;

	  if (!averagecount)
	  {
		VpanConversion = vpanaverage >> FBC_REGULATION_AVERAGE_EXP;
		VbatConversion = vbataverage >> FBC_REGULATION_AVERAGE_EXP;
		IpanConversion = ipanaverage >> FBC_REGULATION_AVERAGE_EXP;
		IbatConversion = ibataverage >> FBC_REGULATION_AVERAGE_EXP;
		Ref1Conversion = ref1average >> FBC_REGULATION_AVERAGE_EXP;
		Ref2Conversion = ref2average >> FBC_REGULATION_AVERAGE_EXP;
		averagecount = 1 << FBC_REGULATION_AVERAGE_EXP;
		vpanaverage = vbataverage = ipanaverage =
		ibataverage = ref1average = ref2average = 0;
	  }
	}
	else  // Call standard handler
	  {
		HAL_ADC_IRQHandler(&hadc1);
		HAL_ADC_IRQHandler(&hadc2);
	  }
}

/**
* @brief This function handles HRTIM master timer global interrupt.
*/
void HRTIM1_Master_IRQHandler(void)
{
// Accelerate the calculation of loop back.
  if (__HAL_HRTIM_MASTER_GET_FLAG(&hhrtim1, HRTIM_MASTER_FLAG_MREP) != RESET &&	//HRTIM_MASTER_FLAG_MREP - Master Repetition interrupt flag (bit MREP in MISR)
	  __HAL_HRTIM_MASTER_GET_ITSTATUS(&hhrtim1, HRTIM_MASTER_IT_MREP) != RESET)		//HRTIM_MASTER_IT_MREP - Master Repetition interrupt enable 
   {
		 
	int16_t vpan, ipan, vbat, ibat
#ifdef USE_ADDITIONAL_VOLTAGE_REFERENCE
	, ref1, ref2
#endif
	;
	static uint32_t optimizetime = 0;
	static uint64_t power = 0;

	__HAL_HRTIM_MASTER_CLEAR_IT(&hhrtim1, HRTIM_MASTER_IT_MREP);

	if (BSP_FBC_State() == FBC_STATE_STOPPED) BSP_FBC_Reset();
	else
	{
	/* Begin of critical section, disable preemption */
//	__disable_fault_irq();
	__disable_irq();
	vpan = VpanConversion;    vbat = VbatConversion;
	ipan = IpanConversion;    ibat = IbatConversion;
#ifdef USE_ADDITIONAL_VOLTAGE_REFERENCE
	ref1 = Ref1Conversion;    ref2 = Ref2Conversion;
#endif
	/* End of critical section, enable preemption*/
//	__enable_fault_irq();	
	__enable_irq();


	// MPPT optimization
	optimizetime++;

	if (optimizetime > BSP_REGULATIONS_TIMEOUT(FBC_OPTIMIZE_TRANSIENT_TIME))
	{
		power += (vpan * ipan);
		if (optimizetime >= (BSP_REGULATIONS_TIMEOUT(FBC_OPTIMIZE_TRANSIENT_TIME)+
							(1 << FBC_OPTIMIZE_AVERAGE_EXP)))
		{
			power >>= FBC_OPTIMIZE_AVERAGE_EXP; // Calculate average
			if (power  <= LastPowerOfPanel)
			{// Change search direction
				 LastStepOptimize = (LastStepOptimize > 0)?
				-FBC_VALUE2ADC(FBC_OPTIMIZE_VOLTAGE_STEP, FBC_PANEL_VOLTAGE_RATIO):
				+FBC_VALUE2ADC(FBC_OPTIMIZE_VOLTAGE_STEP, FBC_PANEL_VOLTAGE_RATIO);
			}
			//else if (power == LastPowerOfPanel) LastStepOptimize = 0;

			PanelVoltageMPPT += LastStepOptimize;
			// Deviation range limitation
			if (PanelVoltageMPPT < FBC_VALUE2ADC(FBC_OPTIMIZE_MIN_VOLTAGE,
														 FBC_PANEL_VOLTAGE_RATIO))
				PanelVoltageMPPT = FBC_VALUE2ADC(FBC_OPTIMIZE_MIN_VOLTAGE,
														 FBC_PANEL_VOLTAGE_RATIO);
			else
			if (PanelVoltageMPPT > FBC_VALUE2ADC(FBC_OPTIMIZE_MAX_VOLTAGE,
														 FBC_PANEL_VOLTAGE_RATIO))
				PanelVoltageMPPT = FBC_VALUE2ADC(FBC_OPTIMIZE_MAX_VOLTAGE,
														 FBC_PANEL_VOLTAGE_RATIO);
			LastPowerOfPanel = power;
			power = optimizetime = 0;
		}
	}

	// Main regulation rules for current, voltage and MPPT optimizing
	if ((ibat
#ifdef USE_ADDITIONAL_VOLTAGE_REFERENCE
		- (ref2 - FBC_REFERENCE_CORRECT)
#endif
		) > BatCurrentLimit || (vbat
#ifdef USE_ADDITIONAL_VOLTAGE_REFERENCE
		- (ref1 - FBC_REFERENCE_CORRECT)
#endif
		) > BatVoltageLimit)
	{
		BridgeGearNumber--;
		power = optimizetime = 0; /* Reset optimizer */
		PanelVoltageMPPT = FBC_VALUE2ADC(FBC_START_PANEL_VOLTAGE,
										 FBC_PANEL_VOLTAGE_RATIO);
		/* Waiting when transient to be ended */
		if (OverloadTimeout) OverloadTimeout--;
		else{
			ch_state=BSP_FBC_State();	//for debug
			BSP_FBC_SetState(FBC_STATE_STOPPED);
			ch_state=BSP_FBC_State();	//for debug
		}
		PowerLimitation = BSP_REGULATIONS_TIMEOUT(FBC_POWER_LIMITATION_FLARE);
	}
	else
	  {
		OverloadTimeout = BSP_REGULATIONS_TIMEOUT(FBC_OVERLOAD_TRANSIENT_TIME);
	    if (ipan < FBC_VALUE2ADC(FBC_STOP_PANEL_CURRENT,FBC_PANEL_CURRENT_RATIO))
	    {
		  BridgeGearNumber++;
		  power = optimizetime = 0; /* Reset optimizer */
		  PanelVoltageMPPT = FBC_VALUE2ADC(FBC_START_PANEL_VOLTAGE,
		  								   FBC_PANEL_VOLTAGE_RATIO);
		  /* Waiting when transient to be ended */
				if (StoppingTimeout) StoppingTimeout--;
				else
						BSP_FBC_SetState(FBC_STATE_STOPPED);
				}
	    else
	    {
	  	  StoppingTimeout = BSP_REGULATIONS_TIMEOUT(FBC_STOPPING_TRANSIENT_TIME);
	 	  if (vpan > PanelVoltageMPPT) 
				BridgeGearNumber++;
			else
				BridgeGearNumber--;
	    }
	  }

	BSP_FBC_CutGearNumber();
	// Setup new PWM values
	BSP_FBC_SetDutyCycles();
	SysBridgeTimeout = BSP_APPROXIMATE_TIMEOUT(FBC_START_REPEAT_PAUSE);
	}
	if (PowerLimitation) PowerLimitation--;
   }
  else // Standard handler
	  HAL_HRTIM_IRQHandler(&hhrtim1,HRTIM_TIMERINDEX_MASTER);
}

/**
* @brief This function handles RCC global interrupt.
*/
void RCC_IRQHandler(void) {BSP_Reset();}

/**
* @brief This function handles EXTI line[15:10] interrupts.
*/
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

/* From Master */
void EXTI2_TSC_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
}

void TIM3_IRQHandler(void)
{
   HAL_TIM_IRQHandler(&htim3);
}

uint16_t minutesCnt;

void TIM6_DAC1_IRQHandler(void)
{
	if(__HAL_TIM_GET_FLAG(&htim6,TIM_SR_UIF)){
		__HAL_TIM_CLEAR_FLAG(&htim6,TIM_SR_UIF);
		minutesCnt++;
	}
	else
		HAL_TIM_IRQHandler(&htim6);
}


void I2C1_EV_IRQHandler(void)
{
  HAL_SMBUS_EV_IRQHandler(&hsmbus1);
}


void I2C1_ER_IRQHandler(void)
{
  HAL_SMBUS_ER_IRQHandler(&hsmbus1);
}

/****** Redirect from Deadlock to Reset *******************************/

void NMI_Handler(void)				{BSP_Reset();}
void HardFault_Handler(void)		{BSP_Reset();}
void MemManage_Handler(void)		{BSP_Reset();}
void BusFault_Handler(void)			{BSP_Reset();}
void UsageFault_Handler(void)		{BSP_Reset();}
void SVC_Handler(void)				{BSP_Reset();}
void DebugMon_Handler(void)			{BSP_Reset();}
void PendSV_Handler(void)			{BSP_Reset();}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
