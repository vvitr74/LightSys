#include "conf_struct.h"

const uint16_t conf[]={
	//On/Off charger parameters
	12000,	//[0] FBC_START_PANEL_VOLTAGE, mV
	100,		//[1] FBC_STOP_PANEL_CURRENT, mA
	41000,	//[2] FBC_LI_BATTERY_DOWNED_VOLTAGE, mV
	10000,	//[3] FBC_PB_BATTERY_DOWNED_VOLTAGE, mV
	5/1,		//[4] FBC_START_REPEAT_PAUSE, s
	
	// Optimization constraints
	43000,	//[5] FBC_OPTIMIZE_MAX_VOLTAGE, mV
	5000,		//[6] FBC_OPTIMIZE_MIN_VOLTAGE, mV
	
	// Sunlight detection parameters
	7000,		//[7] BSP_SUNLIGHT_MORNING, mV
	5000,		//[8] BSP_SUNLIGHT_NIGHT, mV
	10/1,		//[9] BSP_SUNLIGHT_DETECTION_TIME, s
	
	// Lighting control settings
	10/1,	 //[10] BSP_LIGHTING_TIME, s
	80/1,	 //[11] BSP_LIGHTING_ALIGHT, [%]
	10/1,	 //[12] BSP_LIGHTING_DIMMED, [%]
	100,	 //[13] BSP_DIMMING_VELOCITY, [%]/[s]
	10/1,	 //[14] BSP_CONTROL_LOGIC_TIMEOUT, [s]
	23,		 //[15] LIGHTING_CEREMONIAL_STEPS
	
	// Presence sensor settings
	1,		 //[16] BSP_PRESENCE_POLAITY_HIGH
	500,	 //[17] BSP_PRESENCE_DETECTION_TIME, ms
	
	// Li battery default charge profile
	42000,	//[18] FBC_LI_BATTERY_CHARGE_VOLTAGE, mV
	8000,		//[19] FBC_LI_BATTERY_CHARGE_CURRENT, mA
	31000,	//[20] FBC_LI_BATTERY_REDUCE_VOLTAGE, mV
	1000,		//[21] FBC_LI_BATTERY_REDUCE_CURRENT, mA
	26000,	//[22] FBC_LI_BATTERY_CUTOFF_VOLTAGE, mV
	500,		//[23] FBC_LI_BATTERY_CUTOFF_CURRENT, mA
	50,			//[24] FBC_LI_BATTERY_THRESHOLD_VOLTAGE, mV
	100,		//[25] FBC_LI_BATTERY_THRESHOLD_CURRENT, mA
	
	// Pb battery default charge profile
	//todo
};

const Ceremonial_Struct Ceremonial[23]={
	{(4), 100, 100}, // 0..4 min Alight=100% Dim=100%
	{(1000 + 4), 100, 96},
	{(2000 + 4), 100, 92},
	{(3000 + 4), 100, 88},
	{(4000 + 4), 100, 84},
	{(5000 + 4), 100, 80},
	{(6000 + 4), 100, 76},
	{(7000 + 4), 100, 72},
	{(8000 + 4), 100, 68},
	{(9000 + 4), 100, 64},
	{(10000 + 4), 100, 60},

	{(6), 100, 60},  // 4:10..6 min Alight=100% Dim=90%
	{(1000 + 6), 100, 57},
	{(2000 + 6), 100, 54},
	{(3000 + 6), 100, 51},
	{(4000 + 6), 100, 48},
	{(5000 + 6), 100, 45},
	{(6000 + 6), 100, 42},
	{(7000 + 6), 100, 39},
	{(8000 + 6), 100, 36},
	{(9000 + 6), 100, 33},
	{(10000 + 6), 100, 30},

	{(24), 100, 30} // 6:10..24 min .....
};

