

#include "bt_ddb_flash.h"

#include <string.h>

#include "type.h"
#include "spi_flash.h"
#include "debug.h"
#include "flash_config.h"



//蓝牙参数保存位置:size:4KB(0x1ff000~0x1fffff)
#define BTDB_CONFIGDATA_START_ADDR		BT_CONFIG_ADDR

//参数配置表:0x1ff000-0x1fffff(4K)
#define BTDB_CONFIG_ADDR				BTDB_CONFIGDATA_START_ADDR
#define BTDB_CONFIG_MEM_SIZE			(4*1024)

//蓝牙连接设备地址保存位置:size:12KB(0x1fb000~0x1fdfff)
#define BTDB_USERDATA_START_ADDR		BT_DATA_ADDR

//配对设备信息:0x1fb000-0x1fcfff(8K)
#define BTDB_TOTAL_RECORD_ADDR			(BTDB_USERDATA_START_ADDR)
#define BTDB_TOTAL_RECORD_MEM_SIZE		(8*1024)

//最后1次配对设备信息:0x1fd000-0x1fdfff(4K)
#define BTDB_ALIVE_RECORD_ADDR			(BTDB_TOTAL_RECORD_ADDR+BTDB_TOTAL_RECORD_MEM_SIZE)
#define BTDB_ALIVE_RECORD_MEM_SIZE		(4*1024)

#define BTDB_NVM_RECORD_CNT				((BTDB_ALIVE_RECORD_MEM_SIZE+BTDB_TOTAL_RECORD_MEM_SIZE)/4096)
#define BTDB_NVM_SECTOR_SIZE			(4096)

//BT RECORD INFOR
#define MVBT_DB_FLAG						"MVBT"
#define MVBT_DB_FLAG_SIZE					4

//BT LAST ADDR
#define MVBT_DB_LAST_FLAG					"MVBT"
#define MVBT_DB_LAST_FLAG_SIZE				4
#define BTDB_LAST_RECORD_SIZE				(4+6)	//flag+addr

//保存的连接过的设备信息
#define MAX_BT_DEVICE_NUM					8
#define BT_REC_INFO_LEN						23//(6+16+1) = (addr+linkkey+flag)


#define PRINT_RECORD_INFOR

static void SaveTotalDevRec2Flash(int OneFullRecBlockSize, int TotalRecNum);


/**********************************************************************************************/
/* 打印相应的信息 */
/**********************************************************************************************/
#ifdef PRINT_RECORD_INFOR
static void PrintRecordInfor(const BT_DB_RECORD * record)
{
	uint8_t len=0;
	BT_DBG("---\n");
	BT_DBG("RemoteAddr:");
	for(len=0; len<6; len++)
	{
		BT_DBG("%02x ",record->bdAddr[len]);
	}
	BT_DBG("\nlinkkey:");
	for(len=0; len<16; len++)
	{
		BT_DBG("%02x ",record->linkKey[len]);
	}
	BT_DBG("\n---\n");
}

//~~~~~~~~~~~~~~~~~~
static void ShowOneDevMemoryPairedInfo(int RecId,unsigned char* Info, int Size)
{
    int i,t;
    DBG("MemID = %d: ",RecId);
    for(i = 0 ; i < Size ; i ++)
    {
        t = Info[i];
        t &= 0xFF;
        DBG("%02x ",t);
    }
    DBG("\n");
}
static void ShowOneDevSavedPairingInfo(int RecId,int AddrOffset, int Size)
{
    int i,t;
    uint8_t C;
    DBG("RecID = %d: ",RecId);
    for(i = 0; i < Size ; i ++)
    {
        SpiFlashRead(AddrOffset + i, &C, 1, 0);
        t = C; 
        t &= 0xFF;
        DBG("%02x ",t);
    }
    DBG("\n");
}

static void ShowFlashLoadResult(int RecIdxFromZero, unsigned char *BtAddr,unsigned char *LinkKey,unsigned char *Property)
{
    int i,t;
    DBG("FlhID = %d: ",RecIdxFromZero);
    for(i = 0 ; i < 6 ; i ++)
    {
        t = BtAddr[i]; 
        t &= 0xFF;
        DBG("%02x ",t);
    }
    
    for(i = 0 ; i < 16 ; i ++)
    {
        t = LinkKey[i]; 
        t &= 0xFF;
        DBG("%02x ",t);
    }
    
    t = Property[0]; 
        t &= 0xFF;
        DBG("%02x\n",t);
}
#endif //show bt pairing info


