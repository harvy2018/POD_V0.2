
#include "stm32f10x.h"	
#include "BD.h"
#include "bsp_usart.h"	
#include "uartPort.h"	


typedef u32 crc24;  
#define CRC24_INIT 0x000000; 
#define CRC24_POLY 0x1864cfb; 

// 24-bit CRC checking 
//------------------------------------------------------ 
crc24 crc_octets(u8 *octets, u32 len) 
{ 
      crc24 crc = CRC24_INIT; 
      int i; 
 
      while(len--)  
      { 
            crc ^= (*octets++) << 16; 
            for (i = 0; i < 8; i++)  
            { 
                  crc <<= 1; 
                  if (crc & 0x1000000) 
                  crc ^= CRC24_POLY; 
            } 
      } 
      return crc & 0xffffff; 
} 

u8  BD_MessagePro(u8 *databuf,u16 datalen)
{

    crc24 CRC_calculated = 0; 

    u8 cFirst; 

    u32 nMsg1004_Counter = 0; 
    u32 nMsg1005_Counter = 0; 
    u32 nMsg1104_Counter = 0; 

    u32 MsgID = 0; 
    u32 nMsgLen = 0; 

    u8 msgBuf[1024]; 
    u8 crcBuf[3]; 
  //  u16 counter=0;

    cFirst=databuf[0];          
    
    if( (cFirst & 0xFF) == 0xD3 ) 
    { 
          u8 cTemp1 = databuf[1];  
          // Check Reserved 6 bits == 0, 0xFC = 111111 00 
          if((cTemp1 & 0xFC) != 0) 
                return 3; 

          u8 cTemp2 = databuf[2];
          //read message length; 
          nMsgLen = (cTemp1 & 0x03)*256 + (cTemp2 & 0xFF); 

          msgBuf[0] = cFirst; 
          msgBuf[1] = cTemp1; 
          msgBuf[2] = cTemp2; 

          // read message contents 
          for(u32 j = 0; j < nMsgLen; j++) 
                msgBuf[j+3] =databuf[j+3]; 
           
          // read CRC-24 parity 
          for(u32 j = 0; j < 3; j++) 
                crcBuf[j] = databuf[datalen-4+j];  
           
          // Perform CRC check 
          CRC_calculated = crc_octets(msgBuf, nMsgLen + 3); 
           // If CRC check passed, decode message data 
          if( CRC_calculated == (crcBuf[0] << 16)  + (crcBuf[1] << 8) + crcBuf[2] ) 
          {  
              return 0;
//                MsgID = msgBuf[3 + 0]*16+(msgBuf[3+1] & 0xF0)/16; 

//                switch(MsgID) 
//                { 
//                case 1004: 
//                      nMsg1004_Counter++; 
//                      break; 

//                case 1005: 
//                      nMsg1005_Counter++; 
//                      break; 
//                                         
//                case 1104: 
//                      nMsg1104_Counter++; 
//                      break; 
//                 
//                default:  
//                      printf("Message:%d, not defined message number!",MsgID);

//                      break; 
//                } 
          } 
          else  // CRC24 check failed 
          { 
//                printf("CRC24 check not passed!\r\n");  
                 return 2;
                // TO DO, Deal with CRC24 check failure; 
          } 

    }     // EOF if(ch_buf[i]=='D') 
       return 1;   
    
}



