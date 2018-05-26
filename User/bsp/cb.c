
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "cb.h"
#include "gas.h"
#include "PLCMessage.h"
#include "SerialPort.h"
#include "dev_state.h"	
#include "flashmgr.h"	
#include "global.h"	
//#include "CRC16.H"


TaxRcvBuffer     TaxRBuffer[TAX_MAX];
TaxSndBuffer     TaxSBuffer[TAX_MAX];

u32 m_ProgramAddr = ApplicationAddress;
u32 SpiFlash_ProgramAddr=0;

u8 Header_Array[sizeof(struct Protocol_Header)+4]; //(33)协议帧头存储空间+4BYTE CRC
u8 FW_INFO_Array[sizeof(struct _FW_INFO)]; //(13)估计信息存储空间
u32 SERIAL_NUM=1; //流水号
u32 S_ERR,CRCERR;
u16 P_Rev=0x1122; //协议版本号
u8 FW_Rev[4]; //固件版本号
u8 FW_DLoad_Finish=0;
//u32 FW_Size;//固件大小
FW_INFO* fw_info=(FW_INFO*)FW_INFO_Array;
FW_UPDATA_INFO fw_updata_info;

u32 RX1err=0,RX2err=0,RX3err=0,RX4err=0;//抄报接收错误

/**报税口的全局变量***/
u8  DSumDataQuery_Flag=0;//日累计数据查询标志
u8  MSumDataQuery_Flag=0;//月累计数据查询标志
u8  TSumDataQuery_Flag=0;//总累计数据查询标志

DMA_Channel_TypeDef * TX_DMA_Channl;
DMA_Channel_TypeDef * RX_DMA_Channl;

u32 TxPtr[Gun_Sum+1];
u32 TxDlen[Gun_Sum+1];
u8  TxRecDlen[Gun_Sum+1];

u8 Gun_Query_Repeat;

u8 com_status[5];

u32 PLC_SEND_T;
u32 TX_Modulation;
u32 KEY_TXMOD[2];//调制模式和key

Com_Status CB_PORT[PORT_SUM+1];
Com_Gun_Binding Com_Gun_BindInfo[PORT_SUM+1];

//PLC_Status PLC_PORT;

Reset_Status PLC_PORT;
Reset_Status WTD_RESET;
Reset_Status PIN_RESET;
Reset_Status SOFT_RESET;

SystemDignose_Info sys_fault;

PLCSndBuffer PLC_SBuffer;
PLCRcvBuffer PLC_RBuffer;

void Protocol_Resp(struct Protocol_Header *protocol_head, u8 F_Sum,u8 F_Num, RES_ERR res)
{
	  u32 D_CRC;
	 
	  memcpy(protocol_head->Preamble,"PLCM",4);
		protocol_head->Com_len=sizeof(Header_Array);
	
	 if((protocol_head->Command_ID&0x8000)==0x8000)
		  protocol_head->Command_ID&=0x0FFF;
	 else
		  protocol_head->Command_ID&=0x1FFF;
	 
	 
		protocol_head->Frame_Num[1]=F_Sum; //总帧数
		protocol_head->Frame_Num[0]=F_Num; //当前帧号
	 
    protocol_head->Board_ID[0]=HW_ID[0]; 
		protocol_head->Board_ID[1]=HW_ID[1]; 
		protocol_head->Board_ID[2]=HW_ID[2]; 
		protocol_head->Board_ID[3]=HW_ID[3];
	 
		protocol_head->Res=res; 
		D_CRC=CRC_CalcBlockCRC((u32*)Header_Array,protocol_head->Com_len-4);
		*(u32*)(Header_Array+sizeof(struct Protocol_Header))=D_CRC;

	
		PLC_Data_Pre_Dlen=PLCBUF_LEN;//512
		res=genPLCData(Header_Array,sizeof(Header_Array),PLC_Data_Pre,&PLC_Data_Pre_Dlen);
		
		if(PLC_SBuffer.rptr == PLC_SBuffer.wptr)
		{
		    for(u16 i=0;i<PLC_Data_Pre_Dlen;i++)
				{
					PLC_SBuffer.data[PLC_SBuffer.wptr]=PLC_Data_Pre[i];
					PLC_SBuffer.wptr++;
					
					if(PLC_SBuffer.wptr==PLC_SND_BUFFER_SIZE)
				   PLC_SBuffer.wptr%=PLC_SND_BUFFER_SIZE;//1024
					
				}	
		}
		else
		{
			SEGGER_RTT_printf(0,"PLC_SBuf Busy!Data Don't Send!\r\n");
		//	bsp_Delay(10);
		}		

		PLC_Send_Enable();
	
	//	Uart2SendBuf(Header_Array,sizeof(Header_Array));
}


u8 Protocol_Send(struct Protocol_Header *protocol_head, u16 command_id, u8 F_Sum,u8 F_Num, u8* data, u16 length)
{
	    u32 D_CRC;
	    u8 *buf;
	
	    u8 res;
	    u8 F_Len=Pack_Head_LEN;
	
	    buf=(u8*)malloc(length+Pack_Head_Tail_LEN);
	    if(buf==0)
	    {
			  SEGGER_RTT_printf(0,"memory error!%s:%d\r\n", __FILE__,__LINE__);
		      return MEM_ERR;
		  }
		
	//	F_Len=sizeof(struct Protocol_Header)-1;//帧头长度去掉1字节返回值
		
    if(command_id==TEMP_Data)
		   protocol_head->Serial_Num=0;//流水号	
		else
			 protocol_head->Serial_Num=SERIAL_NUM;//流水号
		
		
		memcpy(protocol_head->Preamble,"PLCM",4);
		protocol_head->Com_len=length+Pack_Head_Tail_LEN;//帧总长度，包含帧头和校验
		protocol_head->Command_ID=command_id;
		
		protocol_head->Protocol_Rev=P_Rev;  //协议版本号
		protocol_head->Frame_Num[1]=F_Sum; //总帧数
		protocol_head->Frame_Num[0]=F_Num; //当前帧号
		
		protocol_head->Board_ID[0]=HW_ID[0]; 
		protocol_head->Board_ID[1]=HW_ID[1]; 
		protocol_head->Board_ID[2]=HW_ID[2]; 
		protocol_head->Board_ID[3]=HW_ID[3];
	
		memcpy(buf,(u8*)protocol_head,F_Len);//帧头部分
		memcpy(buf+F_Len,data,length);//报文部分

		D_CRC=CRC_CalcBlockCRC((u32*)buf,protocol_head->Com_len-4);//校验不含最后4字节校验位
	 // CRC_ResetDR();
		//memcpy(buf+protocol_head->Com_len-4,(u8*)&D_CRC,4);//校验部分
		*(u32*)(buf+protocol_head->Com_len-4)=D_CRC;
	
     		
		PLC_Data_Pre_Dlen=PLCBUF_LEN;//512
		res=genPLCData(buf,protocol_head->Com_len,PLC_Data_Pre,&PLC_Data_Pre_Dlen);
		
		
		if(!res) 
		{	
			if(PLC_SBuffer.rptr == PLC_SBuffer.wptr)
			{
				//memcpy(&PLC_SBuffer.data[PLC_SBuffer.wptr],PLC_Data_Pre,PLC_Data_Pre_Dlen);
				for(u16 i=0;i<PLC_Data_Pre_Dlen;i++)
				{
					PLC_SBuffer.data[PLC_SBuffer.wptr]=PLC_Data_Pre[i];
					PLC_SBuffer.wptr++;
					
					if(PLC_SBuffer.wptr==PLC_SND_BUFFER_SIZE)
				    PLC_SBuffer.wptr=0;//1024
					
				}		
			
			}
			else
			{
				SEGGER_RTT_printf(0,"PLC_SBuf Busy!Data Don't Send!\r\n");
			}
			
			PLC_Send_Enable();
		}		
    else
		{
				SEGGER_RTT_printf(0,"PLC_SBuf Generate Fail!\r\n");
		}			

	//	SERIAL_NUM++;  //接收到相同流水号应答后再增加
		free(buf);
		SEGGER_RTT_printf(0,"SERIAL_NUM:%d ,unixtime:%d \r\n", SERIAL_NUM,unixtime);
		return P_OK;
			
}

void PLC_BufIndex_Add(u32 pos)
{
	
	PLC_RBuffer.rptr+=pos;
	PLC_RBuffer.rptr%=PLC_RCV_BUFFER_SIZE;
	
}