/**********************************************************************************************/
/*  */
/**********************************************************************************************/
/****************************************************************************
 * DDB list重新分配新设备的位置
 * list满了，则移除第一个设备信息
 ****************************************************************************/
static uint32_t DbdAllocateRecord(void)
{
    uint32_t count;
	
	//find the available item if any
	for(count = 0 ; count < MAX_BT_DEVICE_NUM ; count ++)
	{
		if(btManager.btLinkDeviceInfo[count].UsedFlag == 0)
			return count;
	}

	//now it is full, I will remove the first record
    for(count = 1 ; count < MAX_BT_DEVICE_NUM ; count ++)
    {
        memcpy(&(btManager.btLinkDeviceInfo[count-1]),&(btManager.btLinkDeviceInfo[count]),sizeof(BT_LINK_DEVICE_INFO));
    }

    btManager.btLinkDeviceInfo[MAX_BT_DEVICE_NUM - 1].UsedFlag = 0;

    return MAX_BT_DEVICE_NUM - 1;
}

/****************************************************************************
 *  获取到list中已记录的蓝牙个数
 ****************************************************************************/
uint32_t GetCurTotaBtRecNum(void)
{
    uint32_t count;
    
    for(count = 0 ; count < MAX_BT_DEVICE_NUM ; count ++)
    {
        if(!(btManager.btLinkDeviceInfo[count].UsedFlag))
			break;
    }
    
    return (count);
}

/*---------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
uint32_t DdbFindRecord(const uint8_t *bdAddr)
{
    uint32_t count;
	
	for (count = 0; count < MAX_BT_DEVICE_NUM; count++) 
	{
		if(btManager.btLinkDeviceInfo[count].UsedFlag == 0)continue;
		if (memcmp(bdAddr, btManager.btLinkDeviceInfo[count].device.bdAddr, 6) == 0)
		{
            return count;
        }
    }
    return DDB_NOT_FOUND;
}

/*---------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
void DdbDeleteRecord(uint8_t index)
{
	uint32_t count = 0;

	if(index != (MAX_BT_DEVICE_NUM-1))
	{
		for(count = (index+1) ; count < MAX_BT_DEVICE_NUM ; count ++)
	    {
	        memcpy(&(btManager.btLinkDeviceInfo[count-1]),&(btManager.btLinkDeviceInfo[count]),sizeof(BT_LINK_DEVICE_INFO));
	    }

	    btManager.btLinkDeviceInfo[MAX_BT_DEVICE_NUM - 1].UsedFlag = 0;
	}
	else
	{
		memset(&btManager.btLinkDeviceInfo[count], 0, sizeof(BT_LINK_DEVICE_INFO));
	}
}


/****************************************************************************
 *  从flash中加载保存的设备信息
 *  input:  OneFullRecBlockSize - 每块保存数据的大小
 *          RecIdxFromZero - 数据索引
 *          BtAddr - 读取的蓝牙地址信息
 *          Linkkey - 读取的蓝牙秘钥
 *          Property - 包含 trust, keytype, pinlen信息
 *  output: 0:失败;    1:成功
 ****************************************************************************/
static int FlshLoad1of8Dev(int OneFullRecBlockSize, int RecIdxFromZero, uint8_t* BtAddr, uint8_t* LinkKey, uint8_t* Property)
{
	uint8_t Tmp[MVBT_DB_FLAG_SIZE];
	uint32_t i;
	uint32_t StartOffset = BTDB_TOTAL_RECORD_ADDR;

    OneFullRecBlockSize += MVBT_DB_FLAG_SIZE;//including 3Bytes of sync data,zhouyi,20140411

	//get the data
	for(i = StartOffset; i <= StartOffset + BTDB_TOTAL_RECORD_MEM_SIZE - OneFullRecBlockSize; i += OneFullRecBlockSize)
	{
		SpiFlashRead(i, Tmp, MVBT_DB_FLAG_SIZE, 0);
		if((Tmp[0] == 'M') && (Tmp[1] == 'V') && (Tmp[2] == 'B') && (Tmp[3] == 'T'))
		{
			SpiFlashRead(i + MVBT_DB_FLAG_SIZE, Tmp, 1, 1); //rec count

			if(RecIdxFromZero >= (uint32_t)Tmp[0])
			{
				return 0;//not found
			}

			i = i + RecIdxFromZero * 23 + MVBT_DB_FLAG_SIZE + 1;

			//get addr
			SpiFlashRead(i, BtAddr, 6, 0);

			//key
			SpiFlashRead(i + 6, LinkKey, 16, 0);

			//property
			SpiFlashRead(i + 22, Property, 1, 0);

#ifdef PRINT_RECORD_INFOR
            ShowFlashLoadResult(RecIdxFromZero, BtAddr,LinkKey,Property);
#endif            
			return 1;
		}
	}

	return 0;
}

