///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: bt_app_func.c
//  maintainer: Halley
///////////////////////////////////////////////////////////////////////////////

#include "type.h"
#include "delay.h"
#include "debug.h"

#include "chip_info.h"

#include "bt_stack_api.h"
#include "bt_config.h"
#include "bt_app_interface.h"
#include "bt_manager.h"
#include "bt_app_func.h"
#include "bt_platform_interface.h"
#include "bt_ddb_flash.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

#include "bt_a2dp_api.h"
#include "bt_avrcp_api.h"
#include "bt_obex_api.h"

#include "bt_config.h"
#include "bb_api.h"

#include "rtos_api.h"

#ifdef CFG_FUNC_AI_EN
#include "ai.h"
#endif

extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;
extern uint8_t CheckMatch(uint8_t *bdAddr);

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
			APP_DBG("efuse is null\n");
			//2.generate a random address
			{
				//uint8_t addr[6] = BT_ADDRESS;
				random_mac = Chip_RandomSeedGet();
				*(devAddr+2) = (uint8_t)(random_mac&0xff);
				*(devAddr+3) = (uint8_t)((random_mac>>8)&0xff);
				*(devAddr+4) = (uint8_t)((random_mac>>16)&0xff);
				*(devAddr+5) = (uint8_t)((random_mac>>24)&0xff);
			}
		}
	}

	CheckMatch(devAddr);
	
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
	
#if BT_HID_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_HID;
#endif
	
#if BT_MFI_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_MFI;
#endif
	
#if BT_OBEX_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_OBEX;
#endif

#if BT_PBAP_SUPPORT == ENABLE
	profiles |= BT_PROFILE_SUPPORTED_PBAP;
#endif

#ifdef BT_TWS_SUPPORT
	profiles |= BT_PROFILE_SUPPORTED_TWS;
#endif

	return profiles;
}

/***********************************************************************************
 * 
 **********************************************************************************/
void prinfBtConfigParams(void)
{
	uint8_t i;
	APP_DBG("**********\nLocal Device Infor:\n");

	//bluetooth name and address
	APP_DBG("Bt Name:%s\n", btStackConfigParams->bt_LocalDeviceName);
	APP_DBG("FlashBtAddress(NAP-UAP-LAP):");
	for(i=0;i<5;i++)
	{
		APP_DBG("%02x:", btStackConfigParams->bt_LocalDeviceAddr[i]);
	}
	APP_DBG("%02x\n", btStackConfigParams->bt_LocalDeviceAddr[5]);
	
	APP_DBG("SystemBtAddress(LAP-UAP-NAP):");
	for(i=0;i<5;i++)
	{
		APP_DBG("%02x:", btManager.btDevAddr[i]);
	}
	APP_DBG("%02x\n", btManager.btDevAddr[5]);
	
	//ble address
	APP_DBG("BleAddress:");
	for(i=0;i<5;i++)
	{
		APP_DBG("%02x:", btStackConfigParams->ble_LocalDeviceAddr[i]);
	}
	APP_DBG("%02x\n", btStackConfigParams->ble_LocalDeviceAddr[5]);

	APP_DBG("Freq trim:0x%x\n", btStackConfigParams->bt_trimValue);
	APP_DBG("**********\n");
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
		APP_DBG("bt addr error\n");
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
		APP_DBG("bt name error\n");
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
		APP_DBG("header error\n");
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
			APP_DBG("bt database read error!\n");
			return;
		}
	}
	
	//BT ADDR
	ret = CheckBtAddr(btStackConfigParams->bt_LocalDeviceAddr);
	if(ret != 0)
	{
		paramsUpdate = 1;
		GetBtDefaultAddr(btStackConfigParams->bt_LocalDeviceAddr);
	}
	if(BT_MATCH_CHECK)
	{
		ret = CheckMatch(btStackConfigParams->bt_LocalDeviceAddr);
		if(ret != 0)
		{
			paramsUpdate = 1;
		}
	}
	
	//将ADDR进行反序操作:btStackConfigParams->bt_LocalDeviceAddr(NAP-UAP-LAP)->btManager.btDevAddr(LAP-UAP-NAP)
	btManager.btDevAddr[5]=btStackConfigParams->bt_LocalDeviceAddr[0];
	btManager.btDevAddr[4]=btStackConfigParams->bt_LocalDeviceAddr[1];
	btManager.btDevAddr[3]=btStackConfigParams->bt_LocalDeviceAddr[2];
	btManager.btDevAddr[2]=btStackConfigParams->bt_LocalDeviceAddr[3];
	btManager.btDevAddr[1]=btStackConfigParams->bt_LocalDeviceAddr[4];
	btManager.btDevAddr[0]=btStackConfigParams->bt_LocalDeviceAddr[5];
	
	//BLE ADDR
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
#ifdef	CFG_XIAOAI_AI_EN
	//小爱要求同地址
	{
#if 0
		int i = 0;
		for(i=0;i < 6;i++)
		{
			btStackConfigParams->ble_LocalDeviceAddr[i] = btStackConfigParams->bt_LocalDeviceAddr[5 - i];
		}
#else
		memcpy(btStackConfigParams->ble_LocalDeviceAddr, btStackConfigParams->bt_LocalDeviceAddr, 6);
#endif
	}

