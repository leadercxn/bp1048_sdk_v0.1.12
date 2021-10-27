///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: bt_app_func.c
//  maintainer: Halley
///////////////////////////////////////////////////////////////////////////////

#include "bt_app_func.h"

#include "type.h"
#include "delay.h"
#include "debug.h"

//#include "app_config.h"
#include "chip_info.h"

#include "bt_app_interface.h"
#include "bt_ddb_flash.h"
#include "bt_manager.h"
#include "bt_platform_interface.h"
#include "bt_stack_api.h"
#include "bt_config.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

#include "bt_a2dp_api.h"
#include "bt_avrcp_api.h"

#include "bt_config.h"
#include "bb_api.h"

#ifdef CFG_FUNC_AI
#include "ai.h"
#endif

extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;

/***********************************************************************************
 * 
 **********************************************************************************/
/**
 * @brief	Get Bt Address
 * @param	device Address
 * 			mode: 1=from efuse  0=default
 * @return	
 */
extern uint8_t Efuse_ReadData(uint8_t Addr);
static bool GetBtDefaultAddr(uint8_t *devAddr)
{
	uint8_t i;
	uint32_t sum=0;
	uint32_t random_mac = 0;

	if(devAddr == NULL)
		return FALSE;

	//1.frome efuse 2-6 and sum
	{
		for(i=0;i<5;i++)
		{
			*(devAddr+i) = Efuse_ReadData(2+i);
			sum += *(devAddr+i);
		}
		*(devAddr+5) = (uint8_t)(sum&0x000000ff);

		if((*devAddr == 0)&&(*(devAddr+1) == 0)&&(*(devAddr+2) == 0)&&(*(devAddr+3) == 0)&&(*(devAddr+4) == 0))
		{
			DBG("efuse is null\n");
		}
		else
		{
			return TRUE;
		}
	}

	//2.generate a random address
	{
		//uint8_t addr[6] = BT_ADDRESS;
		random_mac = Chip_RandomSeedGet();
		*devAddr = (uint8_t)(random_mac&0xff);
		*(devAddr+1) = (uint8_t)((random_mac>>8)&0xff);
		*(devAddr+2) = (uint8_t)((random_mac>>16)&0xff);
		*(devAddr+3) = (uint8_t)((random_mac>>24)&0xff);
	}
	
	return TRUE;
}

/***********************************************************************************
 * 
 **********************************************************************************/
uint32_t GetSupportProfiles(void)
{
	uint32_t		profiles = 0;

	
#if BT_HFP_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_HFP;
#endif

#if BT_A2DP_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_A2DP;
#endif

#if BT_AVRCP_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_AVRCP;
#endif

#if BT_SPP_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_SPP;
#endif

	return profiles;
}

/***********************************************************************************
 * 
 **********************************************************************************/
void prinfBtConfigParams(void)
{
	uint8_t i;
	DBG("**********\nLocal Device Infor:\n");

	//bluetooth name and address
	DBG("Bt Name:%s\n", btStackConfigParams->bt_LocalDeviceName);
	DBG("BtAddress:");
	for(i=0;i<5;i++)
	{
		DBG("%x:", btStackConfigParams->bt_LocalDeviceAddr[i]);
	}
	DBG("%x\n",btStackConfigParams->bt_LocalDeviceAddr[5]);
	
	//ble address
	DBG("BleAddress:");
	for(i=0;i<5;i++)
	{
		DBG("%x:", btStackConfigParams->ble_LocalDeviceAddr[i]);
	}
	DBG("%x\n",btStackConfigParams->ble_LocalDeviceAddr[5]);

	DBG("Freq trim:0x%x\n", btStackConfigParams->bt_trimValue);
	DBG("**********\n");
}


/***********************************************************************************
 * 从flash中Load蓝牙配置参数
 **********************************************************************************/