//指令解析
u8 Protocol_Free(u8 *P_Data)
{
	struct tm *ASCTIME;
	
	  u16 Data_len;//总长度
		u8  res=0;
		u8  SUM_F,NUM_F;
		u32 Data_CRC,Data_CRC_r;
		u16 i;
		struct Protocol_Header *protocol_header;	 
	  u32 S_Num;//流水号
	  static u8 F_OK=0;//已经确认完成的包	
	
		u8 *PLC_Send_Static;
		u8 *PLC_Send_Static_T;
		u16 data_size;
	  u32 Send_int=0;		
	 		
	  u8 Data_Buf[4096];//数据缓冲
	  u8 readsector;

		u32 spiFlash_Raddr=0;
		u32 intFlash_Waddr=0;
		u32 intFlash_BaseAddr;
		static u32 NewFwCRC=0;
		static u32 NewFwSize=0;
		u8  PING=0;
	  		
		protocol_header=(struct Protocol_Header *)Header_Array;
		
		memcpy(protocol_header,(struct Protocol_Header *)P_Data,Pack_Head_LEN);	
		

		Data_len=	protocol_header->Com_len; //获得帧总长度
	  if(Data_len==0)
			 return DataLen_ERR;
		
		P_Rev= protocol_header->Protocol_Rev;//获得协议版本号

		SUM_F= protocol_header->Frame_Num[1];//总帧数
		NUM_F= protocol_header->Frame_Num[0];//当前帧号
	
	  S_Num=protocol_header->Serial_Num;//接受到的流水号
		
		Data_CRC=CRC_CalcBlockCRC((u32*)P_Data,Data_len-4);//4字节CRC
		
		Data_CRC_r=*(u32*)(P_Data+Data_len-4);
//		Data_CRC_r<<=8;
//		Data_CRC_r|=P_Data[Data_len-2];
		
		if(Data_CRC==Data_CRC_r)
		{
			for(i=0;i<Data_len-Pack_Head_Tail_LEN;i++)	//取有效数据
			{			 
				Data_Buf[i]=P_Data[Pack_Head_LEN+i];		
			}	

		}
		else
		{
				SEGGER_RTT_printf(0,"CRC err %s:%d\r\n", __FILE__,__LINE__);
		
				Protocol_Resp(protocol_header,1,1,CRC_ERR);
			  CRCERR++;
			  return CRC_ERR;
			
		}				
		
		if((protocol_header->Command_ID&0x8000)==0x8000)
		{
			if(S_Num!=0)
			{
				if(S_Num!=SERIAL_NUM)
				{
					SEGGER_RTT_printf(0,"SERIAL_NUM err!--- S_Num:%d, SERIAL_NUM:%d \r\n", S_Num,SERIAL_NUM);
					S_ERR++;
					return S_Num_ERR;
				}
		  }
				
		}
		

		switch(protocol_header->Command_ID)
		{    
		
			case FW_Updata_Notice: //固件新版本通知
									
			    memcpy((u8*)&fw_updata_info,Data_Buf,sizeof(fw_updata_info));		
					
			    if(fw_updata_info.Mode==1)//进入升级流程
					{   			 							
						bsp_StartTimer(Upgrade_OutTimer,10000);//升级超时定时器，10s收不到数据则退出升级流程
						
						if(*(u8*)flashSectionStruct[FLASH_SECTION_PINGPONG_SEL].base==0xFF)//ping区
						{
								Protocol_Resp(protocol_header,1,1,1);//应答返回值1，请求PONG区的固件		
								NewFwCRC=fw_updata_info.CRC_Pong;
								NewFwSize=fw_updata_info.Size_Pong;
						}						
						else
						{
							 Protocol_Resp(protocol_header,1,1,0);//应答返回值0，请求PING区的固件
							 NewFwCRC=fw_updata_info.CRC_Ping;	
							 NewFwSize=fw_updata_info.Size_Ping;						
						}						
					
					
						SEGGER_RTT_printf(0,"NEW FW size:%dByte,CRC:0x%08X\r\n",NewFwSize,NewFwCRC);
						
						PLC_SW=0;	//关闭上传
						setDevState(DEV_STATE_UPGRADE);
						TIM_Cmd(TIM2,DISABLE); 
						SpiFlash_ProgramAddr=0;
						F_OK=0;
					}
					else//退出升级流程
					{
						PLC_SW=1;	//开启上传
						setDevState(DEV_STATE_IDLE);
						TIM_Cmd(TIM2,ENABLE); 
						SpiFlash_ProgramAddr=0;
						F_OK=0;
						SEGGER_RTT_printf(0,"Exit FW Upgrade!\r\n");
						Protocol_Resp(protocol_header,1,1,2);//应答返回值2，退出升级
					}
						
			
					
			 break;    
			
					
			case FW_Updata_Data: //固件升级包
				
					
		      bsp_StartTimer(Upgrade_OutTimer,10000);//升级超时定时器，10s收不到数据则退出升级流程
			    //存储数据.....		    
					if(F_OK<NUM_F)	
					{					
						
						SEGGER_RTT_printf(0,"%d--%d,NewFWSize:%dByte,CRC:0x%08X\r\n",NUM_F,SUM_F,NewFwSize,NewFwCRC);
						
						res=uf_WriteBuffer(FLASH_SST25VF016B,Data_Buf,SpiFlash_ProgramAddr,Data_len-Pack_Head_Tail_LEN);
						 if(res==0)
						 {
								bsp_DelayMS(200);
							  res=uf_WriteBuffer(FLASH_SST25VF016B,Data_Buf,SpiFlash_ProgramAddr,Data_len-Pack_Head_Tail_LEN);
							  if(res==0)
								{
									SEGGER_RTT_printf(0,"spiFlash Write Fail!\r\n");
									return 0;
								}
						 }
						 
							SpiFlash_ProgramAddr+=Data_len-Pack_Head_Tail_LEN;
							F_OK=NUM_F;
						 
					} 		
									 
					if(SUM_F==NUM_F)	//固件接受完成，flash写入完成，发送升级结果
					{	
						 SEGGER_RTT_printf(0,"%d--%d,NewFWSize:%dByte,CRC:0x%08X\r\n",NUM_F,SUM_F,NewFwSize,NewFwCRC);																 
						 SEGGER_RTT_printf(0,"FW Receive Finished!\r\n");
						 
						
						 readsector=NewFwSize/SF_PAGE_SIZE;
																
						//写入内部flash
						{
								spiFlash_Raddr=0;
								
								if(*(u8*)flashSectionStruct[FLASH_SECTION_PINGPONG_SEL].base==0xFF)//ping区
								{
										intFlash_Waddr=flashSectionStruct[FLASH_SECTION_FW_PONG].wptr+0x800;	
										intFlash_BaseAddr=flashSectionStruct[FLASH_SECTION_FW_PONG].base+0x800;									
                    PING=0xFF;					
									
								}
								else//pong区
								{
										intFlash_Waddr=flashSectionStruct[FLASH_SECTION_FW_PING].wptr+0x800;
									  intFlash_BaseAddr=flashSectionStruct[FLASH_SECTION_FW_PING].base+0x800;	
									  PING=0;	
										
								}	
								
								for(i=0;i<readsector;i++)
								{
									 uf_ReadBuffer(FLASH_SST25VF016B,Data_Buf,spiFlash_Raddr,SF_PAGE_SIZE);	
									 spiFlash_Raddr+=SF_PAGE_SIZE;
									 res=uf_WriteBuffer(FLASH_STM32F103RET6,Data_Buf,intFlash_Waddr,SF_PAGE_SIZE);
									 if(res==0)
									 {
										 bsp_DelayMS(200);
										 res=uf_WriteBuffer(FLASH_STM32F103RET6,Data_Buf,intFlash_Waddr,SF_PAGE_SIZE);
										 if(res==0)
										 {
												SEGGER_RTT_printf(0,"Inter Flash Write Fail!\r\n");
												return 0;
										 }
									 }
									 
									 intFlash_Waddr+=SF_PAGE_SIZE;
									
									
								}
								
								if(NewFwSize%SF_PAGE_SIZE)
								{
										uf_ReadBuffer(FLASH_SST25VF016B,Data_Buf,spiFlash_Raddr,NewFwSize%SF_PAGE_SIZE);	
										spiFlash_Raddr+=NewFwSize%SF_PAGE_SIZE;
										 res=uf_WriteBuffer(FLASH_STM32F103RET6,Data_Buf,intFlash_Waddr,NewFwSize%SF_PAGE_SIZE);
										 if(res==0)
										 {
											 bsp_DelayMS(200);
											 res=uf_WriteBuffer(FLASH_STM32F103RET6,Data_Buf,intFlash_Waddr,SF_PAGE_SIZE);
											 if(res==0)
											 {
													SEGGER_RTT_printf(0,"Inter Flash Write Fail!\r\n");
													return 0;
											 }
										 }
										 
										 intFlash_Waddr+=SF_PAGE_SIZE;
								}
																				
									
							}
						
							
						  Data_CRC=CRC_CalcBlockCRC((u32*)intFlash_BaseAddr,NewFwSize);
							
						  //校验通过............
							if(Data_CRC==NewFwCRC)
						  {
							  memcpy(fw_info->MagicCode,"firm",4);
								memcpy(fw_info->MajorVersion,fw_updata_info.FW_Rev,4);
								fw_info->Size=NewFwSize;		
								fw_info->Crc=Data_CRC;	
								
								if(PING==0xFF)//ping区
								{
									res=uf_WriteBuffer(FLASH_STM32F103RET6,(u8*)fw_info,flashSectionStruct[FLASH_SECTION_FW_PONG].base,sizeof(FW_INFO));				
								}
								else//pong区
								{
									res=uf_WriteBuffer(FLASH_STM32F103RET6,(u8*)fw_info,flashSectionStruct[FLASH_SECTION_FW_PING].base,sizeof(FW_INFO));
								}	
							
							
							   if(res==0)
								 {
										bsp_DelayMS(200);
									 
										if(PING==0xFF)//ping区
											res=uf_WriteBuffer(FLASH_STM32F103RET6,(u8*)fw_info,flashSectionStruct[FLASH_SECTION_FW_PONG].base,sizeof(FW_INFO));										
										else//pong区										
											res=uf_WriteBuffer(FLASH_STM32F103RET6,(u8*)fw_info,flashSectionStruct[FLASH_SECTION_FW_PING].base,sizeof(FW_INFO));
											
										 if(res==0)
										 {
												SEGGER_RTT_printf(0,"Firmware Info Write Fail!\r\n");
												return 0;
										 }
																			 
								 }
								 
								PING=~PING;//PING<-->PONG切换
							
								res=uf_WriteBuffer(FLASH_STM32F103RET6,&PING,flashSectionStruct[FLASH_SECTION_PINGPONG_SEL].base,1);								
								if(res==0)
								{
										bsp_DelayMS(200);
										res=uf_WriteBuffer(FLASH_STM32F103RET6,&PING,flashSectionStruct[FLASH_SECTION_PINGPONG_SEL].base,1);								
										if(res==0)
										{
											SEGGER_RTT_printf(0,"PING<->PONG Change Fail!\r\n");
											return 0;
										}
										
								}	
								
								Protocol_Resp(protocol_header,SUM_F,NUM_F,P_OK);//最后一帧的应答
								
								SEGGER_RTT_printf(0,"Upgrade Ok! SYSTERM REBOOT...\r\n");	
								bsp_DelayMS(3000);									 
								NVIC_SystemReset();
															 
							}	
							else
							{
								SEGGER_RTT_printf(0,"FW CRC ERROR!CalcCRC=0x%08X,NewFwCRC=0x%08X\r\n",Data_CRC,NewFwCRC);	
								setDevState(DEV_STATE_IDLE);
			          TIM_Cmd(TIM2,ENABLE); 
								
							}								
						
					}
					  
					  bsp_DelayMS(100);//等待Flash写入完成
						Protocol_Resp(protocol_header,SUM_F,NUM_F,P_OK);//除最后一帧的应答
					
				 break;
		
					 
			case TAX_RESP: //税务信息应答
				 SEGGER_RTT_printf(0,"Get TAX Info Answer!\r\n");
	
				 res=LocalData_Old(FlashReadPtrBuf[data_flashaddr_index]);
				 if(!res)
				 {
					 SEGGER_RTT_printf(0,"Data is olded!\r\n");
					 SERIAL_NUM++;
					 
					 if(SERIAL_NUM==0)
						 SERIAL_NUM=1;
					 
					 data_flashaddr_index++;
					 FlashReadPtrNum--;
					 Data_Upload_Num++;
				//	 PLC_Send_Fail_Cnt--;
					  PLC_Send_Fail_Cnt=0;
					/**测试代码**/ 
					 if(Send_FX==1) 
						 Send_F1++;	
					 else if(Send_FX==2) 
						 Send_F2++;
					 else
						 Send_F3++;
					 
					   Send_FX=0;
					 /**测试代码**/ 
				 }
				 else
					 SEGGER_RTT_printf(0,"Data isn't olded!\r\n");
			
			break;
			case OTD_RESP: 
				
			   SEGGER_RTT_printf(0,"Get OTD data Answer!---RespTime:%dms\r\n",SYSTEMTICK-PLC_Send_T3);
			 	 
				 res=LocalData_Old(FlashReadPtrBuf[data_flashaddr_index]);
				 if(!res)
				 {
					 SEGGER_RTT_printf(0,"Data is olded!\r\n");
					 SERIAL_NUM++;  
					 if(SERIAL_NUM==0)
						 SERIAL_NUM=1;
									 
					 data_flashaddr_index++;
					 FlashReadPtrNum--;
					 Data_Upload_Num++;
					// PLC_Send_Fail_Cnt--;
					 PLC_Send_Fail_Cnt=0;
					 /**测试代码**/  
					 if(Send_FX==1) 
						 Send_F1++;	
					 else if(Send_FX==2) 
						 Send_F2++;
					 else
						 Send_F3++;
					 
					   Send_FX=0;
					 /**测试代码**/ 
				 }
				 else
					 SEGGER_RTT_printf(0,"Data isn't olded!\r\n");
				 
			break;
				case DSD_RESP: 
					
				 SEGGER_RTT_printf(0,"Get DSD Data Answer!\r\n");
				 res=LocalData_Old(FlashReadPtrBuf[data_flashaddr_index]);
				 if(!res)
				 {
					 SEGGER_RTT_printf(0,"Data is olded!\r\n");
					 SERIAL_NUM++;
					 if(SERIAL_NUM==0)
						 SERIAL_NUM=1;
					 
					 data_flashaddr_index++;
					 FlashReadPtrNum--;
					 Data_Upload_Num++;
				//	 PLC_Send_Fail_Cnt--;
					  PLC_Send_Fail_Cnt=0;
					 
					  /**测试代码**/  
					 if(Send_FX==1) 
						 Send_F1++;	
					 else if(Send_FX==2) 
						 Send_F2++;
					 else
						 Send_F3++;
					 
					   Send_FX=0;
					 /**测试代码**/ 
				 }
				 else
					 SEGGER_RTT_printf(0,"Data isn't olded!\r\n");
			
			break;
			case MSD_RESP: 
				 SEGGER_RTT_printf(0,"Get MSD Data Answer!\r\n");
				 
				 res=LocalData_Old(FlashReadPtrBuf[data_flashaddr_index]);
				 if(!res)
				 {
					 SEGGER_RTT_printf(0,"Data is olded!\r\n");
					 SERIAL_NUM++;
					 if(SERIAL_NUM==0)
						 SERIAL_NUM=1;
					 
					 data_flashaddr_index++;
					 FlashReadPtrNum--;
					 Data_Upload_Num++;
					// PLC_Send_Fail_Cnt--;
					  PLC_Send_Fail_Cnt=0;
					  /**测试代码**/  
					 if(Send_FX==1) 
						 Send_F1++;	
					 else if(Send_FX==2) 
						 Send_F2++;
					 else
						 Send_F3++;
					 
					   Send_FX=0;
					 /**测试代码**/ 
				 }
				 else
					 SEGGER_RTT_printf(0,"Data isn't olded!\r\n");
				 
			break;
			case TSD_RESP: 
				 SEGGER_RTT_printf(0,"Get TSD Data Answer!\r\n");
				 
				 res=LocalData_Old(FlashReadPtrBuf[data_flashaddr_index]);
				 if(!res)
				 {
					 SEGGER_RTT_printf(0,"Data is olded!\r\n");
					 SERIAL_NUM++;
					 if(SERIAL_NUM==0)
						 SERIAL_NUM=1;
					 
					 data_flashaddr_index++;
					 FlashReadPtrNum--;
					 Data_Upload_Num++;
					// PLC_Send_Fail_Cnt--;
					  PLC_Send_Fail_Cnt=0;
					  /**测试代码**/  
					 if(Send_FX==1) 
						 Send_F1++;	
					 else if(Send_FX==2) 
						 Send_F2++;
					 else
						 Send_F3++;
					 
					   Send_FX=0;
					 /**测试代码**/ 
				 }
				 else
					 SEGGER_RTT_printf(0,"Data isn't olded!\r\n");
				 
			break;
				 
			case  FAULT_RESP:
				 SEGGER_RTT_printf(0,"Get Fault Message Answer!\r\n");
				 
				 res=LocalData_Old(FlashReadPtrBuf[data_flashaddr_index]);
				 if(!res)
				 {
					 SEGGER_RTT_printf(0,"Data is olded!\r\n");
					 SERIAL_NUM++;
					 if(SERIAL_NUM==0)
						 SERIAL_NUM=1;
					 
					 data_flashaddr_index++;
					 FlashReadPtrNum--;
					 Data_Upload_Num++;
					 PLC_Send_Fail_Cnt--;
				 }
				 else
					 SEGGER_RTT_printf(0,"Data isn't olded!\r\n");
				break;

			case  TEMP_RESP:
				 SEGGER_RTT_printf(0,"Get Temperature Answer! PLC Response Time:%dms\r\n",SYSTEMTICK-PLC_SEND_T);
			  // T_upload_cnt--;
			     T_upload_cnt=0;
			    // SERIAL_NUM++;
			 
				//	PLC_Send_Fail_Cnt--;

			break;
			
			case  FWVER_RESP:
				 SEGGER_RTT_printf(0,"Get FW REV Message Answer!\r\n");
			   SERIAL_NUM++;
			   setPlcState(PLC_STATE_CONNECT);
		//		 PLC_Send_Fail_Cnt--;
			   PLC_Data_Send(FW_VER,4,Time_Data,1,1);//发送时间同步请求
			
			   if(getRTCState()!=RTC_STATE_SYNCED)
			     setRTCState(RTC_STATE_SYNCING);
				 
			break;

			case  TIME_SYNC:					
							
					unixtime= *(u32*)Data_Buf;
					RTC_Config();
			    
			   	t_stamp=unixtime+28800;//补8小时时差
			    ASCTIME=localtime((time_t *)&t_stamp);
			
			    CB_Year=ASCTIME->tm_year;
					CB_Month=ASCTIME->tm_mon;
					CB_Day=ASCTIME->tm_mday;
			
					sprintf(CB_Time.year,"%4d",ASCTIME->tm_year+1900);//设置全局时间变量
					sprintf(CB_Time.month,"%2d",ASCTIME->tm_mon+1);
					sprintf(CB_Time.day,"%2d",ASCTIME->tm_mday);
					sprintf(CB_Time.hour,"%2d",ASCTIME->tm_hour);
					sprintf(CB_Time.minute,"%2d",ASCTIME->tm_min);
			     
			    SEGGER_RTT_printf(0,"Get Time Stamp:%d,	Time Synchronized!\r\n",unixtime);
				//	Protocol_Resp(protocol_header,SUM_F,NUM_F,P_OK);//应答
				//	RTC_SYS_Flag=1;
			
			    setRTCState(RTC_STATE_SYNCED);
					
			 //   PLC_Send_Fail_Cnt--;
			    FWrev_upload_cnt=0;
			    SERIAL_NUM++;
			    SystemDignose();//系统诊断
			//		Price_Calibration(1000);
				 
			break;
			
			case  PLC_Send_Int:	
						  
				 Send_int= *(u32*)Data_Buf;
			
			   SEGGER_RTT_printf(0,"Get PLC_send_intval command! Send_interval=%d\r\n",Send_int);
				//  RTC_Config();
					Protocol_Resp(protocol_header,SUM_F,NUM_F,P_OK);//应答
			   					
			   // bsp_DelayMS(1000);
						    
			    if(Send_int==0xFFFF)
					{			
						
						 SYSTEM_RESET(Manual_Reset);	
						
					}		
					else if(Send_int==0xFFFE)//开始PLC上传
					{			
							PLC_SW=1;
						 
					}			
					else if(Send_int==0xFFFD)//停止PLC上传
					{			
							PLC_SW=0;
						 
					}
					else if(Send_int==0xFFFC)
					{
						// sf_EraseSector(SpiFlash_DataDET_ADDR);
						 uf_EraseSector(FLASH_SST25VF016B,flashSectionStruct[FLASH_SECTION_STATISTICS_DATA].base);
						 SEGGER_RTT_printf(0,"spiFlash SEND COUNTER CLEARED!!!------ \r\n");
						
						// res=FLASH_ProgramStart(Systeminfo_Address,sizeof(struct _SystemDignose_Info));	
						 uf_EraseSector(FLASH_STM32F103RET6,flashSectionStruct[FLASH_SECTION_SYSTEM_INFO].base);
						 bsp_DelayMS(1000);
						 if(res==4)
						 {	
							  SEGGER_RTT_printf(0,"intFlash record info cleared!!!------ \r\n");
						 }
						 
						 PLC_Send_Cnt=0;
						 PLC_Send_Fail_Cnt=0;
						 Send_F1=0;
						 Send_F2=0;
						 Send_F3=0;
						 
						 memset((u8*)&PLC_PORT,0,sizeof(Reset_Status));
						 memset((u8*)&WTD_RESET,0,sizeof(Reset_Status));
						 memset((u8*)&SOFT_RESET,0,sizeof(Reset_Status));
						 memset((u8*)&PIN_RESET,0,sizeof(Reset_Status));
						 
					}
					else if(Send_int==0xFFFB)//单价查询请求
					{
						//	Price_Calibration(1000);
						
							for(i=1;i<=4;i++)
							{
//							  CB_PORT[i].Gun_A.TSD_Same_Cnt=0;
//							  CB_PORT[i].Gun_B.TSD_Same_Cnt=0;
								
							  CB_PORT[i].Gun_A.Price_Query_Flag=1;
							  CB_PORT[i].Gun_B.Price_Query_Flag=1;
							}
						
                  }
					
				  if(Send_int<=60000)				
					{
						 key=Send_int;						
						 uf_ReadBuffer(FLASH_SST25VF016B,(u8*)KEY_TXMOD,flashSectionStruct[FLASH_SECTION_PLCSETUP_DATA].base,sizeof(KEY_TXMOD));
						
						 KEY_TXMOD[0]=key;
						 			 
						 uf_WriteBuffer(FLASH_SST25VF016B,(u8*)KEY_TXMOD,flashSectionStruct[FLASH_SECTION_PLCSETUP_DATA].base,sizeof(KEY_TXMOD));
									
					}
					
				 //PLC_Send_Fail_Cnt--;
			break;
			case PLC_TX_Modulation_Set:
				  
				  TX_Modulation=*(u32*)Data_Buf;
			    if(TX_Modulation>=0 || TX_Modulation<=3)				
					{				
					   uf_ReadBuffer(FLASH_SST25VF016B,(u8*)KEY_TXMOD,flashSectionStruct[FLASH_SECTION_PLCSETUP_DATA].base,sizeof(KEY_TXMOD));
						
						 KEY_TXMOD[1]=TX_Modulation;
						
						 uf_WriteBuffer(FLASH_SST25VF016B,(u8*)KEY_TXMOD,flashSectionStruct[FLASH_SECTION_PLCSETUP_DATA].base,sizeof(KEY_TXMOD));
												
					}
			    Protocol_Resp(protocol_header,SUM_F,NUM_F,P_OK);//应答
					
      break;		
					
	    case PLC_TX_Modulation_Get:
					
			     uf_ReadBuffer(FLASH_SST25VF016B,(u8*)KEY_TXMOD,flashSectionStruct[FLASH_SECTION_PLCSETUP_DATA].base,sizeof(KEY_TXMOD));
						
			     TX_Modulation=KEY_TXMOD[1];
			     
			     PLC_Data_Send((u8*)&TX_Modulation, sizeof(TX_Modulation), PLC_TX_Modulation_Get&0x1FFF,1,1);//发送
      break;
					
			case 	PLC_Send_Query:
				 
			  //  PLC_SW=0;//先停止上传数据
			    data_size=sizeof(key)+sizeof(Send_Cnt_F)+(sizeof(Reset_Status)*4);
			
			    PLC_Send_Static=(u8*)malloc(data_size);
			    PLC_Send_Static_T=PLC_Send_Static;
			
			    if(PLC_Send_Static!=0)
					{
						
						Send_Cnt_F.PLC_Send_Sum=PLC_Send_Cnt;
						Send_Cnt_F.PLC_Send_Fail_Sum=PLC_Send_Fail_Cnt;
						Send_Cnt_F.PLC_Send_F1=Send_F1;
						Send_Cnt_F.PLC_Send_F2=Send_F2;
						Send_Cnt_F.PLC_Send_F3=Send_F3;
						Send_Cnt_F.PLC_Send_OK_Sum=Send_F1+Send_F2+Send_F3;


						memcpy(PLC_Send_Static,(u8*)&key,sizeof(key));
						PLC_Send_Static+=sizeof(key);

						memcpy(PLC_Send_Static,(u8*)&Send_Cnt_F,sizeof(Send_Cnt_F));
						PLC_Send_Static+=sizeof(Send_Cnt_F);

						memcpy(PLC_Send_Static,(u8*)&PLC_PORT,sizeof(Reset_Status));
						PLC_Send_Static+=sizeof(Reset_Status);

						memcpy(PLC_Send_Static,(u8*)&WTD_RESET,sizeof(Reset_Status));
						PLC_Send_Static+=sizeof(Reset_Status);

						memcpy(PLC_Send_Static,(u8*)&SOFT_RESET,sizeof(Reset_Status));
						PLC_Send_Static+=sizeof(Reset_Status);

						memcpy(PLC_Send_Static,(u8*)&PIN_RESET,sizeof(Reset_Status));
						//	PLC_Send_Static+=sizeof(Reset_Status);


						PLC_Data_Send(PLC_Send_Static_T, data_size, PLC_Send_Query&0x1FFF,1,1);//发送
						if(getRTCState()==RTC_STATE_SYNCED)  
						   uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Send_Cnt_F,flashSectionStruct[FLASH_SECTION_STATISTICS_DATA].base,sizeof(Send_Cnt_F));
						else
							 SEGGER_RTT_printf(0,"RTC Need Sync!Can't Return Statics!\r\n", __FILE__,__LINE__);
						
					}
			    else
					{
							SEGGER_RTT_printf(0,"MEM err %s:%d\r\n", __FILE__,__LINE__);
				      return MEM_ERR;
					}
									
			    free(PLC_Send_Static_T);
				
			break;
		}
							
							
			return P_OK;	
}			


