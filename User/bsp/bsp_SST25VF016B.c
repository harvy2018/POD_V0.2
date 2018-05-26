/*
*********************************************************************************************************
*	                                  
*	模块名称 : 串行FLASH  SST25VF016B驱动模块
*	文件名称 : bsp_SST25VF016B.c
*	版    本 : V1.0
*	说    明 : 实现SST25VF016B的读写操作。写操作采用AAI指令提高写入效率。
*			   使用STM32的硬件SPI接口，非IO模拟时序
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-08-21 armfly  ST固件库升级到V3.5.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
	应用说明：CPU上电后必须先调用一次 sf_Init()函数
*/

#include "stm32f10x.h"
#include "string.h"
#include "bsp_SST25VF016B.h"

/* 
	定义驱动串行Flash的SPI和GPIO端口 :
		PA5 = SCK
		PA6 = MISO
		PA7 = MOSI
		PB2 = CS
*/
#define SPI_FLASH          SPI1
#define SF_CLK             RCC_APB2Periph_SPI1
#define SF_GPIO            GPIOA
#define SF_GPIO_CLK        RCC_APB2Periph_GPIOA
#define SF_PIN_SCK         GPIO_Pin_5
#define SF_PIN_MISO        GPIO_Pin_6
#define SF_PIN_MOSI        GPIO_Pin_7

#define SF_CS             GPIO_Pin_4
#define SF_CS_GPIO        GPIOA
#define SF_CS_GPIO_CLK    RCC_APB2Periph_GPIOA

/* 片选口线置低选中  */
#define SF_CS_LOW()       GPIO_ResetBits(SF_CS_GPIO, SF_CS)

/* 片选口线置高不选中 */
#define SF_CS_HIGH()      GPIO_SetBits(SF_CS_GPIO, SF_CS)

#define CMD_AAI       0xAD  	/* AAI 连续编程指令 */
#define CMD_DISWR	    0x04		/* 禁止写, 退出AAI状态 */
#define CMD_EWRSR	    0x50		/* 允许写状态寄存器的命令 */
#define CMD_WRSR      0x01  	/* 写状态寄存器命令 */
#define CMD_WREN      0x06		/* 写使能命令 */
#define CMD_READ      0x03  	/* 读数据区命令 */
#define CMD_RDSR      0x05		/* 读状态寄存器命令 */
#define CMD_RDID      0x9F		/* 读器件ID命令 */
#define CMD_SE        0x20		/* 擦除扇区命令 */
#define CMD_BE        0xC7		/* 批量擦除命令 */

#define CMD_BE        0xC7		/* 批量擦除命令 */

#define DUMMY_BYTE    0xA5		/* 哑命令，可以为任意值，用于读操作 */

#define WIP_FLAG      0x01		/* 状态寄存器中的正在编程标志（WIP) */

static uint8_t sf_SendByte(uint8_t _ucValue);
static void sf_WriteEnable(void);
static void sf_WriteStatus(uint8_t _ucValue);
static void sf_WaitForWriteEnd(void);
static uint8_t sf_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _uiLen);
static uint8_t sf_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize);
static uint8_t sf_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen);

static uint8_t s_spiBuf[SF_PAGE_SIZE];	/* 用于写函数，先读出整个page，修改缓冲区后，再整个page回写 */

