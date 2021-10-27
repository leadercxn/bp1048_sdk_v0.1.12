/**
 **************************************************************************************
 * @file    main_task.c
 * @brief   Program Entry 
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "type.h"
#include "app_config.h"
#include "timeout.h"
#include "rtos_api.h"
#include "app_message.h"
#include "debug.h"
#include "dma.h"
#include "clk.h"
#include "main_task.h"
#include "timer.h"
#include "otg_detect.h"
#include "irqn.h"
#include "watchdog.h"
#include "dac.h"
#include "dac_interface.h"
#include "audio_adc.h"
#include "adc_interface.h"
#ifdef CFG_RES_FLASHFS_EN
#include "file.h"
#endif
#include "breakpoint.h"
#include "audio_vol.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "device_service.h"
#include "device_detect.h" 
#include "bt_stack_service.h"
#ifdef CFG_FUNC_RECORDER_FLASHFS
#include "playback_service.h"
#endif
#include "remind_sound_service.h"
#include "shell.h"
#include "ctrlvars.h"
#include "communication.h"
#include "mode_switch_api.h"
#include "media_play_mode.h"
#include "flash_boot.h"
#include "breakpoint.h"
#include "sadc_interface.h"
#include "adc.h"
#include "otg_device_hcd.h"
#include "spdif.h"
#include "otg_device_standard_request.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "adc_key.h"
#include "delay.h"
#include "ir_key.h"
#include "rtc_alarm.h"
#include "rtc.h"
#include "i2s.h"
#include "i2s_interface.h"
#include "uarts_interface.h"
#include "bt_ddb_flash.h"
#include "bb_api.h"
#include "bt_config.h"
#include "bt_stack_api.h"
#include "recorder_service.h"
#include "hdmi_in_api.h"
#include "bt_play_mode.h"
#include "bt_hf_mode.h"
#include "efuse.h"
#include "rest_mode.h"
#include "audio_common.h"
#include "device_detect.h"

extern   HDMIInfo  			 *gHdmiCt;

//#define  CFG_FUNC_REMIND_DEEPSLEEP		//PowerStateChange()

#ifdef CFG_FUNC_DISPLAY_EN
#include "display_service.h"
#endif
#if FLASH_BOOT_EN
void start_up_grate(uint32_t UpdateResource);
bool IsUpdataFileExist(void);
#endif
void SystemGotoPowerDown(void);
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
extern void LineInModeEffectConfig(void);
extern void AudioEffectsInit_LineIn(void);
#endif
void SysVarInit(void);
inline void SleepMainAppTask(void);
inline void WakeupMainAppTask(void);
inline void StartModeExit(void);
static void SelectModeBeforeEnter(void);
uint16_t AudioDAC0DataSetNull(void* Buf, uint16_t Len);
uint16_t AudioDAC0DataSpaceLenGetNull(void);
#ifdef CFG_RES_AUDIO_I2SOUT_EN
void AudioI2sOutParamsSet(void);
#endif

#define MAIN_APP_TASK_STACK_SIZE		512//1024
#define MAIN_APP_TASK_PRIO				4
#define MAIN_APP_TASK_SLEEP_PRIO		6 //����deepsleep ��Ҫ�������task������ȼ���
#define MAIN_NUM_MESSAGE_QUEUE			10

#define SHELL_TASK_STACK_SIZE			512//1024
#define SHELL_TASK_PRIO					2
#define	MODE_JITTER_TIME				50
#define	MODE_ENTER_TIME					5000
#define	MODE_EXIT_TIME					3000
#define	MODE_WORK_TIME					0xfffffff 
#define MAIN_APP_MSG_TIMEOUT			50	/* 50 ms */

#ifdef CFG_APP_REST_MODE_EN//�����겻��ͬʱ��
	#ifdef CFG_FUNC_REMIND_DEEPSLEEP
		#error	"Please donot use CFG_FUNC_REMIND_DEEPSLEEP!!!"
	#endif
#endif

//void mv_speex_init(void);

/**����appconfigȱʡ����:DMA 8��ͨ������**/
/*1��cec��PERIPHERAL_ID_TIMER3*/
/*2��SD��¼����PERIPHERAL_ID_SDIO RX/TX*/
/*3�����ߴ��ڵ�����PERIPHERAL_ID_UART1 RX/TX,����ʹ��USB HID����ʡDMA��Դ*/
/*4����·������PERIPHERAL_ID_AUDIO_ADC0_RX*/
/*5��Mic������PERIPHERAL_ID_AUDIO_ADC1_RX��mode֮��ͨ������һ��*/
/*6��Dac0������PERIPHERAL_ID_AUDIO_DAC0_TX mode֮��ͨ������һ��*/
/*7��DacX�迪��PERIPHERAL_ID_AUDIO_DAC1_TX mode֮��ͨ������һ��*/
/*ע��DMA 8��ͨ�����ó�ͻ:*/
/*a��UART���ߵ�����DAC-X�г�ͻ,Ĭ�����ߵ���ʹ��USB HID*/
/*b��UART���ߵ�����HDMI/SPDIFģʽ��ͻ*/

static const uint8_t DmaChannelMap[29] = {
	255,//PERIPHERAL_ID_SPIS_RX = 0,	//0
	255,//PERIPHERAL_ID_SPIS_TX,		//1
#ifdef CFG_APP_HDMIIN_MODE_EN
	5,//PERIPHERAL_ID_TIMER3,			//2
#else
	255,//PERIPHERAL_ID_TIMER3,			//2
#endif
	4,//PERIPHERAL_ID_SDIO_RX,			//3
	4,//PERIPHERAL_ID_SDIO_TX,			//4
	255,//PERIPHERAL_ID_UART0_RX,		//5
	255,//PERIPHERAL_ID_TIMER1,			//6
	255,//PERIPHERAL_ID_TIMER2,			//7
	255,//PERIPHERAL_ID_SDPIF_RX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SDPIF_TX,		//8 SPDIF_RX /TX same chanell
	255,//PERIPHERAL_ID_SPIM_RX,		//9
	255,//PERIPHERAL_ID_SPIM_TX,		//10
	255,//PERIPHERAL_ID_UART0_TX,		//11
	
#ifdef CFG_COMMUNICATION_BY_UART	
	7,//PERIPHERAL_ID_UART1_RX,			//12
	6,//PERIPHERAL_ID_UART1_TX,			//13
#else
	255,//PERIPHERAL_ID_UART1_RX,		//12
	255,//PERIPHERAL_ID_UART1_TX,		//13
#endif

	255,//PERIPHERAL_ID_TIMER4,			//14
	255,//PERIPHERAL_ID_TIMER5,			//15
	255,//PERIPHERAL_ID_TIMER6,			//16
	0,//PERIPHERAL_ID_AUDIO_ADC0_RX,	//17
	1,//PERIPHERAL_ID_AUDIO_ADC1_RX,	//18
	2,//PERIPHERAL_ID_AUDIO_DAC0_TX,	//19
	3,//PERIPHERAL_ID_AUDIO_DAC1_TX,	//20
	255,//PERIPHERAL_ID_I2S0_RX,		//21
#if	(defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S_PORT==0))
	7,//PERIPHERAL_ID_I2S0_TX,			//22
#else	
	255,//PERIPHERAL_ID_I2S0_TX,		//22
#endif	
	255,//PERIPHERAL_ID_I2S1_RX,		//23
#if	(defined(CFG_RES_AUDIO_I2SOUT_EN )&&(CFG_RES_I2S_PORT==1))
	7,	//PERIPHERAL_ID_I2S1_TX,		//24
#else
	255,//PERIPHERAL_ID_I2S1_TX,		//24
#endif
	255,//PERIPHERAL_ID_PPWM,			//25
	255,//PERIPHERAL_ID_ADC,     		//26
	255,//PERIPHERAL_ID_SOFTWARE,		//27
};

//������ �ǼǱ�
uint32_t		SoftwareFlag;

uint8_t gEnterBtHfMode = 0;
uint8_t gEnterBtRecordMode = 0;

MainAppContext	mainAppCt;

uint16_t EndModeExit_set = MSG_NONE;//�ж��Ƿ�Ϊ�����˳�mainapp

TIMER ModeTimer;//����ģʽ�л�Ƶ�����Ա��ֽ��գ����У���Ϣ

static AppMode NextModeProcess(AppMode StartMode);

#ifdef CFG_COMMUNICATION_BY_UART
uint8_t UartRxBuf[1024] = {0};
uint8_t UartTxBuf[1024] = {0};
#endif

#ifdef CFG_RES_IR_NUMBERKEY
bool Number_select_flag = 0;
uint16_t Number_value = 0;
TIMER Number_selectTimer;
#endif

#ifdef	CFG_FUNC_REMIND_DEEPSLEEP
//��ʼ��������ʾ���󣬱�Ƿ�����ʾ���󣬸ý���ʲôcase ����
MessageId PowerStateMsgHandle = MSG_NONE;
#endif

static int32_t MainAppInit(void)
{
	memset(&mainAppCt, 0, sizeof(MainAppContext));

	mainAppCt.msgHandle = MessageRegister(MAIN_NUM_MESSAGE_QUEUE);
	mainAppCt.state = TaskStateCreating;
	mainAppCt.MState = ModeStateNone;
	mainAppCt.appBackupMode = AppModeIdle;
	mainAppCt.appCurrentMode = AppModeIdle;
	mainAppCt.appTargetMode = AppModeIdle;
	return 0;
}


//�����²�service created��Ϣ����Ϻ�start��Щservcie
static void MainAppServiceCreating(uint16_t msgId)
{
	if(msgId == MSG_AUDIO_CORE_SERVICE_CREATED)
	{
		APP_DBG("AudioCore service created\n");
		mainAppCt.AudioCoreSync = TRUE;
	}
	else if(msgId == MSG_DEVICE_SERVICE_CREATED)
	{
		APP_DBG("Device service created\n");
		mainAppCt.DeviceSync = TRUE;
	}
#ifdef CFG_FUNC_DISPLAY_EN
	if(msgId == MSG_DISPLAY_SERVICE_CREATED)
	{
		APP_DBG("Display service created\n");
		mainAppCt.DisplaySync = TRUE;
	}
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(msgId == MSG_REMIND_SOUND_SERVICE_CREATED)
	{
		APP_DBG("Remind service created\n");
		mainAppCt.RemindSoundSync = TRUE;
	}
#endif

	if(mainAppCt.AudioCoreSync
		&& mainAppCt.DeviceSync
#ifdef CFG_FUNC_DISPLAY_EN
		&& mainAppCt.DisplaySync
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
		&& mainAppCt.RemindSoundSync
#endif
		)
	{
		DeviceServiceStart();
		mainAppCt.DeviceSync = FALSE;
		AudioCoreServiceStart();
		mainAppCt.AudioCoreSync = FALSE;
#ifdef CFG_FUNC_DISPLAY_EN
		DisplayServiceStart();
		mainAppCt.DisplaySync = FALSE;
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
		RemindSoundServiceStart();
		mainAppCt.RemindSoundSync = FALSE;
#endif
		mainAppCt.state = TaskStateReady;
	}
}


//�����²�service started����Ϻ�׼��ģʽ�л���
static void MainAppServiceStarting(uint16_t msgId)
{
	if(msgId == MSG_AUDIO_CORE_SERVICE_STARTED)
	{
		APP_DBG("AudioCore service started\n");
		mainAppCt.AudioCoreSync = TRUE;
	}
	else if(msgId == MSG_DEVICE_SERVICE_STARTED)
	{
		APP_DBG("Device service started\n");
		mainAppCt.DeviceSync = TRUE;
	}
#ifdef CFG_FUNC_DISPLAY_EN
	else if(msgId == MSG_DISPLAY_SERVICE_STARTED)
	{
		APP_DBG("Display service started\n");
		mainAppCt.DisplaySync = TRUE;
	}
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
	else if(msgId == MSG_REMIND_SOUND_SERVICE_STARTED)
	{
		APP_DBG("Remind service started\n");
		mainAppCt.RemindSoundSync = TRUE;
	}
#endif

	if(mainAppCt.AudioCoreSync
		&& mainAppCt.DeviceSync
#ifdef CFG_FUNC_DISPLAY_EN
		&& mainAppCt.DisplaySync
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
		&& mainAppCt.RemindSoundSync
#endif
		)
	{
		mainAppCt.DeviceSync = FALSE;
		mainAppCt.AudioCoreSync = FALSE;
#ifdef CFG_FUNC_DISPLAY_EN
		mainAppCt.DisplaySync = FALSE;
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
		mainAppCt.RemindSoundSync = FALSE;
#endif

		//�ȴ�������ʾ����deviceɨ��ʱ�Ρ���waitingģʽ��ʵʩ��
		SoftFlagRegister(SoftFlagWaitDetect);//�˱���ڼ�target���ı䡣����Ӧdeepsleep���˳�/���롣
#ifdef CFG_FUNC_REMIND_SOUND_EN
		SoftFlagRegister(SoftFlagWaitSysRemind);
		mainAppCt.SysRemind = SOUND_REMIND_KAIJI;
#endif

		mainAppCt.appCurrentMode = AppModeWaitingPlay;
		mainAppCt.appTargetMode = AppModeWaitingPlay;
		;//����Waitingģʽ������ʾ���������������׸���Чģʽ-���ϵ����
		//ModeStateSet(ModeCreate);
		if(ModeStateSet(ModeCreate))
		{
			mainAppCt.MState = ModeStateEnter;
			TimeOutSet(&ModeTimer, MODE_ENTER_TIME);//����ģʽ�� ��ʱ����
			mainAppCt.state = TaskStateRunning;
		}
		else
		{
			MessageContext		msgSend;
			mainAppCt.MState = ModeStateExit;
			msgSend.msgId		= MSG_MAINAPP_NEXT_MODE;
			//APP_DBG("enter mode err, go to next mode\n");
			MessageSend(mainAppCt.msgHandle, &msgSend);
		}
	}
}

#ifdef CFG_FUNC_DEEPSLEEP_EN
static void MainAppServicePause(void)
{
	if(mainAppCt.state == TaskStateRunning)//����״̬����Ӧ
	{
#if (defined(CFG_APP_BT_MODE_EN) && defined(CFG_BT_BACKGROUND_RUN_EN))
		if(GetBtStackCt())
		{
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
		}

#endif
		DeviceServicePause();
		mainAppCt.DeviceSync = FALSE;
//		AudioCoreServicePause();
//		mainAppCt.AudioCoreSync = FALSE;
#ifdef CFG_FUNC_DISPLAY_EN
		DisplayServicePause();
		mainAppCt.DisplaySync = FALSE;
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
		RemindSoundServicePause();
		mainAppCt.RemindSoundSync = FALSE;
#endif
		mainAppCt.state = TaskStatePausing;
		//Work̬�跢���˳���Noneֱ����ͣ��Enter��~ed��Ϣ��ʱִ��exit��Exit��~ed��Ϣ��ʱ����Pause̬
		if(mainAppCt.MState == ModeStateWork)
		{
			StartModeExit();
		}
	}
}

static void MainAppServicePauseBt(void)
{
	if(mainAppCt.state == TaskStateRunning)//����״̬����Ӧ
	{
		DeviceServicePause();
		mainAppCt.DeviceSync = FALSE;
//		AudioCoreServicePause();
//		mainAppCt.AudioCoreSync = FALSE;
#ifdef CFG_FUNC_DISPLAY_EN
		DisplayServicePause();
		mainAppCt.DisplaySync = FALSE;
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
		RemindSoundServicePause();
		mainAppCt.RemindSoundSync = FALSE;
#endif
		mainAppCt.state = TaskStatePausing;
		//Work̬�跢���˳���Noneֱ����ͣ��Enter��~ed��Ϣ��ʱִ��exit��Exit��~ed��Ϣ��ʱ����Pause̬
		if(mainAppCt.MState == ModeStateWork)
		{
			StartModeExit();
		}
	}
}

