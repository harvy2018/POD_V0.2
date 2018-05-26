
#ifndef _GIMBAL_H_
#define _GIMBAL_H_

#include "stm32f10x.h"	

#define ANGLE_DEGREE_SCALE 0.02197265625
#define SPEED_SCALE 0.1220740379
#define SPEED 100


#define PREAMBLE 0x3E
#define CMD_GET_ANGLES_EXT  61
#define CMD_CONTROL   67
#define CMD_CONFIRM   67

#define CMD_CALIB_MAG 59
#define CMD_CALIB_ACC  65
#define CMD_CALIB_GYRO  103


#define MODE_NO_CONTROL  0
#define MODE_SPEED 1
#define MODE_ANGLE 2
#define MODE_SPEED_ANGLE 3

#pragma pack(1)

typedef enum _CALIB_STATE
{
    CALBI_INIT=0,
    CALBI_GYRO,
    CALBI_ACC,
    CALBI_MAG,
    CALBI_MAX
    
}CALIB_STATE;

typedef struct _GIMBAL_SERIAL_HEADER
{
    u8 Preamble;
    u8 CmdID;
    u8 DataSize;
    u8 HeaderCC;   
    
}GIMBAL_SERIAL_HEADER;

typedef struct _ANGLES_EXT_DATA
{
   s16 ROLL_IMU_ANGLE;
   s16 ROLL_RC_TARGET_ANGLE;
   s32 ROLL_STATOR_ROTOR_ANGLE;
//    u8 ROLL_IMUAngle_NegFalg;
//	u8 ROLL_SRAngle_NegFalg;
   u8  RESERVED_1[10];  
   
   s16 PITCH_IMU_ANGLE;
   s16 PITCH_RC_TARGET_ANGLE;
   s32 PITCH_STATOR_ROTOR_ANGLE;
//	u8 PITCH_IMUAngle_NegFalg;
//	u8 PITCH_SRAngle_NegFalg;
   u8  RESERVED_2[10];
    
   s16 YAW_IMU_ANGLE;
   s16 YAW_RC_TARGET_ANGLE;
   s32 YAW_STATOR_ROTOR_ANGLE;
//	u8 YAW_IMUAngle_NegFalg;
//	u8 YAW_SRAngle_NegFalg;
   u8  RESERVED_3[10];
     
}ANGLES_EXT_DATA;

typedef struct _CMD_CONTROL_DATA
{
   u8  CONTROL_MODE;
   s16 SPEED_ROLL;
   s16 ANGLE_ROLL;
   s16 SPEED_PITCH;
   s16 ANGLE_PITCH;
   s16 SPEED_YAW;
   s16 ANGLE_YAW;
     
}CMD_CONTROL_DATA;


void GimbalDataProcess(void);

void GetAngle(void);
void UploadAngle(void);
void GimbalAngleSet(s16 pitch,s16 yaw);
void GimbalGoZero(void);
void MicroModify(s16 diffPitch,s16 diffYaw);
void Calib_POD(void);

void MicroModify_Quit(void);
#endif