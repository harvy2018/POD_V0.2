/***************************************************************************************
File name: localTime.c
Date: 2016-9-27
Author: Wangwei/GuokuYan
Description:
     This file contains all the variables and API for local time maintainence.
***************************************************************************************/
#include "string.h"
#include "time.h"
#include "localTime.h"

static u32 secondTick = 0;
static ClockSyncState syncState = CLOCK_LOCAL;

/*************************************************************************************,
Function Name: updateLocalTick
Description:
   update the local global seconds tick when rtc interrupt occurs. This function intends to be called in the RTC
ISR.
Parameter:
    clock: RTC internal second register.
Return:
    NULL
***************************************************************************************/
void   updateLocalTick(u32 clock)
{
    secondTick = clock;    
}


/*************************************************************************************,
Function Name: getLocalTick
Description:
   Retrieve the global tick count for application usage.
ISR.
Parameter:
    NULL
Return:
    NULL
***************************************************************************************/
u32  getLocalTick()
{
    return secondTick;
}
/*************************************************************************************,
Function Name: getLocalFormatedTime
Description:
   Helper function to retrieve formated time structure based on the application request..
ISR.
Parameter:
    clock:[I], Second based local clock.
    localtime[I/O]: formated YYYY/MM/DD/HH/MM/SS 
Return:
    NULL
***************************************************************************************/
void getLocalFormatedTime(u32 clock, LocalTimeStamp *time)
{
    struct tm *t;
    u32 timestick;
    timestick=clock+LOCAL_TIME_ZONE_SECOND_OFFSET;
    t = localtime((time_t *)&timestick);
    sprintf(time->year,"%4d",   t->tm_year+REFERENCE_YEAR_BASE);
    sprintf(time->month,"%2d",t->tm_mon+1);
    sprintf(time->date,"%2d",   t->tm_mday);
    sprintf(time->hour,"%2d",t->tm_hour);
    sprintf(time->minute,"%2d",   t->tm_min);
    sprintf(time->second,"%2d",   t->tm_sec);
}


void ClockStateSet(ClockSyncState state)
{
    syncState = state;
}


ClockSyncState ClockStateGet()
{
    return syncState;
}