//�����²�service Pausing����Ϻ�׼��ģʽ�л���
static void MainAppServicePausing(uint16_t msgId)
{
//	if(msgId == MSG_AUDIO_CORE_SERVICE_PAUSED)
//	{
//		APP_DBG("AudioCore service paused\n");
//		mainAppCt.AudioCoreSync = TRUE;
//	}
//	else
	if(msgId == MSG_DEVICE_SERVICE_PAUSED)
	{
		APP_DBG("Device service paused\n");
		mainAppCt.DeviceSync = TRUE;
	}

#ifdef CFG_FUNC_DISPLAY_EN
	else if(msgId == MSG_DISPLAY_SERVICE_PAUSED)
	{
		APP_DBG("Display service paused\n");
		mainAppCt.DisplaySync = TRUE;
	}
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	else if(msgId == MSG_REMIND_SOUND_SERVICE_PAUSED)
	{
		APP_DBG("Remind service paused\n");
		mainAppCt.RemindSoundSync = TRUE;
	}
#endif

	if(mainAppCt.DeviceSync //mainAppCt.AudioCoreSync &&
#ifdef CFG_FUNC_REMIND_SOUND_EN
		&& mainAppCt.RemindSoundSync
#endif
#ifdef CFG_FUNC_DISPLAY_EN
		&& mainAppCt.DisplaySync
#endif
		&& (mainAppCt.MState == ModeStateNone || mainAppCt.MState == ModeStatePause)
		)
	{
#if (defined(CFG_APP_BT_MODE_EN) && defined(CFG_BT_BACKGROUND_RUN_EN))
		if(msgId != MSG_BTSTACK_DEEPSLEEP)
		{
			uint8_t btDisconnectTimeout = 0;
			APP_DBG("confirm bt disconnect\n");
			//while(GetBtDeviceConnState() != BT_DEVICE_CONNECTION_MODE_NONE)
			while(btManager.btLinkState)
			{
				//2s timeout
				vTaskDelay(50);
				btDisconnectTimeout++;
				if(btDisconnectTimeout>=40)
					break;
			}
			APP_DBG("kill bt\n");
			//bb reset
			rwip_reset();
			BT_IntDisable();
			//Kill bt stack service
			BtStackServiceKill();
			vTaskDelay(10);
		}
#endif
		APP_DBG("Sleep\n");
		AudioCoreSinkMute(0,TRUE,TRUE);
		AudioCoreSinkDisable(0);

		if(msgId != MSG_BTSTACK_DEEPSLEEP)
			vTaskPrioritySet(mainAppCt.taskHandle, MAIN_APP_TASK_SLEEP_PRIO);//�趨������ȼ�

		DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);
		SleepMainAppTask();
#ifdef CFG_FUNC_MAIN_DEEPSLEEP_EN
		Reset_McuSystem();//����ϵ��Ĭ�Ͻ���deepsleepӦ�ó�����������ϵͳ��λ������
#endif
		SoftFlagDeregister(SoftFlagWakeUpSouceIsCEC);
		if(msgId != MSG_BTSTACK_DEEPSLEEP)
		{
			DeepSleeping();
		}

#ifdef	BT_SNIFF_ENABLE
		else
		{
			Efuse_ReadDataDisable();
			SysDeepsleepStart();
			while(Bt_sniff_sniff_start_state_get())//û�˳�sniff�Ͳ�תmaintask��
			{
				vTaskDelay(1);
			}
			Clock_APllLock(240000);
			Efuse_ReadDataEnable();
			SysDeepsleepStop();
		}
#endif//BT_SNIFF_ENABLE

#ifdef CFG_FUNC_WAKEUP_MCU_RESET
	Reset_McuSystem();
#endif
		WakeupMainAppTask();
		if(msgId != MSG_BTSTACK_DEEPSLEEP)
		{
#if (defined(CFG_APP_BT_MODE_EN) && defined(CFG_BT_BACKGROUND_RUN_EN))
			WDG_Feed();
			BtStackServiceStart();
			WDG_Feed();
#endif

			vTaskPrioritySet(mainAppCt.taskHandle, MAIN_APP_TASK_PRIO);
		}
		DeviceServiceResume();
//		AudioCoreServiceResume();
#ifdef CFG_FUNC_DISPLAY_EN
		DisplayServiceResume();
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
		RemindSoundServiceResume();
#endif
		SoftFlagDeregister(SoftFlagDeepSleepRequest);

		if(SoftFlagGet(SoftFlagBtHfDelayEnterSleep))
		{
			mainAppCt.appTargetMode = AppModeBtAudioPlay;
			mainAppCt.appCurrentMode = mainAppCt.appTargetMode;
			mainAppCt.appBackupMode = AppModeIdle;//�����ģʽ
		}
		else if(ModeResumeMask(mainAppCt.appCurrentMode)
			|| (mainAppCt.appCurrentMode == AppModeWaitingPlay && !SoftFlagGet(SoftFlagWaitModeMask)))
		{
			if(SoftFlagGet(SoftFlagWakeUpSouceIsCEC))
				mainAppCt.appCurrentMode = AppModeHdmiAudioPlay;
			else
				mainAppCt.appCurrentMode = NextModeProcess(mainAppCt.appCurrentMode);
			mainAppCt.appTargetMode = mainAppCt.appCurrentMode;
			mainAppCt.appBackupMode = AppModeIdle;//�����ģʽ
			SoftFlagDeregister(SoftFlagPlayback);//��ȫ������
		}
		else
		{
			if(SoftFlagGet(SoftFlagWakeUpSouceIsCEC))
				mainAppCt.appCurrentMode = AppModeHdmiAudioPlay;
			mainAppCt.appTargetMode = mainAppCt.appCurrentMode;
		}
		SoftFlagDeregister(SoftFlagBtHfDelayEnterSleep);
		SoftFlagDeregister(SoftFlagWakeUpSouceIsCEC);

#ifdef CFG_FUNC_REMIND_WAKEUP
		if(mainAppCt.appCurrentMode != AppModeWaitingPlay)
		{
			mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
			mainAppCt.appTargetMode = mainAppCt.appCurrentMode = AppModeWaitingPlay;
		}
		SoftFlagRegister(SoftFlagWaitSysRemind);
		mainAppCt.SysRemind = SOUND_REMIND_KAIJI;
#endif
		ModeStateSet(ModeCreate);
		mainAppCt.MState = ModeStateEnter;
		TimeOutSet(&ModeTimer, MODE_ENTER_TIME);//����ģʽ�� ��ʱ����
		mainAppCt.state = TaskStateRunning;
		WDG_Feed();
		DelayMs(200);//ּ������һ��'po'������mic�йأ��ɸ���ʵ��Ч��΢����ʱ��
		AudioCoreSinkEnable(0);
		AudioCoreSinkUnmute(0,TRUE,TRUE);
		SystemVolSet();
	}
}
#endif

//����ģʽ��ѡ����һ����Чģʽ����ģʽ��ʱ�������ȡ�
//backup������ǰ������backup�����£���һ��ģʽ��backup��backup����һ��ģʽ��ֻ�ڷ���Enterʱ����
static AppMode NextModeProcess(AppMode StartMode)
{
	AppMode FindMode;

	if(mainAppCt.appBackupMode != AppModeIdle)
	{
		if(CheckModeResource(mainAppCt.appBackupMode))
		{
			FindMode = mainAppCt.appBackupMode;
		}
		else
		{
			FindMode = NextAppMode(mainAppCt.appBackupMode);
		}
	}
	else
	{
		FindMode = NextAppMode(mainAppCt.appCurrentMode);
	}
	return FindMode;
}

/**
 * @func        MsgProcessModeResource
 * @brief       ����ģʽ��Դ����¼���Ϣ�������ı�ģʽҲ��˴���������Ϣ���������ģʽƵ���л�
 * @param       uint16_t msgId  
 * @Output      None
 * @return      bool TRUE��appTargetMode���ģ�FALSE��appTargetMode���䡣
 * @Others      �˺�������targetmode���ص���ģʽѡ��Ĳ���Ԥ���塣�����Ƿ��л�currentModeȡ����MStateģʽ״̬��
 * Record
 */
static bool MsgProcessModeResource(uint16_t msgId)
{
	AppMode AppModes = AppModeWaitingPlay;
	switch(msgId)
	{
		case MSG_DEVICE_SERVICE_LINE_IN:
		case MSG_DEVICE_SERVICE_CARD_IN:
		case MSG_DEVICE_SERVICE_DISK_IN:
#if 1//ndef CFG_COMMUNICATION_BY_USB //USB HID���ߵ�����USB device ��������Ȳ�����
		case MSG_DEVICE_SERVICE_USB_DEVICE_IN:
#endif
		case MSG_DEVICE_SERVICE_HDMI_IN:
			if(mainAppCt.appCurrentMode == AppModeRestPlay)
			{
				break;
			}
			if(!SoftFlagGet(SoftFlagRecording | SoftFlagPlayback) //�طŻ�¼���ڼ䣬��������
#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT == ENABLE)
				&& (mainAppCt.appCurrentMode != AppModeBtHfPlay) //����ͨ�������У�������
				&& (mainAppCt.appCurrentMode != AppModeBtRecordPlay) //����K��¼�������У�������
#endif
#endif
				&& (mainAppCt.appCurrentMode != AppModeUsbDevicePlay)
				)
			{
				if(!SoftFlagGet(SoftFlagWaitModeMask))// ��ǿ��Waiting�׶�
				{
#ifndef CFG_FUNC_APP_MODE_AUTO
					AppModes = FindModeByPlugInMsg(msgId);
#else
					if(mainAppCt.appTargetMode == AppModeWaitingPlay)//����Դʱ�����κ�����Դ��Ҫ����ģʽ���ͺ���Ȳ��޹ء�
					{
						AppModes = FindModeByPlugInMsg(msgId);
					}
#endif
				}
				if(AppModes != mainAppCt.appTargetMode && AppModes != AppModeWaitingPlay)
				{
					mainAppCt.appTargetMode = AppModes;
					if(mainAppCt.appCurrentMode != AppModeWaitingPlay)//������ˡ�
					{
						mainAppCt.appBackupMode = mainAppCt.appCurrentMode;//����Ȳ�ֻ��һ��ģʽ����
					}
					return TRUE;
				}
			}
			break;
		case MSG_DEVICE_SERVICE_CARD_OUT:
		case MSG_DEVICE_SERVICE_LINE_OUT:
		case MSG_DEVICE_SERVICE_DISK_OUT:
#if 1//ndef CFG_COMMUNICATION_BY_USB //USB���ߵ�����USB device ��������Ȳ�����
		case MSG_DEVICE_SERVICE_USB_DEVICE_OUT:	
#endif
		case MSG_DEVICE_SERVICE_HDMI_OUT:
			if(!CheckModeResource(mainAppCt.appCurrentMode))
			{
				AppModes  = NextModeProcess(mainAppCt.appCurrentMode);
				if(AppModes != mainAppCt.appTargetMode)
				{
					mainAppCt.appTargetMode = AppModes;
					APP_DBG("\nFind New:%s @:%s  DevOut:%d ",ModeNameStr(AppModes), ModeNameStr(mainAppCt.appTargetMode), msgId);
					return TRUE;
				}
			}
			break;
			
#ifdef CFG_APP_BT_MODE_EN
#ifdef BT_AUTO_ENTER_PLAY_MODE
		case MSG_DEVICE_SERVICE_BTPLAY_IN:
			APP_DBG("MSG: MSG_DEVICE_SERVICE_BTPLAY_IN\n");
			if(mainAppCt.appCurrentMode != AppModeBtAudioPlay)
			{
				mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
				mainAppCt.appTargetMode = AppModeBtAudioPlay;
				return TRUE;
			}
			break;
#endif

#if (BT_HFP_SUPPORT == ENABLE)
		case MSG_DEVICE_SERVICE_BTHF_IN:
			APP_DBG("MSG: MSG_DEVICE_SERVICE_BTHF_IN\n");
			if((mainAppCt.appCurrentMode != AppModeBtHfPlay)&&(!gEnterBtHfMode))
			{
				if(!gEnterBtRecordMode)
					mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
				mainAppCt.appTargetMode = AppModeBtHfPlay;
				gEnterBtHfMode = 1;

#ifdef BT_EXIT_HF_RESUME_PLAY_STATE
				if(mainAppCt.appCurrentMode == AppModeBtAudioPlay)
				{
					if(GetBtPlayState() == BT_PLAYER_STATE_PLAYING) // bt playing
					{
						SoftFlagRegister(SoftFlagResPlayStateMask);
					}
				}
#endif
				return TRUE;
			}
			break;
			
		case MSG_DEVICE_SERVICE_BTHF_OUT:
			APP_DBG("MSG: MSG_DEVICE_SERVICE_BTHF_OUT\n");
			if((mainAppCt.appCurrentMode == AppModeBtHfPlay)&&(gEnterBtHfMode))
			{
				gEnterBtHfMode = 0;
				if((mainAppCt.appBackupMode != AppModeIdle)&&(mainAppCt.appBackupMode != AppModeBtHfPlay))
				{
					mainAppCt.appTargetMode = mainAppCt.appBackupMode;
				}
				else
				{
					mainAppCt.appTargetMode = AppModeBtAudioPlay;
				}
				return TRUE;
			}
			break;
			
			case MSG_DEVICE_SERVICE_BTRECORD_IN:
				APP_DBG("MSG: MSG_DEVICE_SERVICE_BTRECORD_IN\n");
				if((mainAppCt.appCurrentMode != AppModeBtRecordPlay)&&(!gEnterBtRecordMode))
				{
					mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
					mainAppCt.appTargetMode = AppModeBtRecordPlay;
					gEnterBtRecordMode = 1;
					return TRUE;
				}
				break;
				
			case MSG_DEVICE_SERVICE_BTRECORD_OUT:
				APP_DBG("MSG: MSG_DEVICE_SERVICE_BTRECORD_OUT\n");
				if((mainAppCt.appCurrentMode == AppModeBtRecordPlay)&&(gEnterBtRecordMode))
				{
					gEnterBtRecordMode = 0;
					if((mainAppCt.appBackupMode != AppModeIdle)&&(mainAppCt.appBackupMode != AppModeBtRecordPlay))
					{
						mainAppCt.appTargetMode = mainAppCt.appBackupMode;
					}
					else
					{
						mainAppCt.appTargetMode = AppModeBtAudioPlay;
					}
					return TRUE;
				}
				break;
#endif
#endif
#ifdef CFG_FUNC_ALARM_EN
		  case MSG_RTC_ALARMING:
		  		AppModes = AppModeRestPlay;
				if(AppModes != mainAppCt.appTargetMode)
				{
					mainAppCt.appTargetMode = AppModes;
					if(mainAppCt.appTargetMode == mainAppCt.appBackupMode)
					{
						mainAppCt.appBackupMode = AppModeIdle;//����
					}
					return TRUE;
				}
		  		break;
#endif

		case MSG_MODE:
#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT == ENABLE)
			if(mainAppCt.appCurrentMode == AppModeBtHfPlay) //����ͨ�������У�����ǿ���л�ģʽ
			{
				return FALSE;
			}
#endif
#ifdef BT_RECORD_FUNC_ENABLE
			if(mainAppCt.appCurrentMode == AppModeBtRecordPlay)
			{
				return FALSE;
			}					
#endif
#endif
			if(SoftFlagGet(SoftFlagFrameSizeChange))
			{
				break;//����
			}
			AppModes = NextModeProcess(mainAppCt.appCurrentMode);
			if(AppModes != mainAppCt.appTargetMode)
			{
				mainAppCt.appTargetMode = AppModes;
				if(mainAppCt.appTargetMode == mainAppCt.appBackupMode)
				{
					mainAppCt.appBackupMode = AppModeIdle;//����
				}
				return TRUE;
			}
			break;
#ifdef CFG_APP_REST_MODE_EN
		case MSG_POWER:
#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT == ENABLE)
			if(mainAppCt.appCurrentMode == AppModeBtHfPlay) //����ͨ�������У�����ǿ���л�ģʽ
			{
				return FALSE;
			}
#endif
#ifdef BT_RECORD_FUNC_ENABLE
			if(mainAppCt.appCurrentMode == AppModeBtRecordPlay)
			{
				return FALSE;
			}
#endif
#endif
			if(SoftFlagGet(SoftFlagFrameSizeChange))
			{
				break;//����
			}
			AppModes = AppModeRestPlay;
			if(AppModes != mainAppCt.appTargetMode)
			{
//				RestModeEnable();
//				RestBackupAppSet(mainAppCt.appTargetMode);

				if(!ModeResumeMask(mainAppCt.appCurrentMode))
				{
					mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
				}
				mainAppCt.appTargetMode = AppModes;
//				SoftFlagRegister(SoftFlagDeepSleepRequest);//��֪reset modeҪ�ػ���

				if(mainAppCt.appTargetMode == mainAppCt.appBackupMode)
				{
					mainAppCt.appBackupMode = AppModeIdle;//����
				}
				return TRUE;
			}
			break;
#endif
#ifdef CFG_FUNC_RECORDER_EN
		case MSG_REC_PLAYBACK:
			if((mainAppCt.appCurrentMode == AppModeUDiskPlayBack 
				|| mainAppCt.appCurrentMode == AppModeCardPlayBack
				|| mainAppCt.appCurrentMode == AppModeFlashFsPlayBack)
			   && mainAppCt.appBackupMode != AppModeIdle)
			{
				mainAppCt.appTargetMode = NextModeProcess(mainAppCt.appBackupMode);
				return TRUE;
			}
			if(mainAppCt.appCurrentMode == AppModeRestPlay)
			{
				break;
			}
#ifdef CFG_APP_BT_MODE_EN			
#ifdef BT_RECORD_FUNC_ENABLE
			if(mainAppCt.appCurrentMode == AppModeBtRecordPlay)
			{
				break;
			}					
#endif
#if (BT_HFP_SUPPORT == ENABLE)
			if(mainAppCt.appCurrentMode == AppModeBtHfPlay) //����ͨ�������У����ܽ���¼���ط�ģʽ
			{
				return FALSE;
			}
#endif
#endif
#ifdef CFG_FUNC_RECORD_SD_UDISK
#ifdef CFG_FUNC_RECORD_UDISK_FIRST
			if(CheckModeResource(AppModeUDiskPlayBack))
			{

				mainAppCt.appTargetMode = AppModeUDiskPlayBack;
				if(mainAppCt.appCurrentMode != AppModeWaitingPlay)
				{
					mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
				}
				SoftFlagRegister(SoftFlagPlayback);
				APP_DBG("target:%s\n", "AppModeUDiskPlayBack");
				return TRUE;
			}
			else if( CheckModeResource(AppModeCardPlayBack))
			{
				mainAppCt.appTargetMode = AppModeCardPlayBack;
				if(mainAppCt.appCurrentMode != AppModeWaitingPlay)
				{
					mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
				}
				SoftFlagRegister(SoftFlagPlayback);
				APP_DBG("target:%s\n", "AppModeCardPlayBack");
				return TRUE;
			}
			break;
#else //CFG_FUNC_RECORD_UDISK_FIRST
			if(CheckModeResource(AppModeCardPlayBack))
			{

				mainAppCt.appTargetMode = AppModeCardPlayBack;
				if(mainAppCt.appCurrentMode != AppModeWaitingPlay)
				{
					mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
				}
				SoftFlagRegister(SoftFlagPlayback);
				APP_DBG("target:%s\n", "AppModeCardPlayBack");
				return TRUE;
			}
			else if(CheckModeResource(AppModeUDiskPlayBack))
			{
				mainAppCt.appTargetMode = AppModeUDiskPlayBack;
				if(mainAppCt.appCurrentMode != AppModeWaitingPlay)
				{
					mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
				}
				SoftFlagRegister(SoftFlagPlayback);
				APP_DBG("target:%s\n", "AppModeUDiskPlayBack");
				return TRUE; 
			}
			break;
#endif//CFG_FUNC_RECORD_UDISK_FIRST
#elif defined(CFG_FUNC_RECORD_FLASHFS)
		if(CheckModeResource(AppModeFlashFsPlayBack))
		{
			mainAppCt.appTargetMode = AppModeFlashFsPlayBack;
			if(mainAppCt.appCurrentMode != AppModeWaitingPlay)
			{
				mainAppCt.appBackupMode = mainAppCt.appCurrentMode;
			}
			SoftFlagRegister(SoftFlagPlayback);
			APP_DBG("target:%s\n", "AppModeFlashFsPlayBack");
			return TRUE;
		}
#endif //CFG_FUNC_RECORD_SD_UDISK
#endif//CFG_FUNC_RECORDER_EN
		default:
			break;	
	}
	return FALSE;
}