#endif

#ifdef BT_TWS_SUPPORT
	//BT name 蓝牙名称按照BT_NAME
	strcpy((void *)btStackConfigParams->bt_LocalDeviceName, BT_NAME);
#else
	//BT name 蓝牙名称从flash中获取
	ret = CheckBtName(btStackConfigParams->bt_LocalDeviceName);
	if(ret != 0)
	{
		paramsUpdate = 1;
		strcpy((void *)btStackConfigParams->bt_LocalDeviceName, BT_NAME);
	}
#endif //#if defined(BT_TWS_SUPPORT)
#ifdef	CFG_XIAOAI_AI_EN
	strcpy((void *)btStackConfigParams->bt_LocalDeviceName, BT_NAME);
#endif
#ifdef	CFG_FUNC_AI_EN
	strcpy((void *)btStackConfigParams->bt_LocalDeviceName, BT_NAME);
#endif

	//BLE name 蓝牙名称按照BT_NAME
	//strcpy((void *)btStackConfigParams->ble_LocalDeviceName, BLE_NAME);
	//BLE name 蓝牙名称从flash中获取
	ret = CheckBtName(btStackConfigParams->ble_LocalDeviceName);
	if(ret != 0)
	{
		paramsUpdate = 1;
		strcpy((void *)btStackConfigParams->ble_LocalDeviceName, BT_NAME);
	}
#ifdef	CFG_XIAOAI_AI_EN
	strcpy((void *)btStackConfigParams->ble_LocalDeviceName, BT_NAME);
#endif
#ifdef	CFG_FUNC_AI_EN
	strcpy((void *)btStackConfigParams->ble_LocalDeviceName, BT_NAME);
#endif
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
		//蓝牙公共配置参数,暂时按照宏定义默认的参数进行配置 bt_config.h,频偏值保留flash中数据
		//如后续有需要能动态调整,或者上位机工具修改的,就需要保存到flash中进行管理
		btStackConfigParams->bt_TxPowerValue = BT_TX_POWER_LEVEL;
		
		btStackConfigParams->bt_SupportProfile = GetSupportProfiles();
		
		//simple Pairing enable
		//当simplePairing=1时,pinCode无效;反之亦然
		btStackConfigParams->bt_simplePairingFunc = BT_SIMPLEPAIRING_FLAG;
		btStackConfigParams->bt_pinCodeLen = BT_PINCODE_LEN;
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
		APP_DBG("save bt params to flash\n");
		//保存默认的配置参数到flash，但不包括bt_ConfigHeader及bt_trimValue
		BtDdb_InitBtConfigurationParams(btStackConfigParams);
	}
	
#ifdef CFG_FUNC_AI_EN
//	ai_ble_set((char*)btStackConfigParams->bt_LocalDeviceName,btStackConfigParams->bt_LocalDeviceAddr,btStackConfigParams->bt_LocalDeviceAddr);
	ai_ble_set((char*)btStackConfigParams->bt_LocalDeviceName,btManager.btDevAddr,btManager.btDevAddr);
#endif
	prinfBtConfigParams();
}

/***********************************************************************************
 * 配置BB的参数
 **********************************************************************************/
