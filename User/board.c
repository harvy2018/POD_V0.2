

#include "stm32f10x.h"	



void Get_SerialNum(unsigned int* SerialID)
{
    SerialID[0] = *(unsigned int*)(0x1FFFF7E8);
    SerialID[1] = *(unsigned int*)(0x1FFFF7F0);	
    SerialID[2] = *(unsigned int*)(0x1FFFF7EC);

    SerialID[3] = 0; //–≠“È‘§¡Ù
}



void Watchdog_Init(void)
{
	
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: 40KHz(LSI) / 128 = 0.3125 KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_256);

  /* Set counter reload value to 4095 */  //4096/312.5=13.1s
  IWDG_SetReload(0xFFF);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
	
}

