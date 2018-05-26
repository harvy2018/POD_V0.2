#ifndef _CAM_H_
#define _CAM_H_

#include "stm32f10x.h"	



void Cam_Operation(u16 cmd,u16 para);
void CamDataProcess();
u8 GetOpticalZoom(u16 pqrs);
void UploadOpticalZoom(u8 OpticalZoom);
void CamCmdSend(u8 *cmdbuf,u8 len);
void CamZoomPosQry(void);
#endif