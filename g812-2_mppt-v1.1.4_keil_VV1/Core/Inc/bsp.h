/*
 * bsp.h
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

#ifndef BSP_H_
#define BSP_H_

#include "stm32f3xx_hal.h"
//#include "config.h"
#include "conf_struct.h"


#ifdef NUCLEO_F334R8
#undef USE_HSE_RESONATOR
#endif

#define USE_HSE_RESONATOR
#ifdef USE_HSE_RESONATOR
#define SYSTEM_CORE_CLOCK  72000000		/*[Hz]*/
#else
#define SYSTEM_CORE_CLOCK  64000000		/*[Hz]*/
#endif

#define BSP_LED1	((uint8_t)0x01)
#define BSP_LED2	((uint8_t)0x02)
#define BSP_LED3	((uint8_t)0x04)
#define BSP_LED4	((uint8_t)0x08)
#define BSP_LED5	((uint8_t)0x10)
#define BSP_LEDALL	(BSP_LED1|BSP_LED2|BSP_LED3|BSP_LED4|BSP_LED5)

#define BSP_LED_ORANGE1		BSP_LED1
#define BSP_LED_ORANGE2		BSP_LED2
#define BSP_LED_YELLOW		BSP_LED3
#define BSP_LED_GREEN		BSP_LED4
#define BSP_LED_RED			BSP_LED5

typedef enum {
	LED_STATE_EXTINGUISHED = 0,
	LED_STATE_ALIGHT,
	LED_STATE_SLOW_BLINKING,
	LED_STATE_FAST_BLINKING
} BSP_LEDState_TypeDef;

typedef enum {
	LOAD_STATE_DISCONNECTED = 0,
	LOAD_STATE_CONNECTED = !LOAD_STATE_DISCONNECTED
} BSP_LoadState_TypeDef;

typedef enum {
	DIM_STATE_EXTINGUISHED = 0,
	DIM_STATE_ALIGHT,
	DIM_STATE_DIM
} BSP_DimState_TypeDef;

typedef enum {
	FBC_STATE_STOPPED = 0,
	FBC_STATE_WORKING = !FBC_STATE_STOPPED
} BSP_FBCState_TypeDef;

typedef enum {
	BAT_STATE_UNDERCHARGED = 0,
	BAT_STATE_CHARGED = !BAT_STATE_UNDERCHARGED
} BSP_BATState_TypeDef;

/* Options are dependent on the equipment */
// Lighting
#define FBC_DIMMING_FREQUENCY		1000	/*[Hz] integer*/

// Bridge
#define FBC_BUCKBOOST_FREQUENCY		220000	/*[Hz] integer*/
#define FBC_DEADTIME_FALL			35		/*[nS] integer*/
#define FBC_DEADTIME_RISE			35		/*[nS] integer*/
#define FBC_BUCKBOOST_DELTA			5/100 	/*[%/100] driver charge from cycle*/
#define FBC_BUCKBOOST_DUTY1			80/100	/*[%/100] buck fracture from cycle*/
#define FBC_BUCKBOOST_DUTY2			50/100	/*[%/100] boost fracture from cycle*/
#define FBC_FRACTIONS_SCALE			10000	/* Equivalent one in fractions */
#define FBC_REGULATION_AVERAGE_EXP 	3       /* 2^n members in ADC average (quick divide)*/
#define FBC_STOPPING_TRANSIENT_TIME	1000	/*[ms]*/
#define FBC_OVERLOAD_TRANSIENT_TIME 1000	/*[ms]*/

// MPPT optimizer
#define FBC_OPTIMIZE_AVERAGE_EXP	8       /*8 is max*/                                       		/* 2^n members in power average (quick divide)*/
#define FBC_OPTIMIZE_TRANSIENT_TIME 2		/*[ms]*/
#define FBC_OPTIMIZE_VOLTAGE_STEP	200		/*[mV]*/

// Ratio of resistors in measurement circuits
#define FBC_PANEL_VOLTAGE_RATIO		(3300+47000)/3300	/*[Ohm/Ohm] 50300 mV max*/
#define FBC_BATTERY_VOLTAGE_RATIO	(3300+47000)/3300	/*[Ohm/Ohm] 50300 mV max*/
#define FBC_PANEL_CURRENT_RATIO		499000/24099/4		/*[mOhm/Ohm/Ohm] 15024 mA max*/
#define FBC_BATTERY_CURRENT_RATIO	499000/24099/4	 	/*[mOhm/Ohm/Ohm] 15024 mA max*/