/*
*********************************************************************************************************
*	函 数 名: sf_InitHard
*	功能说明: 初始化串行Flash硬件接口（配置STM32的SPI时钟、GPIO)
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void sf_InitHard(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 使能 SPI 和 GPIO 时钟 */
	RCC_APB2PeriphClockCmd(SF_CLK | SF_GPIO_CLK | SF_CS_GPIO_CLK, ENABLE);
	
	/* 配置SPI引脚SCK、MISO 和 MOSI为复用推挽模式 */
	GPIO_InitStructure.GPIO_Pin = SF_PIN_SCK | SF_PIN_MISO | SF_PIN_MOSI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SF_GPIO, &GPIO_InitStructure);
	
	/* 配置片选口线为推挽输出模式 */
	GPIO_InitStructure.GPIO_Pin = SF_CS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SF_CS_GPIO, &GPIO_InitStructure);
	
	/* 片选置高，不选中 */
	SF_CS_HIGH();
	
	/* 配置SPI硬件参数 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 数据位长度 ： 8位 */
	/* SPI_CPOL和SPI_CPHA结合使用决定时钟和数据采样点的相位关系、
	   本例配置: 总线空闲是高电平,第2个边沿（上升沿采样数据)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* 时钟上升沿采样数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* 时钟的第2个边沿采样数据 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* 片选控制方式：软件控制 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;	/* 波特率预分频系数：4分频 */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(SPI_FLASH, &SPI_InitStructure);
	
	SPI_Cmd(SPI_FLASH, ENABLE);		/* 使能SPI  */
	
	SF_CS_LOW();				/* 软件方式，使能串行Flash片选 */
	sf_SendByte(CMD_DISWR);		/* 发送禁止写入的命令,即使能软件写保护 */
	SF_CS_HIGH();				/* 软件方式，禁能串行Flash片选 */
	
	sf_WaitForWriteEnd();		/* 等待串行Flash内部操作完成 */
	
	sf_WriteStatus(0);			/* 解除所有BLOCK的写保护 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_EraseSector
*	功能说明: 擦除指定的扇区
*	形    参：_uiSectorAddr : 扇区地址
*	返 回 值: 无
*********************************************************************************************************
*/
void sf_EraseSector(uint32_t _uiSectorAddr)
{
	sf_WriteEnable();								/* 发送写使能命令 */
	
	/* 擦除扇区操作 */
	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_SE);							/* 发送擦除命令 */
	sf_SendByte((_uiSectorAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	sf_SendByte((_uiSectorAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	sf_SendByte(_uiSectorAddr & 0xFF);				/* 发送扇区地址低8bit */
	SF_CS_HIGH();									/* 禁能片选 */
	
	sf_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}

void sf_EraseBlock(uint8_t blocksize, uint32_t _uiSectorAddr)
{
	
	sf_WriteEnable();								/* 发送写使能命令 */
	
	/* 擦除扇区操作 */
	SF_CS_LOW();									/* 使能片选 */
	
	if(blocksize==32)
	  sf_SendByte(0x52);	/* 发送32K BLOCK擦除命令 */
  else if(blocksize==64)
		sf_SendByte(0xD8);	/* 发送64K BLOCK擦除命令 */
	else
		sf_SendByte(0xD8);	
		
	sf_SendByte((_uiSectorAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	sf_SendByte((_uiSectorAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	sf_SendByte(_uiSectorAddr & 0xFF);				/* 发送扇区地址低8bit */
	SF_CS_HIGH();									/* 禁能片选 */
	
	sf_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}
/*
*********************************************************************************************************
*	函 数 名: sf_EraseChip
*	功能说明: 擦除整个芯片
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/  
void sf_EraseChip(void)
{
	sf_WriteEnable();								/* 发送写使能命令 */
	
	/* 擦除扇区操作 */
	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_BE);							/* 发送擦除命令 */
	SF_CS_HIGH();									/* 禁能片选 */
	
	sf_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_PageWrite
*	功能说明: 向一个page内写入若干字节。字节个数不能超出页面大小（4K)
*	形    参：	_pBuf : 数据源缓冲区；
*				_uiWriteAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 无
*********************************************************************************************************
*/  
void sf_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i;
	
	/* AAI指令要求传入的数据个数是偶数 */
	if ((_usSize < 2) && (_usSize % 2))
	{
		return ;
	}

	sf_WriteEnable();								/* 发送写使能命令 */
	
	/* 擦除扇区操作 */
	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_AAI);							/* 发送AAI命令(地址自动增加编程) */
	sf_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	sf_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	sf_SendByte(_uiWriteAddr & 0xFF);				/* 发送扇区地址低8bit */	
	sf_SendByte(*_pBuf++);							/* 发送第1个数据 */
	sf_SendByte(*_pBuf++);							/* 发送第2个数据 */
	SF_CS_HIGH();									/* 禁能片选 */
	
	sf_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */

	_usSize -= 2;									/* 计算剩余字节数 */
	
	for (i = 0; i < _usSize / 2; i++)
	{
		SF_CS_LOW();								/* 使能片选 */
		sf_SendByte(CMD_AAI);						/* 发送AAI命令(地址自动增加编程) */
		sf_SendByte(*_pBuf++);					/* 发送数据 */
		sf_SendByte(*_pBuf++);					/* 发送数据 */
		SF_CS_HIGH();								/* 禁能片选 */
		sf_WaitForWriteEnd();						/* 等待串行Flash内部写操作完成 */
	}
	
	/* 进入写保护状态 */
	SF_CS_LOW();
	sf_SendByte(CMD_DISWR);							
	SF_CS_HIGH();
	
	sf_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_ReadBuffer
*	功能说明: 连续读取若干字节。字节个数不能超出芯片容量。
*	形    参：	_pBuf : 数据源缓冲区；
*				_uiReadAddr ：首地址
*				_usSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 无
*********************************************************************************************************
*/
void sf_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > SF_TOTAL_SIZE)
	{
		return;
	}
		
	/* 擦除扇区操作 */
	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_READ);							/* 发送读命令 */
	sf_SendByte((_uiReadAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	sf_SendByte((_uiReadAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	sf_SendByte(_uiReadAddr & 0xFF);				/* 发送扇区地址低8bit */	
	while (_uiSize--)
	{
		*_pBuf++ = sf_SendByte(DUMMY_BYTE);			/* 读一个字节并存储到pBuf，读完后指针自加1 */
	}
	SF_CS_HIGH();									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_CmpData
*	功能说明: 比较Flash的数据.
*	形    参：	_ucpTar : 数据缓冲区
*				_uiSrcAddr ：Flash地址
*				_uiSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 0 = 相等, 1 = 不等
*********************************************************************************************************
*/
static uint8_t sf_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)
{
	uint8_t ucValue;

	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((_uiSrcAddr + _uiSize) > SF_TOTAL_SIZE)
	{
		return 1;
	}

	if (_uiSize == 0) 	
	{
		return 0;
	}
		

	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_READ);							/* 发送读命令 */
	sf_SendByte((_uiSrcAddr & 0xFF0000) >> 16);		/* 发送扇区地址的高8bit */
	sf_SendByte((_uiSrcAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	sf_SendByte(_uiSrcAddr & 0xFF);					/* 发送扇区地址低8bit */	
	while (_uiSize--)
	{
		/* 读一个字节 */
		ucValue = sf_SendByte(DUMMY_BYTE);		
		if (*_ucpTar++ != ucValue)
		{
			SF_CS_HIGH();
			return 1;
		}
	}
	SF_CS_HIGH();	
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: sf_NeedErase
*	功能说明: 判断写PAGE前是否需要先擦除。
*	形    参： _ucpOldBuf ： 旧数据
*			   _ucpNewBuf ： 新数据
*			   _uiLen ：数据个数，不能超过页面大小
*	返 回 值: 0 : 不需要擦除， 1 ：需要擦除
*********************************************************************************************************
*/ 
static uint8_t sf_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)
{
	uint16_t i;
	uint8_t ucOld;

	/*
	算法第1步：old 求反, new 不变
	      old    new
		  1101   0101
	~     1111
		= 0010   0101

	算法第2步: old 求反的结果与 new 位与
		  0010   old
	&	  0101   new
		 =0000

	算法第3步: 结果为0,则表示无需擦除. 否则表示需要擦除
	*/

	for (i = 0; i < _usLen; i++)
	{
		ucOld = *_ucpOldBuf++;
		ucOld = ~ucOld;

		/* 注意错误的写法: if (ucOld & (*_ucpNewBuf++) != 0) */
		if ((ucOld & (*_ucpNewBuf++)) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: sf_AutoWritePage
*	功能说明: 写1个PAGE并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*	形    参：	_pBuf : 数据源缓冲区；
*				_uiWriteAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 0 : 错误， 1 ： 成功
*********************************************************************************************************
*/ 
static uint8_t sf_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
	uint16_t i;
	uint16_t j;					/* 用于延时 */
	uint32_t uiFirstAddr;		/* 扇区首址 */
	uint8_t ucNeedErase;		/* 1表示需要擦除 */
	//uint8_t ucaFlashBuf[SF_PAGE_SIZE];
	uint8_t cRet;

	/* 长度为0时不继续操作,直接认为成功 */
	if (_usWrLen == 0)
	{
		return 1;
	}	
	
	/* 如果偏移地址超过芯片容量则退出 */
	if (_uiWrAddr >= SF_TOTAL_SIZE)
	{
		return 0;
	}

	/* 如果数据长度大于扇区容量，则退出 */
	if (_usWrLen > SF_PAGE_SIZE)
	{
		return 0;
	}

	/* 如果FLASH中的数据没有变化,则不写FLASH */
	sf_ReadBuffer(s_spiBuf, _uiWrAddr, _usWrLen);
	if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
	{
		return 1;
	}

	/* 判断是否需要先擦除扇区 */
	/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
	ucNeedErase = 0;
	if (sf_NeedErase(s_spiBuf, _ucpSrc, _usWrLen))
	{
		ucNeedErase = 1;
	}

	uiFirstAddr = _uiWrAddr & (~(SF_PAGE_SIZE - 1));

	if (_usWrLen == SF_PAGE_SIZE)		/* 整个扇区都改写 */
	{
		for	(i = 0; i < SF_PAGE_SIZE; i++)
		{
			s_spiBuf[i] = _ucpSrc[i];
		}
	}
	else						/* 改写部分数据 */
	{
		/* 先将整个扇区的数据读出 */
		sf_ReadBuffer(s_spiBuf, uiFirstAddr, SF_PAGE_SIZE);

		/* 再用新数据覆盖 */
		i = _uiWrAddr & (SF_PAGE_SIZE - 1);
		memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
	}

	/* 写完之后进行校验，如果不正确则重写，最多3次 */
	cRet = 0;
	for (i = 0; i < 3; i++)
	{

		/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
		if (ucNeedErase == 1)
		{			
			sf_EraseSector(uiFirstAddr);		/* 擦除1个扇区 */
		}

		/* 编程一个PAGE */
		sf_PageWrite(s_spiBuf, uiFirstAddr, SF_PAGE_SIZE);
	
		if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
		{
			cRet = 1;
			break;
		}
		else
		{
			if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
			{
				cRet = 1;
				break;
			}

			/* 失败后延迟一段时间再重试 */
			for (j = 0; j < 10000; j++);
		}
	}

	return cRet;
}

/*
*********************************************************************************************************
*	函 数 名: sf_WriteBuffer
*	功能说明: 写1个扇区并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*	形    参：	_pBuf : 数据源缓冲区；
*				_uiWrAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 1 : 成功， 0 ： 失败
*********************************************************************************************************
*/ 
uint8_t sf_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
	uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
	
	Addr = _uiWriteAddr % SF_PAGE_SIZE;
	count = SF_PAGE_SIZE - Addr;
	NumOfPage =  _usWriteSize / SF_PAGE_SIZE;
	NumOfSingle = _usWriteSize % SF_PAGE_SIZE;
	
	if (Addr == 0) /* 起始地址是页面首地址  */
	{
		if (NumOfPage == 0) /* 数据长度小于页面大小 */
		{
			if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
			{
				return 0;
			}
		}
		else 	/* 数据长度大于等于页面大小 */
		{
			while (NumOfPage--)
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, SF_PAGE_SIZE) == 0)
				{
					return 0;
				}				
				_uiWriteAddr +=  SF_PAGE_SIZE;
				_pBuf += SF_PAGE_SIZE;
			}
			if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
			{
				return 0;
			}				
		}
	}
	else  /* 起始地址不是页面首地址  */
	{
		if (NumOfPage == 0) /* 数据长度小于页面大小 */
		{
			if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;
				
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
				{
					return 0;
				}
								
				_uiWriteAddr +=  count;
				_pBuf += count;
			
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, temp) == 0)
				{
					return 0;
				}				
			}
			else
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
				{
					return 0;
				}
			}
		}
		else	/* 数据长度大于等于页面大小 */
		{
			_usWriteSize -= count;
			NumOfPage =  _usWriteSize / SF_PAGE_SIZE;
			NumOfSingle = _usWriteSize % SF_PAGE_SIZE;
			
			if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
			{
				return 0;
			}			
				
			_uiWriteAddr +=  count;
			_pBuf += count;
			
			while (NumOfPage--)
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, SF_PAGE_SIZE) == 0)
				{
					return 0;
				}				
				_uiWriteAddr +=  SF_PAGE_SIZE;
				_pBuf += SF_PAGE_SIZE;
			}
			
			if (NumOfSingle != 0)
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
				{
					return 0;
				}				
			}
		}
	}
	return 1;	/* 成功 */
}