/**********************************************************************************************
 * 加载flash中保存的蓝牙配对记录信息
 **********************************************************************************************/
uint8_t BtDdb_Open(const uint8_t * localBdAddr)
{
	uint32_t 		count;
	
	/* See if the device database file exists. If it does not exist then
	 * create a new one.
	 *
	 * For some reasons, I just clear the info.
	 */
	for(count = 0 ; count < MAX_BT_DEVICE_NUM ; count ++)
	{
		uint8_t Property;
		btManager.btLinkDeviceInfo[count].UsedFlag = FlshLoad1of8Dev(1 + BT_REC_INFO_LEN * MAX_BT_DEVICE_NUM/*one rec block size*/,
													 count,btManager.btLinkDeviceInfo[count].device.bdAddr,
														   btManager.btLinkDeviceInfo[count].device.linkKey,
														   &Property);
		if(btManager.btLinkDeviceInfo[count].UsedFlag)
		{
//			  Record->trusted = (RecNum & 0x80) ? 1 : 0; //bit 7
//			  Record->keyType = (RecNum>>4) & 0x07; 	 //bit 4.5.6
//			  Record->pinLen  = (RecNum & 0x0F) ;		 //bit 3,2,1,0			  
			btManager.btLinkDeviceInfo[count].device.trusted = (Property & 0x80) ? 1 : 0; //bit 7
			btManager.btLinkDeviceInfo[count].device.keyType = (Property>>4) & 0x07;	  //bit 4.5.6
			btManager.btLinkDeviceInfo[count].device.pinLen  = (Property & 0x0F) ;		  //bit 3,2,1,0 			 
		}
	}
	return 0;
}


/**********************************************************************************************
 * 新增新的蓝牙配对信息
 * input: record
 **********************************************************************************************/
bool BtDdb_AddOneRecord(const BT_DB_RECORD * record)
{
	uint32_t count;
	uint32_t ddbUpdate = 0;
		
	count = DdbFindRecord((const uint8_t*)&(record->bdAddr));
	if (count == DDB_NOT_FOUND)
	{
		count = DbdAllocateRecord();

		btManager.btLinkDeviceInfo[count].UsedFlag = 1;
		memcpy(&(btManager.btLinkDeviceInfo[count].device),record,sizeof(BT_DB_RECORD));

		ddbUpdate = 1;
	}
	else
	{
		if(memcmp(btManager.btLinkDeviceInfo[count].device.linkKey, record->linkKey, 16) != 0)
		{
			memcpy(&(btManager.btLinkDeviceInfo[count].device),record,sizeof(BT_DB_RECORD));
			btManager.btLinkDeviceInfo[count].UsedFlag = 1;
			ddbUpdate = 1;
		}
	}

	if(ddbUpdate)
	{
		ddbUpdate = 0;
		//save total device info to flash
		SaveTotalDevRec2Flash(1 + BT_REC_INFO_LEN * MAX_BT_DEVICE_NUM/*one total rec block size*/,
					 	GetCurTotaBtRecNum());
	}

#ifdef PRINT_RECORD_INFOR
	PrintRecordInfor(record);
#endif
	
	return TRUE;
}

/**********************************************************************************************
 * 查找 最后一次连接设备的保存的位置
 * input:  StartOffset - 起始偏移地址
 * output: 查询到的偏移地址
 **********************************************************************************************/
