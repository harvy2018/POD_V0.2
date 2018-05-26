/*
*********************************************************************************************************
*	                                  
*	ģ������ : �ж�ģ��
*	�ļ����� : stm32f10x_it.c
*	��    �� : V1.0
*	˵    �� : ���ļ�������е��жϷ�������Ϊ�˱��������˽�����õ����жϣ����ǲ����齫�жϺ����Ƶ�����
*			���ļ���
*			
*			����ֻ��Ҫ�����Ҫ���жϺ������ɡ�һ���жϺ������ǹ̶��ģ��������޸��������ļ���
*				Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm\startup_stm32f10x_hd.s
*			
*			�����ļ��ǻ�������ļ�������ÿ���жϵķ���������Щ����ʹ����WEAK �ؼ��֣���ʾ�����壬�����
*			��������c�ļ����ض����˸÷��������������ͬ��������ô�����ļ����жϺ������Զ���Ч����Ҳ��
*			�����ض���ĸ�����C++�еĺ������ص��������ơ�
*				
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v0.1    2009-12-27 armfly  �������ļ���ST�̼���汾ΪV3.1.2
*		v1.0    2011-01-11 armfly  ST�̼���������V3.4.0�汾��
*		v2.0    2011-10-16 armfly  ST�̼���������V3.5.0�汾��
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "stm32f10x_it.h"
#include "bsp_timer.h"
#include "bsp_usart.h"

#include "uartPort.h"	
#include "dev_state.h"	
/*
*********************************************************************************************************
*	Cortex-M3 �ں��쳣�жϷ������
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	�� �� ��: NMI_Handler
*	����˵��: ���������жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/  
void NMI_Handler(void)
{
}

/*
*********************************************************************************************************
*	�� �� ��: HardFault_Handler
*	����˵��: Ӳ��ʧЧ�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/ 
void HardFault_Handler(void)
{
  /* ��Ӳ��ʧЧ�쳣����ʱ������ѭ�� */
	while (1)
  {
		 //SEGGER_RTT_printf(0,"HardFault!!!\r\n");
  }
}

void hard_fault_handler_c(unsigned int * hardfault_args)
{
	static unsigned int stacked_r0;
	static unsigned int stacked_r1;
	static unsigned int stacked_r2;
	static unsigned int stacked_r3;
	static unsigned int stacked_r12;
	static unsigned int stacked_lr;
	static unsigned int stacked_pc;
	static unsigned int stacked_psr;
	static unsigned int SHCSR;
	static unsigned char MFSR;
	static unsigned char BFSR;	
	static unsigned short int UFSR;
	static unsigned int HFSR;
	static unsigned int DFSR;
	static unsigned int MMAR;
	static unsigned int BFAR;

	stacked_r0 = ((unsigned long) hardfault_args[0]);
	stacked_r1 = ((unsigned long) hardfault_args[1]);
	stacked_r2 = ((unsigned long) hardfault_args[2]);
	stacked_r3 = ((unsigned long) hardfault_args[3]);
	stacked_r12 = ((unsigned long) hardfault_args[4]);
	/*�쳣�жϷ���ʱ������쳣ģʽ�ض�������R14,��lr�����óɸ��쳣ģʽ��Ҫ���صĵ�ַ*/
	stacked_lr = ((unsigned long) hardfault_args[5]); 	
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);
	
  SHCSR = (*((volatile unsigned long *)(0xE000ED24))); //ϵͳHandler���Ƽ�״̬�Ĵ���
	MFSR = (*((volatile unsigned char *)(0xE000ED28)));	//�洢������fault״̬�Ĵ���	
	BFSR = (*((volatile unsigned char *)(0xE000ED29)));	//����fault״̬�Ĵ���	
	UFSR = (*((volatile unsigned short int *)(0xE000ED2A)));//�÷�fault״̬�Ĵ���		
	HFSR = (*((volatile unsigned long *)(0xE000ED2C)));  //Ӳfault״̬�Ĵ���			
	DFSR = (*((volatile unsigned long *)(0xE000ED30)));	//����fault״̬�Ĵ���
	MMAR = (*((volatile unsigned long *)(0xE000ED34)));	//�洢�����ַ�Ĵ���
	BFAR = (*((volatile unsigned long *)(0xE000ED38))); //����fault��ַ�Ĵ���
	while (1);

}
/*
*********************************************************************************************************
*	�� �� ��: MemManage_Handler
*	����˵��: �ڴ�����쳣�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/   
void MemManage_Handler(void)
{
  /* ���ڴ�����쳣����ʱ������ѭ�� */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	�� �� ��: BusFault_Handler
*	����˵��: ���߷����쳣�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/    
void BusFault_Handler(void)
{
  /* �������쳣ʱ������ѭ�� */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	�� �� ��: UsageFault_Handler
*	����˵��: δ�����ָ���Ƿ�״̬�жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/   
void UsageFault_Handler(void)
{
  /* ���÷��쳣ʱ������ѭ�� */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	�� �� ��: SVC_Handler
*	����˵��: ͨ��SWIָ���ϵͳ��������жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/   
void SVC_Handler(void)
{
}

/*
*********************************************************************************************************
*	�� �� ��: DebugMon_Handler
*	����˵��: ���Լ������жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/   
void DebugMon_Handler(void)
{
}

/*
*********************************************************************************************************
*	�� �� ��: PendSV_Handler
*	����˵��: �ɹ����ϵͳ��������жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/     
void PendSV_Handler(void)
{
}

/*
*********************************************************************************************************
*	�� �� ��: SysTick_Handler
*	����˵��: ϵͳ��શ�ʱ���жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/     
extern void SysTick_ISR(void);	/* ���������ⲿ�ĺ��� */
extern u32 SYSTEMTICK;
void SysTick_Handler(void)
{
	SYSTEMTICK++;
	SysTick_ISR();	/* ���������bsp_timer.c�� */
}
/*
*********************************************************************************************************
*	STM32F10x�ڲ������жϷ������
*	�û��ڴ�����õ������жϷ���������Ч���жϷ���������ο������ļ�(startup_stm32f10x_xx.s)
*********************************************************************************************************
*/


extern vu32 unixtime;
void RTC_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    /* Clear the RTC Second interrupt */
    RTC_ClearITPendingBit(RTC_IT_SEC);

  //	unixtime_sendflag=1;
    /* Wait until last write operation on RTC registers has finished */
   
    /* Reset RTC Counter when Time is 23:59:59 */
		unixtime=RTC_GetCounter();
//    if (RTC_GetCounter() == 0x00015180)
//    {
//      RTC_SetCounter(0x0);
//      /* Wait until last write operation on RTC registers has finished */
//      RTC_WaitForLastTask();
//    }
//		Time_Display();
  }
}

