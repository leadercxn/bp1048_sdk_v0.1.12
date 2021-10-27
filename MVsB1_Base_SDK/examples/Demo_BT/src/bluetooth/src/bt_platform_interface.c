/**
 **************************************************************************************
 * @file    platform_interface.c
 * @brief   platform interface
 *
 * @author  Halley
 * @version V1.1.0
 *
 * $Created: 2016-07-22 16:24:11$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#include "type.h"
#include "debug.h"

#if FUNC_OS_EN
#include "rtos_api.h"
#endif

#include "bt_stack_api.h"
#include "bt_platform_interface.h"
#include "bt_config.h"
#include "bt_manager.h"

#include "bt_app_interface.h"
#include "bt_ddb_flash.h"

//OS
static void * osMalloc(uint32_t size);
static void osFree(void * ptr);

//DDB
//
static bool OpenBtRecord(const uint8_t * localBdAddr);
static bool CloseBtRecord(void);
static bool AddBtRecord(const BT_DB_RECORD * btDbRecord);
static bool DeleteBtRecord(const uint8_t * bdAddr);
static bool FindBtRecord(const uint8_t * bdAddr, BT_DB_RECORD * btDbRecord);
static bool FindBtRecordByIndex(uint8_t index, BT_DB_RECORD * btDbRecord);

/*****************************************************************************
 *OS
 ****************************************************************************/
PLATFORM_INTERFACE_OS_T		pfiOS = {
	osMalloc,
	osFree,
	NULL,
	NULL
};


#if FUNC_OS_EN
static void * osMalloc(uint32_t size)
{
	return osPortMalloc(size);
}

static void osFree(void * ptr)
{
	osPortFree(ptr);
}
#else
static void * osMalloc(uint32_t size)
{
	return NULL;
}

static void osFree(void * ptr)
{
	;
}
#endif
/*****************************************************************************
 *DDB
 ****************************************************************************/
PLATFORM_INTERFACE_BT_DDB_T	pfiBtDdb = {
		OpenBtRecord,
		CloseBtRecord,
		AddBtRecord,
		DeleteBtRecord,
		FindBtRecord,
		FindBtRecordByIndex
};

/*---------------------------------------------------------------------------
 *            OpenBtRecord()
 *---------------------------------------------------------------------------
 */
static bool OpenBtRecord(const uint8_t * localBdAddr)
{
	//load bt ddb record
	BtDdb_Open(localBdAddr);

	/*
	* Get the last BtAddr and ready to connect
	*/
	BtDdb_GetLastBtAddr(GetBtManager()->btDdbLastAddr);
	return TRUE;
}

/*---------------------------------------------------------------------------
 *            CloseBtRecord()
 *---------------------------------------------------------------------------
 */
static bool CloseBtRecord(void)
{
	return TRUE;
}

/*---------------------------------------------------------------------------
 *            AddBtRecord()
 *---------------------------------------------------------------------------
 */
static bool AddBtRecord(const BT_DB_RECORD * btDbRecord)
{
	BtDdb_AddOneRecord(btDbRecord);
	return TRUE;
}

/*---------------------------------------------------------------------------
 *            DeleteBtRecord()
 *---------------------------------------------------------------------------
 */
static bool DeleteBtRecord(const uint8_t * remoteBdAddr)
{
	uint32_t count;
	
	count = DdbFindRecord(remoteBdAddr);
	
	if (count != DDB_NOT_FOUND) 
	{
		DdbDeleteRecord(count);
		return TRUE;
	}
	return TRUE;
}


/*---------------------------------------------------------------------------
 *            FindBtRecord()
 *---------------------------------------------------------------------------
 */
static bool FindBtRecord(const uint8_t * remoteBdAddr, BT_DB_RECORD * btDbRecord)
{
	uint32_t count;
	
	count = DdbFindRecord(remoteBdAddr);

	if (count != DDB_NOT_FOUND) 
	{
		*btDbRecord = btManager.btLinkDeviceInfo[count].device;
		return TRUE;
	}

	return FALSE;
}

/*---------------------------------------------------------------------------
 *            FindBtRecordByIndex()
 *---------------------------------------------------------------------------
*/
static bool FindBtRecordByIndex(uint8_t index, BT_DB_RECORD * btDbRecord)
{
	return FALSE;
}



