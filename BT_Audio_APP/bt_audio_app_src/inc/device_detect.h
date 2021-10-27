/**
 **************************************************************************************
 * @file    device_detect.h
 * @brief
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-1-5 11:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __DEVICE_DETECT_H__
#define __DEVICE_DETECT_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "timeout.h"

typedef enum _PlugEvent
{
	DEVICE_DETECT_NONE,
	DEVICE_DETECT_PLUGIN_JITTER,
	DEVICE_DETECT_PLUGIN,
	DEVICE_DETECT_PLUGOUT_JITTER,
	DEVICE_DETECT_PLUGOUT,
}PlugEvent;

#define	LINEIN_STATE_BIT		0x01
#define	LINEIN_EVENT_BIT		0x02
#define CARDIN_STATE_BIT		0x04
#define CARDIN_EVENT_BIT		0x08
#define UDISKIN_STATE_BIT		0x10
#define UDISKIN_EVENT_BIT		0x20
#define USB_DEVICE_STATE_BIT	0x40
#define USB_DEVICE_EVENT_BIT	0x80
#define HDMI_HPD_STATE_BIT		0x100
#define HDMI_HPD_EVENT_BIT		0x200

extern uint8_t sDevice_Inserted_Flag;

#ifdef HDMI_HPD_CHECK_DETECT_EN
extern TIMER	HDMIExitTimer;
extern uint8_t  HDMIExitFlg;
#endif

//#ifdef CFG_FUNC_UDISK_DETECT
bool IsUDiskLink(void);
//#endif
bool IsCardIn(void);
uint32_t DeviceDetect(void);
// 上电时，硬件扫描消抖
void InitDeviceDetect(void);
void SetDeviceDetectVarReset(void);
void SetDeviceDetectVarReset_Deepsleep(void);
void SetDeviceInitState(uint32_t init_state);
uint32_t GetDeviceInitState(void);
void SetUSBAudioExitFlag(uint8_t flag);
uint8_t GetUSBAudioExitFlag(void);
void UDiskRemovedFlagSet(bool State);

#endif //__DEV_DETECT_H__