static void AfterWaitingMode(void)
{
	mainAppCt.appTargetMode = AppModeWaitingPlay ;//AppModeBtAudioPlay;//���ʼֵ
	if(mainAppCt.appBackupMode != AppModeIdle)//�м���
	{
		APP_DBG("Backup:%d\n", mainAppCt.appBackupMode);
		mainAppCt.appTargetMode = NextModeProcess(mainAppCt.appBackupMode);
		APP_DBG("\n select 0:%s\n", ModeNameStr(mainAppCt.appTargetMode));
	}
	else
	{
		mainAppCt.appTargetMode = FindPriorityMode();
		APP_DBG("\n select 1:%s\n", ModeNameStr(mainAppCt.appTargetMode));
	}
	if(mainAppCt.appTargetMode != AppModeWaitingPlay)//ע�⣺�˺�ϵ����ģʽ������ռ
	{
		ModeStateSet(ModeStop);
		mainAppCt.MState = ModeStateExit;
		TimeOutSet(&ModeTimer, MODE_EXIT_TIME);//����ģʽ�� ��ʱ����
	}
	//û�к���ģʽʱ������waiting�����˳���
}

//���exit��ʵʩEnterǰģʽ�жϡ�
static void SelectModeBeforeEnter(void)
{
	if(!CheckModeResource(mainAppCt.appTargetMode))//�ٴ���Դȷ�ϣ��Է���Ϣ��ʧ��������Ч��ģʽ����
	{
		mainAppCt.appTargetMode = NextModeProcess(mainAppCt.appCurrentMode);
		mainAppCt.appBackupMode = AppModeIdle;//�����ģʽ
		SoftFlagDeregister(SoftFlagPlayback);//��ȫ������
	}
	else if(mainAppCt.appBackupMode != AppModeIdle && (!CheckModeResource(mainAppCt.appBackupMode) || mainAppCt.appBackupMode == mainAppCt.appTargetMode))
	{//�������������طţ��ο�����ʱҪ����������һ��ģʽ�����backup����
		mainAppCt.appBackupMode = AppModeIdle;
		SoftFlagDeregister(SoftFlagPlayback);//��ȫ������
	}
//#ifdef  CFG_FUNC_REMIND_DEEPSLEEP
	if(SoftFlagGet(SoftFlagDeepSleepRequest | SoftFlagWaitSysRemind) && mainAppCt.appTargetMode != AppModeWaitingPlay)
	{
		if(!ModeResumeMask(mainAppCt.appCurrentMode))
		{
			mainAppCt.appBackupMode = mainAppCt.appTargetMode;
		}
		mainAppCt.appTargetMode = AppModeWaitingPlay;
	}
//#endif
	if(mainAppCt.appTargetMode == AppModeWaitingPlay)
	{
	
		mainAppCt.appCurrentMode = mainAppCt.appTargetMode = AppModeWaitingPlay;//ǿ����������ģʽ
		vTaskDelay(10);//������ּ����idle�������У���ɾ��task�ͷ���Դ��������Ƭ��
		
		APP_DBG("\nModeStart:--------------------%s--------------------\n", ModeNameStr(mainAppCt.appCurrentMode));
		ModeStateSet(ModeCreate);//�˴���Ӧ���ǳ�ʼ��ʧ�ܺ��ģʽ������
		mainAppCt.MState = ModeStateEnter;
		TimeOutSet(&ModeTimer, MODE_ENTER_TIME);//����ģʽ�� ��ʱ����
	}
	else
	{
		mainAppCt.appCurrentMode = mainAppCt.appTargetMode;
		vTaskDelay(10);//������ּ����idle�������У���ɾ��task�ͷ���Դ��������Ƭ��
		
		APP_DBG("\nModeStart:--------------------%s--------------------\n", ModeNameStr(mainAppCt.appCurrentMode));
		//ModeStateSet(ModeCreate);//�˴���Ӧ���ǳ�ʼ��ʧ�ܺ��ģʽ������
		if(ModeStateSet(ModeCreate))
		{
			mainAppCt.MState = ModeStateEnter;
			TimeOutSet(&ModeTimer, MODE_ENTER_TIME);//����ģʽ�� ��ʱ����
		}
		else
		{
			MessageContext		msgSend;
			mainAppCt.MState = ModeStateExit;
			msgSend.msgId		= MSG_MAINAPP_NEXT_MODE;
			//APP_DBG("enter mode err, go to next mode\n");
			MessageSend(mainAppCt.msgHandle, &msgSend);	
		}
	}
}

inline void StartModeExit(void)
{
	ModeStateSet(ModeStop);
	mainAppCt.MState = ModeStateExit;
	TimeOutSet(&ModeTimer, MODE_EXIT_TIME);//��ʱά��	
}


inline void EndModeExit(void)
{
	SoftFlagDeregister(SoftFlagDecoderMask | SoftFlagDecoderSwitch);//��ȫ������
	ModeStateSet(ModeKill);
	//vTaskDelay(10);//������ּ����idle�������У���ɾ��task�ͷ���Դ��������Ƭ��
	SoftFlagDeregister(SoftFlagRecording | SoftFlagPlayback);//����
	MessageContext		MsgSend;
	MsgSend.msgId = MSG_APP_EXIT;
	MessageSend(GetDeviceMessageHandle(), &MsgSend);//֪ͨdevice����keymaptable
#ifdef CFG_FUNC_DEEPSLEEP_EN
	if(mainAppCt.state == TaskStatePausing)//deepsleep����׶Σ�mode������̬ͣ��
	{
		vTaskDelay(10);//������ּ����idle�������У���ɾ��task�ͷ���Դ��������Ƭ��
		mainAppCt.MState = ModeStatePause;
		MainAppServicePausing(EndModeExit_set);
	}
	else
#endif
	{
		SelectModeBeforeEnter();
	#if (defined(CFG_APP_USB_AUDIO_MODE_EN))&&(defined(CFG_COMMUNICATION_BY_USB))
		if(GetUSBAudioExitFlag())
		{
			SetUSBAudioExitFlag(FALSE);
			SetDeviceDetectVarReset();
		}
	#endif
	}
	SoftFlagDeregister(SoftFlagFrameSizeChange);//����
}

//��Ƶģʽ����ʱ����ģʽ�����Ϣ���жϺ�����
static void StateAtAppRunning(bool Change)
{
	if(Change && !SoftFlagGet(SoftFlagWaitModeMask))//app���Ž׶���ģʽ������
	{
		APP_DBG("Work change Jitter!\n");
		TimeOutSet(&ModeTimer, MODE_JITTER_TIME);
	}

	if(IsTimeOut(&ModeTimer) && (mainAppCt.appCurrentMode != mainAppCt.appTargetMode))
	{
		if(SoftFlagGet(SoftFlagFrameSizeChange))
		{
			APP_DBG("Wait FrameChage\n");
			TimeOutSet(&ModeTimer, MODE_JITTER_TIME);
		}
		else
		{
			StartModeExit();
		}
	}
}

/**
 * @func        MainAppModeProcess
 * @brief       ģʽ�Ľ��롢�˳�������
 * @param       uint16_t Msg  
 * @Output      None
 * @return      None
 * @Others      �˺�����Ҫ��mode��״̬������϶�ʱ�������ͱ�����ʵ������app,created/started ����stopped,kill Msgͬ����
 * @Others      Ҫ�㣺work״̬�ڼ���Ϣ������ת��
 * Record
 */
