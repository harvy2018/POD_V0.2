
#include <stdio.h>			/* ��Ϊ�õ���printf���������Ա����������ļ� */
#include <string.h>
#include <time.h>	
#include <math.h>	

#include "SEGGER_RTT.h"
#include "stm32f10x.h"		/* ���Ҫ��ST�Ĺ̼��⣬�����������ļ� */

#include "bsp_usart.h"		/* printf����������������ڣ����Ա����������ļ� */
#include "bsp_timer.h"		/* systick��ʱ��ģ�� */
#include "download.h"	

#include "18b20.h"
#include "rtc.h"
#include "bsp_spi_flash.h"

#include "dev_state.h"
#include "SBUS.h"	
#include "board.h"	
#include "uartPort.h"	
#include "Gimbal.h"
#include "cam.h"

#define DEBUG 1
#define LOG_ON 1

//#define PITCH_DOWN_LIMIT   -30
//#define PITCH_UP_LIMIT   20
//#define YAW_DOWN_LIMIT   -50
//#define YAW_UP_LIMIT   50



#define NVIC_VectTab_BIAS    0       //�̼������ַ0x8000000

extern UartRBuffer	uartRBuffer[UART_TOTAL_PORT_NUM];

u8 FW_VER[4]={2,0,2,0};	


volatile u32 SYSTEMTICK;
volatile u32 unixtime;

u32 HW_ID[4];//cpu id

u32 cin;//����̨����

u8 T_upload_cnt;


u8 Angle_Init_Flag=1;
u8 Angle_Init_Cnt=0;


static void InitBoard(void);
static void PrintfLogo(void);
static void GPIO_Configuration(void);
void NVIC_Configuration(void);

void Get_SerialNum(unsigned int* SerialID);
void Watchdog_Init(void);

u8 testbuf[32]="test jlink spi flash!";
u8 data[32];
u8 len;
u16 availBytes;
u8 buf[128];
u8 comdata;
extern u8 servo_state;

extern SBUS_VALUE YAW;
extern SBUS_VALUE PITCH;
extern u8 Angle_Init_Flag;

int main(void)
{
    u8 i;
    u32 count100ms=0;
    
	uartBufferInitialize();   
	InitBoard();	
	Get_SerialNum(HW_ID);		
	BUZZ_ON;
	bsp_DelayMS(200);
		

	NVIC_Configuration(); 
	BUZZ_OFF;
     	
    ServoInit();
    
    PrintfLogo();
				
    while(1)
    {
        if(getSysTick() % 100 == 0) // 100ms based tasks
        {
			count100ms++;					
				
		    FKProcess();						
								
			GimbalDataProcess();
					
			ServoActTask();	
            
            CamDataProcess();

			if(count100ms%10==0)		
			{
				LED_Blink;
                Angle_Init_Cnt++;               
                if(Angle_Init_Flag==1 && Angle_Init_Cnt>=5)
				{
                    GetAngle();
                    Angle_Init_Cnt=5;
                }                       
                
			}
						
		     bsp_DelayMS(1);											            
            
        }
        else
        {
                ;//Nothing to do here right now
        }
        
    }
							 
			
			
}




static void GPIO_Configuration(void)
{	
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 			//����GPIOB clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 			//����GPIOB clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 			//����GPIOB clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
//    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE );
		
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC \
//			| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG,
//				ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//;GPIO_Mode_AF_PP
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); // PA11-->tim1 4channel  PWM1 

	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); //PB5��LED, PB0��buzz
	
}

/*
*********************************************************************************************************
*	�� �� ��: InitBoard
*	����˵��: ��ʼ��Ӳ���豸
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitBoard(void)
{		
	
	NVIC_SetVectorTable(NVIC_VectTab_FLASH,NVIC_VectTab_BIAS);//�ж�������ƫ������
	
	
	/* �����������ʱ�� */
	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	
	
	/* ���ð�����LEDָʾ��GPIO */
	
    GPIO_Configuration();	
	/* ��ʼ��systick��ʱ������������ʱ�ж� */
	bsp_InitTimer(); 
	/* ���ô��ڣ�����printf��� */
	bsp_InitUart();
	bsp_InitUart2();
	bsp_InitUart3();
    bsp_InitUart4();
	
//	sf_InitHard();	/* ��ʼ��SPIӲ�� */
	//TIMX_Init(CALC_TYPE_US);
	TIM1_Init();
	
//	TIM_Init(TIM2,CB_Query_Interval);//��˰�ڶ�ʱ��ѯ��ʱ��
	//TIM_Init(TIM3,PLC_RecTimeout_Interval);//PLC�ڶ�ʱ��ѯ��ʱ��

		
	
}


void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;

  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
 // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
 // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
//	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	
	
		   			

}


static void PrintfLogo(void)
{	
	

	SEGGER_RTT_printf(0,"*************************************************************\r\n");
	SEGGER_RTT_printf(0,"* Release Date : %s %s\r\n", __DATE__,__TIME__);	
	SEGGER_RTT_printf(0,"* FW Rev: %d.%d.%d.%d\r\n", FW_VER[0],FW_VER[1],FW_VER[2],FW_VER[3]);		
	SEGGER_RTT_printf(0,"*************************************************************\r\n");
	
//	printf("*************************************************************\r\n");
//	printf("* Release Date : %s %s\r\n", __DATE__,__TIME__);	
//	printf("* FW Rev: %d.%d.%d.%d\r\n", FW_VER[0],FW_VER[1],FW_VER[2],FW_VER[3]);	
//	printf("* HW ID: %08X %08X %08X %08X \r\n",__REV(HW_ID[0]),__REV(HW_ID[1]),__REV(HW_ID[2]),__REV(HW_ID[3]));	
//	printf("*************************************************************\r\n");

//	SEGGER_RTT_ConfigDownBuffer(1,"H",seggerdown,16,SEGGER_RTT_MODE_DEFAULT);
//	SEGGER_RTT_ConfigUpBuffer(1,"H",seggerup,Channel_MID,SEGGER_RTT_MODE_DEFAULT);
//	SEGGER_RTT_TerminalOut(1,"*************************************************************\r\n");
//	SEGGER_RTT_TerminalOut(1,"* Release Date : %s %s\r\n");	
//	SEGGER_RTT_TerminalOut(1,"* FW Rev : %d.%d.%d.%d\r\n");		
//	SEGGER_RTT_TerminalOut(1,"*************************************************************\r\n");

	
}







