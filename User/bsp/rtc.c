
#include <stdio.h>	
#include <time.h>
#include "rtc.h"

#include "dev_state.h"	


extern u32 unixtime;
extern u8 Rx1Buffer[1024];
extern u8  Command_ID;
extern u8  FunctionNum;
//u8 Data_Buf[50];//数据缓冲
u8 Com_len;//


void RTC_Configuration(void)
{

	/* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();

  /* Enable LSE */
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {}

  /* Select LSE as RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC registers synchronization */
  RTC_WaitForSynchro();

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();


  /* Set RTC prescaler: set RTC period to 1sec */
  RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
		
	setRTCState(RTC_STATE_INIT);
	
//	RTC_SetCounter(unixtime);
}


void RTC_Config(void)
{
	if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
	  
		RTC_Configuration();
		// rtc_flag=0;
		Time_Adjust();
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
		
	}	
	else
	{
	   // rtc_flag=1;		
		RTC_Configuration();
		// rtc_flag=0;
		Time_Adjust();
		
		RTC_WaitForSynchro();
		/* Enable the RTC Second */
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
	}

	

}

void Time_Display()
{
  
   struct tm *ptim; 
	//extern time_t TIME;
   // u8 hour1,hour2,minute1,minute2,sec1,sec2;
	
	ptim = localtime(&unixtime); 
 	ptim->tm_hour+=8;
	ptim->tm_hour=ptim->tm_hour%24;


//	printf("Date: %0.4d:%0.2d:%0.2d  ", ptim->tm_year+1900, ptim->tm_mon+1,ptim->tm_mday);	
//  printf("Time: %0.2d:%0.2d:%0.2d\r\n", ptim->tm_hour, ptim->tm_min,ptim->tm_sec);
	
	 //向上发送数据
//				 Uart1SendData(0x11);  //头
//	       Uart1SendData(0x66);
//				 Uart1SendData(0x09);  //命令号
//				 Uart1SendData(0x01);  //功能号
//				 Uart1SendData(0x04);  //长度
//				 Uart1SendData(unixtime>>24);  //数据
//				 Uart1SendData(unixtime>>16); 
//				 Uart1SendData(unixtime>>8); 
//				 Uart1SendData(unixtime); 
//				 Uart1SendData(0x22);  //
	
}


void Time_Adjust(void)
{
  /* Wait until last write operation on RTC registers has finished */
 // RTC_WaitForLastTask();
  /* Change the current time */
  RTC_SetCounter(unixtime);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
	
	  /* Enable the RTC Second */
  RTC_ITConfig(RTC_IT_SEC, ENABLE);

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

}












