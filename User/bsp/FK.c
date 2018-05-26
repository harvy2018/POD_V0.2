
#include "stm32f10x.h"	
#include "FK.h"
#include "bsp_usart.h"	
#include "uartPort.h"	



void FK_MessagePro(void)
{
   FKPro *ptr;    
   u16 availBytes,i;
   u8 buf[1024];
   u8 checkcode=0,cc;
   
   ptr=(FKPro *)buf;
    
    if(GetUartRxIdleFlag(UART_FK)==1)
    {
          ClearUartRxIdleFlag(UART_FK);
          availBytes=uartGetAvailBufferedDataNum(UART_FK);                   
          uartRead(UART_FK,buf, availBytes);
          
          if(ptr->FunctionID==0x24)
          {
              cc=buf[availBytes-1];
             
              for(i=0;i<availBytes-1;i++)
                 checkcode+=buf[i]; 
              
              if(checkcode==cc)
              {
                  switch(ptr->Address)
                  {

                      case 0x42://from K505
                      {
                          
                      }
                      break;

                      default:
                      {
//                          uartWriteBuffer(UART_GS,buf,availBytes);                  
//                          uartSendBufferOut(UART_GS);
                      }
                      break;
                                                
                  }
              }
              else
              {
                  printf("DTR data check error!\r\n");
              }
          }      
        
    }
}
