
#ifndef _PLC_STATE_H_
#define _PLC_STATE_H_


#include "stm32f10x.h"	
#pragma pack(1)

typedef enum _PLC_STATE
{
	PLC_STATE_IDLE,
	PLC_STATE_INIT,
	PLC_STATE_CONNECTING,
	PLC_STATE_CONNECT,
	PLC_STATE_MAX
	
}PLC_STATE;

typedef enum _RTC_STATE
{
	RTC_STATE_IDLE,
	RTC_STATE_INIT,
	RTC_STATE_SYNCING,
	RTC_STATE_SYNCED,
	RTC_STATE_MAX
	
}RTC_STATE;


typedef enum _GUN_STATE
{
	GUN_STATE_IDLE,
	GUN_STATE_INIT,
	GUN_STATE_PHY_CONNECT,
	GUN_STATE_LOG_CONNECTING,
	GUN_STATE_GUN_IDLE,
	GUN_STATE_GUN_BUSY,
	GUN_STATE_MAX
	
}GUN_STATE;


typedef enum _DEV_STATE
{
	DEV_STATE_IDLE,
	DEV_STATE_SAMPLE,
	DEV_STATE_UPGRADE,
	DEV_STATE_NOSAMPLE,
	DEV_STATE_MAX
	
}DEV_STATE;


DEV_STATE getDevState(void);
void setDevState(DEV_STATE state);

GUN_STATE getGunState(void);
void setGunState(GUN_STATE state);


RTC_STATE getRTCState(void);
void setRTCState(RTC_STATE state);


PLC_STATE getPlcState(void);
void setPlcState(PLC_STATE state);
#pragma pack()
#endif

