

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
#include "SEGGER_RTT.h"

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

u8 RTT_BUF[20];

u8 Num_Read;
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
		SEGGER_RTT_printf(0,"NO Flash!\r\n");
				
		while (1);	/* ͣ�� */
	}
	/* ��⴮��Flash OK */
	SEGGER_RTT_printf(0,"Detect Flash,ID = %08X\r\n", id);
	
	sf_DispMenu();		/* ��ӡ������ʾ */
	while(1)
	{
		cmd = SEGGER_RTT_WaitKey();	/* �Ӵ��ڶ���һ���ַ� */
		switch (cmd)
		{
			case '1':
				SEGGER_RTT_printf(0,"[1- Read spiFlash, input addr and data lenght to read:\r\n");
			//  scanf("%x",&Read_Add);
		
//			  cmd=SEGGER_RTT_WaitKey()-0x30;
//			  Read_Add=cmd<<12;
			  bsp_DelayMS(5000);
			 // ClearDog;	
			  if(SEGGER_RTT_HasData(0))
					Num_Read=SEGGER_RTT_Read(0,RTT_BUF,20);
			
				Read_Add=atoi(RTT_BUF);  
				
			  SEGGER_RTT_printf(0,"input addr:%x,Num_Read:%d\r\n",Read_Add,Num_Read);
			
//			  cmd=SEGGER_RTT_WaitKey()-0x30;
//			  Read_len=cmd<<9;
				memset(RTT_BUF,0,20);
				 bsp_DelayMS(5000);
				 if(SEGGER_RTT_HasData(0))
					Num_Read=SEGGER_RTT_Read(0,RTT_BUF,20);
			
				 Read_len=atoi(RTT_BUF);  
			
			//  scanf("%x",&Read_len);
			  SEGGER_RTT_printf(0,"input length:%d,Num_Read:%d\r\n",Read_len,Num_Read);
				 
				memset(RTT_BUF,0,20);
				sf_ReadTest(Read_Add,Read_len);		/* ������Flash���ݣ�����ӡ������������ */
				break;	
				
			case '2':
				SEGGER_RTT_printf(0,"[2- Write spiFlash, addr:0x%X,length:%dByte]\r\n", TEST_ADDR, TEST_SIZE);				
			//	sf_WriteTest();		/* д����Flash���ݣ�����ӡд���ٶ� */
				break;

			case '3':
				SEGGER_RTT_printf(0,"[3- Erase Flash]\r\n");				
			//	sf_Erase();			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
				break;

			case '4':
				SEGGER_RTT_printf(0,"[4- д��������Flash, ȫ0x55]\r\n");				
			//	sf_WriteAll(0x55);			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
				break;

			case '5':
				SEGGER_RTT_printf(0,"[5- д��������Flash, ȫ0xAA]\r\n");				
			//	sf_WriteAll(0xAA);			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
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
	//	SEGGER_RTT_printf(0,"û�м�⵽����Flash!\r\n");
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
//	SEGGER_RTT_printf(0,"��⵽����Flash,ID = %08X\r\n", id);
	
	/* �����Ի����� */
	for (i = 0; i < 4096; i++)
	{		
		buf[i] = i;
	}

	for (i = 0; i < SF_TOTAL_SIZE / SF_PAGE_SIZE; i++)
	{
		if (sf_WriteBuffer(buf, i * SF_PAGE_SIZE, SF_PAGE_SIZE) == 0)
		{
			//SEGGER_RTT_printf(0,"д����Flash����\r\n");
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
	//	SEGGER_RTT_printf(0,"û�м�⵽����Flash!\r\n");
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
//	SEGGER_RTT_printf(0,"��⵽����Flash,ID = %08X\r\n", id);
	


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
			//SEGGER_RTT_printf(0,"д����Flash����\r\n");
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
	SEGGER_RTT_printf(0," READ Flash OK,Data:\r\n");
	
	/* ��ӡ���� */
	for (i = 0; i < _uiSize; i++)
	{
		SEGGER_RTT_printf(0," %02X", buf[i]);
	//	SEGGER_RTT_printf(0," %c", buf[i]);
		if ((i % 16) == 0 && i!=0)
		{
			SEGGER_RTT_printf(0,"\r\n");	/* ÿ����ʾ8�ֽ����� */
			bsp_DelayMS(2);
		}		
	}

	/* ��ӡ���ٶ� */
	SEGGER_RTT_printf(0,"length: %dByte, elapse: %dms, speed: %dB/s\r\n", _uiSize, iTime2 - iTime1, (_uiSize * 1000) / (iTime2 - iTime1));
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
		SEGGER_RTT_printf(0,"д����Flash����\r\n");
		return;
	}
	else
	{
		iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */
		SEGGER_RTT_printf(0,"д����Flash�ɹ���\r\n");
	}
	

	/* ��ӡ���ٶ� */
	SEGGER_RTT_printf(0,"���ݳ���: %d�ֽ�, д��ʱ: %dms, д�ٶ�: %dB/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
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
			SEGGER_RTT_printf(0,"д����Flash����\r\n");
			return;
		}
	}
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */	

	/* ��ӡ���ٶ� */
	SEGGER_RTT_printf(0,"���ݳ���: %dK�ֽ�, д��ʱ: %dms, д�ٶ�: %dB/s\r\n", SF_TOTAL_SIZE / 1024, iTime2 - iTime1, (SF_TOTAL_SIZE * 1000) / (iTime2 - iTime1));
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
 	SEGGER_RTT_printf(0,"��������Flash��ɣ�, ��ʱ: %dms\r\n", iTime2 - iTime1);
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
	SEGGER_RTT_printf(0,"��ַ��0x%08X; ���ݳ��� = 1024\r\n", _uiAddr);
	
	/* ��ӡ���� */
	for (i = 0; i < 1024; i++)
	{
		SEGGER_RTT_printf(0," %02X", buf[i]);
		
		if ((i & 15) == 15)
		{
			SEGGER_RTT_printf(0,"\r\n");	/* ÿ����ʾ8�ֽ����� */
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
	SEGGER_RTT_printf(0,"\r\n*******************************************\r\n");
	SEGGER_RTT_printf(0,"Input command:\r\n");
	SEGGER_RTT_printf(0,"[1- Read spiFlash, addr:?,length:?Byte]\r\n");
	SEGGER_RTT_printf(0,"[2- Write spiFlash,addr:0x%X,length:%dByte]\r\n", TEST_ADDR, TEST_SIZE);				
	SEGGER_RTT_printf(0,"[3- Erase Flash]\r\n");				
//	SEGGER_RTT_printf(0,"��4 - д��������Flash, ȫ0x55��\r\n");				
//	SEGGER_RTT_printf(0,"��5 - д��������Flash, ȫ0xAA��\r\n");				
//	SEGGER_RTT_printf(0,"��Z - ��ȡǰ1K����ַ�Զ����١�\r\n");
//	SEGGER_RTT_printf(0,"��X - ��ȡ��1K����ַ�Զ����ӡ�\r\n");
	SEGGER_RTT_printf(0,"Any key - command note!\r\n");
}

