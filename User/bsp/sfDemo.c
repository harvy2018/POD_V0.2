

/*
	请使用串口工具（如超级终端）观察程序打印到串口的操作信息。通过PC机键盘输入操作指令。


检测到串行Flash,ID = 00BF2541

*******************************************
请选择操作命令:
【1 - 读串行Flash, 地址:0x0,长度:4096字节】
【2 - 写串行Flash, 地址:0x0,长度:4096字节】
【3 - 擦除整个串行Flash】
【4 - 写整个串行Flash, 全0x55】
【5 - 写整个串行Flash, 全0xAA】
【Z - 读取前1K，地址自动减少】
【X - 读取后1K，地址自动增加】
其他任意键 - 显示命令提示

*/


#include "stm32f10x.h"
#include <stdio.h>
#include "bsp_timer.h"
#include "bsp_SST25VF016B.h"
#include "bsp_usart.h"	
#include "SEGGER_RTT.h"

#define TEST_ADDR		0			/* 读写测试地址 */
#define TEST_SIZE		4096		/* 读写测试数据大小 */

/* 仅允许本文件内调用的函数声明 */
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
*	函 数 名: sf_Demo
*	功能说明: 串行EEPROM读写例程
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void sf_Demo(void)
{	
	char cmd;
	uint32_t id;
	uint32_t uiReadPageNo = 0;
	u32 Read_Add, Read_len;
	sf_InitHard();	/* 初始化SPI硬件 */
	
	id = sf_ReadID();
	if (id != SST25VF016B_ID)
	{
		/* 没有检测到芯片 */
		SEGGER_RTT_printf(0,"NO Flash!\r\n");
				
		while (1);	/* 停机 */
	}
	/* 检测串行Flash OK */
	SEGGER_RTT_printf(0,"Detect Flash,ID = %08X\r\n", id);
	
	sf_DispMenu();		/* 打印命令提示 */
	while(1)
	{
		cmd = SEGGER_RTT_WaitKey();	/* 从串口读入一个字符 */
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
				sf_ReadTest(Read_Add,Read_len);		/* 读串行Flash数据，并打印出来数据内容 */
				break;	
				
			case '2':
				SEGGER_RTT_printf(0,"[2- Write spiFlash, addr:0x%X,length:%dByte]\r\n", TEST_ADDR, TEST_SIZE);				
			//	sf_WriteTest();		/* 写串行Flash数据，并打印写入速度 */
				break;

			case '3':
				SEGGER_RTT_printf(0,"[3- Erase Flash]\r\n");				
			//	sf_Erase();			/* 擦除串行Flash数据，实际上就是写入全0xFF */
				break;

			case '4':
				SEGGER_RTT_printf(0,"[4- 写整个串行Flash, 全0x55]\r\n");				
			//	sf_WriteAll(0x55);			/* 擦除串行Flash数据，实际上就是写入全0xFF */
				break;

			case '5':
				SEGGER_RTT_printf(0,"[5- 写整个串行Flash, 全0xAA]\r\n");				
			//	sf_WriteAll(0xAA);			/* 擦除串行Flash数据，实际上就是写入全0xFF */
				break;
			
			case 'z':	
			case 'Z': /* 读取前1K */
				if (uiReadPageNo > 0)
				{
					uiReadPageNo--;
				}
				sf_ViewData(uiReadPageNo * 1024);
				break;

			case 'x':	
			case 'X': /* 读取后1K */
				if (uiReadPageNo < 2048 - 1)
				{
					uiReadPageNo++;
				}
				sf_ViewData(uiReadPageNo * 1024);
				break;

			default:
				sf_DispMenu();	/* 无效命令，重新打印命令提示 */
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
	sf_InitHard();	/* 初始化SPI硬件 */
	
	id = sf_ReadID();
	
	if (id != SST25VF016B_ID)
	{
		/* 没有检测到芯片 */
	//	SEGGER_RTT_printf(0,"没有检测到串行Flash!\r\n");
		Uart1SendData(0x11);  //头
		Uart1SendData(0x66);
		Uart1SendData(0x06);  //命令号
		Uart1SendData(0x01);  //功能号
		Uart1SendData(0x0);  //长度
		Uart1SendData(0x01);  //长度
		Uart1SendData(0x01);  //数据0x01 没有检测到串行Flash!
		Uart1SendData(0x22);  //尾	
    
    return;		
		//while (1);	/* 停机 */
	}
	/* 检测串行Flash OK */
//	SEGGER_RTT_printf(0,"检测到串行Flash,ID = %08X\r\n", id);
	
	/* 填充测试缓冲区 */
	for (i = 0; i < 4096; i++)
	{		
		buf[i] = i;
	}

	for (i = 0; i < SF_TOTAL_SIZE / SF_PAGE_SIZE; i++)
	{
		if (sf_WriteBuffer(buf, i * SF_PAGE_SIZE, SF_PAGE_SIZE) == 0)
		{
			//SEGGER_RTT_printf(0,"写串行Flash出错！\r\n");
			//return;
				Uart1SendData(0x11);  //头
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //命令号
				Uart1SendData(0x01);  //功能号
				Uart1SendData(0x0);  //长度
				Uart1SendData(0x01);  //长度
				Uart1SendData(0x02);  //数据0x02 写Flash出错!
				Uart1SendData(0x22);  //尾	
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
				Uart1SendData(0x11);  //头
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //命令号
				Uart1SendData(0x01);  //功能号
				Uart1SendData(0x0);  //长度
				Uart1SendData(0x01);  //长度
				Uart1SendData(0x04);  //数据0x03 读出数据与写入数据不符!
				Uart1SendData(0x22);  //尾	
				  return;		
				
			}
		}
	}
		
	Uart1SendData(0x11);  //头
	Uart1SendData(0x66);
	Uart1SendData(0x06);  //命令号
	Uart1SendData(0x01);  //功能号
	Uart1SendData(0x0);  //长度
	Uart1SendData(0x01);  //长度
	Uart1SendData(0x0);  //数据0x0 校验成功
	Uart1SendData(0x22);  //尾	
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
	

	
	sf_InitHard();	/* 初始化SPI硬件 */
	
	id = sf_ReadID();
	
	if (id != SST25VF016B_ID)
	{
		/* 没有检测到芯片 */
	//	SEGGER_RTT_printf(0,"没有检测到串行Flash!\r\n");
		Uart1SendData(0x11);  //头
		Uart1SendData(0x66);
		Uart1SendData(0x06);  //命令号
		Uart1SendData(0x01);  //功能号
		Uart1SendData(0x0);  //长度
		Uart1SendData(0x01);  //长度
		Uart1SendData(0x01);  //数据0x01 没有检测到串行Flash!
		Uart1SendData(0x22);  //尾	
    
    return;		
		//while (1);	/* 停机 */
	}
	/* 检测串行Flash OK */
//	SEGGER_RTT_printf(0,"检测到串行Flash,ID = %08X\r\n", id);
	


	if(FNo==1) //写入
	{
		
		flash_data=*(Buf+4);
		
	  /* 填充测试缓冲区 */
		for (i = 0; i < flash_data_len; i++)
		{		
			buf[i] = flash_data;
		}
		
		if (sf_WriteBuffer(buf, flash_sector * SF_PAGE_SIZE, flash_data_len) == 0)
		{
			//SEGGER_RTT_printf(0,"写串行Flash出错！\r\n");
			//return;
				Uart1SendData(0x11);  //头
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //命令号
				Uart1SendData(0x01);  //功能号
				Uart1SendData(0x0);  //长度
			  Uart1SendData(0x01);  //长度
				Uart1SendData(0x02);  //数据0x02 写Flash出错!
				Uart1SendData(0x22);  //尾	
			    return;		
		}
		else
		{
				Uart1SendData(0x11);  //头
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //命令号
				Uart1SendData(0x01);  //功能号
				Uart1SendData(0x0);  //长度
			  Uart1SendData(0x01);  //长度
				Uart1SendData(0x04);  //数据0x04 写入成功
				Uart1SendData(0x22);  //尾	
				  return;			
		}
	}
	
	if(FNo==2)
	{
		
		sf_ReadBuffer(buf2, flash_sector * SF_PAGE_SIZE, flash_data_len); 
		data_len_H=(flash_data_len>>8)&0xff;
		data_len_L=flash_data_len&0xff;
		
				Uart1SendData(0x11);  //头
				Uart1SendData(0x66);
				Uart1SendData(0x06);  //命令号
				Uart1SendData(0x01);  //功能号
				Uart1SendData(data_len_H);  //长度
				Uart1SendData(data_len_L);  //长度
				Uart1SendData(0x05);  //数据0x05 后续上传BUF内容
				Uart1SendBuf(buf2,flash_data_len);
				Uart1SendData(0x22);  //尾	
				 
		return;						
	
	}
			
	
}
/*
*********************************************************************************************************
*	函 数 名: sf_ReadTest
*	功能说明: 读串行Flash测试
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sf_ReadTest(uint32_t _uiReadAddr, uint32_t _uiSize)
{
	uint32_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];
	
	/* 起始地址 = 0， 数据长度为 256 */
	iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	sf_ReadBuffer(buf, _uiReadAddr, _uiSize);
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */
	SEGGER_RTT_printf(0," READ Flash OK,Data:\r\n");
	
	/* 打印数据 */
	for (i = 0; i < _uiSize; i++)
	{
		SEGGER_RTT_printf(0," %02X", buf[i]);
	//	SEGGER_RTT_printf(0," %c", buf[i]);
		if ((i % 16) == 0 && i!=0)
		{
			SEGGER_RTT_printf(0,"\r\n");	/* 每行显示8字节数据 */
			bsp_DelayMS(2);
		}		
	}

	/* 打印读速度 */
	SEGGER_RTT_printf(0,"length: %dByte, elapse: %dms, speed: %dB/s\r\n", _uiSize, iTime2 - iTime1, (_uiSize * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: sf_WriteTest
*	功能说明: 写串行Flash测试
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sf_WriteTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];
	
	/* 填充测试缓冲区 */
	for (i = 0; i < TEST_SIZE; i++)
	{		
		buf[i] = i;
	}
	
	/* 写EEPROM, 起始地址 = 0，数据长度为 256 */
	iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	if (sf_WriteBuffer(buf, TEST_ADDR, TEST_SIZE) == 0)
	{
		SEGGER_RTT_printf(0,"写串行Flash出错！\r\n");
		return;
	}
	else
	{
		iTime2 = bsp_GetRunTime();	/* 记下结束时间 */
		SEGGER_RTT_printf(0,"写串行Flash成功！\r\n");
	}
	

	/* 打印读速度 */
	SEGGER_RTT_printf(0,"数据长度: %d字节, 写耗时: %dms, 写速度: %dB/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: sf_WriteAll
*	功能说明: 写串行EEPROM全部数据
*	形    参：_ch : 写入的数据
*	返 回 值: 无
*********************************************************************************************************
*/
static void sf_WriteAll(uint8_t _ch)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[SF_PAGE_SIZE];
	
	/* 填充测试缓冲区 */
	for (i = 0; i < TEST_SIZE; i++)
	{		
		buf[i] = _ch;
	}
	
	/* 写EEPROM, 起始地址 = 0，数据长度为 256 */
	iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	for (i = 0; i < SF_TOTAL_SIZE / SF_PAGE_SIZE; i++)
	{
		if (sf_WriteBuffer(buf, i * SF_PAGE_SIZE, SF_PAGE_SIZE) == 0)
		{
			SEGGER_RTT_printf(0,"写串行Flash出错！\r\n");
			return;
		}
	}
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */	

	/* 打印读速度 */
	SEGGER_RTT_printf(0,"数据长度: %dK字节, 写耗时: %dms, 写速度: %dB/s\r\n", SF_TOTAL_SIZE / 1024, iTime2 - iTime1, (SF_TOTAL_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	函 数 名: sf_ReadTest
*	功能说明: 读串行EEPROM全部数据，并打印出来
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sf_Erase(void)
{
	int32_t iTime1, iTime2;

		iTime1 = bsp_GetRunTime();	/* 记下开始时间 */
	  sf_EraseChip();
  	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */	

	/* 打印读速度 */
 	SEGGER_RTT_printf(0,"擦除串行Flash完成！, 耗时: %dms\r\n", iTime2 - iTime1);
	return;
}


/*
*********************************************************************************************************
*	函 数 名: sf_ViewData
*	功能说明: 读串行Flash并显示，每次显示1K的内容
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sf_ViewData(uint32_t _uiAddr)
{
	uint16_t i;
	uint8_t buf[1024];

	sf_ReadBuffer(buf, _uiAddr,  1024);		/* 读数据 */
	SEGGER_RTT_printf(0,"地址：0x%08X; 数据长度 = 1024\r\n", _uiAddr);
	
	/* 打印数据 */
	for (i = 0; i < 1024; i++)
	{
		SEGGER_RTT_printf(0," %02X", buf[i]);
		
		if ((i & 15) == 15)
		{
			SEGGER_RTT_printf(0,"\r\n");	/* 每行显示8字节数据 */
		}		
	}
}

/*
*********************************************************************************************************
*	函 数 名: sf_DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sf_DispMenu(void)
{
	SEGGER_RTT_printf(0,"\r\n*******************************************\r\n");
	SEGGER_RTT_printf(0,"Input command:\r\n");
	SEGGER_RTT_printf(0,"[1- Read spiFlash, addr:?,length:?Byte]\r\n");
	SEGGER_RTT_printf(0,"[2- Write spiFlash,addr:0x%X,length:%dByte]\r\n", TEST_ADDR, TEST_SIZE);				
	SEGGER_RTT_printf(0,"[3- Erase Flash]\r\n");				
//	SEGGER_RTT_printf(0,"【4 - 写整个串行Flash, 全0x55】\r\n");				
//	SEGGER_RTT_printf(0,"【5 - 写整个串行Flash, 全0xAA】\r\n");				
//	SEGGER_RTT_printf(0,"【Z - 读取前1K，地址自动减少】\r\n");
//	SEGGER_RTT_printf(0,"【X - 读取后1K，地址自动增加】\r\n");
	SEGGER_RTT_printf(0,"Any key - command note!\r\n");
}

