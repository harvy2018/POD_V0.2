/*
*********************************************************************************************************
*	                                  
*	模块名称 : 定时器模块
*	文件名称 : bsp_timer.c
*	版    本 : V2.0
*	说    明 : 设置systick中断，实现若干个软件定时器
*	修改记录 :
*		版本号  日期       作者    说明
*		v0.1    2009-12-27 armfly  创建该文件，ST固件库版本为V3.1.2
*		v1.0    2011-01-11 armfly  ST固件库升级到V3.4.0版本。
*       v2.0    2011-10-16 armfly  ST固件库升级到V3.5.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include <stdio.h>

#include "bsp_timer.h"
#include "cb.h"

#define TIMX      TIM2
#define TIMX_CLK  RCC_APB1Periph_TIM2

/* 
	在此定义若干个软件定时器全局变量
	注意，必须增加__IO 即 volatile，因为这个变量在中断和主程序中同时被访问，有可能造成编译器错误优化。
*/
#define TMR_COUNT	4		/* 软件定时器的个数，第1个保留给bsp_DelayMS()使用 */

SOFT_TMR g_Tmr[TMR_COUNT];

/* 全局运行时间，单位10ms，用于uIP */
__IO uint32_t g_iRunTime = 0;

static void bsp_SoftTimerDec(SOFT_TMR *_tmr);
RCC_ClocksTypeDef m_CLK;


uint32_t  getSysTick()
{
    return g_iRunTime;
}


void bsp_InitTimer(void)
{
	uint8_t i;
	
	u32 clk;
	
	/* 清零所有的软件定时器 */
	for (i = 0; i < TMR_COUNT; i++)
	{
		g_Tmr[i].count = 0;
		g_Tmr[i].flag = 0;
	}
	
	/* 
		配置systic中断周期为1ms，并启动systick中断。
    	这个函数在 \Libraries\CMSIS\CM3\CoreSupport\core_cm3.h 
    	
    	Systick中断向量在(\Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm\
    		startup_stm32f10x_hd.s 文件中定义为 SysTick_Handler。
    	SysTick_Handler函数的实现在stm32f10x_it.c 文件。
    	SysTick_Handler函数调用了SysTick_ISR()函数，在本文件末尾。
    */	
	RCC_GetClocksFreq(&m_CLK);
	
	SysTick_Config(m_CLK.HCLK_Frequency/ 1000);
	
}

/*
*********************************************************************************************************
*	函 数 名: SysTick_ISR
*	功能说明: SysTick中断服务程序，每隔1ms进入1次
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void SysTick_ISR(void)
{
	static uint16_t s_count = 0;
	extern u16 AdcValue;
	uint8_t i;

	for (i = 0; i < TMR_COUNT; i++)
	{
		bsp_SoftTimerDec(&g_Tmr[i]);
	}

	g_iRunTime++;	/* 全局运行时间每1ms增1 */	
	if (g_iRunTime == 0x7FFFFFFF)
	{
		g_iRunTime = 0;
	}
		
	if (++s_count >= 3000)//5s喂狗
	{
		s_count = 0;
		
	//	LED_Blink;   
	//  ClearDog;	


		/* 
			下面的代码实现所有按键的检测。按键检测每隔10ms一次就行了，一般做40ms的滤波处理就可以
			有效过滤掉机械动作造成的按键抖动。
		*/
	//	bsp_KeyPro();		/* 该函数在 bsp_button.c 中实现 */
		
	}
	
