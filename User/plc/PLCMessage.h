#ifndef __PLCMESSAGE_H_
#define __PLCMESSAGE_H_

#include<stdbool.h>
#define MAX_ERRINFO_LENGTH 512
typedef unsigned char BYTE;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned char UINT8;
typedef short INT16;
typedef int INT32;

#pragma pack(1)

enum ToneMaskId
{
	Cenelec_A_36 = 0,
	Cenelec_A_25,
	Cenelec_B,
	Cenelec_BC,
	Cenelec_BCD,
	FCC_Low_Band,
	FCC_High_Band,
	FCC_Full_Band
};

enum MessageIds
{
	Data_Transfer_Id = 0x00,
	Get_System_Info_Id = 0x01,
	Get_PHY_PIB_Id = 0x02,
	Get_MAC_PIB_Id = 0x03,
	Set_Info_Id = 0x04,
	Shut_Down_Id = 0x05,
	Setup_Alarm_Id = 0x06,
	Alarm_Id = 0x07,
	Network_Register_Id = 0x08,
	Network_Start_Id = 0x08,
	Network_Unregister_Id = 0x09,
	Connect_Id = 0x0A,
	Disconnect_Id = 0x0B, 
	Load_System_Config_Id = 0x0C,
	Set_MAC_PIB_Id = 0x0D,
	Clear_PHY_PIB_Id = 0x0E,
	Clear_MAC_PIB_Id = 0x0F,
	Attach_Id = 0x10,
	Detach_Id = 0x11,
	Discover_Id = 0x12,
	Firmware_UpGrade_Id = 0x13
};

enum ResetType
{
	SoftReset = 0x0000,
	SoftShutdown = 0x0001
};

enum DiscoverType
{
	NetworkDiscover = 0x00,
	RouteDiscover = 0x01,
	PathDiscover = 0x02,
	ExhaustiveBeaconDiscover = 0x03
};

enum Ports
{
	SCI_A = 0,
	SCI_B = 1
};

enum CurCmd
{
	GetSystemInfoRequestCmd = 0x01,
	LoadSystemConfig_PortCmd = 0x02,
	LoadSystemConfig_DeviceModeCmd = 0x03,
	LoadSystemConfig_G3ConfigCmd = 0x04,
	ShutDownMessageCmd = 0x05,
	DiscoverNetworkRequestCmd = 0x06,
	AttachRequestCmd = 0x07,
	DetachRequestCmd = 0x08,
	DataTransferRequestCmd = 0x09,
	SetTxInfoCmd = 0x0A,
	SetRxInfoCmd = 0x0B
};

struct Header
{
	BYTE _Id;			//消息种类
	BYTE _Origin;		//7(ORG:1 host to PLC; 0 PLC to host)  6(RPY确认回复位。如果设置此位，消息接受者需要返回确认信息（返回消息头，重置CRC或者设置CRC为0，设置RPY，无消息体）。此位只用于DATA_TRANSFER.indicate) 5~4(RESV保留) 3~0(SEQ序列号)
	UINT16 _Length;     //16位消息长度。消息体长度+消息头CRC长度（2字节）+消息体CRC长度（2字节）（无消息体的消息长度为4）。支持的最大消息体长度为1500字节，即最大消息长为1504字节。
};

struct CRC_Header
{
	UINT16 _HeaderCRC;  //消息头的CRC16数值。从Message Type开始到Message PayLoad Length。不包含消息头CRC16和消息体的CRC16。0代表不提供CRC16
	UINT16 _MessageCRC; //Message PayLoad CRC16：消息体的CRC16
};

struct GetSystemInfoRequest
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
};

struct SystemInfo
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	BYTE _Build;
	BYTE _Revision;
	BYTE _Minor;
	BYTE _Major;
	
	BYTE _Reserved_0[18];
	///	BYTE _SerialNumber[16];
	BYTE _DeviceType;
	BYTE _DeviceMode;
	UINT16 _HardwareRevision;
	//	BYTE _EUI[6];
	BYTE Reserved_1[6];
	BYTE _PortAssigments;
	//	BYTE _Flag0;
	//	UINT16 _MAC_RX_Max_Hold_PPDU;
	//	UINT16 _PRIME_MAC_Max_Conns;
	//	UINT16 _MAC_Max_RX_Q_Length;
	BYTE _Reserved_2[7];
	BYTE _TMR_COH_Flags;
	//	BYTE _SecurityProfile;
	
	//	UINT16 _AddressType;
	BYTE _Reserved_3[3];
	BYTE _LongAddress[8];
	//	UINT16 _PanId;
	//	UINT16 _ShortAddress;
	//	UINT16 _SegmentLength;
	BYTE _Reserved_4[6];
};

struct PortDesignation
{
	UINT16 _Type;		// 0x0001
	UINT16 _Length; // 1
	BYTE _Port;
};

struct SystemConfig
{
	UINT16 _Type;		// 0x0003
	UINT16 _Length; // 26
	BYTE _Reserved_0[18 + 6];
	//UINT16 _SerialNumberLength;  // Not used by G3
	//BYTE _SerialNumber[16];
	//BYTE _EUI[6];
	BYTE _DeviceMode;
	BYTE _Reserved_1;
};

struct G3Configuration
{
	UINT16 _Type;		//0x0008
	UINT16 _Length;	//16
	BYTE _Reserved_0[2];
	BYTE _LongAddress[8];
	BYTE _Reserved_1[4];
	UINT16 _MAC_SegmentSize;  // obsolete for new dsp s/w but needs to be set to 239 for older versions.
};

