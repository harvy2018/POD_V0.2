/*
*********************************************************************************************************
*	                                  
*	模块名称 : 中断模块
*	文件名称 : stm32f10x_it.c
*	版    本 : V1.0
*	说    明 : 本文件存放所有的中断服务函数。为了便于他人了解程序用到的中断，我们不建议将中断函数移到其他
*			的文件。
*			
*			我们只需要添加需要的中断函数即可。一般中断函数名是固定的，除非您修改了启动文件：
*				Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm\startup_stm32f10x_hd.s
*			
*			启动文件是汇编语言文件，定了每个中断的服务函数，这些函数使用了WEAK 关键字，表示弱定义，因此如
*			果我们在c文件中重定义了该服务函数（必须和它同名），那么启动文件的中断函数将自动无效。这也就
*			函数重定义的概念，这和C++中的函数重载的意义类似。
*				
*	修改记录 :
*		版本号  日期       作者    说明
*		v0.1    2009-12-27 armfly  创建该文件，ST固件库版本为V3.1.2
*		v1.0    2011-01-11 armfly  ST固件库升级到V3.4.0版本。
*		v2.0    2011-10-16 armfly  ST固件库升级到V3.5.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
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
*	Cortex-M3 内核异常中断服务程序
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	函 数 名: NMI_Handler
*	功能说明: 不可屏蔽中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/  
void NMI_Handler(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: HardFault_Handler
*	功能说明: 硬件失效中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/ 
void HardFault_Handler(void)
{
  /* 当硬件失效异常发生时进入死循环 */
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
	/*异常中断发生时，这个异常模式特定的物理R14,即lr被设置成该异常模式将要返回的地址*/
	stacked_lr = ((unsigned long) hardfault_args[5]); 	
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);
	
  SHCSR = (*((volatile unsigned long *)(0xE000ED24))); //系统Handler控制及状态寄存器
	MFSR = (*((volatile unsigned char *)(0xE000ED28)));	//存储器管理fault状态寄存器	
	BFSR = (*((volatile unsigned char *)(0xE000ED29)));	//总线fault状态寄存器	
	UFSR = (*((volatile unsigned short int *)(0xE000ED2A)));//用法fault状态寄存器		
	HFSR = (*((volatile unsigned long *)(0xE000ED2C)));  //硬fault状态寄存器			
	DFSR = (*((volatile unsigned long *)(0xE000ED30)));	//调试fault状态寄存器
	MMAR = (*((volatile unsigned long *)(0xE000ED34)));	//存储管理地址寄存器
	BFAR = (*((volatile unsigned long *)(0xE000ED38))); //总线fault地址寄存器
	while (1);

}
/*
*********************************************************************************************************
*	函 数 名: MemManage_Handler
*	功能说明: 内存管理异常中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/   
void MemManage_Handler(void)
{
  /* 当内存管理异常发生时进入死循环 */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	函 数 名: BusFault_Handler
*	功能说明: 总线访问异常中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/    
void BusFault_Handler(void)
{
  /* 当总线异常时进入死循环 */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	函 数 名: UsageFault_Handler
*	功能说明: 未定义的指令或非法状态中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/   
void UsageFault_Handler(void)
{
  /* 当用法异常时进入死循环 */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	函 数 名: SVC_Handler
*	功能说明: 通过SWI指令的系统服务调用中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/   
void SVC_Handler(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: DebugMon_Handler
*	功能说明: 调试监视器中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/   
void DebugMon_Handler(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: PendSV_Handler
*	功能说明: 可挂起的系统服务调用中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/     
void PendSV_Handler(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: SysTick_Handler
*	功能说明: 系统嘀嗒定时器中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/     
extern void SysTick_ISR(void);	/* 声明调用外部的函数 */
extern u32 SYSTEMTICK;
void SysTick_Handler(void)
{
	SYSTEMTICK++;
	SysTick_ISR();	/* 这个函数在bsp_timer.c中 */
}
/*
*********************************************************************************************************
*	STM32F10x内部外设中断服务程序
*	用户在此添加用到外设中断服务函数。有效的中断服务函数名请参考启动文件(startup_stm32f10x_xx.s)
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
        //读SR后读DR清除Idle
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
        //读SR后读DR清除Idle
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
        //读SR后读DR清除Idle
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
        //读SR后读DR清除Idle
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
        //读SR后读DR清除Idle
        c = UART5->SR;
        c = UART5->DR;    
        
        
    }	

  
}





void TIM2_IRQHandler(void) //抄报定时查询定时器
{
	
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{  
		   TIM_Cmd(TIM2,DISABLE);
//     DMA_Cmd(DMA1_Channel4, DISABLE);//关闭发送 
//		 DMA_Cmd(DMA1_Channel2, DISABLE);
//		 DMA_Cmd(DMA1_Channel6, DISABLE);
//		 DMA_Cmd(DMA2_Channel5, DISABLE);
//		
//		 DMA_Cmd(DMA1_Channel5, DISABLE);//关闭接受 
//		 DMA_Cmd(DMA1_Channel6, DISABLE);
//		 DMA_Cmd(DMA1_Channel3, DISABLE);
//		 DMA_Cmd(DMA2_Channel3, DISABLE);
		
 //    LED_Blink;	
	   TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		 
  //    GasDataQuery_TimeOut=1;//查询到时
	
    // CB_PORT[3].GasDataRes_Flag=1;
		// CB_PORT[1].GasDataRes_Flag=1;
	}
}

/****
void TIM3_IRQHandler(void)//PLC接收超时定时器
{
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
    
     PlcData_Get_Flag=1;//PLC口数据结束
	
	   TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
   

	}
}
*/
/*
*********************************************************************************************************
*	函 数 名: PPP_IRQHandler
*	功能说明: 外设中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/    
/* 
	因为中断服务程序往往和具体的应用有关，会用到用户功能模块的变量、函数。如果在本文件展开，会增加大量的
	外部变量声明或者include语句。
	
	因此，我们推荐这个地方只写一个调用语句，中断服务函数的本体放到对应的用户功能模块中。
	增加一层调用会降低代码的执行效率，不过我们宁愿损失这个效率，从而增强程序的模块化特性。
	
	增加extern关键字，直接引用用到的外部函数，避免在文件头include其他模块的头文件
extern void ppp_ISR(void);	
void PPP_IRQHandler(void)
{
	ppp_ISR();
}
*/
