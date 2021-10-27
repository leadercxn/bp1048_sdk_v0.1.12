

#include <string.h>

#include "type.h"
#include "spi_flash.h"
#include "flash_config.h"
#include "bt_ddb_flash.h"
#include "debug.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

/**********************************************************************************************/
/* ��ӡ��Ӧ����Ϣ */
/**********************************************************************************************/
#ifdef PRINT_RECORD_INFOR
static void PrintRecordInfor(const BT_DB_RECORD * record)
{
	uint8_t len=0;
	APP_DBG("---\n");
	APP_DBG("RemoteAddr:");
	for(len=0; len<6; len++)
	{
		APP_DBG("%02x ",record->bdAddr[len]);
	}
	APP_DBG("\nlinkkey:");
	for(len=0; len<16; len++)
	{
		APP_DBG("%02x ",record->linkKey[len]);
	}
	APP_DBG("\n---\n");
}

//~~~~~~~~~~~~~~~~~~
static void ShowOneDevMemoryPairedInfo(int RecId,unsigned char* Info, int Size)
{
    int i,t;
    APP_DBG("MemID = %d: ",RecId);
    for(i = 0 ; i < Size ; i ++)
    {
        t = Info[i];
        t &= 0xFF;
        APP_DBG("%02x ",t);
    }
    APP_DBG("\n");
}
static void ShowOneDevSavedPairingInfo(int RecId,int AddrOffset, int Size)
{
    int i,t;
    uint8_t C;
    APP_DBG("RecID = %d: ",RecId);
    for(i = 0; i < Size ; i ++)
    {
        SpiFlashRead(AddrOffset + i, &C, 1, 0);
        t = C; 
        t &= 0xFF;
        APP_DBG("%02x ",t);
    }
    APP_DBG("\n");
}

static void ShowFlashLoadResult(int RecIdxFromZero, unsigned char *BtAddr,unsigned char *LinkKey,unsigned char *Property
#ifdef BT_TWS_SUPPORT
								, unsigned char *Role, unsigned char *Profile
#endif								
								)
{
    int i,t;
    APP_DBG("FlhID = %d: ",RecIdxFromZero);
    for(i = 0 ; i < 6 ; i ++)
    {
        t = BtAddr[i]; 
        t &= 0xFF;
        APP_DBG("%02x ",t);
    }
    
    for(i = 0 ; i < 16 ; i ++)
    {
        t = LinkKey[i]; 
        t &= 0xFF;
        APP_DBG("%02x ",t);
    }
    
    t = Property[0]; 
        t &= 0xFF;
        APP_DBG("%02x ",t);
		
#ifdef BT_TWS_SUPPORT
    t = Role[0]; 
        t &= 0xFF;
        APP_DBG("%02x\n",t);
		
    t = Profile[0]; 
        t &= 0xFF;
        APP_DBG("%02x\n",t);
#endif
}
#endif //show bt pairing info


/**********************************************************************************************/
/*  */
/**********************************************************************************************/
/****************************************************************************
 * DDB list���·������豸��λ��
 *#if defined(BT_TWS_SUPPORT)
 * ��һ����Ϣ��TWS��Ϣ 
 * list���ˣ����Ƴ��ڶ����豸��Ϣ
 *#else
 * list���ˣ����Ƴ���һ���豸��Ϣ
 *#endif
 ****************************************************************************/
static uint32_t DbdAllocateRecord(void)
{
    uint32_t count;
	
	//find the available item if any
#ifdef BT_TWS_SUPPORT
	//��[1]��ʼ����device��ַ��Ϣ
	for(count = 1 ; count < MAX_BT_DEVICE_NUM ; count ++)
#else
	for(count = 0 ; count < MAX_BT_DEVICE_NUM ; count ++)
#endif
	{
		if(btManager.btLinkDeviceInfo[count].UsedFlag == 0)
			return count;
	}

	//now it is full, I will remove the first record
#ifdef BT_TWS_SUPPORT
    for(count = 2 ; count < MAX_BT_DEVICE_NUM ; count ++)
#else
	for(count = 1 ; count < MAX_BT_DEVICE_NUM ; count ++)
#endif
    {
        memcpy(&(btManager.btLinkDeviceInfo[count-1]),&(btManager.btLinkDeviceInfo[count]),sizeof(BT_LINK_DEVICE_INFO));
    }

    btManager.btLinkDeviceInfo[MAX_BT_DEVICE_NUM - 1].UsedFlag = 0;

    return MAX_BT_DEVICE_NUM - 1;
}

/****************************************************************************
 *  ��ȡ��list���Ѽ�¼����������
 ****************************************************************************/
uint32_t GetCurTotaBtRecNum(void)
{
    uint32_t count;
#ifdef BT_TWS_SUPPORT
    for(count = 1 ; count < MAX_BT_DEVICE_NUM ; count ++)
#else
	for(count = 0 ; count < MAX_BT_DEVICE_NUM ; count ++)
#endif
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
#ifdef BT_TWS_SUPPORT
		btManager.btLinkDeviceInfo[count].remote_profile = 0xff;
#endif
	}
}