void DMA_Configuration(DMA_Channel_TypeDef* Channel,u32 BufferSize, \
	                     u32 PeripheralBaseAddr,u32 MemoryBaseAddr,u32 Dir,u32 Priority)
{
    DMA_InitTypeDef DMA_InitStructure;
    /* DMA clock enable */
  //  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//DMA1
    
    /* DMA1 Channel4 (triggered by USART1 Tx event) Config */
    DMA_DeInit(Channel);  
    DMA_InitStructure.DMA_PeripheralBaseAddr = PeripheralBaseAddr;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)MemoryBaseAddr;
    DMA_InitStructure.DMA_DIR = Dir;
    DMA_InitStructure.DMA_BufferSize = BufferSize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
 // DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//DMA_Mode_Normal;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = Priority;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(Channel, &DMA_InitStructure);
    
	 if(Dir==DMA_DIR_PeripheralSRC)
	 {
		 DMA_ITConfig(Channel, DMA_IT_TC, ENABLE);
     DMA_ITConfig(Channel, DMA_IT_TE, ENABLE);
	 }
		
		
		
}
				

void SYSTEM_RESET(u16 FaultNum)
{
	
  u8 res;

	memcpy((u8*)&sys_fault,(u8*)(flashSectionStruct[FLASH_SECTION_SYSTEM_INFO].base),sizeof(struct _SystemDignose_Info));
	
	sys_fault.Fault_Num=FaultNum;
	sys_fault.TimeStamp=RTC_GetCounter();

	sys_fault.com1=CB_PORT[1];
	sys_fault.com2=CB_PORT[2];
	sys_fault.com3=CB_PORT[3];
	sys_fault.com4=CB_PORT[4];
	
	//PLC重连次数
	if(getRTCState()==RTC_STATE_SYNCED)//时间同步过，说明SOFT_RESET变量值已经是历史累计数据
	{	
		
		sys_fault.com5.counter=PLC_PORT.counter;	
		
		
			SOFT_RESET.counter++;
			sys_fault.soft_reset.counter=SOFT_RESET.counter;	
		
		
	}
	else//与集中器没有进行时间同步
	{
		if(sys_fault.com5.counter==0xFFFFFFFF)	
			sys_fault.com5.counter=PLC_PORT.counter;	
		else
			sys_fault.com5.counter+=PLC_PORT.counter;	
		
					
			SOFT_RESET.counter++;
		
		if(sys_fault.soft_reset.counter==0xFFFFFFFF)	
			sys_fault.soft_reset.counter=SOFT_RESET.counter;	
		else
			sys_fault.soft_reset.counter+=SOFT_RESET.counter;	
				
		 
	}
	
	if(FaultNum==PLC_CONNECT_FAULT)		//是PLC引起的软复位，则记录时间
	{		
		sys_fault.com5.last_reset_time[(sys_fault.com5.counter-1)%10]=sys_fault.TimeStamp;	
	 
	}
	 //软复位次数	  
	  sys_fault.soft_reset.last_reset_time[(sys_fault.soft_reset.counter-1)%10]=sys_fault.TimeStamp;
	
	
	//WTD,PIN复位信息在系统诊断中已经记录进flash
//	sys_fault.watchdog=WTD_RESET;
//	sys_fault.pin_reset=PIN_RESET;
//	sys_fault.soft_reset=SOFT_RESET;

	 res=uf_WriteBuffer(FLASH_STM32F103RET6,(u8*)&sys_fault,flashSectionStruct[FLASH_SECTION_SYSTEM_INFO].base,sizeof(struct _SystemDignose_Info));
	
	 if(res)
	 {
		 SEGGER_RTT_printf(0,"System info have storaged,system reboot...\r\n");
	 }
	 	
	 PLC_PWR_OFF;
	 
	 bsp_DelayMS(1000);
	 
	 //存储PLC发送统计信息
	 if(getRTCState()==RTC_STATE_SYNCED)//说明抄报与集中器时间同步过
	 {
			Send_Cnt_F.PLC_Send_Sum=PLC_Send_Cnt;
			Send_Cnt_F.PLC_Send_Fail_Sum=PLC_Send_Fail_Cnt;
			Send_Cnt_F.PLC_Send_F1=Send_F1;
			Send_Cnt_F.PLC_Send_F2=Send_F2;
			Send_Cnt_F.PLC_Send_F3=Send_F3;
			Send_Cnt_F.PLC_Send_OK_Sum=Send_F1+Send_F2+Send_F3;

			uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Send_Cnt_F,flashSectionStruct[FLASH_SECTION_STATISTICS_DATA].base,sizeof(Send_Cnt_F));
			bsp_DelayMS(2000);

			PLC_PWR_OFF; 	
			bsp_DelayMS(10000);
			// Fault_Info_Storage(&SpiFlash_DataStorageAddr,FaultNum);	
			NVIC_SystemReset();
	 }
	 else
	 {
		    PLC_PWR_OFF; 	
				bsp_DelayMS(10000);
				NVIC_SystemReset();

	 }

}

