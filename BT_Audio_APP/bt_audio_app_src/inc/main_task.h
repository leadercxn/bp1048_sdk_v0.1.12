/**
 **************************************************************************************
 * @file    main_task.h
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
#ifndef __MAIN_TASK_H__
#define __MAIN_TASK_H__


#include "type.h"
#include "rtos_api.h"
#include "app_config.h"
#include "mode_switch_api.h"
#include "audio_core_api.h"
#include "app_message.h"
#include "timeout.h"
#ifdef CFG_APP_USB_AUDIO_MODE_EN //此设备 detect需2S
#define	MODE_WAIT_DEVICE_TIME			1900
#else
#define	MODE_WAIT_DEVICE_TIME			800
#endif

typedef enum
{
	ModeStateNone	= 0,//首次启动前 状态。
	ModeStateEnter,
	ModeStateWork,
	ModeStateExit,
	ModeStatePause,//暂停态，应用：1、模式进入idle，2、deepsleep 退出到此状态，醒来再进。
}ModeState;

typedef struct _SysVolContext
{
	bool		MuteFlag;	//AudioCore软件mute
	int8_t	 	AudioSourceVol[AUDIO_CORE_SOURCE_MAX_MUN];	//Source增益控制step，小于等于32	
	int8_t	 	AudioSinkVol[AUDIO_CORE_SINK_MAX_NUM];		//Sink增益控制step，小于等于32	
	
}SysVolContext;

typedef struct _MainAppContext
{
	xTaskHandle			taskHandle;
	MessageHandle		msgHandle;
	TaskState			state;
	ModeState			MState;
	AppMode				appCurrentMode;
	AppMode				appTargetMode;//for scan /detect
	AppMode				appBackupMode;//for App backup

	uint16_t			SamplesPreFrame;
	
#ifdef CFG_RES_AUDIO_DAC0_EN
	uint32_t			*DACFIFO;
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	uint32_t			*DACXFIFO;
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	uint32_t			*I2SFIFO;
#endif

	uint32_t			*ADCFIFO;
	AudioCoreContext 	*AudioCore;
	uint16_t			SourcesMuteState;//纪录source源mute使能情况,目前只判断left
	
	uint16_t 			*Source0Buf_ADC1;//ADC1 取mic数据

	uint16_t 			*Sink0Buf_DAC0;

#ifdef CFG_RES_AUDIO_DACX_EN
	uint16_t			*Sink2Buf_DACX;
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	uint16_t			*Sink2Buf_I2S;
#endif

	uint32_t 			SampleRate;
	bool				DeviceSync;
	bool				AudioCoreSync;
#ifdef CFG_FUNC_DISPLAY_EN
	bool				DisplaySync;
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
	bool				RemindSoundSync;
	char *				SysRemind;
#endif
#ifdef CFG_FUNC_ALARM_EN
	uint32_t 			AlarmID;//闹钟ID对应bit位
	bool				AlarmFlag;
	bool				AlarmRemindStart;//闹铃提示音开启标志
	uint32_t 			AlarmRemindCnt;//闹铃提示音循环次数
	#ifdef CFG_FUNC_SNOOZE_EN
	bool				SnoozeOn;
	uint32_t 			SnoozeCnt;// 贪睡时间计数
	#endif
#endif
	SysVolContext		gSysVol;
    uint8_t     MusicVolume;
#ifdef CFG_APP_BT_MODE_EN
    uint8_t     HfVolume;
#endif
	uint8_t     EffectMode;
    uint8_t     MicVolume;
	uint8_t     MicVolumeBak;
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	uint8_t     EqMode;
	#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN   
    uint8_t     EqModeBak;
	uint8_t     EqModeFadeIn;
	uint8_t     eqSwitchFlag;
	#endif
#endif
    uint8_t  	ReverbStep;	
    uint8_t  	ReverbStepBak;
#ifdef CFG_FUNC_MIC_TREB_BASS_EN	
	uint8_t 	MicBassStep;
    uint8_t 	MicTrebStep;
	uint8_t 	MicBassStepBak;
    uint8_t 	MicTrebStepBak;
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN	
	uint8_t 	MusicBassStep;
    uint8_t 	MusicTrebStep;
#endif
#ifdef  CFG_APP_HDMIIN_MODE_EN
	uint8_t  	hdmiArcOnFlg;
	uint8_t     hdmiSourceMuteFlg;
	uint8_t     hdmiResetFlg;
#endif
}MainAppContext;

extern MainAppContext	mainAppCt;

#define SoftFlagNoRemind		BIT(0)	//提示音故障
#define	SoftFlagDecoderRun		BIT(1)	
#define SoftFlagDecoderSwitch	BIT(2) //解码器预备切出，目的是过滤消息
#define SoftFlagDecoderApp		BIT(3) //App复用解码器
#define SoftFlagDecoderRemind	BIT(4)	//提示音占用解码器
#define SoftFlagDecoderMask		(SoftFlagDecoderRemind | SoftFlagDecoderApp | SoftFlagDecoderSwitch)//解码器掩码
#define SoftFlagRecording		BIT(5)	//录音进行标记，禁止后插先播，模式切换需清理
#define SoftFlagPlayback		BIT(6)	//回放进行标记，禁止后插先播，模式切换需清理

#if FLASH_BOOT_EN 
#define SoftFlagMvaInCard		BIT(7)	//文件预搜索发现SD卡有MVA包 卡拔除时清理
#define SoftFlagMvaInUDisk		BIT(8)	//文件预搜索发现U盘有Mva包 U盘拔除时清理
#endif

#define SoftFlagMediaToRecMode	BIT(9)//0:no 1:doing

#define SoftFlagDeepSleepRequest	BIT(10)

//标记 屏蔽提示音阶段	典型应用于 接打电话(模式)后，退回原先模式，不希望播放模式提示音。
#define SoftFlagRemindMask		BIT(11)

#define SoftFlagAiProcess		BIT(12)//ai处理可实施
#define SoftFlagMediaRectate	BIT(13)//0: not rec 1:rec
#define SoftFlagDecRecFile		BIT(14)//0: not del file 1:del file
#define SoftFlagMediaPlayRecRemind BIT(15)//0 not play 1:playing

//通话模式,蓝牙断开连接,延时播放提示音,即退回到每个模式时播放
#define SoftFlagDiscDelayMask	BIT(16)
//蓝牙播放模式下播放歌曲,通话呼入呼出后,回到蓝牙播放模式,需要继续播放歌曲
#define SoftFlagResPlayStateMask	BIT(17)

//标记上电第一次选择模式前，由于Waitingplay的存在，需标记第一个模式实施状态
#define SoftFlagWaitDetect		BIT(18)		//FirstMode		BIT(17)//SoftFlagWaitDevice
#define SoftFlagWaitSysRemind	BIT(19)		//for Play sys remind:deepsleep/wakeup/PowerOn。。。
#define SoftFlagWaitModeMask	(SoftFlagWaitDetect | SoftFlagWaitSysRemind )//登记有效时，进入Waiting模式，不响应退出。

//标记本次唤醒源是否为CEC唤醒
#define SoftFlagWakeUpSouceIsCEC BIT(20)

//标记本次deepsleep消息是否来自于TV
#define SoftFlagDeepSleepMsgIsFromTV BIT(21)

//标记来电时等待提示音播放完成再进入通话状态
#define SoftFlagWaitBtRemindEnd		 BIT(22)
//标记延时进入通话状态
#define SoftFlagDelayEnterBtHf		 BIT(23)
#ifdef CFG_FUNC_SOFT_ADJUST_IN
#define SoftFlagBtSra				BIT(24)
#endif
#define SoftFlagFrameSizeChange		BIT(25)

//标记蓝牙通话状态下延迟进入睡眠模式
#define SoftFlagBtHfDelayEnterSleep	BIT(26)

//标记来电时记录当前蓝牙播放的状态
#define SoftFlagBtCurPlayStateMask	BIT(27)

#define SoftFlagMask			0xFFFFFFFF
void SoftFlagRegister(uint32_t SoftEvent);
void SoftFlagDeregister(uint32_t SoftEvent);
bool SoftFlagGet(uint32_t SoftEvent);

#define MEDIA_VOLUME_STR_C		("0:/")
#define MEDIA_VOLUME_STR_U		("1:/")
#define MEDIA_VOLUME_C			0
#define MEDIA_VOLUME_U			1

#ifdef CFG_COMMUNICATION_BY_UART
extern uint8_t UartRxBuf[1024];
extern uint8_t UartTxBuf[1024];
#endif
#ifdef CFG_RES_IR_NUMBERKEY
extern bool Number_select_flag;
extern uint16_t Number_value;
extern TIMER Number_selectTimer;
#endif
/**
 * @brief	Start a main program task.
 * @param	NONE
 * @return	
 */
int32_t MainAppTaskStart(void);


/**
 * @brief	Get message receive handle of main app
 * @param	NONE
 * @return	MessageHandle
 */
MessageHandle GetMainMessageHandle(void);

uint32_t GetSystemMode(void);

//void AudioDACInit(uint32_t sampleRate);

/**
 * @brief	clear audio core sink buffer(DAC)
 * @param	NONE
 * @return	NONE
 */
void AudioDACSinkBufClear(void);

void AudioADC1ParamsSet(uint32_t sampleRate, uint16_t gain, uint8_t gainBoostSel);

void ResumeAudioCoreMicSource(void);

#endif /*__MAIN_TASK_H__*/
