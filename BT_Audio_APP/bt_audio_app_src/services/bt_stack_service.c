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
#include "app_config.h"
#include "gpio.h" //for BOARD
#include "debug.h"
#include "rtos_api.h"
#include "app_message.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "dma.h"
#include "timeout.h"
#include "irqn.h"
//#include "ble_api.h"
#include "bt_config.h"
#include "bt_stack_api.h"
#include "bt_app_func.h"
#include "bt_stack_service.h"
#include "bt_play_mode.h"
#include "bt_play_api.h"
#include "bt_hf_mode.h"
#include "bt_app_interface.h"
#include "ble_api.h"
#include "ble_app_func.h"
#include "bt_avrcp_api.h"
#include "bt_manager.h"
#include "mode_switch_api.h"
#include "main_task.h"
#include "bt_pbap_api.h"
#include "bt_platform_interface.h"

#include "bb_api.h"

#include "clk.h"
#include "reset.h"

#include "remind_sound_service.h"
#include "bt_ddb_flash.h"

#include "bt_record_mode.h"

#include "backup.h"

#ifdef CFG_FUNC_AI_EN
#include "ai.h"
#endif

#ifdef CFG_XIAOAI_AI_EN
#include "xm_xiaoai_api.h"
#endif


#ifdef CFG_APP_BT_MODE_EN

#ifdef CFG_BT_BACKGROUND_RUN_EN
uint8_t gBtHostStackMemHeap[BT_STACK_MEM_SIZE];
#endif

//BR/EDR STACK SERVICE
#define BT_STACK_SERVICE_STACK_SIZE		768//1024
#define BT_STACK_SERVICE_PRIO			4
#define BT_STACK_NUM_MESSAGE_QUEUE		10

//USER SERVICE //用来处理协议栈callback中用户需要处理的msg
#define BT_USER_SERVICE_STACK_SIZE		256
#define BT_USER_SERVICE_PRIO			3
#define BT_USER_NUM_MESSAGE_QUEUE		10


#ifdef MVA_BT_OBEX_UPDATE_FUNC_SUPPORT
	#define BT_OBEX_SERVICE_STACK_SIZE		512
	#define BT_OBEX_SERVICE_PRIO			4
	xTaskHandle			bt_obex_taskHandle;
#endif

typedef struct _BtStackServiceContext
{
	xTaskHandle			taskHandle;
	MessageHandle		msgHandle;
	TaskState			serviceState;

	uint8_t				serviceWaitResume;	//1:蓝牙不在后台运行时,开启通话,退出播放模式,不能kill蓝牙协议栈

	uint8_t				bbErrorMode;
	uint8_t				bbErrorType;
}BtStackServiceContext;

static BtStackServiceContext	*btStackServiceCt = NULL;

BT_CONFIGURATION_PARAMS		*btStackConfigParams = NULL;

static uint32_t bbIsrCnt = 0;

//蓝牙通话模式/通话录音模式相关参数
extern uint8_t gEnterBtHfMode;
extern uint8_t gEnterBtRecordMode;

//蓝牙服务sniff 服务task。
#ifdef BT_SNIFF_ENABLE

typedef struct _BtUserServiceContext
{
	xTaskHandle			taskHandle;
	MessageHandle		msgHandle;
	TaskState			serviceState;

}BtUserServiceContext;

static BtUserServiceContext	*btUserServiceCt = NULL;

#endif	//BT_SNIFF_ENABLE

static void BtRstStateCheck(void);

//#ifdef BT_FAST_POWER_ON_OFF_FUNC
void BtScanPageStateCheck(void);
void BtScanPageStateSet(BT_SCAN_PAGE_STATE state);
//#endif

#ifdef BT_SNIFF_ENABLE
static void BtUserServiceEntrance(void * param);
#endif
/***********************************************************************************
 * 蓝牙测试盒校准频偏完成回调函数
 **********************************************************************************/
void BtFreqOffsetAdjustComplete(unsigned char offset)
{
	int8_t ret = 0;
	APP_DBG("++++++[BT_OFFSET]  offset:0x%x ++++++\n", offset);

	btManager.btLastAddrUpgradeIgnored = 1;

	//判断是否和当前默认值一致,不一致更新保存到flash
	if(offset != btStackConfigParams->bt_trimValue)
	{
		btStackConfigParams->bt_trimValue = offset;
	
		//save to flash
		ret = BtDdb_SaveBtConfigurationParams(btStackConfigParams);
		
		if(ret)
			APP_DBG("[BT_OFFSET]update Error!!!\n");
		else
			APP_DBG("$$$[BT_OFFSET] update $$$\n");
	}

	//清除最后1次配对记录
	BtDdb_LastBtAddrErase();
}

/***********************************************************************************
 * 蓝牙middleware的消息传递入口函数
 **********************************************************************************/