static void MainAppModeProcess(uint16_t Msg)
{
	MessageContext	msgSend;
#ifdef CFG_FUNC_DISPLAY_EN
	MessageContext	DispmsgSend;
#endif
	bool	TargetModeChange = FALSE;

	if(!SoftFlagGet(SoftFlagWaitModeMask))//�����׶Σ����Զ����ģʽ�仯��
	{
		TargetModeChange = MsgProcessModeResource(Msg);
	}
	switch(mainAppCt.MState)
	{
			//����̬�������µ�TargetMode��������
		case ModeStateEnter: 
			if(IsTimeOut(&ModeTimer))
			{
				APP_DBG("\nEnter %s timeout��care for ram.\n", ModeNameStr(mainAppCt.appCurrentMode));
				StartModeExit();
			}
			else
			{
				switch(AppStateMsgAck(Msg))
				{	
						//ģʽ�Ѵ��� ����
					case MSG_APP_CREATED:
						if(mainAppCt.state == TaskStatePausing)
						{
							StartModeExit();
						}
						else if(mainAppCt.appCurrentMode == mainAppCt.appTargetMode || SoftFlagGet(SoftFlagWaitModeMask))
						{
							ModeStateSet(ModeStart);
						}
						else
						{
							StartModeExit();
							APP_DBG("\nexiting %s to %s After Created", ModeNameStr(mainAppCt.appCurrentMode), ModeNameStr(mainAppCt.appTargetMode));
						}
						break;

						//ģʽ������ ����
					case MSG_APP_STARTED:
						if(mainAppCt.state == TaskStatePausing)
						{
							StartModeExit();
						}
						else if(mainAppCt.appCurrentMode == mainAppCt.appTargetMode || SoftFlagGet(SoftFlagWaitModeMask))//˵�����������targetyû�б仯,waitingģʽ����
						{
							APP_DBG("Mode Started\n");
							mainAppCt.MState = ModeStateWork;
							TimeOutSet(&ModeTimer, MODE_WORK_TIME);
							msgSend.msgId = Msg;
							MessageSend(GetDeviceMessageHandle(), &msgSend);//֪ͨdevice����keymaptable	
                            #ifdef CFG_FUNC_DISPLAY_EN
							DispmsgSend.msgId = MSG_DISPLAY_SERVICE_DEV;
							MessageSend(GetDisplayMessageHandle(), &DispmsgSend);
                            #endif
						}
						else//target�б仯��������˳�����
						{
							StartModeExit();
						}
						break;
				}
			}
			break;

			//�ȶ�����״̬��ϵͳ��Ϣ�����ڴ˴���������Ϣת��ǰmode App��
		case ModeStateWork:
			AudioPlayerMenuCheck();
			switch(Msg)
			{
					//ģʽ���ϴ���
				case MSG_MEDIA_PLAY_MODE_FAILURE://����������������stop����;like:SD/Udisk�ա���ʼ��ʧ��
					mainAppCt.appTargetMode = NextModeProcess(mainAppCt.appCurrentMode);
#ifdef	CFG_FUNC_DEV_ERROR_MODE
					//�����豸���ϵ�ģʽ�����������ʧ���˳����ر���ֻ��һ������ģʽʱ��
					if(mainAppCt.appCurrentMode == AppModeCardAudioPlay)
					{
						ResourceDeregister(AppResourceCardForPlay);
					}
					else if(mainAppCt.appCurrentMode == AppModeUDiskAudioPlay)
					{
						ResourceDeregister(AppResourceUDiskForPlay);
					}
#endif
					StartModeExit();
					APP_DBG("MEDIA_PLAY_MODE_FAILURE\n");
					break;

				case MSG_WAITING_PERMISSION_MODE:
#ifdef	CFG_FUNC_REMIND_DEEPSLEEP
					if(SoftFlagGet(SoftFlagDeepSleepRequest))
					{
						SoftFlagDeregister(SoftFlagDeepSleepRequest);
						if(PowerStateMsgHandle == MSG_DEEPSLEEP)
						{
							msgSend.msgId = MSG_TASK_DEEPSLEEP;
							PowerStateMsgHandle = MSG_NONE;
						}
						else if(PowerStateMsgHandle == MSG_BTSTACK_DEEPSLEEP)
						{
							msgSend.msgId = MSG_TASK_BTSNIFF;
							PowerStateMsgHandle = MSG_NONE;
						}
						else if(PowerStateMsgHandle == MSG_POWERDOWN)
						{
							msgSend.msgId = MSG_POWERCHAGE_POWERDOWN;
							PowerStateMsgHandle = MSG_NONE;
						}
						MessageSend(mainAppCt.msgHandle, &msgSend);
					}
					else
#endif
					{
						AfterWaitingMode();
					}
					break;

				default: //��ϵͳ��Ϣת����app
					//AudioPlayerMenuCheck();
					if(Msg != MSG_NONE && GetAppMessageHandle() != NULL)
					{
					
					//	APP_DBG("MSG=%x\n",Msg);
						msgSend.msgId = Msg;
						MessageSend(GetAppMessageHandle(), &msgSend);
					}


					break;
			}
			StateAtAppRunning(TargetModeChange);
			break;

				//����̬�������µ�TargetMode��������
		case ModeStateExit:
			if(IsTimeOut(&ModeTimer))
			{
				APP_DBG("\nExit %s timeout��care for task.\n", ModeNameStr(mainAppCt.appCurrentMode));
				EndModeExit();
			}
			else if(AppStateMsgAck(Msg) == MSG_APP_STOPPED)
			{
				APP_DBG("exit stopped OK !\n");
				EndModeExit();
			}
			break;
		default://case ModeStatePause:
//			if(mainAppCt.appTargetMode != AppModeIdle && CheckModeResource(mainAppCt.appTargetMode))
//			{
//				mainAppCt.appCurrentMode = mainAppCt.appTargetMode;
//				APP_DBG("\nMainApp:Enter:%s\n", ModeNameStr(mainAppCt.appCurrentMode));
//				ModeStateSet(ModeCreate);//�˴���Ӧ���ǳ�ʼ��ʧ�ܺ��ģʽ������
//				mainAppCt.MState = ModeStateEnter;
//				TimeOutSet(&ModeTimer, MODE_ENTER_TIME);//����ģʽ�� ��ʱ����
//			}
			break;
	}
}

//��ģʽ�µ�ͨ����Ϣ����, ���е���ʾ���ڴ˴������Ҫ����ô�APIǰ��ȷ��APP running״̬�����������û׼���á�
void CommonMsgProccess(uint16_t Msg)
{
#ifdef CFG_FUNC_DISPLAY_EN
	MessageContext	msgSend;
#endif
	switch(Msg)
	{
		case MSG_MENU://�˵���
			APP_DBG("menu key\n");
			AudioPlayerMenu();
			break;

		case MSG_MUTE:
			APP_DBG("MSG_MUTE\n");
			#ifdef  CFG_APP_HDMIIN_MODE_EN
			if(GetSystemMode() == AppModeHdmiAudioPlay)
			{
				if(IsHDMISourceMute() == TRUE)
					HDMISourceUnmute();
				else
					HDMISourceMute();
				gHdmiCt->hdmiActiveReportMuteStatus = IsHDMISourceMute();
				gHdmiCt->hdmiActiveReportMuteflag = 2;
			}
			else
			#endif
			{
				AudioPlayerMute();
			}
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MUTE;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;

		case MSG_MAIN_VOL_UP:
			SystemVolUp();
			APP_DBG("MSG_MAIN_VOL_UP\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_VOL;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;

		case MSG_MAIN_VOL_DW:
			SystemVolDown();
			APP_DBG("MSG_MAIN_VOL_DW\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_VOL;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;

		case MSG_MUSIC_VOLUP:
			AudioMusicVolUp();
			APP_DBG("MSG_MUSIC_VOLUP\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MUSIC_VOL;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;

		case MSG_MUSIC_VOLDOWN:
			AudioMusicVolDown();
			APP_DBG("MSG_MUSIC_VOLDOWN\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MUSIC_VOL;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;

		#if CFG_RES_MIC_SELECT
		case MSG_MIC_VOLUP:
			AudioMicVolUp();
			APP_DBG("MSG_MIC_VOLUP\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MIC_VOL;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;

		case MSG_MIC_VOLDOWN:
			AudioMicVolDown();
			APP_DBG("MSG_MIC_VOLDOWN\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MIC_VOL;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;
		#endif

#ifdef CFG_APP_BT_MODE_EN
		case MSG_BT_PLAY_SYNC_VOLUME_CHANGED:
			APP_DBG("MSG_BT_PLAY_SYNC_VOLUME_CHANGED\n");
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
			AudioMusicVolSet(GetBtSyncVolume());
#endif
			break;
#endif

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		#ifdef CFG_FUNC_MIC_KARAOKE_EN
		case MSG_MIC_EFFECT_UP:
			if(mainAppCt.ReverbStep < MAX_MIC_REVB_STEP)
			{
				mainAppCt.ReverbStep++;
			}
			else
			{
				mainAppCt.ReverbStep = 0;
			}
			ReverbStepSet(mainAppCt.ReverbStep);
			APP_DBG("MSG_MIC_EFFECT_UP\n");
			APP_DBG("ReverbStep = %d\n", mainAppCt.ReverbStep);
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;
		#endif

		#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
		case MSG_EQ:
			APP_DBG("MSG_EQ\n");
			if(mainAppCt.EqMode < EQ_MODE_VOCAL_BOOST)
			{
				mainAppCt.EqMode++;
			}
			else
			{
				mainAppCt.EqMode = EQ_MODE_FLAT;
			}
			APP_DBG("EqMode = %d\n", mainAppCt.EqMode);
			#ifndef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
			EqModeSet(mainAppCt.EqMode);
			#endif
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_EQ;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;
		#endif

		#ifdef CFG_FUNC_MIC_KARAOKE_EN
		case MSG_EFFECTMODE:
			if(mainAppCt.MState == ModeStateWork && !SoftFlagGet(SoftFlagFrameSizeChange))
			{
				APP_DBG("MSG_EFFECTMODE\n");
				if(mainAppCt.appCurrentMode == AppModeBtHfPlay)//����ͨ��ģʽ�£���֧����Чģʽ�л�
				{
					break;
				}
				if(mainAppCt.EffectMode < EFFECT_MODE_WaWaYin)
				{
					mainAppCt.EffectMode++;
				}
				else
				{
					mainAppCt.EffectMode = EFFECT_MODE_HunXiang;
				}
				APP_DBG("EffectMode = %d\n", mainAppCt.EffectMode);
				switch(mainAppCt.EffectMode)
				{
					case EFFECT_MODE_HunXiang:
						#ifdef CFG_FUNC_REMIND_SOUND_EN
						gCtrlVars.remind_type = REMIND_TYPE_KEY;
						RemindSoundServiceItemRequest(SOUND_REMIND_LIUXINGH,FALSE);
						#endif
						break;
					case EFFECT_MODE_DianYin:
						#ifdef CFG_FUNC_REMIND_SOUND_EN
						gCtrlVars.remind_type = REMIND_TYPE_KEY;
						RemindSoundServiceItemRequest(SOUND_REMIND_DIANYIN,FALSE);
						#endif
						break;
					case EFFECT_MODE_MoYin:
						#ifdef CFG_FUNC_REMIND_SOUND_EN
						gCtrlVars.remind_type = REMIND_TYPE_KEY;
						RemindSoundServiceItemRequest(SOUND_REMIND_MOYIN,FALSE);
						#endif
						break;
					case EFFECT_MODE_HanMai:
						#ifdef CFG_FUNC_REMIND_SOUND_EN
						gCtrlVars.remind_type = REMIND_TYPE_KEY;
						RemindSoundServiceItemRequest(SOUND_REMIND_HANMAI,FALSE);
						#endif
						break;
					case EFFECT_MODE_NanBianNv:
						#ifdef CFG_FUNC_REMIND_SOUND_EN
						gCtrlVars.remind_type = REMIND_TYPE_KEY;
						RemindSoundServiceItemRequest(SOUND_REMIND_NVSHEN,FALSE);
						#endif
						break;
					case EFFECT_MODE_NvBianNan:
						#ifdef CFG_FUNC_REMIND_SOUND_EN
						gCtrlVars.remind_type = REMIND_TYPE_KEY;
						RemindSoundServiceItemRequest(SOUND_REMIND_NANSHEN,FALSE);
						#endif
						break;
					case EFFECT_MODE_WaWaYin:
						#ifdef CFG_FUNC_REMIND_SOUND_EN
						gCtrlVars.remind_type = REMIND_TYPE_KEY;
						RemindSoundServiceItemRequest(SOUND_REMIND_WAWAYIN,FALSE);
						#endif
						break;
				}
				AudioEffectModeSel(mainAppCt.EffectMode, 1);//0=init hw,1=effect,2=hw+effect
				{
					extern TIMER EffectChangeTimer;
					TimeOutSet(&EffectChangeTimer, 500);//��ʱ�޸ķ�������֤��ЧģʽƵ���л�ʱ���ܹ����������(�ָ�Ϊ500ms��ʱ�ȴ�����)
				}
				#ifdef CFG_FUNC_BREAKPOINT_EN
				BackupInfoUpdata(BACKUP_SYS_INFO);
				#endif
			}
			break;

		case MSG_MIC_FIRST:
			APP_DBG("MSG_MIC_FIRST\n");
			#ifdef CFG_FUNC_SHUNNING_EN
			gCtrlVars.ShunningMode = !gCtrlVars.ShunningMode;
			APP_DBG("ShunningMode = %d\n", gCtrlVars.ShunningMode);
			#endif
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_SHUNNING;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;
		#endif

		#ifdef CFG_FUNC_MIC_TREB_BASS_EN
		case MSG_MIC_TREB_UP:
			APP_DBG("MSG_MIC_TREB_UP\n");
			if(mainAppCt.MicTrebStep < MAX_BASS_TREB_GAIN)
			{
				mainAppCt.MicTrebStep++;
			}
			APP_DBG("MicTrebStep = %d\n", mainAppCt.MicTrebStep);
			MicBassTrebAjust(mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_TRE;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MIC_TREB_DW:
			APP_DBG("MSG_MIC_TREB_DW\n");
			if(mainAppCt.MicTrebStep)
			{
				mainAppCt.MicTrebStep--;
			}
			APP_DBG("MicTrebStep = %d\n", mainAppCt.MicTrebStep);
			MicBassTrebAjust(mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_TRE;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MIC_BASS_UP:
			APP_DBG("MSG_MIC_BASS_UP\n");
			if(mainAppCt.MicBassStep < MAX_BASS_TREB_GAIN)
			{
				mainAppCt.MicBassStep++;
			}
			APP_DBG("MicBassStep = %d\n", mainAppCt.MicBassStep);
			MicBassTrebAjust(mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_BAS;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MIC_BASS_DW:
			APP_DBG("MSG_MIC_BASS_DW\n");
			if(mainAppCt.MicBassStep)
			{
				mainAppCt.MicBassStep--;
			}
			APP_DBG("MicBassStep = %d\n", mainAppCt.MicBassStep);
			MicBassTrebAjust(mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_BAS;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;
		#endif

		#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
		case MSG_MUSIC_TREB_UP:
			APP_DBG("MSG_MUSIC_TREB_UP\n");
			if(mainAppCt.MusicTrebStep < MAX_BASS_TREB_GAIN)
			{
				mainAppCt.MusicTrebStep++;
			}
			APP_DBG("MusicTrebStep = %d\n", mainAppCt.MusicTrebStep);
			MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MUSIC_TREB_DW:
			APP_DBG("MSG_MUSIC_TREB_DW\n");
			if(mainAppCt.MusicTrebStep)
			{
				mainAppCt.MusicTrebStep--;
			}
			APP_DBG("MusicTrebStep = %d\n", mainAppCt.MusicTrebStep);
			MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MUSIC_BASS_UP:
			APP_DBG("MSG_MUSIC_BASS_UP\n");
			if(mainAppCt.MusicBassStep < MAX_BASS_TREB_GAIN)
			{
				mainAppCt.MusicBassStep++;
			}
			APP_DBG("MusicBassStep = %d\n", mainAppCt.MusicBassStep);
			MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MUSIC_BASS_DW:
			APP_DBG("MSG_MUSIC_BASS_DW\n");
			if(mainAppCt.MusicBassStep)
			{
				mainAppCt.MusicBassStep--;
			}
			APP_DBG("MusicBassStep = %d\n", mainAppCt.MusicBassStep);
			MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;
		#endif

		case MSG_3D:
			APP_DBG("MSG_3D\n");
			#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
			gCtrlVars.music_threed_unit.three_d_en = !gCtrlVars.music_threed_unit.three_d_en;
			#endif
			#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
			gCtrlVars.music_threed_plus_unit.three_d_en = !gCtrlVars.music_threed_plus_unit.three_d_en;
			#endif
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_3D;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;

		case MSG_VOCAL_CUT:
			APP_DBG("MSG_VOCAL_CUT\n");
			gCtrlVars.vocal_cut_unit.vocal_cut_en = !gCtrlVars.vocal_cut_unit.vocal_cut_en;
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_VOCAL_CUT;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;

		case MSG_VB:
			APP_DBG("MSG_VB\n");
			#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
			gCtrlVars.music_vb_unit.vb_en = !gCtrlVars.music_vb_unit.vb_en;
			#endif
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_VB;
            MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			break;

#endif

		case MSG_REC_MUSIC:
			SetRecMusic(0);
			break;

		#ifdef CFG_FUNC_RTC_EN
		case MSG_RTC_SET_TIME:
			RTC_ServiceModeSelect(0,0);
			break;

		case MSG_RTC_SET_ALARM:
			RTC_ServiceModeSelect(0,1);
			break;

		case MSG_RTC_DISP_TIME:
			RTC_ServiceModeSelect(0,2);
			break;

		case MSG_RTC_UP:
			RTC_RtcUp();
			break;

		case MSG_RTC_DOWN:
			RTC_RtcDown();
			break;

		#ifdef CFG_FUNC_SNOOZE_EN
		case MSG_RTC_SNOOZE:
			if(mainAppCt.AlarmRemindStart)
			{
				mainAppCt.AlarmRemindCnt = 0;
				mainAppCt.AlarmRemindStart = 0;
				mainAppCt.SnoozeOn = 1;
				mainAppCt.SnoozeCnt = 0;
			}
			break;
		#endif

		#endif //end of  CFG_FUNC_RTC_EN

		case MSG_REMIND1:
			#ifdef CFG_FUNC_REMIND_SOUND_EN
			gCtrlVars.remind_type = REMIND_TYPE_BACKGROUND;
			RemindSoundServiceItemRequest(SOUND_REMIND_ZHANGSHE,FALSE);
			#endif
			break;

		case MSG_DEVICE_SERVICE_BATTERY_LOW:
			//RemindSound request
			APP_DBG("MSG_DEVICE_SERVICE_BATTERY_LOW\n");
#ifdef CFG_FUNC_REMIND_SOUND_EN
			RemindSoundServiceItemRequest(SOUND_REMIND_DLGUODI, FALSE);
#endif
			break;

			//�������ӶϿ���Ϣ,������ʾ��
			case MSG_BT_STATE_CONNECTED:
				APP_DBG("[BT_STATE]:BT Connected...\n");
				//�쳣���������У�����ʾ���ӶϿ���ʾ��

				if(btManager.btDutModeEnable)
					break;

				if(!(btCheckEventList&BT_EVENT_L2CAP_LINK_DISCONNECT))
				{
					#ifdef CFG_FUNC_REMIND_SOUND_EN
					if(RemindSoundServiceItemRequest(SOUND_REMIND_CONNECT, FALSE))
					{
						if(!SoftFlagGet(SoftFlagWaitBtRemindEnd)&&SoftFlagGet(SoftFlagDelayEnterBtHf))
						{
							SoftFlagRegister(SoftFlagWaitBtRemindEnd);
						}
					}
					else
					#endif
					{
						if(SoftFlagGet(SoftFlagDelayEnterBtHf))
						{
							MessageContext		msgSend;
							SoftFlagDeregister(SoftFlagDelayEnterBtHf);

							msgSend.msgId = MSG_DEVICE_SERVICE_ENTER_BTHF_MODE;
							MessageSend(GetMainMessageHandle(), &msgSend);
						}
					}
				}
				break;

			case MSG_BT_STATE_DISCONNECT:
				APP_DBG("[BT_STATE]:BT Disconnected...\n");

				if(btManager.btDutModeEnable)
					break;

				APP_DBG("Bt Disconnect Reason:");
				switch(GetBtDisconReason())
				{
					case BT_DISCON_REASON_REMOTE_DIS:
						APP_DBG("BT_DISCON_REASON_REMOTE_DIS\n");
						break;

					case BT_DISCON_REASON_LOCAL_DIS:
						APP_DBG("BT_DISCON_REASON_LOCAL_DIS\n");
						break;

					case BT_DISCON_REASON_LINKLOSS:
						APP_DBG("BT_DISCON_REASON_LINKLOSS\n");
						break;

					default:
						break;
				}

				//�쳣���������У�����ʾ���ӶϿ���ʾ��
				if(!(btCheckEventList&BT_EVENT_L2CAP_LINK_DISCONNECT))
				{
					#ifdef CFG_FUNC_REMIND_SOUND_EN
					if((mainAppCt.appCurrentMode != AppModeBtHfPlay)&&(mainAppCt.appCurrentMode != AppModeBtRecordPlay))
						RemindSoundServiceItemRequest(SOUND_REMIND_DISCONNE, FALSE);
					else
						SoftFlagRegister(SoftFlagDiscDelayMask);
					#endif
				}
				break;

		default:
			#ifdef CFG_ADC_LEVEL_KEY_EN
			AdcLevelMsgProcess(Msg);
			#endif
			break;
	}
}


extern void rwip_reset(void);

//test, sam 20190702
void AudioResRelease(void);
bool AudioResMalloc(uint16_t SampleLen);
void AudioResInit(void);
void SamplesFrameUpdata(void);
//static bool IsEffectProcessDone = 1;
//void EffectProcessFlagSet(bool flag)
//{
//	IsEffectProcessDone = flag;
//}

//bool EffectProcessFlagGet(void)
//{
//	return IsEffectProcessDone;
//}

#ifdef	CFG_FUNC_REMIND_DEEPSLEEP
void PowerStateChange()
{
	//�ٴ�ע���Ϸ��ͻ�������һЩ�������ߵĲ�����
	if(!SoftFlagGet(SoftFlagDeepSleepRequest))
	{
		SoftFlagRegister(SoftFlagDeepSleepRequest | SoftFlagWaitSysRemind);
		mainAppCt.SysRemind = SOUND_REMIND_GUANJI;
		StartModeExit();
	}
}
#endif

extern uint16_t RstFlag;
extern uint8_t Report_Error_Code(void);
static void MainAppTaskEntrance(void * param)
{
	int16_t i;
	MessageContext		msg;

	DelayMsFunc = (DelayMsFunction)vTaskDelay; //���Os��������������ʱ�������ȣ���OSĬ��ʹ��DelayMs
	DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);
#ifdef CFG_FUNC_MAIN_DEEPSLEEP_EN


	WDG_Feed();
	vTaskDelay(3000); //���Խ׶���ر��ִ���ʱ���Է�оƬ����SW���޷��ٴ����أ�������flashboot
	WDG_Feed();
	vTaskDelay(2000);
	APP_DBG("DeepSleep\n");
//#ifdef CFG_PARA_WAKEUP_SOURCE_IR
//	IRKeyInit();//�˴�ΪIR���ó�ʼ�� 
////#endif
	SoftFlagDeregister(SoftFlagWakeUpSouceIsCEC);
	DeepSleeping();
#endif
	SarADC_Init();

	//For OTG check
#if defined(CFG_FUNC_UDISK_DETECT)
#if defined(CFG_FUNC_USB_DEVICE_EN)
 	OTG_PortSetDetectMode(1,1);
#else
 	OTG_PortSetDetectMode(1,0);
#endif
#else
#if defined(CFG_FUNC_USB_DEVICE_EN)
 	OTG_PortSetDetectMode(0,1);
#else
 	OTG_PortSetDetectMode(0,0);
#endif
#endif
 	Timer_Config(TIMER2,1000,0);
 	Timer_Start(TIMER2);
 	NVIC_EnableIRQ(Timer2_IRQn);

#ifdef CFG_FUNC_BREAKPOINT_EN
 	BP_LoadInfo();
#endif


	SysVarInit();
	
	SoftFlagDeregister(SoftFlagMask);

	CtrlVarsInit();//��Ч��ʼ��������������Ҫ����	
	
#ifdef CFG_APP_HDMIIN_MODE_EN
	HDMI_CEC_DDC_Init();
#endif

	///////////////////////////////AudioCore/////////////////////////////////////////
	mainAppCt.AudioCore =  (AudioCoreContext*)&AudioCore;
	memset(mainAppCt.AudioCore, 0, sizeof(AudioCoreContext));
	mainAppCt.SamplesPreFrame = CFG_PARA_SAMPLES_PER_FRAME;//Ĭ������֡��С
	mainAppCt.SampleRate = CFG_PARA_SAMPLE_RATE;
	if(!AudioResMalloc(mainAppCt.SamplesPreFrame))
	{
		APP_DBG("Res Error!\n");
		return;
	}
	
	//DAC init
#if defined (CFG_RES_AUDIO_DAC0_EN) && defined (CFG_RES_AUDIO_DACX_EN)
	AudioDAC_Init(ALL, mainAppCt.SampleRate, (void*)mainAppCt.DACFIFO, mainAppCt.SamplesPreFrame * 2 * 2 *2, (void*)mainAppCt.DACXFIFO, mainAppCt.SamplesPreFrame * 2 * 2);
#elif defined (CFG_RES_AUDIO_DAC0_EN)
	AudioDAC_Init(DAC0, mainAppCt.SampleRate, (void*)mainAppCt.DACFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2, NULL, 0);
#elif defined (CFG_RES_AUDIO_DACX_EN)
	AudioDAC_Init(DAC1, mainAppCt.SampleRate, NULL, 0, (void*)mainAppCt.DACXFIFO, mainAppCt.SamplesPreFrame * 2 * 2);
#else
	APP_DBG("DAC0 DACX All Disable!!!!!!!!!!\n");
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
    AudioI2sOutParamsSet();
#endif

	//Mic1 analog  = Soure0.
	AudioADC_AnaInit();
	//AudioADC_VcomConfig(1);//MicBias en
	AudioADC_MicBias1Enable(1);

    #if CFG_RES_MIC_SELECT
	AudioADC_DynamicElementMatch(ADC1_MODULE, TRUE, TRUE);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 15, 4);//0db bypass
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 15, 4);

	//Mic1   digital
	AudioADC_DigitalInit(ADC1_MODULE, mainAppCt.SampleRate, (void*)mainAppCt.ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);

	//Soure0.
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].Enable = 0;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].FuncDataGet = AudioADC1DataGet;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].FuncDataGetLen = AudioADC1DataLenGet;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].IsSreamData = TRUE;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmFormat = 2;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf = (int16_t *)mainAppCt.Source0Buf_ADC1;
	AudioCoreSourceEnable(MIC_SOURCE_NUM);
    #endif
	//sink0
	mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Enable = 0;
    #ifdef CFG_RES_AUDIO_DAC0_EN
	mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].FuncDataSet = AudioDAC0DataSet;
	mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].FuncDataSpaceLenGet = AudioDAC0DataSpaceLenGet;
    #else
	mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].FuncDataSet = AudioDAC0DataSetNull;
	mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].FuncDataSpaceLenGet = AudioDAC0DataSpaceLenGetNull;
    #endif
	mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].FuncDataSet = AudioDAC0DataSet;
	mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].FuncDataSpaceLenGet = AudioDAC0DataSpaceLenGet;
	mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf = (int16_t*)mainAppCt.Sink0Buf_DAC0;
	mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmFormat = 2;
	AudioCoreSinkEnable(AUDIO_DAC0_SINK_NUM);