static uint32_t BtDdb_GetLastAddrWriteOffset(uint32_t StartOffset)
{
	uint32_t i;
	uint8_t Tmp[4];
	uint32_t Step = BTDB_LAST_RECORD_SIZE;
		
	for(i = StartOffset; i <= StartOffset + BTDB_ALIVE_RECORD_MEM_SIZE  - Step ; i += Step)
	{
		SpiFlashRead(i, Tmp, MVBT_DB_LAST_FLAG_SIZE, 0);
		if((Tmp[0] == 'M') && (Tmp[1] == 'V') && (Tmp[2] == 'B') && (Tmp[3] == 'T'))
		{
			if(i != StartOffset)
			{
				return i - Step;
			}
			else
			{
				break; //data is full
			}
		}
		else if((Tmp[0] != 0xFF) || (Tmp[1] != 0xFF) || (Tmp[2] != 0xFF) || (Tmp[3] != 0xFF)) //some error data found, then skip it
		{
			if(i != StartOffset)
			{
				return i - Step;
			}
			else
			{
				break; //data is full
			}
		}
	}

	if(i > StartOffset + BTDB_ALIVE_RECORD_MEM_SIZE - Step)
	{
		//empty data aera, for first usage
		return (StartOffset + BTDB_ALIVE_RECORD_MEM_SIZE - (BTDB_ALIVE_RECORD_MEM_SIZE % Step) - Step);
	}

	//not found or buffer is full now
	//擦除last addr保存区域 0x1dc000-0x1dcfff (4k)
	printf("flash erase\n");
	SpiFlashErase(SECTOR_ERASE, (BTDB_ALIVE_RECORD_ADDR) /4096 , 1);

	return (StartOffset + BTDB_ALIVE_RECORD_MEM_SIZE - (BTDB_ALIVE_RECORD_MEM_SIZE % Step) - Step);

}

/**********************************************************************************************
 * 更新最后一次连接过的蓝牙设备地址
 * input:  BtLastAddr - 蓝牙地址
 * output: 1=success;  0=fail;
 **********************************************************************************************/
bool BtDdb_UpgradeLastBtAddr(uint8_t *BtLastAddr)
{
	uint32_t AliveOffset;
	uint32_t TmpOffset;
	uint8_t Tmp[MVBT_DB_LAST_FLAG_SIZE];

	if(btManager.btLastAddrUpgradeIgnored)
		return 0;

	AliveOffset = BTDB_ALIVE_RECORD_ADDR;
	TmpOffset = BtDdb_GetLastAddrWriteOffset(AliveOffset);
	BT_DBG("Addr write Offset = %x\n", (int)TmpOffset);
	if(TmpOffset >= AliveOffset)
	{
		strcpy((char*)Tmp, MVBT_DB_LAST_FLAG);
		SpiFlashWrite(TmpOffset, Tmp, MVBT_DB_LAST_FLAG_SIZE, 0);
		return (0 == SpiFlashWrite(TmpOffset + MVBT_DB_LAST_FLAG_SIZE, BtLastAddr, 6, 0)) ? 1 : 0;
	}

	return 0;
}

/**********************************************************************************************
 * 加载最后一次连接过的蓝牙设备地址
 * input:  BtLastAddr - 蓝牙地址
 * output: 1=success;  0=fail;
 **********************************************************************************************/
bool BtDdb_GetLastBtAddr(uint8_t *BtLastAddr)
{
	uint32_t i;
	uint8_t Tmp[MVBT_DB_LAST_FLAG_SIZE];
	uint32_t StartOffset, Step = BTDB_LAST_RECORD_SIZE; //9 BYTE for alive addr (3 majic, 6 addr)

	StartOffset = BTDB_ALIVE_RECORD_ADDR;

	for(i = StartOffset; i < StartOffset + BTDB_ALIVE_RECORD_MEM_SIZE / BTDB_LAST_RECORD_SIZE * BTDB_LAST_RECORD_SIZE; i += Step)
	{
		SpiFlashRead(i, Tmp, MVBT_DB_LAST_FLAG_SIZE, 0);
		if((Tmp[0] == 'M') && (Tmp[1] == 'V') && (Tmp[2] == 'B') && (Tmp[3] == 'T'))
		{
			SpiFlashRead(i + MVBT_DB_LAST_FLAG_SIZE, BtLastAddr, 6, 0);
			
			DBG("Last device addr:");
			for(i=0;i<6;i++)
			{
				DBG("0x%02x ", BtLastAddr[i]);
			}
			DBG("\n");
			return 1;
		}
		else if((Tmp[0] != 0xFF) || (Tmp[1] != 0xFF) || (Tmp[2] != 0xFF) || (Tmp[3] != 0xFF)) //some error data found, then skip it
		{
			break; //error data found, I just return fail.
		}
	}
	
	DBG("LAST ADDR NOT FOUND\n");
	//not found or buufer is full now
	return 0;
}