/****************************************************************************
 *  ��flash�м��ر�����豸��Ϣ
 *  input:  OneFullRecBlockSize - ÿ�鱣�����ݵĴ�С
 *          RecIdxFromZero - ��������
 *          BtAddr - ��ȡ��������ַ��Ϣ
 *          Linkkey - ��ȡ��������Կ
 *          Property - ���� trust, keytype, pinlen��Ϣ
 *  output: 0:ʧ��;    1:�ɹ�
 ****************************************************************************/
static int FlshLoad1of8Dev(int OneFullRecBlockSize, int RecIdxFromZero, uint8_t* BtAddr, uint8_t* LinkKey, uint8_t* Property
#ifdef BT_TWS_SUPPORT
							, uint8_t* Role, uint8_t* Profile
#endif							
							)
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
			i = i + RecIdxFromZero * BT_REC_INFO_LEN + MVBT_DB_FLAG_SIZE + 1;
			//get addr
			SpiFlashRead(i, BtAddr, 6, 0);

			//key
			SpiFlashRead(i + 6, LinkKey, 16, 0);

			//property
			SpiFlashRead(i + 22, Property, 1, 0);
#ifdef BT_TWS_SUPPORT
			//tws_role
			SpiFlashRead(i + 23, Role, 1, 0);

			//profile
			SpiFlashRead(i + 24, Profile, 1, 0);

#ifdef PRINT_RECORD_INFOR
            ShowFlashLoadResult(RecIdxFromZero, BtAddr,LinkKey,Property, Role, Profile);
#endif      
#else
#ifdef PRINT_RECORD_INFOR
            ShowFlashLoadResult(RecIdxFromZero, BtAddr,LinkKey,Property);
#endif 
#endif //#ifdef BT_TWS_SUPPORT
      
			return 1;
		}
	}

	return 0;
}

/**********************************************************************************************
 * ����flash�б����������Լ�¼��Ϣ
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
														   &Property
#ifdef BT_TWS_SUPPORT
														   ,
														   &btManager.btLinkDeviceInfo[count].tws_role,
														   &btManager.btLinkDeviceInfo[count].remote_profile
#endif														   
														   );
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

void BtDdb_PrintRecord(void)
{
	uint32_t i;
	uint8_t k;
	uint8_t cnt;	//��ʾ���ű�洢�˼����豸
	uint8_t Tmp[MVBT_DB_LAST_FLAG_SIZE];
	uint32_t Step = (MVBT_DB_LAST_FLAG_SIZE + BT_REC_INFO_LEN * MAX_BT_DEVICE_NUM + 1);
	uint32_t StartOffset = BTDB_TOTAL_RECORD_ADDR;	// �����ʱ���ǴӸߵ��ͣ�������ʱ��ӵ͵���
	BT_LINK_DEVICE_INFO info[8];

	APP_DBG("print the remote name: 0x%x\n", StartOffset);
	for(i = StartOffset; i < (BTDB_TOTAL_RECORD_ADDR + BTDB_TOTAL_RECORD_MEM_SIZE - 4); i += Step)
	{
		SpiFlashRead(i, Tmp, MVBT_DB_LAST_FLAG_SIZE, 0);
		if((Tmp[0] == 'M') && (Tmp[1] == 'V') && (Tmp[2] == 'B') && (Tmp[3] == 'T'))
		{
			SpiFlashRead(i + 4, &cnt, 1, 0);
			APP_DBG("%d remote devices are saved\n", cnt);
			for(k = 0; k < cnt; k++)
			{
				SpiFlashRead((i + 5 + k * BT_REC_INFO_LEN), info[k].device.bdAddr, 6, 0);
				
#ifdef FLASH_SAVE_REMOTE_BT_NAME
#ifdef BT_TWS_SUPPORT
				SpiFlashRead((i + 30 + k * BT_REC_INFO_LEN), info[k].device.bdName, 40, 0);
#else
				SpiFlashRead((i + 28 + k * BT_REC_INFO_LEN), info[k].device.bdName, 40, 0);
#endif
#endif

#ifdef FLASH_SAVE_REMOTE_BT_NAME
				APP_DBG("%x:  Remote device [%d] name: %s  addr:%02X:%02X:%02X:%02X:%02X:%02X\n", i, k, info[k].device.bdName, 
					info[k].device.bdAddr[0],
					info[k].device.bdAddr[1],
					info[k].device.bdAddr[2],
					info[k].device.bdAddr[3],
					info[k].device.bdAddr[4],
					info[k].device.bdAddr[5]);
#else
				APP_DBG("%x:  Remote device [%d]  addr:%02X:%02X:%02X:%02X:%02X:%02X\n", i, k, 
					info[k].device.bdAddr[0],
					info[k].device.bdAddr[1],
					info[k].device.bdAddr[2],
					info[k].device.bdAddr[3],
					info[k].device.bdAddr[4],
					info[k].device.bdAddr[5]);
#endif
			}
			break;
		}
		else if((Tmp[0] != 0xFF) || (Tmp[1] != 0xFF) || (Tmp[2] != 0xFF) || (Tmp[3] != 0xFF)) //some error data found, then skip it
		{
			APP_DBG("some error data found, then skip it\n");
			continue;	// ����Ҫ����ѭ��������Ѱ����һ��509B������
		}
	}

	return;
}

/**********************************************************************************************
 * �����µ����������Ϣ
 * input: record
 **********************************************************************************************/
