/*
********************************************************************************
*                           
*                              程序下载部分
*
* File          : Download.C
* Version       : V1.0
* By            : whq
*
* For           : Stm32f10x
* Mode          : Thumb2
* Toolchain     : 
*                   RealView Microcontroller Development Kit (MDK)
*                   Keil uVision
* Description   : 
*
*******************************************************************************/

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include <stdbool.h>
#include "Download.h"




//static u32 m_EraseCounter = 0;


/*******************************************************************************
* Function Name :unsigned int FLASH_PagesMask(volatile unsigned int Size)
* Description   :计算所要擦除 的页数
* Input         :
* Output        :
* Other         :
*******************************************************************************/
static u32 FLASH_PagesMask(vu32 Size)
{
    u32 pagenumber = 0x0;
    u32 size = Size;

    if ((size % PAGE_SIZE) != 0)
    {
        pagenumber = (size / PAGE_SIZE) + 1;
    }
    else
    {
        pagenumber = size / PAGE_SIZE;
    }
    return pagenumber;
}


/*******************************************************************************
* Function Name :u32 FLASH_WriteBank(u32 *pData, u32 addr, u32 size)
* Description   :写入一块数据
* Input         :pData:数据；addr:数据的地址；size:长度
* Output        :TRUE:成功，FALSE:失败。
* Other         :
*******************************************************************************/
u32 FLASH_WriteBank(u8 *pData, u32 addr, u32 size)
{
    vu16 *pDataTemp = (vu16 *)pData;
    vu32 temp = addr;
//    FLASH_Status FLASHStatus = FLASH_COMPLETE;
//    u32 NbrOfPage = 0;
//
//    NbrOfPage = FLASH_PagesMask(addr + size - ApplicationAddress);
//    for (; (m_EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); m_EraseCounter++)
//    {
//        FLASHStatus = FLASH_ErasePage(ApplicationAddress + (PAGE_SIZE * m_EraseCounter));
//    }

    for (; temp < (addr + size); pDataTemp++, temp += 2)
    {
        FLASH_ProgramHalfWord(temp, *pDataTemp);
        if (*pDataTemp != *(vu16 *)temp)
        {
            return false;
        }
    }

    return true;
}

/*******************************************************************************
* Function Name :void FLASH_ProgramDone(void)
* Description   :烧写启动
* Input         :
* Output        :
* Other         :
*******************************************************************************/
FLASH_Status FLASH_ProgramStart(u32 ADDR,vu32 ApplicationSize)
{
    FLASH_Status FLASHStatus = FLASH_COMPLETE;
    u32 NbrOfPage = 0;
    vu32 EraseCounter = 0;

    FLASH_Unlock();
//    m_EraseCounter = 0;

    NbrOfPage = FLASH_PagesMask(ApplicationSize);
    for (; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        FLASHStatus = FLASH_ErasePage(ADDR + (PAGE_SIZE * EraseCounter));
    }
		return FLASHStatus;
}


/*******************************************************************************
* Function Name :void FLASH_ProgramDone(void)
* Description   :烧写结束
* Input         :
* Output        :
* Other         :
*******************************************************************************/
void FLASH_ProgramDone(void)
{
    FLASH_Lock();
}


/********************************* END ****************************************/

