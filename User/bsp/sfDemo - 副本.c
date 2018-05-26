

/*
	��ʹ�ô��ڹ��ߣ��糬���նˣ��۲�����ӡ�����ڵĲ�����Ϣ��ͨ��PC�������������ָ�


��⵽����Flash,ID = 00BF2541

*******************************************
��ѡ���������:
��1 - ������Flash, ��ַ:0x0,����:4096�ֽڡ�
��2 - д����Flash, ��ַ:0x0,����:4096�ֽڡ�
��3 - ������������Flash��
��4 - д��������Flash, ȫ0x55��
��5 - д��������Flash, ȫ0xAA��
��Z - ��ȡǰ1K����ַ�Զ����١�
��X - ��ȡ��1K����ַ�Զ����ӡ�
��������� - ��ʾ������ʾ

*/


#include "stm32f10x.h"
#include <stdio.h>
#include "bsp_timer.h"
#include "bsp_SST25VF016B.h"
#include "bsp_usart.h"	

#define TEST_ADDR		0			/* ��д���Ե�ַ */
#define TEST_SIZE		4096		/* ��д�������ݴ�С */

/* �������ļ��ڵ��õĺ������� */
static void sf_DispMenu(void);
static void sf_ReadTest(uint32_t _uiReadAddr, uint32_t _uiSize);
static void sf_WriteTest(void);
static void sf_Erase(void);
static void sf_ViewData(uint32_t _uiAddr);
static void sf_WriteAll(uint8_t _ch);
uint8_t buf[SF_PAGE_SIZE];
uint8_t buf2[SF_PAGE_SIZE];

/*
*********************************************************************************************************
*	�� �� ��: sf_Demo
*	����˵��: ����EEPROM��д����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void sf_Demo(void)
{	
	char cmd;
	uint32_t id;
	uint32_t uiReadPageNo = 0;
	u32 Read_Add, Read_len;
	sf_InitHard();	/* ��ʼ��SPIӲ�� */
	
	id = sf_ReadID();
	if (id != SST25VF016B_ID)
	{
		/* û�м�⵽оƬ */
		printf("û�м�⵽����Flash!\r\n");
				
		while (1);	/* ͣ�� */
	}
	/* ��⴮��Flash OK */
	printf("��⵽����Flash,ID = %08X\r\n", id);
	
	sf_DispMenu();		/* ��ӡ������ʾ */
	while(1)
	{
		cmd = getchar();	/* �Ӵ��ڶ���һ���ַ� */
		switch (cmd)
		{
			case '1':
				printf("��1 - ������Flash, �����ַ�Ͷ�ȡ���ݳ��ȣ�\r\n");
			  scanf("%x",&Read_Add);
			  printf("����ĵ�ַ:%x\r\n",Read_Add);
			  scanf("%x",&Read_len);
			  printf("����ĳ���:%x\r\n",Read_len);
			
				sf_ReadTest(Read_Add,Read_len);		/* ������Flash���ݣ�����ӡ������������ */
				break;	
				
			case '2':
				printf("��2 - д����Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);				
				sf_WriteTest();		/* д����Flash���ݣ�����ӡд���ٶ� */
				break;

			case '3':
				printf("��3 - ������������Flash��\r\n");				
				sf_Erase();			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
				break;

			case '4':
				printf("��4 - д��������Flash, ȫ0x55��\r\n");				
				sf_WriteAll(0x55);			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
				break;

			case '5':
				printf("��5 - д��������Flash, ȫ0xAA��\r\n");				
				sf_WriteAll(0xAA);			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
				break;
			
			case 'z':	
			case 'Z': /* ��ȡǰ1K */
				if (uiReadPageNo > 0)
				{
					uiReadPageNo--;
				}
				sf_ViewData(uiReadPageNo * 1024);
				break;

			case 'x':	
			case 'X': /* ��ȡ��1K */
				if (uiReadPageNo < 2048 - 1)
				{
					uiReadPageNo++;
				}
				sf_ViewData(uiReadPageNo * 1024);
				break;

			default:
				sf_DispMenu();	/* ��Ч������´�ӡ������ʾ */
				break;				
				
		}
	}
}