//	AdcPro();
	
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SoftTimerDec
*	功能说明: 每隔1ms对所有定时器变量减1。必须被SysTick_ISR周期性调用。
*	形    参：_tmr : 定时器变量指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_SoftTimerDec(SOFT_TMR *_tmr)
{
	if (_tmr->flag == 0)
	{
		if (_tmr->count > 0)
		{
			/* 如果定时器变量减到1则设置定时器到达标志 */
			if (--_tmr->count == 0)
			{
				_tmr->flag = 1;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DelayMS
*	功能说明: ms级延迟，延迟精度为正负1ms
*	形    参：n : 延迟长度，单位1 ms。 n 应大于2
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_DelayMS(uint32_t n)
{
	/* 避免 n = 1 出现主程序死锁 */
	if (n <= 1)
	{
		n = 2;
	}
	
	__set_PRIMASK(1);  		/* 关中断 */
	g_Tmr[0].count = n;
	g_Tmr[0].flag = 0;
	__set_PRIMASK(0);  		/* 开中断 */

	while (1)
	{
		CPU_IDLE();	/* 此处是空操作。用户可以定义，让CPU进入IDLE状态，以降低功耗；或实现喂狗 */

		/* 等待延迟时间到 */
		if (g_Tmr[0].flag == 1)
		{
			break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_StartTimer
*	功能说明: 启动一个定时器，并设置定时周期。
*	形    参：	_id     : 定时器ID，值域【1,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*						  定时器ID = 0 已用于bsp_DelayMS()函数
*				_period : 定时周期，单位1ms
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_StartTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* while(1); 死机 */
		return;
	}

	__set_PRIMASK(1);  		/* 关中断 */
	g_Tmr[_id].count = _period;
	g_Tmr[_id].flag = 0;
	__set_PRIMASK(0);  		/* 开中断 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CheckTimer
*	功能说明: 检测定时器是否超时
*	形    参：	_id     : 定时器ID，值域【1,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*						  0 用于
*				_period : 定时周期，单位1ms
*	返 回 值: 返回 0 表示定时未到， 1表示定时到
*********************************************************************************************************
*/
uint8_t bsp_CheckTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		return 0;
	}

	if (g_Tmr[_id].flag == 1)
	{
		g_Tmr[_id].flag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetRunTime
*	功能说明: 获取CPU运行时间，单位1ms
*	形    参：无
*	返 回 值: CPU运行时间，单位1ms
*********************************************************************************************************
*/
int32_t bsp_GetRunTime(void)
{
	int runtime; 

	__set_PRIMASK(1);  		/* 关中断 */
	
	runtime = g_iRunTime;	/* 由于在Systick中断被改写，因此关中断进行保护 */
		
	__set_PRIMASK(0);  		/* 开中断 */

	return runtime;
}

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;

uint16_t PrescalerValue = 0;


void TIM_Init(TIM_TypeDef* TIMx,u32 sec)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_DeInit(TIMx);
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 2000*sec;//定时3秒
	TIM_TimeBaseStructure.TIM_Prescaler = 36000-1;//定时器时钟频率72M
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIMx, TIM_FLAG_Update); 
	TIM_ARRPreloadConfig(TIMx, ENABLE);
	/* TIM IT enable */
	TIM_ITConfig(TIMx,TIM_IT_Update, ENABLE);
	/* TIM2 enable counter */
	TIM_Cmd(TIMx, ENABLE);
}


void TIM1_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
		uint16_t PrescalerValue = 0;
  /* Compute the prescaler value */
//  PrescalerValue = (uint16_t) (SystemCoreClock / 72000000) - 1;
	PrescalerValue=0;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 2000; // 72 MHz / 2000 = 36 KHz
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
}


void TIM4_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
		uint16_t PrescalerValue = 0;
  /* Compute the prescaler value */
//  PrescalerValue = (uint16_t) (SystemCoreClock / 72000000) - 1;
	PrescalerValue=0;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 2000; // 72 MHz / 2000 = 36 KHz
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
}


void PWM3_SET(u16 p)
{
	u16 Peroid=1;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OCInitStructure.TIM_Pulse = Peroid*p;

	TIM_OC4Init(TIM1, &TIM_OCInitStructure);

	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM1, ENABLE);

	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
		
}

void TIMX_Init(CALC_TYPE type)  
{  
    TIM_TimeBaseInitTypeDef TimX;  
    RCC_APB1PeriphClockCmd(TIMX_CLK,ENABLE);  
    TimX.TIM_Period=1; //???   
    if(type==CALC_TYPE_S) //延时以S为单位时，时钟频率57600Hz，外部需要1250次计时   
    {  
        TimX.TIM_Prescaler=7200-1; //10KHZ 100us 
    }else if(type==CALC_TYPE_MS)  
    {  
        TimX.TIM_Prescaler=720-1; ////100KHz 10us,定时器计数100次为1ms   
 
    }else if(type==CALC_TYPE_US)  
    {     
        TimX.TIM_Prescaler=72-1; //1MHz ,计数1次为us   
    }else  
    {  
        TimX.TIM_Prescaler=7200-1;  //10KHZ
    }  
    TimX.TIM_ClockDivision=0;  
    TimX.TIM_CounterMode=TIM_CounterMode_Down; //向下计数   
    TIM_TimeBaseInit(TIMX,&TimX);         
}  
  
void TIMX_S_CALC(uint32_t s)  
{  
    u16 counter=(s*10000)&0xFFFF; //前提定时器时钟为10kHz   
    TIM_Cmd(TIMX,ENABLE);  
    TIM_SetCounter(TIMX,counter); //设置计数值   
      
    while(counter>1)  
    {  
        counter=TIM_GetCounter(TIMX);  
    }  
    TIM_Cmd(TIMX,DISABLE);  
}  
  
void TIMX_MS_CALC(uint32_t ms)  
{  
    u16 counter=(ms*100)&0xFFFF;   //&0xFFFF 防止溢出65535
    TIM_Cmd(TIMX,ENABLE);  
    TIM_SetCounter(TIMX,counter); //设置计数值   
      
    while(counter>1)  
    {  
        counter=TIM_GetCounter(TIMX);  
    }  
    TIM_Cmd(TIMX,DISABLE);  
}  
  
void TIMX_US_CALC(uint32_t us)  
{  
    u16 counter=us&0xffff;  
    TIM_Cmd(TIMX,ENABLE);  
    TIM_SetCounter(TIMX,counter); //设置计数值   
  
    while(counter>1)  
    {  
        counter=TIM_GetCounter(TIMX);  
    }  
    TIM_Cmd(TIMX,DISABLE);  
}  