/****************************************************************************************/
/* 擦除最后连接设备的保存区域 */
/****************************************************************************************/
void BtDdb_LastBtAddrErase(void)
{
	SpiFlashErase(SECTOR_ERASE, (BTDB_ALIVE_RECORD_ADDR) /4096 , 1);
	BT_DBG("[BT_OFFSET]erase last bt addr area\n");
}

/****************************************************************************************/
/* 擦除行为 */
/****************************************************************************************/
bool BtDdb_Erase(void)
{
//	int32_t i=0;
//	uint8_t buf[4];

	printf("flash erase: bt record\n");

	//擦除0x1d8000-0x1dbfff (16k)
	/*for(i=0;i<BTDB_NVM_RECORD_CNT;i++)
	{
		SpiFlashErase(SECTOR_ERASE, (BTDB_TOTAL_RECORD_ADDR + i * BTDB_NVM_SECTOR_SIZE) /4096 , 1);
	}*/
	
	SpiFlashErase(BLOCK_ERASE, 0x1e0000 /0x10000 , 1);
	
	return 1;
}

/****************************************************************************************/
/* 对蓝牙参数配置表数据进行读，写操作 */
/****************************************************************************************/
int8_t BtDdb_LoadBtConfigurationParams(BT_CONFIGURATION_PARAMS *params)
{
	SPI_FLASH_ERR_CODE ret=0;

	if(params == NULL)
	{
		BT_DBG("[BTDDB]read error:params is null\n");
		return -1;
	}
	
	ret = SpiFlashRead(BTDB_CONFIG_ADDR, (uint8_t*)params, sizeof(BT_CONFIGURATION_PARAMS), 0);
	if(ret != FLASH_NONE_ERR)
		return -3;	//flash 读取错误

	//数据读取后，外面进行处理
	return 0;
	
}

int8_t BtDdb_SaveBtConfigurationParams(BT_CONFIGURATION_PARAMS *params)
{
	SPI_FLASH_ERR_CODE ret=0;
	
	if(params == NULL)
	{
		BT_DBG("[BTDDB]write error:params is null\n");
		return -1;
	}

	//1.erase
	SpiFlashErase(SECTOR_ERASE, (BTDB_CONFIG_ADDR/4096), 1);

	//2.write params
	ret = SpiFlashWrite(BTDB_CONFIG_ADDR, (uint8_t*)params, sizeof(BT_CONFIGURATION_PARAMS), 0);
	if(ret != FLASH_NONE_ERR)
	{
		BT_DBG("[BTDDB]write error:%d\n", ret);
		return -2;
	}

	BT_DBG("[BTDDB]write success\n");
	return 0;
}

/****************************************************************************
 *  查询flash中保存的蓝牙数据的offset
 ****************************************************************************/
