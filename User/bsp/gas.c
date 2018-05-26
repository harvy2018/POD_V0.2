

#include <string.h>
#include <stdlib.h>
#include <time.h>	
#include "gas.h"
#include "cb.h"
#include "dev_state.h"	
#include "flashmgr.h"	
#include "global.h"	
#include "oildatapro.h"	

u8 TAXD_Resp_Len = 16; //δ��ʼ����˰����Ϣ��ֻ��10�ֽ�˰��װ�����к���Ϣ
u8 OTD_Resp_Len =  28;
u8 DSD_Resp_Len  = 34;
u8 MSD_Resp_Len =  32;
u8 TSD_Resp_Len =  30;
u8 KTK=0;//��̩��


u8 ratio_oil_0,ratio_oil_1;
char ratio_oil_char[5];

u32 spiFlash_DetectAddr=0xF0000; //1MB-64k��ʼ��ַ ��������ַ0x1F0000
u32 SpiFlash_DataStorageAddr=0xF0000;
u32 SpiFlash_DataStorCurAddr;//�������ݴ洢��ǰ��ַ

struct _CB_Time CB_Time;
struct tm *ASCTIME2;

unsigned char NEW[4]="UVWX";
unsigned char OLD[4]="QRSP";

struct tm DATE;
u32 t_stamp;

unsigned char package_type;//��֡��־
unsigned char package_num;//��֡��֡��
unsigned char package_num_s;//��֡��֡��
unsigned char command;//������

struct Tax_Data *p_t; //init_data�ṹ����ʱָ��

struct Tax_Data tax_data[PORT_SUM+1];
struct Tax_Data_X *tax_data_x;

struct One_Time_Data one_time_data[TAX_PORT_Sum][OnePortGunNum];
struct One_Time_Data OTData[TAX_PORT_Sum][OnePortGunNum];

struct One_Time_Data *otd_ptr,*otd_ptrx;

struct Day_Sum_Data     day_sum_data[TAX_PORT_Sum][OnePortGunNum];
struct Day_Sum_Data     *dsd_ptr;
struct Month_Sum_Data   month_sum_data[TAX_PORT_Sum][OnePortGunNum];
struct Month_Sum_Data   *msd_ptr;
struct Total_Sum_Data   total_sum_data[TAX_PORT_Sum][OnePortGunNum][2];
struct Total_Sum_Data   *tsd_ptr;

//u32 CB_Sample_Time [TAX_PORT_Sum][OnePortGunNum];
	
struct TaxD_Command      taxd_command;
struct OTD_Command        otd_command;
struct DSD_Command        dsd_command;
struct MSD_Command        msd_command;
struct TSD_Command        tsd_command;

struct OILDATA_ANALYSIS   oildata_analysis[TAX_PORT_Sum][OnePortGunNum];  

u8 Data_Need_Storage;
u16 CB_Year,CB_Month,CB_Day;
struct s sx[TAX_PORT_Sum][OnePortGunNum];
u32 ERROR1,ERROR2,ERROR3,GX,GX1,D_Same;
u32 TSDNoMatch0,TSDNoMatch1,Correct;



u8 OilCmdGenerate(u8 command_code,u8 gun_num,u8 tsd_mode,u8 ret_mode)
{
	
	u8 *ptr;
	u8 i;

	switch(command_code)
	{	 
		case TAXD_Code:
			taxd_command.preamble=0xBB;
		    taxd_command.len=4;
		    taxd_command.rfn=0xFF;
		    taxd_command.command=0x83;
		    taxd_command.gun_num=gun_num;
			
		    taxd_command.ccd= 0xFF;
		    taxd_command.ccd^=0x83;
		    taxd_command.ccd^=gun_num;	
		
			TxPtr[gun_num]=(u32)(&taxd_command);
			TxDlen[gun_num]=sizeof(struct TaxD_Command);
			TxRecDlen[gun_num]=TAXD_Resp_Len;		
		    
		    break;
		
		case OTD_Code:
			  otd_command.preamble=0xBB;
		    otd_command.len=5;
		    otd_command.rfn=0xFF;
		    otd_command.command=0x86;
		    otd_command.gun_num=gun_num;
		    otd_command.ret_mode=ret_mode;
		
		    otd_command.ccd= 0xFF;
		    otd_command.ccd^=0x86;
		    otd_command.ccd^=gun_num;	
        otd_command.ccd^=ret_mode;	

        TxPtr[gun_num]=(u32)(&otd_command);
				TxDlen[gun_num]=sizeof(struct OTD_Command);	
				TxRecDlen[gun_num]=OTD_Resp_Len;			
		break;
		
		case DSD_Code:
			  dsd_command.preamble=0xBB;
		    dsd_command.len=13;
		    dsd_command.rfn=0xFF;
		    dsd_command.command=0x87;
		    dsd_command.gun_num=gun_num;
		
			  memcpy(dsd_command.year,CB_Time.year,4);
        memcpy(dsd_command.month,CB_Time.month,2);
		    memcpy(dsd_command.day,CB_Time.day,2);
		    dsd_command.ret_mode=ret_mode;
		
		    dsd_command.ccd= 0xFF;
		
		    ptr=&dsd_command.command;
		    for(i=0;i<11;i++)
		      dsd_command.ccd^=*(ptr+i);
		   
		
		   	TxPtr[gun_num]=(u32)(&dsd_command);						
				TxDlen[gun_num]=sizeof(struct DSD_Command);	
				TxRecDlen[gun_num]=DSD_Resp_Len;		
		    	
		break;
				
		case MSD_Code:
			  msd_command.preamble=0xBB;
		    msd_command.len=11;
		    msd_command.rfn=0xFF;
		    msd_command.command=0x88;
		    msd_command.gun_num=gun_num;
		    memcpy(msd_command.year,CB_Time.year,4);
        memcpy(msd_command.month,CB_Time.month,2);
		    msd_command.ret_mode=ret_mode;
		
		    msd_command.ccd= 0xFF;
		
		    ptr=&msd_command.command;
		    for(i=0;i<9;i++)
		      msd_command.ccd^=*(ptr+i);
		    	
		    TxPtr[gun_num]=(u32)(&msd_command);
				TxDlen[gun_num]=sizeof(struct MSD_Command);		
				TxRecDlen[gun_num]=MSD_Resp_Len;	
	  break;
		
		case TSD_Code:
			  tsd_command.preamble=0xBB;
		    tsd_command.len=6;
		    tsd_command.rfn=0xFF;
		    tsd_command.command=0x89;
		    tsd_command.gun_num=gun_num;
				tsd_command.tsd_mode=tsd_mode;
		    tsd_command.ret_mode=ret_mode;
		
		    tsd_command.ccd= 0xFF;
		    tsd_command.ccd^=0x89;
		    tsd_command.ccd^=gun_num;	
		    tsd_command.ccd^=tsd_mode;	
        tsd_command.ccd^=ret_mode;			
			
		    TxPtr[gun_num]=(u32)&(tsd_command);
				TxDlen[gun_num]=sizeof(struct TSD_Command);		
				TxRecDlen[gun_num]=TSD_Resp_Len;			
		break;
			
	}
			
}


void Command_Gen_Search(struct TSD_Command * tsd_order, u8 gun_num)
{
		tsd_order->preamble=0xBB;
		tsd_order->len=6;
		tsd_order->rfn=0xFF;
		tsd_order->command=0x89;
		tsd_order->gun_num=gun_num;
		tsd_order->tsd_mode=0;//tsd_mode;
		tsd_order->ret_mode=1;//ret_mode;

		tsd_order->ccd= 0xFF;
		tsd_order->ccd^=0x89;
		tsd_order->ccd^=gun_num;	
		tsd_order->ccd^=0;//tsd_mode;	
		tsd_order->ccd^=1;//ret_mode;			
			
}

void Command_Gen_OTD(struct OTD_Command * otd_order, u8 gun_num,u8 ret_mode)
{
		otd_order->preamble=0xBB;
		otd_order->len=5;
		otd_order->rfn=0xFF;
		otd_order->command=0x86;
		otd_order->gun_num=gun_num;		
		otd_order->ret_mode=ret_mode;//ret_mode;

		otd_order->ccd= 0xFF;
		otd_order->ccd^=0x86;
		otd_order->ccd^=gun_num;	
		otd_order->ccd^=ret_mode;//ret_mode;			
	
			
}

u8 Gas_Data_Storage(u8* data,u16 Dlen,u32* WAddr,u8 gun_num,u8 port_num,u8 data_type)//WAddr��FLASH��ַ���������ǵ�ַ
{
	u8 res;
	u8 *wptr;
	u8 Tlen;
	struct Storage_info storage_info;
	
	Tlen=Dlen+sizeof(struct Storage_info);//�洢����+�洢����״̬��Ϣ�ܳ���
	
	wptr=(u8*)malloc(Tlen);
	
	if(wptr!=0)
	{
		
		storage_info.time_cb=unixtime-CB_Query_Interval;//��ó���ʱ��
		storage_info.data_type=data_type;
		
		storage_info.X_num=gun_num;
		storage_info.X_num=storage_info.X_num<<8;
		storage_info.X_num|=port_num;
		
		storage_info.data_len=Dlen;	
		if(data_storage==1)
		{
//			storage_info.data_status[0]='N';
//			storage_info.data_status[1]='E';
//			storage_info.data_status[2]='W';
			memcpy(storage_info.data_status,"UVWX",4);
		}
		else
		{
//			storage_info.data_status[0]='O';
//			storage_info.data_status[1]='L';
//			storage_info.data_status[2]='D';
			memcpy(storage_info.data_status,"QRSP",4);
		}
		
		memcpy(wptr,data,Dlen);
		memcpy((u8*)(wptr+Dlen),(u8*)&storage_info,sizeof(struct Storage_info));
		
	//	res=sf_WriteBuffer(wptr,*WAddr,Tlen); 
		res=uf_WriteBuffer(FLASH_SST25VF016B,wptr,*WAddr,Tlen);
		
		if(res)
		{
			*WAddr+=Tlen;
			Data_Storage_Num++; 
		//  SEGGER_RTT_printf(0,"Data_Storage_Num:%d, Data_Upload_Num:%d\r\n",Data_Storage_Num,Data_Upload_Num);		
		}
		else
		{
				
			  free(wptr);
			  SEGGER_RTT_printf(0,"data write fail!\r\n");
				return 1;//���ش洢ʧ��
		}
				
	}
	else
	{
	  SEGGER_RTT_printf(0,"mem error!\r\n"); 
		free(wptr);
	   return 2;
	}
	free(wptr);
	return 0;
	
}