/* Voltage references, physical values*/
#define FBC_REFERENCE_VOLTAGE 		3000	/*[mV] voltage of external reference source */
#define FBC_REFANALOG_VOLTAGE 		3300	/*[mV] supply voltage of analog hardware */

/* Helpers macro, depend on the configuration, do not change it*/
#define FBC_REFERENCE_CORRECT 		FBC_REFERENCE_VOLTAGE * 4096 / FBC_REFANALOG_VOLTAGE
#define FBC_VALUE2ADC(value, ratio) (10 * 4096 * value / FBC_REFANALOG_VOLTAGE * \
									 100 / (1000 * ratio))
#define FBC_ADC2VALUE(code, ratio)  (code * ratio * FBC_REFANALOG_VOLTAGE / 4096)

/* The calculations depend on the configuration, do not change it */
#define FBC_BUCKBOOST_PERIOD  		(SYSTEM_CORE_CLOCK / 10 * 64 / (FBC_BUCKBOOST_FREQUENCY / 10))
#define FBC_BUCKBOOST_WIDTH(duty)	FBC_BUCKBOOST_PERIOD * duty / 100
#define FBC_DIMMING_PERIOD			(SYSTEM_CORE_CLOCK / 10 / 2 / (FBC_DIMMING_FREQUENCY / 10))
#define FBC_DIMMING_WIDTH(duty)		FBC_DIMMING_PERIOD * duty / 100
#define FBC_CONVERSION_WIDTH		5120

/* Minimal indication time of state */
#define FBC_POWER_LIMITATION_FLARE	1000	/*[ms]*/

/* Timeout calculation helper based on TIM3 */
#define BSP_APPROXIMATE_TIMEOUT(timesec) (FBC_DIMMING_FREQUENCY * timesec)

/* Timeout calculation helper based on HRTIM */
#define BSP_REGULATIONS_TIMEOUT(milisec) (FBC_BUCKBOOST_FREQUENCY / \
										 (1 << FBC_REGULATION_AVERAGE_EXP) * milisec / 1000)

/* Active limits of charging (0..4095) in ADC units */
extern volatile  uint16_t BatCurrentLimit;
extern volatile  uint16_t BatVoltageLimit;

/* Injected conversions of ADC, averaged (0..4095) */
extern volatile  uint16_t Ref1Conversion;
extern volatile  uint16_t Ref2Conversion;
extern volatile  uint16_t VpanConversion;
extern volatile  uint16_t VbatConversion;
extern volatile  uint16_t IpanConversion;
extern volatile  uint16_t IbatConversion;

/* Regulators */
extern volatile   int32_t BridgeGearNumber;
extern volatile  uint32_t DimmingDutyCycle;

/* MPPT optimizer */
extern volatile  uint32_t LastPowerOfPanel;
extern volatile  int16_t  LastStepOptimize;
extern volatile  uint16_t PanelVoltageMPPT;

/* Auto decrement timeout counters */
extern volatile  uint32_t SysSharedTimeout;
extern volatile  uint32_t SysBridgeTimeout;

/* Presence detection timeout */
extern volatile  uint32_t PresenceDetector;

/* Active dimming levels [%]*/
extern volatile  uint8_t  AlightStateLevel;
extern volatile  uint8_t  DimmedStateLevel;

void BSP_Init(void);	// Initialization of hardware
void BSP_Reset(void);	// Global system restart

/* External Load Control */
void BSP_ELC_SetState(BSP_LoadState_TypeDef state);
BSP_LoadState_TypeDef BSP_ELC_State(void);

/* Light Dimming Control */
void BSP_LDC_SetState(BSP_DimState_TypeDef state);
BSP_DimState_TypeDef  BSP_LDC_State(void);

/* LED Display */
void BSP_LED_SetState(uint8_t leds, BSP_LEDState_TypeDef state);
BSP_LEDState_TypeDef BSP_LED_State(uint8_t leds);
/* Return: LED_STATE_EXTINGUISHED if all requested LEDs extinguished
 * 		   LED_STATE_SLOW_BLINKING if all requested LEDs blinking slow
 * 		   LED_STATE_FAST_BLINKING if all requested LEDs blinking fast
 * 		   LED_STATE_ALIGHT in all other cases.	 */

/* Full Bridge Converter */
void BSP_FBC_SetState( BSP_FBCState_TypeDef state);
void BSP_FBC_Reset(void); /* Recalculate and setup duty cycles for
							  * actual voltage ratio and power MPPT*/
BSP_FBCState_TypeDef  BSP_FBC_State(void);

void BSP_FBC_SetGearNumber(void);
void BSP_FBC_CutGearNumber(void);
void BSP_FBC_SetDutyCycles(void);

#endif /* BSP_H_ */


