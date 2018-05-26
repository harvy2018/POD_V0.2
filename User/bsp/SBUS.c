
#include "stm32f10x.h"	
#include "SBUS.h"
#include "bsp_usart.h"	
#include "uartPort.h"	
#include "camera.h"	
#include "pelco_d.h"	
#include "board.h"
#include "Gimbal.h"
#include "cam.h"

u8 sbusData[25];
u16 channels[18];
u16 servos[18];


u8 Servo_Yfollow_on[]={0x3E,0x1F,0x06,0x25,0x01,0x1F,0x01,0x00,0x00,0x00,0x21};
u8 Servo_Pfollow_on[]={0x3E,0x1F,0x06,0x25,0x01,0x1E,0x02,0x00,0x00,0x00,0x21};

u8 Servo_Yfollow_off[]={0x3E,0x1F,0x06,0x25,0x01,0x1F,0x00,0x00,0x00,0x00,0x20};
u8 Servo_Pfollow_off[]={0x3E,0x1F,0x06,0x25,0x01,0x1E,0x00,0x00,0x00,0x00,0x1F};

u8 motor_on[]= {0x3E,0x4D,0x00,0x4D,0x00};
u8 motor_off[]={0x3E,0x6D,0x00,0x6D,0x00};

u32 servo_state=SERVO_NORMAL;

u8 Angle_Requst=0;//地面控制端角度请求标识

SBUS_VALUE YAW={Channel_MID,0,0};
SBUS_VALUE PITCH={Channel_MID,0,0};

SBUS_SIGNAL_STATE failsafe_status;

void ServoReload(void)
{
	for(u8 i=1;i<=16;i++)
	  Servo(i,Channel_MID);
						
	DigiServo(1,1);
	DigiServo(2,1);
	
}

//void ServoReturn(void)
//{
//	if(YAW.cnt>=4)
//	{
//		YAW.value=Channel_MIN;						
//		YAW.cnt-=4;				
//		Servo(Yaw_Channel,YAW.value);
//	}
//	else if(YAW.cnt<=(-4))
//	{
//		YAW.value=Channel_MAX;
//		YAW.cnt+=4;					
//		Servo(Yaw_Channel,YAW.value);
//	}
//	else if(YAW.cnt>0 && YAW.cnt<4)
//	{
//		YAW.value=Channel_DOWN;						
//		YAW.cnt--;				
//		Servo(Yaw_Channel,YAW.value);
//	}
//	else if(YAW.cnt<0 && YAW.cnt>(-4))
//	{
//		YAW.value=Channel_UP;						
//		YAW.cnt++;				
//		Servo(Yaw_Channel,YAW.value);
//	}
//	else
//	{
//		YAW.value=Channel_MID;
//		Servo(Yaw_Channel,YAW.value);
//	}
//	
//	
//	
//	
//	if(PITCH.cnt>=4)
//	{
//		PITCH.value=Channel_MIN;						
//		PITCH.cnt-=4;				
//		Servo(Pitch_Channel,PITCH.value);
//	}
//	else if(PITCH.cnt<=(-4))
//	{
//		PITCH.value=Channel_MAX;
//		PITCH.cnt+=4;					
//		Servo(Pitch_Channel,PITCH.value);
//	}
//	else if(PITCH.cnt>0 && PITCH.cnt<4)
//	{
//		PITCH.value=Channel_DOWN;
//		PITCH.cnt--;					
//		Servo(Pitch_Channel,PITCH.value);
//	}
//	else if(PITCH.cnt<0 && PITCH.cnt>(-4))
//	{
//		PITCH.value=Channel_UP;
//		PITCH.cnt++;					
//		Servo(Pitch_Channel,PITCH.value);
//	}
//	else
//	{
//		PITCH.value=Channel_MID;					
//		Servo(Pitch_Channel,PITCH.value);
//	}
//		
//	printf("PITCH.cnt:%d,YAW.cnt:%d\r\n",PITCH.cnt,YAW.cnt);
//	if(PITCH.cnt==0 && YAW.cnt==0)
//		servo_state=2;		
//	
//}

