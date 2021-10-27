



#ifndef __BT_DDB_FLASH_H__
#define __BT_DDB_FLASH_H__

//#include "bt_stack_types.h"
#include "bt_manager.h"

#define DDB_NOT_FOUND  0xffff

/**
 * @brief  open bt database
 * @param  localBdAddr - bt device address
 * @return offset
 * @Note 
 *
 */
uint8_t BtDdb_Open(const uint8_t * localBdAddr);

/**
 * @brief  close bt database
 * @param  NONE
 * @return TRUE - success
 * @Note 
 *
 */
bool BtDdb_Close(void);

/**
 * @brief  add one record to bt database
 * @param  record - the structure pointer(BT_DB_RECORD)
 * @return TRUE - success
 * @Note 
 *
 */
bool BtDdb_AddOneRecord(const BT_DB_RECORD * record);

/**
 * @brief  clear bt database area
 * @param  NONE
 * @return TRUE - success
 * @Note 
 *
 */
bool BtDdb_Erase(void);


int8_t BtDdb_LoadBtConfigurationParams(BT_CONFIGURATION_PARAMS *params);

int8_t BtDdb_SaveBtConfigurationParams(BT_CONFIGURATION_PARAMS *params);

void DdbDeleteRecord(uint8_t index);

uint32_t DdbFindRecord(const uint8_t *bdAddr);

bool BtDdb_GetLastBtAddr(uint8_t *BtLastAddr);

bool BtDdb_UpgradeLastBtAddr(uint8_t *BtLastAddr);

void BtDdb_LastBtAddrErase(void);

#endif //__BT_DDB_FLASH_H__