static int8_t CheckBtAddr(uint8_t* addr)
{
	if(((addr[0]==0x00)&&(addr[1]==0x00)&&(addr[2]==0x00)&&(addr[3]==0x00)&&(addr[4]==0x00)&&(addr[5]==0x00))
			|| ((addr[0]==0xff)&&(addr[1]==0xff)&&(addr[2]==0xff)&&(addr[3]==0xff)&&(addr[4]==0xff)&&(addr[5]==0xff)))
	{
		//蓝牙地址不符合预期
		BT_DBG("[DDB]:bt addr error\n");
		return -1;
	}
	else
	{
		return 0;
	}
}

static int8_t CheckBtName(uint8_t* nameString)
{
	if(((nameString[0]==0x00)&&(nameString[1]==0x00)&&(nameString[2]==0x00))
			|| ((nameString[0]==0xff)&&(nameString[1]==0xff)&&(nameString[2]==0xff)))
	{
		//蓝牙名称不符合预期
		BT_DBG("[DDB]:bt name error\n");
		return -1;
	}
	else
	{
		return 0;
	}
}

static int8_t CheckBtParamHeader(uint8_t* header)
{
	if((header[0]=='M')&&(header[1]=='V')&&(header[2]=='B')&&(header[3]=='T'))
	{
		//蓝牙参数header符合预期
		return 0;
	}
	else
	{
		BT_DBG("[DDB]:header error\n");
		return -1;
	}
}

void LoadBtConfigurationParams(void)
{
	int8_t ret = 0;
	uint8_t paramsUpdate = 0;
	if(btStackConfigParams == NULL)
		return;

	ret = BtDdb_LoadBtConfigurationParams(btStackConfigParams);
	if(ret == -3)
	{
		//读取异常，read again
		ret = BtDdb_LoadBtConfigurationParams(btStackConfigParams);
		if(ret == -3)
		{
			BT_DBG("bt database read error!\n");
			return;
		}
	}
	
	btStackConfigParams->bt_LocalDeviceAddr[0]=0x12;
	btStackConfigParams->bt_LocalDeviceAddr[1]=0x78;
	btStackConfigParams->bt_LocalDeviceAddr[2]=0x5A;
	btStackConfigParams->bt_LocalDeviceAddr[3]=0x43;
	btStackConfigParams->bt_LocalDeviceAddr[4]=0x9A;
	btStackConfigParams->bt_LocalDeviceAddr[5]=0x65;
	//BT
	ret = CheckBtAddr(btStackConfigParams->bt_LocalDeviceAddr);
	if(ret != 0)
	{
		paramsUpdate = 1;
		GetBtDefaultAddr(btStackConfigParams->bt_LocalDeviceAddr);
	}

	//BLE
	ret = CheckBtAddr(btStackConfigParams->ble_LocalDeviceAddr);
	if(ret != 0)
	{
		paramsUpdate = 1;
		
		//ble address
		//ble name:通过配置BLE广播信息来配置(ble_app_func.c)
		memcpy(btStackConfigParams->ble_LocalDeviceAddr, btStackConfigParams->bt_LocalDeviceAddr,6);
		btStackConfigParams->ble_LocalDeviceAddr[0] = btStackConfigParams->ble_LocalDeviceAddr[0]|0xc0;
		btStackConfigParams->ble_LocalDeviceAddr[4] += 0x60;
	}

	//BT name 蓝牙名称还是按照bt_config.h的来
	/*ret = CheckBtName(btStackConfigParams->bt_LocalDeviceName);
	if(ret != 0)
	{
		paramsUpdate = 1;
		strcpy((void *)btStackConfigParams->bt_LocalDeviceName, BT_NAME);
	}*/
	strcpy((void *)btStackConfigParams->bt_LocalDeviceName, BT_NAME);

	//BT PARAMS
	ret = CheckBtParamHeader(btStackConfigParams->bt_ConfigHeader);
	if(ret != 0)
	{
		paramsUpdate = 1;
		
		btStackConfigParams->bt_ConfigHeader[0]='M';
		btStackConfigParams->bt_ConfigHeader[1]='V';
		btStackConfigParams->bt_ConfigHeader[2]='B';
		btStackConfigParams->bt_ConfigHeader[3]='T';
		
		//note:在使用默认参数时，trimValue一定不能为0xff，否则会导致bb工作不起来；
		btStackConfigParams->bt_trimValue = BT_TRIM;

	#if 0
		btStackConfigParams->bt_TxPowerValue = BT_TX_POWER_LEVEL;
		
		btStackConfigParams->bt_SupportProfile = GetSupportProfiles();
		
		//simple Pairing enable
		//当simplePairing=1时,pinCode无效;反之亦然
		btStackConfigParams->bt_simplePairingFunc = 1;
		strcpy((char*)btStackConfigParams->bt_pinCode,BT_PINCODE);
		
		//inquiry scan params
		btStackConfigParams->bt_InquiryScanInterval = BT_INQUIRYSCAN_INTERVAL;
		btStackConfigParams->bt_InquiryScanWindow = BT_INQUIRYSCAN_WINDOW;
		
		//page scan params
		btStackConfigParams->bt_PageScanInterval = BT_PAGESCAN_INTERVAL;
		btStackConfigParams->bt_PageScanWindow = BT_PAGESCAN_WINDOW;
	#endif
	}
	{
		btStackConfigParams->bt_trimValue = BT_TRIM;//0x0A;
		//除了频偏值保留flash中数据，其他参数恢复成bt_config.h默认参数
		btStackConfigParams->bt_TxPowerValue = BT_TX_POWER_LEVEL;
		
		btStackConfigParams->bt_SupportProfile = GetSupportProfiles();
		
		//simple Pairing enable
		//当simplePairing=1时,pinCode无效;反之亦然
		btStackConfigParams->bt_simplePairingFunc = 1;
		strcpy((char*)btStackConfigParams->bt_pinCode,BT_PINCODE);
		
		//inquiry scan params
		btStackConfigParams->bt_InquiryScanInterval = BT_INQUIRYSCAN_INTERVAL;
		btStackConfigParams->bt_InquiryScanWindow = BT_INQUIRYSCAN_WINDOW;
		
		//page scan params
		btStackConfigParams->bt_PageScanInterval = BT_PAGESCAN_INTERVAL;
		btStackConfigParams->bt_PageScanWindow = BT_PAGESCAN_WINDOW;
	}

	if(paramsUpdate)
	{
		BT_DBG("[DDB]:save bt params to flash\n");
		//保存默认的配置参数到flash
		BtDdb_SaveBtConfigurationParams(btStackConfigParams);
	}
	
	#ifdef CFG_FUNC_AI
	ai_ble_set(btStackConfigParams->bt_LocalDeviceName,btStackConfigParams->bt_LocalDeviceAddr,btStackConfigParams->ble_LocalDeviceAddr);
	#endif
	prinfBtConfigParams();

}

