/**********************************************************************
File name: errorCode.h
Date: 2016-8-24
Author: Wangwei
Description:
    Define all the error code system applied  here.
**********************************************************************/
#ifndef	_ERROR_CODE
#define	_ERROR_CODE

typedef enum _errorCode
{
    E_SUCCESS=0,                // Succeed
    E_FAIL,                         // Commonly fails
    E_NO_MEMORY,			// Fails due to no enough memory
    E_EMPTY,				// Fails due to buffer empty
    E_FULL,                         // Buffer is full
    E_NOT_SUPPORT,		// The command is not support yet
    E_CHECK_CODE,			// Checksum/CRC check error
    E_MESSAGE_TYPE,         // Message type error
    E_AMBIGUITY,                // Ambiguity,Need further processing
    E_INPUT_PARA,			// Input parameter is/are wrong
    E_DEVICE_BUSY,			// Device is busy
    E_WAITING,                         //  Normal waiting
    E_WAIT_CONFIRM,
    E_SHORT_FRAME,		// Frame is broken
    E_LONG_FRAME,			// Frame is long
    E_UNKNOWN_MESSAGE,	// Unknown message
    E_PLC_G3_CONFIG,            // PLC G3 config 
    E_PLC_DEVICE_MODE,      // PLC Device mode
    E_PLC_PORT_CONFIG,      // PLC port config
    E_PLC_SET_RX_INFO,
    E_PLC_SET_TX_INFO,
    E_PLC_SET_SHUTDOWN,
    E_PLC_ATTACH,
    E_PROTOCOL_HEADER,      // Protocol header error
    E_PROTOCOL_BODY,          // Protocol body error           
    E_PROTOCOL_SHORT,        // Protocal short frame
    E_PROTOCOL_LONG,          // Protocol long frame
    E_PROTOCOL_CC,              // Protocol check code
    E_GUN_INITIALIZE,
    E_MAX
} errorCode;
#endif