#ifdef CFG_RES_AUDIO_DACX_EN
	//���ı�ԭ���ṹ��sink2 ����DAC-X�����Ը���ʵ���������
	mainAppCt.AudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Enable = 0;
	mainAppCt.AudioCore->AudioSink[AUDIO_DACX_SINK_NUM].FuncDataSet = AudioDAC1DataSet;
	mainAppCt.AudioCore->AudioSink[AUDIO_DACX_SINK_NUM].FuncDataSpaceLenGet = AudioDAC1DataSpaceLenGet;
	mainAppCt.AudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf = (int16_t*)mainAppCt.Sink2Buf_DACX;
	mainAppCt.AudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmFormat = 2;
	AudioCoreSinkEnable(AUDIO_DACX_SINK_NUM);
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	//���ı�ԭ���ṹ��sink2 ����I2S OUTs�����Ը���ʵ���������
	mainAppCt.AudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Enable = 0;
#if CFG_RES_I2S_MODE == 1
	AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].SreamDataState = 1;//I2S slave ʱ�����masterû�нӣ��п��ܻᵼ��DACҲ����������
#endif
#if (CFG_RES_I2S_PORT == 0)
	mainAppCt.AudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].FuncDataSet = AudioI2S0_DataSet;
	mainAppCt.AudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].FuncDataSpaceLenGet = AudioI2S0_DataSpaceLenGet;
#else
	mainAppCt.AudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].FuncDataSet = AudioI2S1_DataSet;
	mainAppCt.AudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].FuncDataSpaceLenGet = AudioI2S1_DataSpaceLenGet;
#endif
	mainAppCt.AudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf = (int16_t*)mainAppCt.Sink2Buf_I2S;
	mainAppCt.AudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmFormat = 2;
	AudioCoreSinkEnable(AUDIO_I2SOUT_SINK_NUM);
#endif

	////Audio Core��������
	SystemVolSet();
	
	for( i = 0; i < AUDIO_CORE_SOURCE_MAX_MUN; i++)
	{
	   mainAppCt.AudioCore->AudioSource[i].PreGain = 4095;//Ĭ��ʹ��4095�� 0dB
	}

	AudioCoreServiceCreate(mainAppCt.msgHandle);
	mainAppCt.AudioCoreSync = FALSE;
	DeviceServiceCreate(mainAppCt.msgHandle);
	mainAppCt.DeviceSync = FALSE;
#ifdef CFG_FUNC_DISPLAY_EN
	DisplayServiceCreate(mainAppCt.msgHandle);
	mainAppCt.DisplaySync = FALSE;
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
	RemindSoundServiceCreate(mainAppCt.msgHandle);
	mainAppCt.RemindSoundSync = FALSE;
#endif

#ifdef CFG_FUNC_ALARM_EN
	mainAppCt.AlarmFlag = FALSE;
#endif

#if (defined(CFG_APP_BT_MODE_EN) && defined(CFG_BT_BACKGROUND_RUN_EN))
	//���������񴴽������˴�,�Ա���������Э��ջʹ�õ��ڴ�ռ�,��Ӱ������������; ����˯��ʱ������stack�ٴο��������ϵ������˳���
	BtStackServiceStart();//�����豸����serivce ����ʧ��ʱ��Ŀǰ�ǹ�����ͬ����Ϣ��
//	IRKeyInit();//clkԴ���ģ�
#endif
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	{
		extern TIMER EffectChangeTimer;
		TimeOutSet(&EffectChangeTimer, 120);
	}
