/*
 * config.c
 *
 *  Created on: 16 марта 2016
 *      Author: Viacheslav Azarov
 *      Configuration constants of algorithms
 ******************************************************************************
 *  This software developed by engineer Viacheslav Azarov with free tools
 *  from STMicroelectronics on personal wish employer Danil Rogalev and saves
 *  all rights all authors and contributors.
 ******************************************************************************
 */
#include "config.h"

const Ceremonial_Struct	Ceremonial[LIGHTING_CEREMONIAL_STEPS] =
	{
		{CEREMONIAL_TIME(4), 100, 100}, // 0..4 min Alight=100% Dim=100%
		{CEREMONIAL_TIME(1000 + 4), 100, 96},
		{CEREMONIAL_TIME(2000 + 4), 100, 92},
		{CEREMONIAL_TIME(3000 + 4), 100, 88},
		{CEREMONIAL_TIME(4000 + 4), 100, 84},
		{CEREMONIAL_TIME(5000 + 4), 100, 80},
		{CEREMONIAL_TIME(6000 + 4), 100, 76},
		{CEREMONIAL_TIME(7000 + 4), 100, 72},
		{CEREMONIAL_TIME(8000 + 4), 100, 68},
		{CEREMONIAL_TIME(9000 + 4), 100, 64},
		{CEREMONIAL_TIME(10000 + 4), 100, 60},

		{CEREMONIAL_TIME(6), 100, 60},  // 4:10..6 min Alight=100% Dim=90%
		{CEREMONIAL_TIME(1000 + 6), 100, 57},
		{CEREMONIAL_TIME(2000 + 6), 100, 54},
		{CEREMONIAL_TIME(3000 + 6), 100, 51},
		{CEREMONIAL_TIME(4000 + 6), 100, 48},
		{CEREMONIAL_TIME(5000 + 6), 100, 45},
		{CEREMONIAL_TIME(6000 + 6), 100, 42},
		{CEREMONIAL_TIME(7000 + 6), 100, 39},
		{CEREMONIAL_TIME(8000 + 6), 100, 36},
		{CEREMONIAL_TIME(9000 + 6), 100, 33},
		{CEREMONIAL_TIME(10000 + 6), 100, 30},

		{CEREMONIAL_TIME(24), 100, 30} // 6:10..24 min .....
	};
// For more time, default values used from config.h