void ServoReturn(void)
{
	if(YAW.cnt>0)
	{
		YAW.value=Channel_DOWN2;						
		YAW.cnt--;				
		Servo(Yaw_Channel,YAW.value);
	}
	else if(YAW.cnt<0)
	{
		YAW.value=Channel_UP2;
		YAW.cnt++;					
		Servo(Yaw_Channel,YAW.value);
	}
	else
	{
		YAW.value=Channel_MID;
		Servo(Yaw_Channel,YAW.value);
	}
		
	
	if(PITCH.cnt>0)
	{
		PITCH.value=Channel_DOWN2;						
		PITCH.cnt--;				
		Servo(Pitch_Channel,PITCH.value);
	}
	else if(PITCH.cnt<0)
	{
		PITCH.value=Channel_UP2;
		PITCH.cnt++;					
		Servo(Pitch_Channel,PITCH.value);
	}
	else
	{
		PITCH.value=Channel_MID;					
		Servo(Pitch_Channel,PITCH.value);
	}
		
	SEGGER_RTT_printf(0,"PITCH.cnt:%d,YAW.cnt:%d\r\n",PITCH.cnt,YAW.cnt);
	if(PITCH.cnt==0 && YAW.cnt==0)
		servo_state=SERVO_RETURN_COMPLETE;		
	
}

void ServoFreeze(void)
{
	PITCH.value=Channel_MID;
	YAW.value=Channel_MID;
	Servo(Pitch_Channel,PITCH.value);
	Servo(Yaw_Channel,YAW.value);
}

	  
void ServoInit(void)
{
	for(u8 i=1;i<=16;i++)
	Servo(i,Channel_MID);
					
	DigiServo(1,1);
	DigiServo(2,1);
}

//void ServoControl()
//{
//	YAW.value=Channel_UP;
//	YAW.cnt++;						
//	Servo(Yaw_Channel,YAW.value);
//}


