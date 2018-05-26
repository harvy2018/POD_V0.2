/*
*********************************************************************************************************
*	                                  
*	ģ������ : ��ʱ��ģ��
*	�ļ����� : bsp_timer.c
*	��    �� : V2.0
*	˵    �� : ����systick�жϣ�ʵ�����ɸ������ʱ��
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v0.1    2009-12-27 armfly  �������ļ���ST�̼���汾ΪV3.1.2
*		v1.0    2011-01-11 armfly  ST�̼���������V3.4.0�汾��
*       v2.0    2011-10-16 armfly  ST�̼���������V3.5.0�汾��
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
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
	�ڴ˶������ɸ������ʱ��ȫ�ֱ���
	ע�⣬��������__IO �� volatile����Ϊ����������жϺ���������ͬʱ�����ʣ��п�����ɱ����������Ż���
*/
#define TMR_COUNT	4		/* �����ʱ���ĸ�������1��������bsp_DelayMS()ʹ�� */

SOFT_TMR g_Tmr[TMR_COUNT];

/* ȫ������ʱ�䣬��λ10ms������uIP */
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
	
	/* �������е������ʱ�� */
	for (i = 0; i < TMR_COUNT; i++)
	{
		g_Tmr[i].count = 0;
		g_Tmr[i].flag = 0;
	}
	
	/* 
		����systic�ж�����Ϊ1ms��������systick�жϡ�
    	��������� \Libraries\CMSIS\CM3\CoreSupport\core_cm3.h 
    	
    	Systick�ж�������(\Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm\
    		startup_stm32f10x_hd.s �ļ��ж���Ϊ SysTick_Handler��
    	SysTick_Handler������ʵ����stm32f10x_it.c �ļ���
    	SysTick_Handler����������SysTick_ISR()�������ڱ��ļ�ĩβ��
    */	
	RCC_GetClocksFreq(&m_CLK);
	
	SysTick_Config(m_CLK.HCLK_Frequency/ 1000);
	
}

/*
*********************************************************************************************************
*	�� �� ��: SysTick_ISR
*	����˵��: SysTick�жϷ������ÿ��1ms����1��
*	��    �Σ���
*	�� �� ֵ: ��
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

	g_iRunTime++;	/* ȫ������ʱ��ÿ1ms��1 */	
	if (g_iRunTime == 0x7FFFFFFF)
	{
		g_iRunTime = 0;
	}
		
	if (++s_count >= 3000)//5sι��
	{
		s_count = 0;
		
	//	LED_Blink;   
	//  ClearDog;	


		/* 
			����Ĵ���ʵ�����а����ļ�⡣�������ÿ��10msһ�ξ����ˣ�һ����40ms���˲�����Ϳ���
			��Ч���˵���е������ɵİ���������
		*/
	//	bsp_KeyPro();		/* �ú����� bsp_button.c ��ʵ�� */
		
	}
	
