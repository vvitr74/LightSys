#ifndef _CONF_STRUCT_H
#define _CONF_STRUCT_H


#include "stm32f3xx.h"

extern const uint16_t conf[50];

//const uint16_t conf[]={
//	//On/Off charger parameters
//	12000,	//[0] FBC_START_PANEL_VOLTAGE, mV
//	300,		//[1] FBC_STOP_PANEL_CURRENT, mA
//	41000,	//[2] FBC_LI_BATTERY_DOWNED_VOLTAGE, mV
//	10000,	//[3] FBC_PB_BATTERY_DOWNED_VOLTAGE, mV
//	5/1,		//[4] FBC_START_REPEAT_PAUSE, s
//	
//	// Optimization constraints
//	43000,	//[5] FBC_OPTIMIZE_MAX_VOLTAGE, mV
//	5000,		//[6] FBC_OPTIMIZE_MIN_VOLTAGE, mV
//	
//	// Sunlight detection parameters
//	7000,		//[7] BSP_SUNLIGHT_MORNING, mV
//	5000,		//[8] BSP_SUNLIGHT_NIGHT, mV
//	10/1,		//[9] BSP_SUNLIGHT_DETECTION_TIME, s
//	
//	// Lighting control settings
//	10/1,	 //[10] BSP_LIGHTING_TIME, s
//	80/1,	 //[11] BSP_LIGHTING_ALIGHT, [%]
//	10/1,	 //[12] BSP_LIGHTING_DIMMED, [%]
//	100,	 //[13] BSP_DIMMING_VELOCITY, [%]/[s]
//	10/1,	 //[14] BSP_CONTROL_LOGIC_TIMEOUT, [s]
//	23,		 //[15] LIGHTING_CEREMONIAL_STEPS
//	
//	// Presence sensor settings
//	1,		 //[16] BSP_PRESENCE_POLAITY_HIGH
//	500,	 //[17] BSP_PRESENCE_DETECTION_TIME, ms
//	
//	// Li battery default charge profile
//	42000,	//[18] FBC_LI_BATTERY_CHARGE_VOLTAGE, mV
//	8000,		//[19] FBC_LI_BATTERY_CHARGE_CURRENT, mA
//	31000,	//[20] FBC_LI_BATTERY_REDUCE_VOLTAGE, mV
//	1000,		//[21] FBC_LI_BATTERY_REDUCE_CURRENT, mA
//	26000,	//[22] FBC_LI_BATTERY_CUTOFF_VOLTAGE, mV
//	500,		//[23] FBC_LI_BATTERY_CUTOFF_CURRENT, mA
//	50,			//[24] FBC_LI_BATTERY_THRESHOLD_VOLTAGE, mV
//	100,		//[25] FBC_LI_BATTERY_THRESHOLD_CURRENT, mA
//	
//	// Pb battery default charge profile
//	//todo
//};

#define CEREMONIAL_TIME(minutes)		(minutes * 60000)
#define LED_SLOW_BLINK_FREQUENCY		1/1		/*[Hz]*/
#define LED_FAST_BLINK_FREQUENCY		10/1	/*[Hz]*/

//On/Off charger parameters
#define FBC_START_PANEL_VOLTAGE 					12000//conf[0]
#define FBC_STOP_PANEL_CURRENT  					conf[1]
#define FBC_LI_BATTERY_DOWNED_VOLTAGE 		conf[2]
#define FBC_PB_BATTERY_DOWNED_VOLTAGE 		conf[3]
#define FBC_START_REPEAT_PAUSE						conf[4]

// Optimization constraints
#define FBC_OPTIMIZE_MAX_VOLTAGE					conf[5]
#define FBC_OPTIMIZE_MIN_VOLTAGE					conf[6]

// Sunlight detection parameters
#define BSP_SUNLIGHT_MORNING							conf[7]
#define BSP_SUNLIGHT_NIGHT								conf[8]
#define BSP_SUNLIGHT_DETECTION_TIME				conf[9]

// Lighting control settings
#define BSP_LIGHTING_TIME									conf[10]
#define BSP_LIGHTING_ALIGHT								80/1//conf[11]
#define BSP_LIGHTING_DIMMED								10/1//conf[12]
#define BSP_DIMMING_VELOCITY							conf[13]
#define BSP_CONTROL_LOGIC_TIMEOUT					conf[14]
#define LIGHTING_CEREMONIAL_STEPS					23//conf[15]

// Presence sensor settings
#define BSP_PRESENCE_POLAITY_HIGH					conf[16]
#define BSP_PRESENCE_DETECTION_TIME				conf[17]

// Li battery default charge profile
#define FBC_LI_BATTERY_CHARGE_VOLTAGE			42000//conf[18]
#define FBC_LI_BATTERY_CHARGE_CURRENT			8000//conf[19]
#define FBC_LI_BATTERY_REDUCE_VOLTAGE			conf[20]
#define FBC_LI_BATTERY_REDUCE_CURRENT			conf[21]
#define FBC_LI_BATTERY_CUTOFF_VOLTAGE			conf[22]
#define	FBC_LI_BATTERY_CUTOFF_CURRENT			conf[23]
#define FBC_LI_BATTERY_THRESHOLD_VOLTAGE	conf[24]
#define FBC_LI_BATTERY_THRESHOLD_CURRENT	conf[25]

// Pb battery default charge profile
#define FBC_PB_BATTERY_MIN_VOLTAGE				4000	//mV
#define FBC_PB_BATTERY_MAX_VOLTAGE				13600	//mV
#define FBC_PB_BATTERY_CAP								60000	//mA/h
#define FBC_PB_BATTERY_BULK_CURRENT 0.1*FBC_PB_BATTERY_CAP	//mA
#define FBC_PB_BULK_TIMEOUT	4320	//min
#define FBC_PB_BATTERY_ABSORPTION_VOLTAGE	14500	//mV
#define FBC_PB_BATTERY_ABSORPTION_MIN_CURRENT	0.02*FBC_PB_BATTERY_CAP	//mA
#define FBC_PB_ABSORPTION_TIMEOUT	400	//min
#define FBC_PB_BATTERY_EQUALIZATION_VOLTAGE	15500	//mV
#define FBC_PB_BATTERY_EQUALIZATION_CURRENT	0.03*FBC_PB_BATTERY_CAP	//mA
#define FBC_PB_EQUALIZATION_TIMEOUT	20	//min
#define FBC_PB_BATTERY_FLOAT_VOLTAGE	13500	//mV
#define FBC_PB_BATTERY_FLOAT_REDUCE_VOLTAGE	10000	//mV

typedef struct
	{
		uint16_t	time;		/*min*/
		uint8_t		alight;		/*[%]*/
		uint8_t		dim;		/*[%]*/
	}
		Ceremonial_Struct;

extern const Ceremonial_Struct Ceremonial[LIGHTING_CEREMONIAL_STEPS];


#endif