#endif
	APP_DBG("run\n");
	

	while(1)
	{
		
		
		WDG_Feed();
		MessageRecv(mainAppCt.msgHandle, &msg, MAIN_APP_MSG_TIMEOUT);
	
#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
        extern uint32_t  Silence_Power_Off_Time;
        if(!Silence_Power_Off_Time)
		{
			Silence_Power_Off_Time = SILENCE_POWER_OFF_DELAY_TIME;
			APP_DBG("power off\n");
			#ifdef	CFG_FUNC_POWERKEY_EN
			msg.msgId =  MSG_POWERDOWN;
			#endif

			#ifdef	CFG_FUNC_DEEPSLEEP_EN
			msg.msgId =  MSG_DEEPSLEEP;
			#endif
        }
#endif
#if FLASH_BOOT_EN
		//�豸�������ţ�Ԥ����mva���Ǽǣ����������γ�ʱȡ���Ǽǡ�
		#ifndef FUNC_UPDATE_CONTROL
		if((Report_Error_Code() == UPDAT_OK) || (Report_Error_Code() == NEEDLESS_UPDAT) 
			/*||(Report_Error_Code() == USER_CODE_RUN_START && RstFlag == 0x4)*/)
		{
			;
		}
		else if(SoftFlagGet(SoftFlagMvaInCard) && GetSystemMode() == AppModeCardAudioPlay)
		{
			APP_DBG("updata file exist in Card\n");
			#ifdef FUNC_OS_EN
			if(SDIOMutex != NULL)
			{
				osMutexLock(SDIOMutex);
			}
			#endif
			start_up_grate(AppResourceCard);
			#ifdef FUNC_OS_EN
			if(SDIOMutex != NULL)
			{
				osMutexUnlock(SDIOMutex);
			}
			#endif
		}
		else if(SoftFlagGet(SoftFlagMvaInUDisk) && GetSystemMode() == AppModeUDiskAudioPlay)
		{
			APP_DBG("updata file exist in Udisk\n");
			#ifdef FUNC_OS_EN
			if(UDiskMutex != NULL)
			{
				//osMutexLock(UDiskMutex);
				while(osMutexLock_1000ms(UDiskMutex) != 1)
				{
					WDG_Feed();
				}
			}
			#endif
			start_up_grate(AppResourceUDisk);
			#ifdef FUNC_OS_EN
			if(UDiskMutex != NULL)
			{
				osMutexUnlock(UDiskMutex);
			}
			#endif
		}
		#endif
#endif		
		switch(msg.msgId)
		{
#ifdef CFG_FUNC_REMIND_SOUND_EN
			case MSG_REMIND_SOUND_SERVICE_CREATED:
#endif
#ifdef CFG_FUNC_DISPLAY_EN
			case MSG_DISPLAY_SERVICE_CREATED:
#endif
			case MSG_DEVICE_SERVICE_CREATED:
			case MSG_AUDIO_CORE_SERVICE_CREATED:	
				MainAppServiceCreating(msg.msgId);
				break;
			
#ifdef CFG_FUNC_REMIND_SOUND_EN
			case MSG_REMIND_SOUND_SERVICE_STARTED:
#endif
#ifdef CFG_FUNC_DISPLAY_EN
			case MSG_DISPLAY_SERVICE_STARTED:
#endif
			case MSG_DEVICE_SERVICE_STARTED:
			case MSG_AUDIO_CORE_SERVICE_STARTED:
				MainAppServiceStarting(msg.msgId);
				break;
#if FLASH_BOOT_EN
			case MSG_DEVICE_SERVICE_CARD_OUT:
				SoftFlagDeregister(SoftFlagMvaInCard);//����mva�����
				break;
			
			case MSG_DEVICE_SERVICE_DISK_OUT:
				SoftFlagDeregister(SoftFlagMvaInUDisk);//����mva�����
				break;
			
			case MSG_UPDATE:
				#ifdef FUNC_UPDATE_CONTROL
				APP_DBG("UPDATE MSG\n");
				//�豸�������ţ�Ԥ����mva���Ǽǣ����������γ�ʱȡ���Ǽǡ�
				if(SoftFlagGet(SoftFlagMvaInCard) && GetSystemMode() == AppModeCardAudioPlay)
				{
					APP_DBG("updata file exist in Card\n");
					start_up_grate(AppResourceCard);
				}
				else if(SoftFlagGet(SoftFlagMvaInUDisk) && GetSystemMode() == AppModeUDiskAudioPlay)
				{
					APP_DBG("updata file exist in Udisk\n");
					start_up_grate(AppResourceUDisk);
				}
				#endif
				break;
#endif

#ifdef	CFG_FUNC_POWERKEY_EN
			case MSG_POWERDOWN:

#ifdef	CFG_FUNC_REMIND_DEEPSLEEP
				PowerStateMsgHandle = MSG_POWERDOWN;
				PowerStateChange();
				break;
			case MSG_POWERCHAGE_POWERDOWN:
#endif
				APP_DBG("POWERDOWN\n");
				//��Ϣת������ǰAPP��APP�����ֳ�������Stopģʽ������Ϣ����main
				//SystemPowerDown();
				SystemGotoPowerDown();
				break;
#endif
#ifdef CFG_FUNC_DEEPSLEEP_EN
			case MSG_DEEPSLEEP:
#ifdef CFG_APP_HDMIIN_MODE_EN
				if(GetSystemMode() == AppModeHdmiAudioPlay)
				{
					if(SoftFlagGet(SoftFlagDeepSleepMsgIsFromTV) == 0)//�ǵ��Ӷ˷����Ĺػ�
					{
						HDMI_CEC_ActivePowerOff(200);
					}
					SoftFlagDeregister(SoftFlagDeepSleepMsgIsFromTV);
				}
#endif
#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE))
				if(GetSystemMode() == AppModeBtHfPlay)
				{
					if(SoftFlagGet(SoftFlagBtHfDelayEnterSleep) == 0)//ͨ�������йػ�
					{
						gEnterBtHfMode = 0;
						ExitBtHfMode();
						SoftFlagRegister(SoftFlagBtHfDelayEnterSleep);
					}
				}
#endif
#ifdef	CFG_FUNC_REMIND_DEEPSLEEP
//				if(!SoftFlagGet(SoftFlagDeepSleepRequest))
//				{
//					SoftFlagRegister(SoftFlagDeepSleepRequest | SoftFlagWaitSysRemind);
//					mainAppCt.SysRemind = SOUND_REMIND_GUANJI;
//					StartModeExit();
//				}
				PowerStateMsgHandle = MSG_DEEPSLEEP;
				PowerStateChange();
				break;
			case MSG_TASK_DEEPSLEEP:
#endif
				EndModeExit_set = MSG_NONE;
				MainAppServicePause();
				break;
			case MSG_BTSTACK_DEEPSLEEP:
#ifdef	CFG_FUNC_REMIND_DEEPSLEEP
				PowerStateMsgHandle = MSG_BTSTACK_DEEPSLEEP;
				PowerStateChange();
				break;
			case MSG_TASK_BTSNIFF:
#endif
				EndModeExit_set = MSG_BTSTACK_DEEPSLEEP;
				MainAppServicePauseBt();
				break;
			case MSG_AUDIO_CORE_SERVICE_PAUSED:
			case MSG_DEVICE_SERVICE_PAUSED:
			case MSG_REMIND_SOUND_SERVICE_PAUSED:
			case MSG_DISPLAY_SERVICE_PAUSED:
				if(mainAppCt.state == TaskStatePausing)// ״̬����
				{
					MainAppServicePausing(msg.msgId);
				}
				break;
#endif

#ifdef CFG_SOFT_POWER_KEY_EN
			case MSG_SOFT_POWER:
				SoftKeyPowerOff();
				break;
#endif

#ifdef CFG_APP_BT_MODE_EN
			case MSG_BT_ENTER_DUT_MODE:
				BtEnterDutModeFunc();
				break;
				
			case MSG_BTSTACK_BB_ERROR:
				APP_DBG("bb and bt stack reset\n");
				RF_PowerDownBySw();
				WDG_Feed();
				//reset bb and bt stack
				rwip_reset();
				BtStackServiceKill();
				WDG_Feed();
				vTaskDelay(50);
				RF_PowerDownByHw();
				WDG_Feed();
				//reset bb and bt stack
				BtStackServiceStart();
				break;


			


#if (BT_HFP_SUPPORT == ENABLE)
				case MSG_DEVICE_SERVICE_ENTER_BTHF_MODE:
				{
					EnterBtHfMode();
				}
				break;
#endif
#ifdef BT_AUTO_ENTER_PLAY_MODE
			case MSG_BT_A2DP_STREAMING:
				//���Ÿ���ʱ,��ģʽ�л�����,���ڴ���Ϣ�п�ʼ����ģʽ�л�����
				if((mainAppCt.appCurrentMode != AppModeBtAudioPlay)&&(mainAppCt.appCurrentMode != AppModeBtHfPlay))
				{
					MessageContext		msgSend;
					
					APP_DBG("Enter Bt Audio Play Mode...\n");
					ResourceRegister(AppResourceBtPlay);
					
					// Send message to main app
					msgSend.msgId		= MSG_DEVICE_SERVICE_BTPLAY_IN;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				break;
#endif
#endif
		}
		//����̬ �� deepsleepǰ���˳�����
		if(mainAppCt.state == TaskStateRunning ||(mainAppCt.state == TaskStatePausing && (mainAppCt.MState != ModeStatePause || mainAppCt.MState != ModeStateNone)))
		{
			if(msg.msgId == MSG_AUDIO_CORE_EFFECT_CHANGE)
			{
				APP_DBG("msg.msgId == MSG_AUDIO_CORE_EFFECT_CHANGE\n");
#ifdef CFG_RES_AUDIO_DAC0_EN
				AudioDAC_DigitalMute(DAC0, TRUE, TRUE);
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
				AudioDAC_DigitalMute(DAC1, TRUE, TRUE);
#endif
				vTaskDelay(15);

				gCtrlVars.audio_effect_init_flag = 1;
				AudioEffectsDeInit();//�ͷ���Ч�ڴ�
				if(mainAppCt.appCurrentMode != AppModeBtHfPlay)
				{
					AudioEffectsInit();//���·�����Ч�ڴ棬��������������λ���ᵥ������ĳ����Ч
					AudioEffectParamSync();//��Ч�������º���Ҫ��ͬ����ز�����������
				}
				if(GetAudioCoreServiceState() == TaskStatePaused)
				{
					AudioCoreServiceResume();
				}
				gCtrlVars.audio_effect_init_flag = 0;

#ifdef BT_RECORD_FUNC_ENABLE
				{
					extern void BtRecordAudioResInitOk(void);
					if(GetSystemMode() == AppModeBtRecordPlay)
						BtRecordAudioResInitOk();
				}
#endif
				vTaskDelay(15);
#ifdef CFG_RES_AUDIO_DAC0_EN
				AudioDAC_DigitalMute(DAC0, FALSE, FALSE);
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
				AudioDAC_DigitalMute(DAC1, FALSE, FALSE);

#endif
			}
			//��̬�ڴ��С����
			if(msg.msgId == MSG_AUDIO_CORE_FRAME_SIZE_CHANGE)
			{
				AudioEffectsDeInit();//�ͷ���Ч�ڴ�
				SoftFlagRegister(SoftFlagFrameSizeChange);
				APP_DBG("msg.msgId == MSG_AUDIO_CORE_FRAME_SIZE_CHANGE\n");
				
				mainAppCt.SourcesMuteState = 0;
				for(i = 0; i<AUDIO_CORE_SOURCE_MAX_MUN; i++)
				{
					mainAppCt.SourcesMuteState <<= 1;
						//APP_DBG("mainAppCt.AudioCore->AudioSource[%d].LeftMuteFlag =%d\n", i, mainAppCt.AudioCore->AudioSource[i].LeftMuteFlag);
					if(mainAppCt.AudioCore->AudioSource[i].LeftMuteFlag == TRUE)
					{
						mainAppCt.SourcesMuteState |= 0x01;
					}
					//APP_DBG("mainAppCt.SourcesMuteState = %x\n", mainAppCt.SourcesMuteState);
					AudioCoreSourceMute(i, 1, 1);
				}
				vTaskDelay(10);

				AudioADC_DigitalMute(ADC1_MODULE, 1, 1);
				vTaskDelay(10);
#ifdef CFG_RES_AUDIO_DAC0_EN
				AudioDAC_DigitalMute(DAC0, TRUE, TRUE);
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
				AudioDAC_DigitalMute(DAC1, TRUE, TRUE);
#endif
				vTaskDelay(15);

				if(GetAudioCoreServiceState() == TaskStatePaused)
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_AUDIO_CORE_SERVICE_PAUSED;
					MessageSend(GetMainMessageHandle(), &msgSend);
				}
				else
				{
					AudioCoreServicePause();
				}
			}
			if(msg.msgId == MSG_AUDIO_CORE_SERVICE_PAUSED)
			{
				if(GetAppMessageHandle() != NULL)
				{
					MessageContext		msgSend;
					msgSend.msgId = MSG_APP_RES_RELEASE;
					MessageSend(GetAppMessageHandle(), &msgSend);
				}
			}
			if(msg.msgId == MSG_APP_RES_RELEASE_SUC)
			{
				APP_DBG("MSG_APP_RES_RELEASE_SUC\n");
				AudioResRelease();
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
				AudioEffectsDeInit();
#endif
				SamplesFrameUpdata();//AudioCore Pause ס֮������л�ϵͳ��֡��С
				if(!AudioResMalloc(mainAppCt.SamplesPreFrame))
				{
					APP_DBG("Res Error!\n");
					return;
				}
				{
					MessageContext		msgSend;
					if(GetAppMessageHandle() != NULL)
					{
						msgSend.msgId = MSG_APP_RES_MALLOC;
						MessageSend(GetAppMessageHandle(), &msgSend);
					}
				}
			}
			if(msg.msgId == MSG_APP_RES_MALLOC_SUC)
			{
				//APP_DBG("MSG_APP_RES_MALLOC_SUC\n");
				AudioResInit();
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
				gCtrlVars.audio_effect_init_flag = 1;
				if(mainAppCt.appCurrentMode != AppModeBtHfPlay)
				{
					AudioEffectsInit();
					AudioEffectParamSync();//��Ч�������º���Ҫ��ͬ����ز�����������
				}
				gCtrlVars.audio_effect_init_flag = 0;
#endif				
				{
					MessageContext		msgSend;
					if(GetAppMessageHandle() != NULL)
					{
						msgSend.msgId = MSG_APP_RES_INIT;
						MessageSend(GetAppMessageHandle(), &msgSend);
					}
				}
			}
			if(msg.msgId == MSG_APP_RES_INIT_SUC)
			{
				//APP_DBG("(msg.msgId == MSG_APP_RES_INIT_SUC)\n");
//				AudioCoreDeinit();
				AudioCoreServiceResume();

				AudioADC_DigitalMute(ADC1_MODULE, 0, 0);
				vTaskDelay(10);

				for(i = 0; i<AUDIO_CORE_SOURCE_MAX_MUN; i++)
				{
					if((mainAppCt.SourcesMuteState & 0x01) == 0x00)//sourcr ��ԭ������unmute״̬����ԭsource״̬
					{
						AudioCoreSourceUnmute(AUDIO_CORE_SOURCE_MAX_MUN - 1 - i, 1, 1);
						//APP_DBG("i = %d, mainAppCt.SourcesMuteState = %x\n", i, mainAppCt.SourcesMuteState);
					}
					mainAppCt.SourcesMuteState >>= 1;
				}
				vTaskDelay(10);
#ifdef CFG_RES_AUDIO_DAC0_EN
				AudioDAC_DigitalMute(DAC0, FALSE, FALSE);
#endif
				
#ifdef CFG_RES_AUDIO_DACX_EN
				AudioDAC_DigitalMute(DAC1, FALSE, FALSE);
				
#endif
				SoftFlagDeregister(SoftFlagFrameSizeChange);
			}
			
			MainAppModeProcess(msg.msgId);
			
#ifdef	CFG_FUNC_AUDIO_EFFECT_EN
			if(mainAppCt.appCurrentMode != AppModeWaitingPlay && mainAppCt.MState == ModeStateWork)
			{
				void EffectChange(void);
				EffectChange();
			}
#endif
		}
	}
}

/***************************************************************************************
 *
 * APIs
 *
 */

int32_t MainAppTaskStart(void)
{
	MainAppInit();
#ifdef CFG_FUNC_SHELL_EN
	shell_init();
	xTaskCreate(mv_shell_task, "SHELL", SHELL_TASK_STACK_SIZE, NULL, SHELL_TASK_PRIO, NULL);
#endif
	xTaskCreate(MainAppTaskEntrance, "MainApp", MAIN_APP_TASK_STACK_SIZE, NULL, MAIN_APP_TASK_PRIO, &mainAppCt.taskHandle);
	return 0;
}

MessageHandle GetMainMessageHandle(void)
{
	return mainAppCt.msgHandle;
}


uint32_t GetSystemMode(void)
{
	return mainAppCt.appCurrentMode;
}

void SoftFlagRegister(uint32_t SoftEvent)
{
	SoftwareFlag |= SoftEvent;
	return ;
}

void SoftFlagDeregister(uint32_t SoftEvent)
{
	SoftwareFlag &= ~SoftEvent;
	return ;
}

bool SoftFlagGet(uint32_t SoftEvent)
{
	return SoftwareFlag & SoftEvent ? TRUE : FALSE;
}

