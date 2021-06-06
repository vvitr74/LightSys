/*
 * smbus.c
 *
 *  Created on: 20 Mart 2016 Y.
 *      Author: Pluton
 */
//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
#include <string.h>
//
#include "gpio.h"
#include "smbus.h"
//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//

#define I2C1_CLOCK_Pin 			GPIO_PIN_6
#define I2C1_CLOCK_GPIO_Port 	GPIOB
#define I2C1_DATA_Pin 			GPIO_PIN_7
#define I2C1_DATA_GPIO_Port 	GPIOB

typedef struct {
	uint8_t SBS_cmd;
	uint8_t op;
	uint8_t dt_type;
	uint8_t lock;
	void *pVar;
	uint32_t period;
	uint32_t time_stop;
	uint32_t err_cnt;
} t_SBS_Commands;

typedef enum{
	SMBUS_CYCLE_IDLE,
	SMBUS_CYCLE_INIT_READ,
	SMBUS_CYCLE_WAIT_STEP1,
	SMBUS_CYCLE_WAIT_STEP2,
	SMBUS_CYCLE_ERROR
} e_smbus_cycle_state;

union dat_conv
{
    int8_t sc[4];
    uint8_t uc[4];
    uint16_t us[2];
    int16_t ss[2];
    uint32_t ul;
    int32_t l4;
    float f;
};

const uint8_t table_crc8[] = {
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
	0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
	0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
	0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
	0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
	0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
	0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
	0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
	0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
	0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
	0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
	0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
	0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
	0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
	0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
	0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
	0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};


SMBUS_HandleTypeDef hsmbus1;
t_SBS_Commands SBS_Commands[SMBUS_TOTAL_SBS_CMD];

unsigned short Voltage = 0;
signed short Current = 0;
unsigned short RemainingCapacity = 0;
uint8_t smbusBufRX[16];

int SMBUS_data_init(void);

void MX_I2C1_SMBUS_Init(void)
{
  hsmbus1.Instance = I2C1;
 // hsmbus1.Init.Timing = 0x10808DD3;  // 100 kHz
  hsmbus1.Init.Timing = 0x00101D7C;    // 50 kHz
  hsmbus1.Init.AnalogFilter = SMBUS_ANALOGFILTER_ENABLED;
  hsmbus1.Init.OwnAddress1 = 0;
  hsmbus1.Init.AddressingMode = SMBUS_ADDRESSINGMODE_7BIT;
  hsmbus1.Init.DualAddressMode = SMBUS_DUALADDRESS_DISABLED;
  hsmbus1.Init.OwnAddress2 = 0;
  hsmbus1.Init.OwnAddress2Masks = SMBUS_OA2_NOMASK;
  hsmbus1.Init.GeneralCallMode = SMBUS_GENERALCALL_DISABLED;
  hsmbus1.Init.NoStretchMode = SMBUS_NOSTRETCH_DISABLED;
  hsmbus1.Init.PacketErrorCheckMode = SMBUS_PEC_ENABLED;
  hsmbus1.Init.PeripheralMode = SMBUS_PERIPHERAL_MODE_SMBUS_HOST;
  hsmbus1.Init.SMBusTimeout = 0x0000836E;
  HAL_SMBUS_Init(&hsmbus1);
  SMBUS_data_init();

}

void HAL_SMBUS_MspInit(SMBUS_HandleTypeDef* hsmbus)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hsmbus->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = I2C1_CLOCK_Pin|I2C1_DATA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __I2C1_CLK_ENABLE();

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

uint8_t calc_crc8( uint8_t *data, uint16_t len )
{
uint8_t crc = 0;
uint16_t i;
    for ( i=0; i<len; ++i ) {
        crc = table_crc8[crc ^ *data++];
    }
	return crc;
}

int32_t add_SBS_Commands(uint8_t index, uint8_t SBS_cmd, void *pVar, uint8_t dt_type, uint32_t period)
{
t_SBS_Commands 	*p;
	if ( index >= SMBUS_TOTAL_SBS_CMD )	return -1;
	if ( pVar == NULL )	return -2;
	p = (t_SBS_Commands *)&SBS_Commands[index];
	p->SBS_cmd = SBS_cmd;
	p->op = OP_RD;
	p->period = period;
	p->time_stop = 0;
	p->pVar = pVar;
	p->dt_type	= dt_type;
	p->lock = 0;
	p->time_stop = HAL_GetTick();
	p->err_cnt = 0;
	return 0;
}

int SMBUS_data_init(void)
{
int rez;

	rez = add_SBS_Commands(SMBUS_Volt, SMBUS_SBS_Cmd_Voltage, &Voltage, DT_UINT16, SMBUS_SBS_Cmd_PERIOD_Voltage);
	if ( rez == -1 )	return -1;
	rez = add_SBS_Commands(SMBUS_Amp, SMBUS_SBS_Cmd_Current, &Current, DT_INT16,  SMBUS_SBS_Cmd_PERIOD_Current);
	if ( rez == -1 )	return -1;
	rez = add_SBS_Commands(SMBUS_RemCap, SMBUS_SBS_Cmd_RemainingCapacity, &RemainingCapacity, DT_UINT16,
							SMBUS_SBS_Cmd_PERIOD_RemainingCapacity);
	if ( rez == -1 )	return -1;
	return 0;
}

