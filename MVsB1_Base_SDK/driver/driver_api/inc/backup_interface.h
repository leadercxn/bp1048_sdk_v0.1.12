/**
 **************************************************************************************
 * @file    backup_interface.h
 * @brief   backup interface
 *
 * @author  Sam
 * @version V1.1.0
 *
 * $Created: 2018-06-05 15:17:05$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef		__BACKUP_INTERFACE_H__
#define		__BACKUP_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

#ifndef CFG_APP_CONFIG
#define POWERKEY_CNT		2000
#endif

typedef enum _POWERKEY_CLOCK
{
	BACKUP_CLK_32K_OSC = 0,
	BACKUP_CLK_32K_RC32,
	BACKUP_CLK_32K_XM
}BACKUP_CLOCK;


#define		POWERKEY_MODE_BYPASS 				0
#define		POWERKEY_MODE_PUSH_BUTTON			1
#define		POWERKEY_MODE_SLIDE_SWITCH_LPD 		2//硬开关高唤醒
#define		POWERKEY_MODE_SLIDE_SWITCH_HPD		3//硬开关低唤醒


void Backup_Clock(BACKUP_CLOCK Clock_Sel);

void Backup_PowerDown(void);

void SystemPowerKeyInit(uint32_t Mode, uint16_t CountTime);

bool SystemPowerKeyDetect(void);

void SystemPowerDown(void);

void PowerKeyModeGet(void);

void BACKUP_RTCSet(uint32_t rtc_time_set,uint32_t wakeup_s);

uint32_t BACKUP_GetConfig(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__BACKUP_INTERFACE_H__