extern UartRBuffer	uartRBuffer[UART_TOTAL_PORT_NUM];

void USART1_IRQHandler(void)
{
    u8 c;
    
   if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
    /* Read one byte from the receive data register */        
        c=USART_ReceiveData(USART1);
        uartPutCharIntoRcvBuffer(0,c);		
         SEGGER_RTT_printf(0,"%02X ",c);
    }

    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {              
        uartRBuffer[0].idleFlag=1;
        //��SR���DR���Idle
        c = USART1->SR;
        c = USART1->DR;      
        SEGGER_RTT_printf(0,"%\r\n");               
    }
	
}

void USART2_IRQHandler(void)//SBUS PORT
{
    u8 c;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
    /* Read one byte from the receive data register */        
        c=USART_ReceiveData(USART2);
        uartPutCharIntoRcvBuffer(1,c);
		//	  USART_SendData(USART2,c);
		//		  USART_SendData(USART1,c);
     
		
    }
    
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {       
         uartRBuffer[1].idleFlag=1;
        //��SR���DR���Idle
        c = USART2->SR;
        c = USART2->DR;       
        
    }
}

void USART3_IRQHandler(void)
{

     u8 c;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
    /* Read one byte from the receive data register */        
        c=USART_ReceiveData(USART3);
        uartPutCharIntoRcvBuffer(2,c);
			 
		
    }
    
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
    {       
         uartRBuffer[2].idleFlag=1;
        //��SR���DR���Idle
        c = USART3->SR;
        c = USART3->DR;                           
    }
}


void UART4_IRQHandler(void)
{
    u8 c;
    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
    /* Read one byte from the receive data register */        
        c=USART_ReceiveData(UART4);
        uartPutCharIntoRcvBuffer(3,c);
		
    }
    
    if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)
    {       
         uartRBuffer[3].idleFlag=1;
        //��SR���DR���Idle
        c = UART4->SR;
        c = UART4->DR;                           
    }
    
}


void UART5_IRQHandler(void)
{
    u8 c;
	
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
    { 	
         c=USART_ReceiveData(UART5);	 
         uartPutCharIntoRcvBuffer(4,c);						
    } 
		
	
    if(USART_GetITStatus(UART5, USART_IT_IDLE) != RESET)
    {       
            
        uartRBuffer[4].idleFlag=1;
        //��SR���DR���Idle
        c = UART5->SR;
        c = UART5->DR;    
        
        
    }	

  
}





void TIM2_IRQHandler(void) //������ʱ��ѯ��ʱ��
{
	
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{  
		   TIM_Cmd(TIM2,DISABLE);
//     DMA_Cmd(DMA1_Channel4, DISABLE);//�رշ��� 
//		 DMA_Cmd(DMA1_Channel2, DISABLE);
//		 DMA_Cmd(DMA1_Channel6, DISABLE);
//		 DMA_Cmd(DMA2_Channel5, DISABLE);
//		
//		 DMA_Cmd(DMA1_Channel5, DISABLE);//�رս��� 
//		 DMA_Cmd(DMA1_Channel6, DISABLE);
//		 DMA_Cmd(DMA1_Channel3, DISABLE);
//		 DMA_Cmd(DMA2_Channel3, DISABLE);
		
 //    LED_Blink;	
	   TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		 
  //    GasDataQuery_TimeOut=1;//��ѯ��ʱ
	
    // CB_PORT[3].GasDataRes_Flag=1;
		// CB_PORT[1].GasDataRes_Flag=1;
	}
}

/****
void TIM3_IRQHandler(void)//PLC���ճ�ʱ��ʱ��
{
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
    
     PlcData_Get_Flag=1;//PLC�����ݽ���
	
	   TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
   

	}
}
*/
/*
*********************************************************************************************************
*	�� �� ��: PPP_IRQHandler
*	����˵��: �����жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/    
/* 
	��Ϊ�жϷ�����������;����Ӧ���йأ����õ��û�����ģ��ı���������������ڱ��ļ�չ���������Ӵ�����
	�ⲿ������������include��䡣
	
	��ˣ������Ƽ�����ط�ֻдһ��������䣬�жϷ������ı���ŵ���Ӧ���û�����ģ���С�
	����һ����ûή�ʹ����ִ��Ч�ʣ�����������Ը��ʧ���Ч�ʣ��Ӷ���ǿ�����ģ�黯���ԡ�
	
	����extern�ؼ��֣�ֱ�������õ����ⲿ�������������ļ�ͷinclude����ģ���ͷ�ļ�
extern void ppp_ISR(void);	
void PPP_IRQHandler(void)
{
	ppp_ISR();
}
*/