static uint32_t FlshGetPairingInfoWriteOffset(uint32_t StartOffset, uint32_t Step)
{
	uint32_t i;
	uint8_t Tmp[MVBT_DB_FLAG_SIZE];

	for(i = StartOffset; i <= StartOffset + BTDB_TOTAL_RECORD_MEM_SIZE - Step - 4/* 4 byte Magic Number */; i += Step)
	{
		SpiFlashRead(i, Tmp, MVBT_DB_FLAG_SIZE, 0);
		if((Tmp[0] == 'M') && (Tmp[1] == 'V') && (Tmp[2] == 'B') && (Tmp[3] == 'T'))
		{
			if(i != StartOffset)
			{
				return i - Step;
			}
			else
			{
				break; //"data is full, need to erase Flash
			}
		}
		else
		{
			if((Tmp[0] != 0xFF) || (Tmp[1] != 0xFF) || (Tmp[2] != 0xFF) || (Tmp[3] != 0xFF)) //some error data found, then skip it
			{
				if(i != StartOffset)
				{
					return i - Step;
				}
				else
				{
					break; //data is full
				}
			}
		}
	}
	
	if( i > StartOffset + BTDB_TOTAL_RECORD_MEM_SIZE - Step - 4) /* 4 means 4 byte Magic Number */
	{
		//empty DATA aera, for first usage
		return (StartOffset + BTDB_TOTAL_RECORD_MEM_SIZE - 4 - ((BTDB_TOTAL_RECORD_MEM_SIZE - 4) % Step) - Step);
	}
		

	//erase & write magic number
	SpiFlashErase(SECTOR_ERASE, (StartOffset) /4096 , 1);
	SpiFlashErase(SECTOR_ERASE, (StartOffset+4096) /4096 , 1);
	Tmp[0] = 'P';
	Tmp[1] = 'R';
	Tmp[2] = 'I';
	Tmp[3] = 'F';
	SpiFlashWrite(StartOffset + BTDB_TOTAL_RECORD_MEM_SIZE - 4, Tmp, 4, 1);	//write magic number

	return (StartOffset + BTDB_TOTAL_RECORD_MEM_SIZE - 4 - ((BTDB_TOTAL_RECORD_MEM_SIZE - 4) % Step) - Step);

}


/****************************************************************************
 *  获取list中对应index的设备信息
 ****************************************************************************/
static uint32_t Get1of8RecInfo(uint8_t RecIdx/*from 0*/, uint8_t *Data/*size must be no less than 23B*/)
{
    if(RecIdx >= GetCurTotaBtRecNum())return 0; //not found
    
    memcpy(Data,btManager.btLinkDeviceInfo[RecIdx].device.bdAddr,6);
    memcpy(Data+6,btManager.btLinkDeviceInfo[RecIdx].device.linkKey,16);
    Data[22] = (((btManager.btLinkDeviceInfo[RecIdx].device.trusted & 0x01) << 7) |
                ((btManager.btLinkDeviceInfo[RecIdx].device.keyType & 0x07) << 4) |
                ((btManager.btLinkDeviceInfo[RecIdx].device.pinLen  & 0x0F)));
    return 1;
}

/****************************************************************************
 *  保存List中所有的蓝牙设备信息到flash
 ****************************************************************************/
static void SaveTotalDevRec2Flash(int OneFullRecBlockSize, int TotalRecNum)
{
	uint8_t i, Tmp[MVBT_DB_FLAG_SIZE];
//	uint32_t t;
	uint32_t StartOffset = BTDB_TOTAL_RECORD_ADDR;
	uint8_t OneBtRec[23];
	
    OneFullRecBlockSize += MVBT_DB_FLAG_SIZE;//including 3Bytes of sync data,zhouyi,20140411

	StartOffset = FlshGetPairingInfoWriteOffset(StartOffset, OneFullRecBlockSize);
	DBG("-----------------------------------New0x%lx\n", StartOffset);
	Tmp[0] = 'M';
	Tmp[1] = 'V';
	Tmp[2] = 'B';
	Tmp[3] = 'T';
	SpiFlashWrite(StartOffset, Tmp, MVBT_DB_FLAG_SIZE, 1);
	StartOffset += MVBT_DB_FLAG_SIZE;

	//total num
	Tmp[0] = TotalRecNum & 0xFF;
	SpiFlashWrite(StartOffset, Tmp, 1, 1);
	DBG("TOTAL NUM:%d\n", TotalRecNum);
	StartOffset++;
	for(i = 0; i < TotalRecNum; i++)
	{
		//get data
		if(Get1of8RecInfo(i, OneBtRec))
		{
#ifdef PRINT_RECORD_INFOR            
			ShowOneDevMemoryPairedInfo(i,OneBtRec,BT_REC_INFO_LEN);
#endif            
			SpiFlashWrite(StartOffset, OneBtRec, BT_REC_INFO_LEN, 1);
			StartOffset += BT_REC_INFO_LEN;
#ifdef PRINT_RECORD_INFOR            
            ShowOneDevSavedPairingInfo(i,StartOffset - BT_REC_INFO_LEN,BT_REC_INFO_LEN);
#endif            
		}
	}
}