void FKProcess(void)
{
	
	u16 availBytes;
	u8 comdata;
	u8 cc;
	u8 buf[12];
	PELCO PelcoData;
    
	if(GetUartRxIdleFlag(UART_FK)==1)
    {	
	    ClearUartRxIdleFlag(UART_FK);
        
        availBytes=uartGetAvailBufferedDataNum(UART_FK);
       if(availBytes>=sizeof(PELCO))
       {
start:
		cc=0;
		uartRead(UART_FK,buf,sizeof(PELCO)); 		
		if(buf[0]==0xFF)
		{
			for(u8 i=1;i<=sizeof(PELCO)-2;i++)
			{
				cc+=buf[i];
			}
			cc&=0xFF;
			
			if(cc==buf[sizeof(PELCO)-1])
			{
				ServoReload();
				
				memcpy((u8*)&PelcoData,buf,sizeof(PELCO));
				switch(PelcoData.cmd)
				{
					case PITCH_UP:           
                    {
                        PITCH.value=Channel_UP2;					   						 					
                        Servo(Pitch_Channel,PITCH.value);
                        SEGGER_RTT_printf(0,"Pitch_Channel:%d\r\n",PITCH.value);     

                    }
					break;
					
					case PITCH_DOWN:
                    {
                        PITCH.value=Channel_DOWN2;					
                        Servo(Pitch_Channel,PITCH.value);
                         SEGGER_RTT_printf(0,"Pitch_Channel:%d\r\n",PITCH.value); 
                    }
					break;
					
					case YAW_UP:
                    {
                        YAW.value=Channel_UP2;                      						
                        Servo(Yaw_Channel,YAW.value);
                         SEGGER_RTT_printf(0,"Yaw_Channel:%d\r\n",YAW.value); 
                    }
					break;
					
					case YAW_DOWN:
					{
					     YAW.value=Channel_DOWN2;									
						 Servo(Yaw_Channel,YAW.value);
                         SEGGER_RTT_printf(0,"Yaw_Channel:%d\r\n",YAW.value); 
				    }
					break;
					
					case SERVO_RETURN:
						GimbalGoZero();	
					break;
					
					case SERVO_FREEZE:
						ServoFreeze();
					    Zoom(ZOOM_STOP_R); 
					
					break;
					
					case SERVO_FOLLOW_MODE:
                        BUZZ_ON;                   
                        uartWriteBuffer(UART_GIMBAL,Servo_Yfollow_on,sizeof(Servo_Yfollow_on));
                        uartSendBufferOut(UART_GIMBAL);                                          				   
                        bsp_DelayMS(200);
                        uartWriteBuffer(UART_GIMBAL,Servo_Pfollow_on,sizeof(Servo_Pfollow_on));
                        uartSendBufferOut(UART_GIMBAL);                         
                        BUZZ_OFF;
					break;
					
					case SERVO_LOCK_MODE:
						  BUZZ_ON;                   
                          uartWriteBuffer(UART_GIMBAL,Servo_Yfollow_off,sizeof(Servo_Yfollow_off));
                          uartSendBufferOut(UART_GIMBAL);                                          				   
                          bsp_DelayMS(200);
                          uartWriteBuffer(UART_GIMBAL,Servo_Pfollow_off,sizeof(Servo_Pfollow_off));
                          uartSendBufferOut(UART_GIMBAL);  
					      BUZZ_OFF;
					break;
					
					case MOTOR_ON:
						  BUZZ_ON;						
                          uartWriteBuffer(UART_GIMBAL,motor_on,sizeof(motor_on));
                          uartSendBufferOut(UART_GIMBAL);                           
					      bsp_DelayMS(50);
					      BUZZ_OFF;
					break;
					
					case MOTOR_OFF:
						  BUZZ_ON;
                          uartWriteBuffer(UART_GIMBAL,motor_off,sizeof(motor_off));
                          uartSendBufferOut(UART_GIMBAL);   
					      bsp_DelayMS(50);				    
					      BUZZ_OFF;
					break;
										
					case ZOOM_IN://放大
						Cam_Operation(ZOOM_IN,PelcoData.data); 
                       SEGGER_RTT_printf(0,"ZOOM_IN:%d\r\n",PelcoData.data);                       
					break;
                    
					case ZOOM_OUT://缩小
						Cam_Operation(ZOOM_OUT,PelcoData.data); 	
                    SEGGER_RTT_printf(0,"ZOOM_OUT:%d\r\n",PelcoData.data);                     
					break;
					
					case ZOOM_STOP:
					    Cam_Operation(ZOOM_STOP,0);
                        SEGGER_RTT_printf(0,"ZOOM_STOP!\r\n");
                      //  bsp_DelayMS(100);	                    
                        CamZoomPosQry();                     
					break;
                                       
                    case ZOOM_TO_POS:
                        Cam_Operation(ZOOM_TO_POS,PelcoData.data); 
                        SEGGER_RTT_printf(0,"ZOOM_TO_POS:%d\r\n",PelcoData.data); 
                    break;
                    
                    case ZOOM_POS_QRY:
                        Cam_Operation(ZOOM_POS_QRY,0); 
                        SEGGER_RTT_printf(0,"ZOOM_POS_QRY!\r\n");        
                    break;
            
					case PICTURE:
						Cam_Operation(PICTURE,PelcoData.data);
                        SEGGER_RTT_printf(0,"PICTURE!\r\n");                        
					break;
					case RECORD_START:
						Cam_Operation(RECORD_START,PelcoData.data);	
                        SEGGER_RTT_printf(0,"RECORD_START!\r\n");                    
					break;
					case RECORD_STOP:
						Cam_Operation(RECORD_STOP,PelcoData.data);	
                        SEGGER_RTT_printf(0,"RECORD_STOP!\r\n");                       
					break;
                            
					case ZOOM_OUT_MAX:
                      //  Zoom(ZOOM_OUT_MAX); 
                    break;
                    
                    
                    case UPLOAD_ANGLE:
                    {
                        GetAngle();
                        Angle_Requst=1;
                        
                    }
                    break;
                    
                    case CALIB_CMD:
                    {
                        Calib_POD();
                    }
                    break;
                    case MICRO_MODIFY:
                    {
                        s16 diffPitch,diffYaw; 
                     
                        diffPitch=(s16)((s8)PelcoData.addr * ((s8)(PelcoData.data&0xFF)));
                        diffYaw=(s16)((s8)PelcoData.addr * ((s8)((PelcoData.data>>8)&0xFF)));
                        MicroModify(diffPitch,diffYaw);
                        
                    }
					break;
                    case MICRO_MODIFY_QUIT:
                    {
                        MicroModify_Quit();
                    }
                    break;
                    
					default:
//						printf("Unspport cmd:%d\r\n",PelcoData.cmd);
					break;
					
				}
				
			}
			else
			{
				uartClearRcvBuffer(UART_FK);
//				printf("cc err!\r\n");
			}
												
		}
		else
		{
			uartClearRcvBuffer(UART_FK);
	//		printf("preamble err!\r\n");
		}
		
		availBytes=uartGetAvailBufferedDataNum(UART_FK);
		if(availBytes>=sizeof(PELCO))
        {
			ClearUartRxIdleFlag(UART_FK);
            goto start;
            	
        }
			
		
	 }
  }

				
}

