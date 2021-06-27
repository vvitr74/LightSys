//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
#include <string.h>

#include "mb.h"
#include "mbport.h"
#include "mbutils.h" 
#include "Stm32f3xx.h"

//#include "protocol.h"

#define MB_COIL_START        1
#define MB_DISCRETE_START    1
#define MB_INPUT_REG_START   1
#define MB_HOLDING_REG_START 1

#define MB_COIL_NUM          0      // Should be a multiple of 8 for byte array
#define MB_DISCRETE_NUM      96     // Should be a multiple of 8 for byte array
#define MB_INPUT_REG_NUM     64
#define MB_HOLDING_REG_NUM   176

#define MB_BITS_UCHAR        8U


#define REG_HOLDING_START   1000
#define REG_HOLDING_NREGS   5

#define FLASH_START_ADDR		0x0800F000

static UCHAR mbDiscreteInputBuf[MB_DISCRETE_NUM / 8]; // 96 / 8 = 12
static USHORT mbInputRegBuf[MB_INPUT_REG_NUM];
static USHORT mbHoldingRegBuf[MB_HOLDING_REG_NUM];

//extern telemetry_t telemetry_R;
//extern sysInfo_t sysInfo_R;

void flashWrite(uint32_t addr, uint16_t len);

// ----------------------------------------------------------------------------
//void MB_CopyStructToDiscreteInputs(void)
//{
//   // Copy struct data to single-bit Discrete Inputs (read-only) ...
//   int size = sizeof(telemetry_R.eventFlags);
//   memcpy(&mbDiscreteInputBuf[0], &(telemetry_R.eventFlags.flags), size);
//   memcpy(&mbDiscreteInputBuf[size], &(telemetry_R.status.status), sizeof(telemetry_R.status));
//}
// ----------------------------------------------------------------------------
//void MB_CopyStructToInputRegs(void)
//{
//    memcpy(&mbInputRegBuf[0], &telemetry_R.bytes[0], sizeof(telemetry_R));
//    memcpy(&mbInputRegBuf[40], &sysInfo_R.bytes[0], sizeof(sysInfo_R));
//}

void flashWrite(uint32_t addr, uint16_t len)
{
	FLASH_EraseInitTypeDef flasher;
	uint32_t pgErr;
//	extern volatile UCHAR  ucRTUBuf[MB_SER_PDU_SIZE_MAX];
//	uint16_t ucRTUBufHalfWrd[MB_SER_PDU_SIZE_MAX/2];
//	uint16_t bufInd=0;
	uint32_t temp=addr;
	
	flasher.NbPages=1;
	flasher.PageAddress=addr;
	flasher.TypeErase=FLASH_TYPEERASE_PAGES;

	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&flasher,&pgErr);
	FLASH_WaitForLastOperation(1000);
//	for(int i=0;i<sizeof(ucRTUBuf);i+=2){
//		ucRTUBufHalfWrd[bufInd]=(ucRTUBuf[i+1]<<8) | ucRTUBuf[i];
//		bufInd++;
//	}
	for(int i=0;i<len;i++){
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,temp,(uint64_t)mbHoldingRegBuf[i]);
		FLASH_WaitForLastOperation(1000);
		temp+=2;
	}
	HAL_FLASH_Lock();
}


eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress,
                            USHORT usNCoils, eMBRegisterMode eMode )
{
    return MB_ENOERR;
}

eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
     eMBErrorCode eStatus = MB_ENOERR;
   
   int index = 0;
   index = (int)(usAddress - MB_DISCRETE_START); // Index number in discrete inputs
   
   if((index >= 0) && (index <= MB_DISCRETE_NUM - usNDiscrete))
   {
       while(usNDiscrete > 0)
       {
          if (usNDiscrete >= 8)
          {
             *pucRegBuffer++ = xMBUtilGetBits(mbDiscreteInputBuf, index, 8);
             index += 8;
             usNDiscrete -= 8;
          }
          else
          {
             *pucRegBuffer++ = xMBUtilGetBits(mbDiscreteInputBuf, index, usNDiscrete);
             index += usNDiscrete;
             usNDiscrete -= usNDiscrete;
          }
       }
   }
   else
   {
      eStatus = MB_ENOREG;
   }
   return eStatus;
}

eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode eStatus = MB_ENOERR;
    
    int index = 0;
    index = (int)(usAddress - MB_INPUT_REG_START);

    if((index >= 0) && (index <= MB_INPUT_REG_NUM - usNRegs))
    {
       while(usNRegs > 0)
       {
          *pucRegBuffer++ = (unsigned char)(mbInputRegBuf[index] >> 8);
          *pucRegBuffer++ = (unsigned char)(mbInputRegBuf[index] & 0xFF);
          index++;
          usNRegs--;
       }
   }
   else
   {
      eStatus = MB_ENOREG;
   }
   return eStatus;
}


eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress,
                              USHORT usNRegs, eMBRegisterMode eMode )
{
	
	eMBErrorCode eStatus = MB_ENOERR;
	
	uint16_t regs=usNRegs;
	
	int index=0;
	index = (int)(usAddress - MB_HOLDING_REG_START);
	
	if((index >= 0) && (index <= MB_HOLDING_REG_NUM - usNRegs)){
		while(regs>0){
			mbHoldingRegBuf[index]=*pucRegBuffer++;
			mbHoldingRegBuf[index]=mbHoldingRegBuf[index]<<8;
			mbHoldingRegBuf[index]|=*pucRegBuffer++;
			index++;
      regs--;
		}
		//todo stop DC/DC
		flashWrite(FLASH_START_ADDR, usNRegs);
		//todo reset MCU
	}
	else
   {
      eStatus = MB_ENOREG;
   }
  return eStatus;
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
