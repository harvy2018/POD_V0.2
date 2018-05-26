#ifndef __SWAPORDER_H_
#define __SWAPORDER_H_

#include "PLCMessage.h"

union UInt16_Swap
{
	UINT16 _Data;
	BYTE _DataArray[2];
};

union Int16_Swap
{
	INT16 _Data;
	BYTE _DataArray[2];
};

union UInt32_Swap
{
	UINT32 _Data;
	BYTE _DataArray[4];
};

union Int32_Swap
{
	INT32 _Data;
	BYTE _DataArray[4];
};


INT16 SwapOrder_Int16(INT16 data);
UINT16 SwapOrder_UInt16(UINT16 data);

UINT32 SwapOrder_Int32(INT32 data);
UINT32 SwapOrder_UInt32(UINT32 data);

UINT32 SwapInLine(BYTE* pointer, int size);

#endif