/*
*********************************************************************************************************
*	函 数 名: sf_ReadID
*	功能说明: 读取器件ID
*	形    参：无
*	返 回 值: 32bit的器件ID (最高8bit填0，有效ID位数为24bit）
*********************************************************************************************************
*/  
uint32_t sf_ReadID(void)
{
	uint32_t uiID; 
	uint8_t id1, id2, id3;
	
	sf_WriteEnable();								/* 发送写使能命令 */
	
	/* 擦除扇区操作 */
	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_RDID);							/* 发送读ID命令 */
	id1 = sf_SendByte(DUMMY_BYTE);					/* 读ID的第1个字节 */
	id2 = sf_SendByte(DUMMY_BYTE);					/* 读ID的第2个字节 */
	id3 = sf_SendByte(DUMMY_BYTE);					/* 读ID的第3个字节 */
	SF_CS_HIGH();									/* 禁能片选 */

	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;
	
	return uiID;
}		

/*
*********************************************************************************************************
*	函 数 名: sf_SendByte
*	功能说明: 向器件发送一个字节，同时从MISO口线采样器件返回的数据
*	形    参：_ucByte : 发送的字节值
*	返 回 值: 从MISO口线采样器件返回的数据
*********************************************************************************************************
*/
static uint8_t sf_SendByte(uint8_t _ucValue)
{
	/* 等待上个数据未发送完毕 */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_TXE) == RESET);
	
	/* 通过SPI硬件发送1个字节 */
	SPI_I2S_SendData(SPI_FLASH, _ucValue);
	
	/* 等待接收一个字节任务完成 */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* 返回从SPI总线读到的数据 */
	return SPI_I2S_ReceiveData(SPI_FLASH);
}

