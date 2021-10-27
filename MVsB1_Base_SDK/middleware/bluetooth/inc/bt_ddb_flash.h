



#ifndef __BT_DDB_FLASH_H__
#define __BT_DDB_FLASH_H__
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
//#include "bt_stack_types.h"
#include "bt_manager.h"

#define DDB_NOT_FOUND  0xffff

//������������λ��:size:4KB(0x1ff000~0x1fffff)
#define BTDB_CONFIGDATA_START_ADDR		BT_CONFIG_ADDR

//�������ñ�:0x1ff000-0x1fffff(4K)
#define BTDB_CONFIG_ADDR				BTDB_CONFIGDATA_START_ADDR
#define BTDB_CONFIG_MEM_SIZE			(4*1024)

//���������豸��ַ����λ��:size:12KB(0x1fb000~0x1fdfff)
#define BTDB_USERDATA_START_ADDR		BT_DATA_ADDR

//����豸��Ϣ:0x1fb000-0x1fcfff(8K)
#define BTDB_TOTAL_RECORD_ADDR			(BTDB_USERDATA_START_ADDR)
#define BTDB_TOTAL_RECORD_MEM_SIZE		(8*1024)

//���1������豸��Ϣ:0x1fd000-0x1fdfff(4K)
#define BTDB_ALIVE_RECORD_ADDR			(BTDB_TOTAL_RECORD_ADDR+BTDB_TOTAL_RECORD_MEM_SIZE)
#define BTDB_ALIVE_RECORD_MEM_SIZE		(4*1024)

#define BTDB_NVM_RECORD_CNT				((BTDB_ALIVE_RECORD_MEM_SIZE+BTDB_TOTAL_RECORD_MEM_SIZE)/4096)
#define BTDB_NVM_SECTOR_SIZE			(4096)

//BT RECORD INFOR
#define MVBT_DB_FLAG						"MVBT"
#define MVBT_DB_FLAG_SIZE					4

//���1������豸��¼������Թ���8���豸��Ѱ��
//�������1����Լ�¼�б���ʹ��
//BT LAST ADDR
#define MVBT_DB_LAST_FLAG					"MVBT"
#define MVBT_DB_LAST_FLAG_SIZE				4
#define BTDB_LAST_RECORD_SIZE				(4+6+1)	//flag+addr+profile-flag

//��������ӹ����豸��Ϣ
#ifdef BT_TWS_SUPPORT
	#define MAX_BT_DEVICE_NUM					9//8 //(tws1+device8)
	#define BT_REC_INFO_LEN						65//(6+16+1+1+1) + 40 = (addr+linkkey+flag+role+profile) + remotename
	static uint32_t FlshGetPairingInfoWriteOffset(uint32_t StartOffset, uint32_t Step);
#else
	#define MAX_BT_DEVICE_NUM					8
	#define BT_REC_INFO_LEN						63//(6+16+1) + 40 = (addr+linkkey+flag) + remotename
#endif

/*extern uint8_t KeyEnc;*/
//#define PRINT_RECORD_INFOR

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

int8_t BtDdb_InitBtConfigurationParams(BT_CONFIGURATION_PARAMS *params);

void DdbDeleteRecord(uint8_t index);

uint32_t DdbFindRecord(const uint8_t *bdAddr);
#ifdef BT_TWS_SUPPORT
bool BtDdb_GetLastBtAddr(uint8_t *BtLastAddr, uint8_t* profile);
void BtDdb_GetTwDeviceLinkKey(uint8_t *LinkKey);
bool BtDdb_ClearTwsDeviceAddrList(void);
void BtDdb_ClrTwsDevInfor(void);
bool BtDdb_GetTwsDeviceAddr(uint8_t *BtTwsAddr);
uint32_t BtDdb_UpgradeTwsInfor(uint8_t *BtTwsAddr);

//���tws infor ��ؼĴ����ļ�¼,�����flash������
bool BtDdb_ClearTwsDeviceRecord(void);

#else
bool BtDdb_GetLastBtAddr(uint8_t *BtLastAddr);
#endif
bool BtDdb_UpgradeLastBtAddr(uint8_t *BtLastAddr, uint8_t BtLastProfile);

bool BtDdb_UpgradeLastBtProfile(uint8_t *BtLastAddr, uint8_t BtLastProfile);

void BtDdb_LastBtAddrErase(void);

void BtDdb_PrintRecord(void);

void SaveTotalDevRec2Flash(int OneFullRecBlockSize, int TotalRecNum);

#endif //__BT_DDB_FLASH_H__

