
#ifndef _FLASHMGR_H_
#define _FLASHMGR_H_


#include "stm32f10x.h"	
#include "stdint.h"	
#include "stm32f10x_flash.h"

#pragma pack(1)
typedef enum _Flash_Sector_ID
{
	FLASH_SECTION_FW_UPGRADE=0,
	FLASH_SECTION_GAS_DATA,
	FLASH_SECTION_STATISTICS_DATA,
	FLASH_SECTION_PLCSETUP_DATA,
	FLASH_SECTION_TAXCODE_DATA,
	
	FLASH_SECTION_FW_BOOT,
	FLASH_SECTION_PINGPONG_SEL,
	FLASH_SECTION_FLASH_MANAGER_VER,
	FLASH_SECTION_SYSTEM_INFO,
	FLASH_SECTION_CONFIG_PARA,
	FLASH_SECTION_SPI_TEST,
	FLASH_SECTION_FW_PING,
	FLASH_SECTION_FW_PONG,
	FLASH_SECTION_MAX
	
}Flash_Sector_ID;


typedef enum _FLASH_TYPE
{
	FLASH_IN,			// Ƭ��FLASH
	FLASH_EX,			// Ƭ��FLASH
	FLASH_MAX
	
}FLASH_TYPE;	

//��ƻ������ݽṹ�����㲻ͬ��ʹ������
typedef struct _FlashSectionStruct
{
	FLASH_TYPE type;     //  Flash ����
	 u32     base;      //  ��������ʼ��ַ
   u32     size;	 //  �����Ĵ�С
	u32     wptr;     //  дָ��
	u32     rptr;      //   ��ָ��
	
}FlashSectionStruct;

#pragma pack()

void uf_InitHard(FLASH_TYPE type);
uint32_t uf_ReadID(FLASH_TYPE type);

void uf_EraseChip(FLASH_TYPE type);
void uf_EraseSector(FLASH_TYPE type,uint32_t _uiSectorAddr);
void uf_EraseBlock(FLASH_TYPE type,uint8_t blocksize, uint32_t _uiSectorAddr);
//void uf_PageWrite(FLASH_TYPE type,uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
uint8_t uf_WriteBuffer(FLASH_TYPE type,uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize);
void uf_ReadBuffer(FLASH_TYPE type,uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);



extern  FlashSectionStruct flashSectionStruct[];
extern u8 Area_Used;
extern u8 Area_Unused;
extern u8 Area_Info;



#endif

