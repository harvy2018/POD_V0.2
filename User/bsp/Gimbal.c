

#include "Gimbal.h"
#include "uartPort.h"
#include "SEGGER_RTT.h"
#include "board.h"	
#include "pelco_d.h"

ANGLES_EXT_DATA AngleDataRT;

extern u8 Angle_Requst;

CALIB_STATE Calib_POD_Flag=CALBI_INIT;

//s16 ANGLE_PITCH_IMU_INIT=0;
//s16 ANGLE_PITCH_SRT_INIT=0;
//s16 ANGLE_YAW_IMU_INIT=0;
//s16 ANGLE_YAW_SRT_INIT=0;

void SendGimbalCmd(u8 cmdid,u8 *buf,u8 datasize)
{
    GIMBAL_SERIAL_HEADER header;
    u8 datacc=0;
    u8 sendbuf[128]={0};
    header.Preamble=PREAMBLE;
    header.CmdID=cmdid;
    header.DataSize=datasize;
    header.HeaderCC=header.CmdID+ header.DataSize;
    
    for(u8 i=0;i<datasize;i++)
       datacc+=buf[i];
    
    
    memcpy(sendbuf,(u8*)&header,sizeof(header));
    memcpy(sendbuf+sizeof(header),buf,datasize);
    memcpy(sendbuf+sizeof(header)+datasize,&datacc,1);
    
    uartWriteBuffer(UART_GIMBAL,sendbuf,sizeof(header)+datasize+1);//多发一个datacc
    uartSendBufferOut(UART_GIMBAL);
    
    
}

void GetAngle(void)
{   
    u8 para=0;
   
    SendGimbalCmd(CMD_GET_ANGLES_EXT,&para,0);
}

void Calib_POD(void)
{
    u8 para=0;
    Calib_POD_Flag=CALBI_GYRO;
    SEGGER_RTT_printf(0,"start gyro calib!\r\n"); 
    SendGimbalCmd(CMD_CALIB_GYRO,&para,0);   
}


void Calib_ACC(void)
{
    u8 para=0;
    Calib_POD_Flag=CALBI_ACC;
    SendGimbalCmd(CMD_CALIB_ACC,&para,0);   
}

void Calib_MAG(void)
{
     u8 para=0; 
     Calib_POD_Flag=CALBI_MAG;   
     SendGimbalCmd(CMD_CALIB_MAG,&para,0);   
}


void Calib_OK(void)
{
    PELCO Pelcodata; 

    Calib_POD_Flag=CALBI_INIT;   
    Pelcodata.preamble=0xFF;
    Pelcodata.addr=0x01;
    Pelcodata.cmd=CALIB_CMD;
    Pelcodata.data=0;
    Pelcodata.data=0;
    Pelcodata.cc=0x45;
    
    uartWriteBuffer(UART_FK,(u8*)&Pelcodata,sizeof(PELCO));
    uartSendBufferOut(UART_FK);    
   
}

void MicroModify(s16 diffPitch,s16 diffYaw)
{
   u16 speed;
   CMD_CONTROL_DATA control_data;
      
   speed=20/SPEED_SCALE;
   
   control_data.CONTROL_MODE =MODE_ANGLE;
   control_data.SPEED_PITCH  =speed;
   control_data.SPEED_ROLL   =speed;
   control_data.SPEED_YAW    =speed;
   
   control_data.ANGLE_ROLL=0;
   control_data.ANGLE_PITCH = diffPitch + AngleDataRT.PITCH_IMU_ANGLE;
   control_data.ANGLE_YAW = diffYaw + AngleDataRT.YAW_IMU_ANGLE;
   

   SendGimbalCmd(CMD_CONTROL,(u8*)&control_data,sizeof(CMD_CONTROL_DATA));
    
 
    
}

void MicroModify_Quit(void)
{
   u16 speed;
   CMD_CONTROL_DATA control_data;
      
   speed=0;
   
   control_data.CONTROL_MODE =MODE_NO_CONTROL;
   control_data.SPEED_PITCH  =speed;
   control_data.SPEED_ROLL   =speed;
   control_data.SPEED_YAW    =speed;
   
   control_data.ANGLE_ROLL=0;
   control_data.ANGLE_PITCH = AngleDataRT.PITCH_IMU_ANGLE;
   control_data.ANGLE_YAW =AngleDataRT.YAW_IMU_ANGLE;
   

   SendGimbalCmd(CMD_CONTROL,(u8*)&control_data,sizeof(CMD_CONTROL_DATA));
 
}