void BtMidMessageManage(BtMidMessageId messageId, uint8_t Param)
{
	MessageContext		msgSend;
	MessageHandle 		msgHandle;

	switch(messageId)
	{
		case MSG_BT_MID_UART_RX_INT:
			msgHandle = GetBtStackServiceMsgHandle();
			msgSend.msgId = MSG_BTSTACK_RX_INT;
			MessageSend(msgHandle, &msgSend);
			break;

		case MSG_BT_MID_ACCESS_MODE_IDLE:
#ifdef BT_RECONNECTION_FUNC
			BtReconnectDevice();
#endif
			break;

		case MSG_BT_MID_STACK_INIT:
			{
				//此处配置协议栈初始化完成后，是否进入到蓝牙可被搜索可被连接状态;
				//1=进入到可被搜索可被连接状态;  0=进入到不可被搜索不可被连接状态
				GetBtManager()->btAccessModeEnable = 1;
			}
			break;

		case MSG_BT_MID_STATE_CONNECTED:
			{
				MessageContext		msgSend;
				msgSend.msgId		= MSG_BT_STATE_CONNECTED;
				MessageSend(GetMainMessageHandle(), &msgSend);

				SetBtPlayState(BT_PLAYER_STATE_STOP);
			}
			break;
		
		case MSG_BT_MID_STATE_DISCONNECT:
			{
				MessageContext		msgSend;
				msgSend.msgId		= MSG_BT_STATE_DISCONNECT;
				MessageSend(GetMainMessageHandle(), &msgSend);

				SetBtPlayState(BT_PLAYER_STATE_STOP);
			}
			break;

		case MSG_BT_MID_STATE_FAST_ENABLE:
//#ifdef BT_FAST_POWER_ON_OFF_FUNC
			if(GetSystemMode() == AppModeBtAudioPlay)
				BtScanPageStateSet(BT_SCAN_PAGE_STATE_OPENING);
			else
				BtScanPageStateSet(BT_SCAN_PAGE_STATE_CLOSING);
//#endif
			break;

//////////////////////////////////////////////////////////////////////////////////////////////////
//AVRCP
		case MSG_BT_MID_PLAY_STATE_CHANGE:
			if((Param == BT_PLAYER_STATE_PLAYING)&&(GetA2dpState() == BT_A2DP_STATE_STREAMING))
			{
				msgHandle = GetMainMessageHandle();
				msgSend.msgId		= MSG_BT_A2DP_STREAMING;
				MessageSend(msgHandle, &msgSend);
			}
			
			msgHandle = GetBtPlayMessageHandle();
			if(msgHandle == NULL)
				break;

			if(GetBtPlayState() == Param)
				break;
			
			SetBtPlayState(Param);

			// Send message to bt play mode
			msgSend.msgId		= MSG_BT_PLAY_STATE_CHANGED;
			MessageSend(msgHandle, &msgSend);
			break;

		case MSG_BT_MID_STREAM_PAUSE:
			msgHandle = GetBtPlayMessageHandle();
			if(msgHandle == NULL)
				break;
			// Send message to bt play mode
			msgSend.msgId		= MSG_BT_PLAY_STREAM_PASUE;
			MessageSend(msgHandle, &msgSend);
			break;

		case MSG_BT_MID_VOLUME_CHANGE:		
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
			msgHandle =GetMainMessageHandle();// GetBtPlayMessageHandle();
			//if(msgHandle == NULL)
			//	break;
			
			SetBtSyncVolume(Param);
			// Send message to bt play mode
			msgSend.msgId		= MSG_BT_PLAY_SYNC_VOLUME_CHANGED;
			MessageSend(msgHandle, &msgSend);
#endif
			break;

//////////////////////////////////////////////////////////////////////////////////////////////////
//HFP
#if (BT_HFP_SUPPORT == ENABLE)
		case MSG_BT_MID_HFP_TASK_RESUME:
			BtHfModeRunningResume();
			break;

		//通话数据格式更新
		case MSG_BT_MID_HFP_CODEC_TYPE_UPDATE:
			BtHfCodecTypeUpdate(Param);
			break;

#ifdef CFG_FUNC_REMIND_SOUND_EN
		//通话模式下呼入电话播放来电提示音
		case MSG_BT_MID_HFP_PLAY_REMIND:
			msgHandle = GetBtHfMessageHandle();
			if(msgHandle == NULL)
				break;
#ifdef CFG_BT_RING_LOCAL
			// Send message to bt play mode
			msgSend.msgId		= MSG_BT_HF_MODE_REMIND_PLAY;
			MessageSend(msgHandle, &msgSend);
#endif
			break;
		
		//通话模式下停止播放提示音
		case MSG_BT_MID_HFP_PLAY_REMIND_END:
			msgHandle = GetBtHfMessageHandle();
			if(msgHandle == NULL)
				break;
			
			RemindSoundServiceEnd();
			break;
#endif

#ifdef BT_RECORD_FUNC_ENABLE
		case MSG_BT_MID_HFP_RECORD_MODE_ENTER://进入通话录音模式
			{
				if(!gEnterBtHfMode)
					EnterBtRecordMode();
			}
			break;
	
		case MSG_BT_MID_HFP_RECORD_MODE_EXIT://退出通话录音模式
			{
				ExitBtRecordMode();
			}
			break;
			
		case MSG_BT_MID_HFP_RECORD_MODE_DEREGISTER://注销通话录音模式
			{
				if(GetSystemMode() == AppModeBtRecordPlay)
				{
					BtRecordModeDeregister();
				}
			}
			break;
#endif

#endif
		case MSG_BT_MID_HFP_CONNECTED:
			gEnterBtHfMode = 0;
			gEnterBtRecordMode = 0;
			break;
	
#if BT_AVRCP_VOLUME_SYNC
		case MSG_BT_MID_AVRCP_PANEL_KEY:
			{
				MessageContext		msgSend={0};
				switch(Param)
				{
				case 65://vol +
					msgSend.msgId=MSG_MUSIC_VOLUP;
				break;
	
				case 66:////vol -
					msgSend.msgId=MSG_MUSIC_VOLDOWN;
				break;
				/*		
					case 68:
						msgSend.msgId=MSG_PLAY;
					break;
					case 70://pause
						msgSend.msgId=MSG_PAUSE;
					break;
					case 75:
						msgSend.msgId=MSG_NEXT;
					break;
					case 76:
						msgSend.msgId=MSG_PRE;
					break;
					default:
					break;*/
				}
				MessageSend(GetMainMessageHandle(), &msgSend);
			}
			break;
#endif
		
//////////////////////////////////////////////////////////////////////////////////////////////////
		default:
			break;
	}
}

