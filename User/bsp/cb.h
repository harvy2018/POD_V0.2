

#ifndef _CB_H_
#define _CB_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"	

#include <stdio.h>

#include <stdlib.h>

#include <string.h>
#include "bsp_timer.h"
#include "bsp_SST25VF016B.h"
#include "bsp_usart.h"	
#include "SEGGER_RTT.h"
#include "download.h"




#define PANID 12355
//#define CB_Query_Interval 4//秒
#define PLC_Upload_Interval 1500//毫秒

#define PORT_SUM 4
#define OnePortGunSum   4

/***系统异常编号*/

#define NO_FAULT 0

#define COM1_PHY_DISCONNECT 1001
#define COM2_PHY_DISCONNECT 1002
#define COM3_PHY_DISCONNECT 1003
#define COM4_PHY_DISCONNECT 1004
#define COMA_PHY_DISCONNECT 1094

#define COM1_LOGIC_DISCONNECT 1101
#define COM2_LOGIC_DISCONNECT 1102
#define COM3_LOGIC_DISCONNECT 1103
#define COM4_LOGIC_DISCONNECT 1104
#define COMA_LOGIC_DISCONNECT 1194

#define PLC_CONNECT_FAULT 1200
#define PLC_CONNECT_FAKE  1201

#define Manual_Reset 1024

#define GasDataQuery_Exceed_MAX 2000 //2000基础编号，需加上枪号确定是哪条枪


#define RESET_SOFT_MODE 3000
#define RESET_WTD_MODE 3001
#define RESET_PIN_MODE 3002

#define PLC_RecBuf_Full 5000

/*系统异常编号***/


#define HeartBeat  0x0FFF

#define FW_REQ_COMMAND  0x0002
#define UPDATA_COMPLETE  0x0004


#define Fault_Code 0x0060
#define TEMP_Data  0x0061
#define FWrev_Data 0x0062
#define Time_Data  0x0063
#define Log_Data   0x0064

#define CNT_Data   0x0999


#define PLC_Send_Int  0x7998
#define PLC_TX_Modulation_Set  0x7067
#define PLC_TX_Modulation_Get  0x7068



#define FW_Updata_Notice 0x7064
#define FW_Updata_Data 0x7065
#define PLC_Send_Query 0x7066

#define TAX_RESP  0x8083
#define OTD_RESP  0x8086
#define DSD_RESP  0x8087
#define MSD_RESP  0x8088
#define TSD_RESP  0x8089

#define FAULT_RESP    0x8060
#define TEMP_RESP     0x8061
#define FWVER_RESP    0x8062
#define TIME_SYNC     0x8063


#define Pack_Head_Tail_LEN  (sizeof(struct Protocol_Header)-1+4)//减去1字节res返回值，加4字节CRC值 
#define Pack_Head_LEN  (sizeof(struct Protocol_Header)-1) //减去1字节res返回值


#define USART1_DR  0x40013804
#define USART2_DR  0x40004404
#define USART3_DR  0x40004804
#define UART4_DR   0x40004C04

#define USART1_TX_DMA_Ch  DMA1_Channel4 
#define USART1_RX_DMA_Ch  DMA1_Channel5 

#define USART2_TX_DMA_Ch  DMA1_Channel7 
#define USART2_RX_DMA_Ch  DMA1_Channel6

#define USART3_TX_DMA_Ch  DMA1_Channel2 
#define USART3_RX_DMA_Ch  DMA1_Channel3 

#define UART4_TX_DMA_Ch   DMA2_Channel5 
#define UART4_RX_DMA_Ch   DMA2_Channel3 


#define MAX_FAIL_CNT 100


#define SpiFlash_DetCapacity 4096
#define SpiFlash_DataDET_ADDR  0x1FF000  //发送统计信息存储地址
#define SpiFlash_Interval_ADDR 0x1FE000  //发送间隔值存储地址
#define SpiFlash_TaxCode_ADDR 0x1FD000  //发送间隔值存储地址



#define	  TAX_RCV_BUFFER_SIZE     256
#define	  TAX_SND_BUFFER_SIZE     128
#define   TAX_PORT_NUM		        4+1
#define	  PLC_SND_BUFFER_SIZE     2048
#define	  PLC_RCV_BUFFER_SIZE     1024

#pragma pack(1)

typedef struct _TaxRBuffer		// 税控数据接收缓冲区管理结构体
{
     
	   u16   rcv_count;//
     u16   wptr;		// 写指针，中断ISR每收到一个字节加一
     u16   rptr;		// 读指针， 应用任务更改
     u8    data[TAX_RCV_BUFFER_SIZE];    // 数据缓冲区
	   u8    overFlow;
	    
	
}TaxRcvBuffer; 



