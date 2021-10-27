/**
 **************************************************************************************
 * @file    bt_stack_service.c
 * @brief   
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2018-2-9 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#include <string.h>
#include "type.h"
//#include "app_config.h"
#include "gpio.h" //for BOARD
#include "debug.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "dma.h"
#include "timeout.h"
#include "irqn.h"
#include "ble_api.h"
#include "ble_app_func.h"

#include "clk.h"
#include "reset.h"
#include "bb_api.h"
#include "bt_app_func.h"
#include "bt_app_interface.h"
#include "bt_avrcp_api.h"
#include "bt_manager.h"
#include "bt_pbap_api.h"
#include "bt_platform_interface.h"
#include "bt_stack_api.h"
#include "app_message.h"
#include "bt_config.h"

#include "bt_stack_service.h"
#ifdef CFG_FUNC_AI
#include "ai.h"
#endif

//#ifdef CFG_APP_BT_MODE_EN

//#ifdef CFG_BT_BACKGROUND_RUN_EN
uint8_t gBtHostStackMemHeap[BT_STACK_MEM_SIZE];
//#endif

//BR/EDR STACK SERVICE
#define BT_STACK_SERVICE_STACK_SIZE		768
#define BT_STACK_SERVICE_PRIO			3
#define BT_STACK_NUM_MESSAGE_QUEUE		10

//USER SERVICE //用来处理协议栈callback中用户需要处理的msg
#define BT_USER_SERVICE_STACK_SIZE		256
#define BT_USER_SERVICE_PRIO			3
#define BT_USER_NUM_MESSAGE_QUEUE		10

typedef struct _BtStackServiceContext
{
//	xTaskHandle			taskHandle;
//	MessageHandle		msgHandle;
	TaskState			serviceState;

	uint8_t				serviceWaitResume;	//1:蓝牙不在后台运行时,开启通话,退出播放模式,不能kill蓝牙协议栈

	uint8_t				bbErrorMode;
	uint8_t				bbErrorType;
}BtStackServiceContext;

static BtStackServiceContext	*btStackServiceCt = NULL;

BT_CONFIGURATION_PARAMS		*btStackConfigParams = NULL;

//extern uint8_t bt_powerup_flag;

////////////////////////////////////////////////////////////////////////////////////
void update_btDdb(uint8_t addr)
{
	btStackConfigParams->bt_LocalDeviceAddr[3] = addr;
	BtDdb_SaveBtConfigurationParams(btStackConfigParams);
	BtDdb_Erase();
}

void BBMatchReport(void)
{
}

void SendDeepSleepMsg(void)
{
}

void WakeupBtStackService(void)
{
}

void BtFreqOffsetAdjustComplete(unsigned char offset)
{
}

void BtCancelConnect(void)
{
}

void BtCntClkSet(void)
{
	Clock_BTDMClkSelect(OSC_32K_MODE);
	Clock_OSC32KClkSelect(HOSC_DIV_32K_CLK_MODE);
	Clock_32KClkDivSet(Clock_OSCClkDivGet());  //如果这里出现问题，请检查SystemClockInit，不要在这里直接改——Tony
	Clock_BBCtrlHOSCInDeepsleep(0);//禁止baseband进入sniff后硬件自动关闭HOSC 24M
}

void BBErrorReport(uint8_t mode, uint32_t errorType)
{
}
////////////////////////////////////////////////////////////////////////////////////

unsigned char BT_Stack_Service_Context[sizeof(BtStackServiceContext)];
unsigned char BT_Stack_Config_Params[sizeof(BT_CONFIGURATION_PARAMS)];


#define ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE 0x0008
uint8_t bleNotifyBuf[128]={0};

extern void aec_debug_dump_process(void);

void BtStackServiceEntrance(void)
{

	uint8_t len;
	uint8_t c;
	BTStackMemAlloc(BT_STACK_MEM_SIZE, gBtHostStackMemHeap, 0);

	BT_DBG("BtStackServiceEntrance.\n");

	//BR/EDR init
	if(!BtStackInit())
	{
		BT_DBG("error init bt device\n");
		//出现初始化异常时,蓝牙协议栈任务挂起
		while(1)
		{
			vTaskDelay(2);
		}
	}
	else
	{
		BT_DBG("bt device init success!\n");
	}

	//BLE init
#if (BLE_SUPPORT == ENABLE)
	{
		//初始化LE待发送数据
		for( len = 0; len < sizeof( bleNotifyBuf ); len++ )
			bleNotifyBuf[ len ]= len;

		InitBlePlaycontrolProfile();
		
		if(!InitBleStack(&g_playcontrol_app_context, &g_playcontrol_profile))
		{
			BT_DBG("error ble stack init\n");
		}
	}
#endif

	//以下是蓝牙协议栈运转，A2DP连接上后会再调用解码播放
	while(1)//CC_TODO: main while1
	{

		rw_main();

		BTStackRun();

//这个函数判断有无音频数据，如有，则解码并播放
		Hfp_Decode();

		//使用串口命令发送数据，例程使用的是调试口，请确定串口号和板级跳线一致
		// if(UART0_RecvByte(&c))
		// {
		// 	switch( c ){
		// 	case 'S':
		// 		//这里请等LE连接上后再发送数据
		// 		att_server_notify( ( uint8_t )ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE, bleNotifyBuf, sizeof( bleNotifyBuf ) );
		// 		break;
		// 	}
		// }

		//aec_debug_dump_process();
		vTaskDelay(2);
	}
}

/**
 * @brief	Start bluetooth stack service initial.
 * @param	NONE
 * @return	
 */


static void BtStackServiceInit(void)
{
	BT_DBG("bluetooth stack service init.\n");

	btStackServiceCt = (BtStackServiceContext*)BT_Stack_Service_Context;//(BtStackServiceContext*)osPortMalloc(sizeof(BtStackServiceContext));

	memset(btStackServiceCt, 0, sizeof(BtStackServiceContext));
	
	btStackConfigParams = (BT_CONFIGURATION_PARAMS*)BT_Stack_Config_Params;//(BT_CONFIGURATION_PARAMS*)osPortMalloc(sizeof(BT_CONFIGURATION_PARAMS));

	memset(btStackConfigParams, 0, sizeof(BT_CONFIGURATION_PARAMS));


}

/**
 * @brief	Start bluetooth stack service.
 * @param	NONE
 * @return	
 */
extern bool has_sdcard;
void BtStackServiceStart(void)
{
	BtBbParams bbParams;

	while(!has_sdcard){vTaskDelay(2);}

	memset((uint8_t*)BB_EM_MAP_ADDR, 0, BB_EM_SIZE);//clear em erea
	
	ClearBtManagerReg();

	SetBtStackState(BT_STACK_STATE_INITAILIZING);
	
	BtStackServiceInit();

	//load bt stack all params
	LoadBtConfigurationParams();
		
	//BB init
	ConfigBtBbParams(&bbParams);
	Bt_init((void*)&bbParams);

	//host memory init
	SetBtPlatformInterface(&pfiOS, &pfiBtDdb);

	BtStackServiceEntrance();
}