bool BtDdb_AddOneRecord(const BT_DB_RECORD * record)
{
	uint32_t count;

	//����record�ṹ�е�������Ϣ
#ifdef FLASH_SAVE_REMOTE_BT_NAME
	memset(record->bdName, 0, 40);
	memcpy(record->bdName, btManager.remoteName, btManager.remoteNameLen);
#endif

#ifdef BT_TWS_SUPPORT
	uint32_t totalCount;
#endif
	
	if(btManager.btDutModeEnable)
		return FALSE;
	
	count = DdbFindRecord((const uint8_t*)&(record->bdAddr));
	if (count == DDB_NOT_FOUND)
	{
		count = DbdAllocateRecord();

		btManager.btLinkDeviceInfo[count].UsedFlag = 1;
		memcpy(&(btManager.btLinkDeviceInfo[count].device),record,sizeof(BT_DB_RECORD));
#ifdef BT_TWS_SUPPORT
		btManager.btLinkDeviceInfo[count].remote_profile = 0xff;

		btManager.ddbUpdate = 1;
	}
	else if(count == 0)
	{
		//tws ��Լ�¼��������
		if(memcmp(btManager.btLinkDeviceInfo[count].device.linkKey, record->linkKey, 16) != 0)
		{
			memcpy(&(btManager.btLinkDeviceInfo[count].device),record,sizeof(BT_DB_RECORD));
			btManager.btLinkDeviceInfo[count].UsedFlag = 1;
			btManager.ddbUpdate = 1;
		}
	}
	else
	{
		totalCount = GetCurTotaBtRecNum();
		//ȷ�ϸ��µ��豸��Ϣ���б��е�����
		if((count+1) != totalCount)
		{
			//���б�����ɾ��֮ǰ�ļ�¼
			DdbDeleteRecord(count);
			//����¼���µ��б����
			
			memcpy(&(btManager.btLinkDeviceInfo[totalCount-1].device),record,sizeof(BT_DB_RECORD));
			btManager.btLinkDeviceInfo[totalCount-1].UsedFlag = 1;
			btManager.btLinkDeviceInfo[totalCount-1].remote_profile = 0xff;
#endif//#ifndef BT_TWS_SUPPORT
			btManager.ddbUpdate = 1;
		}
		else
		{
			if(memcmp(btManager.btLinkDeviceInfo[count].device.linkKey, record->linkKey, 16) != 0)
			{
				memcpy(&(btManager.btLinkDeviceInfo[count].device),record,sizeof(BT_DB_RECORD));
				btManager.btLinkDeviceInfo[count].UsedFlag = 1;
				btManager.ddbUpdate = 1;
#ifdef BT_TWS_SUPPORT
				btManager.btLinkDeviceInfo[count].remote_profile = 0xff;
				btManager.btDdbLastProfile = 0; //��ͬһ�������豸��link key�����仯ʱ,ͬ����Ҫ������ӹ���profile����
			}
#endif//#if defined(BT_TWS_SUPPORT)
		}
	}

	return TRUE;
}

#ifndef BT_TWS_SUPPORT
/**********************************************************************************************
 * ���� ���һ�������豸�ı����λ��
 * input:  StartOffset - ��ʼƫ�Ƶ�ַ
 * output: ��ѯ����ƫ�Ƶ�ַ
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
	//����last addr�������� 0x1dc000-0x1dcfff (4k)
	printf("flash erase\n");
	SpiFlashErase(SECTOR_ERASE, (BTDB_ALIVE_RECORD_ADDR) /4096 , 1);

	return (StartOffset + BTDB_ALIVE_RECORD_MEM_SIZE - (BTDB_ALIVE_RECORD_MEM_SIZE % Step) - Step);

}
#endif //#ifndef BT_TWS_SUPPORT
/**********************************************************************************************
 * �������һ�����ӹ��������豸��ַ
 * input:  BtLastAddr - ������ַ
 * output: 1=success;  0=fail;
 **********************************************************************************************/