/*
*********************************************************************************************************
*	函 数 名: sf_WriteEnable
*	功能说明: 向器件发送写使能命令
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sf_WriteEnable(void)
{
	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_WREN);							/* 发送命令 */
	SF_CS_HIGH();									/* 禁能片选 */  
}

/*
*********************************************************************************************************
*	函 数 名: sf_WriteStatus
*	功能说明: 写状态寄存器
*	形    参：_ucValue : 状态寄存器的值
*	返 回 值: 无
*********************************************************************************************************
*/  
static void sf_WriteStatus(uint8_t _ucValue)
{
	/* 第1步：先使能写状态寄存器 */
	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_EWRSR);							/* 发送命令， 允许写状态寄存器 */
	SF_CS_HIGH();									/* 禁能片选 */ 
	
	/* 第2步：再写状态寄存器 */
	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_WRSR);							/* 发送命令， 写状态寄存器 */
	sf_SendByte(_ucValue);							/* 发送数据：状态寄存器的值 */
	SF_CS_HIGH();									/* 禁能片选 */ 	
}	

/*
*********************************************************************************************************
*	函 数 名: sf_WaitForWriteEnd
*	功能说明: 采用循环查询的方式等待器件内部写操作完成
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/  
static void sf_WaitForWriteEnd(void)
{
	SF_CS_LOW();									/* 使能片选 */
	sf_SendByte(CMD_RDSR);							/* 发送命令， 读状态寄存器 */
	while((sf_SendByte(DUMMY_BYTE) & WIP_FLAG) == SET);	/* 判断状态寄存器的忙标志位 */
	SF_CS_HIGH();									/* 禁能片选 */ 	
}