typedef struct _TaxSBuffer		// 税控数据发送缓冲区管理结构体
{
     u16   wptr;		// 写指针    
     u16   rptr;		// 读指针，发送函数每发送一个字节，减一
     u8    data[TAX_SND_BUFFER_SIZE];      // 发送缓冲区
	
}TaxSndBuffer; 	


typedef enum _TAX_PORT_NUM
{
	TAX_PORT1=1,
	TAX_PORT2,
	TAX_PORT3,
	TAX_PORT4,
	TAX_MAX
	
}TaxPortNum;



typedef struct _PLCSndBuffer
{
     u16   wptr;		// 写指针
     u16   rptr;		// 读指针
     u8    data[PLC_SND_BUFFER_SIZE];
}PLCSndBuffer; 

typedef struct _PLCRcvBuffer
{
     u16   wptr;		// 写指针
     u16   rptr;		// 读指针
     u8    data[PLC_RCV_BUFFER_SIZE];
}PLCRcvBuffer; 



/*******************************************************/

struct _CB_Time
{
	char year[5];
	char month[3];
	char day[3];
	char hour[3];
	char minute[3];
	
};


typedef enum _ResErr
{
	 P_OK,
	 CRC_ERR,
	 MEM_ERR,
	 S_Num_ERR,
	 DataLen_ERR
	 
}RES_ERR;

struct _Send_Cnt_F
{
	u32 PLC_Send_Sum;//发送总次数
	u32 PLC_Send_Fail_Sum;//发送总失败次数
	u32 PLC_Send_F1; //1次发送成功的次数
	u32 PLC_Send_F2; //2次发送成功的次数
	u32 PLC_Send_F3; //大于2次发送的成功次数
	u32 PLC_Send_OK_Sum; //总计发送成功次数
	
};
/*
typedef enum _SystemFault
{
	 NO_Fault,
	 PLC_RecBuf_Full,
	 GasDataQuery_NOReady,
	 GasDataQuery_Exceed_MAX,
	 COM_Phy_DISCONNECT,
	 COM_Logic_DISCONNECT,
	 PLC_Logic_DISCONNECT
	
}SystemFault;
*/
//typedef enum _ResErr RES_ERR;


struct Protocol_Header
{
	u8  Preamble[4];
	u16 Com_len;  //总长度
	u16 Command_ID;  //命令号
	u32 Serial_Num; //流水号
	u16 Protocol_Rev;     //协议版本号
	u8  Frame_Num[2];   //帧号
	u32 Board_ID[4];  //  //
  u8  Res;   //返回结果
} ;
  
typedef struct _FW_UPDATA_INFO
{
	u8 FW_Rev[4]; //固件版本号;
  u32 Size_Ping;//固件大小
	u32 Size_Pong;//固件大小
	u32 CRC_Ping;
	u32 CRC_Pong;
	u32 Mode; //1:进入升级  0：退出升级
	
}FW_UPDATA_INFO;

typedef struct _FW_INFO
{
	
	u8 MagicCode[4];//"firm"
	u8 MajorVersion[2];
	u8 MinorVersion[2];
	u32 Size;
	u32 Crc;

}FW_INFO;

typedef struct _Gun_Status
{
	u8 gun_num; //枪号
	u8 Init_Status;//初始化标志,0:未初始化，1：已初始化
	u8 DSumDataQuery_Flag;//日累计数据查询标志
	u8 MSumDataQuery_Flag;//月累计数据查询标志
	u8 TSumDataQuery_Flag;//总累计数据查询标志
	u8 OilDataInitOk;//第一笔总累计数据初始化完成
	u8 Price_Query_Flag;//单价查询标志
	u8 Price_Same_Cnt;//单价相同计次
	u8 Price[4];
	
	u8  Gun_State;
	u8  OnQuery_Flag;//本次是否正在查询标志
	u8  First_Gun;//当天第一枪标志
	u8  MSD_Same_Cnt;//月累计相同计数
	u32 TSD_Same_Cnt;//总累计相同计数
	u32 NoData_Time;
	u32 Fail_Cnt;//查询失败次数
	u16 fault_num;
}Gun_Status;



typedef struct _Com_Status
{
	Gun_Status Gun_A;
	Gun_Status Gun_B;
	u8 COM_Num;  //串口号
	u8 Phy_Connect_State; //无连接0；
	u8 Logic_Connect_State;
	u8 GasDataRes_Flag;//抄报口数据查询结果标志
  u8 TaxDataQuery_Flag;//税务信息查询标志
  u16 RecDlen;//已接受数据长度
}Com_Status;


