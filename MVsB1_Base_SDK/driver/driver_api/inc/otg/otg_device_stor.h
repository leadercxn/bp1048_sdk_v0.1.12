/**
 *****************************************************************************
 * @file     otg_device_stor.h
 * @author   Owen
 * @version  V1.0.0
 * @date     24-June-2015
 * @brief    stor device interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#ifndef __OTG_DEVICE_STOR_H__
#define	__OTG_DEVICE_STOR_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
	
#include "type.h"

void OTG_DeviceStorInit(void);
void OTG_DeviceStorProcess(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
