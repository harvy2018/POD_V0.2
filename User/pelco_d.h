

#ifndef _PELCO_D_H_
#define _PELCO_D_H_

#include "stm32f10x.h"	



#define PITCH_UP      0x0800
#define PITCH_DOWN    0x1000
#define YAW_UP        0x0400
#define YAW_DOWN      0x0200
#define SERVO_RETURN  0x0700
#define SERVO_FREEZE  0x0000

#define SERVO_FOLLOW_MODE  0x0001
#define SERVO_LOCK_MODE  0x8000

#define MOTOR_ON      0x0011
#define MOTOR_OFF     0x8100

#define PICTURE       0x0004
#define RECORD_START  0x0B00
#define RECORD_STOP   0x0900

#define ZOOM_IN       0x2000
#define ZOOM_OUT_MAX  0x3000
#define ZOOM_OUT      0x4000
#define ZOOM_STOP     0x6000
#define ZOOM_TO_POS   0x5000 //新增命令,直接变焦到指定倍率
#define ZOOM_POS_QRY  0x7000 //新增命令，查询当前变焦倍率

#define CALIB_CMD     0x2222
#define UPLOAD_ANGLE  0x3333
#define MICRO_MODIFY  0x6666
#define MICRO_MODIFY_QUIT  0x7777
#define UPLOAD_OPTICAL_ZOOM  0x8888

#pragma pack(1)

typedef struct _PELCO
{
	u8 preamble;
	u8 addr;
	u16 cmd;
	u16 data;
	u8 cc;
	
}PELCO;


#endif