void SysVarInit(void)
{
	int16_t i;
#ifdef CFG_FUNC_BREAKPOINT_EN
	BP_SYS_INFO *pBpSysInfo;

	pBpSysInfo = (BP_SYS_INFO *)BP_GetInfo(BP_SYS_INFO_TYPE);

#ifdef CFG_FUNC_MAIN_DEEPSLEEP_EN
	if(SoftFlagGet(SoftFlagWakeUpSouceIsCEC))
	{
		pBpSysInfo->CurModuleId = AppModeHdmiAudioPlay;
	}
	SoftFlagDeregister(SoftFlagWakeUpSouceIsCEC);
#endif
	if(!ModeResumeMask(pBpSysInfo->CurModuleId))
	{
		mainAppCt.appBackupMode = pBpSysInfo->CurModuleId;//�ϵ����Waiting���ϵ����ģʽʹ��backup�ָ���
		if((mainAppCt.appBackupMode <= AppModeWaitingPlay) || (mainAppCt.appBackupMode >= AppModeCardPlayBack))
		{
			mainAppCt.appBackupMode = AppModeBtAudioPlay;
		}
	}
	//mainAppCt.appBackupMode = AppModeBtAudioPlay;
	APP_DBG("%d,%d\n", mainAppCt.appCurrentMode, pBpSysInfo->CurModuleId);
    
	mainAppCt.MusicVolume = pBpSysInfo->MusicVolume;
	if((mainAppCt.MusicVolume > CFG_PARA_MAX_VOLUME_NUM) || (mainAppCt.MusicVolume <= 0))
	{
		mainAppCt.MusicVolume = CFG_PARA_MAX_VOLUME_NUM;
	}
	APP_DBG("MusicVolume:%d,%d\n", mainAppCt.MusicVolume, pBpSysInfo->MusicVolume);	
	
	#ifdef CFG_FUNC_MIC_KARAOKE_EN
	mainAppCt.EffectMode = pBpSysInfo->EffectMode;
	if((mainAppCt.EffectMode > EFFECT_MODE_NvBianNan) || (mainAppCt.EffectMode <= 10))
	{
		mainAppCt.EffectMode = EFFECT_MODE_HunXiang;
	}
	#else
	mainAppCt.EffectMode = EFFECT_MODE_NORMAL;
	#endif
	APP_DBG("EffectMode:%d,%d\n", mainAppCt.EffectMode, pBpSysInfo->EffectMode);
	
	mainAppCt.MicVolume = pBpSysInfo->MicVolume;
	if((mainAppCt.MicVolume > CFG_PARA_MAX_VOLUME_NUM) || (mainAppCt.MicVolume <= 0))
	{
		mainAppCt.MicVolume = CFG_PARA_MAX_VOLUME_NUM;
	}
	mainAppCt.MicVolumeBak = mainAppCt.MicVolume;
	APP_DBG("MicVolume:%d,%d\n", mainAppCt.MicVolume, pBpSysInfo->MicVolume);

	#ifdef CFG_APP_BT_MODE_EN
	mainAppCt.HfVolume = pBpSysInfo->HfVolume;
	if((mainAppCt.HfVolume > CFG_PARA_MAX_VOLUME_NUM) || (mainAppCt.HfVolume <= 0))
	{
		mainAppCt.HfVolume = CFG_PARA_MAX_VOLUME_NUM;
	}
	APP_DBG("HfVolume:%d,%d\n", mainAppCt.HfVolume, pBpSysInfo->HfVolume);
	#endif
	
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	mainAppCt.EqMode = pBpSysInfo->EqMode;
	if(mainAppCt.EqMode > EQ_MODE_VOCAL_BOOST)
	{
		mainAppCt.EqMode = EQ_MODE_FLAT;
	}
	#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN    
	mainAppCt.EqModeBak = mainAppCt.EqMode;
	mainAppCt.EqModeFadeIn = 0;
	mainAppCt.eqSwitchFlag = 0;
	#endif
	APP_DBG("EqMode:%d,%d\n", mainAppCt.EqMode, pBpSysInfo->EqMode);
#endif

	mainAppCt.ReverbStep = pBpSysInfo->ReverbStep;
    if((mainAppCt.ReverbStep > MAX_MIC_DIG_STEP) || (mainAppCt.ReverbStep <= 0))
	{
		mainAppCt.ReverbStep = MAX_MIC_DIG_STEP;
	}
	mainAppCt.ReverbStepBak = mainAppCt.ReverbStep;
	APP_DBG("ReverbStep:%d,%d\n", mainAppCt.ReverbStep, pBpSysInfo->ReverbStep);
	
#ifdef CFG_FUNC_MIC_TREB_BASS_EN	
    mainAppCt.MicBassStep = pBpSysInfo->MicBassStep;
    if((mainAppCt.MicBassStep > MAX_MIC_DIG_STEP) || (mainAppCt.MicBassStep <= 0))
	{
		mainAppCt.MicBassStep = 7;
	}
    mainAppCt.MicTrebStep = pBpSysInfo->MicTrebStep;
    if((mainAppCt.MicTrebStep > MAX_MIC_DIG_STEP) || (mainAppCt.MicTrebStep <= 0))
	{
		mainAppCt.MicTrebStep = 7;
	}
	mainAppCt.MicTrebStepBak = mainAppCt.MicTrebStep;
	mainAppCt.MicBassStepBak = mainAppCt.MicBassStep;
	APP_DBG("MicTrebStep:%d,%d\n", mainAppCt.MicTrebStep, pBpSysInfo->MicTrebStep);
	APP_DBG("MicBassStep:%d,%d\n", mainAppCt.MicBassStep, pBpSysInfo->MicBassStep);
#endif

#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN	
    mainAppCt.MusicBassStep = pBpSysInfo->MusicBassStep;
    if((mainAppCt.MusicBassStep > MAX_MUSIC_DIG_STEP) || (mainAppCt.MusicBassStep <= 0))
	{
		mainAppCt.MusicBassStep = 7;
	}
    mainAppCt.MusicTrebStep = pBpSysInfo->MusicTrebStep;
    if((mainAppCt.MusicTrebStep > MAX_MUSIC_DIG_STEP) || (mainAppCt.MusicTrebStep <= 0))
	{
		mainAppCt.MusicTrebStep = 7;
	}
	APP_DBG("MusicTrebStep:%d,%d\n", mainAppCt.MusicTrebStep, pBpSysInfo->MusicTrebStep);
	APP_DBG("MusicBassStep:%d,%d\n", mainAppCt.MusicBassStep, pBpSysInfo->MusicBassStep);
#endif

#else
	mainAppCt.appBackupMode = AppModeBtAudioPlay;		  
	mainAppCt.MusicVolume = CFG_PARA_MAX_VOLUME_NUM;
	#ifdef CFG_APP_BT_MODE_EN
	mainAppCt.HfVolume = CFG_PARA_MAX_VOLUME_NUM;
	#endif
	#ifdef CFG_FUNC_MIC_EN
	mainAppCt.EffectMode = EFFECT_MODE_HunXiang;
	#else
	mainAppCt.EffectMode = EFFECT_MODE_NORMAL;
	#endif
	mainAppCt.MicVolume = CFG_PARA_MAX_VOLUME_NUM;
	
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	mainAppCt.EqMode = EQ_MODE_FLAT;
 	#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN    
    mainAppCt.EqModeBak = mainAppCt.EqMode;
	mainAppCt.eqSwitchFlag = 0;
	#endif
#endif
	mainAppCt.ReverbStep = MAX_MIC_DIG_STEP;

#ifdef CFG_FUNC_MIC_TREB_BASS_EN	
	mainAppCt.MicBassStep = 7;
	mainAppCt.MicTrebStep = 7;
#endif	

#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN	
	mainAppCt.MusicBassStep = 7;
	mainAppCt.MusicTrebStep = 7;
#endif	

#endif

#ifdef CFG_APP_HDMIIN_MODE_EN
	mainAppCt.hdmiArcOnFlg = 1;
	mainAppCt.hdmiResetFlg = 0;
#endif
    for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
	{
		mainAppCt.gSysVol.AudioSinkVol[i] = CFG_PARA_MAX_VOLUME_NUM;
	}

	for(i = 0; i < AUDIO_CORE_SOURCE_MAX_MUN; i++)
	{
		if(i == MIC_SOURCE_NUM)
		{
			mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = mainAppCt.MicVolume;
		}
		else if(i == APP_SOURCE_NUM)
		{
			mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.MusicVolume;
		}
		else if(i == REMIND_SOURCE_NUM)
		{
			#if CFG_PARAM_FIXED_REMIND_VOL
			mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = CFG_PARAM_FIXED_REMIND_VOL;
			#else
			mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = mainAppCt.MusicVolume;
			#endif
		}
		else
		{
			mainAppCt.gSysVol.AudioSourceVol[i] = CFG_PARA_MAX_VOLUME_NUM;		
		}
	}
	mainAppCt.gSysVol.MuteFlag = FALSE;	
}

#ifdef	CFG_FUNC_POWERKEY_EN
void SystemGotoPowerDown(void)
{
	MessageContext		msgSend;

	msgSend.msgId = MSG_TASK_POWERDOWN;
	MessageSend(GetAppMessageHandle(), &msgSend);
}
#endif

#if defined(CFG_FUNC_DEEPSLEEP_EN) || defined(CFG_FUNC_MAIN_DEEPSLEEP_EN)
inline void SleepMainAppTask(void)
{
	AudioCoreSourceDisable(MIC_SOURCE_NUM);
//	OTG_PortSetDetectMode(0,0);//����usb/device ɨ������

#ifdef DISP_DEV_7_LED
	NVIC_DisableIRQ(Timer6_IRQn);
#endif
	NVIC_DisableIRQ(Timer2_IRQn);
	WDG_Disable();
//	OTG_PowerDown(); //usb device ���� �� ��ֹ��
	OTG_DeepSleepBackup();
#ifdef CFG_FUNC_UDISK_DETECT
	UDiskRemovedFlagSet(TRUE);
#endif
	ADC_Disable();//����ɨ��
	AudioADC_PowerDown();
	AudioADC_VcomConfig(2);//ע�⣬VCOM���DAC�����ص�������2��PowerDown VCOM

	SPDIF_AnalogModuleDisable();//spdif,HDMI
#ifdef CFG_RES_AUDIO_DAC0_EN
	AudioDAC_PowerDown(DAC0);
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	AudioDAC_PowerDown(DAC1);
#endif
}


inline void WakeupMainAppTask(void)
{
//	OTG_PortSetDetectMode(1,1);

#ifdef DISP_DEV_7_LED
	NVIC_EnableIRQ(Timer6_IRQn);
#endif
//	OTG_DeviceInit();	//	OTG_PowerDown(); //usb device ����, pc������� ��

	//��Ӧ AudioDAC_PowerDown(DAC0);

	//DAC init
#if defined (CFG_RES_AUDIO_DAC0_EN) && defined (CFG_RES_AUDIO_DACX_EN)
	AudioDAC_Init(ALL, mainAppCt.SampleRate, (void*)mainAppCt.DACFIFO, mainAppCt.SamplesPreFrame * 2 * 2 *2, (void*)mainAppCt.DACXFIFO, mainAppCt.SamplesPreFrame * 2 * 2);
#elif defined (CFG_RES_AUDIO_DAC0_EN)
	AudioDAC_Init(DAC0, mainAppCt.SampleRate, (void*)mainAppCt.DACFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2, NULL, 0);
#elif defined (CFG_RES_AUDIO_DACX_EN)
	AudioDAC_Init(DAC1, mainAppCt.SampleRate, NULL, 0, (void*)mainAppCt.DACXFIFO, mainAppCt.SamplesPreFrame * 2 * 2);
#else
	APP_DBG("DAC0 DACX All Disable!!!!!!!!!!\n");
#endif

	//Mic1  analog ��ӦAudioADC_PowerDown(); ע��dacҪ���䡣
	AudioADC_AnaInit();
	AudioADC_VcomConfig(1);//MicBias en
	AudioADC_MicBias1Enable(1);
#if CFG_RES_MIC_SELECT
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);

	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 15, 4);//0db bypass
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 15, 4);

	//Mic1   digital
	AudioADC_DigitalInit(ADC1_MODULE, mainAppCt.SampleRate, (void*)mainAppCt.ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
	AudioCoreSourceEnable(MIC_SOURCE_NUM);
#endif
//	SPDIF_AnalogModuleDisable();//spdif,HDMI

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	AudioI2sOutParamsSet();
#endif

#if defined(CFG_RES_ADC_KEY_SCAN) || defined(CFG_FUNC_POWER_MONITOR_EN)
	SarADC_Init();
#endif

#ifdef CFG_RES_ADC_KEY_USE	
	AdcKeyInit();
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IR) && defined(CFG_RES_IR_KEY_USE)
	IRKeyInit();
#endif
	OTG_WakeupResume();
 	Timer_Config(TIMER2,1000,0);
 	Timer_Start(TIMER2);
 	Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
	NVIC_EnableIRQ(Timer2_IRQn);
#if defined(CFG_FUNC_USB_DEVICE_EN) || defined(CFG_FUNC_UDISK_DETECT)
	vTaskDelay(100);
	OTG_PortLinkCheck();
#if (defined(CFG_APP_USB_AUDIO_MODE_EN))&&(defined(CFG_COMMUNICATION_BY_USB))
	SetDeviceDetectVarReset_Deepsleep();
#endif
#endif
}

#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
void AudioI2sOutParamsSet(void)
{
	I2SParamCt i2s_set;
	i2s_set.IsMasterMode=CFG_RES_I2S_MODE;// 0:master 1:slave
	i2s_set.SampleRate=mainAppCt.SampleRate;
	i2s_set.I2sFormat=I2S_FORMAT_I2S;
	i2s_set.I2sBits=I2S_LENGTH_16BITS;
	i2s_set.I2sTxRxEnable=1;
#if (CFG_RES_I2S_PORT == 0)
	i2s_set.TxPeripheralID=PERIPHERAL_ID_I2S0_TX;
#else
	i2s_set.TxPeripheralID=PERIPHERAL_ID_I2S1_TX;
#endif
	i2s_set.TxBuf=(void*)mainAppCt.I2SFIFO;
	i2s_set.TxLen=mainAppCt.SamplesPreFrame * 2 * 2 * 2;
#if (CFG_RES_I2S_IO_PORT==0)
//i2s0  group_gpio0
	GPIO_PortAModeSet(GPIOA0, 9);// mclk out
	GPIO_PortAModeSet(GPIOA1, 6);// lrclk
	GPIO_PortAModeSet(GPIOA2, 5);// bclk
	GPIO_PortAModeSet(GPIOA3, 7);// dout
	GPIO_PortAModeSet(GPIOA4, 1);// din
//i2s0  group_gpio0
#else //lif (CFG_RES_I2S_IO_PORT==1)
//i2s1  group_gpio1 
	GPIO_PortAModeSet(GPIOA7, 5);//mclk out
	GPIO_PortAModeSet(GPIOA8, 1);//lrclk
	GPIO_PortAModeSet(GPIOA9, 2);//bclk
	GPIO_PortAModeSet(GPIOA10, 4);//do
	GPIO_PortAModeSet(GPIOA11, 2);//di
//i2s1  group_gpio1
#endif
#if (CFG_RES_I2S_PORT == 0)
	AudioI2S_Init(I2S0_MODULE, &i2s_set);
#else
	AudioI2S_Init(I2S1_MODULE, &i2s_set);
#endif
}
#endif

//mic channel: 
void AudioADC1ParamsSet(uint32_t sampleRate, uint16_t gain, uint8_t gainBoostSel)
{
#if CFG_RES_MIC_SELECT
	AudioCoreSourceDisable(MIC_SOURCE_NUM);
	AudioADC_AnaInit();
	//AudioADC_VcomConfig(1);//MicBias en
	AudioADC_MicBias1Enable(1);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);
	AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, gain, gainBoostSel);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, gain, gainBoostSel);

	//Mic1	 digital
	memset(mainAppCt.ADCFIFO, 0, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
	AudioADC_DigitalInit(ADC1_MODULE, sampleRate, (void*)mainAppCt.ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
#endif
}

//��������HFͨ��ʱ,MIC�����ϴ�,�˳���״̬,�ָ���֮ǰ״̬
void ResumeAudioCoreMicSource(void)
{
#if CFG_RES_MIC_SELECT
	AudioADC1ParamsSet(mainAppCt.SampleRate, 15, 4);
	
	//Soure0.
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].Enable = 1;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].FuncDataGet = AudioADC1DataGet;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].FuncDataGetLen = AudioADC1DataLenGet;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].IsSreamData = TRUE;
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmFormat = 2;//test mic audio effect
	mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf = (int16_t *)mainAppCt.Source0Buf_ADC1;
	AudioCoreSourceEnable(MIC_SOURCE_NUM);
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	AudioDAC_DigitalMute(DAC1, TRUE, TRUE);
	vTaskDelay(20);	
	AudioDAC_Pause(DAC1);
	AudioDAC_Disable(DAC1);
	AudioDAC_Reset(DAC1);
	AudioDAC_SampleRateChange(DAC1, CFG_PARA_SAMPLE_RATE);//�ָ�
	AudioDAC_Enable(DAC1);
	AudioDAC_Run(DAC1);
	AudioDAC_DigitalMute(DAC1, FALSE, FALSE);