extern uint32_t btReConProtectCnt;
static void CheckBtEventTimer(void)
{
	//开机蓝牙回连和主动连接冲突时,保护时间5s;对方远端设备先发起连接,5s内不能发起回连
	if(btReConProtectCnt)
	{
		btReConProtectCnt++;
		if(btReConProtectCnt>=5000)
		{
			btReConProtectCnt = 0;
		}
	}

	//获取蓝牙播放状态
	if(GetBtManager()->avrcpPlayStatusTimer.timerFlag)
	{
		if(IsTimeOut(&GetBtManager()->avrcpPlayStatusTimer.timerHandle))
		{
			BT_A2DP_STATE state = GetA2dpState();
			if(state == BT_A2DP_STATE_STREAMING)
			{
				BTCtrlGetPlayStatus();
				TimerStart_BtPlayStatus();
			}
			else
			{
				TimerStop_BtPlayStatus();
			}
		}
	}

#ifdef BT_RECONNECTION_FUNC
	if(GetBtManager()->btReconnectDelayCount)
	{
		GetBtManager()->btReconnectDelayCount++;
		if(GetBtManager()->btReconnectDelayCount>200)
		{
			GetBtManager()->btReconnectDelayCount = 0;
			BtReconnectDevice();
		}
	}
	
	if(GetBtManager()->btReconnectTimer.timerFlag & TIMER_STARTED)
	{
		if(IsTimeOut(&GetBtManager()->btReconnectTimer.timerHandle))
		{
			CheckBtReconnectTimer();
		}
	}
#endif

//#if (defined(BT_FAST_POWER_ON_OFF_FUNC)&&defined(CFG_BT_BACKGROUND_RUN_EN))
	BtScanPageStateCheck();
//#endif

	BtRstStateCheck();
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**
 * @brief	Get message receive handle of bt stack manager
 * @param	NONE
 * @return	MessageHandle
 */
MessageHandle GetBtStackServiceMsgHandle(void)
{
	if(!btStackServiceCt)
		return NULL;
	
	return btStackServiceCt->msgHandle;
}

TaskState GetBtStackServiceState(void)
{
	if(!btStackServiceCt)
		return 0;
	
	return btStackServiceCt->serviceState;
}

void update_btDdb(uint8_t addr)
{
	btStackConfigParams->bt_LocalDeviceAddr[3] = addr;
	BtDdb_SaveBtConfigurationParams(btStackConfigParams);
	BtDdb_Erase();
}

static void CheckBtErrorState(void)
{
	btEventListCount++;
	if(btCheckEventList)
	{
		//防止手机端不主动断开AVRCP,在手机端断开A2DP后,设置5S超时,然后主动断开AVRCP
		if((btCheckEventList&BT_EVENT_AVRCP_DISCONNECT)&&(btEventListCount == btEventListB0Count))
		{
			APP_DBG("[btCheckEventList]: BT_EVENT_AVRCP_DISCONNECT\n");
			if(GetAvrcpState() > BT_AVRCP_STATE_NONE)
			{
				AvrcpDisconnect();
			}
			btCheckEventList &= ~BT_EVENT_AVRCP_DISCONNECT;
			btEventListB0Count = 0;
		}

		//当接收到BT_STACK_EVENT_COMMON_CONNECTION_ABORTED事件,当前链路出现异常,会主动断开手机,需要主动回连手机,并自动播放歌曲
		//超时时间为30s
		if((btCheckEventList&BT_EVENT_L2CAP_LINK_DISCONNECT)&&(btEventListCount == btEventListB1Count))
		{
			APP_DBG("[btCheckEventList]: BT_EVENT_L2CAP_LINK_DISCONNECT\n");
			
			btCheckEventList &= ~BT_EVENT_L2CAP_LINK_DISCONNECT;
			btEventListB0Count = 0;
		}

		if((btCheckEventList&BT_EVENT_AVRCP_CONNECT)&&(btEventListB2Count))
		{
			btEventListB2Count--;
			if(!btEventListB2Count)
			{
				btCheckEventList &= ~BT_EVENT_AVRCP_CONNECT;
				btEventListB2Count = 0;
				if((GetAvrcpState() < BT_AVRCP_STATE_CONNECTED)&&(GetA2dpState() >= BT_A2DP_STATE_CONNECTED))
				{
					BtAvrcpConnect(GetBtManager()->remoteAddr);
				}
			}
		}
	}
}

//此函数lib中调用，客户可根据自己条件配置
void BtCntClkSet(void)
{
#if BT_SNIFF_CLK_SEL == BT_SNIFF_HOSC_CLK
	//HOSC 32K
	Clock_BTDMClkSelect(OSC_32K_MODE);
	Clock_OSC32KClkSelect(HOSC_DIV_32K_CLK_MODE);
	Clock_32KClkDivSet(Clock_OSCClkDivGet());  //如果这里出现问题，请检查SystemClockInit，不要在这里直接改——Tony
	Clock_BBCtrlHOSCInDeepsleep(0);//禁止baseband进入sniff后硬件自动关闭HOSC 24M

#elif BT_SNIFF_CLK_SEL == BT_SNIFF_RC_CLK
	//RC
	sniff_rc_init_set();//Rc 初始化参数
	//RC 32K
	Clock_BTDMClkSelect(RC_CLK32_MODE);//select rc_clk_32k
	Clock_32KClkDivSet(750);     //set osc_clk_32k = 24M/32K=750

	Clock_RcCntWindowSet(63);//64-1  --  32K/64 = 500

	Clock_RC32KClkDivSet( Clock_RcFreqGet(1) / ((uint32_t)(32*1000)) );
	Clock_RcFreqAutoCntStart();

	Clock_BBCtrlHOSCInDeepsleep(1);//Deepsleep时,BB接管HOSC

#elif BT_SNIFF_CLK_SEL == BT_SNIFF_LOSC_CLK
	//btclk freq set
	BACKUP_32KEnable(OSC32K_SOURCE);
	sniff_cntclk_set(1);//sniff cnt clk 32768 Hz default not use

	Clock_BTDMClkSelect(OSC_32K_MODE);
	Clock_OSC32KClkSelect(LOSC_32K_MODE);
	Clock_BBCtrlHOSCInDeepsleep(0);//禁止baseband进入sniff后硬件自动关闭HOSC 24M

#endif
}

static void BtStackServiceEntrance(void * param)
{
	MessageContext		msgRecv;
	
	APP_DBG("BtStackServiceEntrance.\n");

	//BR/EDR init
	if(!BtStackInit())
	{
		APP_DBG("error init bt device\n");
		//出现初始化异常时,蓝牙协议栈任务挂起
		while(1)
		{
			MessageRecv(btStackServiceCt->msgHandle, &msgRecv, 0xFFFFFFFF);
		}
	}
	else
	{
		APP_DBG("bt device init success!\n");
	}

	//BLE init
#if (BLE_SUPPORT == ENABLE)
	{
		InitBlePlaycontrolProfile();
		
		if(!InitBleStack(&g_playcontrol_app_context, &g_playcontrol_profile))
		{
			APP_DBG("error ble stack init\n");
		}
	}
#endif

	while(1)
	{
		//在蓝牙协议栈当前所有的事情处理完后,才会挂起任务
		//if(!HasBtDataToProccess())
		{
			MessageRecv(btStackServiceCt->msgHandle, &msgRecv, 1);
		}

		switch(msgRecv.msgId)
		{
			case MSG_BTSTACK_BB_ERROR:
				{
					MessageContext		msgSend;
					MessageHandle 		msgHandle;
					msgHandle = GetMainMessageHandle();
					msgSend.msgId = MSG_BTSTACK_BB_ERROR;

					MessageSend(msgHandle, &msgSend);

					if(btStackServiceCt->bbErrorMode == 1)
					{
						APP_DBG("BT ERROR:0x%x\n", btStackServiceCt->bbErrorType);
					}
					else if(btStackServiceCt->bbErrorMode == 2)
					{
						APP_DBG("BLE ERROR:0x%x\n", btStackServiceCt->bbErrorType);
					}
				}
				break;
		}
		
		rw_main();
		
		BTStackRun();
		CheckBtEventTimer();

#ifdef CFG_XIAOAI_AI_EN
		if(SoftFlagGet(SoftFlagAiProcess))
		{
			xm_ai_encode_data_run_loop();

		}
		ai_ble_run_loop();
#endif

		#ifdef CFG_FUNC_AI_EN
		if(SoftFlagGet(SoftFlagAiProcess))
		{
			ai_run_loop();
		}
		#endif
		//bt异常情况处理
		CheckBtErrorState();

#ifdef BT_HFP_MODE_DISABLE
		extern uint32_t gHfpCallNoneWaiting;
		if(gHfpCallNoneWaiting)
		{
			gHfpCallNoneWaiting++;
			if(gHfpCallNoneWaiting>=1000) //delay 1000ms
			{
				gHfpCallNoneWaiting=0;
//				printf("HfpCallNoneWaiting end\n");
			}
		}
#endif
	}
}

/**
 * @brief	Start bluetooth stack service initial.
 * @param	NONE
 * @return	
 */
static bool BtStackServiceInit(void)
{
	APP_DBG("bluetooth stack service init.\n");

	btStackServiceCt = (BtStackServiceContext*)osPortMalloc(sizeof(BtStackServiceContext));
	if(btStackServiceCt == NULL)
	{
		return FALSE;
	}
	memset(btStackServiceCt, 0, sizeof(BtStackServiceContext));
	
	btStackConfigParams = (BT_CONFIGURATION_PARAMS*)osPortMalloc(sizeof(BT_CONFIGURATION_PARAMS));
	if(btStackConfigParams == NULL)
	{
		return FALSE;
	}
	memset(btStackConfigParams, 0, sizeof(BT_CONFIGURATION_PARAMS));

	btStackServiceCt->msgHandle = MessageRegister(BT_STACK_NUM_MESSAGE_QUEUE);
	if(btStackServiceCt->msgHandle == NULL)
	{
		return FALSE;
	}
	btStackServiceCt->serviceState = TaskStateCreating;

	//register bt middleware message send interface
	BtAppiFunc_MessageSend(BtMidMessageManage);

#ifdef BT_SNIFF_ENABLE
	//user service
	btUserServiceCt = (BtUserServiceContext*)osPortMalloc(sizeof(BtUserServiceContext));
	if(btUserServiceCt == NULL)
	{
		return FALSE;
	}
	memset(btUserServiceCt, 0, sizeof(BtUserServiceContext));

	btUserServiceCt->msgHandle = MessageRegister(BT_USER_NUM_MESSAGE_QUEUE);
	if(btUserServiceCt->msgHandle == NULL)
	{
		return FALSE;
	}
#endif//BT_SNIFF_ENABLE

	//bt rf module check
	//TimeOutSet(&btRfTimerHandle, 2000);

	return TRUE;
}

/**
 * @brief	Start bluetooth stack service.
 * @param	NONE
 * @return	
 */
 
bool BtStackServiceStart(void)
{
	bool		ret = TRUE;
	BtBbParams bbParams;
	if((btStackServiceCt->serviceWaitResume)&&(btStackServiceCt))
	{
		btStackServiceCt->serviceWaitResume = 0;
		return ret;
	}

	memset((uint8_t*)BB_EM_MAP_ADDR, 0, BB_EM_SIZE);//clear em erea
	
	ClearBtManagerReg();

	SetBtStackState(BT_STACK_STATE_INITAILIZING);
	
	ret = BtStackServiceInit();
	if(ret)
	{
		btStackServiceCt->taskHandle = NULL;
		
		//load bt stack all params
		LoadBtConfigurationParams();
		
		//BB init
		ConfigBtBbParams(&bbParams);
		Bt_init((void*)&bbParams);

		//host memory init
		SetBtPlatformInterface(&pfiOS, &pfiBtDdb);
		
#ifdef CFG_BT_BACKGROUND_RUN_EN
		//在蓝牙开启后台运行时,host的内存采用数组,避免存在申请/释放带来碎片化的风险
		BTStackMemAlloc(BT_STACK_MEM_SIZE, gBtHostStackMemHeap, 0);
#else
		BTStackMemAlloc(BT_STACK_MEM_SIZE, NULL, 1);//PTS测试时需要开大内存
#endif
		xTaskCreate(BtStackServiceEntrance, 
					"BtStack", 
					BT_STACK_SERVICE_STACK_SIZE, 
					NULL, 
					BT_STACK_SERVICE_PRIO, 
					&btStackServiceCt->taskHandle);
		if(btStackServiceCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
#ifdef MVA_BT_OBEX_UPDATE_FUNC_SUPPORT
		void bt_obex_upgrate(void);
		xTaskCreate(bt_obex_upgrate,
							"bt_obex_upgrate",
							BT_OBEX_SERVICE_STACK_SIZE,
							NULL,
							BT_OBEX_SERVICE_PRIO,
							&bt_obex_taskHandle);
		if(bt_obex_taskHandle == NULL)
		{
			ret = FALSE;
		}
#endif
#ifdef BT_SNIFF_ENABLE
		xTaskCreate(BtUserServiceEntrance,
							"BtUserService",
							BT_USER_SERVICE_STACK_SIZE,
							NULL,
							BT_USER_SERVICE_PRIO,
							&btUserServiceCt->taskHandle);
		if(btUserServiceCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
#endif//BT_SNIFF_ENABLE
	}
	if(!ret)
		APP_DBG("BtStack service create fail!\n");
	return ret;
}

/**
 * @brief	Kill bluetooth stack service.
 * @param	NONE
 * @return	
 */
bool BtStackServiceKill(void)
{
	int32_t ret = 0;
	if(btStackServiceCt == NULL)
	{
		return FALSE;
	}

	//btStackService
	//Msgbox
	if(btStackServiceCt->msgHandle)
	{
		MessageDeregister(btStackServiceCt->msgHandle);
		btStackServiceCt->msgHandle = NULL;
	}
	
	//task
	if(btStackServiceCt->taskHandle)
	{
		vTaskDelete(btStackServiceCt->taskHandle);
		btStackServiceCt->taskHandle = NULL;
	}

	//deregister bt middleware message send interface
	BtAppiFunc_MessageSend(NULL);

#if (BLE_SUPPORT == ENABLE)
	UninitBleStack();
	UninitBlePlaycontrolProfile();
#endif

	//stack deinit
	ret = BtStackUninit();
	if(!ret)
	{
		APP_DBG("Bt Stack Uninit fail!!!\n");
		return FALSE;
	}

	if(btStackConfigParams)
	{
		osPortFree(btStackConfigParams);
		btStackConfigParams = NULL;
	}
	//
	if(btStackServiceCt)
	{
		osPortFree(btStackServiceCt);
		btStackServiceCt = NULL;
	}
	APP_DBG("!!btStackServiceCt\n");
	

	return TRUE;
}

//
void BtStackServiceWaitResume(void)
{
	btStackServiceCt->serviceWaitResume = 1;
}

//注:需要判断当前是否在中断中，需要调用不同的消息发送函数接口
extern uint32_t GetIPSR( void );
void WakeupBtStackService(void)
{
/*	MessageContext		msgSend;
	MessageHandle 		msgHandle;
	msgHandle = btStackServiceCt->msgHandle;
	msgSend.msgId = MSG_BTSTACK_RX_INT;

	if(GetIPSR())
		MessageSendFromISR(msgHandle, &msgSend);
	else
		MessageSend(msgHandle, &msgSend);
*/
}
void BBMatchReport(void)
{
	MessageContext		msgSend;
	msgSend.msgId		= MSG_BTSTACK_BB_ERROR;
	MessageSend(mainAppCt.msgHandle, &msgSend);
}

void BBErrorReport(uint8_t mode, uint32_t errorType)
{
	MessageContext		msgSend;
	MessageHandle 		msgHandle;
	if(btStackServiceCt == NULL)
		return;
	
	msgHandle = btStackServiceCt->msgHandle;
	msgSend.msgId = MSG_BTSTACK_BB_ERROR;

	btStackServiceCt->bbErrorMode = mode;
	btStackServiceCt->bbErrorType = (uint8_t)errorType;

	//isr
	MessageSendFromISR(msgHandle, &msgSend);
}

void BBIsrReport(void)
{
	bbIsrCnt++;
}

void BT_IntDisable(void)
{
	NVIC_DisableIRQ(18);//BT_Interrupt =18
	NVIC_DisableIRQ(19);//BLE_Interrupt =19
}

void BT_ModuleClose(void)
{
	Reset_RegisterReset(MDM_REG_SEPA);
	Reset_FunctionReset(BTDM_FUNC_SEPA|MDM_FUNC_SEPA|RF_FUNC_SEPA);
	Clock_Module2Disable(ALL_MODULE2_CLK_SWITCH); //close clock
}


/***********************************************************************************
 * 
 **********************************************************************************/
uint8_t GetBtStackCt(void)
{
	if(btStackServiceCt)
		return 1;
	else
		return 0;
}

/***********************************************************************************
 * 快速开关蓝牙
 * 断开蓝牙连接，蓝牙进入不可被搜索，不可被连接状态
 **********************************************************************************/
//#if (defined(BT_FAST_POWER_ON_OFF_FUNC)&&defined(CFG_BT_BACKGROUND_RUN_EN))
void BtScanPageStateSet(BT_SCAN_PAGE_STATE state)
{
	btManager.btScanPageState = state;
}

BT_SCAN_PAGE_STATE BtScanPageStateGet(void)
{
	return btManager.btScanPageState;
}

void BtFastPowerOff(void)
{
#ifdef BT_SNIFF_ENABLE
	if(!Bt_sniff_fastpower_get())
#endif
	{
		BtScanPageStateSet(BT_SCAN_PAGE_STATE_CLOSING);
	}
#ifdef BT_SNIFF_ENABLE
	else
	{
		BtScanPageStateSet(BT_SCAN_PAGE_STATE_SNIFF);
	}
#endif
}

void BtFastPowerOn(void)
{
#ifdef BT_SNIFF_ENABLE
	if(!Bt_sniff_fastpower_get())
#endif
	{
		if(btStackServiceCt->serviceWaitResume)
		{
			btStackServiceCt->serviceWaitResume = 0;
			return;
		}

		BtScanPageStateSet(BT_SCAN_PAGE_STATE_OPENING);
	}
#ifdef BT_SNIFF_ENABLE
	else
	{
		if(Bt_sniff_fastpower_get())
		{
			Bt_sniff_fastpower_en();
		}
		BtScanPageStateSet(BT_SCAN_PAGE_STATE_ENABLE);
	}
#endif
}

void BtScanPageStateCheck(void)
{
	static uint8_t bt_disconnect_count = 0;
	switch(btManager.btScanPageState)
	{
		case BT_SCAN_PAGE_STATE_CLOSING:
#ifdef BT_RECONNECTION_FUNC
			// If there is a reconnectiong process, stop it
			if(btManager.btReconnectTimer.timerFlag)
			{
				BtStopReconnect();
			}
#endif
			// If there is a bt link, disconnect it
			if(GetBtCurConnectFlag())
			{
				BTDisconnect();
				BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISCONNECTING);
				break;
			}

			//不可被搜索不可被连接
			BTSetAccessMode(BtAccessModeNotAccessible);
			BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISABLE);
			break;
			
		case BT_SCAN_PAGE_STATE_DISCONNECTING:
			if(bt_disconnect_count > 200)	// wait about 200ms
			{
				if(GetBtDeviceConnState() != BT_DEVICE_CONNECTION_MODE_NONE)
				{
					BTSetAccessMode(BtAccessModeNotAccessible);
					BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISABLE);
				}
				else
				{
					if(GetBtCurConnectFlag())
					{
						BTDisconnect();
						BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISCONNECTING);
					}
				}
				bt_disconnect_count = 0;
			}
			else
				bt_disconnect_count++;
			break;
			
		case BT_SCAN_PAGE_STATE_DISABLE:
			// double check wether there is a bt link, if any, disconnect again
			if(GetBtCurConnectFlag())
			{
				BTDisconnect();
				BtScanPageStateSet(BT_SCAN_PAGE_STATE_DISCONNECTING);
			}
			break;

		case BT_SCAN_PAGE_STATE_OPENING:
			BTSetAccessMode(BtAccessModeGeneralAccessible);
			BtScanPageStateSet(BT_SCAN_PAGE_STATE_ENABLE);

			btManager.BtPowerOnFlag = 0;
			BtReconnectDevice();
			break;
			
		case BT_SCAN_PAGE_STATE_ENABLE:
			break;
			
		default:
			break;
	}
}