u8 Fault_Info_Storage(u32* WAddr,u16 fault_num)//WAddr��FLASH��ַ���������ǵ�ַ
{
		u8 res;
		u8 *wptr;
		u8 Tlen;
	  u32 TimeStamp;
		struct Storage_info storage_info;
		
	  TimeStamp=RTC_GetCounter();
		Tlen=4+sizeof(struct Storage_info);//�洢����״̬��Ϣ�ܳ���
		
    wptr=(u8*)malloc(Tlen);
	
		if(wptr!=0)
		{	
			storage_info.data_type=Fault_Code;//�쳣��Ϣ����������
		
			storage_info.X_num=fault_num;//�쳣���
			storage_info.data_len=4;//ʱ���	
			memcpy(storage_info.data_status,"UVWX",4);
			memcpy(wptr,(u8*)&TimeStamp,4);
		  memcpy((u8*)(wptr+4),(u8*)&storage_info,sizeof(struct Storage_info));
		//	res=sf_WriteBuffer(wptr,*WAddr,Tlen); 
			res=uf_WriteBuffer(FLASH_SST25VF016B,wptr,*WAddr,Tlen);
			if(res)
			{
				*WAddr+=Tlen;
				Data_Storage_Num++;
						
			}
			else
			{
					free(wptr);
					return 1;//���ش洢ʧ��
			}
			
		}		
	
	
	free(wptr);
	return 0;
	
}


u8 LocalData_Upload(u8* data,u32 RAddr) //RAddr��FLASH��ַ
{
	u8 res,i;
	u8 port_num;
	u32 rptr;
  struct Storage_info* sptr;
	struct Protocol_Header *p_head;
	Com_Gun_Binding Com_Gun_Bdinfo_temp;
	
	u8 data_temp[sizeof(struct Storage_info)]={0};
	u8 P_HEAD[sizeof(struct Protocol_Header)]={0};
	u8 err_data=0;
	
	//sf_ReadBuffer(data_temp,RAddr-7,sizeof(struct Storage_info)); //7�ֽں�2�ֽ�ƫ����ǹ�ţ�4�ֽ�ʱ�����1�ֽ���������
	uf_ReadBuffer(FLASH_SST25VF016B,data_temp,RAddr-7,sizeof(struct Storage_info));
	
	sptr=(struct Storage_info* )data_temp;

	
	switch(sptr->data_type)	
	{
		case TAXD_Code:
			if(sptr->data_len!=48)
				err_data=1;
		 break;
		 
		 case OTD_Code:
			if(sptr->data_len!=22)
				err_data=1;
		 break;
		
		 case DSD_Code:
			if(sptr->data_len!=28)
				err_data=1;
		 break;
			
		 case MSD_Code:
			if(sptr->data_len!=26)
				err_data=1;
		 break;
			
		 case TSD_Code:
			if(sptr->data_len!=24)
				err_data=1;
		 break;
			
		 default:
			 	err_data=1;
		 break;
		
	}
	
	if((sptr->X_num&0xFF)>4)//���ںŴ���4Ϊ��������
	  err_data=1;

		
	if(err_data)
	{
		Err_Data++;
		data_flashaddr_index++;
		FlashReadPtrNum--;
		return 1;
	}
		
	
	rptr=RAddr-sptr->data_len-7;//���ݿ�ʼ��ַ,2�ֽ�ƫ����ǹ��,���ںŻ��쳣��+1�ֽ��������+4�ֽ�ʱ���
	uf_ReadBuffer(FLASH_SST25VF016B,data,rptr,sptr->data_len+2+4); //���2�ֽ�ǹ�ţ�4�ֽ�ʱ���
	
	port_num=(u8)sptr->X_num;//��ȡ���ں�
		
//	 SEGGER_RTT_Write(0,data,sptr->data_len+2);
//	 SEGGER_RTT_printf(0,"\r\n");
	p_head=(struct Protocol_Header *)P_HEAD;
//Protocol_Send(p_head, sptr->data_type, 1,1, data, sptr->data_len+2);//�෢2�ֽ�ǹ��,���ںŻ��쳣��

	

/*
	if(tax_data[port_num].code_num[0]==0xFF)
	{
		 
		 //sf_ReadBuffer((u8*)&Com_Gun_Bdinfo_temp,(SpiFlash_TaxCode_ADDR+port_num*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
		 uf_ReadBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_Bdinfo_temp,flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+port_num*sizeof(struct _Com_Gun_Binding),sizeof(struct _Com_Gun_Binding));
		 for(i=0;i<10;i++)
		 {
			 if(Com_Gun_Bdinfo_temp.TaxCode[i]==0xFF)
			 {
				 //while(1)
					{
						SEGGER_RTT_printf(0,"tax_data[%d].code_num is 0xFF,please insert COM%d!\r\n",port_num,port_num);
			      bsp_DelayMS(2000);
					}
				}
		 }
		 
			PLC_Send_Fail_Cnt++;	
			PLC_Send_Cnt++;
			Send_FX++;
			memcpy((data+sptr->data_len+2+4),tax_data[port_num].code_num,10);//���˰�����к�	
			Protocol_Send(p_head, sptr->data_type, 1,1, data, sptr->data_len+2+4+10);//�෢2�ֽ�ǹ��,���ںŻ��쳣��;4�ֽ�ʱ���
	}
	else*/
	{
		PLC_Send_Fail_Cnt++;	
		PLC_Send_Cnt++;
		Send_FX++;
		memcpy((data+sptr->data_len+2+4),tax_data[port_num].code_num,10);//���˰�����к�	
		Protocol_Send(p_head, sptr->data_type, 1,1, data, sptr->data_len+2+4+10);//�෢2�ֽ�ǹ��,���ںŻ��쳣��;4�ֽ�ʱ���
	}

   PLC_Send_T3=SYSTEMTICK;
	
		return 0;
	
}

u8 LocalData_Old(u32 RAddr) //RAddr��FLASH��ַ
{
	u8 res;
  struct Storage_info* sptr;
	u8 data_temp[sizeof(struct Storage_info)]={0};
	
	if(data_storage==1)
	{
	//	sf_ReadBuffer(data_temp,RAddr-3-4,sizeof(struct Storage_info)); //2�ֽ�ǹ�ţ����ںţ�4�ֽ�ʱ�����1�ֽ��������
		uf_ReadBuffer(FLASH_SST25VF016B,data_temp,RAddr-3-4,sizeof(struct Storage_info)); //2�ֽ�ǹ�ţ����ںţ�4�ֽ�ʱ�����1�ֽ��������
		
		sptr=(struct Storage_info* )data_temp;		
		
		if((memcmp(sptr->data_status,NEW,4)==0))//δ�ϴ��������ݳ�����ȷ������ȡ��
		{
//			sptr->data_status[0]='O';//������״̬�ĳ����ϴ�
//			sptr->data_status[1]='L';
//			sptr->data_status[2]='D';
			memcpy(sptr->data_status,"QRSP",4);
			
		//	res=sf_WriteBuffer(sptr->data_status,RAddr,3);
			res=uf_WriteBuffer(FLASH_SST25VF016B,sptr->data_status,RAddr,sizeof(OLD));
			
			if(res)
				return 0;
			else
				return 2;
		}
		else
			return 2;
	}
	else
		return 0;//0��ʾ�Ѿ�old���ˣ��˴�ֻΪ����
		
	
	
}



u16 FlashAddr_Search(u32 SearchAddr,u32 SearchLen,u32* RAddrBuf,u32* SearchNum,u8 Search_Final) //���һ������
{
	u8 data_temp_buf[4096+4];//��Ҫ������ַ
	u8* ptr_data;
  u16 index=0;
	u32 pos=0;
	u8 res=0,res1=0;
	u16 old_cnt=0;
	
	
	*SearchNum=0;
	//sf_ReadBuffer(data_temp_buf,SearchAddr,SearchLen);
	uf_ReadBuffer(FLASH_SST25VF016B,data_temp_buf,SearchAddr,SearchLen);
	ptr_data=data_temp_buf;

	while(1)
	{
		
		res=memcmp(ptr_data,NEW,4);//NEW
		res1=memcmp(ptr_data,OLD,4);//OLD
		if(!res) //����NEW��ʶ����
		{
			   if(data_storage==1)
				 {
						pos=ptr_data-data_temp_buf;
						*RAddrBuf=SearchAddr+pos;
					 
					  if(!Search_Final)
						  RAddrBuf++;
				 }
				 
			  (*SearchNum)++;//������1
			 
		}	
		
		if(!res1)//����OLD��ʶ����
		{
		   //��old���������������ٴ��ϴ����˴�ֻΪ����
			  if(data_storage==0)
				{
					pos=ptr_data-data_temp_buf;
					*RAddrBuf=SearchAddr+pos;
					
				  if(!Search_Final)
					  RAddrBuf++;	
				}
			  old_cnt++;//������1	 
		}		
		
		index++;
		ptr_data++;

		if((index+5)>SearchLen)//"NEW"���+1�ֽ����ݳ���,��Ӧ"NEW"���������4096�����߽紦�����
		 break;
	}
	
	return old_cnt;
	
}