void CB_Init(void)
{
	u8 i,j;

	for(i=1;i<=4;i++)
	{
		CB_PORT[i].COM_Num=i;
		
		CB_PORT[i].TaxDataQuery_Flag=0;
		
		CB_PORT[i].Gun_A.DSumDataQuery_Flag=0;
		CB_PORT[i].Gun_A.MSumDataQuery_Flag=0;
		CB_PORT[i].Gun_A.TSumDataQuery_Flag=1;
		
		CB_PORT[i].Gun_B.DSumDataQuery_Flag=0;
		CB_PORT[i].Gun_B.MSumDataQuery_Flag=0;
		CB_PORT[i].Gun_B.TSumDataQuery_Flag=1;
		
		CB_PORT[i].Gun_A.Fail_Cnt=0;
		CB_PORT[i].Gun_A.NoData_Time=0;
		
		CB_PORT[i].Gun_B.Fail_Cnt=0;
		CB_PORT[i].Gun_B.NoData_Time=0;
		
		CB_PORT[i].Gun_A.OnQuery_Flag=0;
		CB_PORT[i].Gun_B.OnQuery_Flag=0;
		
		CB_PORT[i].Gun_A.MSD_Same_Cnt=0;
		CB_PORT[i].Gun_B.MSD_Same_Cnt=0;
		
		CB_PORT[i].Gun_A.TSD_Same_Cnt=0;
		CB_PORT[i].Gun_B.TSD_Same_Cnt=0;
		
		CB_PORT[i].Gun_A.Price_Same_Cnt=0;
		CB_PORT[i].Gun_B.Price_Same_Cnt=0;
		
		CB_PORT[i].Gun_A.Price_Query_Flag=0;
		CB_PORT[i].Gun_B.Price_Query_Flag=0;
			
		CB_PORT[i].Gun_A.Gun_State=GUN_STATE_IDLE;
		CB_PORT[i].Gun_B.Gun_State=GUN_STATE_IDLE;
				
		CB_PORT[i].Phy_Connect_State=0;
		CB_PORT[i].Logic_Connect_State=0;
		
		memset(CB_PORT[i].Gun_A.Price,0,4);
		memset(CB_PORT[i].Gun_B.Price,0,4);
		
		memset(tax_data[i].code_num,0xFF,10);
		
		for(j=0;j<OnePortGunNum;j++)
		{
		  memset(&total_sum_data[i][j][0],0,sizeof(struct Total_Sum_Data));
			memset(&total_sum_data[i][j][1],0,sizeof(struct Total_Sum_Data));
		}
	}

	for(i=0;i<5;i++)
	  com_status[i]=0xFF;
	
	/*测试加油机用以下设置*/
		CB_PORT[1].Gun_A.gun_num=0;	
	  CB_PORT[1].Gun_B.gun_num=0;	
    CB_PORT[1].Gun_A.OnQuery_Flag=1;
	 // CB_PORT[1].Gun_B.OnQuery_Flag=1;
	
	  CB_PORT[2].Gun_A.gun_num=0;	
	  CB_PORT[2].Gun_B.gun_num=0;	
	  CB_PORT[2].Gun_A.OnQuery_Flag=1;
	
	  CB_PORT[3].Gun_A.gun_num=0;	
	  CB_PORT[3].Gun_B.gun_num=0;	
	  CB_PORT[3].Gun_A.OnQuery_Flag=1;
	
	  CB_PORT[4].Gun_A.gun_num=0;	
	  CB_PORT[4].Gun_B.gun_num=0;	
	  CB_PORT[4].Gun_A.OnQuery_Flag=1;

}