typedef struct _Com_Gun_Binding
{
	u8 COM_Num;  //串口号
	u8 Gun_NO[OnePortGunSum];
	u8 TaxCode[10];
	u8 GunCount;//检索到的枪数量
}Com_Gun_Binding;

//typedef struct _PLC_Status
//{
//	u8  Connect_status; //无连接0；
//	u32 Connect_Cnt;
//	u32 Last_Connect_time[10];//上次重连时间
//	u32 NoData_Time;
//	
//}PLC_Status;


typedef struct _Reset_Status
{
	u32 counter; //复位次数
	u32 last_reset_time[10];//上次复位时间
	
}Reset_Status;


typedef struct _SystemDignose_Info
{
	
	u16 Fault_Num;//系统异常编号
	u32 TimeStamp;
	Com_Status com1;
	Com_Status com2;
	Com_Status com3;
	Com_Status com4;
	Reset_Status com5;
	
	Reset_Status watchdog;
	Reset_Status pin_reset;
	Reset_Status soft_reset;
	
//	FW_INFO *fmware;
	
}SystemDignose_Info;


//typedef struct _Fault_Info
//{
//	
//	u16 Fault_Num;//系统异常编号
//	u32 TimeStamp;
//	
//}Fault_Info;

typedef enum 
{
	 Reserved,
	 PLC_Timer,
	 PLC_Communication_Timer,
	 HeartBeat_Timer,
	 SpiFlash_Erase_Timer,
	 PLC_SW_Timer,
	 Upgrade_OutTimer,
	 Other_Timer
	  
}TimerType;


extern volatile u32 unixtime;
//extern volatile u8 Rx2Buffer[RXBUF_LEN];//  __at(0x68100000);

void Protocol_Resp(struct Protocol_Header *protocol_head, u8 F_Sum,u8 F_Num, RES_ERR res);
u8 Protocol_Send(struct Protocol_Header *protocol_head, u16 command_id, u8 F_Sum,u8 F_Num, u8* data, u16 length);
u8 Protocol_Free(u8 *P_Data);

void DMA_Configuration(DMA_Channel_TypeDef* Channel,u32 BufferSize,u32 PeripheralBaseAddr,u32 MemoryBaseAddr,u32 Dir,u32 Priority);

void CB_Init(void);
void CB_PortEvent_Pro(Com_Status *PORTx);
void CB_TaxData_Query(Com_Status *PORTx,u8 gun_num);
void PLC_Send_Enable(void);
void PLC_ReConnect(u8 cnt);
void COM_Connect_Detect(void);
void SYSTEM_RESET(u16 FaultNum);

void PLC_Data_Send(u8* data,u16 dlen,u16 command_id, u8 F_Sum,u8 F_Num);
void CB_HeartBeat(void);
void PLC_BufIndex_Add(u32 pos);
void CB_OTD_Query(Com_Status *PORTx);
u8 CB_DSD_Query_Now(Com_Status *PORTx,u8 Gun_Num);


void COM_Setup(Com_Status *PORTx);
u8 Gun_Num_Search(u8 Gun_Num,u16 S_Delay);
u8 Price_Calibration(u16 S_Delay);
void OilCmdSend(Com_Status *PORTx,u8 Gun_No);


extern u8 Gun_Query_Repeat;
extern u32 S_ERR,CRCERR,RX1err,RX2err;
extern Com_Status CB_PORT[];
extern u32 TX_Modulation;
extern struct _FW_INFO* fw_info; //固件信息
extern TaxRcvBuffer   TaxRBuffer[TAX_PORT_NUM];
extern TaxSndBuffer   taxSBuffer[TAX_PORT_NUM];

extern u32 TxPtr[];
extern u32 TxDlen[];
extern u8  TxRecDlen[];

extern PLCSndBuffer PLC_SBuffer;
extern PLCRcvBuffer PLC_RBuffer;
extern Com_Gun_Binding Com_Gun_BindInfo[PORT_SUM+1];
void CB_OTDData_Disp(Com_Status *PORTx,u8 gun_num);
//uint32_t Cal_crc32(uint32_t *addr, int num) //STM32硬件CRC32算法
//{
//    int i;
//	  u32 crc=0xFFFFFFFF;  
//	
//    for (; num > 0; num--)              
//    {
//        crc = crc ^ (*addr++);     
//        for (i = 0; i < 32; i++)             
//        {
//            if (crc & 0x80000000)            
//                crc = (crc << 1) ^ 0x04C11DB7;
//            else                          
//                crc <<= 1;                 
//        }                             
//        crc &= 0xFFFFFFFF;            
//    }                               
//    return(crc);                   
//}
#pragma pack()

#endif
/********************** END ***************************************************/