bool BtDdb_UpgradeLastBtAddr(uint8_t *BtLastAddr, uint8_t BtLastProfile)
{
#ifndef BT_TWS_SUPPORT
	uint32_t AliveOffset;
	uint32_t TmpOffset;
	uint8_t Tmp[11];
	uint8_t TmpProfile = 0xff;
#else
	uint32_t count, totalCount;
	BT_DB_RECORD record;
#endif
	if(btManager.btDutModeEnable)
		return 0;
	
	if(btManager.btLastAddrUpgradeIgnored)
		return 0;
		
#ifndef BT_TWS_SUPPORT
	AliveOffset = BTDB_ALIVE_RECORD_ADDR;
	TmpOffset = BtDdb_GetLastAddrWriteOffset(AliveOffset);
	APP_DBG("Addr write Offset = %x\n", (int)TmpOffset);
	if(TmpOffset >= AliveOffset)
	{
		Tmp[0] = 'M';
		Tmp[1] = 'V';
		Tmp[2] = 'B';
		Tmp[3] = 'T';
		//addr 4,5,6,7,8,9
		memcpy(&Tmp[4], BtLastAddr, 6);
		TmpProfile &= ~BtLastProfile;
		Tmp[10] = TmpProfile;
		SpiFlashWrite(TmpOffset, Tmp, 11, 1);
		btManager.btDdbLastInfoOffset = TmpOffset;
	}

#else//#ifdef BT_TWS_SUPPORT
	APP_DBG("BtDdb_UpgradeLastBtAddr\n");
	count = DdbFindRecord((const uint8_t*)BtLastAddr);
	if (count == DDB_NOT_FOUND)
	{
		return 0;
	}
	else
	{
		totalCount = GetCurTotaBtRecNum();
		//ȷ�ϸ��µ��豸��Ϣ���б��е�����
		if((count+1) != totalCount)
		{
			//�����¼
			memcpy(&record,&(btManager.btLinkDeviceInfo[count].device),sizeof(BT_DB_RECORD));
			
			//���б�����ɾ��֮ǰ�ļ�¼
			DdbDeleteRecord(count);
			
			//����¼���µ��б����
			memcpy(&(btManager.btLinkDeviceInfo[totalCount-1].device),&record,sizeof(BT_DB_RECORD));
			btManager.btLinkDeviceInfo[totalCount-1].UsedFlag = 1;
			btManager.ddbUpdate = 1;
			btManager.btLinkDeviceInfo[totalCount-1].remote_profile = ~BtLastProfile;
		}
		else
		{
			return 0;
		}
	}

#endif //#ifndef BT_TWS_SUPPORT
	return 0;
}

bool BtDdb_UpgradeLastBtProfile(uint8_t *BtLastAddr, uint8_t BtLastProfile)
{
#ifndef BT_TWS_SUPPORT
//	uint32_t i;
	uint8_t Tmp[MVBT_DB_LAST_FLAG_SIZE];
//	uint32_t StartOffset, Step = BTDB_LAST_RECORD_SIZE;
	uint8_t	TmpAddr[6];
#else
	uint8_t Tmp[BT_REC_INFO_LEN];
	uint32_t Step = (MVBT_DB_LAST_FLAG_SIZE + 1 + BT_REC_INFO_LEN * MAX_BT_DEVICE_NUM);
#endif

	uint8_t TmpProfile = 0xff;
	
#ifdef BT_TWS_SUPPORT
	uint32_t StartOffset = BTDB_TOTAL_RECORD_ADDR;
	uint32_t TotalNumber = 0;
#endif	

	if(btManager.btDutModeEnable)
		return 0;
		
	if(btManager.btLastAddrUpgradeIgnored)
		return 0;
		
#ifdef BT_TWS_SUPPORT
	StartOffset = FlshGetPairingInfoWriteOffset(StartOffset, Step);
	//printf("offset=0x%x\n", StartOffset);
	StartOffset += Step;
	StartOffset += 4;//flag
	StartOffset += 1;//total number

	TotalNumber = GetCurTotaBtRecNum();
	StartOffset += ((TotalNumber-1)*BT_REC_INFO_LEN);

	SpiFlashRead(StartOffset, Tmp, BT_REC_INFO_LEN, 1);

	if(memcmp(BtLastAddr, &Tmp[0], 6) == 0)
	{
		TmpProfile &= ~BtLastProfile;
		SpiFlashWrite(StartOffset + BT_REC_INFO_LEN - 41, &TmpProfile, 1, 1);	// name(40) + profile(1) = 41
		APP_DBG("BtDdb_UpgradeLastBtProfile: 0x%02x\n", TmpProfile);
		
		btManager.btLinkDeviceInfo[TotalNumber-1].remote_profile = ~BtLastProfile;
		return 1;
	}
#else//#ifndef BT_TWS_SUPPORT
	SpiFlashRead(btManager.btDdbLastInfoOffset, Tmp, MVBT_DB_LAST_FLAG_SIZE, 0);
	if((Tmp[0] == 'M') && (Tmp[1] == 'V') && (Tmp[2] == 'B') && (Tmp[3] == 'T'))
	{
		SpiFlashRead(btManager.btDdbLastInfoOffset + MVBT_DB_LAST_FLAG_SIZE, TmpAddr, 6, 0);

		if(memcmp(TmpAddr, BtLastAddr, 6) == 0)
		{
			TmpProfile &= ~BtLastProfile;
			//APP_DBG("update bt profile: %x, addr:0x%x\n", TmpProfile, btManager.btDdbLastInfoOffset);
			SpiFlashWrite(btManager.btDdbLastInfoOffset + MVBT_DB_LAST_FLAG_SIZE + 6, &TmpProfile, 1, 1);
			return 1;
		}
	}
#endif//#ifdef BT_TWS_SUPPORT
	return 0;
}