u8 Gas_Data_Process(u8* RXBuf,Com_Status *PORTn)
{
	struct Gas_Data_Head *gas_data_head;
//	struct Day_Sum_Data dsd_data;
	
	u8 check_code=0;
	u8 XOR_code=0,XOR_code2=0;
	u8 res=0;
	u8 *dptr,*dptr2;//�����ݴ�ָ��
	u8 Port_Num;
	u8 Gun_Num,Gun_A,Gun_B,Gun_Num0;
	u8 GunCache,GunCorrect;
	u8 GunA_OnQ;//��ǰǹA�Ƿ����ڱ���ѯ
	u8 First_Gun;//�����һǹ��־
  u8 i,j,k;
	u8 Dlen,Money_Same,DSD_Same;
  u8 New_Day;
	static u8 New_Mon=0;//New_MonA=0,New_MonB=0;
//	static u8 CB_MSD_ASame=0,CB_MSD_BSame=0;
//	static u8 TSD_Same=0;
	
	s32 money[MAX_COUNT],oil[MAX_COUNT];
	u32 SampleTime_New;
	s32 oil_s,t_s;//�����ͼ���ʱ�̵���ʱ����

	struct Day_Sum_Data   *dsd_ptr_0;
	struct Total_Sum_Data *tsd_ptr0,*tsd_ptrx;
	
	char data_convert[13]={0};
	char price_convert[5]={0};
	
	char zero[24]={0};
	char zero_price[4]={'0','0','0','0'};
	
	u8 Gun_cnt=0;
	u8 CB_Change=1;
	u8 OilDataNeedCorrect=0;
	u8 OilDataInitOk=0;

	gas_data_head=(struct Gas_Data_Head *)RXBuf;
	check_code=*(RXBuf+gas_data_head->len+1);
	
  
	for(int i=0;i<gas_data_head->len-1;i++)
	{
		 XOR_code^=*(RXBuf+i+2);
	}
	
	if(check_code!=XOR_code)
	{
	  SEGGER_RTT_printf(0,"Check error1!\r\n");
		
		TaxRBuffer[PORTn->COM_Num].rptr=0;
		TaxRBuffer[PORTn->COM_Num].wptr=0;
		
		
//		 bsp_DelayMS(2000);	
		return 1;
	}		
	else  //У��ͨ��
	{
		
			if(gas_data_head->command==TAXD_Code)
			{
				Port_Num=PORTn->COM_Num;
				tax_data_x=(struct Tax_Data_X *)(RXBuf);
				
				PORTn->TaxDataQuery_Flag=0;//���˰����Ϣ��ѯ��־
				
				PORTn->Logic_Connect_State=1;		
				
//				PORTn->Gun_A.Gun_State=GUN_STATE_GUN_IDLE;
//				PORTn->Gun_B.Gun_State=GUN_STATE_GUN_IDLE;
				
				if(tax_data_x->code_num[0]>0x39) //��̩��˰��
				{
					   OTD_Resp_Len= 30; //��̩��
					   KTK=1;
					   if(tax_data_x->code_num[0]=='A')
						 {
							 PORTn->Gun_B.gun_num=PORTn->Gun_A.gun_num;
							 PORTn->Gun_B.Gun_State=GUN_STATE_IDLE;
							 if(Com_Gun_BindInfo[Port_Num].GunCount!=1)
							 {
								 Com_Gun_BindInfo[Port_Num].GunCount=1;
							   uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[Port_Num],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+Port_Num*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
					     }
							 
						 }
				}
				else
				{
						 OTD_Resp_Len= 28;
					   KTK=0;
					   if(tax_data_x->code_num[0]=='0')
						 {
							 PORTn->Gun_B.gun_num=PORTn->Gun_A.gun_num;
							 PORTn->Gun_B.Gun_State=GUN_STATE_IDLE;
							 if(Com_Gun_BindInfo[Port_Num].GunCount!=1)
							 {
								 Com_Gun_BindInfo[Port_Num].GunCount=1;
							   uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[Port_Num],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+Port_Num*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
					     }
						 }
				}
				
				
				if(tax_data_x->FirstHead.status==0)//����ǹδ˰���ʼ��
				{
					
					PORTn->Gun_A.Init_Status=0;
					PORTn->Gun_B.Init_Status=0;
					
					memcpy(tax_data[Port_Num].code_num,tax_data_x->code_num,10);
					
					SEGGER_RTT_printf(0,"Port%d TaxCode: ",Port_Num);
					for(i=0;i<10;i++)
					{
						SEGGER_RTT_printf(0,"%c",tax_data[Port_Num].code_num[i]);
					}
					SEGGER_RTT_printf(0,"\r\n");
					
					if(Com_Gun_BindInfo[Port_Num].GunCount==1)
					  CB_OTDData_Disp(&CB_PORT[Port_Num],0);	//�������1��Ӧ��ҲҪ������ʾOTD����
					else if(Com_Gun_BindInfo[Port_Num].GunCount==2)
					{
						CB_OTDData_Disp(&CB_PORT[Port_Num],0);	
						CB_OTDData_Disp(&CB_PORT[Port_Num],1);	
					}
					
				//	return 2;
				}
				else
				{
//					PORTn->Gun_A.gun_num=tax_data_x->gun_code[0];
//					PORTn->Gun_A.Init_Status=1;
//					PORTn->Gun_B.gun_num=tax_data_x->gun_code[1];
//					PORTn->Gun_B.Init_Status=1;
					
					for(int i=0;i<tax_data_x->SecHead.len-1;i++)
					{
						 XOR_code2^=*((&tax_data_x->SecHead.rfn)+i);
					}
					
						if(KTK==1)	
						{
							if(XOR_code2!=tax_data_x->check_code2)
							{
									SEGGER_RTT_printf(0,"KTK TaxInfo Check error2!\r\n");
							}
							else
							{
								
								memcpy(tax_data[Port_Num].code_num,tax_data_x->code_num,10);
								
		//						tax_data[Port_Num].gun_code[0]=tax_data_x->gun_code[0];//ǹ��1
		//						tax_data[Port_Num].gun_code[1]=tax_data_x->gun_code[1];//ǹ��2
								
								memcpy(tax_data[Port_Num].tax_code,tax_data_x->tax_code,19);
								tax_data[Port_Num].tax_code[19]=tax_data_x->tax_code_tail;
								
								memcpy(tax_data[Port_Num].oil_num,tax_data_x->oil_num,4);
								memcpy(tax_data[Port_Num].year,tax_data_x->year,12);//�������ꡢ�¡��ա�ʱ����
								
								memcpy(CB_Time.year,tax_data_x->year,4);//����ȫ��ʱ�����
								memcpy(CB_Time.month,tax_data_x->month,2);
								memcpy(CB_Time.day,tax_data_x->day,2);
								memcpy(CB_Time.hour,tax_data_x->hour,2);
								memcpy(CB_Time.minute,tax_data_x->minute,2);
											
							/*	
								DATE.tm_year=atol(CB_Time.year)-1900;
								DATE.tm_mon=atol(CB_Time.month)-1;
								DATE.tm_mday=atol(CB_Time.day);
								DATE.tm_hour=atol(CB_Time.hour)-8;//ʱ��-8
								DATE.tm_min=atol(CB_Time.minute);
								DATE.tm_sec= 30;//ȡ30��
								*/
						//		unixtime=mktime(&DATE);
						//		RTC_Config();
													
								SEGGER_RTT_printf(0,"PORT:%d, Tax Info:",PORTn->COM_Num);
								SEGGER_RTT_Write(0,(u8*)&tax_data[Port_Num],sizeof(struct Tax_Data));
								SEGGER_RTT_printf(0,"\r\n");
										
							//Gas_Data_Storage((u8*)&tax_data[Port_Num],sizeof(struct Tax_Data),&SpiFlash_DataStorageAddr,0,Port_Num,TAXD_Code);	
								Gas_Data_Storage((u8*)&tax_data[Port_Num],sizeof(struct Tax_Data),&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,0,Port_Num,TAXD_Code);			
						   
				
								if(Com_Gun_BindInfo[Port_Num].GunCount==1)
								  CB_OTDData_Disp(&CB_PORT[Port_Num],0);	//�������1��Ӧ��ҲҪ������ʾOTD����
								else if(Com_Gun_BindInfo[Port_Num].GunCount==2)
								{
									CB_OTDData_Disp(&CB_PORT[Port_Num],0);	
									CB_OTDData_Disp(&CB_PORT[Port_Num],1);	
								}											
								
							}					
						
						}
						else
						{
							if(XOR_code2!=tax_data_x->check_code2)//������˾���ͻ�˰����ϢУ���λtax_data_x->check_code4
							{
									SEGGER_RTT_printf(0,"YTSF TaxInfo Check error2!\r\n");
							}
							else
							{
								
								memcpy(tax_data[Port_Num].code_num,tax_data_x->code_num,10);
								
		//						tax_data[Port_Num].gun_code[0]=tax_data_x->gun_code[0];//ǹ��1
		//						tax_data[Port_Num].gun_code[1]=tax_data_x->gun_code[1];//ǹ��2
								
								memcpy(tax_data[Port_Num].tax_code,tax_data_x->tax_code,19);
								tax_data[Port_Num].tax_code[19]=tax_data_x->tax_code_tail;
								
								memcpy(tax_data[Port_Num].oil_num,tax_data_x->oil_num,4);
								memcpy(tax_data[Port_Num].year,tax_data_x->year,12);//�������ꡢ�¡��ա�ʱ����
								
								memcpy(CB_Time.year,tax_data_x->year,4);//����ȫ��ʱ�����
								memcpy(CB_Time.month,tax_data_x->month,2);
								memcpy(CB_Time.day,tax_data_x->day,2);
								memcpy(CB_Time.hour,tax_data_x->hour,2);
								memcpy(CB_Time.minute,tax_data_x->minute,2);
											
							/*	
								DATE.tm_year=atol(CB_Time.year)-1900;
								DATE.tm_mon=atol(CB_Time.month)-1;
								DATE.tm_mday=atol(CB_Time.day);
								DATE.tm_hour=atol(CB_Time.hour)-8;//ʱ��-8
								DATE.tm_min=atol(CB_Time.minute);
								DATE.tm_sec= 30;//ȡ30��
								*/
						//		unixtime=mktime(&DATE);
						//		RTC_Config();
													
								SEGGER_RTT_printf(0,"PORT:%d, Tax Info:",PORTn->COM_Num);
								SEGGER_RTT_Write(0,(u8*)&tax_data[Port_Num],sizeof(struct Tax_Data));
								SEGGER_RTT_printf(0,"\r\n");
										
							//Gas_Data_Storage((u8*)&tax_data[Port_Num],sizeof(struct Tax_Data),&SpiFlash_DataStorageAddr,0,Port_Num,TAXD_Code);	
								Gas_Data_Storage((u8*)&tax_data[Port_Num],sizeof(struct Tax_Data),&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,0,Port_Num,TAXD_Code);			
							 	
								if(Com_Gun_BindInfo[Port_Num].GunCount==1)
								 CB_OTDData_Disp(&CB_PORT[Port_Num],0);	//�������1��Ӧ��ҲҪ������ʾOTD����
								else if(Com_Gun_BindInfo[Port_Num].GunCount==2)
								{
									CB_OTDData_Disp(&CB_PORT[Port_Num],0);	
									CB_OTDData_Disp(&CB_PORT[Port_Num],1);	
								}
							
						  }
					}
					
							
 					// return 0;
				}	
				
					for(i=1;i<=4;i++)
				  {			
					  uf_ReadBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[i],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+i*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
					
					}
					
				  for(i=0;i<10;i++)
				  {
						if(Com_Gun_BindInfo[Port_Num].TaxCode[i]!=0xFF)
							break;				
				  }
					
					if(i==10)//δ��ȡTaxCode,���ѯǹ��
					{	
						 memcpy(Com_Gun_BindInfo[Port_Num].TaxCode,tax_data[Port_Num].code_num,10);
						
						 Gun_cnt=Gun_Num_Search(3,1000);
						 SEGGER_RTT_printf(0,"\r\nSearch Gun Numer=%d\r\n",Gun_cnt);
						
						 for(i=1;i<5;i++)
						 {
							 if(CB_PORT[i].Phy_Connect_State)
								 SEGGER_RTT_printf(0,"COM%d--GunA:%d, GunB:%d\r\n",i,CB_PORT[i].Gun_A.gun_num,CB_PORT[i].Gun_B.gun_num);
						 }
							
				
						uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[Port_Num],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+Port_Num*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
						
					}
					else//�ѻ�ȡTaxCode
					{						
						if(Com_Gun_BindInfo[Port_Num].Gun_NO[0]==0xFF ||Com_Gun_BindInfo[Port_Num].Gun_NO[1]==0xFF)
						 goto Gun_Search;
						
						
						if(memcmp(Com_Gun_BindInfo[Port_Num].TaxCode,tax_data[Port_Num].code_num,10))//�Ѵ洢��taxcode�뵱ǰ��ȡ�Ĳ�ͬ
						{
						 
							for(i=1;i<=4;i++)
							{
								if(CB_PORT[i].Phy_Connect_State)
								{
									if(!memcmp(tax_data[Port_Num].code_num,Com_Gun_BindInfo[i].TaxCode,10))
									{
											
										 if(Com_Gun_BindInfo[i].GunCount==1)
										 {
											 CB_PORT[Port_Num].Gun_A.gun_num=Com_Gun_BindInfo[i].Gun_NO[0];
											 CB_PORT[Port_Num].Gun_A.Gun_State = GUN_STATE_GUN_IDLE;
											 CB_PORT[Port_Num].Gun_B.Gun_State = GUN_STATE_IDLE;
											 Com_Gun_BindInfo[Port_Num].Gun_NO[0]=Com_Gun_BindInfo[i].Gun_NO[0];
											 Com_Gun_BindInfo[Port_Num].GunCount=Com_Gun_BindInfo[i].GunCount;
											 
										 }
										 else if(Com_Gun_BindInfo[i].GunCount==2)
										 {
											 	CB_PORT[Port_Num].Gun_A.gun_num=Com_Gun_BindInfo[i].Gun_NO[0];
												CB_PORT[Port_Num].Gun_B.gun_num=Com_Gun_BindInfo[i].Gun_NO[1];
											 
											  CB_PORT[Port_Num].Gun_A.Gun_State = GUN_STATE_GUN_IDLE;
									      CB_PORT[Port_Num].Gun_B.Gun_State = GUN_STATE_GUN_IDLE;
												
												Com_Gun_BindInfo[Port_Num].Gun_NO[0]=Com_Gun_BindInfo[i].Gun_NO[0];
												Com_Gun_BindInfo[Port_Num].Gun_NO[1]=Com_Gun_BindInfo[i].Gun_NO[1];		
											 
											 	Com_Gun_BindInfo[Port_Num].GunCount=Com_Gun_BindInfo[i].GunCount;										 
											
										 }
																																																
											CB_Change=0;					
											SEGGER_RTT_printf(0,"Port_Num:%d-->COM:%d,GunCount=%d!\r\n",Port_Num,i,Com_Gun_BindInfo[i].GunCount);
											
									}
//									else
//									{
//										if(!memcmp(tax_data[Port_Num].code_num,tax_data[i].code_num,10))
//									  {
//																					
//											CB_PORT[Port_Num].Gun_A.gun_num=CB_PORT[i].Gun_A.gun_num;
//											CB_PORT[Port_Num].Gun_B.gun_num=CB_PORT[i].Gun_B.gun_num;
//											
//											Com_Gun_BindInfo[Port_Num].Gun_NO[0]=CB_PORT[i].Gun_A.gun_num;
//											Com_Gun_BindInfo[Port_Num].Gun_NO[1]=CB_PORT[i].Gun_B.gun_num;																				
//											
//											CB_Change=0;					
//											SEGGER_RTT_printf(0,"Port_Num:%d-->COM:%d TaxCode Find in Ram!\r\n",Port_Num,i);
//											
//									  }
//									}
								}
							}
							
							if(CB_Change)//���������
							{
Gun_Search:								
								Gun_cnt=Gun_Num_Search(3,1000);
								 SEGGER_RTT_printf(0,"\r\nSearch Gun Numer=%d\r\n",Gun_cnt);
								
								 for(i=1;i<5;i++)
								 {
									 if(CB_PORT[i].Phy_Connect_State)
										 SEGGER_RTT_printf(0,"COM%d--GunA:%d, GunB:%d\r\n",i,CB_PORT[i].Gun_A.gun_num,CB_PORT[i].Gun_B.gun_num);
								 }
							}
							
 							 memcpy(Com_Gun_BindInfo[Port_Num].TaxCode,tax_data[Port_Num].code_num,10);						
				
							uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[Port_Num],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+Port_Num*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
						
						}
						else
						{
																			  
							   if(Com_Gun_BindInfo[Port_Num].GunCount==1)
								 {
									 CB_PORT[Port_Num].Gun_A.gun_num = Com_Gun_BindInfo[Port_Num].Gun_NO[0];
									 CB_PORT[Port_Num].Gun_A.Gun_State = GUN_STATE_GUN_IDLE;
									 CB_PORT[Port_Num].Gun_B.Gun_State = GUN_STATE_IDLE;
								 }
								 else if(Com_Gun_BindInfo[Port_Num].GunCount==2)
								 {
									 CB_PORT[Port_Num].Gun_A.gun_num = Com_Gun_BindInfo[Port_Num].Gun_NO[0];
								   CB_PORT[Port_Num].Gun_B.gun_num = Com_Gun_BindInfo[Port_Num].Gun_NO[1];
									 CB_PORT[Port_Num].Gun_A.Gun_State = GUN_STATE_GUN_IDLE;
									 CB_PORT[Port_Num].Gun_B.Gun_State = GUN_STATE_GUN_IDLE;
									 
								 }
							  						
						}

				  }
				
			}
			
			
			if(gas_data_head->command==OTD_Code)//���μ�������
			{
								 			
				
				 if(PORTn->Gun_A.OnQuery_Flag==1)					 
				 {
					  Gun_Num=PORTn->Gun_A.gun_num;
					  PORTn->Gun_A.Gun_State=GUN_STATE_GUN_IDLE;
				 }
				 else 
				 {				 
					  Gun_Num=PORTn->Gun_B.gun_num;
					  PORTn->Gun_B.Gun_State=GUN_STATE_GUN_IDLE;
				 }
				 
				 
				 Dlen=gas_data_head->len;
				 otd_ptr=(struct One_Time_Data *)(RXBuf+5);
				 otd_ptrx=&OTData[PORTn->COM_Num][Gun_Num];

				
				   if(PORTn->Gun_A.OnQuery_Flag==1)	
				   {						
						 Gun_Num=PORTn->Gun_A.gun_num;	
					
						if(!memcmp(zero,PORTn->Gun_A.Price,4))
							memcpy(PORTn->Gun_A.Price,otd_ptr->price,4);
						else
						{
							if(!memcmp(PORTn->Gun_A.Price,otd_ptr->price,4))
								 PORTn->Gun_A.Price_Same_Cnt++;
							else
							{
								 memcpy(PORTn->Gun_A.Price,otd_ptr->price,4);
								 PORTn->Gun_A.Price_Same_Cnt=0;
							}
						}
						
						SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d,SameCnt:%d,OTD :",PORTn->COM_Num,Gun_Num,PORTn->Gun_A.Price_Same_Cnt);
						
						if(KTK==1)
							SEGGER_RTT_Write(0,(u8*)otd_ptr,(Dlen-6));
						 else
							SEGGER_RTT_Write(0,(u8*)otd_ptr,(Dlen-4));
																	
						SEGGER_RTT_printf(0," $$$$$$\r\n");

						if(PORTn->Gun_A.Price_Same_Cnt==2)//������3�λ����ͬ��OTD����
						{
							PORTn->Gun_A.Price_Query_Flag=0;
							PORTn->Gun_A.Price_Same_Cnt=0;
							PORTn->Gun_A.TSumDataQuery_Flag=1;
						//	memcpy((dsd_data.price),PORTn->Gun_A.Price,4);
																																		
							memcpy(price_convert,PORTn->Gun_A.Price,4);
							memcpy(otd_ptrx->price,PORTn->Gun_A.Price,4);
							oildata_analysis[PORTn->COM_Num][Gun_Num].price=atol(price_convert);
							
							SEGGER_RTT_printf(0,"COM:%d ++ Gun Num:%d,OTD:",PORTn->COM_Num,Gun_Num);
					    SEGGER_RTT_Write(0,(u8*)otd_ptrx,sizeof(struct One_Time_Data));							
					    SEGGER_RTT_printf(0," Data Saved! \r\n");
						
	            
							Gas_Data_Storage((u8*)otd_ptrx,sizeof(struct One_Time_Data),&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,Gun_Num,PORTn->COM_Num,OTD_Code);			
							
						}
						
					
					}						
					else 
					{
					   Gun_Num=PORTn->Gun_B.gun_num;		

						if(!memcmp(zero,PORTn->Gun_B.Price,4))
							memcpy(PORTn->Gun_B.Price,otd_ptr->price,4);
						else
						{
							if(!memcmp(PORTn->Gun_B.Price,otd_ptr->price,4))
								 PORTn->Gun_B.Price_Same_Cnt++;
							else
							{
								 memcpy(PORTn->Gun_B.Price,otd_ptr->price,4);
								 PORTn->Gun_B.Price_Same_Cnt=0;
							}
						}	
						
						SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d,SameCnt:%d,OTD :",PORTn->COM_Num,Gun_Num,PORTn->Gun_B.Price_Same_Cnt);
						
						if(KTK==1)
							SEGGER_RTT_Write(0,(u8*)otd_ptr,(Dlen-6));
						 else
							SEGGER_RTT_Write(0,(u8*)otd_ptr,(Dlen-4));
																	
						SEGGER_RTT_printf(0," $$$$$$\r\n");
						 
						if(PORTn->Gun_B.Price_Same_Cnt==2)//������3�λ����ͬ��OTD����
						{
							PORTn->Gun_B.Price_Query_Flag=0;
							PORTn->Gun_B.Price_Same_Cnt=0;
							PORTn->Gun_B.TSumDataQuery_Flag=1;
						//	memcpy((dsd_data.price),PORTn->Gun_B.Price,4);					
					 //Gas_Data_Storage((u8*)(&dsd_data),sizeof(struct Day_Sum_Data),&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,Gun_Num,PORTn->COM_Num,DSD_Code);			
							
							memcpy(price_convert,PORTn->Gun_B.Price,4);	
							memcpy(otd_ptrx->price,PORTn->Gun_B.Price,4);							
							oildata_analysis[PORTn->COM_Num][Gun_Num].price=atol(price_convert);//��ȡ���µ���
						
							SEGGER_RTT_printf(0,"COM:%d ++ Gun Num:%d,OTD:",PORTn->COM_Num,Gun_Num);
					    SEGGER_RTT_Write(0,(u8*)otd_ptrx,sizeof(struct One_Time_Data));							
					    SEGGER_RTT_printf(0," Data Saved! \r\n");				
							
							Gas_Data_Storage((u8*)otd_ptrx,sizeof(struct One_Time_Data),&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,Gun_Num,PORTn->COM_Num,OTD_Code);			
							
						}
						
					}										
					
				}
					

			if(gas_data_head->command==DSD_Code)//���ۼƼ�������
			{
				 
			
				  t_stamp=unixtime+28800;//��8Сʱʱ��
					ASCTIME2=localtime((time_t *)&t_stamp);	//��ȡ��ǰʱ��
					
					if(ASCTIME2->tm_mday!=CB_Day)
						 New_Day=1;
					else
						 New_Day=0;
										
					
					if(ASCTIME2->tm_mon!=CB_Month)
					{
//						PORTn->Gun_A.DSumDataQuery_Flag=0;
//						PORTn->Gun_B.DSumDataQuery_Flag=0;
//						
//						PORTn->Gun_A.MSumDataQuery_Flag=1;
//						PORTn->Gun_B.MSumDataQuery_Flag=1;
						
						for(i=1;i<=4;i++)
						{         
							CB_PORT[i].Gun_A.DSumDataQuery_Flag=0;
							CB_PORT[i].Gun_B.DSumDataQuery_Flag=0;
							
							CB_PORT[i].Gun_A.MSumDataQuery_Flag=1;
							CB_PORT[i].Gun_B.MSumDataQuery_Flag=1;
							
							for(j=0;j<=OnePortGunNum;j++)
							{
								memset((u8*)&day_sum_data[i][j],0,sizeof(struct Day_Sum_Data));//���㻺������
								memset((u8*)&month_sum_data[i][j],0,sizeof(struct Month_Sum_Data));//���㻺������
							}
																				
						}
						
						New_Mon=1;
					}
									
					
					if(New_Day)//�����
					{
					   //��������	
						CB_Year=ASCTIME2->tm_year;
						CB_Month=ASCTIME2->tm_mon;
						CB_Day=ASCTIME2->tm_mday;			
					
						if(!New_Mon)
						{
							sprintf(CB_Time.year,"%4d",ASCTIME2->tm_year+1900);//����ȫ��ʱ�����
							sprintf(CB_Time.month,"%2d",ASCTIME2->tm_mon+1);
							sprintf(CB_Time.day,"%2d",ASCTIME2->tm_mday);	
						}
																	      						
					}	
					
				  
				 if(PORTn->Gun_A.OnQuery_Flag==1)					 
				 {
						Gun_Num=PORTn->Gun_A.gun_num;
						PORTn->Gun_A.Gun_State=GUN_STATE_GUN_IDLE;
				 }
				 else 
				 {				 
						Gun_Num=PORTn->Gun_B.gun_num;
						PORTn->Gun_B.Gun_State=GUN_STATE_GUN_IDLE;
				 }				
		
					dsd_ptr=(struct Day_Sum_Data *)(RXBuf+5);	
					
					if(gas_data_head->status==0)//��ǹ����δ���� ytsf˰��
					{
													
						 	if(KTK==1)
						    SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d--KTK DSD ERR!\r\n",PORTn->COM_Num,Gun_Num);
							else
								SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d--YTSF DSD NO Data!\r\n",PORTn->COM_Num,Gun_Num);
							
						  return 2;
							
					}
					
					if(!memcmp(dsd_ptr->price,zero_price,4))
					{
					
						SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d--DSD :",PORTn->COM_Num,Gun_Num);
						SEGGER_RTT_Write(0,(u8*)dsd_ptr,(gas_data_head->len-4));
						SEGGER_RTT_printf(0,"\r\n");
						
						SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d--ZERO PRICE!\r\n",PORTn->COM_Num,Gun_Num);
						return 3;
					}
					
					SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d--DSD :",PORTn->COM_Num,Gun_Num);
					SEGGER_RTT_Write(0,(u8*)dsd_ptr,(gas_data_head->len-4));
					SEGGER_RTT_printf(0,"\r\n");
				//Gas_Data_Storage((u8*)(RXBuf+5),sizeof(struct Day_Sum_Data),&SpiFlash_DataStorageAddr,Gun_Num,PORTn->COM_Num,DSD_Code);																
					Gas_Data_Storage((u8*)(RXBuf+5),sizeof(struct Day_Sum_Data),&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,Gun_Num,PORTn->COM_Num,DSD_Code);			
							
						
			}
			
			
				
			if(gas_data_head->command==MSD_Code)//���ۼ�������
			{
									
  				if(PORTn->Gun_A.OnQuery_Flag==1)
					{
//				       if(New_Mon==0)
//						  PORTn->Gun_A.MSumDataQuery_Flag=0;//������ۼƼ�������	
						 
						 Gun_Num=PORTn->Gun_A.gun_num;
						PORTn->Gun_A.Gun_State=GUN_STATE_GUN_IDLE;
						
					}						
					else 
					{
//				        if(New_Mon==0)
//							PORTn->Gun_B.MSumDataQuery_Flag=0;				
						 
						
						 Gun_Num=PORTn->Gun_B.gun_num;
						 PORTn->Gun_B.Gun_State=GUN_STATE_GUN_IDLE;
												
					}
					
				  dptr=(u8*)&month_sum_data[PORTn->COM_Num][Gun_Num];//���ϴθ�ǹ��ѯ����MSD����ȡ�����бȽ�	

					if(month_sum_data[PORTn->COM_Num][Gun_Num].oil_quantity==0)
					{
						memcpy((u8*)dptr,(u8*)(RXBuf+5),sizeof(struct Month_Sum_Data));
						return 0;
					}
											
				  msd_ptr=(struct Month_Sum_Data *)(RXBuf+5);
					res=memcmp(msd_ptr,dptr,sizeof(struct Month_Sum_Data));
					
					if(res)//���ϴ����ݲ�ͬ
					{	
						memcpy((u8*)dptr,(u8*)(RXBuf+5),sizeof(struct Month_Sum_Data));
						Data_Need_Storage=0;
						
						if(PORTn->Gun_A.OnQuery_Flag==1)
						  PORTn->Gun_A.MSD_Same_Cnt=0;
						else
							PORTn->Gun_B.MSD_Same_Cnt=0;
										
					}
					else
					{
						if(PORTn->Gun_A.OnQuery_Flag==1)
						  PORTn->Gun_A.MSD_Same_Cnt++;
						else
							PORTn->Gun_B.MSD_Same_Cnt++;
						
						
						if(PORTn->Gun_A.MSD_Same_Cnt==5)
						{
						  Data_Need_Storage=1;				
						  PORTn->Gun_A.MSumDataQuery_Flag=0;//������ۼƼ�������
							PORTn->Gun_A.TSumDataQuery_Flag=1;							
							
						}					
						else if(PORTn->Gun_B.MSD_Same_Cnt==5)
						{
						  Data_Need_Storage=1;			
							PORTn->Gun_B.MSumDataQuery_Flag=0;//������ۼƼ�������	
							PORTn->Gun_B.TSumDataQuery_Flag=1;
						}
						else
							Data_Need_Storage=0;
						
						
						if(PORTn->Gun_A.MSD_Same_Cnt==5 && PORTn->Gun_B.MSD_Same_Cnt==5)
						{
							
							New_Mon=0;
							
							PORTn->Gun_A.MSD_Same_Cnt=0;
							PORTn->Gun_B.MSD_Same_Cnt=0;
												
														
							sprintf(CB_Time.year,"%4d",ASCTIME2->tm_year+1900);//����ȫ��ʱ�����
							sprintf(CB_Time.month,"%2d",ASCTIME2->tm_mon+1);
							sprintf(CB_Time.day,"%2d",ASCTIME2->tm_mday);	
							
						}				
						
					}				
					
					SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d--MSD :",PORTn->COM_Num,Gun_Num);
					SEGGER_RTT_Write(0,(u8*)dptr,(gas_data_head->len-4));
						//д��flash
					if(Data_Need_Storage==1)
					{
						SEGGER_RTT_printf(0," Data Saved! \r\n");
					  Data_Need_Storage=0;
					//Gas_Data_Storage((u8*)dptr,sizeof(struct Month_Sum_Data),&SpiFlash_DataStorageAddr,Gun_Num,PORTn->COM_Num,MSD_Code);	
						Gas_Data_Storage((u8*)dptr,sizeof(struct Month_Sum_Data),&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,Gun_Num,PORTn->COM_Num,MSD_Code);			
				
					}
					else
					{
						SEGGER_RTT_printf(0," Data don't Saved!\r\n");
				
					}
					
					return 0;
					
				
			}
			
			if(gas_data_head->command==TSD_Code)//���ۼƼ�������
			{
							
			  t_stamp=unixtime+28800;//��8Сʱʱ��
				ASCTIME2=localtime((time_t *)&t_stamp);	//��ȡ��ǰʱ��
				
				if(ASCTIME2->tm_mday!=CB_Day)
					 New_Day=1;
				else
					 New_Day=0;
									
				if(ASCTIME2->tm_mon!=CB_Month)
				{
					
					for(i=1;i<=4;i++)
					{         
						CB_PORT[i].Gun_A.TSumDataQuery_Flag=0;
						CB_PORT[i].Gun_B.TSumDataQuery_Flag=0;
						
						CB_PORT[i].Gun_A.MSumDataQuery_Flag=1;
						CB_PORT[i].Gun_B.MSumDataQuery_Flag=1;
						
						for(j=0;j<=OnePortGunNum;j++)
						{						
							memset((u8*)&month_sum_data[i][j],0,sizeof(struct Month_Sum_Data));//���㻺������
						}
																			
					}
					
					 New_Mon=1;		
				}
					
				if(New_Day)//�����
				{
				   //��������	
					CB_Year=ASCTIME2->tm_year;
					CB_Month=ASCTIME2->tm_mon;
					CB_Day=ASCTIME2->tm_mday;			
				
					if(!New_Mon)
					{
						sprintf(CB_Time.year,"%4d",ASCTIME2->tm_year+1900);//����ȫ��ʱ�����
						sprintf(CB_Time.month,"%2d",ASCTIME2->tm_mon+1);
						sprintf(CB_Time.day,"%2d",ASCTIME2->tm_mday);	
					}
																						
				}					
				
   			if(PORTn->Gun_A.OnQuery_Flag==1)
				{			
				    Gun_Num=PORTn->Gun_A.gun_num;	
				    PORTn->Gun_A.Gun_State=GUN_STATE_GUN_IDLE;	
            OilDataInitOk=PORTn->Gun_A.OilDataInitOk;
					
				}						
				else 
				{
					Gun_Num=PORTn->Gun_B.gun_num;			
					PORTn->Gun_B.Gun_State=GUN_STATE_GUN_IDLE;
					OilDataInitOk=PORTn->Gun_B.OilDataInitOk;					
				}
							
/*************************************************************************************
		    *���������ж�����*
				
				 
*************************************************************************************/
				
				tsd_ptr=(struct Total_Sum_Data *)(RXBuf+5);	//���βɼ�������������																						
				tsd_ptr0=&total_sum_data[PORTn->COM_Num][Gun_Num][THIS];//���ϴθ�ǹ��ѯ����TSD����ȡ�����бȽ�
				
				memcpy(data_convert,tsd_ptr->oil_quantity,12);								
				oil[THIS]=atol(data_convert);	//�²ɼ����ļ�����
				memcpy(data_convert,tsd_ptr->money,12);								
				money[THIS]=atol(data_convert);	//�²ɼ����ļ��ͽ��	
				
				SampleTime_New=unixtime-CB_Query_Interval;
				
				res=memcmp(tsd_ptr,tsd_ptr0,sizeof(struct Total_Sum_Data));
				
				if(!res && OilDataInitOk)//���ϴ�������ͬ				
				{																				
						
					  if(PORTn->Gun_A.OnQuery_Flag==1)
						{
							  PORTn->Gun_A.TSD_Same_Cnt++;
							
								if(PORTn->Gun_A.TSD_Same_Cnt==1)
								{
									total_sum_data[PORTn->COM_Num][Gun_Num][LAST]=total_sum_data[PORTn->COM_Num][Gun_Num][THIS];
									res=OTData_Handle(&oildata_analysis[PORTn->COM_Num][Gun_Num],&OTData[PORTn->COM_Num][Gun_Num]);
								
									 if(res==1)
										 Data_Need_Storage=1;
									 
									  SEGGER_RTT_printf(0,"**************************** Calc_OTDATA Res:%d\r\n",res);	
									 
								}	
							
						}							
						else
						{						
								PORTn->Gun_B.TSD_Same_Cnt++;				
								
								if(PORTn->Gun_B.TSD_Same_Cnt==1)
								{
									total_sum_data[PORTn->COM_Num][Gun_Num][LAST]=total_sum_data[PORTn->COM_Num][Gun_Num][THIS];
									res=OTData_Handle(&oildata_analysis[PORTn->COM_Num][Gun_Num],&OTData[PORTn->COM_Num][Gun_Num]);
								
									 if(res==1)
										 Data_Need_Storage=1;
									 
									  SEGGER_RTT_printf(0,"**************************** Calc_OTDATA Res:%d\r\n",res);	
									 
								}								
								
						}							
											
						
						{//TSD_Same_Cnt>1,���ü���OTD����Ϊû���ͣ����²ɼ�ʱ��
							
							Update_TSDList(&oildata_analysis[PORTn->COM_Num][Gun_Num],oil[THIS],money[THIS],SampleTime_New);																	 													
							if(oildata_analysis[PORTn->COM_Num][Gun_Num].optgun_cnt>0)
							{
								oildata_analysis[PORTn->COM_Num][Gun_Num].optgun_cnt=0;
								GunCorrect=oildata_analysis[PORTn->COM_Num][Gun_Num].optgun[0];
								total_sum_data[PORTn->COM_Num][GunCorrect][THIS]=total_sum_data[PORTn->COM_Num][GunCorrect][LAST];
							  SEGGER_RTT_printf(0,"----------------------- Correct_TSDList!\r\n");	
								 Correct++;
							}
					
						}					
					
				}
				else
				{	
					
			 
					if(OilDataInitOk==0)//���ۼƻ���������
					{
					 
						if(!memcmp(zero,tsd_ptr0,sizeof(struct Total_Sum_Data)))
						{
							memcpy(tsd_ptr0,tsd_ptr,sizeof(struct Total_Sum_Data));
							total_sum_data[PORTn->COM_Num][Gun_Num][LAST]=total_sum_data[PORTn->COM_Num][Gun_Num][THIS];
						}
						else if(!res)//�Ƚϵ����������ϴ�������ͬ
						{
							
							 Data_Need_Storage=0;	
							
							 if(PORTn->Gun_A.OnQuery_Flag==1)
									PORTn->Gun_A.TSD_Same_Cnt++;
								else
									PORTn->Gun_B.TSD_Same_Cnt++;
								
								
								if(PORTn->Gun_A.TSD_Same_Cnt==3 && PORTn->Gun_A.OnQuery_Flag)
								{													
									 oildata_analysis[PORTn->COM_Num][Gun_Num].gun_num=Gun_Num;
									 oildata_analysis[PORTn->COM_Num][Gun_Num].port_num=PORTn->COM_Num;
									 oildata_analysis[PORTn->COM_Num][Gun_Num].tsd_same_cnt=0;
									 oildata_analysis[PORTn->COM_Num][Gun_Num].oil[THIS]=oil[THIS];
									 oildata_analysis[PORTn->COM_Num][Gun_Num].money[THIS]=money[THIS];
									 oildata_analysis[PORTn->COM_Num][Gun_Num].sample_time[THIS]=SampleTime_New;
									
									// PORTn->Gun_A.TSD_Same_Cnt=0;
									 PORTn->Gun_A.OilDataInitOk=1;
									 SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d,init oildatalist!\r\n",PORTn->COM_Num,Gun_Num);		
								}
								
								if(PORTn->Gun_B.TSD_Same_Cnt==3 && PORTn->Gun_B.OnQuery_Flag)
								{
								
									 oildata_analysis[PORTn->COM_Num][Gun_Num].gun_num=Gun_Num;
									 oildata_analysis[PORTn->COM_Num][Gun_Num].port_num=PORTn->COM_Num;
									 oildata_analysis[PORTn->COM_Num][Gun_Num].tsd_same_cnt=0;
									 oildata_analysis[PORTn->COM_Num][Gun_Num].oil[THIS]=oil[THIS];
									 oildata_analysis[PORTn->COM_Num][Gun_Num].money[THIS]=money[THIS];
									 oildata_analysis[PORTn->COM_Num][Gun_Num].sample_time[THIS]=SampleTime_New;
									
									// PORTn->Gun_B.TSD_Same_Cnt=0;
									 PORTn->Gun_B.OilDataInitOk=1;
									SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d,init oildatalist!\r\n",PORTn->COM_Num,Gun_Num);		
								}
							
								 			
						    goto Data_Storage;
																
						}
						else//δ�����յ���ͬ��TSD����
						{
								if(PORTn->Gun_A.OnQuery_Flag==1)
									PORTn->Gun_A.TSD_Same_Cnt=0;
								else
									PORTn->Gun_B.TSD_Same_Cnt=0;
								
								memcpy(tsd_ptr0,tsd_ptr,sizeof(struct Total_Sum_Data));
							  total_sum_data[PORTn->COM_Num][Gun_Num][LAST]=total_sum_data[PORTn->COM_Num][Gun_Num][THIS];
						}
									
					  					
					}
					else
					{
						
						if(PORTn->Gun_A.OnQuery_Flag==1)
							PORTn->Gun_A.TSD_Same_Cnt=0;
					  else
							PORTn->Gun_B.TSD_Same_Cnt=0;
						
						
					//	total_sum_data[PORTn->COM_Num][Gun_Num][LAST]=total_sum_data[PORTn->COM_Num][Gun_Num][THIS];				
						memcpy(tsd_ptr0,tsd_ptr,sizeof(struct Total_Sum_Data));	
						
						
						j=0;//���ڼ�¼������ͬ��ǹ�ż���
					
						for(i=0;i<Com_Gun_BindInfo[PORTn->COM_Num].GunCount;i++)
						{
							if(Com_Gun_BindInfo[PORTn->COM_Num].Gun_NO[i]!=Gun_Num)								
							{
								GunCache=Com_Gun_BindInfo[PORTn->COM_Num].Gun_NO[i];
								tsd_ptrx=&total_sum_data[PORTn->COM_Num][GunCache][THIS];
								res=memcmp(tsd_ptr,tsd_ptrx,sizeof(struct Total_Sum_Data));
								
								if(!res)
								{
									oildata_analysis[PORTn->COM_Num][Gun_Num].optgun[j]=GunCache;
									oildata_analysis[PORTn->COM_Num][Gun_Num].optgun_cnt++;								
									j++;
									
								}							
								
							}
							
						}
							
						if(oildata_analysis[PORTn->COM_Num][Gun_Num].optgun_cnt>0)//����ͬ���ݵ�ǹ�����㱸ѡǹ��OTD���ݣ������´�ǹ����ʷ����
						{
Calc_OTDATA:							
							GunCache=oildata_analysis[PORTn->COM_Num][Gun_Num].optgun[0];//ѡ������һ��ǹ������		
						
              res=OTData_Handle(&oildata_analysis[PORTn->COM_Num][GunCache],&OTData[PORTn->COM_Num][Gun_Num]);
							
							if(res==1)	
							{								
								Data_Need_Storage=1;
								
							}
							else
							{
								SEGGER_RTT_printf(0,"------------------------GunX! Res:%d\r\n",res);	
							}
							
					    SEGGER_RTT_printf(0,"----------------------- Calc_OTDATA Res:%d\r\n",res);	
						
//							if(GunCache==PORTn->Gun_A.gun_num)
//								 PORTn->Gun_A.TSD_Same_Cnt++;
//					
//							if(GunCache==PORTn->Gun_B.gun_num)
//									PORTn->Gun_B.TSD_Same_Cnt++;
						
								
												
							Update_TSDList( &oildata_analysis[PORTn->COM_Num][GunCache],oil[THIS],money[THIS],SampleTime_New);		
							
							Update_OptGun( &oildata_analysis[PORTn->COM_Num][GunCache],&oildata_analysis[PORTn->COM_Num][Gun_Num]);							
						
																						
						}
						else//�뱾ǹ��LAST���ݽ��бȽ�
						{
							//	tsd_ptrx=&total_sum_data[PORTn->COM_Num][Gun_Num][LAST];
							//	res=memcmp(tsd_ptr,tsd_ptrx,sizeof(struct Total_Sum_Data));		

           							
								if(oildata_analysis[PORTn->COM_Num][Gun_Num].oil[LAST]==oil[THIS] && oildata_analysis[PORTn->COM_Num][Gun_Num].money[LAST]==money[THIS])
								{
																							 									  
									  if(oildata_analysis[PORTn->COM_Num][Gun_Num].optgun_cnt>0)
										{
											 
											 GunCache=oildata_analysis[PORTn->COM_Num][Gun_Num].optgun[0];//ѡ������һ��ǹ������		
											 res=OTData_Handle(&oildata_analysis[PORTn->COM_Num][GunCache],&OTData[PORTn->COM_Num][Gun_Num]);
							
											if(res==1)
											  Data_Need_Storage=1;

//											if(oildata_analysis[PORTn->COM_Num][Gun_Num].oil[THIS]!=oildata_analysis[PORTn->COM_Num][GunCache].oil[THIS])
//											{
//												if(GunCache==PORTn->Gun_A.gun_num)//��ѡǹ�ȼ��ڼ���һ���ͣ�����TSD_Same_Cnt����
//											    PORTn->Gun_A.TSD_Same_Cnt=0;

//											  if(GunCache==PORTn->Gun_B.gun_num)
//												  PORTn->Gun_B.TSD_Same_Cnt=0;
//										  }
//											else
//											{
//												if(GunCache==PORTn->Gun_A.gun_num)//
//											    PORTn->Gun_A.TSD_Same_Cnt++;

//											  if(GunCache==PORTn->Gun_B.gun_num)
//												  PORTn->Gun_B.TSD_Same_Cnt++;
//											}
											

											// Correct++;
											 SEGGER_RTT_printf(0,"----------------------- Correct_TSDList Res:%d ^_^\r\n",res);	
											 Correct_TSDList(&oildata_analysis[PORTn->COM_Num][Gun_Num],
																		   &oildata_analysis[PORTn->COM_Num][GunCache],SampleTime_New);	
											 
											 Update_OptGun( &oildata_analysis[PORTn->COM_Num][GunCache],&oildata_analysis[PORTn->COM_Num][Gun_Num]);													 
											
										}
										else
										{
											 SEGGER_RTT_printf(0,"Flow Ratio Maybe Err0!\r\n");
											 TSDNoMatch0++;
										}
										
//											if(PORTn->Gun_A.OnQuery_Flag==1)//���β�ѯ��ǹTHIS==LAST����Ϊ������ͬ
//												 PORTn->Gun_A.TSD_Same_Cnt++;
//											else
//												 PORTn->Gun_B.TSD_Same_Cnt++;																														
									 								
								}
								else
								{
									
Flow_Estimat:
									  k=0;//���ڼ�¼���ٷ���Ҫ���ǹ����
									  for(i=0;i<Com_Gun_BindInfo[PORTn->COM_Num].GunCount;i++)
										{																																		
												GunCache=Com_Gun_BindInfo[PORTn->COM_Num].Gun_NO[i];
											  if(Com_Gun_BindInfo[PORTn->COM_Num].Gun_NO[i]!=Gun_Num)
												{
													res=Flow_Estimate(&oildata_analysis[PORTn->COM_Num][GunCache],oil[THIS],SampleTime_New,FlowRatio);
												  if(res)
													{
														oildata_analysis[PORTn->COM_Num][Gun_Num].optgun[j]=GunCache;
														oildata_analysis[PORTn->COM_Num][Gun_Num].optgun_cnt++;		
												    k++;	
													}
														
												}
										}
										//���㱾ǹ����	
										res=Flow_Estimate(&oildata_analysis[PORTn->COM_Num][Gun_Num],oil[THIS],SampleTime_New,FlowRatio);
										if(res)
										{
											 res=OTData_Handle(&oildata_analysis[PORTn->COM_Num][Gun_Num],&OTData[PORTn->COM_Num][Gun_Num]);
							
											 if(res==1)
												 Data_Need_Storage=1;
											 
										
//												if(PORTn->Gun_A.OnQuery_Flag==1)
//													 PORTn->Gun_A.TSD_Same_Cnt=0;
//												else
//													 PORTn->Gun_B.TSD_Same_Cnt=0;
													
												SEGGER_RTT_printf(0,"------------------------Flow_Estimat Self! Res:%d\r\n",res);	
											 
											 	Update_TSDList(&oildata_analysis[PORTn->COM_Num][Gun_Num],oil[THIS],money[THIS],SampleTime_New);		
																
											  if(oildata_analysis[PORTn->COM_Num][Gun_Num].optgun_cnt>0)
												{
													
													oildata_analysis[PORTn->COM_Num][Gun_Num].optgun_cnt=0;
													GunCorrect=oildata_analysis[PORTn->COM_Num][Gun_Num].optgun[0];
													total_sum_data[PORTn->COM_Num][GunCorrect][THIS]=total_sum_data[PORTn->COM_Num][GunCorrect][LAST];
													SEGGER_RTT_printf(0,"----------------------- Correct_TSDList!\r\n");	
													Correct++;
												}
																						 
											 
										}
										else
										{
											if(oildata_analysis[PORTn->COM_Num][Gun_Num].optgun_cnt>0)
											{
													GunCache=oildata_analysis[PORTn->COM_Num][Gun_Num].optgun[0];//ѡ������һ��ǹ������		
						
													res=OTData_Handle(&oildata_analysis[PORTn->COM_Num][GunCache],&OTData[PORTn->COM_Num][Gun_Num]);
													
													if(res==1)						
														Data_Need_Storage=1;
																																								
													SEGGER_RTT_printf(0,"------------------------Flow_Estimat OPT! Res:%d\r\n",res);
//													if(oildata_analysis[PORTn->COM_Num][GunCache].oil[THIS]!=oil[THIS])
//													{
//														if(GunCache==PORTn->Gun_A.gun_num)//��ѡǹ�ȼ��ڼ���һ���ͣ�����TSD_Same_Cnt����
//															PORTn->Gun_A.TSD_Same_Cnt=0;

//														if(GunCache==PORTn->Gun_B.gun_num)
//															PORTn->Gun_B.TSD_Same_Cnt=0;
//													}
//													else
//													{
//														if(GunCache==PORTn->Gun_A.gun_num)//
//															PORTn->Gun_A.TSD_Same_Cnt++;

//														if(GunCache==PORTn->Gun_B.gun_num)
//															PORTn->Gun_B.TSD_Same_Cnt++;
//													}
													
																	
													
													Update_TSDList(&oildata_analysis[PORTn->COM_Num][GunCache],oil[THIS],money[THIS],SampleTime_New);		
													
													Update_OptGun( &oildata_analysis[PORTn->COM_Num][GunCache],&oildata_analysis[PORTn->COM_Num][Gun_Num]);		
													
											}
											else
											{
												 SEGGER_RTT_printf(0,"Flow Ratio Maybe Err1!\r\n");	
												 SEGGER_RTT_printf(0,"TSData No Match!\r\n");	
												 TSDNoMatch1++;
												 Data_Need_Storage=0;
												 SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d-@@@@@@@@, TSD:",PORTn->COM_Num,Gun_Num);
												 SEGGER_RTT_Write(0,(u8*)tsd_ptr,(gas_data_head->len-4));
												 SEGGER_RTT_printf(0,"\r\n"); 
												
											}
												
											
										}//��ǹ���ٲ�����Ҫ��
																		
									
								 }//�뱾ǹLAST���ݲ�ͬ
								
							
						  }//�뱾ǹLAST���ݽ��бȽ�
												
							
					  }//TSD�����Ѿ�ȷ�������Ҳ�ͬ
																			
					}//���ϴα�ǹ���ݲ�ͬ
				
					
Data_Storage:
					
				  if(PORTn->Gun_A.OnQuery_Flag==1)
					{
						
						 SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d--CNT:%d, TSD:",PORTn->COM_Num,Gun_Num,PORTn->Gun_A.TSD_Same_Cnt);
						 SEGGER_RTT_Write(0,(u8*)tsd_ptr,(gas_data_head->len-4));
						 SEGGER_RTT_printf(0,"\r\n"); 
					}							
					else
					{
						 
						 SEGGER_RTT_printf(0,"COM:%d -- Gun Num:%d--CNT:%d, TSD:",PORTn->COM_Num,Gun_Num,PORTn->Gun_B.TSD_Same_Cnt);
						 SEGGER_RTT_Write(0,(u8*)tsd_ptr,(gas_data_head->len-4));
						 SEGGER_RTT_printf(0,"\r\n"); 
					}							

				//д��flash
				if(Data_Need_Storage==1)
				{
					Data_Need_Storage=0;
					SEGGER_RTT_printf(0,"COM:%d ** Gun Num:%d, OTD:",PORTn->COM_Num,Gun_Num);
					SEGGER_RTT_Write(0,(u8*)(&OTData[PORTn->COM_Num][Gun_Num]),sizeof(struct One_Time_Data));
									
					SEGGER_RTT_printf(0," Data Saved! \r\n");
				
				//	Gas_Data_Storage((u8*)tsd_ptr,sizeof(struct Total_Sum_Data),&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,Gun_Num,PORTn->COM_Num,TSD_Code);			
			  	Gas_Data_Storage((u8*)(&OTData[PORTn->COM_Num][Gun_Num]),sizeof(struct One_Time_Data),&flashSectionStruct[FLASH_SECTION_GAS_DATA].wptr,Gun_Num,PORTn->COM_Num,OTD_Code);			
			
				}
				else
				{
		//			SEGGER_RTT_printf(0,"No OTD Data Saved!\r\n");
			
				}
				
																						
					
		}//TSD����
			
											
			
	}//У��ͨ��
		
	return 0;					
}