//	AdcPro();
	
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SoftTimerDec
*	����˵��: ÿ��1ms�����ж�ʱ��������1�����뱻SysTick_ISR�����Ե��á�
*	��    �Σ�_tmr : ��ʱ������ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_SoftTimerDec(SOFT_TMR *_tmr)
{
	if (_tmr->flag == 0)
	{
		if (_tmr->count > 0)
		{
			/* �����ʱ����������1�����ö�ʱ�������־ */
			if (--_tmr->count == 0)
			{
				_tmr->flag = 1;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_DelayMS
*	����˵��: ms���ӳ٣��ӳپ���Ϊ����1ms
*	��    �Σ�n : �ӳٳ��ȣ���λ1 ms�� n Ӧ����2
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_DelayMS(uint32_t n)
{
	/* ���� n = 1 �������������� */
	if (n <= 1)
	{
		n = 2;
	}
	
	__set_PRIMASK(1);  		/* ���ж� */
	g_Tmr[0].count = n;
	g_Tmr[0].flag = 0;
	__set_PRIMASK(0);  		/* ���ж� */

	while (1)
	{
		CPU_IDLE();	/* �˴��ǿղ������û����Զ��壬��CPU����IDLE״̬���Խ��͹��ģ���ʵ��ι�� */

		/* �ȴ��ӳ�ʱ�䵽 */
		if (g_Tmr[0].flag == 1)
		{
			break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_StartTimer
*	����˵��: ����һ����ʱ���������ö�ʱ���ڡ�
*	��    �Σ�	_id     : ��ʱ��ID��ֵ��1,TMR_COUNT-1�����û���������ά����ʱ��ID���Ա��ⶨʱ��ID��ͻ��
*						  ��ʱ��ID = 0 ������bsp_DelayMS()����
*				_period : ��ʱ���ڣ���λ1ms
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_StartTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* while(1); ���� */
		return;
	}

	__set_PRIMASK(1);  		/* ���ж� */
	g_Tmr[_id].count = _period;
	g_Tmr[_id].flag = 0;
	__set_PRIMASK(0);  		/* ���ж� */
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_CheckTimer
*	����˵��: ��ⶨʱ���Ƿ�ʱ
*	��    �Σ�	_id     : ��ʱ��ID��ֵ��1,TMR_COUNT-1�����û���������ά����ʱ��ID���Ա��ⶨʱ��ID��ͻ��
*						  0 ����
*				_period : ��ʱ���ڣ���λ1ms
*	�� �� ֵ: ���� 0 ��ʾ��ʱδ���� 1��ʾ��ʱ��
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
*	�� �� ��: bsp_GetRunTime
*	����˵��: ��ȡCPU����ʱ�䣬��λ1ms
*	��    �Σ���
*	�� �� ֵ: CPU����ʱ�䣬��λ1ms
*********************************************************************************************************
*/
int32_t bsp_GetRunTime(void)
{
	int runtime; 

	__set_PRIMASK(1);  		/* ���ж� */
	
	runtime = g_iRunTime;	/* ������Systick�жϱ���д����˹��жϽ��б��� */
		
	__set_PRIMASK(0);  		/* ���ж� */

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
	TIM_TimeBaseStructure.TIM_Period = 2000*sec;//��ʱ3��
	TIM_TimeBaseStructure.TIM_Prescaler = 36000-1;//��ʱ��ʱ��Ƶ��72M
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
    if(type==CALC_TYPE_S) //��ʱ��SΪ��λʱ��ʱ��Ƶ��57600Hz���ⲿ��Ҫ1250�μ�ʱ   
    {  
        TimX.TIM_Prescaler=7200-1; //10KHZ 100us 
    }else if(type==CALC_TYPE_MS)  
    {  
        TimX.TIM_Prescaler=720-1; ////100KHz 10us,��ʱ������100��Ϊ1ms   
 
    }else if(type==CALC_TYPE_US)  
    {     
        TimX.TIM_Prescaler=72-1; //1MHz ,����1��Ϊus   
    }else  
    {  
        TimX.TIM_Prescaler=7200-1;  //10KHZ
    }  
    TimX.TIM_ClockDivision=0;  
    TimX.TIM_CounterMode=TIM_CounterMode_Down; //���¼���   
    TIM_TimeBaseInit(TIMX,&TimX);         
}  
  
void TIMX_S_CALC(uint32_t s)  
{  
    u16 counter=(s*10000)&0xFFFF; //ǰ�ᶨʱ��ʱ��Ϊ10kHz   
    TIM_Cmd(TIMX,ENABLE);  
    TIM_SetCounter(TIMX,counter); //���ü���ֵ   
      
    while(counter>1)  
    {  
        counter=TIM_GetCounter(TIMX);  
    }  
    TIM_Cmd(TIMX,DISABLE);  
}  
  
void TIMX_MS_CALC(uint32_t ms)  
{  
    u16 counter=(ms*100)&0xFFFF;   //&0xFFFF ��ֹ���65535
    TIM_Cmd(TIMX,ENABLE);  
    TIM_SetCounter(TIMX,counter); //���ü���ֵ   
      
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
    TIM_SetCounter(TIMX,counter); //���ü���ֵ   
  
    while(counter>1)  
    {  
        counter=TIM_GetCounter(TIMX);  
    }  
    TIM_Cmd(TIMX,DISABLE);  
}  