/**********************************************************************************************
 * �������һ�����ӹ��������豸��ַ
 * input:  BtLastAddr - ������ַ
 * output: 1=success;  0=fail;
 **********************************************************************************************/
#ifdef BT_TWS_SUPPORT
bool BtDdb_GetLastBtAddr(uint8_t *BtLastAddr, uint8_t* profile)
{
	uint32_t totalCount = GetCurTotaBtRecNum();

	if((totalCount>1)&&(totalCount<=MAX_BT_DEVICE_NUM))
	{
		//������б��л�ȡ���1-2���豸�ĵ�ַ
		memcpy(BtLastAddr,btManager.btLinkDeviceInfo[totalCount-1].device.bdAddr,6);
		*profile = ~btManager.btLinkDeviceInfo[totalCount-1].remote_profile;
		return 1;
	}
	else
	{
		memset(BtLastAddr, 0, 6);
		return 0;
	}
}
#else //#ifndef BT_TWS_SUPPORT
bool BtDdb_GetLastBtAddr(uint8_t *BtLastAddr)
{
	uint32_t i,j;
	uint8_t Tmp[MVBT_DB_LAST_FLAG_SIZE];
	uint32_t StartOffset, Step = BTDB_LAST_RECORD_SIZE;
	uint8_t TmpProfile = 0x00;

	StartOffset = BTDB_ALIVE_RECORD_ADDR;

	for(i = StartOffset; i < StartOffset + BTDB_ALIVE_RECORD_MEM_SIZE / BTDB_LAST_RECORD_SIZE * BTDB_LAST_RECORD_SIZE; i += Step)
	{
		SpiFlashRead(i, Tmp, MVBT_DB_LAST_FLAG_SIZE, 0);
		if((Tmp[0] == 'M') && (Tmp[1] == 'V') && (Tmp[2] == 'B') && (Tmp[3] == 'T'))
		{
			SpiFlashRead(i + MVBT_DB_LAST_FLAG_SIZE, BtLastAddr, 6, 0);
			
			APP_DBG("Last device addr:");
			for(j=0;j<6;j++)
			{
				APP_DBG("0x%02x ", BtLastAddr[j]);
			}
			APP_DBG("\n");

			SpiFlashRead(i + MVBT_DB_LAST_FLAG_SIZE + 6, &TmpProfile, 1, 0);
			GetBtManager()->btDdbLastProfile = ~TmpProfile;
			GetBtManager()->btDdbLastInfoOffset = i;
			//APP_DBG("offset:0x%x, R: %x, Profile: %x\n", GetBtManager()->btDdbLastInfoOffset, TmpProfile, GetBtManager()->btDdbLastProfile);
			return 1;
		}
		else if((Tmp[0] != 0xFF) || (Tmp[1] != 0xFF) || (Tmp[2] != 0xFF) || (Tmp[3] != 0xFF)) //some error data found, then skip it
		{
			break; //error data found, I just return fail.
		}
	}
	
	APP_DBG("LAST ADDR NOT FOUND\n");
	//not found or buufer is full now
	return 0;
}
#endif//#ifdef BT_TWS_SUPPORT

/****************************************************************************************/
/* ������������豸�ı������� */
/****************************************************************************************/
void BtDdb_LastBtAddrErase(void)
{
	SpiFlashErase(SECTOR_ERASE, (BTDB_ALIVE_RECORD_ADDR) /4096 , 1);
	APP_DBG("erase last bt addr area\n");
}

/****************************************************************************************/
/* ������Ϊ */
/* ��Լ�¼ */
/****************************************************************************************/
bool BtDdb_Erase(void)
{
	APP_DBG("flash erase: bt record\n");

	//����豸��Ϣ
	SpiFlashErase(SECTOR_ERASE, BTDB_TOTAL_RECORD_ADDR /4096 , 1);
	SpiFlashErase(SECTOR_ERASE, (BTDB_TOTAL_RECORD_ADDR + 4*1024) /4096 , 1);

	//���1�λ����豸��Ϣ
	SpiFlashErase(SECTOR_ERASE, BTDB_ALIVE_RECORD_ADDR /4096 , 1);
	
	return 1;
}

//�˺�������ʹ��(���������ܱ������򣬲����������)
bool BtDdb_Erase_BtConfig(void)
{
	APP_DBG("flash erase: bt config\n");

	SpiFlashErase(SECTOR_ERASE, BTDB_CONFIGDATA_START_ADDR /4096 , 1);
	
	return 1;
}

/****************************************************************************************/
/* �������������ñ����ݽ��ж���д���� */
/****************************************************************************************/
int8_t BtDdb_LoadBtConfigurationParams(BT_CONFIGURATION_PARAMS *params)
{
	SPI_FLASH_ERR_CODE ret=0;

	if(params == NULL)
	{
		APP_DBG("read error:params is null\n");
		return -1;
	}
	
	ret = SpiFlashRead(BTDB_CONFIG_ADDR, (uint8_t*)params, sizeof(BT_CONFIGURATION_PARAMS), 0);
	if(ret != FLASH_NONE_ERR)
		return -3;	//flash ��ȡ����

	//���ݶ�ȡ��������д���
	return 0;
	
}

