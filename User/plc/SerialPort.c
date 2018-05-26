
#include "SerialPort.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bsp_usart.h"	
#include "PLCMessage.h"
#include "cb.h"



u8 g_bShutDown = false;



void ReadFile(BYTE* Desbuf,USART_TypeDef* USARTx,u16 NumToRead, u16* ReadNum)
{

	
	 bsp_StartTimer(PLC_Communication_Timer,5*1000);//5秒超时
	 while(NumToRead)
   {	
			
		 if(USART_GetFlagStatus(USARTx,USART_FLAG_RXNE))
			{
				*Desbuf=(u8)USART_ReceiveData(USARTx);
				 Desbuf++;
				 (*ReadNum)++;
				 NumToRead--;
		
			}
			else if(bsp_CheckTimer(PLC_Communication_Timer))
			{
				break;
			}
			
	 }
		
			
	   
}


BYTE* Read(UINT16 length, UINT16* bytesRead)
{
	BYTE* buffer = (BYTE*)malloc(length);
	
  if(buffer!=0)
    {	
			memset(buffer, 0, length);
			*bytesRead = 0;
			ReadFile(buffer, UART5, length, bytesRead);
		 
	  }
	
	return buffer;
	
}


bool Write(BYTE* bytes, UINT16 length)
{
	 Uart5SendBuf(bytes,length);

		return true;
	
}

bool ClosePort()
{

	return true;
}


void SerialReadThread(void)
{
	static int count = 0;
	struct SystemInfo sysInfo;
	struct StatusMessage statusMessage;
	struct AttachConfirm attachConfirm;
	struct DataTransferConfirm dataTransferConfirm;
	BYTE* _body;
	struct Header* _header;
	UINT16 bytesToRead;
	UINT16 bytesRead;
	u8 Attach_flag=0;
	
	static u16 count2 = 0;
	extern u8 UART2_ISR_FLAG;
	memset((void*)&sysInfo, 0, sizeof(struct SystemInfo));

	memset((void*)&statusMessage, 0, sizeof(struct SystemInfo));

 	memset((void*)&attachConfirm, 0, sizeof(struct AttachConfirm));
	
	memset((void*)&dataTransferConfirm, 0, sizeof(struct DataTransferConfirm));
	
	while(g_bShutDown == false)
	{
		 bytesToRead = sizeof(struct Header);	//需要读取长度
		 bytesRead = 0;					//实际读到长度
 		_header = (struct Header*)Read(bytesToRead, &bytesRead);
		if (_header != NULL 
		  && bytesToRead == bytesRead)
		{
			bytesToRead = _header->_Length;
			bytesRead = 0;
			_body = Read(bytesToRead, &bytesRead);
			if ((_header->_Length&1) == 1)
			{
				UINT16 temp=0;
				Read(1, &temp);
			}
			if (_body != NULL
			  && bytesToRead == bytesRead)
			{
				switch (_header->_Id)
				{
				case Get_System_Info_Id:
					RecvMessage(_header, _body, (void*)&sysInfo);
					if (!CheckInit(sysInfo))
					{
						SendLoadSystemConfig_Port(SCI_A, SCI_A);
					}
					SendSetTxInfo();
					//SendDiscoverNetworkRequest();
					break;
				case Load_System_Config_Id:
					RecvMessage(_header, _body, (void*)&statusMessage);
					if (g_curCmd == LoadSystemConfig_PortCmd && statusMessage._Status == 0)
					{
						SendLoadSystemConfig_DeviceMode(0);
					}
					if (g_curCmd == LoadSystemConfig_DeviceModeCmd && statusMessage._Status == 0)
					{
						SendShutDownMessage();
					}
					break;
				case Shut_Down_Id:
					RecvMessage(_header, _body, (void*)&statusMessage);
					if (statusMessage._Status == 0)
					{
						SendGetSystemInfoRequest();
					}
					break;
				case Discover_Id:
				//	RecvDiscoverNetworkConfirm(_header, _body);
					if (_PAN_Id != 0)
					{
						SendAttachRequest();
					}
					else
					{
						SendDiscoverNetworkRequest();
					}
					break;
				case Attach_Id:
					RecvMessage(_header, _body, (void*)&attachConfirm);
					g_init = true;
					_PAN_Id = attachConfirm._PAN_Id;
				    _LBA_Address = attachConfirm._NetworkAddress;
					genIpv6Addr[8] = _PAN_Id / 256;
					genIpv6Addr[9] = _PAN_Id % 256;
					genIpv6Addr[14] = _LBA_Address / 256;
					genIpv6Addr[15] = _LBA_Address % 256;
			//		Sleep(1000);
				 	printf("attach success: id=0x%04X, shortAddr=0x%04X\n", attachConfirm._PAN_Id, attachConfirm._NetworkAddress);
					Attach_flag=1;
			///	 SendData(data_1,7);
					break;
				case Data_Transfer_Id:
					if (_header->_Length == 8)  //DATA_TRANSFER.confirm
					{
						RecvMessage(_header, _body, (void*)&dataTransferConfirm);
						if (dataTransferConfirm._Status == 0)
						{
					//		printf("接收到dataTransferConfirm\n");
						}
					}
					else						//DATA_TRANSFER.Indication
					{
						RecvDataTransferConfirm(_header, _body);
						count++;
						printf("count is %d:\n", count);
					}
				//	SendData((u8*)&count,2);
					break;
					
				case Set_Info_Id:
					RecvMessage(_header, _body, (void*)&statusMessage);
					if (g_curCmd == SetTxInfoCmd && statusMessage._Status == 0)
					{
						//SendSetRxInfo();
						SendDiscoverNetworkRequest();
					}
					if (g_curCmd == SetRxInfoCmd && statusMessage._Status == 0)
					{
						SendDiscoverNetworkRequest();
					}
					break;
				default:
					break;
				}
			}
			if (_body != NULL)
			{
				free(_body);
			}
		}
		if (_header != NULL)
		{
			free(_header);
		}
		
		if(Attach_flag)
		{
//			  if(UART2_ISR_FLAG)
//				{
//						  bsp_DelayMS(500);
//					    if(Uart2_InNum!=Uart2_OutNum)
//							{
//								SendData(Rx2Buffer,Uart2_InNum);
//								//Uart2_OutNum=Uart2_InNum;
//								Uart2_InNum=0;
//								UART2_ISR_FLAG=0;
//							}
					    
//				}
//			  sprintf(data_1,"%d",count2++);
//				SendData(data_1,strlen(data_1));
//			  bsp_DelayMS(500);
			 // Uart2SendBuf(&count2,1);
			// count2++;
		}
	}
	//return ;
}