#endif

#ifdef CFG_RES_AUDIO_DAC0_EN
	AudioDAC_DigitalMute(DAC0, TRUE, TRUE);
	vTaskDelay(20);
	AudioDAC_Pause(DAC0);
	AudioDAC_Disable(DAC0);
	AudioDAC_Reset(DAC0);
	AudioDAC_SampleRateChange(DAC0, CFG_PARA_SAMPLE_RATE);//�ָ�
	AudioDAC_Enable(DAC0);
	AudioDAC_Run(DAC0);
	AudioDAC_DigitalMute(DAC0, FALSE, FALSE);
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
{
	I2SParamCt i2s_set;
	i2s_set.IsMasterMode=CFG_RES_I2S_MODE;// 0:master 1:slave
	i2s_set.SampleRate=mainAppCt.SampleRate;
	i2s_set.I2sFormat=I2S_FORMAT_I2S;
	i2s_set.I2sBits=I2S_LENGTH_16BITS;
	i2s_set.I2sTxRxEnable=1;
#if (CFG_RES_I2S_PORT == 0)
	i2s_set.TxPeripheralID=PERIPHERAL_ID_I2S0_TX;
#else
	i2s_set.TxPeripheralID=PERIPHERAL_ID_I2S1_TX;
#endif
	i2s_set.TxBuf=(void*)mainAppCt.I2SFIFO;
	i2s_set.TxLen=mainAppCt.SamplesPreFrame * 2 * 2 * 2;
	AudioI2S_DeInit(I2S0_MODULE);
	AudioI2S_DeInit(I2S1_MODULE);
#if (CFG_RES_I2S_IO_PORT==0)
//i2s0  group_gpio0
	GPIO_PortAModeSet(GPIOA0, 9);// mclk out
	GPIO_PortAModeSet(GPIOA1, 6);// lrclk
	GPIO_PortAModeSet(GPIOA2, 5);// bclk
	GPIO_PortAModeSet(GPIOA3, 7);// dout
	GPIO_PortAModeSet(GPIOA4, 1);// din
//i2s0  group_gpio0
#else //lif (CFG_RES_I2S_IO_PORT==1)
//i2s1  group_gpio1 
	GPIO_PortAModeSet(GPIOA7, 5);//mclk out
	GPIO_PortAModeSet(GPIOA8, 1);//lrclk
	GPIO_PortAModeSet(GPIOA9, 2);//bclk
	GPIO_PortAModeSet(GPIOA10, 4);//do
	GPIO_PortAModeSet(GPIOA11, 2);//di
//i2s1  group_gpio1
#endif
#if (CFG_RES_I2S_PORT == 0)
	AudioI2S_Init(I2S0_MODULE, &i2s_set);
#else
	AudioI2S_Init(I2S1_MODULE, &i2s_set);
#endif
}

#endif

}


void AudioDACSinkBufClear(void)
{
	uint16_t AudioCoreSinkBufLen = mainAppCt.SamplesPreFrame * 2 * 2;
	memset(mainAppCt.Sink0Buf_DAC0, 0, AudioCoreSinkBufLen);

#ifdef CFG_RES_AUDIO_DACX_EN
	memset(mainAppCt.Sink2Buf_DACX, 0, AudioCoreSinkBufLen);
#endif
}

void EffectPcmBufMalloc(uint32_t SampleLen);
void EffectPcmBufRelease(void);
//֡��С�л�����Ҫ�ͷ�������Ƶͨ·��ص��ڴ���Դ
//AudioCore��Ҫ��ͣ
//�������������Ƶͨ·����Դ
void AudioResRelease(void)
{
	//uint32_t i;
    APP_DBG("AudioResRelease\n");
#ifdef CFG_RES_AUDIO_DAC0_EN
	AudioDAC_Disable(DAC0);
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	AudioDAC_Disable(DAC1);
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	//I2S hold
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	//I2S DMA Disable
#endif

#if CFG_RES_MIC_SELECT
	AudioADC_Disable(ADC1_MODULE);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	//I2S DMA Disable
#endif

#ifdef CFG_RES_AUDIO_DAC0_EN
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC0_TX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC0_TX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC0_TX, DMA_ERROR_INT);
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_ERROR_INT);
#endif
#ifdef CFG_RES_AUDIO_DAC0_EN
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC0_TX);
#endif
#ifdef CFG_RES_AUDIO_DACX_EN
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC1_TX);
#endif
#ifdef CFG_RES_AUDIO_DAC0_EN
	AudioDAC_Reset(DAC0);
#endif
#ifdef CFG_RES_AUDIO_DACX_EN
	AudioDAC_Reset(DAC1);
#endif

#ifdef CFG_FUNC_RECORDER_EN
	ReleaseRecorderCtSink1Buf();
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	if(mainAppCt.Sink2Buf_DACX != NULL)
	{
		APP_DBG("Sink2Buf_DACX\n");
		osPortFree(mainAppCt.Sink2Buf_DACX);
		mainAppCt.Sink2Buf_DACX = NULL;
	}
#endif
	if(mainAppCt.Sink0Buf_DAC0 != NULL)
	{
		APP_DBG("Sink0Buf_DAC0\n");
		osPortFree(mainAppCt.Sink0Buf_DAC0);
		mainAppCt.Sink0Buf_DAC0 = NULL;
	}
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	//I2S Sink2Buf_I2S
	if(mainAppCt.Sink2Buf_I2S != NULL)
	{
		APP_DBG("Sink2Buf_I2S\n");
		osPortFree(mainAppCt.Sink2Buf_I2S);
		mainAppCt.Sink2Buf_I2S = NULL;
	}
#endif
#if CFG_RES_MIC_SELECT
	if(mainAppCt.Source0Buf_ADC1 != NULL)
	{
		APP_DBG("Source0Buf_ADC1\n");
		osPortFree(mainAppCt.Source0Buf_ADC1);
		mainAppCt.Source0Buf_ADC1 = NULL;
	}
	if(mainAppCt.ADCFIFO != NULL)
	{
		APP_DBG("ADC1FIFO\n");
		osPortFree(mainAppCt.ADCFIFO);
		mainAppCt.ADCFIFO = NULL;
	}
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	if(mainAppCt.I2SFIFO != NULL)
	{
		APP_DBG("I2SFIFO\n");
		osPortFree(mainAppCt.I2SFIFO);
		mainAppCt.I2SFIFO = NULL;
	}
#endif
#ifdef CFG_RES_AUDIO_DACX_EN
	if(mainAppCt.DACXFIFO != NULL)
	{
		APP_DBG("DACXFIFO\n");
		osPortFree(mainAppCt.DACXFIFO);
		mainAppCt.DACXFIFO = NULL;
	}
#endif
#ifdef CFG_RES_AUDIO_DAC0_EN
	if(mainAppCt.DACFIFO != NULL)
	{
		APP_DBG("DACFIFO\n");
		osPortFree(mainAppCt.DACFIFO);
		mainAppCt.DACFIFO = NULL;
	}
#endif

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	EffectPcmBufRelease();
#endif
	APP_DBG("AudioResRelease ok\n");
	APP_DBG("AudioResRelease remain mem size: %d\n",(320 * 1024 - osPortRemainMem())/1024);
}

bool AudioResMalloc(uint16_t SampleLen)
{
	uint16_t FifoLenStereo = SampleLen * 2 * 2 * 2;//������8����С��֡������λbyte
	uint16_t FifoLenMono = SampleLen * 2 * 2;//������4����С��֡������λbyte
	uint16_t AudioCoreBufLen = SampleLen * 2 * 2;//AudioCore�ӿھ���������������
	APP_DBG("AudioResMalloc\n");
	APP_DBG("mainAppCt.SamplesPreFrame = %d\n", SampleLen);
	//////////����DMA fifo
	APP_DBG("Audio Res Malloc from End\n");
#ifdef CFG_RES_AUDIO_DAC0_EN
	mainAppCt.DACFIFO = (uint32_t*)osPortMallocFromEnd(FifoLenStereo);//DAC fifo
	if(mainAppCt.DACFIFO != NULL)
	{
		memset(mainAppCt.DACFIFO, 0, FifoLenStereo);
	}
	else
	{
		APP_DBG("malloc DACFIFO error\n");
		return FALSE;
	}
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	mainAppCt.DACXFIFO = (uint32_t*)osPortMallocFromEnd(FifoLenMono);//DACX fifo
	if(mainAppCt.DACXFIFO != NULL)
	{
		memset(mainAppCt.DACXFIFO, 0, FifoLenMono);
	}
	else
	{
		APP_DBG("malloc DACXFIFO error\n");
		return FALSE;
	}
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	mainAppCt.I2SFIFO = (uint32_t*)osPortMallocFromEnd(FifoLenStereo);//I2S fifo
	if(mainAppCt.I2SFIFO != NULL)
	{
		memset(mainAppCt.I2SFIFO, 0, FifoLenStereo);
	}
	else
	{
		APP_DBG("malloc I2SFIFO error\n");
		return FALSE;
	}
#endif
#if CFG_RES_MIC_SELECT	
	mainAppCt.ADCFIFO = (uint32_t*)osPortMallocFromEnd(FifoLenStereo);//ADC fifo
	if(mainAppCt.ADCFIFO != NULL)
	{
		memset(mainAppCt.ADCFIFO, 0, FifoLenStereo);
	}
	else
	{
		APP_DBG("malloc ADCFIFO error\n");
		return FALSE;
	}

///////////////////////////////AudioCore/////////////////////////////////////////
	//mic
	mainAppCt.Source0Buf_ADC1 = (uint16_t*)osPortMallocFromEnd(AudioCoreBufLen);//stereo
	if(mainAppCt.Source0Buf_ADC1 != NULL)
	{
		memset(mainAppCt.Source0Buf_ADC1, 0, AudioCoreBufLen);
	}
	else
	{
		APP_DBG("malloc Source0Buf_ADC1 error\n");
		return FALSE;
	}
#endif
	mainAppCt.Sink0Buf_DAC0 = (uint16_t*)osPortMallocFromEnd(AudioCoreBufLen);//stereo
	if(mainAppCt.Sink0Buf_DAC0 != NULL)
	{
		memset(mainAppCt.Sink0Buf_DAC0, 0, AudioCoreBufLen);
	}
	else
	{
		APP_DBG("malloc Sink0Buf_DAC0 error\n");
		return FALSE;
	}

#ifdef CFG_RES_AUDIO_DACX_EN
	mainAppCt.Sink2Buf_DACX = (uint16_t*)osPortMallocFromEnd(AudioCoreBufLen);//mono
	if(mainAppCt.Sink2Buf_DACX != NULL)
	{
		memset(mainAppCt.Sink2Buf_DACX, 0, AudioCoreBufLen);
	}
	else
	{
		APP_DBG("malloc Sink2Buf_DACX error\n");
		return FALSE;
	}
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	mainAppCt.Sink2Buf_I2S = (uint16_t*)osPortMallocFromEnd(AudioCoreBufLen);//stereo
	if(mainAppCt.Sink2Buf_I2S != NULL)
	{
		memset(mainAppCt.Sink2Buf_I2S, 0, AudioCoreBufLen);
	}
	else
	{
		APP_DBG("malloc Sink2Buf_I2S error\n");
		return FALSE;
	}
#endif
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	EffectPcmBufMalloc(SampleLen);
	EffectPcmBufClear(SampleLen);
#endif
#ifdef CFG_FUNC_RECORDER_EN
	if(!MallocRecorderCtSink1Buf(SampleLen))
		return FALSE;
#endif

	APP_DBG("\n");
	return TRUE;
}

//��Ƶͨ·��Դ��ʼ��
void AudioResInit(void)
{
	APP_DBG("AudioResInit\n");
#if CFG_RES_MIC_SELECT
	if(mainAppCt.Source0Buf_ADC1 != NULL)
	{
		mainAppCt.AudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf = (int16_t *)mainAppCt.Source0Buf_ADC1;
	}
#endif
	if(mainAppCt.Sink0Buf_DAC0 != NULL)
	{
		mainAppCt.AudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf = (int16_t*)mainAppCt.Sink0Buf_DAC0;
	}

#ifdef CFG_FUNC_RECORDER_EN
	AudioRecResInit();
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	if(mainAppCt.Sink2Buf_DACX != NULL)
	{
		mainAppCt.AudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf = (int16_t*)mainAppCt.Sink2Buf_DACX;
	}
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	if(mainAppCt.Sink2Buf_I2S != NULL)
	{
		mainAppCt.AudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf = (int16_t*)mainAppCt.Sink2Buf_I2S;
	}
#endif

#ifdef CFG_RES_AUDIO_DAC0_EN
	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC0_TX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_DAC0_TX, mainAppCt.SamplesPreFrame * 2 * 2, (void*)mainAppCt.DACFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
	DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_DAC0_TX);
	AudioDAC_Enable(DAC0);
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC1_TX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_DAC1_TX, mainAppCt.SamplesPreFrame * 2, (void*)mainAppCt.DACXFIFO, mainAppCt.SamplesPreFrame * 2 * 2);
	DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_DAC1_TX);
	AudioDAC_Enable(DAC1);
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	AudioI2sOutParamsSet();
#endif
#if CFG_RES_MIC_SELECT
	//DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
	DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC1_RX, mainAppCt.SamplesPreFrame * 2 * 2, (void*)mainAppCt.ADCFIFO, mainAppCt.SamplesPreFrame * 2 * 2 * 2);
	if(AudioADC_IsOverflow(ADC1_MODULE))
	{
		AudioADC_OverflowClear(ADC1_MODULE);
	}
	AudioADC_Clear(ADC1_MODULE);
    DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC1_RX);
    AudioADC_LREnable(ADC1_MODULE, 1, 1);
    AudioADC_Enable(ADC1_MODULE);
#endif
}

void SamplesFrameUpdataMsg(void)//����֡�仯��������Ϣ
{
	MessageContext		msgSend;
	APP_DBG("SamplesFrameUpdataMsg\n");

	msgSend.msgId		= MSG_AUDIO_CORE_FRAME_SIZE_CHANGE;
    MessageSend(mainAppCt.msgHandle, &msgSend);
}

void SamplesFrameUpdata(void)
{
	mainAppCt.SamplesPreFrame = gCtrlVars.SamplesPerFrame;
}

void EffectUpdataMsg(void)
{
	MessageContext		msgSend;
	APP_DBG("EffectUpdataMsg\n");

	msgSend.msgId		= MSG_AUDIO_CORE_EFFECT_CHANGE;
	MessageSend(mainAppCt.msgHandle, &msgSend);
}

uint16_t AudioDAC0DataSetNull(void* Buf, uint16_t Len)
{
     Buf = Buf;
     Len = Len;
     return 0;
}

uint16_t AudioDAC0DataSpaceLenGetNull(void)
{
	return 512*8;
}
