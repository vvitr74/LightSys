/*
 * smbus.h
 *
 *  Created on: 17 Mart 2016 Y.
 *      Author: MVV
 */
//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//
//
#ifndef SMBUS_H_
#define SMBUS_H_
//
//
#include "stm32f3xx_hal.h"
//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//

#define   SMBUS_DEBUG_PERIOD							1000

#define   SMBUS_DEVISE_ADDR_WR							0x16
#define   SMBUS_DEVISE_ADDR_RD							0x17

#define   SMBUS_TOTAL_SBS_CMD							3

#define   SMBUS_Volt									0
#define   SMBUS_Amp										1
#define   SMBUS_RemCap									2

#define   SMBUS_SBS_Cmd_Voltage							0x09
#define   SMBUS_SBS_Cmd_Current							0x0A
#define   SMBUS_SBS_Cmd_RemainingCapacity				0x0F

#define   SMBUS_SBS_Cmd_PERIOD_Voltage					1000
#define   SMBUS_SBS_Cmd_PERIOD_Current					1000
#define   SMBUS_SBS_Cmd_PERIOD_RemainingCapacity		1000

#define   OP_RD											0
#define   OP_WR											1

#define	  DT_INT16                            			0
#define   DT_UINT16                           			1

void MX_I2C1_SMBUS_Init(void);
int SMBUS_task_cycle(void);
int32_t SMBUS_getValue(int32_t indx);
uint32_t SMBUS_getCntErr(int32_t indx);

#endif /* SMBUS_H_ */