void ConfigBtBbParams(BtBbParams *params)
{
	uint8_t pTxPower = BT_TX_POWER_LEVEL;
	uint8_t pPageTxPower = BT_PAGE_TX_POWER_LEVEL;
	if(params == NULL)
		return;

	memset(params, 0 ,sizeof(BtBbParams));

	params->localDevName = (uint8_t *)btStackConfigParams->bt_LocalDeviceName;
	memcpy(params->localDevAddr, btManager.btDevAddr, BT_ADDR_SIZE);
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

	SetRfTxPwrMaxLevel(pTxPower, pPageTxPower);
	
	//BtSetLinkSupervisionTimeout(0x1F40); //5s
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
	if((btStackConfigParams->bt_pinCodeLen)&&(btStackConfigParams->bt_pinCodeLen<17))
	{
		stackParams->btPinCodeLen = btStackConfigParams->bt_pinCodeLen;
		memcpy(stackParams->btPinCode, btStackConfigParams->bt_pinCode, stackParams->btPinCodeLen);
	}
	else
	{
		APP_DBG("ERROR:pin code len %d\n", btStackConfigParams->bt_pinCodeLen);
		stackParams->btSimplePairing = 1;//开启简易配对
	}

	//class of device
	//headset:苹果手机能显示设备的电池电量
#if (BT_SIMPLEPAIRING_FLAG == ENABLE)
	pCod = COD_AUDIO | COD_MAJOR_AUDIO | COD_MINOR_AUDIO_HEADSET | COD_RENDERING;
#else
	//Note:1.苹果手机连BP10的HFP协议,如需要有同步通讯录的弹框,则需要把设备类型配置为handsfree,但不能显示电池电量
	//2.使用pin code的话,必须要将class of device配置为handsfree,否则某些手机不支持
	pCod = COD_AUDIO | COD_MAJOR_AUDIO | COD_MINOR_AUDIO_HANDSFREE | COD_RENDERING;
#endif

	//假如不需要HFP,客户不想手机连接时弹出电话本的权限获取,则需要配置为 HIFI audio
	//pCod = COD_AUDIO | COD_MAJOR_AUDIO | COD_MINOR_AUDIO_HIFIAUDIO | COD_RENDERING;
	
#if (BT_HID_SUPPORT == ENABLE)
	pCod |= (COD_MAJOR_PERIPHERAL | COD_MINOR_PERIPH_KEYBOARD);
#endif
	SetBtClassOfDevice(pCod);

#ifdef BT_TWS_SUPPORT
	stackParams->twsFeatures.twsAppCallback = BtTwsCallback;
	stackParams->twsFeatures.twsSimplePairingCfg = TWS_SIMPLE_PAIRING_SUPPORT;
	stackParams->twsFeatures.twsRoleCfg = TWS_PAIRING_MODE;
	GetBtManager()->twsSimplePairingCfg = TWS_SIMPLE_PAIRING_SUPPORT;

#ifdef TWS_FILTER_USER_DEFINED
	strcpy((void *)btManager.TwsFilterInfor, TWS_FILTER_INFOR);
#endif
#endif//defined(BT_TWS_SUPPORT)

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
	//stackParams->a2dpFeatures.a2dpAudioDataFormat = BT_A2DP_AUDIO_DATA;
#if (defined(BT_AUDIO_AAC_ENABLE) && defined(USE_AAC_DECODER))
	stackParams->a2dpFeatures.a2dpAudioDataFormat = (A2DP_AUDIO_DATA_SBC | A2DP_AUDIO_DATA_AAC);
#else
	stackParams->a2dpFeatures.a2dpAudioDataFormat = (A2DP_AUDIO_DATA_SBC);
#endif

	stackParams->a2dpFeatures.a2dpAppCallback = BtA2dpCallback;
#else
	stackParams->a2dpFeatures.a2dpAppCallback = NULL;
#endif
	
#if BT_AVRCP_SUPPORT == ENABLE
	/* AVRCP features */
	stackParams->avrcpFeatures.supportAdvanced = BT_AVRCP_ADVANCED;
#if ((BT_AVRCP_PLAYER_SETTING == ENABLE)||(BT_AVRCP_VOLUME_SYNC == ENABLE))
	stackParams->avrcpFeatures.supportTgSide = 1;
#else
	stackParams->avrcpFeatures.supportTgSide = 0;
#endif
	stackParams->avrcpFeatures.supportSongTrackInfo = BT_AVRCP_SONG_TRACK_INFOR;
	stackParams->avrcpFeatures.supportPlayStatusInfo = BT_AVRCP_SONG_PLAY_STATE;
	stackParams->avrcpFeatures.avrcpAppCallback = BtAvrcpCallback;
#else
	stackParams->avrcpFeatures.avrcpAppCallback = NULL;
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
#ifndef BT_TWS_SUPPORT
	memset(GetBtManager(), 0, sizeof(BT_MANAGER_ST));
#endif
	BTStatckSetPageTimeOutValue(BT_PAGE_TIMEOUT); 

	ConfigBtStackParams(&stackParams);

	retInit = BTStackRunInit(&stackParams);
	if(retInit != 0)
	{
		APP_DBG("Bt Stack Init ErrCode [%x]\n", (int)retInit);
		return FALSE;
	}
	
#if BT_SPP_SUPPORT == ENABLE
	retInit = SppAppInit(BtSppCallback);
	if(retInit == 0)
	{
		APP_DBG("Spp Init Error!\n");
		return FALSE;
	}
#endif

#if BT_HID_SUPPORT == ENABLE
	retInit = HidAppInit(BtHidCallback);
	if(retInit == 0)
	{
		APP_DBG("Hid Init Error!\n");
		return FALSE;
	}
#endif

#if BT_MFI_SUPPORT == ENABLE
	retInit = MFiAppInit(BtMfiCallback);
	if(retInit == 0)
	{
		APP_DBG("MFi Init Error!\n");
		return FALSE;
	}
#endif
	
#if BT_OBEX_SUPPORT == ENABLE
	retInit = ObexAppInit(BtObexCallback);
	if(retInit == 0)
	{
		APP_DBG("Obex Init Error!\n");
		return FALSE;
	}
#endif
	
#if BT_PBAP_SUPPORT == ENABLE
	retInit = PbapAppInit(BtPbapCallback);
	if(retInit == 0)
	{
		APP_DBG("Pbap Init Error!\n");
		return FALSE;
	}
#endif

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


/***********************************************************************************
 * get device name (max 40bytes)
 **********************************************************************************/
uint8_t* BtDeviceNameGet(void)
{
	return &btStackConfigParams->bt_LocalDeviceName[0];
}
/***********************************************************************************
 * 更新 蓝牙名称到flash
 **********************************************************************************/
int32_t BtDeviceNameSet(uint8_t* deviceName, uint8_t deviceLen)
{
	BT_CONFIGURATION_PARAMS		*btParams = NULL;
	int8_t ret=0;
	APP_DBG("device name set!\n");

	//1.申请RAM
	btParams = (BT_CONFIGURATION_PARAMS*)osPortMalloc(sizeof(BT_CONFIGURATION_PARAMS));
	if(btParams == NULL)
	{
		APP_DBG("ERROR: Ram is not enough!\n");
		return -2;//RAM不够
	}
	memcpy(btParams, btStackConfigParams, sizeof(BT_CONFIGURATION_PARAMS));
	
	//2.将pin code更新
	memset(btStackConfigParams->bt_LocalDeviceName, 0, BT_NAME_SIZE);
	memset(btParams->bt_LocalDeviceName, 0, BT_NAME_SIZE);
	
	memcpy(btStackConfigParams->bt_LocalDeviceName, deviceName, deviceLen);
	memcpy(btParams->bt_LocalDeviceName, deviceName, deviceLen);

	//3.将更新数据保存到flash
	BtDdb_SaveBtConfigurationParams(btParams);
	memset(btParams, 0, sizeof(BT_CONFIGURATION_PARAMS));

	//4.重新从flash中读取数据，再次进行对比
	ret = BtDdb_LoadBtConfigurationParams(btParams);
	if(ret == -3)
	{
		//读取异常，read again
		ret = BtDdb_LoadBtConfigurationParams(btParams);
		if(ret == -3)
		{
			APP_DBG("bt database read error!\n");
			osPortFree(btParams);
			return -3;//读取失败
		}
	}

	ret = memcmp(btStackConfigParams, btParams,sizeof(BT_CONFIGURATION_PARAMS));
	if(ret == 0)
	{
		APP_DBG("save ok!\n");
		osPortFree(btParams);
		return 0;
	}
	else
	{
		APP_DBG("save NG!\n");
		osPortFree(btParams);
		return -4;//保存失败
	}

}

/***********************************************************************************
 * get device ble name (max 40bytes)
 **********************************************************************************/
uint8_t* BtDeviceBleNameGet(void)
{
	return &btStackConfigParams->ble_LocalDeviceName[0];
}
/***********************************************************************************
 * 更新 蓝牙BLE名称到flash
 **********************************************************************************/
int32_t BtDeviceBleNameSet(uint8_t* deviceName, uint8_t deviceLen)
{
	BT_CONFIGURATION_PARAMS		*btParams = NULL;
	int8_t ret=0;
	APP_DBG("device ble name set!\n");

	//1.申请RAM
	btParams = (BT_CONFIGURATION_PARAMS*)osPortMalloc(sizeof(BT_CONFIGURATION_PARAMS));
	if(btParams == NULL)
	{
		APP_DBG("ERROR: Ram is not enough!\n");
		return -2;//RAM不够
	}
	memcpy(btParams, btStackConfigParams, sizeof(BT_CONFIGURATION_PARAMS));
	
	//2.将pin code更新
	memset(btStackConfigParams->ble_LocalDeviceName, 0, BT_NAME_SIZE);
	memset(btParams->ble_LocalDeviceName, 0, BT_NAME_SIZE);
	
	memcpy(btStackConfigParams->ble_LocalDeviceName, deviceName, deviceLen);
	memcpy(btParams->ble_LocalDeviceName, btStackConfigParams->ble_LocalDeviceName, deviceLen);

	//3.将更新数据保存到flash
	BtDdb_SaveBtConfigurationParams(btParams);
	memset(btParams, 0, sizeof(BT_CONFIGURATION_PARAMS));

	//4.重新从flash中读取数据，再次进行对比
	ret = BtDdb_LoadBtConfigurationParams(btParams);
	if(ret == -3)
	{
		//读取异常，read again
		ret = BtDdb_LoadBtConfigurationParams(btParams);
		if(ret == -3)
		{
			APP_DBG("bt database read error!\n");
			osPortFree(btParams);
			return -3;//读取失败
		}
	}

	ret = memcmp(btStackConfigParams, btParams,sizeof(BT_CONFIGURATION_PARAMS));
	if(ret == 0)
	{
		APP_DBG("save ok!\n");
		osPortFree(btParams);
		return 0;
	}
	else
	{
		APP_DBG("save NG!\n");
		osPortFree(btParams);
		return -4;//保存失败
	}

}

/***********************************************************************************
 * pin code get (4bytes)
 **********************************************************************************/
uint8_t* BtPinCodeGet(void)
{
	return btStackConfigParams->bt_pinCode;
}

/***********************************************************************************
 * save pin code to flash
 **********************************************************************************/
int32_t BtPinCodeSet(uint8_t *pinCode)
{
	BT_CONFIGURATION_PARAMS		*btParams = NULL;
	int8_t ret=0;
	APP_DBG("pin code set!\n");

	//1.是否开启简易配对
	if(btStackConfigParams->bt_simplePairingFunc)
		return -3;//模式不对

	//2.申请RAM
	btParams = (BT_CONFIGURATION_PARAMS*)osPortMalloc(sizeof(BT_CONFIGURATION_PARAMS));
	if(btParams == NULL)
	{
		APP_DBG("ERROR: Ram is not enough!\n");
		return -2;//RAM不够
	}
	memcpy(btParams, btStackConfigParams, sizeof(BT_CONFIGURATION_PARAMS));
	
	//3.将pin code更新
	memcpy(btStackConfigParams->bt_pinCode, pinCode, 4);
	memcpy(btParams->bt_pinCode, pinCode, 4);

	//4.将更新数据保存到flash
	BtDdb_SaveBtConfigurationParams(btParams);
	memset(btParams, 0, sizeof(BT_CONFIGURATION_PARAMS));

	//5.重新从flash中读取数据，再次进行对比
	ret = BtDdb_LoadBtConfigurationParams(btParams);
	if(ret == -3)
	{
		//读取异常，read again
		ret = BtDdb_LoadBtConfigurationParams(btParams);
		if(ret == -3)
		{
			APP_DBG("bt database read error!\n");
			osPortFree(btParams);
			return -3;//读取失败
		}
	}

	ret = memcmp(btStackConfigParams, btParams,sizeof(BT_CONFIGURATION_PARAMS));
	if(ret == 0)
	{
		APP_DBG("save ok!\n");
		osPortFree(btParams);
		return 0;
	}
	else
	{
		APP_DBG("save NG!\n");
		osPortFree(btParams);
		return -4;//保存失败
	}
}