//#endif

/***********************************************************************************
 * 蓝牙恢复出厂设置
 **********************************************************************************/
static void BtRstStateCheck(void)
{
	switch(btManager.btRstState)
	{
		case BT_RST_STATE_NONE:
			break;
			
		case BT_RST_STATE_START:
			APP_DBG("bt reset start\n");
#ifdef BT_RECONNECTION_FUNC
			// If there is a reconnectiong process, stop it
			if(btManager.btReconnectTimer.timerFlag)
			{
				BtStopReconnect();
			}
#endif
			// If there is a bt link, disconnect it
			if(GetBtCurConnectFlag())
			{
				BTDisconnect();
			}

			btManager.btRstState = BT_RST_STATE_WAITING;
			btManager.btRstWaitingCount = 0;
			break;
			
		case BT_RST_STATE_WAITING:
			if(btManager.btRstWaitingCount>=3000)
			{
				btManager.btRstWaitingCount = 2000;
				//if(GetBtConnectedProfile())
				if(GetBtManager()->btConnectedProfile)
				{
					BTDisconnect();
				}
				else if(GetBtDeviceConnState() == BT_DEVICE_CONNECTION_MODE_ALL)
				{
					//BTSetAccessMode(BtAccessModeGeneralAccessible);
					btManager.btRstState = BT_RST_STATE_FINISHED;
				}
			}
			else
			{
				btManager.btRstWaitingCount++;
				/*if((!GetBtCurConnectFlag())&&(GetBtDeviceConnState() == BT_DEVICE_CONNECTION_MODE_ALL))
				{
					btManager.btRstState = BT_RST_STATE_FINISHED;
				}*/
			}
			break;
			
		case BT_RST_STATE_FINISHED:
			btManager.btRstWaitingCount = 0;
			memset(btManager.remoteAddr, 0, 6);
			memset(btManager.btDdbLastAddr, 0, 6);
			
			BtDdb_Erase();
			
			btManager.btRstState = BT_RST_STATE_NONE;
			APP_DBG("bt reset complete\n");
			break;
			
		default:
			btManager.btRstState = BT_RST_STATE_NONE;
			break;
	}
}

