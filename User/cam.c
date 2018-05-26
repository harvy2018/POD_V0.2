
#include "SEGGER_RTT.h"
#include "cam.h"
#include "pelco_d.h"
#include "uartPort.h"

u16 ZoomPoistion[30]=
{
    0x0000,0x17ff,0x222, 0x2834,0x2c90,0x2fd8,
    0x327d,0x34bd,0x36b3,0x3850,0x39a3,0x3ab7,
    0x3ba5,0x3c6d,0x3d04,0x3d81,0x3df2,0x3e4a,
    0x3e95,0x3ee0,0x3f06,0x3f38,0x3f5e,0x3f83,
    0x3fa9,0x3fce,0x3fda,0x3fe6,0x3ff2,0x4000 
};

u8 ZoomStopCmd[] ={0x81,0x01,0x04,0x07,0x00,0xFF};
u8 ZoomInCmd[] =  {0x81,0x01,0x04,0x07,0x02,0xFF};
u8 ZoomOutCmd[]=  {0x81,0x01,0x04,0x07,0x03,0xFF};
u8 ZoomPosQury[]= {0x81,0x09,0x04,0x47,0xFF}; 
u8 ZoomToPos[]=   {0x81,0x01,0x04,0x47,0,0,0,0,0xFF};

u8 RP_Mode_SW[]={0xFF,0x01,0x00,0x07,0x00,0x67,0x6F};
u8 PictureCmd[]={0xFF,0x01,0x00,0x07,0x00,0x66,0x6E};
                                     
u8 RecordStartCmd[]={0xFF,0x01,0x00,0x07,0x00,0x65,0x6D};
u8 RecordStopCmd[] ={0xFF,0x01,0x00,0x07,0x00,0x64,0x6C};  


void CamCmdSend(u8 *cmdbuf,u8 len)
{
   uartWriteBuffer(UART_Camera,cmdbuf,len);
   uartSendBufferOut(UART_Camera);  
}

void Cam_Operation(u16 cmd,u16 para)
{
    switch (cmd)
    {
        case ZOOM_IN:
        {
             if(para>0x07)
             {               
                  CamCmdSend(ZoomInCmd,sizeof(ZoomInCmd));
                 
             }
             else 
             {
                 u8 ZoomInX_Cmd[6]={0x81,0x01,0x04,0x07,0x20,0xFF};              
                 ZoomInX_Cmd[4]=0x20+para;
                 
                 CamCmdSend(ZoomInX_Cmd,sizeof(ZoomInX_Cmd));
             }
                 
        }
        break;
        case ZOOM_OUT:
        {            
            if(para>0x07)
            {
               
                 CamCmdSend(ZoomOutCmd,sizeof(ZoomOutCmd));
            }
            else
            {
                 u8 ZoomOutX_Cmd[6]={0x81,0x01,0x04,0x07,0x30,0xFF};                
                 ZoomOutX_Cmd[4]=0x30+para;
               
                 CamCmdSend(ZoomOutX_Cmd,sizeof(ZoomOutX_Cmd));
            }
        }
        break;
        case ZOOM_STOP:
        {
             
             CamCmdSend(ZoomStopCmd,sizeof(ZoomStopCmd));
        }
        break;
        case ZOOM_TO_POS:
        {           
             u16 position;
             if(para>30)
                 break;
             
             position=ZoomPoistion[para-1];
                                  
             ZoomToPos[4]=(position&0xF000)>>12;
             ZoomToPos[5]=(position&0x0F00)>>8;
             ZoomToPos[6]=(position&0x00F0)>>4;
             ZoomToPos[7]=position&0x000F;
                                                                     
           
             CamCmdSend(ZoomToPos,sizeof(ZoomToPos));
             
        }
        break;
        case ZOOM_POS_QRY:
        {
           
             CamCmdSend(ZoomPosQury,sizeof(ZoomPosQury));
        }
        break;
        case PICTURE:
        {                
             CamCmdSend(PictureCmd,sizeof(PictureCmd));
		}		
        break;
        case RECORD_START:
        {          
             CamCmdSend(RecordStartCmd,sizeof(RecordStartCmd));
		}		
        break;
        case RECORD_STOP:
        {         
             CamCmdSend(RecordStopCmd,sizeof(RecordStopCmd));
		}		
        break;
        
        default:
            break;
    }
}

void CamDataProcess(void)
{
    u8 availBytes;
    u8 cambuf[32];
    u16 pqrs=0;//变焦参数
    u8 OpticalZoom=0;
    
    if(GetUartRxIdleFlag(UART_Camera)==1)
    {	
	    ClearUartRxIdleFlag(UART_Camera);
        
        availBytes=uartGetAvailBufferedDataNum(UART_Camera);
        
        if(availBytes>=6 && availBytes<=32)
        {
            uartRead(UART_Camera,cambuf,availBytes); 
            
            if(cambuf[0]==0x90)
            {
                switch (cambuf[1])
                {
                    case 0x50:
                    {
                        if(cambuf[6]==0xFF)//尾部标识
                        {
                            for(u8 i=0;i<4;i++)
                            {
                                pqrs|=cambuf[2+i]&0x0F;
                                if(i!=3)
                                pqrs<<=4;
                            }
                            
                            OpticalZoom=GetOpticalZoom(pqrs);
                            UploadOpticalZoom(OpticalZoom);
                            SEGGER_RTT_printf(0,"OpticalZoom=%dx pqrs=%04X\r\n",OpticalZoom,pqrs);
                        }
                    }
                    break;
                    
                    default:
                        break;
                                                           
                }
            }
            else
            {
                uartClearRcvBuffer(UART_Camera);
            }
                                  
        }
        else
        {
             uartClearRcvBuffer(UART_Camera);
        }
        
    }
                     
}


u8 GetOpticalZoom(u16 pqrs)
{
    for(u8 i=0;i<30;i++)
    {
        if(ZoomPoistion[i]==pqrs)
        {
            return i+1;
        }
    }
    
    for(u8 i=0;i<29;i++)
    {
        if(ZoomPoistion[i]<pqrs && ZoomPoistion[i+1]>pqrs)
        {
            if((pqrs-ZoomPoistion[i])<(ZoomPoistion[i+1]-pqrs))
               return i+1;
            else  if((pqrs-ZoomPoistion[i])>=(ZoomPoistion[i+1]-pqrs))
               return i+2;                     
        }
    }
       
}

void UploadOpticalZoom(u8 OpticalZoom)
{
    u8 buf[6]={0};
    
    
    buf[0]=0xFF;
    buf[1]=0x01;
    buf[2]=(UPLOAD_OPTICAL_ZOOM>>8)&0xFF;
    buf[3]= UPLOAD_OPTICAL_ZOOM&0xFF;  
    buf[4]=OpticalZoom;

    for(u8 i=1;i<5;i++)
     buf[5]+=buf[i];
    
    uartWriteBuffer(UART_FK,buf,sizeof(buf));
    uartSendBufferOut(UART_FK);
}

void CamZoomPosQry(void)
{
     CamCmdSend(ZoomPosQury,sizeof(ZoomPosQury));
}



