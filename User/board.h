#ifndef _BOARD_H_
#define _BOARD_H_

#include "stm32f10x.h"	

#define ClearDog   IWDG_ReloadCounter();



#define LED_Blink   GPIO_WriteBit(GPIOB, GPIO_Pin_5, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5)));

#define BUZZ_ON    GPIO_SetBits(GPIOB, GPIO_Pin_0);	
#define BUZZ_OFF   GPIO_ResetBits(GPIOB, GPIO_Pin_0);	



#endif