/***********************************************************************************
 * 开关蓝牙
 * 断开蓝牙连接，删除蓝牙协议栈任务，关闭蓝牙晶振
 **********************************************************************************/
void BtPowerOff(void)
{
	uint8_t btDisconnectTimeout = 0;
	if(!btStackServiceCt)
		return;
	
	APP_DBG("[Func]:Bt off\n");
	
	if(GetBtStackState() == BT_STACK_STATE_INITAILIZING)
	{
		while(GetBtStackState() == BT_STACK_STATE_INITAILIZING)
		{
			vTaskDelay(10);
			btDisconnectTimeout++;
			if(btDisconnectTimeout>=100)
				break;
		}

		//快速在BT模式和其他模式(共2个模式)切换，需要delay(500);避免蓝牙初始化和反初始化状态未完成导致的错误
		//vTaskDelay(500);
		vTaskDelay(50);
	}
	
	if(GetBtDeviceConnState() == BT_DEVICE_CONNECTION_MODE_NONE)
	{
		BTDisconnect();
	}
#ifdef BT_RECONNECTION_FUNC
	//在蓝牙回连时,需要先取消蓝牙回连行为
	if(GetBtManager()->btReconnectTimer.timerFlag)
	{
		BtStopReconnect();
		vTaskDelay(50);
	}
#endif
	//wait for bt disconnect, 2S timeout
	while(GetBtDeviceConnState() == BT_DEVICE_CONNECTION_MODE_NONE)
	{
		vTaskDelay(10);
		btDisconnectTimeout++;
		if(btDisconnectTimeout>=200)
			break;
	}
	
	//bb reset
	rwip_reset();
	BT_IntDisable();
	//Kill bt stack service
	BtStackServiceKill();
	vTaskDelay(10);
	//reset bt module and close bt clock
	BT_ModuleClose();
}

