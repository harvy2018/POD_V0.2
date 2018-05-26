#ifndef __SERIALPORT_H_
#define __SERIALPORT_H_


#include "stm32f10x.h"
#include <stdbool.h>
#include <string.h>
extern u8 g_bShutDown;

u8 InitPort(u8 portNum);
u8* Read(UINT16 length, UINT16* bytesRead);
bool Write(BYTE* bytes, UINT16 length);
//u8 ClosePort();

void SerialReadThread(void);


#endif