int SMBUS_task_cycle(void)
{
static int	index_cmd = SMBUS_TOTAL_SBS_CMD;
static e_smbus_cycle_state	smbus_cycle_state = SMBUS_CYCLE_IDLE;
uint32_t time;
t_SBS_Commands 	*p;
unsigned char crc8calc, crc8rd ;
uint8_t bufferframe[16];
unsigned short *u16;
signed short *s16;
union dat_conv dc;

	switch ( smbus_cycle_state )
	{
		case SMBUS_CYCLE_IDLE:
			index_cmd++;
			if ( index_cmd >= SMBUS_TOTAL_SBS_CMD )
				index_cmd = 0;
			p = (t_SBS_Commands *)&SBS_Commands[index_cmd];
			if ( p->lock == 1 )
				break;
			time = HAL_GetTick();
			if ( time - p->time_stop < p->period )
				break;
			p->time_stop = time;
			smbus_cycle_state = SMBUS_CYCLE_INIT_READ;
			break;
		case SMBUS_CYCLE_INIT_READ:
			memset(smbusBufRX, 0, sizeof(smbusBufRX));
			p = (t_SBS_Commands *)&SBS_Commands[index_cmd];
			if (HAL_SMBUS_Master_Transmit_IT(&hsmbus1, SMBUS_DEVISE_ADDR_WR, &p->SBS_cmd, 1, SMBUS_FIRST_FRAME) != HAL_OK)
			{
				smbus_cycle_state = SMBUS_CYCLE_ERROR;
				break;
			}
			else
			smbus_cycle_state = SMBUS_CYCLE_WAIT_STEP1;
//			break;
		case SMBUS_CYCLE_WAIT_STEP1:
//			if ( HAL_SMBUS_GetState(&hsmbus1) != HAL_SMBUS_STATE_READY )
//					break;
			while ( HAL_SMBUS_GetState(&hsmbus1) != HAL_SMBUS_STATE_READY );

			if (HAL_SMBUS_Master_Receive_IT(&hsmbus1, SMBUS_DEVISE_ADDR_RD, &smbusBufRX[0], 3, SMBUS_LAST_FRAME_WITH_PEC) != HAL_OK)
			{
				smbus_cycle_state = SMBUS_CYCLE_ERROR;
				break;
			}
			else
			smbus_cycle_state = SMBUS_CYCLE_WAIT_STEP2;
//			break;
		case SMBUS_CYCLE_WAIT_STEP2:
//			if ( HAL_SMBUS_GetState(&hsmbus1) != HAL_SMBUS_STATE_READY )
//				break;
			while ( HAL_SMBUS_GetState(&hsmbus1) != HAL_SMBUS_STATE_READY );

			p = (t_SBS_Commands *)&SBS_Commands[index_cmd];
			bufferframe[0] = SMBUS_DEVISE_ADDR_WR;
			bufferframe[1] = p->SBS_cmd;
			bufferframe[2] = SMBUS_DEVISE_ADDR_RD;
			bufferframe[3] = smbusBufRX[0];
			bufferframe[4] = smbusBufRX[1];
			crc8rd = smbusBufRX[2];
			crc8calc = calc_crc8(bufferframe, 5);
			if ( crc8rd != crc8calc )
			{
				smbus_cycle_state = SMBUS_CYCLE_ERROR;
				break;
			}
			p->err_cnt = 0;
			dc.uc[0] = smbusBufRX[0];
			dc.uc[1] = smbusBufRX[1];
			switch ( p->dt_type )
			{
				case DT_INT16:
					s16 = (signed short *)p->pVar;
					*s16 = dc.ss[0];
					break;
				case DT_UINT16:
					u16 = (unsigned short *)p->pVar;
					*u16 = dc.us[0];
					break;
			}
			smbus_cycle_state = SMBUS_CYCLE_IDLE;
			break;
		case SMBUS_CYCLE_ERROR:
			p = (t_SBS_Commands *)&SBS_Commands[index_cmd];
			p->err_cnt++;
			smbus_cycle_state = SMBUS_CYCLE_IDLE;
			break;
	}
	return 0;
}

int32_t SMBUS_getValue(int32_t indx)
{
t_SBS_Commands 	*p;
union dat_conv dc;
int16_t *ptr_s16;
uint16_t *ptr_us16;

	if ( indx >= SMBUS_TOTAL_SBS_CMD )	return 0xffffffff;
	p = (t_SBS_Commands *)&SBS_Commands[indx];

	switch( p->dt_type )
	{
		case DT_INT16:
			ptr_s16 = (int16_t *)p->pVar;
			dc.l4 = *ptr_s16;
			break;
		case DT_UINT16:
			ptr_us16 = (uint16_t *)p->pVar;
			dc.ul = *ptr_us16;
			break;
	}
	return dc.l4;
}

uint32_t SMBUS_getCntErr(int32_t indx)
{
t_SBS_Commands 	*p;
	if ( indx >= SMBUS_TOTAL_SBS_CMD )	return 0xffffffff;
	p = (t_SBS_Commands *)&SBS_Commands[indx];
	return p->err_cnt;
}