void ServoProcess000(void)
{
	
	u16 availBytes;
	u8 comdata;
	
	availBytes=uartGetAvailBufferedDataNum(UART_GIMBAL);
	uartRead(UART_GIMBAL,&comdata,1); 
				
	ServoReload();
	
	if(comdata=='0')
	{					
		servo_state=1;					
	}
	else
	{														
		if(comdata=='a')
		{
			YAW.value=Channel_UP;
			YAW.cnt++;						
			Servo(Yaw_Channel,YAW.value);
		}
		if(comdata=='d')
		{						 
			YAW.value=Channel_DOWN;						
			YAW.cnt--;				
			Servo(Yaw_Channel,YAW.value);
		}
		
		if(comdata=='w')
		{
			PITCH.value=Channel_UP;
			PITCH.cnt++;						
			Servo(Pitch_Channel,PITCH.value);
		}
		if(comdata=='s')
		{						 
			PITCH.value=Channel_DOWN;
			PITCH.cnt--;						
			Servo(Pitch_Channel,PITCH.value);
		}
								   
	}
				
}
/*
void PrintSBUS()
{
		for(u8 i=0;i<8;i++)
			printf("ch[%d]=%d  ",i,channels[i]);
		
		if(getFailSafeStatus()==SBUS_SIGNAL_OK)
			printf("Sbus sigal ok!\r\n");
		
		if(getFailSafeStatus()==SBUS_SIGNAL_LOST)
			printf("Sbus sigal lost!\r\n");
		
		if(getFailSafeStatus()==SBUS_SIGNAL_FAILSAFE)
			printf("Sbus sigal failsafe!\r\n");
	
	
}

void PrintServo(void)
{
		for(u8 i=0;i<8;i++)
			printf("servos[%d]=%d  ",i,servos[i]);
	
	  printf("\r\n");
	
}
*/
void SBUS_DataProcess(void)
{
    
   u8 availBytes,i;
   u8 buf[100];
	 
   u8 inData,bufferIndex;
   static u8 state=0;
	 static u16 chanle1_t=0;
   static u16 chanle2_t=0;
	 static u16 chanle4_t=0;
	 static u16 chanle3_t=0;
	 static u16 chanle7_t=0;
	
  if(GetUartRxIdleFlag(UART_SBUS)==1)
	{
	  ClearUartRxIdleFlag(UART_SBUS);
	  availBytes=uartGetAvailBufferedDataNum(UART_SBUS);
	
	  if(availBytes>=25)
	  {
	 while(availBytes>0)
	 {
				uartRead(UART_SBUS,&inData,1); 
				availBytes--;
				switch (state)			   
				{
					 case 0:
					 {
						if (inData == 0x0F)
						{
							 bufferIndex = 0;
							 buf[bufferIndex] = inData;
							 buf[24] = 0xFF;
							 state = 1;
								
						}
																				
					 }
					 break;
					 
					 case 1:
					 {
						 
						bufferIndex++;
						buf[bufferIndex] = inData;
						if (bufferIndex < 24 && availBytes == 0)
							state = 0;
					
						if (bufferIndex == 24)
						{
							state = 0;
							if (buf[0]==0x0F && buf[24] == 0)
							{
								  memcpy(sbusData,buf,25);								  
								
								  SBUS_UpdateChannels();
//								  PrintSBUS();
								  bsp_DelayMS(10);
								
								  if(chanle1_t!=channels[1])//通道2俯仰角
									{
										chanle1_t=channels[1]; 										
//										printf("pitch=plus:%d, %0.2f\r\n",chanle1_t,(float)chanle1_t/20);	
									}
									if(chanle3_t!=channels[3])//通道4航行角
									{
										chanle3_t=channels[3]; 										
//										printf("yaw:plus:%d, %0.2f\r\n",chanle3_t,(float)chanle3_t/20);	
									}
									
									
									if(chanle2_t!=channels[2])
									{
								        chanle2_t=channels[2]; 
										Zoom(chanle2_t); 
//										printf("channels[3]=plus:%d, %0.2f\r\n",chanle2_t,(float)chanle2_t/20);	
									}
									
									if(channels[4]<1000)
									{
										if(chanle4_t!=channels[4])
										{
									          Picture_D();
//											  printf("Picture_D\r\n");
											  chanle4_t=channels[4];
										}
									
									}
									else if(channels[4]>1000 && chanle4_t<1000)
									{												
									  Picture_P();
//											  printf("Picture_P\r\n");
											  chanle4_t=channels[4];												
									}
										
									
									if(channels[4]>1200)
									{
										if(chanle4_t!=channels[4])
										{
									          Record_D();
//											  printf("Record_D\r\n");
											  chanle4_t=channels[4];
										}
									
									}
									else if(chanle4_t>1200 && channels[4]<1200)
									{												
												Record_P();
//											  printf("Record_P\r\n");
											  chanle4_t=channels[4];													
									}
																
							}
						
					  }
						else if(bufferIndex>24)
								state = 0;
						
					}
					break;
					 
				}
				
				
			}
				
		 }			  
	        
   }
}