void CB_PortEvent_Pro(Com_Status *PORTx)
{
		 
		 u8 Gun_Num_Ex;		
		 u8 j;
 	   
	   if(PORTx->Gun_A.OnQuery_Flag==1)
			 Gun_Num_Ex=PORTx->Gun_A.gun_num;//查询的枪号
		 else
		   Gun_Num_Ex=PORTx->Gun_B.gun_num;
		 
		
    for(j=1;j<=4;j++)
		{ 
			 if(PORTx->COM_Num==j)
			 {
				 if(TaxRBuffer[j].data[1]==(TaxRBuffer[j].wptr-2))
					 PORTx->GasDataRes_Flag=1;
				 else if(TaxRBuffer[j].data[1]==0x23)//打印税务信息
				 {
					 PORTx->GasDataRes_Flag=1;
					 SEGGER_RTT_printf(0,"RX_OK--PORT%d: Gun:%d,RX[1]:%d,RX:%d!\r\n",j,Gun_Num_Ex,TaxRBuffer[j].data[1]+2,TaxRBuffer[j].wptr);
					 SEGGER_RTT_printf(0,"----------------------------------------\r\n");
					 for(u8 i=0;i<TaxRBuffer[j].wptr;i++)
							 SEGGER_RTT_printf(0,"%02X ",TaxRBuffer[j].data[i]);
					 
					 SEGGER_RTT_printf(0,"\r\n----------------------------------------\r\n");
				 }
				 else
				 {
					 PORTx->GasDataRes_Flag=0;
					 if(TaxRBuffer[j].wptr)
							RX1err++;
					 
					 SEGGER_RTT_printf(0,"RXERR--PORT%d: Gun:%d,RX[1]:%d,RX:%d!\r\n",j,Gun_Num_Ex,TaxRBuffer[j].data[1]+2,TaxRBuffer[j].wptr);
					 SEGGER_RTT_printf(0,"----------------------------------------\r\n");
					 for(u8 i=0;i<TaxRBuffer[j].wptr;i++)
							 SEGGER_RTT_printf(0,"%02X ",TaxRBuffer[j].data[i]);
					 
					 SEGGER_RTT_printf(0,"\r\n----------------------------------------\r\n");
					 
				 }

				 PORTx->RecDlen=TaxRBuffer[j].wptr;
				 TaxRBuffer[j].wptr=0;
				// TaxRBuffer[j].rcv_count=0;
				 
			 }
	   }
		
		 
		 
	    if(PORTx->GasDataRes_Flag==0)//查询失败
			{
												
				if(PORTx->TaxDataQuery_Flag==1)
				{
					//	printf("抄报口：%d, 税务信息查询失败\r\n",PORTx->COM_Num);
					  SEGGER_RTT_printf(0,"CB PORT:%d,Tax Info Query Failed!\r\n",PORTx->COM_Num);
					//  SEGGER_RTT_printf(0,"time: %p.ms\r\n",SYSTEMTICK);
					
					  Gun_Query_Repeat=1;
					// 	Gun_Num_Ex=Gun_Sum;
					  OilCmdGenerate(TAXD_Code, 0, 0, 0);//生成税务信息查询指令 
					
				}
				else if(PORTx->Gun_A.OnQuery_Flag==1)
				{
				   				
					 PORTx->Gun_A.Fail_Cnt++;//查询失败次数加1
					 Gun_Num_Ex=PORTx->Gun_A.gun_num;//更改下次查询的枪号		
					 OilCmdGenerate(TSD_Code, Gun_Num_Ex, 0, 1);//生成日累计加油数据查询指令 	
					//printf("枪号：%d, 查询失败：%d次\r\n",PORTx->Gun_A.gun_num,PORTx->Gun_A.Fail_Cnt);
					 SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d-%d, Query Fail:%dtimes\r\n",PORTx->COM_Num,PORTx->Gun_A.gun_num,Gun_Num_Ex,PORTx->Gun_A.Fail_Cnt);			
					 //SEGGER_RTT_printf(0,"t:%p.ms\r\n",SYSTEMTICK);		
					 Gun_Query_Repeat=1; 
			 
								
				}
				else if(PORTx->Gun_B.OnQuery_Flag==1)
				{			
						PORTx->Gun_B.Fail_Cnt++;//查询失败次数加1	
						Gun_Num_Ex=PORTx->Gun_B.gun_num;//下次查询的枪号
						OilCmdGenerate(TSD_Code, Gun_Num_Ex, 0, 1);//生成日累计加油数据查询指令 	
						//printf("枪号：%d, 查询失败：%d次\r\n",PORTx->Gun_B.gun_num,PORTx->Gun_B.Fail_Cnt);
						SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d-%d, Query Fail:%dtimes\r\n",PORTx->COM_Num,PORTx->Gun_B.gun_num,Gun_Num_Ex,PORTx->Gun_B.Fail_Cnt);
           //  SEGGER_RTT_printf(0,"t:%p.ms\r\n",SYSTEMTICK);	
						Gun_Query_Repeat=1;

					
				}
				else
				{
					//	printf("抄报口：%d, 税务信息查询失败\r\n",PORTx->COM_Num);
					  SEGGER_RTT_printf(0,"CB PORT:%d, NO Query!!!\r\n",PORTx->COM_Num);
								
				}
										
								
				if(PORTx->Gun_A.Fail_Cnt>MAX_FAIL_CNT)//查询失败次数加1
				{
					
					PORTx->Gun_A.Fail_Cnt=0;//清零查询失败次数
					
					if(PORTx->Gun_A.Gun_State==GUN_STATE_GUN_IDLE)
					   PORTx->Gun_A.Gun_State=GUN_STATE_GUN_BUSY;
					else if(PORTx->Gun_A.Gun_State==GUN_STATE_LOG_CONNECTING)
						 PORTx->Gun_A.Gun_State=GUN_STATE_PHY_CONNECT;
					
					
					sys_fault.Fault_Num=GasDataQuery_Exceed_MAX+PORTx->Gun_A.gun_num;
					
				  switch(PORTx->COM_Num)
					{
						case 1:
					    sys_fault.com1.Gun_A.fault_num=GasDataQuery_Exceed_MAX;
						  break;
						case 2:
					    sys_fault.com2.Gun_A.fault_num=GasDataQuery_Exceed_MAX;
						  break;
						case 3:
					    sys_fault.com3.Gun_A.fault_num=GasDataQuery_Exceed_MAX;
						  break;
						case 4:
					    sys_fault.com4.Gun_A.fault_num=GasDataQuery_Exceed_MAX;
						  break;
						
					}
						
			//		  Fault_Info_Storage(&SpiFlash_DataStorageAddr,sys_fault.Fault_Num);	

				}
				
				if(PORTx->Gun_B.Fail_Cnt>MAX_FAIL_CNT)//查询失败次数加1
				{
					
					PORTx->Gun_B.Fail_Cnt=0;//清零查询失败次数
					
					if(PORTx->Gun_B.Gun_State==GUN_STATE_GUN_IDLE)
					   PORTx->Gun_B.Gun_State=GUN_STATE_GUN_BUSY;
					else if(PORTx->Gun_B.Gun_State==GUN_STATE_LOG_CONNECTING)
						 PORTx->Gun_B.Gun_State=GUN_STATE_PHY_CONNECT;
					
					
					sys_fault.Fault_Num=GasDataQuery_Exceed_MAX+PORTx->Gun_B.gun_num;
					switch(PORTx->COM_Num)
					{
						case 1:
					    sys_fault.com1.Gun_B.fault_num=GasDataQuery_Exceed_MAX;
						  break;
						case 2:
					    sys_fault.com2.Gun_B.fault_num=GasDataQuery_Exceed_MAX;
						  break;
						case 3:
					    sys_fault.com3.Gun_B.fault_num=GasDataQuery_Exceed_MAX;
						  break;
						case 4:
					    sys_fault.com4.Gun_B.fault_num=GasDataQuery_Exceed_MAX;
						  break;
						
					 }
					
				//	 	Fault_Info_Storage(&SpiFlash_DataStorageAddr,sys_fault.Fault_Num);	
					
				 }
				 
				 
				 
				  if(Gun_Query_Repeat==1)
					{
						Gun_Query_Repeat=0;
					//	COM_Setup(PORTx);
						if(PORTx->TaxDataQuery_Flag==1)
						  OilCmdSend(PORTx,0);
						else
							OilCmdSend(PORTx,Gun_Num_Ex);
						//DMA_Setup(Gun_Num_Ex);
						PORTx->GasDataRes_Flag=0;
					
					}
								
				
			}
			else//查询成功
			{
					PORTx->GasDataRes_Flag=0;//清零查询成功标志
					PORTx->Phy_Connect_State=1;//物理连接存在
					PORTx->Logic_Connect_State=1;//逻辑连接正常
						
			//	  COM_Setup(PORTx);
				
					//保存加油数据到FLASH				

					Gas_Data_Process(TaxRBuffer[PORTx->COM_Num].data,PORTx);
					 
				//本次数据处理完成，清除各标志位	
					if(PORTx->Gun_A.OnQuery_Flag==1)
					{
						 PORTx->Gun_A.Fail_Cnt=0;//清零查询失败次数
						 PORTx->Gun_A.NoData_Time=0;//清零无数据时间
						
						 if(PORTx->Gun_B.Gun_State==GUN_STATE_GUN_IDLE)
						 {
						   PORTx->Gun_A.OnQuery_Flag=0;//清A枪在查标志
						   PORTx->Gun_B.OnQuery_Flag=1;//设置下次查询B枪
						 }
//							 PORTx->Gun_A.OnQuery_Flag=0;//清A枪在查标志
//						   PORTx->Gun_B.OnQuery_Flag=1;//设置下次查询B枪			
					
					}
					else	
					{					
							PORTx->Gun_B.Fail_Cnt=0;//清零查询失败次数
							PORTx->Gun_B.NoData_Time=0;//清零无数据时间
						 
						  if(PORTx->Gun_A.Gun_State==GUN_STATE_GUN_IDLE)
						  {
								PORTx->Gun_B.OnQuery_Flag=0;//清B枪在查标志
								PORTx->Gun_A.OnQuery_Flag=1;//设置下次查询A枪
						  }
						
//						   PORTx->Gun_B.OnQuery_Flag=0;//清B枪在查标志
//							 PORTx->Gun_A.OnQuery_Flag=1;//设置下次查询A枪
					}
					
					/*****/ 
					
					if(PORTx->Gun_A.Price_Query_Flag==1)//单价查询请求
					{
						PORTx->Gun_A.DSumDataQuery_Flag=0;
						PORTx->Gun_A.MSumDataQuery_Flag=0;
						PORTx->Gun_A.TSumDataQuery_Flag=0;
					}
					
					if(PORTx->Gun_B.Price_Query_Flag==1)//单价查询请求
					{
						PORTx->Gun_B.DSumDataQuery_Flag=0;
						PORTx->Gun_B.MSumDataQuery_Flag=0;
						PORTx->Gun_B.TSumDataQuery_Flag=0;
					}
					
					
					if(PORTx->Gun_A.OnQuery_Flag==1)
					{
						  Gun_Num_Ex=PORTx->Gun_A.gun_num;
						
							if(PORTx->TaxDataQuery_Flag==1)//查税务信息的优先级最高，不受其他查询标志影响
							{
									OilCmdGenerate(TAXD_Code, 0, 0, 0);//生成税务信息查询指令 
							}								
							else if(PORTx->Gun_A.DSumDataQuery_Flag==1)
							{
									OilCmdGenerate(DSD_Code, Gun_Num_Ex, 0, 1);//生成日累计加油数据查询指令 
							}								
							else if(PORTx->Gun_A.MSumDataQuery_Flag==1)
							{
									OilCmdGenerate(MSD_Code, Gun_Num_Ex, 0, 1);//生成月累计加油数据查询指令 
							}								
							else if(PORTx->Gun_A.TSumDataQuery_Flag==1)
							{
									OilCmdGenerate(TSD_Code, Gun_Num_Ex, 0, 1);//生成总累计加油数据查询指令 
							}								
							else
									OilCmdGenerate(OTD_Code, Gun_Num_Ex, 0, 1);//生成当次加油数据查询指令 						
																							
					}						
					else
					{
						  Gun_Num_Ex=PORTx->Gun_B.gun_num;	
						
							if(PORTx->TaxDataQuery_Flag==1)//查税务信息的优先级最高，不受其他查询标志影响
							{
									OilCmdGenerate(TAXD_Code, 0, 0, 0);//生成税务信息查询指令 
							}								
							else if(PORTx->Gun_B.DSumDataQuery_Flag==1)
							{
									OilCmdGenerate(DSD_Code, Gun_Num_Ex, 0, 1);//生成日累计加油数据查询指令 
							}								
							else if(PORTx->Gun_B.MSumDataQuery_Flag==1)
							{
									OilCmdGenerate(MSD_Code, Gun_Num_Ex, 0, 1);//生成月累计加油数据查询指令 
							}								
							else if(PORTx->Gun_B.TSumDataQuery_Flag==1)
							{
									OilCmdGenerate(TSD_Code, Gun_Num_Ex, 0, 1);//生成总累计加油数据查询指令 
							}								
							else
									OilCmdGenerate(OTD_Code, Gun_Num_Ex, 0, 1);//生成当次加油数据查询指令 	
						
					}						
			
					// DMA_Setup(Gun_Num_Ex);	
					 OilCmdSend(PORTx,Gun_Num_Ex);					
           PORTx->GasDataRes_Flag=0;					
					
				}
											
}


