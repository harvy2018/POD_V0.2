
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>	
#include <time.h>	
#include "gas.h"
#include "cb.h"
#include "dev_state.h"	
#include "flashmgr.h"	
#include "global.h"	
#include "stm32f10x.h"

//参数ratio是加油速率，取值10：表示1L/S,  15：表示1.5L/S
//流速符合要求返回1，不符合返回0
u8 Flow_Estimate(struct OILDATA_ANALYSIS* ptr,s32 oil_new,u32 sample_t,u8 ratio)
{
	if(oil_new-ptr->oil[THIS]<=0)
		return false;
	else if((oil_new-ptr->oil[THIS])<=((sample_t-ptr->sample_time[THIS])*10*ratio))
		return true;
	else 
		return false;
		
}


//0:需要请求单价，1：不用请求
u8 Calc_Price(struct OILDATA_ANALYSIS* tsdptr)
{
	
	s32 highPrice,lowPrice,midPrice;	
	float oil,money;
	
	 //金额 / (油量+0.005) < 单价 <＝ 金额 / (油量-0.005) 
//	midPrice = Convert.ToInt32(Math.Round(amount / (sum + 0.0), 2) * 100);
//	highPrice = Convert.ToInt32(Math.Floor((amount * 100) / (sum - 0.5)));
//	lowPrice = Convert.ToInt32(Math.Ceiling((amount * 100) / (sum + 0.5)));
//	if (lowPrice == (amount * 100) / (sum + 0.5)) //正好能够除尽
//		lowPrice++;
//	if (highPrice < lowPrice)  //单价0.01，加油6.45，金额7分，得lowPrice > highPrice
//		lowPrice = highPrice = midPrice;
//	if (midPrice < lowPrice || midPrice > highPrice)
//		throw new Exception("单价计算错误：金额=" + amount + ", 油量=" + sum);
	
	oil=tsdptr->oil[THIS]-tsdptr->oil[LAST];
	money=tsdptr->money[THIS]-tsdptr->money[LAST];
	
	midPrice =(s32)((money/oil)*100+0.5);
	highPrice=floor((money*100)/(oil-0.5));
	lowPrice=ceil((money*100)/(oil+0.5));
	
	if(lowPrice==(money*100)/(oil+0.5))
	   lowPrice++;
	
	if (highPrice < lowPrice)  //单价0.01，加油6.45，金额7分，得lowPrice > highPrice
	{
		lowPrice = midPrice;
		highPrice = midPrice;
		SEGGER_RTT_printf(0,"------------------highPrice < lowPrice!\r\n");
	}
	
//	if (midPrice < lowPrice || midPrice > highPrice)
//	   SEGGER_RTT_printf(0,"Price Err!\r\n");
	
	
	SEGGER_RTT_printf(0,"lowPrice=%d, highPrice=%d\r\n",lowPrice,highPrice);
	
	if(lowPrice!=highPrice)
	{	 	
		return false;//需要请求单价
	}
	else
	{
	   tsdptr->price=lowPrice;//更新单价
		 return true;
	}
	
	
}


void Calc_OTData(struct OILDATA_ANALYSIS* oalistptr,struct One_Time_Data* OTDataPtr)
{
	s32 oil,money;
		
	oil=oalistptr->oil[THIS]-oalistptr->oil[LAST];
	money=oalistptr->money[THIS]-oalistptr->money[LAST];
		
	sprintf(OTDataPtr->day,"%2d",ASCTIME2->tm_mday);
	sprintf(OTDataPtr->hour,"%2d",ASCTIME2->tm_hour);
	sprintf(OTDataPtr->minute,"%2d",ASCTIME2->tm_min);
	
	sprintf(OTDataPtr->oil_quantity,"%06d",oil);
	sprintf(OTDataPtr->money,"%06d",money);
	
	if(oalistptr->price!=-1)
	  sprintf(OTDataPtr->price,"%04d",oalistptr->price);
	

}

void Update_TSDList(struct OILDATA_ANALYSIS* oalistptr,s32 oilthis,s32 moneythis,u32 sampletime)
{
		
	oalistptr->oil[LAST]=oalistptr->oil[THIS];
	oalistptr->money[LAST]=oalistptr->money[THIS];
	oalistptr->sample_time[LAST]=oalistptr->sample_time[THIS];
	
	oalistptr->oil[THIS]=oilthis;
	oalistptr->money[THIS]=moneythis;
	oalistptr->sample_time[THIS]=sampletime;
		
}


void Correct_TSDList(struct OILDATA_ANALYSIS* oalistptrOrigin,struct OILDATA_ANALYSIS* oalistptrOpt,
	                   u32 sampletime)
	                   
