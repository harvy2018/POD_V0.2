

#include "stm32f10x.h"	
#include "flashmgr.h"	
#include "bsp_spi_flash.h"
#include "download.h"	

u8 Area_Used=0xFF; 
u8 Area_Unused=0; 
u8 Area_Info;//
////系统上电后做如下初始化：FLASH_SECTION_MAX
FlashSectionStruct flashSectionStruct[FLASH_SECTION_MAX]=
{
	{ FLASH_EX,    0x000000,   960*1024,   0x000000,  	0x000000}, //FLASH_SECTION_FW_UPGRADE
	{ FLASH_EX,    0x0F0000,  	1024*1024,  0x0F0000,  	0x0F0000}, //FLASH_SECTION_GAS_DATA
	
	{ FLASH_EX,    0x1FF000,   4096, 		  0x1FF000,  	0x1FF000}, //FLASH_SECTION_STATISTICS_DATA
	{ FLASH_EX,    0x1FE000,   4096, 		  0x1FE000,  	0x1FE000}, //FLASH_SECTION_PLCSETUP_DATA
	{ FLASH_EX,    0x1FD000,   4096, 		  0x1FD000,  	0x1FD000}, //FLASH_SECTION_TAXCODE_DATA

	
	{ FLASH_IN,	0x8000000, 	0x3000,    	0x8000000, 	0x8000000},//FLASH_SECTION_FW_BOOT
	
	{ FLASH_IN,	0x8003000, 	0x800,    	0x8003000, 	0x8003000},//FLASH_SECTION_PINGPONG_SEL
	{ FLASH_IN,  0x8003800, 	0x800,      0x8003800, 	0x8003800},//FLASH_SECTION_FLASH_MANAGER_VER
	
	{ FLASH_IN,	0x8004000, 	0x800,      0x8004000, 	0x8004000},//FLASH_SECTION_SYSTEM_INFO
	{ FLASH_IN,	0x8004800, 	0x800,      0x8004800, 	0x8004800},//FLASH_SECTION_CONFIG_PARA
	
	{ FLASH_IN,	0x8005000, 	0x1000,     0x8005000, 	0x8005000},//FLASH_SECTION_SPI_TEST
	{ FLASH_IN,	0x8006000, 	240*1024, 	0x8006000, 	0x8006000},//FLASH_SECTION_FW_PING
	{ FLASH_IN,	0x8042000, 	240*1024,  	0x8042000,  0x8042000}//FLASH_SECTION_FW_PONG
	
	
};

void uf_InitHard(FLASH_TYPE type);
uint32_t uf_ReadID(FLASH_TYPE type);

void uf_EraseChip(FLASH_TYPE type);
void uf_EraseSector(FLASH_TYPE type,uint32_t _uiSectorAddr);
void uf_EraseBlock(FLASH_TYPE type,uint8_t blocksize, uint32_t _uiSectorAddr);
void uf_PageWrite(FLASH_TYPE type,uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
uint8_t uf_WriteBuffer(FLASH_TYPE type,uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize);
void uf_ReadBuffer(FLASH_TYPE type,uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);





void uf_EraseChip(FLASH_TYPE type)
{
	if(type==FLASH_EX)
	  sf_EraseChip();


}

void uf_EraseSector(FLASH_TYPE type,uint32_t _uiSectorAddr)
{
	 if(type==FLASH_EX)
	  sf_EraseSector(_uiSectorAddr);
	 else if(type==FLASH_IN)
	 {
		 FLASH_Unlock();
		 FLASH_ErasePage(_uiSectorAddr);
		 FLASH_Lock();
	 }
		 	 
}


void uf_EraseBlock(FLASH_TYPE type,uint8_t blocksize, uint32_t _uiSectorAddr)
{
	 if(type==FLASH_EX)
		 sf_EraseBlock(blocksize, _uiSectorAddr);
	  
}


//void uf_PageWrite(FLASH_TYPE type,uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
//{
//	if(type==FLASH_EX)
//	  sf_AutoWritePage(_pBuf,_uiWriteAddr, _usSize);
//	else if(type==FLASH_IN)
//	  FLASH_WriteBank(_pBuf, _uiWriteAddr, _usSize);
//	
//	
//}


//成功返回1，失败返回0
uint8_t uf_WriteBuffer(FLASH_TYPE type,uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
	uint8_t res;
	
	if(type==FLASH_EX)
	   res=sf_WriteBuffer(_pBuf,  _uiWriteAddr,  _usWriteSize);
	else if(type==FLASH_IN)
	{
		 res=FLASH_ProgramStart(_uiWriteAddr,_usWriteSize);
		 if(res==FLASH_COMPLETE)		 
		    res=FLASH_WriteBank(_pBuf, _uiWriteAddr, _usWriteSize);
		 else
			 res=0;
		 
		  FLASH_ProgramDone();
	}
	
	return res;
	
}

void uf_ReadBuffer(FLASH_TYPE type,uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	 if(type==FLASH_EX)
	   sf_ReadBuffer(_pBuf,  _uiReadAddr,  _uiSize);
	 
	
}