void FLASH_Test(void)
{	
	char cmd;
	uint32_t id;
	uint32_t uiReadPageNo = 0;
	uint16_t i,j;
	int32_t iTime1, iTime2;
//	uint8_t buf[SF_PAGE_SIZE];
//	uint8_t buf2[SF_PAGE_SIZE];
	sf_InitHard();	/* ��ʼ��SPIӲ�� */
	
	id = sf_ReadID();
	
	if (id != SST25VF016B_ID)
	{
		/* û�м�⵽оƬ */
	//	printf("û�м�⵽����Flash!\r\n");
		Uart1SendData(0x11);  //ͷ
		Uart1SendData(0x66);
		Uart1SendData(0x06);  //�����
		Uart1SendData(0x01);  //���ܺ�
		Uart1SendData(0x0);  //����
		Uart1SendData(0x01);  //����
		Uart1SendData(0x01);  //����0x01 û�м�⵽����Flash!
		Uart1SendData(0x22);  //β	
    
    return;		
		//while (1);	/* ͣ�� */
	}
	/* ��⴮��Flash OK */
//	printf("��⵽����Flash,ID = %08X\r\n", id);
	
	/* �����Ի����� */
	for (i = 0; i < 4096; i++)
	{		
		buf[i] = i;
	}

	for (i = 0; i < SF_TOTAL_SIZE / SF_PAGE_SIZE; i++)
	{
		if (sf_WriteBuffer(buf, i * SF_PAGE_SIZE, SF_PAGE_SIZE) == 0)
		{
			//printf("д����Flash����\r\n");
			//return;
				Uart1SendData(0x11);  //ͷ
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //�����
				Uart1SendData(0x01);  //���ܺ�
				Uart1SendData(0x0);  //����
				Uart1SendData(0x01);  //����
				Uart1SendData(0x02);  //����0x02 дFlash����!
				Uart1SendData(0x22);  //β	
			    return;		
		}
	}
	
	for (i = 0; i < SF_TOTAL_SIZE / SF_PAGE_SIZE; i++)
	{
		
		sf_ReadBuffer(buf2, i * SF_PAGE_SIZE, SF_PAGE_SIZE); 
		
		for (j = 0; j<SF_PAGE_SIZE; j++)
		{
			if(buf2[j]!=(u8)j)
			{
				Uart1SendData(0x11);  //ͷ
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //�����
				Uart1SendData(0x01);  //���ܺ�
				Uart1SendData(0x0);  //����
				Uart1SendData(0x01);  //����
				Uart1SendData(0x04);  //����0x03 ����������д�����ݲ���!
				Uart1SendData(0x22);  //β	
				  return;		
				
			}
		}
	}
		
	Uart1SendData(0x11);  //ͷ
	Uart1SendData(0x66);
	Uart1SendData(0x06);  //�����
	Uart1SendData(0x01);  //���ܺ�
	Uart1SendData(0x0);  //����
	Uart1SendData(0x01);  //����
	Uart1SendData(0x0);  //����0x0 У��ɹ�
	Uart1SendData(0x22);  //β	
	 return;		
	
	
}