void BtPowerOn(void)
{
	APP_DBG("[Func]:Bt on\n");
	vTaskDelay(50);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	vTaskDelay(50);

	//bt stack restart
	BtStackServiceStart();
}

#ifdef BT_RECONNECTION_FUNC
bool BtReconnectStartIsReady(void)
{
	if(GetSystemMode()>AppModeWaitingPlay)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
#endif

/***********************************************************************************
 * 进入蓝牙DUT模式
 **********************************************************************************/
void BtEnterDutModeFunc(void)
{
	uint8_t btDisconnectTimeout = 0;
	if(!GetBtStackCt())
	{
		APP_DBG("Enter dut mode fail\n");
		return;
	}

	if(!btManager.btDutModeEnable)
	{
		btManager.btDutModeEnable = 1;
		
		if(GetBtDeviceConnState() == BT_DEVICE_CONNECTION_MODE_NONE)
		{
			BTDisconnect();
		}
#ifdef BT_RECONNECTION_FUNC
		if(GetBtManager()->btReconnectTimer.timerFlag)
		{
			BtStopReconnect();
		}
#endif
		
		APP_DBG("confirm bt disconnect\n");
		while(GetBtDeviceConnState() == BT_DEVICE_CONNECTION_MODE_NONE)
		{
			//2s timeout
			vTaskDelay(100);
			btDisconnectTimeout++;
			if(btDisconnectTimeout>=50)
				break;
		}

		APP_DBG("clear all pairing list\n");
		BtDdb_Erase();
		
		APP_DBG("[Enter dut mode]\n");
		BTEnterDutMode();
	}
}
/***********************************************************************************
 * 
 **********************************************************************************/

#else

void WakeupBtStackService(void)
{
}

void BBErrorReport(void)
{
}

void BBIsrReport(void)
{
}

#endif

#ifdef BT_SNIFF_ENABLE

typedef enum
{
	SNIFF_EXIT,
	SNIFF_READY,
	SNIFF_ENTER
}_SNIFF_STATE_t;

_SNIFF_STATE_t	sniff_state = SNIFF_EXIT;

void SniffStateSet(_SNIFF_STATE_t state)
{
	sniff_state = state;
}

_SNIFF_STATE_t SniffStateGet()
{
	return sniff_state;
}
//Note:蓝牙的callback是协议栈直接回调函数，客户不能在callback中进行太多处理;
//如有需要，则将callback中的event和params通过msg发送到userService中统一进行处理;
MessageHandle GetBtUserServiceMsgHandle(void)
{
	return btUserServiceCt->msgHandle;
}

TaskState GetBtUserServiceState(void)
{
	return btUserServiceCt->serviceState;
}

//发送蓝牙进入deepsleep的消息，btlib会使用
void SendDeepSleepMsg(void)
{
	MessageContext		msgSend;

	if(SniffStateGet() == SNIFF_EXIT)
		msgSend.msgId = MSG_BTSTACK_SNIFF_STANDBY;
	else
	{
		msgSend.msgId = MSG_BTSTACK_SNIFF_ENTER;
	}

	if(GetA2dpState() >= BT_A2DP_STATE_CONNECTED)
	{
		Bt_sniff_sleep_enter();
		MessageSend(btUserServiceCt->msgHandle, &msgSend);
	}

}

void SysDeepsleepStandbyStatus(void)
{
	MessageContext		msgSend;
	if(GetA2dpState() >= BT_A2DP_STATE_CONNECTED)
	{
		msgSend.msgId		= MSG_BTSTACK_DEEPSLEEP;
		MessageSend(mainAppCt.msgHandle, &msgSend);
	}
}

void SysDeepsleepStart(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_BTSTACK_SNIFF_ENTER;
	MessageSend(btUserServiceCt->msgHandle, &msgSend);

}

void SysDeepsleepStop(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_BTSTACK_SNIFF_EXIT;
	MessageSend(btUserServiceCt->msgHandle, &msgSend);

}


static void BtUserServiceEntrance(void * param)
{
	MessageContext		msgRecv;

	Bt_sniff_state_init();
	SysDeepsleepStop();
//	APP_DBG("______Bt User Service Start\n");
	while(1)
	{
		MessageRecv(btUserServiceCt->msgHandle, &msgRecv, 0xffffffff);

		switch(msgRecv.msgId)
		{
			case MSG_BTSTACK_SNIFF_STANDBY:

				if((Bt_sniff_sniff_start_state_get())&&(Bt_sniff_sleep_state_get()))
				{
//					printf("____MSG_BTSTACK_SNIFF_STANDBY\r\n");
					Bt_sniff_sleep_exit();
					SysDeepsleepStandbyStatus();
					SniffStateSet(SNIFF_READY);
				}
				break;

			case MSG_BTSTACK_SNIFF_ENTER:

				if (Bt_sniff_sniff_start_state_get())
				{
					if(Bt_sniff_sleep_state_get())
					{
						Bt_sniff_sleep_exit();
						BtDeepSleepForUsr();
//						printf("____MSG_BTSTACK_SNIFF_ENTER\r\n");
					}
				}

				break;
			case MSG_BTSTACK_SNIFF_EXIT:
//				printf("____MSG_BTSTACK_SNIFF_EXIT\r\n");
				SniffStateSet(SNIFF_EXIT);

				break;
			case MSG_NONE:

				break;

		}
	}
}
#else
void SendDeepSleepMsg(void)
{

}

#endif	//BT_SNIFF_ENABLE

