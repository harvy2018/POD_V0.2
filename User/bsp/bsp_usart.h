/*
*********************************************************************************************************
*	                                  
*	ģ������ : ��������ģ��    
*	�ļ����� : bsp_usart.h
*	��    �� : V2.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_USART_H
#define __BSP_USART_H
#include "stm32f10x.h"


//#define RXBUF_LEN  1024
#define PLCBUF_LEN  2048

void bsp_InitUart(void);
void bsp_InitUart2(void);
void bsp_InitUart3(void);
void bsp_InitUart4(void);
void bsp_InitUart5(void);

void Uart1SendData(uint8_t data);
void Uart2SendData(uint8_t data);
void Uart3SendData(uint8_t data);
void Uart4SendData(uint8_t data);
void Uart4SendData(uint8_t data);
void Uart5SendData(uint8_t data);

void Uart1PrintBuf(uint8_t * Buf);
void Uart1SendBuf(uint8_t * Buf,uint16_t len);
void Uart2SendBuf(uint8_t * Buf,uint16_t len);
void Uart3SendBuf(uint8_t * Buf,uint16_t len);
void Uart4SendBuf(uint8_t * Buf,uint16_t len);
void Uart5SendBuf(uint8_t * Buf,uint16_t len);
#endif