struct LoadSystemConfig
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	struct PortDesignation * _Port;
	struct SystemConfig * _SystemConfig;
	struct G3Configuration * _G3_Config;
};

struct StatusMessage
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	UINT16 _Status;
};

struct ShutDownMessage
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	UINT16 _ResetType;
};

struct DiscoverNetworkRequest
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	BYTE _DiscoverType;
	BYTE _Duration;			
};

struct PAN_Description
{
	BYTE _CoordAddressMode;
	BYTE _LQI;
	UINT16 _Coord_PAN_Address;
	UINT16 _ShortAddress;
	UINT16 _RoutingCost;
};

struct DiscoverNetworkConfirm
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	BYTE _DiscoverType;
	BYTE _Reserved;	
	UINT16 _Status;
	UINT16 _PAN_Count;
	//
	// There can be more than one set of PAN Description
	// We will just allocate extra memory and create an 
	// array to access
	//
	struct PAN_Description _PAN_Info[];
};

struct AttachRequest
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	UINT16 _PAN_Id;
	UINT16 _LBA_Address;  // short address..
};

struct AttachConfirm
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	UINT16 _Status;
	UINT16 _NetworkAddress; 
	UINT16 _PAN_Id;
};

struct DetachRequest
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	BYTE _ExtendedAddress[8];
};

struct DetachConfirm
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	UINT16 _Status;
	BYTE _ExtendedAddress[8];
};

struct IPv6
{
	BYTE   _Header[4];      //版本号(4位):流量类型(8位):流标签(20位)
	UINT16 _PayloadLength;  // = UPD Length 负载长度(16位)
	UINT8  _NextHeader;		// = 17 - udp message
	UINT8  _HopLimit;		//跳数限制
	
	BYTE   _SourceAddress[16];
	BYTE   _DestinationAddress[16];
};

// variable length struct
struct UDP
{
	UINT16 _SourcePort;
	UINT16 _DestinationPort;
	UINT16 _Length;   // includes data and this header, min size is 8.
	UINT16 _CRC;			// aways set to zero for now
	BYTE  _Data[];
};

struct DataTransferRequest
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	BYTE _NSDU_Handle;
	BYTE _Flags;
	//BYTE _L_SDU_Data[1];
	struct IPv6 _IPv6_Header;
	struct UDP _UDP_Message;
};

struct DataTransfer
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	UINT16 _Spacer;
	
	struct IPv6 _IPv6_Header;
	struct UDP _UDP_Message;
};

struct DataTransferConfirm
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	BYTE _NSDU_Handle;
	BYTE _ConfirmFlag;
	UINT16 _Status;
};

struct DataTransferIndication
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	BYTE _LQI;
	BYTE _Flags;
	//BYTE Data Payload[1];
	struct IPv6 _IPv6_Header;
	struct UDP _UDP_Message;
};

struct SetInfo_TX
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	UINT16 _InfoType; // 0x0002
	UINT16 _InfoLength; // 16/18 depending on G3 version (5.0 and greater)
	BYTE _Flags;			// Band select, coh, tmr
	BYTE _Modulation;
	UINT16 _TX_Level;
	BYTE _ToneMask[14];
};

struct SetInfo_RX
{
	struct Header _Header;
	struct CRC_Header _CRC_Header;
	
	UINT16 _InfoType; // 0x0003
	UINT16 _InfoLength; // 16/18 depending on G3 version (5.0 and greater)
	BYTE _Reserved;
	BYTE _Flags;				//Band select, coh, bli, agc
	UINT16 _GainValue;
	BYTE _ToneMask[14];
};

extern BYTE g_errInfo[MAX_ERRINFO_LENGTH];
extern int g_curCmd;
extern UINT16 _PAN_Id;
extern UINT16 _LBA_Address;
extern bool g_init;
extern BYTE genIpv6Addr[16];
extern BYTE ToneMasks[13][14];

void SetErrInfo(BYTE * errInfo);


bool CheckInit(struct SystemInfo sysInfo);


bool RecvMessage(struct Header *_header, BYTE* _body, void* content);




bool SendDetachRequest(BYTE * longAddr);

bool SendData(BYTE* data, int length);
bool RecvDataTransferConfirm(struct Header *_header, BYTE* _body);


UINT16 SendSetRxInfo(void);


UINT16 SendGetSystemInfoRequest(void);

UINT16 SendLoadSystemConfig_Port(int dataPort, int diagPort);
UINT16 SendLoadSystemConfig_DeviceMode(int deviceMode);
UINT16 SendLoadSystemConfig_G3Config(BYTE* longAddress);

UINT16 SendShutDownMessage(void);
UINT16 SendSetTxInfo(void);
UINT16 SendDiscoverNetworkRequest(void);

UINT16 getDataFromPLCs(BYTE* plcData, UINT16 plcDataLen, INT32 *moreLen, BYTE* orgData, UINT16* orgDataLen);
UINT16 getDataFromPLC(BYTE* plcData, UINT16 plcDataLen, BYTE* orgData, UINT16* orgDataLen);
UINT16 genPLCData(BYTE* orgData, UINT16 orgDataLen, BYTE* plcData, UINT16* plcDataLen);

UINT16 RecvDataFromSerialPort(BYTE* pContent, UINT16* pLen);
UINT16 RecvDiscoverNetworkConfirm(BYTE* pContent);
UINT16 SendAttachRequest(void);

UINT16 initPLCModem(void);
UINT16 setPLCMode(void);
UINT16 setPLCTranChar(void);
UINT16 PLCNetwork(void);

#endif




