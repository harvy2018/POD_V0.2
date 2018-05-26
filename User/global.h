#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "stm32f10x.h"	

#define ClearDog   IWDG_ReloadCounter();

#define LED_Blink   GPIO_WriteBit(GPIOB, GPIO_Pin_0, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_0)));

#define BUZZ_ON    GPIO_SetBits(GPIOA, GPIO_Pin_8);	
#define BUZZ_OFF   GPIO_ResetBits(GPIOA, GPIO_Pin_8);	

extern u32	SYSTEMTICK;
extern vu32 unixtime;
extern u32 Data_Storage_Num;//存储的数据条数
extern u32 Data_Upload_Num;//已上传的数据条数

extern u8 data_storage;//

extern u32 data_flashaddr_index;
extern u32 Err_Data;
extern u32 FlashReadPtrNum;


extern u32 Data_Storage_Num;//存储的数据条数
extern u32 Data_Upload_Num;//已上传的数据条数

extern u32 FlashReadPtrNum;
extern u32 data_flashaddr_index;//PLC上传计数
extern u32 FlashReadPtrBuf[1000];//外部flash中的待传数据，读取地址存储数组

extern struct _Send_Cnt_F Send_Cnt_F;
extern u32 Send_F1,Send_F2,Send_F3,Send_FX;		 
extern u32 PLC_Send_Cnt;//PLC上传次数；
extern u32 PLC_Send_Fail_Cnt;//PLC上传失败次数；
extern u16 PLC_Anysis_Fail_Cnt;
extern u8  FWrev_upload_cnt;
extern u8  T_upload_cnt;
extern u8  PLC_SW;

extern u8  PLC_Data_Pre[];
extern u16 PLC_Data_Pre_Dlen;

extern u32 HW_ID[4];
extern u8 FW_VER[4];
extern u32 key;
extern u8 CB_Query_Interval;
extern u32 PLC_Send_T3;
extern s32 SNR_INT,SNR_DOT;


#endif


