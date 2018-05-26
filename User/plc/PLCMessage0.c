#include "SerialPort.h"
#include "stdio.h"
#include "PLCMessage.h"
#include "CRC16.H"
#include "SwapOrder.h"
#include "cb.h"

BYTE g_errInfo[MAX_ERRINFO_LENGTH] = {0};
int g_curCmd = 0;
UINT16 _PAN_Id = 0;
UINT16 _LBA_Address = 0;
bool g_init = false;

BYTE genIpv6Addr[16] = {0xFE,0x80,0x00,0x00,0x00,0x00,0x00,0x00, 
                        0x00,0x00,0x00,0xFF,0xFE,0x00,0x00,0x00};

BYTE ToneMasks[13][14]  = {{0x17, 0x24, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0 =  Cenelec A36
{0x17, 0x24, 0xFF, 0xFF, 0x00, 0xF8, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1 = Cenelec A25
{0x3F, 0x10, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2 = Cenelec B
{0x3F, 0x1A, 0xFF, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3 = Cenelec BC
{0x3F, 0x20, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4 = Cenelec BCD
{0x21, 0x24, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 5 = FCC Low Band
{0x45, 0x24, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 6 = FCC High Band
{0x21, 0x48, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00 }, // 7 = FCC Full Band.

{0x3B, 0XA4, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 8 = ????
{0x1F, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 }, // 9 = ????
{0x21, 0x0C, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 10= FCC ARIB 12 
{0x21, 0x18, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 11= FCC ARIB 24
{0x21, 0x36, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 12= FCC ARIB 54
};

void SetErrInfo(BYTE * errInfo)
{
	memset(g_errInfo, 0, MAX_ERRINFO_LENGTH);
	memcpy(g_errInfo, errInfo, strlen((char*)errInfo));
}

void MakeCRC_Header(struct CRC_Header * crcHeader, void * header, int headerLength, void * body, int bodyLength)
{
	crcHeader->_HeaderCRC = CRC16_BlockChecksum( header, headerLength);
	if (bodyLength == 0)
	{
		crcHeader->_MessageCRC = 0;
	}
	else
	{
		crcHeader->_MessageCRC = CRC16_BlockChecksum(body, bodyLength);
	}
}

BYTE* MakeMessage(void * message, int size)
{
	int payloadLength = size - sizeof(struct Header);
	int headerLength = sizeof(struct Header) + sizeof(struct CRC_Header); 
	struct Header * header = (struct Header *)message;
	BYTE * pMessage = (BYTE *)message;
	BYTE * msg = NULL;
	int relSize = 0;
   if((size & 1) == 1)
	{
		relSize = size+1;
	}
	else
	{
		relSize = size;
	}
	msg = (BYTE *) malloc(relSize);
	if(msg!=0)
	{
		memset(msg, 0, relSize);
		struct CRC_Header * crcHeader = (struct CRC_Header *)(&pMessage[sizeof(struct Header)]);	
		header->_Length = (UINT16) payloadLength;	
		MakeCRC_Header(crcHeader, header, sizeof(struct Header), & pMessage[headerLength], size-headerLength);	
		memcpy(msg, message, size);
	}
	return msg;
}

bool recvMessage(BYTE* content)
{
	if (content == NULL)
	{
		return false;
	}

	UINT16 bytesRead = 0;
	BYTE* recvHeader = Read(4, &bytesRead);
	if (recvHeader == NULL)
	{
		return false;
	}

	UINT16* recvPayLoad = (UINT16*)(recvHeader+2);
	if (bytesRead == 4)
	{
		BYTE * buffer = Read(*recvPayLoad, &bytesRead);
		if (*recvPayLoad == bytesRead)
		{
			if ((*recvPayLoad & 1) == 1)
			{
				Read(1, &bytesRead);
			}
			memcpy(content, recvHeader, 4);
			memcpy(content+4, buffer, *recvPayLoad);

			free(recvHeader);
			free(buffer);
			return true;
		}
		else
		{
			free(recvHeader);
			free(buffer);
			return false;
		}
	}
	else
	{
		free(recvHeader);
		return false;
	}
}

void InitLoadSystemConfigMsg(struct LoadSystemConfig * _LoadSystemConfigPort)
{
	_LoadSystemConfigPort->_Header._Id = 0x0C;
	_LoadSystemConfigPort->_Header._Origin = 0x80;
	_LoadSystemConfigPort->_Port = NULL;
	_LoadSystemConfigPort->_SystemConfig = NULL;
	_LoadSystemConfigPort->_G3_Config = NULL;
}

BYTE * MakeLoadSystemConfigMsg(struct LoadSystemConfig * config)
{
	UINT16 totalMessageLength = sizeof(struct Header) + sizeof(struct CRC_Header);
	int tlvLength = 0;
	BYTE * payLoad = NULL;
	BYTE * msg = NULL;
	int offset = 0;
	int headerLength = 0;
	
	if (config->_Port != NULL)
	{
		config->_Port->_Type = 0x0001;
		config->_Port->_Length = 1;
		
		tlvLength += sizeof(struct PortDesignation);
	}
	if (config->_SystemConfig != NULL)
	{
		config->_SystemConfig->_Type = 0x0003;
		config->_SystemConfig->_Length = 26;
		
		tlvLength += sizeof(struct SystemConfig);
	}
	if(config->_G3_Config != NULL)
	{
		config->_G3_Config->_Type = 0x0008;
		config->_G3_Config->_Length = 16;
		config->_G3_Config->_MAC_SegmentSize = 239; // obsolete but needed for older dsp s/w
		
		tlvLength += sizeof(struct G3Configuration);
	}
	totalMessageLength += (UINT16)tlvLength;
	headerLength = totalMessageLength - sizeof(struct Header);;
	//
	// Make sure the message length is even
	//
	if ((totalMessageLength & 1) != 0)
	{
		totalMessageLength++;
	}
	msg = (BYTE *) malloc(totalMessageLength);
	if(msg!=0)
	{
			memset(msg, 0, totalMessageLength);
			
			payLoad = (BYTE *) malloc(tlvLength);
			
			if (config->_Port != NULL)
			{
				memcpy(&payLoad[offset], config->_Port, sizeof(struct PortDesignation));
				offset += sizeof(struct PortDesignation);
			}
			if (config->_SystemConfig != NULL)
			{
				memcpy(&payLoad[offset], config->_SystemConfig, sizeof(struct SystemConfig));
				offset += sizeof(struct SystemConfig);
			}
			if(config->_G3_Config != NULL)
			{
				memcpy(&payLoad[offset], config->_G3_Config, sizeof(struct G3Configuration));
				offset += sizeof(struct G3Configuration);
			}
			
			config->_Header._Length = (UINT16)headerLength;
			
			MakeCRC_Header(&config->_CRC_Header, &config->_Header, sizeof(struct Header), payLoad, tlvLength);
			
			offset = 0;
			memcpy(&msg[offset], config, sizeof(struct Header) + sizeof(struct CRC_Header));
			offset += sizeof(struct Header) + sizeof(struct CRC_Header);
			memcpy(&msg[offset], payLoad, tlvLength);
			
			free(payLoad);	
	}
	return msg;
}

int GetLoadSystemConfigMsgSize(struct LoadSystemConfig config)
{
	int size = sizeof(struct Header) + sizeof(struct CRC_Header);
	if (config._Port != NULL)
	{
		size += sizeof(struct PortDesignation);
	}
	if (config._SystemConfig != NULL)
	{
		size += sizeof(struct SystemConfig);
	}
	if (config._G3_Config != NULL)
	{
		size += sizeof(struct G3Configuration);
	}
	size += (size & 1);	
	return size;
}

int ReadVariableLengthMessage(void ** msg, BYTE * message, int offset)
{
	struct Header * header = (struct Header *)message;
	int length = header->_Length + 4;
	
	if (*msg != NULL)
	{
		free(*msg);
	}
	*msg =(void*)malloc(length);

		memset(*msg, 0 , length);
		memcpy(*msg, &message[offset], length);
		offset += length;
	
	return offset;
}

void * FormDataTransferMessage(void * transferStruct, BYTE * msg, int size)
{
	struct DataTransfer * newMessage;
	struct DataTransfer * orgMessage;
	//
	// Size without data..only the basic message with the IPv6 and UPD (no data) headers..
	//
	int UDP_Size = size + sizeof (struct UDP);
	//
	// Total message size.
	//
	int totalMessageSize = sizeof(struct DataTransfer) + size; 
	//
	// allocate space for the new message;
	//
	BYTE * messagePointer = (BYTE *) malloc(totalMessageSize);
	memset(messagePointer, 0, totalMessageSize);
	
	newMessage = (struct DataTransfer *)messagePointer;
	orgMessage = (struct DataTransfer *)transferStruct;
	//
	// clear and copy over the IPv6 and UPD data to the new message...
	//
	newMessage->_Header._Id = orgMessage->_Header._Id;
	newMessage->_Header._Origin = orgMessage->_Header._Origin;
	newMessage->_Header._Length = (UINT16)(totalMessageSize-sizeof(struct Header));
	
	newMessage->_Spacer = orgMessage->_Spacer;
	
	newMessage->_IPv6_Header._Header[0] = 0x60;
	newMessage->_IPv6_Header._NextHeader = 17; // This is value for the UDP header..
	newMessage->_IPv6_Header._HopLimit = orgMessage->_IPv6_Header._HopLimit;
	newMessage->_IPv6_Header._PayloadLength = (UINT16)UDP_Size;
	memcpy(newMessage->_IPv6_Header._DestinationAddress, orgMessage->_IPv6_Header._DestinationAddress, 16);
	memcpy(newMessage->_IPv6_Header._SourceAddress, orgMessage->_IPv6_Header._SourceAddress, 16);
	
	newMessage->_UDP_Message._CRC = 0xABCD;  // just put something here for wireshark..
	newMessage->_UDP_Message._SourcePort = orgMessage->_UDP_Message._SourcePort;
	newMessage->_UDP_Message._DestinationPort = orgMessage->_UDP_Message._DestinationPort;
	newMessage->_UDP_Message._Length = (UINT16)UDP_Size;
	memcpy(&newMessage->_UDP_Message._Data[0], msg, size);
	
	return newMessage;
}

UINT16 CalculateCRC(struct DataTransferRequest * dtr)
{
	UINT32 tempCRC = 0;
	UINT16 _CRC = 0;	
	
	UINT16 Length = dtr->_UDP_Message._Length;
	int size = 16;//Source add
	size += 16;//Destination add
	size += 4;//Length(4bytes);
	size += 4;//Three zeroed bytes + Next header byte
	size += 2;//Source Port(2 bytes)
	size += 2;//Dest port (2 bytes)
	size += 2;//Length(2 bytes)
	size += 2;//checksum (2 zeroed out bytes)
	size += Length - 8;//payload length( total - header(header = 8 bytes)
	
	if((size & 1) == 1) //Make the message end on an even boundary
		size++;
	
	BYTE *ba = (BYTE*) malloc(size);
	if(ba == NULL)
		return _CRC;
	
	int pos=0;
	memcpy(ba,dtr->_IPv6_Header._SourceAddress,16);
	pos += 16;
	
	memcpy(ba + pos, dtr->_IPv6_Header._DestinationAddress,16);
	pos += 16;
	
	ba[pos++] = 0;
	ba[pos++] = 0;	
	ba[pos++] = (Length >> 8) & 0xFF;
	ba[pos++] = Length & 0xFF;
	
	ba[pos++] = 0;
	ba[pos++] = 0;
	ba[pos++] = 0;
	ba[pos++] = 0x11;
	
	ba[pos++] = (dtr->_UDP_Message._SourcePort >> 8) & 0xFF;
	ba[pos++] = dtr->_UDP_Message._SourcePort & 0xFF;
	
	ba[pos++] = (dtr->_UDP_Message._DestinationPort >> 8) & 0xFF;
	ba[pos++] = dtr->_UDP_Message._DestinationPort & 0xFF;
	
	ba[pos++] = (Length >> 8) & 0xFF;
	ba[pos++] = Length & 0xFF;
	
	ba[pos++] = 0;
	ba[pos++] = 0;
	
	memcpy(ba + pos, dtr->_UDP_Message._Data, Length - 8);
	pos += Length - 8;
	
	if((Length & 1) == 1)
		ba[pos++] = 0;
	
	for (int index = 0; index < size; index += 2)
	{
		UINT16 temp = ba[index+1] + (ba[index] << 8);
		tempCRC += temp;
	}
	
	while ((tempCRC >> 16) > 0)
	{
		tempCRC = (tempCRC & 0xFFFF) + (tempCRC >> 16);
	}
	
	_CRC = ((UINT16)~tempCRC);
	dtr->_UDP_Message._CRC = _CRC;
	
	free(ba);
	
	return _CRC;
}

void SwapBytes(struct DataTransferRequest * _pDTR)
{
	//
	// Swap the bytes... for the UINT16 fields.
	//
	_pDTR->_IPv6_Header._PayloadLength = SwapOrder_UInt16(_pDTR->_IPv6_Header._PayloadLength);
	_pDTR->_UDP_Message._DestinationPort = SwapOrder_UInt16(_pDTR->_UDP_Message._DestinationPort);
	_pDTR->_UDP_Message._SourcePort = SwapOrder_UInt16(_pDTR->_UDP_Message._SourcePort);
	_pDTR->_UDP_Message._Length = SwapOrder_UInt16(_pDTR->_UDP_Message._Length);
	_pDTR->_UDP_Message._CRC = SwapOrder_UInt16(_pDTR->_UDP_Message._CRC);
}

UINT16 SendGetSystemInfoRequest()
{
	UINT16 res = 0x0000;
	struct GetSystemInfoRequest _GetSystemInfoRequest;
	memset(&_GetSystemInfoRequest, 0, sizeof(struct GetSystemInfoRequest));
	_GetSystemInfoRequest._Header._Id = Get_System_Info_Id;
	_GetSystemInfoRequest._Header._Origin = 0x80;
	
	void* message = & _GetSystemInfoRequest;
	int size = sizeof(struct GetSystemInfoRequest);

	g_curCmd = GetSystemInfoRequestCmd;	
	BYTE * msg = MakeMessage(message, size);
 	if(!Write(msg, ((size&1)==1)?size+1:size))
 	{
 		res  = 0x0001;
 	}
	free(msg);
	return res;
}

bool RecvMessage(struct Header *_header, BYTE* _body, void* content)
{
	memcpy((BYTE*)content, (BYTE*)_header, sizeof(struct Header));
	memcpy((BYTE*)content+sizeof(struct Header), _body, _header->_Length);
	return true;
}

bool CheckInit(struct SystemInfo sysInfo)
{
//	return false;

	if (sysInfo._DeviceMode != 0)
	{
		return false;
	}
	if ((sysInfo._PortAssigments & 0x03) == 0)
	{
		return false;
	}
	if ((sysInfo._PortAssigments & 0x0C) == 0)
	{
		return false;
	}
 	BYTE longAddr[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 	if (memcmp(sysInfo._LongAddress, longAddr, 8) == 0)
 	{
 		return false;
 	}
	return true;
}

UINT16 SendLoadSystemConfig_Port(int dataPort, int diagPort)
{
	UINT16 res = 0x0000;
	struct LoadSystemConfig _LoadSystemConfigPort;
	InitLoadSystemConfigMsg(&_LoadSystemConfigPort);
	
	if (_LoadSystemConfigPort._Port == NULL)
	{
		_LoadSystemConfigPort._Port = (struct PortDesignation*) malloc(sizeof(struct PortDesignation));
		memset(_LoadSystemConfigPort._Port, 0, sizeof(struct PortDesignation));
	}
	
	//set DataPort
	_LoadSystemConfigPort._Port->_Port &= 0x0C;
	if (dataPort != 0)
	{
		_LoadSystemConfigPort._Port->_Port |= 0x01;
	}
	
	
	//set DiagPort
	_LoadSystemConfigPort._Port->_Port &= 0x03;
	if (diagPort != 0)
	{
		_LoadSystemConfigPort._Port->_Port |= 0x04;
	}
	
	
	BYTE *msg = MakeLoadSystemConfigMsg(&_LoadSystemConfigPort);
	int size  = GetLoadSystemConfigMsgSize(_LoadSystemConfigPort);
	g_curCmd = LoadSystemConfig_PortCmd;	
	
	if(!Write(msg, ((size&1)==1)?size+1:size))
	{
		res  = 0x0001;
 	}

	free(msg);
	if (_LoadSystemConfigPort._Port != NULL)
	{
		free(_LoadSystemConfigPort._Port);
	}
	return res;
}


UINT16 SendLoadSystemConfig_DeviceMode(int deviceMode)
{
	UINT16 res = 0x0000;
	struct LoadSystemConfig _LoadSystemConfigDeviceMode;
	InitLoadSystemConfigMsg(&_LoadSystemConfigDeviceMode);
	
	if (_LoadSystemConfigDeviceMode._SystemConfig == NULL)
	{
		_LoadSystemConfigDeviceMode._SystemConfig = (struct SystemConfig*) malloc(sizeof(struct SystemConfig));
		memset(_LoadSystemConfigDeviceMode._SystemConfig, 0, sizeof(struct SystemConfig));
	}
	
	//set DeviceMode
	_LoadSystemConfigDeviceMode._SystemConfig->_DeviceMode = deviceMode;
	
	BYTE* msg = MakeLoadSystemConfigMsg(&_LoadSystemConfigDeviceMode);
	int size = GetLoadSystemConfigMsgSize(_LoadSystemConfigDeviceMode);
	g_curCmd = LoadSystemConfig_DeviceModeCmd;	
	
	if(!Write(msg, ((size&1)==1)?size+1:size))
	{
		res  = 0x0001;
 	}

	if (_LoadSystemConfigDeviceMode._SystemConfig != NULL)
	{
		free(_LoadSystemConfigDeviceMode._SystemConfig);
	}
	free(msg);
	return res;
}

UINT16 SendLoadSystemConfig_G3Config(BYTE* longAddress)
{
	UINT16 res = 0x0000;
	struct LoadSystemConfig _LoadSystemConfigG3Config;
	InitLoadSystemConfigMsg(&_LoadSystemConfigG3Config);
	
	if (_LoadSystemConfigG3Config._G3_Config == NULL)
	{
		_LoadSystemConfigG3Config._G3_Config = (struct G3Configuration*) malloc(sizeof(struct G3Configuration));
		memset(_LoadSystemConfigG3Config._G3_Config, 0, sizeof(struct G3Configuration));
	}
	
	//set longAddress
	memcpy(_LoadSystemConfigG3Config._G3_Config->_LongAddress, longAddress, 8);
	
	BYTE * msg = MakeLoadSystemConfigMsg(&_LoadSystemConfigG3Config);
	int size = GetLoadSystemConfigMsgSize(_LoadSystemConfigG3Config);
	g_curCmd = LoadSystemConfig_G3ConfigCmd;

	if(!Write(msg, ((size&1)==1)?size+1:size))
	{
		res  = 0x0001;
 	}
	
	if (_LoadSystemConfigG3Config._G3_Config != NULL)
	{
		free(_LoadSystemConfigG3Config._G3_Config);
	}
	free(msg);
	return res;
}


UINT16 SendShutDownMessage()
{
	UINT16 res = 0x0000;

	struct ShutDownMessage _ShutDownMessage;
	memset(&_ShutDownMessage, 0, sizeof(struct ShutDownMessage));
	_ShutDownMessage._Header._Id = Shut_Down_Id;
	_ShutDownMessage._Header._Origin = 0x80;
	_ShutDownMessage._ResetType = SoftReset;

	int size = sizeof(struct ShutDownMessage);
	BYTE* msg = MakeMessage(&_ShutDownMessage, size);
	g_curCmd = ShutDownMessageCmd;

	if (!Write(msg, ((size&1)==1)?size+1:size))
	{
		res = 0x0001;
	}

	free(msg);
    return res;
}

UINT16 SendDiscoverNetworkRequest()
{
	UINT16 res = 0x0000;

	struct DiscoverNetworkRequest _DiscoverNetworkRequest;
	memset(&_DiscoverNetworkRequest, 0, sizeof(_DiscoverNetworkRequest));
	_DiscoverNetworkRequest._Header._Id = Discover_Id;
	_DiscoverNetworkRequest._Header._Origin = 0x80;
    _DiscoverNetworkRequest._DiscoverType = NetworkDiscover;
	_DiscoverNetworkRequest._Duration = 5;
	 		
	int size = sizeof(struct DiscoverNetworkRequest);
	BYTE* msg = MakeMessage(&_DiscoverNetworkRequest, size);
	g_curCmd = DiscoverNetworkRequestCmd;

	if (!Write(msg, ((size&1)==1)?size+1:size))
	{
		res = 0x0001;
	}

	free(msg);
	return res;
}

UINT16 RecvDiscoverNetworkConfirm(BYTE* pContent)
{
	UINT16 res = 0x0000;
		
	struct DiscoverNetworkConfirm* _pDiscoverNetworkConfirm = NULL;
	ReadVariableLengthMessage((void**)&_pDiscoverNetworkConfirm, pContent, 0);

	if (_pDiscoverNetworkConfirm->_Status == 0 && _pDiscoverNetworkConfirm->_PAN_Count >0)
	{
		_PAN_Id = _pDiscoverNetworkConfirm->_PAN_Info[0]._Coord_PAN_Address;
		_LBA_Address = _pDiscoverNetworkConfirm->_PAN_Info[0]._ShortAddress;
	}
	else
	{
		res = 0x000C;
	}

	free(_pDiscoverNetworkConfirm);
	return res;
}

UINT16 SendAttachRequest()
{
	UINT16 res = 0x0000;
	struct AttachRequest _AttachRequest;
	memset(&_AttachRequest, 0, sizeof(struct AttachRequest));
	_AttachRequest._Header._Id = Attach_Id;
	_AttachRequest._Header._Origin = 0x80;
	 
	_AttachRequest._LBA_Address = _LBA_Address;
	_AttachRequest._PAN_Id = _PAN_Id;
	
	int size = sizeof(struct AttachRequest);
	BYTE* msg = MakeMessage(&_AttachRequest, size);

	if (!Write(msg, ((size&1)==1)?size+1:size))
	{
		res = 0x0001;
	}
	free(msg);
	g_curCmd = AttachRequestCmd;

	return res;
}

bool SendDetachRequest(BYTE * longAddr)
{
	bool res = true;
	struct DetachRequest _DetachRequest;
	memset(&_DetachRequest, 0, sizeof(struct DetachRequest));
	_DetachRequest._Header._Id = Detach_Id;
	_DetachRequest._Header._Origin = 0x80;
	
	memcpy(_DetachRequest._ExtendedAddress, longAddr, 8);
	
	int size = sizeof(struct DetachRequest);
	BYTE* msg = MakeMessage(&_DetachRequest, size);
	res = Write(msg, ((size&1)==1)?size+1:size);
	g_curCmd = DetachRequestCmd;
	if(!res)
	{
		SetErrInfo((unsigned char*)"SendDetachRequest报文发送失败");
		res = false;
	}
	free(msg);
	return res;
}

bool SendData(BYTE* data, int length)
{
	bool res = true;
	struct DataTransferRequest _DataTransferRequest;
	memset(&_DataTransferRequest, 0, sizeof(struct DataTransferRequest));
	
	static BYTE _NSDU = 0;
	
	_DataTransferRequest._Header._Id = Data_Transfer_Id;
	_DataTransferRequest._Header._Length = 4 /* _CRC */ + sizeof(struct IPv6) + 8; /* min udp size */
	_DataTransferRequest._Header._Origin = 0x80;
	_DataTransferRequest._NSDU_Handle = _NSDU++;
	_DataTransferRequest._UDP_Message._Length = 8;				/* min udp size */
	_DataTransferRequest._IPv6_Header._PayloadLength = 8;	/* min udp size */
	_DataTransferRequest._IPv6_Header._HopLimit = 8;

	memcpy(_DataTransferRequest._IPv6_Header._SourceAddress, genIpv6Addr, 16);
	
	struct DataTransferRequest* _pDataTransferRequest = (struct DataTransferRequest*)FormDataTransferMessage(&_DataTransferRequest, (unsigned char*)data, strlen((char*)data));
	CalculateCRC(_pDataTransferRequest);
	
	int size = _pDataTransferRequest->_Header._Length + 4;
	
	SwapBytes(_pDataTransferRequest);
	BYTE * msg = MakeMessage(_pDataTransferRequest, size);
	g_curCmd = DataTransferRequestCmd;
	res = Write(msg, ((size&1)==1)?size+1:size);
	SwapBytes(_pDataTransferRequest);
	
	if(!res)
	{
		SetErrInfo((unsigned char*)"DataTransferRequest报文发送失败");
		res = false;
	}
	free(msg);
 	free(_pDataTransferRequest);
	return res;
}

bool RecvDataTransferConfirm(struct Header *_header, BYTE* _body)
{
	bool res = true;
	BYTE * content = (BYTE*)malloc(sizeof(struct Header)+_header->_Length);
	memset(content, 0, sizeof(struct Header)+_header->_Length);
	memcpy(content, _header, sizeof(struct Header));
	memcpy(content+sizeof(struct Header), _body, _header->_Length);
	
	struct DataTransferIndication* _pDataTransferIndication = NULL;
	ReadVariableLengthMessage((void**)&_pDataTransferIndication, content, 0);

	_pDataTransferIndication->_IPv6_Header._PayloadLength = SwapOrder_UInt16(_pDataTransferIndication->_IPv6_Header._PayloadLength);
	_pDataTransferIndication->_UDP_Message._DestinationPort = SwapOrder_UInt16(_pDataTransferIndication->_UDP_Message._DestinationPort);
	_pDataTransferIndication->_UDP_Message._SourcePort = SwapOrder_UInt16(_pDataTransferIndication->_UDP_Message._SourcePort);
	_pDataTransferIndication->_UDP_Message._Length = SwapOrder_UInt16(_pDataTransferIndication->_UDP_Message._Length);
	_pDataTransferIndication->_UDP_Message._CRC = SwapOrder_UInt16(_pDataTransferIndication->_UDP_Message._CRC);

	char buf[100] = {0};
	int length=_pDataTransferIndication->_UDP_Message._Length;
	memcpy(buf, _pDataTransferIndication->_UDP_Message._Data, length-8);
	//printf("recv the data:%s\n", buf);

	free(content);
	free(_pDataTransferIndication);
	return res;
}


UINT16 SendSetTxInfo()
{
	extern u32 TX_Modulation;
	UINT16 res = 0x0000;
  
	struct SetInfo_TX _SetInfo_TX;
	memset(&_SetInfo_TX, 0, sizeof(struct SetInfo_TX));

	_SetInfo_TX._Header._Id = Set_Info_Id;
	_SetInfo_TX._Header._Origin = 0x80;
	_SetInfo_TX._InfoType = 0x0002;
	
	_SetInfo_TX._InfoLength = 18;
	_SetInfo_TX._TX_Level = 32;
	memcpy(_SetInfo_TX._ToneMask, ToneMasks[Cenelec_A_36], sizeof(_SetInfo_TX._ToneMask));

	_SetInfo_TX._Flags &= 0xFE;

	_SetInfo_TX._Flags &= 0xF7;

	_SetInfo_TX._Modulation = TX_Modulation;//QPSK

	_SetInfo_TX._Flags &= 0xBF;
	
	void* message = &_SetInfo_TX;
	int size = sizeof(struct SetInfo_TX);

	BYTE * msg = MakeMessage(message, size);
	g_curCmd = SetTxInfoCmd;
	
	if(!Write(msg, ((size&1)==1)?size+1:size))
	{
		res = 0x0001;
	}
	free(msg);
	return res;
}


UINT16 SendSetTxInfo_3()
{
	UINT16 res = 0x0000;

	struct SetInfo_TX _SetInfo_TX;
	memset(&_SetInfo_TX, 0, sizeof(struct SetInfo_TX));

	_SetInfo_TX._Header._Id = Set_Info_Id;
	_SetInfo_TX._Header._Origin = 0x80;
	_SetInfo_TX._InfoType = 0x0002;
	
	_SetInfo_TX._InfoLength = 18;
	_SetInfo_TX._TX_Level = 32;
	memcpy(_SetInfo_TX._ToneMask, ToneMasks[Cenelec_A_36], sizeof(_SetInfo_TX._ToneMask));

	_SetInfo_TX._Flags &= 0xFE;

	_SetInfo_TX._Flags &= 0xF7;

	_SetInfo_TX._Modulation = 3;

	_SetInfo_TX._Flags &= 0xBF;
	
	void* message = &_SetInfo_TX;
	int size = sizeof(struct SetInfo_TX);

	BYTE * msg = MakeMessage(message, size);
	g_curCmd = SetTxInfoCmd;
	
	if(!Write(msg, ((size&1)==1)?size+1:size))
	{
		res = 0x0001;
	}
	free(msg);
	return res;
}


UINT16 SendSetRxInfo()
{
	UINT16 res = 0;
	struct SetInfo_RX _SetInfo_RX;
	memset(&_SetInfo_RX, 0, sizeof(struct SetInfo_RX));
	
	_SetInfo_RX._Header._Id = Set_Info_Id;
	_SetInfo_RX._Header._Origin = 0x80;
	_SetInfo_RX._InfoType = 0x0003;	
	_SetInfo_RX._InfoLength = 18;

	_SetInfo_RX._Flags &= 0x07;
	
	_SetInfo_RX._Flags &= 0x0E;
	_SetInfo_RX._Flags |= 0x01;

	_SetInfo_RX._Flags &= 0x0D;
	_SetInfo_RX._Flags |= 0x02;
	
	_SetInfo_RX._GainValue=0;//
  
	_SetInfo_RX._Flags &= 0xBF;

	memcpy(_SetInfo_RX._ToneMask, ToneMasks[Cenelec_A_36], sizeof(_SetInfo_RX._ToneMask));

	void* message = &_SetInfo_RX;
	int size = sizeof(struct SetInfo_RX);
	
	BYTE * msg = MakeMessage(message, size);
	g_curCmd = SetRxInfoCmd;
	
	if(!Write(msg, ((size&1)==1)?size+1:size))
	{
		res = 0x0001;
	}
	free(msg);

	return res;
}

UINT16 getDataFromPLCs(BYTE* plcData, UINT16 plcDataLen, INT32 *moreLen, BYTE* orgData, UINT16* orgDataLen)
{
	UINT16 res=0;
	if (NULL == plcData 
	 || NULL == moreLen
	 || NULL == orgData
	 || NULL == orgDataLen)
	{
		if (NULL == plcData)
		{
			res = 0x0001;
		}
		else if (NULL == moreLen)
		{
			res = 0x0002;
		}
		else if (NULL == orgData)
		{
			res = 0x0003;
		}
		else
		{
			res = 0x0004;
		}
	}
	else
	{
		int pos = 0;
		
		while(true)
		{
			if (pos+sizeof(struct Header) > plcDataLen)
			{
				res = 0x0005;
				break;
			}
			if (plcData[pos]<=0x14 && plcData[pos+1]==0x00 && plcData[pos+3]<=0x04)
			{
				break;
			}
			pos++;			
		}
		
		struct Header* _header = 0;
		if (res == 0x0000)
		{
			_header = (struct Header*)(plcData+pos);
			if (_header->_Length + sizeof(struct Header) <= (plcDataLen-pos))
			{
				*moreLen = (plcDataLen-pos) - _header->_Length - sizeof(struct Header);
				if (_header->_Id == Data_Transfer_Id)
				{
					if (_header->_Length != 8)
					{
						struct DataTransferIndication* _pDataTransferIndication = NULL;
						ReadVariableLengthMessage((void**)&_pDataTransferIndication, (plcData+pos), 0);
						
						_pDataTransferIndication->_IPv6_Header._PayloadLength = SwapOrder_UInt16(_pDataTransferIndication->_IPv6_Header._PayloadLength);
						_pDataTransferIndication->_UDP_Message._DestinationPort = SwapOrder_UInt16(_pDataTransferIndication->_UDP_Message._DestinationPort);
						_pDataTransferIndication->_UDP_Message._SourcePort = SwapOrder_UInt16(_pDataTransferIndication->_UDP_Message._SourcePort);
						_pDataTransferIndication->_UDP_Message._Length = SwapOrder_UInt16(_pDataTransferIndication->_UDP_Message._Length);
						_pDataTransferIndication->_UDP_Message._CRC = SwapOrder_UInt16(_pDataTransferIndication->_UDP_Message._CRC);
						
						if (*orgDataLen >= _pDataTransferIndication->_UDP_Message._Length - 8)
						{
							
							*orgDataLen = _pDataTransferIndication->_UDP_Message._Length - 8;
							memcpy(orgData, _pDataTransferIndication->_UDP_Message._Data, *orgDataLen);					
							res = 0x0000;
						}
						else
						{
							res = 0x0009;
						}
						//printf("recv the data:%s\n", buf);						
						free(_pDataTransferIndication);
					}
					else  //
					{
						struct DataTransferConfirm* dataTransferConfirm = (struct DataTransferConfirm*)(plcData+pos);
						if (dataTransferConfirm->_Status == 0)
						{
							res = 0x0008;
						}
						else
						{
							res = dataTransferConfirm->_Status;
						}						
					}
				}
				else  //
				{
					res = 0x0007;
				}
					
			}
			else
			{
				res = 0x0006;
				*moreLen = (plcDataLen-pos) - _header->_Length - sizeof(struct Header);
			}
		}

		if(_header->_Length%2!=0)
		{
			*moreLen -= 1;
		}
	}
	return res;
}

UINT16 getDataFromPLC(BYTE* plcData, UINT16 plcDataLen, BYTE* orgData, UINT16* orgDataLen)
{
	UINT16 res = 0;
	INT32 moreLen = 0;

	extern PLCRcvBuffer PLC_RBuffer;
	BYTE* newPlcData = NULL;
	newPlcData = (BYTE*)malloc(plcDataLen);
	memset(newPlcData, 0, plcDataLen);

	if ((plcDataLen + PLC_RBuffer.rptr) >= PLC_RCV_BUFFER_SIZE)
	{
		memcpy(newPlcData, plcData+PLC_RBuffer.rptr, PLC_RCV_BUFFER_SIZE-PLC_RBuffer.rptr);
		memcpy(newPlcData+PLC_RCV_BUFFER_SIZE-PLC_RBuffer.rptr, plcData, plcDataLen-PLC_RCV_BUFFER_SIZE+PLC_RBuffer.rptr);
	}
	else
	{
		memcpy(newPlcData, plcData+PLC_RBuffer.rptr, plcDataLen);
	}

	res = getDataFromPLCs(newPlcData, plcDataLen, &moreLen, orgData, orgDataLen);
	if(res == 0x0008)
	{
		PLC_BufIndex_Add(plcDataLen-moreLen);
		if (moreLen != 0)
		{
			INT32 moreLen2 = 0;
			res = getDataFromPLCs(newPlcData+plcDataLen-moreLen, moreLen, &moreLen2, orgData, orgDataLen);
			if (res == 0)
			{
				PLC_BufIndex_Add(moreLen-moreLen2);
			}
		}
		else
		{
			res =0x08;
			
		}		
	}
	else
	{
		if (res == 0)
		{
			PLC_BufIndex_Add(plcDataLen-moreLen);
		}
	}

	if (newPlcData != NULL)
	{
		free(newPlcData);
	}

	return res;

}


UINT16 genPLCData( BYTE* orgData, UINT16 orgDataLen, BYTE* plcData, UINT16* plcDataLen)
{
	UINT16 res;
	if (NULL == orgData 
		|| NULL == plcData
		|| NULL == plcDataLen)
	{
		if (NULL == orgData)
		{
			res = 0x0001;
		}
		else if (NULL == *plcData || NULL == plcData)
		{
			res = 0x0002;
		}
		else
		{
			res = 0x0003;
		}
	}
	else
	{
		struct DataTransferRequest _DataTransferRequest;
		memset(&_DataTransferRequest, 0, sizeof(struct DataTransferRequest));
		
		static BYTE _NSDU = 0;
		
		_DataTransferRequest._Header._Id = Data_Transfer_Id;
		_DataTransferRequest._Header._Length = 4 /* _CRC */ + sizeof(struct IPv6) + 8; /* min udp size */
		_DataTransferRequest._Header._Origin = 0x80;
		_DataTransferRequest._NSDU_Handle = _NSDU++;
		_DataTransferRequest._UDP_Message._Length = 8;				/* min udp size */
		_DataTransferRequest._IPv6_Header._PayloadLength = 8;	/* min udp size */
		_DataTransferRequest._IPv6_Header._HopLimit = 8;
		
		memcpy(_DataTransferRequest._IPv6_Header._SourceAddress, genIpv6Addr, 16);
		
		struct DataTransferRequest* _pDataTransferRequest = (struct DataTransferRequest*)FormDataTransferMessage(&_DataTransferRequest, orgData, orgDataLen);
		CalculateCRC(_pDataTransferRequest);

		int realDataLen = _pDataTransferRequest->_Header._Length + 4;
		int padLen = realDataLen;
		if ((realDataLen&1)==1)
		{
			padLen++;
		}

		if (*plcDataLen >= padLen)
		{
			*plcDataLen = padLen;
			SwapBytes(_pDataTransferRequest);
			BYTE* msg = MakeMessage(_pDataTransferRequest, realDataLen);
			memcpy(plcData, msg, *plcDataLen);
			SwapBytes(_pDataTransferRequest);
			free(msg);
			res = 0x0000;
			
		}
		else
		{
			res = 0x0004;
		}
	
		free(_pDataTransferRequest);		
	}
	return res;
}

UINT16 RecvDataFromSerialPort(BYTE* pContent, UINT16* pLen)
{
	UINT16 res = 0x0000;

	UINT16 bytesToRead = 0;	 //需要读取长度
	UINT16 bytesRead = 0;	 //实际读到长度
	struct Header* _header = NULL;
	BYTE* _body = NULL;
  BYTE* _morebyte= NULL;
	if (NULL == pContent
	  ||NULL == pLen)
	{
		if (NULL == pContent)
		{
			res = 0x0002;//第一个参数为NULL
		}
		else
		{
			res = 0x0003;//第二个参数为NULL
		}
	}
	else
	{
		BYTE * headerFirst = NULL;
		BYTE * headerSecond = NULL;
		BYTE * headerThird = NULL;
		BYTE * headerFourth = NULL;

		headerFirst = Read(1, &bytesRead);
		if (NULL == headerFirst || bytesRead != 1)
		{
			res = 0x0004;
		}
		if (res == 0x0000)
		{
			headerSecond = Read(1, &bytesRead);
			if (NULL == headerSecond || bytesRead != 1)
			{
				res = 0x0004;
			}
		}
		if (res == 0x0000)
		{
			headerThird = Read(1, &bytesRead);
			if (NULL == headerThird || bytesRead != 1)
			{
				res = 0x0004;
			}
		}
		if (res == 0x0000)
		{
			headerFourth = Read(1, &bytesRead);
			if (NULL == headerFourth || bytesRead != 1)
			{
				res = 0x0004;
			}
		}
		
		BYTE headerBuffer[4] = {0};
		
		if(res == 0x0000)
		{
			while(true)
			{
				if (*headerFirst<=0x14 && *headerSecond==0x00 && *headerFourth<=0x04)
				{
					break;
				}
				else
				{
					*headerFirst = *headerSecond;
					*headerSecond = *headerThird;
					*headerThird = *headerFourth;
					
					free(headerFourth);
					
					headerFourth = Read(1, &bytesRead);
				}
			}
			
			headerBuffer[0] = *headerFirst;
			headerBuffer[1] = *headerSecond;
			headerBuffer[2] = *headerThird;
			headerBuffer[3] = *headerFourth;
			_header = (struct Header*)headerBuffer;
		
		}
		else
		{
			_header = NULL;
		}


		if (NULL != headerFirst)
		{
			free(headerFirst);
		}
		if (NULL != headerSecond)
		{
			free(headerSecond);
		}
		if (NULL != headerThird)
		{
			free(headerThird);
		}
		if (NULL != headerFourth)
		{
			free(headerFourth);
		}
		
   	SEGGER_RTT_printf(0,"header:0x%08x\r\n",*(u32*)_header);
	
			
		if (_header != NULL)
		{
			bytesToRead = _header->_Length;
			bytesRead = 0;

			_body = Read(bytesToRead, &bytesRead);
			
			if ((_header->_Length&1) == 1)
			{
				UINT16 temp=0;
			
				_morebyte=Read(1, &temp);
				
			}
			if (_body != NULL
			  && bytesToRead == bytesRead)
			{
				if (*pLen >= _header->_Length+sizeof(struct Header))
				{
					*pLen = _header->_Length+sizeof(struct Header);
					
					memcpy(pContent, (BYTE*)_header, sizeof(struct Header));
					
					memcpy(pContent+sizeof(struct Header), _body, _header->_Length);
					
				}
				else
				{
					res = 0x0006;//接收缓冲区太小不足以接收数据
				}
			}
			else
			{
				*pLen = 0;
				res = 0x0005;//接收报文体出错
			}
		}
		else
		{
			*pLen = 0;
			res = 0x0004;//接收报文头出错			
		}
	}


	if (_body != NULL)
	{
		free(_body);
	}

	if (_morebyte != NULL)
	{
		free(_morebyte);
	}

	
	return res;
}

UINT16 setPLCMode()
{
	UINT16 res;
	BYTE *content = (BYTE*)malloc(1024);
	UINT16 length = 1024;
  extern u32 HW_ID[];
	
	res = SendGetSystemInfoRequest();
	
	if (res == 0x0000)
	{
		res = RecvDataFromSerialPort(content, &length);
	 
		if (res == 0x0000)
		{
			struct SystemInfo sysInfo;
			memset((void*)&sysInfo, 0, sizeof(struct SystemInfo));
			memcpy((void*)&sysInfo, content, length);
      			
			if (!CheckInit(sysInfo))
			{
		
				res = SendLoadSystemConfig_Port(SCI_B, SCI_B);
				if(res == 0x0000)
				{
					res = RecvDataFromSerialPort(content, &length);
					if (res == 0x0000)
					{
						struct StatusMessage statusMessage;
						memset((void*)&statusMessage, 0, sizeof(struct StatusMessage));
						memcpy((void*)&statusMessage, content, length);
 						if (statusMessage._Status == 0)
 						{
 							res = SendLoadSystemConfig_DeviceMode(0);
							if (res == 0x0000)
							{
								res = RecvDataFromSerialPort(content, &length);
								if (res == 0x0000)
								{
									struct StatusMessage statusMessage;
									memset((void*)&statusMessage, 0, sizeof(struct StatusMessage));
									memcpy((void*)&statusMessage, content, length);
 									if (statusMessage._Status == 0)
									{
										BYTE longAddr[8] = {0};
										
										memcpy(longAddr,(u8*)HW_ID,8);
										
										res = SendLoadSystemConfig_G3Config(longAddr);
										if (res == 0x0000)
										{
											res = RecvDataFromSerialPort(content, &length);
											if (res == 0x0000)
											{
												struct StatusMessage statusMessage;
												memset((void*)&statusMessage, 0, sizeof(struct StatusMessage));
												memcpy((void*)&statusMessage, content, length);
												if (statusMessage._Status == 0)
												{

												}
												else
												{
													res = 0x0009;
												}
											}
										}
									}
									else
									{
										res = 0x0008;
									}
								}
							}
 						}
						else
						{
							res = 0x0007; 
						}
					}
				}
				
			}
		}
	}	

	free(content);
	return res;
}

UINT16 setPLCTranChar()
{
	UINT16 res;
	BYTE *content = (BYTE*)malloc(1024);
	UINT16 length = 1024;
	
	res = SendShutDownMessage();
	if (res == 0x0000)
	{
		res = RecvDataFromSerialPort(content, &length);
		if (res == 0x0000)
		{
			struct StatusMessage statusMessage;
			memset((void*)&statusMessage, 0, sizeof(struct StatusMessage));
			memcpy((void*)&statusMessage, content, length);

			if (statusMessage._Status == 0)
			{
				bsp_DelayMS(1500);
				res = SendSetTxInfo();
				if (res == 0x0000)
				{
					res = RecvDataFromSerialPort(content, &length);
					if (res == 0x0000)
					{
						struct StatusMessage statusMessage;
						memset((void*)&statusMessage, 0, sizeof(struct StatusMessage));
						memcpy((void*)&statusMessage, content, length);
						
						if (statusMessage._Status == 0)
						{
							//	bsp_DelayMS(1500);
								res = SendSetRxInfo();
								if (res == 0x0000)
								{
										res = RecvDataFromSerialPort(content, &length);
										if (res == 0x0000)
										{
												struct StatusMessage statusMessage;
												memset((void*)&statusMessage, 0, sizeof(struct StatusMessage));
												memcpy((void*)&statusMessage, content, length);
												
												if(statusMessage._Status == 0)
												{
													
												}
												else
												{
													res = 0x000D;
												}
										}

								}
								else
								{
									res = 0x000C;
								}
							
						}
						else
						{
							res = 0x000B;
						}
					}
				}
			}
			else
			{
				res = 0x000A;
			}
		}			
	}	
	
	free(content);
	return res;
}

UINT16 PLCNetwork()
{
	UINT16 res = 0x0000;
	BYTE *content = (BYTE*)malloc(1024);
	UINT16 length = 1024;

// 	while(true)
// 	{
//		bsp_DelayMS(500);	
//		
// 		res = SendDiscoverNetworkRequest();
// 		if (res == 0x0000)
// 		{	
//			res = RecvDataFromSerialPort(content, &length);
// 			if(res == 0x0000)
// 			{
// 				res = RecvDiscoverNetworkConfirm(content);
// 				if (res == 0x0000)
// 				{
// 					break;
// 				}
// 				else
// 				{
// 					continue;	
// 				}
// 			}
// 		}
// 	}

  	_PAN_Id = PANID;
//	_PAN_Id = (_PAN_Id & 0xFCFF); 
	_LBA_Address = 0x0000;

	res = SendAttachRequest();
	if (res == 0x0000)
	{
		res = RecvDataFromSerialPort(content, &length);
		if (res == 0x0000)
		{
			struct AttachConfirm attachConfirm;
			memset((void*)&attachConfirm, 0, sizeof(struct AttachConfirm));
			memcpy((void*)&attachConfirm, content, length);
			if (attachConfirm._Status == 0x0000)
			{
				g_init = true;
				_PAN_Id = attachConfirm._PAN_Id;
				_LBA_Address = attachConfirm._NetworkAddress;
				genIpv6Addr[8] = _PAN_Id / 256;
				genIpv6Addr[9] = _PAN_Id % 256;
				genIpv6Addr[14] = _LBA_Address / 256;
				genIpv6Addr[15] = _LBA_Address % 256;			
				//printf("attach success: id=0x%04X, shortAddr=0x%04X\n", attachConfirm._PAN_Id, attachConfirm._NetworkAddress);
			}
			else
			{
				res = 0x000D;
			}
		}
	}
	
	free(content);
	return res;
}

UINT16 initPLCModem()
{
	UINT16 res = 0x0000;
	
  USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);
	
	bsp_DelayMS(1000);
	
	res = setPLCMode();
	// SEGGER_RTT_printf(0,"PLC init:%s:%d\r\n", __FILE__,__LINE__);
	if (res == 0x0000)
	{
		res = setPLCTranChar();
		
		if (res == 0x0000)
		{
			res = PLCNetwork();
			
		}
	}
	return res;
}

