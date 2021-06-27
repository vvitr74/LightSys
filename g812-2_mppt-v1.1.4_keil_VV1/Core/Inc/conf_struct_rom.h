#ifndef _CONF_STRUCT_H
#define _CONF_STRUCT_H


#include "stm32f3xx.h"

#define FLASH_START_ADDR		0x0800F000

uint16_t readFlash(uint32_t addr);

#define CEREMONIAL_TIME(minutes)		(minutes * 60000)
#define LED_SLOW_BLINK_FREQUENCY		1/1		/*[Hz]*/
#define LED_FAST_BLINK_FREQUENCY		10/1	/*[Hz]*/

//On/Off charger parameters
#define FBC_START_PANEL_VOLTAGE 					12000//*(volatile uint16_t*)(FLASH_START_ADDR+0)
#define FBC_STOP_PANEL_CURRENT  					*(volatile uint16_t*)(FLASH_START_ADDR+2)
#define FBC_LI_BATTERY_DOWNED_VOLTAGE 		*(volatile uint16_t*)(FLASH_START_ADDR+4)
#define FBC_PB_BATTERY_DOWNED_VOLTAGE 		*(volatile uint16_t*)(FLASH_START_ADDR+6)
#define FBC_START_REPEAT_PAUSE						*(volatile uint16_t*)(FLASH_START_ADDR+8)
#define FBC_TYPE													*(volatile uint16_t*)(FLASH_START_ADDR+10)

// Optimization constraints
#define FBC_OPTIMIZE_MAX_VOLTAGE					*(volatile uint16_t*)(FLASH_START_ADDR+12)
#define FBC_OPTIMIZE_MIN_VOLTAGE					*(volatile uint16_t*)(FLASH_START_ADDR+14)

// Sunlight detection parameters
#define BSP_SUNLIGHT_MORNING							*(volatile uint16_t*)(FLASH_START_ADDR+16)
#define BSP_SUNLIGHT_NIGHT								*(volatile uint16_t*)(FLASH_START_ADDR+18)
#define BSP_SUNLIGHT_DETECTION_TIME				*(volatile uint16_t*)(FLASH_START_ADDR+20)

// Lighting control settings
#define BSP_LIGHTING_TIME									*(volatile uint16_t*)(FLASH_START_ADDR+22)
#define BSP_LIGHTING_ALIGHT								80/1//*(volatile uint16_t*)(FLASH_START_ADDR+24)
#define BSP_LIGHTING_DIMMED								10/1//*(volatile uint16_t*)(FLASH_START_ADDR+26)
#define BSP_DIMMING_VELOCITY							*(volatile uint16_t*)(FLASH_START_ADDR+28)
#define BSP_CONTROL_LOGIC_TIMEOUT					*(volatile uint16_t*)(FLASH_START_ADDR+30)
#define LIGHTING_CEREMONIAL_STEPS					23//*(volatile uint16_t*)(FLASH_START_ADDR+32)

// Presence sensor settings
#define BSP_PRESENCE_POLAITY_HIGH					*(volatile uint16_t*)(FLASH_START_ADDR+34)
#define BSP_PRESENCE_DETECTION_TIME				*(volatile uint16_t*)(FLASH_START_ADDR+36)

// Li battery default charge profile
#define FBC_LI_BATTERY_CHARGE_VOLTAGE			*(volatile uint16_t*)(FLASH_START_ADDR+38)
#define FBC_LI_BATTERY_CHARGE_CURRENT			*(volatile uint16_t*)(FLASH_START_ADDR+40)
#define FBC_LI_BATTERY_REDUCE_VOLTAGE			*(volatile uint16_t*)(FLASH_START_ADDR+42)
#define FBC_LI_BATTERY_REDUCE_CURRENT			*(volatile uint16_t*)(FLASH_START_ADDR+44)
#define FBC_LI_BATTERY_CUTOFF_VOLTAGE			*(volatile uint16_t*)(FLASH_START_ADDR+46)
#define	FBC_LI_BATTERY_CUTOFF_CURRENT			*(volatile uint16_t*)(FLASH_START_ADDR+48)
#define FBC_LI_BATTERY_THRESHOLD_VOLTAGE	*(volatile uint16_t*)(FLASH_START_ADDR+50)
#define FBC_LI_BATTERY_THRESHOLD_CURRENT	*(volatile uint16_t*)(FLASH_START_ADDR+52)

// Pb battery default charge profile
#define FBC_PB_BATTERY_MIN_VOLTAGE							*(volatile uint16_t*)(FLASH_START_ADDR+54)	//mV
#define FBC_PB_BATTERY_MAX_VOLTAGE							13600//*(volatile uint16_t*)(FLASH_START_ADDR+56)	//mV
#define FBC_PB_BATTERY_CAP											50000//*(volatile uint16_t*)(FLASH_START_ADDR+58)	//mA/h
#define FBC_PB_BATTERY_BULK_CURRENT 						0.08*FBC_PB_BATTERY_CAP	//mA
#define FBC_PB_BULK_TIMEOUT											*(volatile uint16_t*)(FLASH_START_ADDR+60)	//min
#define FBC_PB_BATTERY_ABSORPTION_VOLTAGE				*(volatile uint16_t*)(FLASH_START_ADDR+62)	//mV
#define FBC_PB_BATTERY_ABSORPTION_MIN_CURRENT		0.02*FBC_PB_BATTERY_CAP	//mA
#define FBC_PB_ABSORPTION_TIMEOUT								*(volatile uint16_t*)(FLASH_START_ADDR+64)	//min
#define FBC_PB_BATTERY_EQUALIZATION_VOLTAGE			*(volatile uint16_t*)(FLASH_START_ADDR+66)	//mV
#define FBC_PB_BATTERY_EQUALIZATION_CURRENT			0.03*FBC_PB_BATTERY_CAP	//mA
#define FBC_PB_EQUALIZATION_TIMEOUT							*(volatile uint16_t*)(FLASH_START_ADDR+68)	//min
#define FBC_PB_BATTERY_FLOAT_VOLTAGE						*(volatile uint16_t*)(FLASH_START_ADDR+70)	//mV
#define FBC_PB_BATTERY_FLOAT_REDUCE_VOLTAGE			*(volatile uint16_t*)(FLASH_START_ADDR+72)	//mV

typedef struct
	{
		uint16_t	time;		/*min*/
		uint8_t		alight;		/*[%]*/
		uint8_t		dim;		/*[%]*/
	}
		Ceremonial_Struct;

extern const Ceremonial_Struct Ceremonial[LIGHTING_CEREMONIAL_STEPS];


#endif
