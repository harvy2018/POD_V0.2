
#ifndef _OILDATAPRO_H_
#define _OILDATAPRO_H_

#include "stm32f10x.h"	
#include "SEGGER_RTT.h"
#include "gas.h"


//参数ratio是加油速率，取值10：表示1L/S,  15：表示1.5L/S
u8 Flow_Estimate(struct OILDATA_ANALYSIS* ptr,s32 oil_new,u32 sample_t,u8 ratio);
u8 Calc_Price(struct OILDATA_ANALYSIS* tsdptr);
void Calc_OTData(struct OILDATA_ANALYSIS* tsdptr,struct One_Time_Data* OTDataPtr);

void Update_TSDList(struct OILDATA_ANALYSIS* oalistptr,s32 oilthis,s32 moneythis,u32 sampletime);

void Correct_TSDList(struct OILDATA_ANALYSIS* oalistptrOrigin,struct OILDATA_ANALYSIS* oalistptrOpt,
	                   u32 sampletime);

//void Update_TSDList(struct Total_Sum_Data *tsdlist,struct Total_Sum_Data *tsd_ptr,
//	                  struct OILDATA_ANALYSIS* oalistptr,s32 oilthis,s32 moneythis,u32 sampletime);

//void Correct_TSDList(struct Total_Sum_Data *tsd_ptrOrigin,struct Total_Sum_Data *tsd_ptrOpt,
//	                   struct OILDATA_ANALYSIS* oalistptrOrigin,struct OILDATA_ANALYSIS* oalistptrOpt,
//	                   u32 sampletime);
										 
void Update_OptGun(struct OILDATA_ANALYSIS* oalistptrOpt,struct OILDATA_ANALYSIS* oalistptrOrigin);	
										 
u8 OTData_Handle(struct OILDATA_ANALYSIS* oalistptr,struct One_Time_Data* otdptr);
										
#endif

