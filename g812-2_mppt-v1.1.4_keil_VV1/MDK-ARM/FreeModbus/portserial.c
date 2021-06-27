/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */


#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "Stm32f3xx.h"
#include "usart.h"
#include "mbrtu.h"

#define USART1_PCLK 			(36000000UL)

unsigned long Time_Cycle = 0;   



/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    if( xRxEnable )
    {
			USART1->CR1 |= USART_CR1_RXNEIE;
    }
    else
    {
			USART1->CR1 &= ~USART_CR1_RXNEIE;			
    }

    if ( xTxEnable )
    {
			USART1->CR1 |= USART_CR1_TXEIE;
    }
    else
    {
			USART1->CR1 &= ~USART_CR1_TXEIE;			
    }
}


//void txd_RS485_EN(void)
//{
//	GPIOC->BSRR = GPIO_BSRR_BS_12;	
//}

//void txd_RS485_DIS(void)
//{
//	GPIOC->BSRR = GPIO_BSRR_BR_12;	
//}

//void check_txd_RS485(void)
//{
//    if ( USART3->ISR & USART_ISR_TC_Msk )
//    {
//			txd_RS485_DIS();
//    }
//    
//    MB_CopyStructToDiscreteInputs();
//    MB_CopyStructToInputRegs();
//}
	
//void flashWrite(uint32_t addr)
//{
//	FLASH_EraseInitTypeDef flasher;
//	uint32_t pgErr;
//	extern volatile UCHAR  ucRTUBuf[MB_SER_PDU_SIZE_MAX];
//	uint16_t ucRTUBufHalfWrd[MB_SER_PDU_SIZE_MAX/2];
//	uint16_t bufInd=0;
//	uint32_t temp=addr;
//	
//	flasher.NbPages=1;
//	flasher.PageAddress=addr;
//	flasher.TypeErase=FLASH_TYPEERASE_PAGES;

//	HAL_FLASH_Unlock();
//	HAL_FLASHEx_Erase(&flasher,&pgErr);
//	FLASH_WaitForLastOperation(1000);
//	for(int i=0;i<sizeof(ucRTUBuf);i+=2){
//		ucRTUBufHalfWrd[bufInd]=(ucRTUBuf[i+1]<<8) | ucRTUBuf[i];
//		bufInd++;
//	}
//	for(int i=0;i<sizeof(ucRTUBufHalfWrd);i++){
//		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,temp,(uint64_t)ucRTUBufHalfWrd[i]);
//		FLASH_WaitForLastOperation(1000);
//		temp+=2;
//	}
//	HAL_FLASH_Lock();
//}

BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	MX_USART1_UART_Init();
	
	NVIC_SetPriority( USART1_IRQn, 1 );
	NVIC_EnableIRQ( USART1_IRQn );
//	USART1->CR1 |= USART_CR1_UE;

	return TRUE;	
}

BOOL xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
	  USART1->TDR = ucByte;
    return TRUE;
}

BOOL xMBPortSerialGetByte( CHAR *pucByte )
{
CHAR data;	
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    
	data = (CHAR)USART1->RDR; 
	*pucByte = data;
  return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived();
}


void USART1_IRQHandler(void)
{
  	if ( USART1->ISR & USART_ISR_RXNE )
		{
				USART1->ICR |= USART_ICR_ORECF;
        pxMBFrameCBByteReceived();			
				USART1->RQR |= USART_RQR_RXFRQ;
		}
		
		if ( USART1->ISR & USART_ISR_TXE )
		{
			USART1->RQR |= USART_RQR_TXFRQ;		
      pxMBFrameCBTransmitterEmpty();			
		}
}



#ifdef MODBUS
void USART1_IRQHandler(void)
{
static unsigned char dummy[8];
static unsigned long value = 0;

		if ( (USART1->ISR & USART_ISR_TXE_TXFNF) && (USART1->CR1 & USART_CR1_TXEIE_TXFNFIE) )
		{
			USART1->ICR |= USART_ICR_TXFECF;             
			USART1->RQR |= USART_RQR_TXFRQ;
//			USART1->TDR = value;
//			value++;						
//			if ( value >= 5 )	
//			{
//				USART1->CR1 |= USART_CR1_RXNEIE_RXFNEIE;				
//				USART1->CR1 &= ~USART_CR1_TXEIE_TXFNFIE;
//				value = 0;
//			}
      pxMBFrameCBTransmitterEmpty();			
			return;
		}	
	
	if( (USART1->ISR & USART_ISR_RXNE_RXFNE) && (USART1->CR1 & USART_CR1_RXNEIE_RXFNEIE) )
	{
		USART1->ICR |= USART_ICR_ORECF;
		USART1->RQR |= USART_RQR_RXFRQ;
		pxMBFrameCBByteReceived();		
//		dummy[value] = USART1->RDR;
		value++;
		if ( value >= 8 )
		{
			value = 0;
			
//			USART1->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;				
//			USART1->CR1 |= USART_CR1_TXEIE_TXFNFIE;
			
//		dummy[0] = dummy[1] = dummy[2] = dummy[3] = dummy[4] = dummy[5] = dummy[6] = dummy[7] = 0;		
		}
	}
}
#endif