void SBUS_UpdateChannels(void) 
{
 
  channels[0]  = ((sbusData[1]|sbusData[2]<< 8) & 0x07FF);
  channels[1]  = ((sbusData[2]>>3|sbusData[3]<<5) & 0x07FF);
  channels[2]  = ((sbusData[3]>>6|sbusData[4]<<2|sbusData[5]<<10) & 0x07FF);
  channels[3]  = ((sbusData[5]>>1|sbusData[6]<<7) & 0x07FF);
  channels[4]  = ((sbusData[6]>>4|sbusData[7]<<4) & 0x07FF);
  channels[5]  = ((sbusData[7]>>7|sbusData[8]<<1|sbusData[9]<<9) & 0x07FF);
  channels[6]  = ((sbusData[9]>>2|sbusData[10]<<6) & 0x07FF);
  channels[7]  = ((sbusData[10]>>5|sbusData[11]<<3) & 0x07FF); // & the other 8 + 2 channels if you need them
/*
  channels[8]  = ((sbusData[12]|sbusData[13]<< 8) & 0x07FF);
  channels[9]  = ((sbusData[13]>>3|sbusData[14]<<5) & 0x07FF);
  channels[10] = ((sbusData[14]>>6|sbusData[15]<<2|sbusData[16]<<10) & 0x07FF);
  channels[11] = ((sbusData[16]>>1|sbusData[17]<<7) & 0x07FF);
  channels[12] = ((sbusData[17]>>4|sbusData[18]<<4) & 0x07FF);
  channels[13] = ((sbusData[18]>>7|sbusData[19]<<1|sbusData[20]<<9) & 0x07FF);
  channels[14] = ((sbusData[20]>>2|sbusData[21]<<6) & 0x07FF);
  channels[15] = ((sbusData[21]>>5|sbusData[22]<<3) & 0x07FF);

  // DigiChannel 1
  if (sbusData[23] & (1<<0)) 
    channels[16] = 1;
  else
    channels[16] = 0;
  
  // DigiChannel 2
  if (sbusData[23] & (1<<1)) 
    channels[17] = 1;
  else
    channels[17] = 0;
 */ 
  // Failsafe
  failsafe_status = SBUS_SIGNAL_OK;
  if (sbusData[23] & (1<<2)) 
    failsafe_status = SBUS_SIGNAL_LOST;
  
  if (sbusData[23] & (1<<3))
    failsafe_status = SBUS_SIGNAL_FAILSAFE;
  

}

u16 getChannel(u8 ch) 
{
  // Read channel data
  if ((ch>0)&&(ch<=16))
    return channels[ch-1];
  else
    return 1023;
  
}

u8 getDigiChannel(u8 ch) 
{
  // Read digital channel data
  if ((ch>0) && (ch<=2)) 
    return channels[15+ch]; 
  else
    return 0;
  
}

void Servo(u8 ch, u16 position) //352~1696
{
  // Set servo position
  if ((ch>0) && (ch<=16)) 
  {
    if (position>2048) 
	{
      position=2048;
    }
    servos[ch-1] = position;
  }
  
}

