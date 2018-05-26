
#ifndef _GAS_H_
#define _GAS_H_

#include "stm32f10x.h"	
#include "SEGGER_RTT.h"
#include "cb.h"

#define Gun_Sum    20

#define OnePortGunNum   2
#define FlowRatio  15 //10: 1L/S,,,15：1.5L/S

#define TAX_PORT_Sum   5

#define TAXD_Code  0x83
#define OTD_Code   0x86
#define DSD_Code   0x87
#define MSD_Code   0x88
#define TSD_Code   0x89


/*
#define TAXD_Resp_Len  60 //正常税务信息，分两包接受，没包加上协议头尾需增加6字节，共增加12字节长度
*/
//#define TAXD_Resp_Len  16 //未初始化的税务信息，只有10字节税控装置序列号信息
//#define OTD_Resp_Len   28
//#define DSD_Resp_Len   34
//#define MSD_Resp_Len   32
//#define TSD_Resp_Len   30

#pragma pack(1)    //设置字节对齐
struct Gas_Data_Head
{
	unsigned char preamble; //前导码 0xBB
	unsigned char len;//长度码=帧号+命令码+参数+校验码
	unsigned char rfn;//帧号
	unsigned char command;//命令码
	unsigned char status;//返回状态
};	

struct Gas_Data_Head2
{
	unsigned char preamble; //前导码 0xBB
	unsigned char len;//长度码=帧号+命令码+参数+校验码
	unsigned char rfn;//帧号
	unsigned char command;//命令码
//	unsigned char status;//返回状态
};	

struct Tax_Data
{
	unsigned char code_num[10];//出厂编号
	unsigned char gun_code[2];
	unsigned char tax_code[20];
	unsigned char oil_num[4];
	unsigned char year[4];
	unsigned char month[2];
	unsigned char day[2];
	unsigned char hour[2];
	unsigned char minute[2];

};

struct Tax_Data_X
{
	struct Gas_Data_Head FirstHead;
	unsigned char code_num[10];//出厂编号
	unsigned char gun_code[2];
	unsigned char tax_code[19];
	unsigned char check_code1;
	struct Gas_Data_Head2 SecHead;
	unsigned char tax_code_tail;
	unsigned char oil_num[4];
	unsigned char year[4];
	unsigned char month[2];
	unsigned char day[2];
	unsigned char hour[2];
	unsigned char minute[2];
	unsigned char check_code2;  //英泰赛福预留空位，科泰康的第二帧校验位
	unsigned char check_code3;//英泰赛福预留空位
	unsigned char check_code4;//英泰赛福的第二帧校验位
	

};

struct One_Time_Data //单次加油数据
{
	 char day[2];
	 char hour[2];
	 char minute[2];

	 char oil_quantity[6];//含两位小数
	 char money[6];//含两位小数
	 char price[4];//含两位小数

};

struct Day_Sum_Data //日累计加油数据
{

	unsigned char year[4];
	unsigned char month[2];
	unsigned char day[2];
	unsigned char oil_quantity[8];
	unsigned char money[8];
	unsigned char price[4];

};

struct Month_Sum_Data //月累计加油数据
{
	unsigned char year[4];
	unsigned char month[2];

	unsigned char oil_quantity[10];
	unsigned char money[10];

};

struct Total_Sum_Data //总累计加油数据
{
	
	unsigned char oil_quantity[12];
	unsigned char money[12];

};

struct TaxD_Command
{
	unsigned char preamble; //前导码 0xBB
	unsigned char len;//长度码=帧号+命令码+参数+校验码
	unsigned char rfn;//帧号
	unsigned char command;//命令码
	unsigned char gun_num;//枪号
	unsigned char ccd;//校验码=帧号^命令码^参数  

};

struct OTD_Command //当次加油数据查询命令
{
	unsigned char preamble; //前导码
	unsigned char len;//长度码=帧号+命令码+参数+校验码
	unsigned char rfn;//帧号
	unsigned char command;//命令码
	unsigned char gun_num;//枪号
	unsigned char ret_mode;//返回模式
	unsigned char ccd;//校验码=帧号^命令码^参数   

};

struct DSD_Command //日累计加油数据查询命令
{
	unsigned char preamble; //前导码
	unsigned char len;//长度码=帧号+命令码+参数+校验码
	unsigned char rfn;//帧号
	unsigned char command;//命令码
	unsigned char gun_num;//枪号
	unsigned char year[4];
	unsigned char month[2];
	unsigned char day[2];
	unsigned char ret_mode;//返回模式
	unsigned char ccd;//校验码=帧号^命令码^参数 

};