{
	 
	oalistptrOpt->oil[LAST]=oalistptrOpt->oil[THIS];
	oalistptrOpt->oil[THIS]=oalistptrOrigin->oil[THIS];
	
	oalistptrOpt->money[LAST]=oalistptrOpt->money[THIS];
	oalistptrOpt->money[THIS]=oalistptrOrigin->money[THIS];
	
	oalistptrOpt->sample_time[LAST]=oalistptrOpt->sample_time[THIS];
	oalistptrOpt->sample_time[THIS]=oalistptrOrigin->sample_time[THIS];
	

	oalistptrOrigin->oil[THIS]=oalistptrOrigin->oil[LAST];
	oalistptrOrigin->money[THIS]=oalistptrOrigin->money[LAST];
	oalistptrOrigin->sample_time[THIS]=sampletime;
	

}

void Update_OptGun(struct OILDATA_ANALYSIS* oalistptrOpt,struct OILDATA_ANALYSIS* oalistptrOrigin)
{
	u8 i;
	memcpy(oalistptrOpt->optgun,oalistptrOrigin->optgun,OnePortGunNum);
	memset(oalistptrOrigin->optgun,0,OnePortGunNum);
	
	oalistptrOpt->optgun_cnt=oalistptrOrigin->optgun_cnt;
	oalistptrOrigin->optgun_cnt=0;
	
	for(i=0;i<OnePortGunNum;i++)
	{
		if(oalistptrOpt->optgun[i]==oalistptrOpt->gun_num)
			oalistptrOpt->optgun[i]=oalistptrOrigin->gun_num;
	}
	
}

u8 OTData_Handle(struct OILDATA_ANALYSIS* oalistptr,struct One_Time_Data* otdptr)
{
	  u8 res;
	  if(oalistptr->oil[THIS]>oalistptr->oil[LAST] && oalistptr->oil[LAST]!=0 && oalistptr->money[THIS]>oalistptr->money[LAST])
		{
			
			res=Calc_Price(oalistptr);
			if(!res)
			{
				oalistptr->price=-1;//单价置为-1，待重新查询单价
				SEGGER_RTT_printf(0,"Request New Price!  $$$$$$\r\n");
				
				if(CB_PORT[oalistptr->port_num].Gun_A.gun_num==oalistptr->gun_num)
					 CB_PORT[oalistptr->port_num].Gun_A.Price_Query_Flag=1;								
				else if(CB_PORT[oalistptr->port_num].Gun_B.gun_num==oalistptr->gun_num)
					 CB_PORT[oalistptr->port_num].Gun_B.Price_Query_Flag=1;
				else
					 SEGGER_RTT_printf(0,"Gun num not find!\r\n");		
				
				Calc_OTData(oalistptr,otdptr);
				return 2;
				
			}
			else
			{								
				SEGGER_RTT_printf(0,"-----------------------Price is OK!\r\n");
				Calc_OTData(oalistptr,otdptr);
				return 1;
										
			}
			
			
		}
		
		return 0;
			
}


//void Update_TSDList(struct Total_Sum_Data *tsdlist,struct Total_Sum_Data *tsd_ptr,
//	                  struct OILDATA_ANALYSIS* oalistptr,s32 oilthis,s32 moneythis,u32 sampletime)
//{
//	
//	tsdlist[LAST]=tsdlist[THIS];
//	memcpy(tsdlist,tsd_ptr,sizeof(struct Total_Sum_Data));
//	
//	oalistptr->oil[LAST]=oalistptr->oil[THIS];
//	oalistptr->money[LAST]=oalistptr->money[THIS];
//	oalistptr->sample_time[LAST]=oalistptr->sample_time[THIS];
//	
//	oalistptr->oil[THIS]=oilthis;
//	oalistptr->money[THIS]=moneythis;
//	oalistptr->sample_time[THIS]=sampletime;
//		
//}


//void Correct_TSDList(struct Total_Sum_Data *tsd_ptrOrigin,struct Total_Sum_Data *tsd_ptrOpt,
//	                   struct OILDATA_ANALYSIS* oalistptrOrigin,struct OILDATA_ANALYSIS* oalistptrOpt,
//	                   u32 sampletime)
//{
//	
//	tsd_ptrOpt[LAST]=tsd_ptrOpt[THIS];
//	tsd_ptrOpt[THIS]=tsd_ptrOrigin[THIS];

//  
//	oalistptrOpt->oil[LAST]=oalistptrOpt->oil[THIS];
//	oalistptrOpt->oil[THIS]=oalistptrOrigin->oil[THIS];
//	
//	oalistptrOpt->money[LAST]=oalistptrOpt->money[THIS];
//	oalistptrOpt->money[THIS]=oalistptrOrigin->money[THIS];
//	
//	oalistptrOpt->sample_time[LAST]=oalistptrOpt->sample_time[THIS];
//	oalistptrOpt->sample_time[THIS]=oalistptrOrigin->sample_time[THIS];
//	
//	

//	tsd_ptrOrigin[THIS]=tsd_ptrOrigin[LAST];
//	oalistptrOrigin->oil[THIS]=oalistptrOrigin->oil[LAST];
//	oalistptrOrigin->money[THIS]=oalistptrOrigin->money[LAST];
//	oalistptrOrigin->sample_time[THIS]=sampletime;
//	

//}

