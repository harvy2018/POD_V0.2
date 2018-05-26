
//#include <absacc.h>
#include "bsp_usart.h"
#include "rtc.h"
#include "sfDemo.h"
#include "18b20.h"
#include <string.h>

//void CAN_DeInit(CAN_TypeDef* CANx);
//void CAN_StructInit(CAN_InitTypeDef* CAN_InitStruct);

//u8  readbuf[] __at(0x68000000); 
//u8  SDBuf[]   __at(0x68010000); 
u8  RS422[10]={0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA};
u8  Comm[128];
u8  Resp[128];
u8  Command_ID=0;
u8  FunctionNum=0;
u8  NET_TEST;
u8  unixtime_sendflag=0;//是否要向上传标志
u8  unixtime_getflag=0; //1秒钟到来标志
extern u32  unixtime;
extern uint16_t  Uart1_InNum;
extern uint16_t  Uart1_OutNum;

extern u8 T18b20_sendflag;
extern u8 Rx1Buffer[1024];//  __at(0x68100000);
u16 RxCounter1;
u8 COM_TEST=0;
u32 unixtime=0;
 //测试LED



void Led_Test(u8 FNo,u8 len,u8 *Buf)
{
	u16 LED_Index;
	if(len!=1)	//错误指令直接返回
		return;

	LED_Index=*Buf;

	switch(FNo)
	{
		case 0:
			 switch (LED_Index)
			 {
			 	case 0:
					GPIO_WriteBit(GPIOC,GPIO_Pin_7,Bit_RESET);
					break;
				case 1:
				    GPIO_WriteBit(GPIOG,GPIO_Pin_13,Bit_RESET);
					break;
				case 2:
				    GPIO_WriteBit(GPIOG,GPIO_Pin_12,Bit_RESET);
					break;
				case 3:
				    GPIO_WriteBit(GPIOG,GPIO_Pin_11,Bit_RESET);
					break;
			    case 4:
				   	GPIO_WriteBit(GPIOC,GPIO_Pin_7,Bit_RESET);
					break;
				default:
					break;
			}		    	
		break;
		case 1:
		      switch (*Buf)
			 {
			 	case 0:
					GPIO_WriteBit(GPIOC,GPIO_Pin_7,Bit_SET);
					break;
				case 1:
				    GPIO_WriteBit(GPIOG,GPIO_Pin_13,Bit_SET);
					break;
				case 2:
				    GPIO_WriteBit(GPIOG,GPIO_Pin_12,Bit_SET);
					break;
				case 3:
				    GPIO_WriteBit(GPIOG,GPIO_Pin_11,Bit_SET);
					break;
			    case 4:
				    GPIO_WriteBit(GPIOC,GPIO_Pin_7,Bit_SET);
					break;
				default:
					break;
			}		
		break;

		default:
		break;
	}

}
				


//测试蜂鸣器				 
void Beep_Test(u8 FNo,u8 len,u8 *Buf)
{	
	switch(FNo)
	{
		case 0:		 
			 GPIO_ResetBits(GPIOC,GPIO_Pin_6);//不鸣叫
		break;

		case 1:
			  GPIO_SetBits(GPIOC,GPIO_Pin_6); //鸣叫
		break;

		default:
		break;
	}
}

//测试网络				 
//void Net_Test(u8 FNo,u8 len,u8 *Buf)
//{
//		eth_in_buf_len = dm9k_receive_packet(eth_in_buf);; // 接收来自NIC的数据包							
//		if(eth_in_buf_len > 0)				   // 有数据包被接收？
//			ethernetif_input(&ethif);	   // 是，将数据包交给LwIP处理	
//}

//测试RS422				
void RS422_Test(u8 FNo,u8 len,u8 *Buf)
{
    u8 i;
 if (FNo==1)	   //232回数
	{
	   COM_TEST=1;
		
//		 Uart1SendData(0x11);  //头
//		 Uart1SendData(0x66);
//		 Uart1SendData(0x07);  //命令号
//		 Uart1SendData(0x01);  //功能号
//		 Uart1SendData(0x0a);  //功能号
//		 Uart1SendBuf(Buf,len);
//		 Uart1SendData(0x22);  //尾	   */
		 
//		 Uart4SendBuf(Buf,len);	//U转串输出	 
	}
	else					//422回数
	{
//	     Uart3SendData(0x11);  //头
//		 Uart3SendData(0x66);
//		 Uart3SendData(0x07);  //命令号
//		 Uart3SendData(0x01);  //功能号
//		 Uart3SendData(0x0a);  //功能号
//		 Uart3SendBuf(Buf,len);
//		 Uart3SendData(0x22);  //尾	 	  
		  COM_TEST=0;
	}

}

//测试内存
void RAM_Test(u8 FNo,u8 len,u8 *Buf)
{
	u32 i;
	u16 data_temp,data_temp0,temp;
	u16 *Ram_temp;
    u8 status=0;

	if (*Buf)	//ADD:0x6820 0000~683F FFFF 第二片
	{
	   Ram_temp= (u16*)0x68200000;
	}
	else  //ADD:0x6800 0000~681F FFFF	第一片
	{
	   Ram_temp= (u16*)0x68000000;
	}

	temp= *(Buf+1);
	temp<<=8;
	temp|= *(Buf+2);
	data_temp=temp;//(u16)(*Buf)<<8+(u16)(*(Buf+1));
	
	for(i=0;i<0xfffff;i++)	 //整片全地址测试
	{
		*Ram_temp=data_temp;
		data_temp0=	*Ram_temp;
		if(data_temp!=data_temp0)
		{
			//向上返回数据
		    status=1;//失败
			break;
		}
		 Ram_temp++;
	}

//	if (!status)
//	{
//		Ram_temp+=0x50000;
//
//		for(i=0;i<1024;i++)
//		{
//			*Ram_temp=data_temp;
//			data_temp0=	*Ram_temp;
//			if(data_temp!=data_temp0)
//			{
//				//向上返回数据
//			    status=1;//失败
//				break;
//			}
//			 Ram_temp++;
//		}
//	}

	 //返回数据
	 Uart1SendData(0x11);  //头
	 Uart1SendData(0x66);
	 Uart1SendData(0x08);  //命令号
	 Uart1SendData(0x01);  //功能号
	 Uart1SendData(0x03);  //长度
	 Uart1SendData(status);  //数据
	 Uart1SendData(data_temp0>>8); 
	 Uart1SendData(data_temp0); 
	 Uart1SendData(0x22);  //尾
		
}