/***********************************************************************************
 * 配置BB的参数
 **********************************************************************************/
void ConfigBtBbParams(BtBbParams *params)
{
	uint8_t pTxPower = BT_TX_POWER_LEVEL;
	if(params == NULL)
		return;

	memset(params, 0 ,sizeof(BtBbParams));

	params->localDevName = (uint8_t *)btStackConfigParams->bt_LocalDeviceName;
	memcpy(params->localDevAddr, btStackConfigParams->bt_LocalDeviceAddr, BT_ADDR_SIZE);
	params->freqTrim = btStackConfigParams->bt_trimValue;

	//em config
	params->em_start_addr = BB_EM_START_PARAMS; //0x40:20010000; 0x80:20020000; 0xc0:20030000 ;0x100: 0x20040000; 0x120:0x20048000

	//agc config
	params->pAgcDisable = 0; //0=auto agc;  1=close agc
	params->pAgcLevel = 1;

	//sniff config
	params->pSniffNego = 0;//1=open;  0=close
	params->pSniffDelay = 0;
	params->pSniffInterval = 0x320;//500ms
	params->pSniffAttempt = 0x01;
	params->pSniffTimeout = 0x01;

	params->bbSniffNotify = NULL;

	SetRfTxPwrMaxLevel(pTxPower);
}


/***********************************************************************************
 * 配置HOST的参数
 **********************************************************************************/