void CB_TaxData_Query(Com_Status *PORTx,u8 gun_num)
{
	
	  OilCmdGenerate(TAXD_Code,gun_num, 0, 0);//生成税务信息查询指令 
	  OilCmdSend(PORTx,gun_num);
	
	  PORTx->Gun_A.Gun_State=GUN_STATE_LOG_CONNECTING;
	  PORTx->Gun_B.Gun_State=GUN_STATE_LOG_CONNECTING;
	  
	
}

void CB_OTDData_Disp(Com_Status *PORTx,u8 gun_num)
{
	
	  OilCmdGenerate(OTD_Code,gun_num, 0, 0);//参数三：00H表示直接回送到显示屏，01H表示明文输出，大于01H表示密文输出
	  OilCmdSend(PORTx,gun_num);
	

}

void OilCmdSend(Com_Status *PORTx,u8 Gun_No)
{
	  
	u16 i;
	u8 *Buf;
	u32 USARTn_DR;
	USART_TypeDef * USARTn;
	
	switch(PORTx->COM_Num)
	{
		case 1:
			TX_DMA_Channl=USART1_TX_DMA_Ch;
			RX_DMA_Channl=USART1_RX_DMA_Ch;
			USARTn_DR=USART1_DR;
			USARTn=USART1;	
			break;
		
		case 2:
			TX_DMA_Channl=USART2_TX_DMA_Ch;
			RX_DMA_Channl=USART2_RX_DMA_Ch;
			USARTn_DR = USART2_DR;
			USARTn = USART2;	
			break;
		
		case 3:
			TX_DMA_Channl=USART3_TX_DMA_Ch;
			RX_DMA_Channl=USART3_RX_DMA_Ch;
			USARTn_DR=USART3_DR;
			USARTn=USART3;
			break;
		
		case 4:
			TX_DMA_Channl=UART4_TX_DMA_Ch;
			RX_DMA_Channl=UART4_RX_DMA_Ch;
			USARTn_DR=UART4_DR;
			USARTn=UART4;			
			break;
		
	 }	
	 
//	 memcpy(TaxSBuffer[PORTx->COM_Num].data+TaxSBuffer[PORTx->COM_Num].rptr,(u8*)TxPtr[Gun_No],TxDlen[Gun_No]);
	
	 Buf=(u8*)TxPtr[Gun_No];
	 
	 for(i=0;i<TxDlen[Gun_No];i++)
	 {
		 TaxSBuffer[PORTx->COM_Num].data[TaxSBuffer[PORTx->COM_Num].wptr]=Buf[i];
		 TaxSBuffer[PORTx->COM_Num].wptr++;
		 if(TaxSBuffer[PORTx->COM_Num].wptr==TAX_SND_BUFFER_SIZE)
		    TaxSBuffer[PORTx->COM_Num].wptr=0;
	 }
	 
	 
	do
	{				 
			USART_SendData(USARTn,TaxSBuffer[PORTx->COM_Num].data[TaxSBuffer[PORTx->COM_Num].rptr]);
			while(USART_GetFlagStatus(USARTn, USART_FLAG_TC) == RESET);
		 
			TaxSBuffer[PORTx->COM_Num].rptr++;
			if(TaxSBuffer[PORTx->COM_Num].rptr == TAX_SND_BUFFER_SIZE )
					TaxSBuffer[PORTx->COM_Num].rptr=0;		
	  	
	}while(TaxSBuffer[PORTx->COM_Num].rptr != TaxSBuffer[PORTx->COM_Num].wptr);

	
}

