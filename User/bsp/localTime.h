/***************************************************************************************
File name: localTime.h
Date: 2016-9-27
Author: Wangwei/GuokuYan
Description:
     This file contains all the variables and API definition for local time maintainence.
***************************************************************************************/
#include "stm32f10x.h"	

#ifndef _LOCAL_TIME_H
#define	_LOCAL_TIME_H
#define LOCAL_TIME_ZONE_SECOND_OFFSET     28800 //8小时的秒数
#define REFERENCE_YEAR_BASE     1900

typedef enum  _ClockSyncState
{
    CLOCK_LOCAL = 0,
    CLOCK_GTC,
    CLOCK_MAX
}ClockSyncState;

typedef struct  _LocalTimeStamp
{
    char  year[5];
    char  month[3];
    char  date[3];
    char  hour[3];
    char  minute[3];
    char  second[3];
}LocalTimeStamp;


u32  getLocalTick();
void getLocalFormatedTime(u32 clock, LocalTimeStamp *localtime);
void ClockStateSet(ClockSyncState state);
ClockSyncState ClockStateGet();



#endif