struct MSD_Command //月累计加油数据查询命令
{
	unsigned char preamble; //前导码
	unsigned char len;//长度码=帧号+命令码+参数+校验码
	unsigned char rfn;//帧号
	unsigned char command;//命令码
	unsigned char gun_num;//枪号
	unsigned char year[4];
	unsigned char month[2];
	unsigned char ret_mode;//返回模式
	unsigned char ccd;//校验码=帧号^命令码^参数 

};

struct TSD_Command //总累计加油数据查询命令
{
	unsigned char preamble; //前导码 
	unsigned char len;//长度码=帧号+命令码+参数+校验码
	unsigned char rfn;//帧号
	unsigned char command;//命令码
	unsigned char gun_num;//枪号
  unsigned char tsd_mode;//查询模式
	unsigned char ret_mode;//返回模式
	unsigned char ccd;//校验码=帧号^命令码^参数 

};



struct Storage_info//11字节
{
	
	u16 X_num;   //枪号，串口号（高字节是枪号，低字节是串口号）或异常编号
	u32 time_cb;//抄报时间
	u8 data_type;//加油数据类别
	u8 data_status[4];//NEW OR OLD
	u8 data_len;//本条数据长度
};

struct s
{
	u32 oil_quantity;
	u32 t;

	
};

//typedef enum _OILACTION
//{
//	LAST,
//	THIS,
//	MARGIN,
//	MAX_ACT	
//	
//}OILACTION;

typedef enum _OILCOUNT
{	
	THIS,
	LAST,
	MAX_COUNT	
	
}OILCOUNT;


struct OILDATA_ANALYSIS //总累计加油数据查询统计
{
	u8 port_num;
	u8 gun_num;
	u8 tsd_same_cnt;
	u8 optgun[OnePortGunNum];//可能的枪号归属
	u8 optgun_cnt;//备选枪计数
	s16 price;
	s32 oil[MAX_COUNT];
	s32 money[MAX_COUNT];
  u32 sample_time[MAX_COUNT];
	
		
};




#pragma pack()
//struct FaultStorage_info //异常信息存储的数据信息
//{
//	u16 fault_num;
//	u8 data_type;//加油数据类别
//	u8 data_status[3];//NEW OR OLD
//	u8 data_len;//本条数据长度
//};

/******************************************************************************************/



u8 OilCmdGenerate(u8 command_code,u8 gun_num,u8 tsd_mode,u8 ret_mode);
u8 Gas_Data_Process(u8* RXBuf,Com_Status *PORTn);

u8 Gas_Data_Storage(u8* data,u16 Dlen,u32* WAddr,u8 gun_num,u8 port_num,u8 data_type);//WAddr是FLASH地址变量，不是地址
u8 Fault_Info_Storage(u32* WAddr,u16 fault_num);//WAddr是FLASH地址变量，不是地址

u8 LocalData_Upload(u8* data,u32 RAddr);
u8 LocalData_Old(u32 RAddr); //RAddr是FLASH地址
u16 FlashAddr_Search(u32 SearchAddr,u32 SearchLen,u32* RAddrBuf,u32* SearchNum,u8 Search_Final);
void Command_Gen_Search(struct TSD_Command * tsd_order, u8 gun_num);
void Command_Gen_OTD(struct OTD_Command * otd_order, u8 gun_num,u8 ret_mode);
void TaxInfo_Check(Com_Status *PORTn);

extern u32 t_stamp;
extern u32 spiFlash_DetectAddr; //1MB起始地址
extern u32 SpiFlash_DataStorageAddr;
extern struct _CB_Time CB_Time;
extern struct TaxD_Command       taxd_command;
extern struct OTD_Command        otd_command;
extern struct DSD_Command        dsd_command;
extern struct MSD_Command        msd_command;
extern struct TSD_Command        tsd_command;
extern struct Tax_Data tax_data[];
extern u32 ERROR1,ERROR2,ERROR3,GX,D_Same;

extern u8 TAXD_Resp_Len ; //未初始化的税务信息，只有10字节税控装置序列号信息
extern u8 OTD_Resp_Len ;
extern u8 DSD_Resp_Len ;
extern u8 MSD_Resp_Len ;
extern u8 TSD_Resp_Len ;
extern u16 CB_Year,CB_Month,CB_Day;
extern struct tm *ASCTIME2;
extern u32 TSDNoMatch0,TSDNoMatch1,Correct;

extern struct Total_Sum_Data   total_sum_data[TAX_PORT_Sum][OnePortGunNum][2];

#endif


