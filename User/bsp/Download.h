/*-------------------------------------------------------------------------

                            ���ز���ͷ�ļ�

                            
-------------------------------------------------------------------------*/


#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"	


/* Constants used by Serial Command Line Mode */
#define CMD_STRING_SIZE         128

#define ApplicationAddress      0x8003000       //APP�����׵�ַ
#define Config_Address          0x807F800       //������ַ
#define Systeminfo_Address      0x807F000       //ϵͳ��Ϣ��ַ
//#define ApplicationSize         15*1024       //����Ԥ���ռ�

//#define ApplicationSize			((*(vu16 *)(0x1FFFF7E0)) * 1000) //оƬ�洢�ռ�


#if defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
 #define PAGE_SIZE                         (0x400)    /* 1 Kbyte */
 #define FLASH_SIZE                        (0x20000)  /* 128 KBytes */
#elif defined STM32F10X_CL
 #define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
 #define FLASH_SIZE                        (0x40000)  /* 256 KBytes */
#elif defined STM32F10X_HD
 #define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
 #define FLASH_SIZE                        (0x80000)  /* 512 KBytes */
#endif


#define ADU_LENGTH 0x400

typedef  void (*FunVoidType)(void);


void CommonInit(void);
void JumpToApp(void);
void CommonExec(void);

FLASH_Status FLASH_ProgramStart(u32 ADDR,vu32 ApplicationSize);
void FLASH_ProgramDone(void);
u32 FLASH_WriteBank(u8 *pData, u32 addr, u32 size);


#endif
/********************** END ***************************************************/

