/*
*********************************************************************************************************
*
*	ģ������ : ����FLASH  SST25VF016B����ģ��
*	�ļ����� :  bsp_SST25VF016B.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2011-08-21 armfly  ST�̼���������V3.5.0�汾��
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_SST25VF016B_H
#define _BSP_SST25VF016B_H

#include <inttypes.h>

/* ���崮��Flash ID */
enum
{
	SST25VF016B_ID = 0xBF2541,
	MX25L1606E_ID = 0xC22015
};

#define SF_UNKNOW_NAME	"Unknow"

typedef struct
{
	uint32_t ChipID;		/* оƬID */
	char ChipName[16];		/* оƬ�ͺ��ַ�������Ҫ������ʾ */
	uint32_t TotalSize;		/* ������ */
	uint16_t PageSize;		/* ҳ���С */
}SFLASH_T;

#define SF_TOTAL_SIZE		(2*1024*1024)	/* ��������2M�ֽ� */
#define SF_PAGE_SIZE    	(4*1024)		/* ҳ���С��4K�ֽ� */

void sf_InitHard(void);
uint32_t sf_ReadID(void);
void sf_EraseChip(void);
void sf_EraseSector(uint32_t _uiSectorAddr);
void sf_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);

uint8_t sf_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize);
void sf_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);

extern SFLASH_T g_tSF;

#endif