void UploadAngle(void)
{
    u8 buf[13]={0};
    
    buf[0]=0xFF;
    buf[1]=0x01;
    buf[2]=(UPLOAD_ANGLE>>8)&0xFF;
    buf[3]= UPLOAD_ANGLE&0xFF;  
    buf[4]=(AngleDataRT.PITCH_IMU_ANGLE>>8)&0xFF;
    buf[5]=AngleDataRT.PITCH_IMU_ANGLE&0xFF;
    buf[6]=(AngleDataRT.YAW_IMU_ANGLE>>8)&0xFF;  
    buf[7]=AngleDataRT.YAW_IMU_ANGLE&0xFF;

    buf[8]=(AngleDataRT.PITCH_STATOR_ROTOR_ANGLE>>8)&0xFF;
    buf[9]=AngleDataRT.PITCH_STATOR_ROTOR_ANGLE&0xFF;
    buf[10]=(AngleDataRT.YAW_STATOR_ROTOR_ANGLE>>8)&0xFF;  
    buf[11]=AngleDataRT.YAW_STATOR_ROTOR_ANGLE&0xFF;
    for(u8 i=1;i<12;i++)
     buf[12]+=buf[i];
    
    uartWriteBuffer(UART_FK,buf,sizeof(buf));
    uartSendBufferOut(UART_FK);
}


void GimbalAngleSet(s16 pitch,s16 yaw)
{

   u16 speed;
   CMD_CONTROL_DATA control_data;
   
   speed=SPEED/SPEED_SCALE;
   
   control_data.CONTROL_MODE=MODE_ANGLE;
   control_data.SPEED_PITCH=speed;
   control_data.SPEED_ROLL=speed;
   control_data.SPEED_YAW=speed;
   
   control_data.ANGLE_ROLL=0;
   control_data.ANGLE_PITCH=pitch;
   control_data.ANGLE_YAW=yaw;
   

   SendGimbalCmd(CMD_CONTROL,(u8*)&control_data,sizeof(CMD_CONTROL_DATA));
   bsp_DelayMS(1500);
   
   speed=0;
   control_data.CONTROL_MODE=MODE_NO_CONTROL; 
   control_data.SPEED_PITCH=speed;
   control_data.SPEED_ROLL=speed;
   control_data.SPEED_YAW=speed;
   SendGimbalCmd(CMD_CONTROL,(u8*)&control_data,sizeof(CMD_CONTROL_DATA));
    
}

void GimbalGoZero(void)
{
//    s16 diff_pitch_imu=0,diff_pitch_srt=0,diff_yaw_imu=0,diff_yaw_srt=0;
//    
//    diff_pitch_imu= AngleDataRT.PITCH_IMU_ANGLE;
//    diff_pitch_srt=AngleDataRT.PITCH_STATOR_ROTOR_ANGLE;
//    diff_yaw_imu= AngleDataRT.YAW_IMU_ANGLE;
//    diff_yaw_srt=AngleDataRT.YAW_STATOR_ROTOR_ANGLE;
    
    
    GimbalAngleSet(AngleDataRT.PITCH_IMU_ANGLE-AngleDataRT.PITCH_STATOR_ROTOR_ANGLE,
                   AngleDataRT.YAW_IMU_ANGLE-AngleDataRT.YAW_STATOR_ROTOR_ANGLE);
                
                
}