int8_t BtDdb_SaveBtConfigurationParams(BT_CONFIGURATION_PARAMS *params)
{
	SPI_FLASH_ERR_CODE ret=0;
	
	if(params == NULL)
	{
		APP_DBG("write error:params is null\n");
		return -1;
	}

	//1.erase
	SpiFlashErase(SECTOR_ERASE, (BTDB_CONFIG_ADDR/4096), 1);

	//2.write params
	ret = SpiFlashWrite(BTDB_CONFIG_ADDR, (uint8_t*)params, sizeof(BT_CONFIGURATION_PARAMS), 1);
	if(ret != FLASH_NONE_ERR)
	{
		APP_DBG("write error:%d\n", ret);
		return -2;
	}

	APP_DBG("write success\n");
	return 0;
}

/************************************************************************************************************/
/* bt stack ��ʼ��ʱ�����Flash�е���������ʱ����flash�е� bt_ConfigHeader��bt_trimValue���в���*/
/************************************************************************************************************/
int8_t BtDdb_InitBtConfigurationParams(BT_CONFIGURATION_PARAMS *params)
{
	SPI_FLASH_ERR_CODE ret=0;
	
	if(params == NULL)
	{
		APP_DBG("write error:params is null\n");
		return -1;
	}

	//1.erase
	SpiFlashErase(SECTOR_ERASE, (BTDB_CONFIG_ADDR/4096), 1);

	//2.write params but un-include "config head" and "trimvalue"
	ret = SpiFlashWrite(BTDB_CONFIG_ADDR, (uint8_t*)params, (sizeof(uint8_t) * 2 * 6 + sizeof(uint8_t) * 2 * 40), 1);
	if(ret != FLASH_NONE_ERR)
	{
		APP_DBG("write addr&name error:%d\n", ret);
		return -2;
	}
	ret = SpiFlashWrite(BTDB_CONFIG_ADDR + (sizeof(uint8_t) * 2 * 6 + sizeof(uint8_t) * 2 * 40 + sizeof(uint8_t) * 5), &params->bt_TxPowerValue, 
			sizeof(BT_CONFIGURATION_PARAMS) - (sizeof(uint8_t) * 2 * 6 + sizeof(uint8_t) * 2 * 40 + sizeof(uint8_t) * 5), 1);
	if(ret != FLASH_NONE_ERR)
	{
		APP_DBG("write others error:%d\n", ret);
		return -2;
	}
	
	APP_DBG("write success\n");
	return 0;
}

#ifdef BT_TWS_SUPPORT
/****************************************************************************
 *  ����TWS�����Ϣ
 ****************************************************************************/
uint32_t BtDdb_UpgradeTwsInfor(uint8_t *BtTwsAddr)
{
	uint32_t count;//, totalCount
	BT_DB_RECORD record;
	uint8_t LinkKey[16];

	count = DdbFindRecord((const uint8_t*)BtTwsAddr);
	if (count == DDB_NOT_FOUND)
	{
		return 0;
	}
	else if(count == 0)
	{
		if(!GetBtManager()->twsFlag)
		{
			APP_DBG("tws infor update\n");
			btManager.btLinkDeviceInfo[0].tws_role = GetBtManager()->twsRole;
			btManager.btLinkDeviceInfo[0].remote_profile = 0xff;
			btManager.ddbUpdate = 1;
		}
		else if(btManager.btLinkDeviceInfo[0].tws_role != GetBtManager()->twsRole)
		{
			APP_DBG("[Warm!]tws role changed: %d -> %d\n", btManager.btLinkDeviceInfo[0].tws_role, GetBtManager()->twsRole);
			btManager.btLinkDeviceInfo[0].tws_role = GetBtManager()->twsRole;
			btManager.btLinkDeviceInfo[0].remote_profile = 0xff;
			btManager.ddbUpdate = 1;
		}
		else
		{
			BtDdb_GetTwDeviceLinkKey(&LinkKey[0]);
			if(memcmp(&LinkKey[0], &(btManager.btLinkDeviceInfo[count].device.linkKey[0]), 16) != 0)
			{
				APP_DBG("[Warm!]must update the linkkey\n");
				btManager.ddbUpdate = 1;
			}
			else
			{
				return 0;
			}
		}
	}
	else
	{
		//totalCount = GetCurTotaBtRecNum();
		//ȷ�ϸ��µ��豸��Ϣ���б��е�����
		//if((count+1) != totalCount)
		{
			//�����¼
			memcpy(&record,&(btManager.btLinkDeviceInfo[count].device),sizeof(BT_DB_RECORD));
			
			//���б�����ɾ��֮ǰ�ļ�¼
			DdbDeleteRecord(count);
			
			//����¼���µ��б����
			memcpy(&(btManager.btLinkDeviceInfo[0].device),&record,sizeof(BT_DB_RECORD));
			btManager.btLinkDeviceInfo[0].UsedFlag = 1;
			btManager.btLinkDeviceInfo[0].remote_profile = 0xff;
			btManager.ddbUpdate = 1;
		}
		/*else
		{
			return 0;
		}*/
	}

	if(btManager.ddbUpdate)
	{
		btManager.ddbUpdate = 0;
		//save total device info to flash
		SaveTotalDevRec2Flash(1 + BT_REC_INFO_LEN * MAX_BT_DEVICE_NUM/*one total rec block size*/,
						GetCurTotaBtRecNum());
		
#ifdef PRINT_RECORD_INFOR
		printf("tws infor update\n");
		PrintRecordInfor((const BT_DB_RECORD*)&record);
#endif
		GetBtManager()->twsFlag = 1;
		memcpy(GetBtManager()->btTwsDeviceAddr, &(btManager.btLinkDeviceInfo[0].device.bdAddr), 6);
	}
	return 0;
}

