
#ifndef _SBUS_H_
#define _SBUS_H_

#include "stm32f10x.h"	
#include "SBUS.h"	


#pragma pack(1)

#define PITCH_DOWN_LIMIT   -10000
#define PITCH_UP_LIMIT   10000
#define YAW_DOWN_LIMIT     -10000
#define YAW_UP_LIMIT   10000


#define Yaw_Channel 4
#define Pitch_Channel 2
#define Channel_MIN 352
#define Channel_MAX 1696
#define Channel_MID 1024

#define Channel_DOWN2 688
#define Channel_UP2 1360

#define Channel_DOWN 856
#define Channel_UP 1192

#define ZOOM_IN_R 360
#define ZOOM_OUT_R 1300
#define ZOOM_STOP_R 1000
#define ZOOM_OUT_MAX 1900

#define interval 50
typedef enum _SBUS_SIGNAL_STATE
{
	SBUS_SIGNAL_OK=0,
	SBUS_SIGNAL_LOST,
	SBUS_SIGNAL_FAILSAFE,
	SBUS_SIGNAL_MAX
	
}SBUS_SIGNAL_STATE;

typedef enum _SERVO_STATE
{
	SERVO_NORMAL=0,
	SERVO_RETURN_START,
	SERVO_RETURN_COMPLETE,
	SERVO_STATE_MAX
	
}SERVO_STATE;

typedef struct _SBUS_VALUE
{
	
	u16 value;
	u16 move_flag;
	int cnt;
}SBUS_VALUE;

SBUS_SIGNAL_STATE getFailSafeStatus(void);
void DigiServo(u8 ch, u8 position);
void Servo(u8 ch, u16 position);
u8  getDigiChannel(u8 ch);
u16 getChannel(u8 ch);
void SBUS_UpdateChannels(void);
void SBUS_DataProcess(void);
void SBUS_DataProcess000(void);
void update_servos(void);
void PrintServo(void);
void ServoReload(void);
void ServoReturn(void);
void ServoFreeze(void);
void ServoInit(void);
void FKProcess(void);
void ServoActTask(void);
#endif

