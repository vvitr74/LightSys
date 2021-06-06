/*
 * config.h
 *
 *  Created on: 30 янв. 2016
 *      Author: Viacheslav Azarov
 *      Default Parameters of algorithms
 ******************************************************************************
 *  This software developed by engineer Viacheslav Azarov with free tools
 *  from STMicroelectronics on personal wish employer Danil Rogalev and saves
 *  all rights all authors and contributors.
 ******************************************************************************
 */

#include <stdint.h>

#ifndef CONFIG_H_
#define CONFIG_H_

// Start input voltage for converter
#define FBC_START_PANEL_VOLTAGE			12000	/*mV*/
#define FBC_STOP_PANEL_CURRENT			300		/*mA*/
#define FBC_START_REPEAT_PAUSE			5/1		/*s*/

// Optimization constraints
#define FBC_OPTIMIZE_MAX_VOLTAGE		43000	/*mV*/
#define FBC_OPTIMIZE_MIN_VOLTAGE		5000	/*mV*/

// Battery default charge profile
#define FBC_BATTERY_CHARGE_VOLTAGE		42000	/*mV*/
#define FBC_BATTERY_CHARGE_CURRENT		8000	/*mA*/
#define FBC_BATTERY_REDUCE_VOLTAGE		31000	/*mV*/
#define FBC_BATTERY_REDUCE_CURRENT		1000	/*mA*/
#define FBC_BATTERY_CUTOFF_CURRENT		500		/*mA*/
#define FBC_BATTERY_CUTOFF_VOLTAGE		26000	/*mV*/
#define FBC_BATTERY_THRESHOLD_CURRENT	100		/*mV*/
#define FBC_BATTERY_THRESHOLD_VOLTAGE	50		/*mA*/


// Sunlight detection parameters
#define BSP_SUNLIGHT_MORNING			7000	/*mV*/
#define BSP_SUNLIGHT_NIGHT				5000	/*mV*/
#define BSP_SUNLIGHT_DETECTION_TIME		10/1	/*s*/

// Lighting control settings
#define BSP_LIGHTING_TIME				10/1	/*s*/
#define BSP_LIGHTING_ALIGHT				80/1	/*[%]*/
#define BSP_LIGHTING_DIMMED				10/1	/*[%]*/
#define BSP_DIMMING_VELOCITY			100		/*[%]/[s]*/

// Blinking parameters
#define LED_SLOW_BLINK_FREQUENCY		1/1		/*[Hz]*/
#define LED_FAST_BLINK_FREQUENCY		10/1	/*[Hz]*/

// Control logic timeout
#define BSP_CONTROL_LOGIC_TIMEOUT		10/1 	/*[s]*/

// Presence sensor settings
#define BSP_PRESENCE_POLAITY_HIGH               // Comment for reverse polarity
#define BSP_PRESENCE_DETECTION_TIME		1/2 	/*[s]*/

#define LIGHTING_CEREMONIAL_STEPS		23

typedef struct
	{
		uint32_t	time;		/*ms*/
		uint8_t		alight;		/*[%]*/
		uint8_t		dim;		/*[%]*/
	}
		Ceremonial_Struct;

extern const Ceremonial_Struct Ceremonial [LIGHTING_CEREMONIAL_STEPS];

#define CEREMONIAL_TIME(minutes)		(minutes * 60000)

#endif /* CONFIG_H_ */