void BtDdb_GetTwDeviceLinkKey(uint8_t *LinkKey)
{
	uint32_t Step = (MVBT_DB_LAST_FLAG_SIZE + 1 + BT_REC_INFO_LEN * MAX_BT_DEVICE_NUM);
	
	uint32_t StartOffset = BTDB_TOTAL_RECORD_ADDR;
	
	StartOffset = FlshGetPairingInfoWriteOffset(StartOffset, Step);
	StartOffset += Step;
	StartOffset += 4;//flag
	StartOffset += 1;//total number

	SpiFlashRead(StartOffset + 6, LinkKey, 16, 1);
}

bool BtDdb_GetTwsDeviceAddr(uint8_t *BtTwsAddr)
{
	bool ret = 0;
	
	if((btManager.btLinkDeviceInfo[0].device.keyType)||(btManager.btLinkDeviceInfo[0].device.pinLen))
	{
		if((btManager.btLinkDeviceInfo[0].remote_profile == 0xff)&&(btManager.btLinkDeviceInfo[0].tws_role != 0xff))
		{
			ret = 1;
			memcpy(BtTwsAddr,btManager.btLinkDeviceInfo[0].device.bdAddr,6);
			btManager.twsRole = btManager.btLinkDeviceInfo[0].tws_role;
		}
	}
	return ret;
}

//���tws infor ��ؼĴ����ļ�¼,�����flash������
bool BtDdb_ClearTwsDeviceRecord(void)
{
	memset(&btManager.btLinkDeviceInfo[0].device, 0, sizeof(BT_DB_RECORD));
	btManager.btLinkDeviceInfo[0].UsedFlag = 0;
	btManager.btLinkDeviceInfo[0].tws_role = BT_TWS_UNKNOW;
	btManager.btLinkDeviceInfo[0].remote_profile = 0xff;

	btManager.twsRole = 0xff;
	btManager.twsFlag = 0;
}

//���DDB��TWS�������Ϣ
void BtDdb_ClrTwsDevInfor(void)
{
	uint32_t StartOffset = BTDB_TOTAL_RECORD_ADDR;
	uint32_t Step = (MVBT_DB_LAST_FLAG_SIZE + 1 + BT_REC_INFO_LEN * MAX_BT_DEVICE_NUM);
	uint8_t tmp[BT_REC_INFO_LEN];
	
	BtDdb_ClearTwsDeviceRecord();
	
	StartOffset = FlshGetPairingInfoWriteOffset(StartOffset, Step);
	StartOffset += Step;
	StartOffset += 4;//flag
	StartOffset += 1;//total number

	memset(tmp, 0, BT_REC_INFO_LEN);
	SpiFlashWrite(StartOffset, tmp, BT_REC_INFO_LEN, 1);
	APP_DBG("clear tws device infor\n");

	btManager.twsFlag = 0;
	btManager.twsRole = 0xff;
}

//���tws infor flag(����ֻ�ǽ����1bytes��0)
bool BtDdb_ClrTwsDevInforFlag(void)
{
	uint8_t Tmp[BT_REC_INFO_LEN];
	uint32_t Step = (MVBT_DB_LAST_FLAG_SIZE + 1 + BT_REC_INFO_LEN * MAX_BT_DEVICE_NUM);
	uint8_t TmpProfile = 0xff;
	
	uint32_t StartOffset = BTDB_TOTAL_RECORD_ADDR;
	
	StartOffset = FlshGetPairingInfoWriteOffset(StartOffset, Step);
	StartOffset += Step;
	StartOffset += 4;//flag
	StartOffset += 1;//total number

	SpiFlashRead(StartOffset, Tmp, BT_REC_INFO_LEN, 1);

	TmpProfile = 0;
	SpiFlashWrite(StartOffset + BT_REC_INFO_LEN - 1, &TmpProfile, 1, 1);
	APP_DBG("clear tws device infor flag\n");

	btManager.twsFlag = 0;
	btManager.twsRole = 0xff;
	
	return 1;
}

