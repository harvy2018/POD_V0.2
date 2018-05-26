

#include "dev_state.h"	

static PLC_STATE  plcState = PLC_STATE_IDLE;
static RTC_STATE  rtcState = RTC_STATE_IDLE;
static GUN_STATE  gunState = GUN_STATE_IDLE;
static DEV_STATE  devState = DEV_STATE_IDLE;

PLC_STATE getPlcState()
{
	return plcState;
}

void setPlcState(PLC_STATE state)
{
      plcState = state;
}



RTC_STATE getRTCState()
{
	return rtcState;
}

void setRTCState(RTC_STATE state)
{
      rtcState = state;
}



GUN_STATE getGunState()
{
	return gunState;
}

void setGunState(GUN_STATE state)
{
      gunState = state;
}


DEV_STATE getDevState()
{
	return devState;
}

void setDevState(DEV_STATE state)
{
	  devState=state;
}