void DigiServo(u8 ch, u8 position) 
{
  // Set digital servo position
  if ((ch>0) && (ch<=2)) 
  {
    if (position>0) 
	{
      position=1;
    }
    servos[15+ch] = position;
  }
  
}

SBUS_SIGNAL_STATE getFailSafeStatus(void)
{
	return failsafe_status;
}


void update_servos(void) 
{
    // Send data to servos
    // Passtrough mode = false >> send own servo data
    // Passtrough mode = true >> send received channel data
    uint8_t i;
   // if (!sbus_passthrough) 
		{
        // clear received channel data
        for (i=1; i<24; i++) 
		{
            sbusData[i] = 0;
        }
    
        // reset counters
        uint8_t ch = 0;
        uint8_t bit_in_servo = 0;
        uint8_t byte_in_sbus = 1;
        uint8_t bit_in_sbus = 0;
        
        // store servo data
        //sbus数据没11bit表示一个通道，把总共176个bit也就是16个模拟通道的数据值，另加2个数字通道，分别放到u8型数组中，因此数组共计24个元素，sbusdata[0]是标志头
        for (i=0; i<176; i++) 
		{
            if (servos[ch] & (1<<bit_in_servo)) 
			{
                sbusData[byte_in_sbus] |= (1<<bit_in_sbus);
            }
            bit_in_sbus++;
            bit_in_servo++;

            if (bit_in_sbus == 8) 
			{
                bit_in_sbus =0;
                byte_in_sbus++;
            }
            if (bit_in_servo == 11) 
			{
                bit_in_servo =0;
                ch++;
            }
        }
    
        // DigiChannel 1
        if (channels[16] == 1) 
		{
            sbusData[23] |= (1<<0);
        }
        // DigiChannel 2
        if (channels[17] == 1) 
		{
            sbusData[23] |= (1<<1);
        }        
        
        // Failsafe
        if (failsafe_status == SBUS_SIGNAL_LOST) 
		{
            sbusData[23] |= (1<<2);
        }
        
        if (failsafe_status == SBUS_SIGNAL_FAILSAFE) 
		{
            sbusData[23] |= (1<<2);
            sbusData[23] |= (1<<3);
        }
    }
    // send data out
	sbusData[0]=0x0F;
    
    
	 uartWriteBuffer(UART_SBUS,sbusData,sizeof(sbusData));                    
     uartSendBufferOut(UART_SBUS); 
		
//		 for(i=0;i<25;i++)
//			 SEGGER_RTT_printf(0,"%02X ",sbusData[i]);
//		
//		 SEGGER_RTT_printf(0,"\r\n");

}




void ServoActTask(void)
{
    if(servo_state==SERVO_NORMAL)
    {
        if(PITCH.value==Channel_DOWN2)
        {
            if(PITCH.cnt>=PITCH_DOWN_LIMIT)
                PITCH.cnt--;
            else
            {
                PITCH.cnt=PITCH_DOWN_LIMIT;
                PITCH.value=Channel_MID;
                Servo(Pitch_Channel,PITCH.value);
            }
                
        }
                            
        if(PITCH.value==Channel_UP2)
        {
            if(PITCH.cnt<=PITCH_UP_LIMIT)
                 PITCH.cnt++;
            else
            {
                PITCH.cnt=PITCH_UP_LIMIT;
                PITCH.value=Channel_MID;
                Servo(Pitch_Channel,PITCH.value);
            }
            
        }
                        
        if(YAW.value==Channel_DOWN2)
        {
            if(YAW.cnt>=YAW_DOWN_LIMIT)
                YAW.cnt--;
            else
            {
                 YAW.cnt=YAW_DOWN_LIMIT;
                 YAW.value=Channel_MID;
                 Servo(Yaw_Channel,YAW.value);
            }
        }
        
        if(YAW.value==Channel_UP2)
        {
            if(YAW.cnt<=YAW_UP_LIMIT)
                YAW.cnt++;
            else
            {
                 YAW.cnt=YAW_UP_LIMIT;
                 YAW.value=Channel_MID;
                 Servo(Yaw_Channel,YAW.value);
            }
        }
        
    }
    
//	 SEGGER_RTT_printf(0,"PITCH.cnt:%d,YAW.cnt:%d\r\n",PITCH.cnt,YAW.cnt);
     update_servos();	
    
}

