/**
 **************************************************************************************
 * @file    platform_interface.h
 * @brief   platfomr interface
 *
 * @author  Halley
 * @version V1.1.0
 *
 * $Created: 2016-07-22 16:24:11$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

/**
* @addtogroup Bluetooth
* @{
* @defgroup platform_interface platform_interface.h
* @{
*/


#ifndef __BT_PLATFORM_INTERFACE_H__
#define __BT_PLATFORM_INTERFACE_H__

#include "bt_manager.h"

typedef struct _PLATFORM_INTERFACE_OS_T
{
	void*	(*osMalloc)(uint32_t size);
	void	(*osFree)(void* ptr);

	void*	(*osMemcpy)(void* dest, const void * src, uint32_t size);
	void*	(*osMemset)(void * dest, uint8_t byte, uint32_t size);
}PLATFORM_INTERFACE_OS_T;

PLATFORM_INTERFACE_OS_T		pfiOS;

typedef struct _PLATFORM_INTERFACE_BT_DDB_T
{
	bool	(*OpenBtRecord)(const uint8_t * localBdAddr);
	bool	(*CloseBtRecord)(void);
	bool	(*AddBtRecord)(const BT_DB_RECORD * btDbRecord);
	bool	(*DeleteBtRecord)(const uint8_t * bdAddr);
	bool	(*FindBtRecord)(const uint8_t * bdAddr, BT_DB_RECORD * btDbRecord);
	bool	(*FindBtRecordByIndex)(uint8_t index, BT_DB_RECORD * btDbRecord);
}PLATFORM_INTERFACE_BT_DDB_T;

PLATFORM_INTERFACE_BT_DDB_T	pfiBtDdb;

void SetBtPlatformInterface(PLATFORM_INTERFACE_OS_T * pfiOS,
							PLATFORM_INTERFACE_BT_DDB_T * pfiBtDdb
							);


#endif //__PLATFORM_INTERFACE_H__