void RTC_TEST(u8 FNo,u8 len,u8 *Buf)
{
   unixtime=*Buf;
	 Buf++;
	 unixtime=(unixtime<<8)+*Buf;
	 Buf++;
	 unixtime=(unixtime<<8)+*Buf;
	 Buf++;
	 unixtime=(unixtime<<8)+*Buf;

	 unixtime+=2; //补偿手动设置误差2秒
	
	 
	 RTC_Configuration();
 
     Time_Adjust();
	 BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
   unixtime_sendflag=1;//表示需要向上返回时间信息
}




//指令解析
void  Command_Free(void)
{
	u8 Com_len=0;
	u8 i;
	u8 Data_Buf[256];//数据缓冲

	if(Rx1Buffer[Uart1_OutNum]==0xEE)//桢头判断
    {	   
	        Uart1_OutNum++;
			while(Uart1_InNum==Uart1_OutNum);                    
			if(Rx1Buffer[Uart1_OutNum]==0x99)
				Uart1_OutNum++;		
			else
		   	  return ;
		
			
			while(Uart1_InNum==Uart1_OutNum); //取命令号
			Command_ID=Rx1Buffer[Uart1_OutNum];
			Uart1_OutNum++;
			
			while(Uart1_InNum==Uart1_OutNum); //取功能号
			FunctionNum=Rx1Buffer[Uart1_OutNum];
			Uart1_OutNum++;

			while(Uart1_InNum==Uart1_OutNum); //取命令长度
			Com_len=Rx1Buffer[Uart1_OutNum];
			Uart1_OutNum++;
			
			for(i=0;i<Com_len;i++)	//取有效数据
			{	
			    while(Uart1_InNum==Uart1_OutNum); 
			    Data_Buf[i]=Rx1Buffer[Uart1_OutNum];
			    Uart1_OutNum++;
			}
					
			while(Uart1_InNum==Uart1_OutNum); //判断尾                   
			if(Rx1Buffer[Uart1_OutNum]==0xDD)
			{	Uart1_OutNum++;
			}
			else
			{	return;
			}
			
			switch(Command_ID)
			{    
			    Uart1_OutNum=0;
				Uart1_InNum=0;
				case 0x00: //测试LED灯
			     	Led_Test(FunctionNum,Com_len,Data_Buf);
				 break;

				 case 0x01: //测试PSAM卡1
				 	;
				 break;

				 case 0x02: //测试PSAM卡2
				 	;
				 break;

				 case 0x03: //测试PSAM卡3
				 	;
				 break;

				 case 0x04: //测试PSAM卡4
				 ;
				 break;

				 case 0x05: //测试蜂鸣器
				 	Beep_Test(FunctionNum,Com_len,Data_Buf);
				 break;

				 case 0x06: //测试FLASH
					 
				   if(FunctionNum==0)
					 {
						 
				     FLASH_Test();
						 
					 }
					 else if(FunctionNum==3)
					 {	 
							sf_InitHard();	/* 初始化SPI硬件 */
							sf_EraseChip();
							Uart1SendData(0x11);  //头
							Uart1SendData(0x66);
							Uart1SendData(0x06);  //命令号
							Uart1SendData(0x01);  //功能号
							Uart1SendData(0x0);  //长度
							Uart1SendData(0x01);  //长度
							Uart1SendData(0x06);  //数据0x06 Flash擦除完成!
							Uart1SendData(0x22);  //尾
					 }
					 else
						 FLASH_WD_TEST(FunctionNum,Com_len,Data_Buf);
					 
					  
					 
				 break;

				 case 0x07: //测试RS422接口
				 	RS422_Test(FunctionNum,Com_len,Data_Buf);
				 break;
						
				 case 0x08: //测试外部RAM
				//	 RAM_Test(FunctionNum,Com_len,Data_Buf);
				 break;

				 case 0x09 ://测试实时时钟
				    
					   RTC_TEST(FunctionNum,Com_len,Data_Buf);

				 break;

				 case 0x0a ://测试SD 卡
				     ;
				 break;

				 case 0x0b://铁电存储器测试   取消实时时钟上传

					   ;
				 break;

				 case 0x0c://取消实时时钟上传
//				 	   if(unixtime_sendflag)
//					    {
								unixtime_sendflag=0;
								RTC_ITConfig(RTC_IT_SEC, DISABLE);	   	//先禁止实施时钟中断，以保护数据						
//							}
//							else
//							{
//								unixtime_sendflag=1;
//								RTC_ITConfig(RTC_IT_SEC, ENABLE);	   
//							}								
					  
				 break;	 
				 case 0x0d://温度传感器测试	
			       if(FunctionNum) 
							  T18b20_sendflag=0;
					   if(!FunctionNum) 
							  T18b20_sendflag=1;
				 break;	 

				 case 0x0E://网络测试		 
				 	   if (FunctionNum==1)
					      NET_TEST=1;
					   else
					  	  NET_TEST=0;
				 break;
				 	 
				 default:
				 break;
			}					
    }  
}


