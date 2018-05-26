// This fine define TaxControlDev communication interface and structure */

/**************************************************************************
File name: uartPort.h
Date:        2016-8-24
Author:     Wangwei
***************************************************************************/
#ifndef _UART_PORT_H
#define _UART_PORT_H

#include "stm32f10x.h"	
#include <stdbool.h>

#include "errorCode.h"

#define	UART_TOTAL_PORT_NUM	   5
#define	UART_TAX_PORT_NUM	   2
#define	UART_UPSTREAM_NO		 4


#define	FK_RCV_BUFFER_SIZE      1024
#define	FK_SND_BUFFER_SIZE      256

#define	COMMON_RCV_BUFFER_SIZE      256
#define	COMMON_SND_BUFFER_SIZE      128

#define	SBUS_RCV_BUFFER_SIZE      256
#define	SBUS_SND_BUFFER_SIZE      128

typedef enum _ConnectionState
{
	CONNECTION_STATE_INIT,
	CONNECTION_STATE_CONNECT,
	CONNECTION_STATE_MAX
}ConnectionState;


typedef enum _UartBindState
{
	PORT_UNBOUND=0,
	PORT_BOUNDED,
	PORT_BIND_MAX	
}UartBindState;

typedef enum _UartPortNum
{
	UART_FK=0,  
	UART_SBUS,	
    UART_GIMBAL,		// 
	UART_Camera,		
    UART_Idle,
	UART_PORT_MAX	
}UartPortNum;

//typedef enum _UartPortNum
//{
//	UART_TAX_PORT_0 = 0,		// Tax Control Dev1
//	UART_TAX_PORT_1,		// Tax Control Dev2		
//	UART_TAX_PORT_2,		// Tax Control Dev3
//	UART_TAX_PORT_3,		// Tax Control Dev4
//	UART_TAX_PORT_4,		// Up streamer Port for PLC/485
//	UART_PORT_MAX	
//}UartPortNum;




typedef struct _UartPortState
{  
    ConnectionState 	cState;
    UartBindState		bState;
    
}UartPortState;

typedef struct _UartRBuffer		//UART receive buffer structure
{
    unsigned short  size;			// size of the buffer
    unsigned short	wptr;			// write pointer, ISR increment by 1 once got a byte
    unsigned short	rptr;			// read pointer, application increment by 1 once fetch one byte
    unsigned char   *data;    		// data buffer pointer
    unsigned char   overFlow;    	// once "wptr+1 == rptr", set this flag, means the buffer overflow
    unsigned char   idleFlag;  
	
}UartRBuffer; 

typedef struct _UartSBuffer		//UART send buffe structure
{
    unsigned short	size;				// size of the buffer
    unsigned short  wptr;				// application increment by 1 once put a byte into buffer    
    unsigned short  rptr;			// send function or ISR increment by one once sent a byte out
    unsigned char	*data;      		// data buffer pointer
    
}UartSBuffer;


// Define the API for UART access(receive/send/control/misc)
errorCode uartBufferInitialize();
unsigned short  uartGetAvailBufferedDataNum(UartPortNum port);
errorCode	uartRead(UartPortNum port, unsigned char *buf, unsigned short len);
errorCode uartWriteBuffer(UartPortNum port,unsigned char *buf,unsigned short len);
errorCode uartSendBufferOut(UartPortNum port);
errorCode uartConnectionDetect();
UartPortState *uartGetPortState();
errorCode uartSetPortState(UartPortState *pUartPortState);
errorCode uartPortBufferReset(UartPortNum port);
void uartPutCharIntoRcvBuffer(unsigned char  port, char c);

extern UartRBuffer	uartRBuffer[UART_TOTAL_PORT_NUM];

u8 GetUartRxIdleFlag(UartPortNum port);
void ClearUartRxIdleFlag(UartPortNum port);
void uartClearRcvBuffer(UartPortNum port);

#endif