void CB_Send_Search(u8 COM_Num,u8 *Buf,u32 TxDlen)
{
	  
	u16 i;
	USART_TypeDef * USARTn;
	switch (COM_Num)
	{
		case 1:
			USARTn=USART1;
		break;
		case 2:
			USARTn=USART2;
		break;
		case 3:
			USARTn=USART3;
		break;
		case 4:
			USARTn=UART4;
		break;
		default:
			break;
	}
	
	for(i=0;i<TxDlen;i++)
	{
		USART_SendData(USARTn, *Buf);
		while (USART_GetFlagStatus(USARTn, USART_FLAG_TC) == RESET);
		Buf++;
	}

	
}


u8 Gun_Num_Search(u8 Gun_Num,u16 S_Delay)
{
	
  struct TSD_Command  tsd_order;
  u8 j;
	u8 TxDlen;
	u8 Search_ON[5]={0};//查询开启标志
	u8 Get_Data[5];//三次查询中得到数据的次数
	u8 Get_Gun_cnt[5]={0};//0~255中得到的枪号个数
	u8 Search_cnt;//同一枪号查询次数
	u8 OnePortGun;
	u8 NeedSearchCnt=3;//同一枪号需要查询的次数
	u8 res;
	s16 i;
	
	TxDlen=sizeof(struct TSD_Command);	
	
	SEGGER_RTT_printf(0,"Start Gun Num Search...\r\n");
	
  for(j=1;j<5;j++)
	{
		if(CB_PORT[j].Phy_Connect_State)
		{
			Search_ON[j]=1;
			OnePortGun=Com_Gun_BindInfo[j].GunCount;
		}
	}		
	
	
	for(i=0;i<=Gun_Num;i++)
	{
	
		memset(Get_Data,0,5);

		Search_cnt=0;		

		Command_Gen_Search(&tsd_order,i);
		
					
Search_Gun:		
		  		
		    for(j=1;j<5;j++)
		    {
				if(Search_ON[j] && CB_PORT[j].Phy_Connect_State)
				  CB_Send_Search(j,(u8*)(&tsd_order),TxDlen);
			}
				
			Search_cnt++;
			SEGGER_RTT_printf(0," %d",i);			
			LED_Blink;
			bsp_DelayMS(S_Delay);
			
			ClearDog;			
				
			for(j=1;j<5;j++)
			{
				 if(TaxRBuffer[j].data[1]==(TaxRBuffer[j].wptr-2) && Search_ON[j])
				 {
					  if(TaxRBuffer[j].data[1]>4)
							Get_Data[j]++;
				 }
				 
				 TaxRBuffer[j].wptr=0;//清零接收buf
			}
			
					
			 if(Search_cnt<NeedSearchCnt)
			 {			 				
				 goto Search_Gun;
			 }
			 else
			 {
				
				 for(j=1;j<5;j++)
				 {				 
					 if(Get_Data[j]>0)
					 {
						 if(Get_Gun_cnt[j]==0)
						 {
							  CB_PORT[j].Gun_A.gun_num=i;			
							  CB_PORT[j].Gun_A.Gun_State=GUN_STATE_GUN_IDLE;
							  Get_Gun_cnt[j]++;
							  
							  Com_Gun_BindInfo[j].COM_Num=j;
							  Com_Gun_BindInfo[j].Gun_NO[0]=i;
							 
						 }
						 else
						 {
							  CB_PORT[j].Gun_B.gun_num=i;
							  CB_PORT[j].Gun_B.Gun_State=GUN_STATE_GUN_IDLE;
							  Get_Gun_cnt[j]++;
							 
							  Com_Gun_BindInfo[j].COM_Num=j;
							  Com_Gun_BindInfo[j].Gun_NO[1]=i;
						 }
					 
					 }	

					 if(Get_Gun_cnt[j]==OnePortGun)
						 Search_ON[j]=0;
				 }
				 
				 						 
//				 if(!Search_ON[1] && !Search_ON[2])
//					 return 4;
				 				 
			 }
				 		 
	 }
	
	
	
	for(j=1;j<5;j++)
	{
		if(CB_PORT[j].Phy_Connect_State)
		{
			 Com_Gun_BindInfo[j].GunCount=Get_Gun_cnt[j];					
			// res=sf_WriteBuffer((u8*)&Com_Gun_BindInfo[j],(SpiFlash_TaxCode_ADDR+j*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
			 res=uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[j],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+j*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
			
		   if(!res)
				 SEGGER_RTT_printf(0,"Com_Gun_BindInfo[%d] WriteFlash Failed.\r\n",j);		
		}
	}		
	
	
	 
	 return (Get_Gun_cnt[1]+Get_Gun_cnt[2]+Get_Gun_cnt[3]+Get_Gun_cnt[4]);
	 	
}


u8 Price_Calibration(u16 S_Delay)
{

	struct Day_Sum_Data dsd_data;
    struct OTD_Command  otd_order;
	struct One_Time_Data *dptr;
    u8 i,j,Cnt;
	u8 TxDlen,DSD_Data_Len;
	u8 Search_ON[5]={0};//查询开启标志
	u8 Price_Same[5];//查询中得到单价相同的次数

	u8 Price[5][4];//Price[串口号][单价]
	u8 Price_Zero[4]={0};
	u8 Gun_No;//枪号
	u8 Calibration_A=1;//1：校准A枪；0：校准B枪

	TxDlen=sizeof(struct OTD_Command);	
	DSD_Data_Len=sizeof(struct Day_Sum_Data);
	
	SEGGER_RTT_printf(0,"Start Price Calibration...\r\n");

Calibration_Begin:	
	
	Cnt=0;
	
	for(j=1;j<=4;j++)
	{
		if(CB_PORT[j].Phy_Connect_State)
			Search_ON[j]=1;
		
		memset(&Price[j][j-1],0,4);//单价初始化为0
	}		
	
	memset(Price_Same,0,5);
	memset((u8*)&dsd_data,0,sizeof(struct Day_Sum_Data));
	

	while(1)//
	{
						  			 
      for(i=1;i<=4;i++)//四口轮询
		  {
				if(Calibration_A)
				  Gun_No=CB_PORT[i].Gun_A.gun_num;
				else
					Gun_No=CB_PORT[i].Gun_B.gun_num;
				
				if(Search_ON[i] && CB_PORT[j].Phy_Connect_State)
				{
					 Command_Gen_OTD(&otd_order,Gun_No,1);
					 CB_Send_Search(i,(u8*)(&otd_order),TxDlen);
				}
				
		  }
			
			SEGGER_RTT_printf(0," %d",Cnt++);			
			LED_Blink;
			bsp_DelayMS(S_Delay);
			

      for(j=1;j<5;j++)
			{		
				if(TaxRBuffer[j].data[1]==(TaxRBuffer[j].wptr-2) && Search_ON[j])
				{
					 dptr=(struct One_Time_Data *)(TaxRBuffer[j].data+5);
					 if(!memcmp(Price_Zero,&Price[j][0],4))
						   memcpy(&Price[j][0],dptr->price,4);
					 else
					 {
							if(!memcmp(&Price[j][0],dptr->price,4))				
								 Price_Same[j]++;
							else
							{
								memcpy(&Price[j][0],dptr->price,4);
								Price_Same[j]=0;
							}
							
					 }	
					 
				}
				
				TaxRBuffer[j].wptr=0;
				
				if(Price_Same[j]==3)
					 Search_ON[j]=0;	
				
			}
			 	
			 
			 if(Search_ON[1]==0 && Search_ON[2]==0 && Search_ON[3]==0 && Search_ON[4]==0)
			 {
				 
				 if(Calibration_A)
				 {
					 for(j=1;j<=4;j++)
					 {
						 if(CB_PORT[j].Phy_Connect_State)
						 {
							 memcpy((dsd_data.price),&Price[j][0],4);
					   //Gas_Data_Storage((u8*)(&dsd_data),DSD_Data_Len,&SpiFlash_DataStorageAddr,CB_PORT[j].Gun_A.gun_num,j,DSD_Code);			
							 Gas_Data_Storage((u8*)(&dsd_data),DSD_Data_Len,&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,CB_PORT[j].Gun_A.gun_num,j,DSD_Code);			
						
	             SEGGER_RTT_printf(0,"\r\nCOM%d,Gun_A:%d,Price=%4.4s\r\n",j,CB_PORT[j].Gun_A.gun_num,dsd_data.price);
						 }
					 }
					 
				   Calibration_A=0;
					 
					
				   goto Calibration_Begin;
					 
			   }
				 else				 
				 {
					 for(j=1;j<=4;j++)
					 {
						 if(CB_PORT[j].Phy_Connect_State)
						 {
							 memcpy(&(dsd_data.price),&Price[j][0],4);
						// Gas_Data_Storage((u8*)(&dsd_data),DSD_Data_Len,&SpiFlash_DataStorageAddr,CB_PORT[j].Gun_B.gun_num,j,DSD_Code);	
							 Gas_Data_Storage((u8*)(&dsd_data),DSD_Data_Len,&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,CB_PORT[j].Gun_B.gun_num,j,DSD_Code);			
						 
							 SEGGER_RTT_printf(0,"\r\nCOM%d,Gun_B:%d,Price=%4.4s\r\n",j,CB_PORT[j].Gun_B.gun_num,dsd_data.price);	
						 }
					 }
					 
					  return 0;
					 
				 }
				 
				 
			 }		 			 
				 		 
	 }
	
	// return (0);
	 	
}