void FLASH_WD_TEST(u8 FNo,u8 len,u8 *Buf)
{	
	char cmd;
	uint32_t id;
	uint32_t uiReadPageNo = 0;
	uint16_t i,j,flash_data_len,flash_sector;
	int32_t iTime1, iTime2;
//	uint8_t buf[SF_PAGE_SIZE];
//	uint8_t buf2[SF_PAGE_SIZE];
	u8 flash_data;
	u8 data_len_H,data_len_L;
	
	flash_data_len=*Buf;
	flash_data_len<<=8;
	flash_data_len|=*(Buf+1);
	
	flash_sector=*(Buf+2);
	flash_sector<<=8;
	flash_sector|=*(Buf+3);
	

	
	sf_InitHard();	/* ��ʼ��SPIӲ�� */
	
	id = sf_ReadID();
	
	if (id != SST25VF016B_ID)
	{
		/* û�м�⵽оƬ */
	//	printf("û�м�⵽����Flash!\r\n");
		Uart1SendData(0x11);  //ͷ
		Uart1SendData(0x66);
		Uart1SendData(0x06);  //�����
		Uart1SendData(0x01);  //���ܺ�
		Uart1SendData(0x0);  //����
		Uart1SendData(0x01);  //����
		Uart1SendData(0x01);  //����0x01 û�м�⵽����Flash!
		Uart1SendData(0x22);  //β	
    
    return;		
		//while (1);	/* ͣ�� */
	}
	/* ��⴮��Flash OK */
//	printf("��⵽����Flash,ID = %08X\r\n", id);
	


	if(FNo==1) //д��
	{
		
		flash_data=*(Buf+4);
		
	  /* �����Ի����� */
		for (i = 0; i < flash_data_len; i++)
		{		
			buf[i] = flash_data;
		}
		
		if (sf_WriteBuffer(buf, flash_sector * SF_PAGE_SIZE, flash_data_len) == 0)
		{
			//printf("д����Flash����\r\n");
			//return;
				Uart1SendData(0x11);  //ͷ
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //�����
				Uart1SendData(0x01);  //���ܺ�
				Uart1SendData(0x0);  //����
			  Uart1SendData(0x01);  //����
				Uart1SendData(0x02);  //����0x02 дFlash����!
				Uart1SendData(0x22);  //β	
			    return;		
		}
		else
		{
				Uart1SendData(0x11);  //ͷ
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //�����
				Uart1SendData(0x01);  //���ܺ�
				Uart1SendData(0x0);  //����
			  Uart1SendData(0x01);  //����
				Uart1SendData(0x04);  //����0x04 д��ɹ�
				Uart1SendData(0x22);  //β	
				  return;			
		}
	}
	
	if(FNo==2)
	{
		
		sf_ReadBuffer(buf2, flash_sector * SF_PAGE_SIZE, flash_data_len); 
		data_len_H=(flash_data_len>>8)&0xff;
		data_len_L=flash_data_len&0xff;
		
				Uart1SendData(0x11);  //ͷ
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //�����
				Uart1SendData(0x01);  //���ܺ�
				Uart1SendData(data_len_H);  //����
				Uart1SendData(data_len_L);  //����
				Uart1SendData(0x05);  //����0x05 �����ϴ�BUF����
				Uart1SendBuf(buf2,flash_data_len);
				Uart1SendData(0x22);  //β	
				 
		return;						
	
	}
		
	
	
	
}
/*
*********************************************************************************************************
*	�� �� ��: sf_ReadTest
*	����˵��: ������Flash����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sf_ReadTest(uint32_t _uiReadAddr, uint32_t _uiSize)
{
	uint32_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];
	
	/* ��ʼ��ַ = 0�� ���ݳ���Ϊ 256 */
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	sf_ReadBuffer(buf, _uiReadAddr, _uiSize);
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */
	printf("������Flash�ɹ����������£�\r\n");
	
	/* ��ӡ���� */
	for (i = 0; i < _uiSize; i++)
	{
		printf(" %02X", buf[i]);
		
		if ((i & 15) == 15)
		{
			printf("\r\n");	/* ÿ����ʾ8�ֽ����� */
		}		
	}

	/* ��ӡ���ٶ� */
	printf("���ݳ���: %d�ֽ�, ����ʱ: %dms, ���ٶ�: %dB/s\r\n", _uiSize, iTime2 - iTime1, (_uiSize * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sf_WriteTest
*	����˵��: д����Flash����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sf_WriteTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];
	
	/* �����Ի����� */
	for (i = 0; i < TEST_SIZE; i++)
	{		
		buf[i] = i;
	}
	
	/* дEEPROM, ��ʼ��ַ = 0�����ݳ���Ϊ 256 */
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	if (sf_WriteBuffer(buf, TEST_ADDR, TEST_SIZE) == 0)
	{
		printf("д����Flash����\r\n");
		return;
	}
	else
	{
		iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */
		printf("д����Flash�ɹ���\r\n");
	}
	

	/* ��ӡ���ٶ� */
	printf("���ݳ���: %d�ֽ�, д��ʱ: %dms, д�ٶ�: %dB/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sf_WriteAll
*	����˵��: д����EEPROMȫ������
*	��    �Σ�_ch : д�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sf_WriteAll(uint8_t _ch)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[SF_PAGE_SIZE];
	
	/* �����Ի����� */
	for (i = 0; i < TEST_SIZE; i++)
	{		
		buf[i] = _ch;
	}
	
	/* дEEPROM, ��ʼ��ַ = 0�����ݳ���Ϊ 256 */
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	for (i = 0; i < SF_TOTAL_SIZE / SF_PAGE_SIZE; i++)
	{
		if (sf_WriteBuffer(buf, i * SF_PAGE_SIZE, SF_PAGE_SIZE) == 0)
		{
			printf("д����Flash����\r\n");
			return;
		}
	}
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */	

	/* ��ӡ���ٶ� */
	printf("���ݳ���: %dK�ֽ�, д��ʱ: %dms, д�ٶ�: %dB/s\r\n", SF_TOTAL_SIZE / 1024, iTime2 - iTime1, (SF_TOTAL_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sf_ReadTest
*	����˵��: ������EEPROMȫ�����ݣ�����ӡ����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sf_Erase(void)
{
	int32_t iTime1, iTime2;

		iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	  sf_EraseChip();
  	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */	

	/* ��ӡ���ٶ� */
 	printf("��������Flash��ɣ�, ��ʱ: %dms\r\n", iTime2 - iTime1);
	return;
}


/*
*********************************************************************************************************
*	�� �� ��: sf_ViewData
*	����˵��: ������Flash����ʾ��ÿ����ʾ1K������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sf_ViewData(uint32_t _uiAddr)
{
	uint16_t i;
	uint8_t buf[1024];

	sf_ReadBuffer(buf, _uiAddr,  1024);		/* ������ */
	printf("��ַ��0x%08X; ���ݳ��� = 1024\r\n", _uiAddr);
	
	/* ��ӡ���� */
	for (i = 0; i < 1024; i++)
	{
		printf(" %02X", buf[i]);
		
		if ((i & 15) == 15)
		{
			printf("\r\n");	/* ÿ����ʾ8�ֽ����� */
		}		
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sf_DispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sf_DispMenu(void)
{
	printf("\r\n*******************************************\r\n");
	printf("��ѡ���������:\r\n");
	printf("��1 - ������Flash, ��ַ:?,����:?�ֽڡ�\r\n");
	printf("��2 - д����Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);				
	printf("��3 - ������������Flash��\r\n");				
	printf("��4 - д��������Flash, ȫ0x55��\r\n");				
	printf("��5 - д��������Flash, ȫ0xAA��\r\n");				
	printf("��Z - ��ȡǰ1K����ַ�Զ����١�\r\n");
	printf("��X - ��ȡ��1K����ַ�Զ����ӡ�\r\n");
	printf("��������� - ��ʾ������ʾ\r\n");
}