void TaxInfo_Check(Com_Status *PORTn)
{
	    u8 i,Port_Num,Gun_cnt;	
   
      Port_Num = PORTn->COM_Num;
	
			for(i=1;i<=4;i++)
			{			
				uf_ReadBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[i],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+i*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
		
			}
			
			for(i=0;i<10;i++)
			{
				if(Com_Gun_BindInfo[Port_Num].TaxCode[i]!=0xFF && Com_Gun_BindInfo[Port_Num].TaxCode[i]!=0)
					break;				
			}
			
			if(i==10)//δ��ȡTaxCode,���ѯǹ��
			{	
				 CB_TaxData_Query(&CB_PORT[Port_Num],0);	
				 bsp_DelayMS(1000);
			   CB_PortEvent_Pro(&CB_PORT[Port_Num]);	
				
			}
			else//�ѻ�ȡTaxCode
			{						
				
				memcpy(tax_data[Port_Num].code_num,Com_Gun_BindInfo[Port_Num].TaxCode,10);	
				if(tax_data[Port_Num].code_num[0]>0x39) //��̩��˰��
				{
						 OTD_Resp_Len= 30; //��̩��
						 KTK=1;
					 
						 if(tax_data[Port_Num].code_num[0]=='A')
						 {
							 PORTn->Gun_B.gun_num=PORTn->Gun_A.gun_num;
							 PORTn->Gun_B.Gun_State=GUN_STATE_IDLE;
							 
							 if(Com_Gun_BindInfo[Port_Num].GunCount!=1)
							 {
								 Com_Gun_BindInfo[Port_Num].GunCount=1;
							   uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[Port_Num],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+Port_Num*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
					     }
						 }
				}
				else
				{
						 OTD_Resp_Len= 28;
						 KTK=0;
					   
						 if(tax_data[Port_Num].code_num[0]=='0')
						 {
							 PORTn->Gun_B.gun_num=PORTn->Gun_A.gun_num;
							 PORTn->Gun_B.Gun_State=GUN_STATE_IDLE;
							 if(Com_Gun_BindInfo[Port_Num].GunCount!=1)
							 {
								 Com_Gun_BindInfo[Port_Num].GunCount=1;
							   uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[Port_Num],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+Port_Num*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
					     }
							 
						 }
				}
				
				if(Com_Gun_BindInfo[Port_Num].Gun_NO[0]==0xFF ||Com_Gun_BindInfo[Port_Num].Gun_NO[1]==0xFF)
				{
						
					 Gun_cnt=Gun_Num_Search(3,1000);
					 SEGGER_RTT_printf(0,"\r\nSearch Gun Numer=%d\r\n",Gun_cnt);
							
					 for(i=1;i<5;i++)
					 {
						 if(CB_PORT[i].Phy_Connect_State)
							 SEGGER_RTT_printf(0,"COM%d--GunA:%d, GunB:%d\r\n",i,CB_PORT[i].Gun_A.gun_num,CB_PORT[i].Gun_B.gun_num);
					 }
					 					 																												
						uf_WriteBuffer(FLASH_SST25VF016B,(u8*)&Com_Gun_BindInfo[Port_Num],(flashSectionStruct[FLASH_SECTION_TAXCODE_DATA].base+Port_Num*sizeof(struct _Com_Gun_Binding)),sizeof(struct _Com_Gun_Binding));
					
				}
				else
				{
											
						 if(Com_Gun_BindInfo[Port_Num].GunCount==1)
						 {
							 CB_PORT[Port_Num].Gun_A.gun_num = Com_Gun_BindInfo[Port_Num].Gun_NO[0];
							 CB_PORT[Port_Num].Gun_A.Gun_State = GUN_STATE_GUN_IDLE;
						 }
						 else if(Com_Gun_BindInfo[Port_Num].GunCount==2)
						 {
							 CB_PORT[Port_Num].Gun_A.gun_num = Com_Gun_BindInfo[Port_Num].Gun_NO[0];
							 CB_PORT[Port_Num].Gun_B.gun_num = Com_Gun_BindInfo[Port_Num].Gun_NO[1];
							 CB_PORT[Port_Num].Gun_A.Gun_State = GUN_STATE_GUN_IDLE;
							 CB_PORT[Port_Num].Gun_B.Gun_State = GUN_STATE_GUN_IDLE;
							 
						 }
												
				}
	
		}
}