void ConfigBtStackParams(BtStackParams *stackParams)
{
	uint32_t pCod = 0;
	if(stackParams == NULL)
		return ;

	memset(stackParams, 0 ,sizeof(BtStackParams));

	/* Set support profiles */
	stackParams->supportProfiles = GetSupportProfiles();

	/* Set local device name */
	stackParams->localDevName = (uint8_t *)btStackConfigParams->bt_LocalDeviceName;

	/* Set callback function */
	stackParams->callback = BtStackCallback;
	stackParams->scoCallback = NULL;//GetScoDataFromApp;

	//simple pairing
	stackParams->btSimplePairing = btStackConfigParams->bt_simplePairingFunc;
	memcpy(stackParams->btPinCode, btStackConfigParams->bt_pinCode, 4);

	//class of device
	pCod = COD_AUDIO | COD_MAJOR_AUDIO | COD_MINOR_AUDIO_HEADSET | COD_RENDERING;
	SetBtClassOfDevice(pCod);

#if BT_HFP_SUPPORT == ENABLE
	/* HFP features */
	stackParams->hfpFeatures.wbsSupport = BT_HFP_SUPPORT_WBS;
	stackParams->hfpFeatures.hfpAudioDataFormat = BT_HFP_AUDIO_DATA;
	stackParams->hfpFeatures.hfpAppCallback = BtHfpCallback;
#else
	stackParams->hfpFeatures.hfpAppCallback = NULL;
#endif
	
#if BT_A2DP_SUPPORT == ENABLE
	/* A2DP features */
	stackParams->a2dpFeatures.a2dpAudioDataFormat = BT_A2DP_AUDIO_DATA;
	stackParams->a2dpFeatures.a2dpAppCallback = BtA2dpCallback;
#else
	stackParams->a2dpFeatures.a2dpAppCallback = NULL;
#endif
	
#if BT_AVRCP_SUPPORT == ENABLE
	/* AVRCP features */
	stackParams->avrcpFeatures.supportAdvanced = BT_AVRCP_ADVANCED;
	stackParams->avrcpFeatures.supportTgSide = BT_AVRCP_VOLUME_SYNC;
	stackParams->avrcpFeatures.supportSongTrackInfo = BT_AVRCP_SONG_TRACK_INFOR;
	stackParams->avrcpFeatures.supportPlayStatusInfo = BT_AVRCP_SONG_PLAY_STATE;
	stackParams->avrcpFeatures.avrcpAppCallback = BtAvrcpCallback;
#else
	stackParams->avrcpFeatures.avrcpAppCallback = NULL;
#endif

#if BT_SPP_SUPPORT == ENABLE
	stackParams->SppAppFeatures.sppAppCallback = BtSppCallback;
	stackParams->SppAppFeatures.sppAppHook = BtSppHookFunc;
#else
	stackParams->SppAppFeatures.sppAppCallback = NULL;
#endif
}

/***********************************************************************************
 * 初始化蓝牙HOST
 **********************************************************************************/
bool BtStackInit(void)
{
//	bool ret;
	int32_t retInit=0;

	BtStackParams	stackParams;

	memset(GetBtManager(), 0, sizeof(BT_MANAGER_ST));

	BTStatckSetPageTimeOutValue(BT_PAGE_TIMEOUT); 

	ConfigBtStackParams(&stackParams);

	retInit = BTStackRunInit(&stackParams);
	if(retInit != 0)
	{
		BT_DBG("Bt Stack Init ErrCode [%x]\n", (int)retInit);
		return FALSE;
	}

	return TRUE;
}

/***********************************************************************************
 * 
 **********************************************************************************/
void UninitBt(void)
{
	BTStackRunUninit();
}

bool BtStackUninit(void)
{
	int32_t ret=0;
	UninitBt();
	
	ret = BTStackMemFree();
	if(ret == -1)
	{
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************
 * 
 **********************************************************************************/
void BtStackRunNotify(void)
{
//	OSQueueMsgSend(MSG_NEED_BT_STACK_RUN, NULL, 0, MSGPRIO_LEVEL_HI, 0);
}


