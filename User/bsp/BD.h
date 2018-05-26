#ifndef _BD_H_
#define _BD_H_

#include "stm32f10x.h"	

#pragma pack(1)

typedef struct _FKPro
{
	u8 HID;
	u8 Address;
    u8 FunctionID;
	u16 DataLen;
    u8 StartAddr;
    u8 RetFlag;	
	u8 cc;	
	
}FKPro;

void BD_MessageUpload(void);


#endif