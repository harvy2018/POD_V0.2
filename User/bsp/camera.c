#include "stm32f10x.h"	
#include "bsp_timer.h"


void Record_D(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//;GPIO_Mode_Out_PP
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); /* PB14 FLASH WP, PB15 RS485 DIR*/
	GPIO_ResetBits(GPIOB, GPIO_Pin_9);

}

void Record_P(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//;GPIO_Mode_Out_PP
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); /* PB14 FLASH WP, PB15 RS485 DIR*/
	GPIO_SetBits(GPIOB, GPIO_Pin_9);
}

void Picture_D(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//;GPIO_Mode_Out_PP
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); /* PB14 FLASH WP, PB15 RS485 DIR*/
	GPIO_ResetBits(GPIOB, GPIO_Pin_8);

}

void Picture_P(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//;GPIO_Mode_Out_PP
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); /* PB14 FLASH WP, PB15 RS485 DIR*/
	GPIO_SetBits(GPIOB, GPIO_Pin_8);
}

void Zoom(u16 p)
{
	PWM3_SET(p);
	
}