void GimbalDataProcess(void)
{
    GIMBAL_SERIAL_HEADER *header;

    float pitch_imu=0,yaw_imu=0;
    float pitch_sr=0,yaw_sr=0;
    
    char txt[128]={0};
    u8 availBytes,datacc=0;
    u8 rxbuf[255];

    if(GetUartRxIdleFlag(UART_GIMBAL)==1)
	{
	  ClearUartRxIdleFlag(UART_GIMBAL);
        
      availBytes=uartGetAvailBufferedDataNum(UART_GIMBAL);
	  if(availBytes>sizeof(GIMBAL_SERIAL_HEADER))
	  {           
        uartRead(UART_GIMBAL,rxbuf,availBytes); 

		
        header=(GIMBAL_SERIAL_HEADER *)rxbuf;

        if(header->HeaderCC==(header->DataSize+header->CmdID))
        {
               
            for(u8 i=0;i<header->DataSize;i++)
              datacc+=rxbuf[i+sizeof(GIMBAL_SERIAL_HEADER)];
        
            if(datacc==rxbuf[sizeof(GIMBAL_SERIAL_HEADER)+header->DataSize])
            {
            
                switch(header->CmdID)
                {
                    case CMD_GET_ANGLES_EXT:
                    {
                        ANGLES_EXT_DATA *AngleData;
                        AngleData=(ANGLES_EXT_DATA *)(rxbuf+sizeof(GIMBAL_SERIAL_HEADER));
             
                        AngleDataRT.ROLL_IMU_ANGLE=AngleData->ROLL_IMU_ANGLE;
                        AngleDataRT.ROLL_STATOR_ROTOR_ANGLE=AngleData->ROLL_STATOR_ROTOR_ANGLE;
                        
                        AngleDataRT.PITCH_IMU_ANGLE=AngleData->PITCH_IMU_ANGLE;
                        AngleDataRT.PITCH_STATOR_ROTOR_ANGLE=AngleData->PITCH_STATOR_ROTOR_ANGLE;
                                   
                        AngleDataRT.YAW_IMU_ANGLE=AngleData->YAW_IMU_ANGLE;
                        AngleDataRT.YAW_STATOR_ROTOR_ANGLE=AngleData->YAW_STATOR_ROTOR_ANGLE;
                        
               /*         
                        if(AngleDataRT.PITCH_IMU_ANGLE & 0x8000)  //角度是负值        
                        {
                            AngleDataRT.PITCH_IMU_ANGLE=0x10000-AngleDataRT.PITCH_IMU_ANGLE;
                            AngleDataRT.PITCH_IMUAngle_NegFalg=1;
                        }
                        if(AngleDataRT.PITCH_STATOR_ROTOR_ANGLE & 0x8000)  //角度是负值        
                        {
                            AngleDataRT.PITCH_STATOR_ROTOR_ANGLE=0x10000-AngleDataRT.PITCH_STATOR_ROTOR_ANGLE;
                            AngleDataRT.PITCH_SRAngle_NegFalg=1;
                        }
                        
                        if(AngleDataRT.YAW_IMU_ANGLE & 0x8000)  //角度是负值        
                        {
                            AngleDataRT.YAW_IMU_ANGLE=0x10000-AngleDataRT.YAW_IMU_ANGLE;
                            AngleDataRT.YAW_IMUAngle_NegFalg=1;
                        }
                        if(AngleDataRT.YAW_STATOR_ROTOR_ANGLE & 0x8000)  //角度是负值        
                        {
                            AngleDataRT.YAW_STATOR_ROTOR_ANGLE=0x10000-AngleDataRT.YAW_STATOR_ROTOR_ANGLE;
                            AngleDataRT.YAW_SRAngle_NegFalg=1;
                        }
                        */
                        pitch_imu=(float)AngleDataRT.PITCH_IMU_ANGLE*ANGLE_DEGREE_SCALE;
                        pitch_sr=(float)AngleDataRT.PITCH_STATOR_ROTOR_ANGLE*ANGLE_DEGREE_SCALE;
                        
                        yaw_imu=(float)AngleDataRT.YAW_IMU_ANGLE*ANGLE_DEGREE_SCALE;
                        yaw_sr=(float)AngleDataRT.YAW_STATOR_ROTOR_ANGLE*ANGLE_DEGREE_SCALE;
                        
                        sprintf(txt,"Pitch imu:%0.2f,Pitch sr:%0.2f, Yaw imu:%0.2f,Yaw sr:%0.2f\r\n",
                               pitch_imu,pitch_sr,yaw_imu,yaw_sr);
                        
                        SEGGER_RTT_printf(0,"%s",txt);
                        
                        if(Angle_Requst==1)
                        {
                            UploadAngle();
                            Angle_Requst=0;
                        }
                        
                    }
                    break;
                    case CMD_CONFIRM:
                    {

                        switch(Calib_POD_Flag)
                        {
                            case CALBI_GYRO:                       
                                if(header->DataSize==2)                                  
                                {
                                    SEGGER_RTT_printf(0,"start acc calib!\r\n"); 
                                    Calib_ACC(); 
                                }                                    
                            break;
                            
                            case CALBI_ACC:                           
                                if(header->DataSize==2)                                  
                                {
                                    SEGGER_RTT_printf(0,"start mag calib!\r\n"); 
                                    Calib_MAG();  
                                }                                    
                            break;
                                
                            case CALBI_MAG:                                                             
                                 Calib_OK(); 
                                 SEGGER_RTT_printf(0,"pod calib ok!\r\n"); 
                            break;
                           
                        }
                        
                            
                    }                    
                    break;
                    
                    
                    default:
                       SEGGER_RTT_printf(0,"not support cmd!\r\n",txt); 
                    break;
                            
                        
                  }
              }
                  
          }
        
              
      }
  }
       
}
