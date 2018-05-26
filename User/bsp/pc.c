
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
u8  unixtime_sendflag=0;//�Ƿ�Ҫ���ϴ���־
u8  unixtime_getflag=0; //1���ӵ�����־
extern u32  unixtime;
extern uint16_t  Uart1_InNum;
extern uint16_t  Uart1_OutNum;

extern u8 T18b20_sendflag;
extern u8 Rx1Buffer[1024];//  __at(0x68100000);
u16 RxCounter1;
u8 COM_TEST=0;
u32 unixtime=0;
 //����LED



void Led_Test(u8 FNo,u8 len,u8 *Buf)
{
	u16 LED_Index;
	if(len!=1)	//����ָ��ֱ�ӷ���
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
				


//���Է�����				 
void Beep_Test(u8 FNo,u8 len,u8 *Buf)
{	
	switch(FNo)
	{
		case 0:		 
			 GPIO_ResetBits(GPIOC,GPIO_Pin_6);//������
		break;

		case 1:
			  GPIO_SetBits(GPIOC,GPIO_Pin_6); //����
		break;

		default:
		break;
	}
}

//��������				 
//void Net_Test(u8 FNo,u8 len,u8 *Buf)
//{
//		eth_in_buf_len = dm9k_receive_packet(eth_in_buf);; // ��������NIC�����ݰ�							
//		if(eth_in_buf_len > 0)				   // �����ݰ������գ�
//			ethernetif_input(&ethif);	   // �ǣ������ݰ�����LwIP����	
//}

//����RS422				
void RS422_Test(u8 FNo,u8 len,u8 *Buf)
{
    u8 i;
 if (FNo==1)	   //232����
	{
	   COM_TEST=1;
		
//		 Uart1SendData(0x11);  //ͷ
//		 Uart1SendData(0x66);
//		 Uart1SendData(0x07);  //�����
//		 Uart1SendData(0x01);  //���ܺ�
//		 Uart1SendData(0x0a);  //���ܺ�
//		 Uart1SendBuf(Buf,len);
//		 Uart1SendData(0x22);  //β	   */
		 
//		 Uart4SendBuf(Buf,len);	//Uת�����	 
	}
	else					//422����
	{
//	     Uart3SendData(0x11);  //ͷ
//		 Uart3SendData(0x66);
//		 Uart3SendData(0x07);  //�����
//		 Uart3SendData(0x01);  //���ܺ�
//		 Uart3SendData(0x0a);  //���ܺ�
//		 Uart3SendBuf(Buf,len);
//		 Uart3SendData(0x22);  //β	 	  
		  COM_TEST=0;
	}

}

//�����ڴ�
void RAM_Test(u8 FNo,u8 len,u8 *Buf)
{
	u32 i;
	u16 data_temp,data_temp0,temp;
	u16 *Ram_temp;
    u8 status=0;

	if (*Buf)	//ADD:0x6820 0000~683F FFFF �ڶ�Ƭ
	{
	   Ram_temp= (u16*)0x68200000;
	}
	else  //ADD:0x6800 0000~681F FFFF	��һƬ
	{
	   Ram_temp= (u16*)0x68000000;
	}

	temp= *(Buf+1);
	temp<<=8;
	temp|= *(Buf+2);
	data_temp=temp;//(u16)(*Buf)<<8+(u16)(*(Buf+1));
	
	for(i=0;i<0xfffff;i++)	 //��Ƭȫ��ַ����
	{
		*Ram_temp=data_temp;
		data_temp0=	*Ram_temp;
		if(data_temp!=data_temp0)
		{
			//���Ϸ�������
		    status=1;//ʧ��
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
//				//���Ϸ�������
//			    status=1;//ʧ��
//				break;
//			}
//			 Ram_temp++;
//		}
//	}

	 //��������
	 Uart1SendData(0x11);  //ͷ
	 Uart1SendData(0x66);
	 Uart1SendData(0x08);  //�����
	 Uart1SendData(0x01);  //���ܺ�
	 Uart1SendData(0x03);  //����
	 Uart1SendData(status);  //����
	 Uart1SendData(data_temp0>>8); 
	 Uart1SendData(data_temp0); 
	 Uart1SendData(0x22);  //β
		
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

	 unixtime+=2; //�����ֶ��������2��
	
	 
	 RTC_Configuration();
 
     Time_Adjust();
	 BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
   unixtime_sendflag=1;//��ʾ��Ҫ���Ϸ���ʱ����Ϣ
}




//ָ�����
void  Command_Free(void)
{
	u8 Com_len=0;
	u8 i;
	u8 Data_Buf[256];//���ݻ���

	if(Rx1Buffer[Uart1_OutNum]==0xEE)//��ͷ�ж�
    {	   
	        Uart1_OutNum++;
			while(Uart1_InNum==Uart1_OutNum);                    
			if(Rx1Buffer[Uart1_OutNum]==0x99)
				Uart1_OutNum++;		
			else
		   	  return ;
		
			
			while(Uart1_InNum==Uart1_OutNum); //ȡ�����
			Command_ID=Rx1Buffer[Uart1_OutNum];
			Uart1_OutNum++;
			
			while(Uart1_InNum==Uart1_OutNum); //ȡ���ܺ�
			FunctionNum=Rx1Buffer[Uart1_OutNum];
			Uart1_OutNum++;

			while(Uart1_InNum==Uart1_OutNum); //ȡ�����
			Com_len=Rx1Buffer[Uart1_OutNum];
			Uart1_OutNum++;
			
			for(i=0;i<Com_len;i++)	//ȡ��Ч����
			{	
			    while(Uart1_InNum==Uart1_OutNum); 
			    Data_Buf[i]=Rx1Buffer[Uart1_OutNum];
			    Uart1_OutNum++;
			}
					
			while(Uart1_InNum==Uart1_OutNum); //�ж�β                   
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
				case 0x00: //����LED��
			     	Led_Test(FunctionNum,Com_len,Data_Buf);
				 break;

				 case 0x01: //����PSAM��1
				 	;
				 break;

				 case 0x02: //����PSAM��2
				 	;
				 break;

				 case 0x03: //����PSAM��3
				 	;
				 break;

				 case 0x04: //����PSAM��4
				 ;
				 break;

				 case 0x05: //���Է�����
				 	Beep_Test(FunctionNum,Com_len,Data_Buf);
				 break;

				 case 0x06: //����FLASH
					 
				   if(FunctionNum==0)
					 {
						 
				     FLASH_Test();
						 
					 }
					 else if(FunctionNum==3)
					 {	 
							sf_InitHard();	/* ��ʼ��SPIӲ�� */
							sf_EraseChip();
							Uart1SendData(0x11);  //ͷ
							Uart1SendData(0x66);
							Uart1SendData(0x06);  //�����
							Uart1SendData(0x01);  //���ܺ�
							Uart1SendData(0x0);  //����
							Uart1SendData(0x01);  //����
							Uart1SendData(0x06);  //����0x06 Flash�������!
							Uart1SendData(0x22);  //β
					 }
					 else
						 FLASH_WD_TEST(FunctionNum,Com_len,Data_Buf);
					 
					  
					 
				 break;

				 case 0x07: //����RS422�ӿ�
				 	RS422_Test(FunctionNum,Com_len,Data_Buf);
				 break;
						
				 case 0x08: //�����ⲿRAM
				//	 RAM_Test(FunctionNum,Com_len,Data_Buf);
				 break;

				 case 0x09 ://����ʵʱʱ��
				    
					   RTC_TEST(FunctionNum,Com_len,Data_Buf);

				 break;

				 case 0x0a ://����SD ��
				     ;
				 break;

				 case 0x0b://����洢������   ȡ��ʵʱʱ���ϴ�

					   ;
				 break;

				 case 0x0c://ȡ��ʵʱʱ���ϴ�
//				 	   if(unixtime_sendflag)
//					    {
								unixtime_sendflag=0;
								RTC_ITConfig(RTC_IT_SEC, DISABLE);	   	//�Ƚ�ֹʵʩʱ���жϣ��Ա�������						
//							}
//							else
//							{
//								unixtime_sendflag=1;
//								RTC_ITConfig(RTC_IT_SEC, ENABLE);	   
//							}								
					  
				 break;	 
				 case 0x0d://�¶ȴ���������	
			       if(FunctionNum) 
							  T18b20_sendflag=0;
					   if(!FunctionNum) 
							  T18b20_sendflag=1;
				 break;	 

				 case 0x0E://�������		 
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