#if 1
bool BtDdb_ClearTwsDeviceAddrList(void)
{
	memset(btManager.btTwsDeviceAddr, 0, 6);
	BtDdb_ClrTwsDevInforFlag();
	
	return 1;
}
#else
bool BtDdb_ClearTwsDeviceAddrList(void)
{
	memset(btManager.btTwsDeviceAddr, 0, 6);
	
	if((btManager.btLinkDeviceInfo[0].device.keyType)||(btManager.btLinkDeviceInfo[0].device.pinLen))
	{
		memset(&btManager.btLinkDeviceInfo[0].device, 0, sizeof(BT_DB_RECORD));
		btManager.btLinkDeviceInfo[0].UsedFlag = 0;
		btManager.btLinkDeviceInfo[0].tws_role = BT_TWS_UNKNOW;
		btManager.btLinkDeviceInfo[0].remote_profile = 0xff;

		SaveTotalDevRec2Flash(1 + BT_REC_INFO_LEN * MAX_BT_DEVICE_NUM/*one total rec block size*/,
						GetCurTotaBtRecNum());
		
		APP_DBG("tws infor clear\n");
	}
	return 1;
}
#endif // 1
#else
bool BtDdb_ClearTwsDeviceAddrList(void)
{
	return 0;
}

#endif//#ifdef BT_TWS_SUPPORT

/****************************************************************************
 *  ��ѯflash�б�����������ݵ�offset
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
 *  ��ȡlist�ж�Ӧindex���豸��Ϣ
 ****************************************************************************/
static uint32_t Get1of8RecInfo(uint8_t RecIdx/*from 0*/, uint8_t *Data/*size must be no less than 23B*/)
{
    if(RecIdx >= GetCurTotaBtRecNum())return 0; //not found
    
    memcpy(Data,btManager.btLinkDeviceInfo[RecIdx].device.bdAddr,6);
    memcpy(Data+6,btManager.btLinkDeviceInfo[RecIdx].device.linkKey,16);
    Data[22] = (((btManager.btLinkDeviceInfo[RecIdx].device.trusted & 0x01) << 7) |
                ((btManager.btLinkDeviceInfo[RecIdx].device.keyType & 0x07) << 4) |
                ((btManager.btLinkDeviceInfo[RecIdx].device.pinLen  & 0x0F)));
#ifdef BT_TWS_SUPPORT
	if(RecIdx == 0)
	{
		Data[23] = GetBtManager()->twsRole;
	}
	else
	{
		Data[23] = 0;
	}

	Data[24] = btManager.btLinkDeviceInfo[RecIdx].remote_profile;
#endif //defined(BT_TWS_SUPPORT)

#ifdef FLASH_SAVE_REMOTE_BT_NAME
#ifdef BT_TWS_SUPPORT
	memcpy(Data + 25, btManager.btLinkDeviceInfo[RecIdx].device.bdName, 40);
#else
	memcpy(Data + 23, btManager.btLinkDeviceInfo[RecIdx].device.bdName, 40);
#endif
#endif

    return 1;
}

/****************************************************************************
 *  ����List�����е������豸��Ϣ��flash
 ****************************************************************************/
void SaveTotalDevRec2Flash(int OneFullRecBlockSize, int TotalRecNum)
{
	uint8_t i, Tmp[5];
	uint32_t StartOffset = BTDB_TOTAL_RECORD_ADDR;
	uint8_t OneBtRec[BT_REC_INFO_LEN];

    OneFullRecBlockSize += MVBT_DB_FLAG_SIZE;//including 3Bytes of sync data,zhouyi,20140411

	StartOffset = FlshGetPairingInfoWriteOffset(StartOffset, OneFullRecBlockSize);
	APP_DBG("-----------------------------------New0x%lx(%d)\n", StartOffset, StartOffset);
	Tmp[0] = 'M';
	Tmp[1] = 'V';
	Tmp[2] = 'B';
	Tmp[3] = 'T';
	//total num
	Tmp[4] = TotalRecNum & 0xFF;
	SpiFlashWrite(StartOffset, Tmp, 5, 1);
	APP_DBG("TOTAL NUM:%d\n", TotalRecNum);
	StartOffset += 5;
	
	for(i = 0; i < TotalRecNum; i++)
	{
		//get data
		if(Get1of8RecInfo(i, OneBtRec))
		{
#ifdef PRINT_RECORD_INFOR            
			ShowOneDevMemoryPairedInfo(i,OneBtRec,BT_REC_INFO_LEN);
#endif
			if(i == (TotalRecNum - 1))
			{
				APP_DBG("Update the profile!!!\n");
				OneBtRec[24] = ~btManager.btDdbLastProfile;	//�½����remote device����profile
			}
			SpiFlashWrite(StartOffset, OneBtRec, BT_REC_INFO_LEN, 1);
			StartOffset += BT_REC_INFO_LEN;
#ifdef PRINT_RECORD_INFOR            
            ShowOneDevSavedPairingInfo(i,StartOffset - BT_REC_INFO_LEN,BT_REC_INFO_LEN);
#endif            
		}
	}
}

