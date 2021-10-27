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
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

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
	
#ifdef BT_TWS_SUPPORT
	GetBtManager()->btReconnectDeviceFlag = RECONNECT_NONE;
	/*
	* Get the last BtAddr and ready to connect
	*/
	BtDdb_GetLastBtAddr(GetBtManager()->btDdbLastAddr, &GetBtManager()->btDdbLastProfile);
	if(((GetBtManager()->btDdbLastAddr[0]==0)
		&&(GetBtManager()->btDdbLastAddr[1]==0)
		&&(GetBtManager()->btDdbLastAddr[2]==0)
		&&(GetBtManager()->btDdbLastAddr[3]==0)
		&&(GetBtManager()->btDdbLastAddr[4]==0)
		&&(GetBtManager()->btDdbLastAddr[5]==0))
		||
		((GetBtManager()->btDdbLastAddr[0]==0xff)
		&&(GetBtManager()->btDdbLastAddr[1]==0xff)
		&&(GetBtManager()->btDdbLastAddr[2]==0xff)
		&&(GetBtManager()->btDdbLastAddr[3]==0xff)
		&&(GetBtManager()->btDdbLastAddr[4]==0xff)
		&&(GetBtManager()->btDdbLastAddr[5]==0xff)))
	{
		
	}
	else
	{
		GetBtManager()->btReconnectDeviceFlag |= RECONNECT_DEVICE;

		APP_DBG("last device addr %x:%x:%x:%x:%x:%x, profile:0x%02x\n", GetBtManager()->btDdbLastAddr[0],GetBtManager()->btDdbLastAddr[1],GetBtManager()->btDdbLastAddr[2],
			GetBtManager()->btDdbLastAddr[3],GetBtManager()->btDdbLastAddr[4],GetBtManager()->btDdbLastAddr[5], GetBtManager()->btDdbLastProfile);
	}

	GetBtManager()->twsFlag = BtDdb_GetTwsDeviceAddr(GetBtManager()->btTwsDeviceAddr);

	/*
	* Get the tws BtAddr and ready to connect
	*/
	if(!GetBtManager()->twsFlag)
	{
		APP_DBG("no tws paired list\n");
		GetBtManager()->twsRole = BT_TWS_UNKNOW;
	}
	else
	{
		APP_DBG("tws role: %d\n", GetBtManager()->twsRole);

	#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
		extern void ble_advertisement_data_update(void);
		ble_advertisement_data_update();
	#endif

		/*if(GetBtManager()->twsRole == BT_TWS_SLAVE)
		{
		#ifdef BT_TWS_POWERON_RECONNECTION
			GetBtManager()->btReconnectDeviceFlag |= RECONNECT_TWS;
		#endif
		}
		else
		{
		#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
			extern void ble_advertisement_data_update(void);
			ble_advertisement_data_update();
		#endif
		}*/
	}
#else
	/*
	* Get the last BtAddr and ready to connect
	*/
	BtDdb_GetLastBtAddr(GetBtManager()->btDdbLastAddr);
#endif
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