void PLC_Send_Enable(void)
{
	  // USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	  // USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);
		 USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
}

void PLC_ReConnect(u8 cnt)
{
	   u8 res;	

		 ClearDog;	 
	   PLC_PWR_OFF; 
	   bsp_DelayMS(10000);
	   PLC_PWR_ON; 
	   ClearDog;	
	   bsp_DelayMS(10000);
	
	   PLC_Anysis_Fail_Cnt=0;												

	    PLC_RBuffer.rptr=0;
	    PLC_RBuffer.wptr=0;
							
	   while(cnt--)
		 {
				res = initPLCModem();
			 
			  PLC_PORT.counter++;
		    PLC_PORT.last_reset_time[(PLC_PORT.counter-1)%10]=RTC_GetCounter();
			 
				if(!res)
					 break;
				
				bsp_DelayMS(5000);
				 ClearDog;	
				  
		 }
		 	
			
			if(res)//PLC初始化失败
			{
				//系统复位
			  //PLC_PORT.Connect_status=0;
				setPlcState(PLC_STATE_IDLE);
				SYSTEM_RESET(PLC_CONNECT_FAULT);
			}
			else
			{
	//			PLC_Connect_Flag=1;//PLC连接成功
				setPlcState(PLC_STATE_INIT);
				
			//  PLC_PORT.Connect_status=1;
				SEGGER_RTT_printf(0,"PLC ReConnected OK!\r\n");
						
				memcpy((u8*)&sys_fault,(u8*)(flashSectionStruct[FLASH_SECTION_SYSTEM_INFO].base),sizeof(struct _SystemDignose_Info));
	
				sys_fault.TimeStamp=RTC_GetCounter();
				
				//PLC重连次数
				if(sys_fault.com5.counter==0xFFFFFFFF)	
					sys_fault.com5.counter=PLC_PORT.counter;	
				else
					sys_fault.com5.counter=PLC_PORT.counter;	
				
				 sys_fault.com5.last_reset_time[(sys_fault.com5.counter-1)%10]=sys_fault.TimeStamp;
				
				res=uf_WriteBuffer(FLASH_STM32F103RET6,(u8*)&sys_fault,flashSectionStruct[FLASH_SECTION_SYSTEM_INFO].base,sizeof(struct _SystemDignose_Info));
	
				 if(res)
				 {
					 SEGGER_RTT_printf(0,"PLC connection info have updated!\r\n");
				 }
				 
         
				 bsp_DelayMS(5000);
				  ClearDog;	

			}
			
				USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
			  PLC_Data_Send(FW_VER,4,FWrev_Data,1,1);//发送固件版本信息
				     setPlcState(PLC_STATE_CONNECTING);
			
}


void COM_Connect_Detect(void)
{ 
	    u16 fault_num;
    	u16 io_data1;
	    u16 io_data3;

	    u8 com[5]={0};//未连接的com口为1
	    u8 com_cnt=0;
			u8 i,j;
			static u8 flash_erase_flag=0;
			u8 N=7,X=3;
				
			
	  /*spiFlash数据清除检测*/
	    io_data3=GPIO_ReadInputData(GPIOC);
		  io_data3&=0x2000;//判断PC13口电平		
			if((io_data3==0)&&(flash_erase_flag==0))
			{      
				  spiFlash_DataRegionErase();
				
					flash_erase_flag=1;
				  SEGGER_RTT_printf(0,"----------spiFlash_DataRegionErase!-----------\r\n");	
	        bsp_StartTimer(SpiFlash_Erase_Timer,5000);					
			}
						
			if(bsp_CheckTimer(SpiFlash_Erase_Timer))
				flash_erase_flag=0;
			
		 /*spiFlash数据清除检测*/	
			
	
	    for(i=0;i<N;i++)
			{
				io_data1=GPIO_ReadInputData(GPIOC);
				io_data1&=0x0F;
				
				for(j=0;j<4;j++)
				{
					if(io_data1&(0x08>>j))
					{
						com[j+1]++;																			
					}				
				}
							
				bsp_DelayMS(70); //  11/9600*(32+6+16+6)=68.75ms
				
			}
	  
		  for(j=0;j<4;j++)
			{
				if(com[j+1]>=((N+X)/2))		//
				{
					com[j+1]=1;	
					com_cnt++;
				}					
				else
					com[j+1]=0;
			} 
			
							
			if(com_cnt==4)
			{
		//	  Fault_Info_Storage(&SpiFlash_DataStorageAddr,COMA_PHY_DISCONNECT);	
					SEGGER_RTT_printf(0,"ALL COM DisConnected!\r\n");
		      for(i=1;i<5;i++)
				     CB_PORT[i].Phy_Connect_State=0;
			}
			else if(com_cnt>0)
			{
				SEGGER_RTT_printf(0,"Port:");
				for(i=1;i<=4;i++)
				{
					if(com[i])
					{
						switch(i)
						{	
							case 4:															
								fault_num=COM4_PHY_DISCONNECT;
								CB_PORT[4].Phy_Connect_State=0;
							break;
							case 3:															
								fault_num=COM3_PHY_DISCONNECT;
								CB_PORT[3].Phy_Connect_State=0;
							break;
							case 2:															
								fault_num=COM2_PHY_DISCONNECT;
								CB_PORT[2].Phy_Connect_State=0;
							break;
							case 1:															
								fault_num=COM1_PHY_DISCONNECT;
								CB_PORT[1].Phy_Connect_State=0;
							break;
						}
						
					//	  Fault_Info_Storage(&SpiFlash_DataStorageAddr,fault_num);
							SEGGER_RTT_printf(0,"[X] ");
					}
					else
					{																											
							CB_PORT[i].Phy_Connect_State=1;
							SEGGER_RTT_printf(0,"[%d] ",i);
																										
					}
																
				}
				SEGGER_RTT_printf(0,"\r\n");
			} 						
			else//四个串口全都连接
			{
				SEGGER_RTT_printf(0,"ALL COM Connected OK!\r\n");
				
				for(i=1;i<5;i++)
				{
				  CB_PORT[i].Phy_Connect_State=1;
					CB_PORT[i].Gun_A.Gun_State=GUN_STATE_PHY_CONNECT;
					CB_PORT[i].Gun_B.Gun_State=GUN_STATE_PHY_CONNECT;
				}
				
			}
	
			
//如果串口状态发生改变，重新查询税务信息	
//com[i]是本次检测到的状态，com_status是上次的状态；1是未连接，0是已连接
			if(memcmp(com_status,com,5))
			{
				for(i=0;i<5;i++)
				{
					if(com_status[i]!=0xFF)//判断com_status是否为初始值
						break;
				}
				
				if(i==5)
					memcpy(com_status,com,5);
				else
				{
					for(i=1;i<5;i++)
					{
						if(com_status[i]!=com[i])
						{
							if(com_status[i]==1 && com[i]==0)	//新插上的串口
							{								
								CB_PORT[i].TaxDataQuery_Flag=0;//	
								SEGGER_RTT_printf(0,"Need Query TaxInfo!\r\n");
							}								
														
							com_status[i]=com[i];
							SEGGER_RTT_printf(0,"COM%d Status Changed!\r\n",i);
							
						}							
					}
				}
					
			}		
			
}

/*

void DMA_Setup(u8 Gun_No)
{
	  DMA_Configuration(TX_DMA_Channl, TxDlen[Gun_No], USARTn_DR, TxPtr[Gun_No], DMA_DIR_PeripheralDST, DMA_Priority_High);
		DMA_Configuration(RX_DMA_Channl, TxRecDlen[Gun_No],USARTn_DR,(u32)RxBUF,DMA_DIR_PeripheralSRC,DMA_Priority_High);
		USART_DMACmd(USARTn, USART_DMAReq_Tx, ENABLE);
		USART_DMACmd(USARTn, USART_DMAReq_Rx, ENABLE);		
		DMA_Cmd(TX_DMA_Channl, ENABLE);
		DMA_Cmd(RX_DMA_Channl, ENABLE);
}
*/

void PLC_Data_Send(u8* data,u16 dlen,u16 command_id, u8 F_Sum,u8 F_Num)
{
	struct Protocol_Header p_head;

	Protocol_Send(&p_head,command_id,F_Sum,F_Num,data,dlen);
 
	// PLC_Send_Fail_Cnt++;	
	
}

void CB_HeartBeat(void)
{
	struct Protocol_Header p_head;

	u8 data=0x88;
	
	Protocol_Send(&p_head,HeartBeat,1,1,&data,1);
//	 PLC_Send_Fail_Cnt++;	
	
}

