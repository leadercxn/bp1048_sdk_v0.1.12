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
#ifdef CFG_APP_USB_AUDIO_MODE_EN //���豸 detect��2S
#define	MODE_WAIT_DEVICE_TIME			1900
#else
#define	MODE_WAIT_DEVICE_TIME			800
#endif

typedef enum
{
	ModeStateNone	= 0,//�״�����ǰ ״̬��
	ModeStateEnter,
	ModeStateWork,
	ModeStateExit,
	ModeStatePause,//��̬ͣ��Ӧ�ã�1��ģʽ����idle��2��deepsleep �˳�����״̬�������ٽ���
}ModeState;

typedef struct _SysVolContext
{
	bool		MuteFlag;	//AudioCore���mute
	int8_t	 	AudioSourceVol[AUDIO_CORE_SOURCE_MAX_MUN];	//Source�������step��С�ڵ���32	
	int8_t	 	AudioSinkVol[AUDIO_CORE_SINK_MAX_NUM];		//Sink�������step��С�ڵ���32	
	
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
	uint16_t			SourcesMuteState;//��¼sourceԴmuteʹ�����,Ŀǰֻ�ж�left
	
	uint16_t 			*Source0Buf_ADC1;//ADC1 ȡmic����

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
	uint32_t 			AlarmID;//����ID��Ӧbitλ
	bool				AlarmFlag;
	bool				AlarmRemindStart;//������ʾ��������־
	uint32_t 			AlarmRemindCnt;//������ʾ��ѭ������
	#ifdef CFG_FUNC_SNOOZE_EN
	bool				SnoozeOn;
	uint32_t 			SnoozeCnt;// ̰˯ʱ�����
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

#define SoftFlagNoRemind		BIT(0)	//��ʾ������
#define	SoftFlagDecoderRun		BIT(1)	
#define SoftFlagDecoderSwitch	BIT(2) //������Ԥ���г���Ŀ���ǹ�����Ϣ
#define SoftFlagDecoderApp		BIT(3) //App���ý�����
#define SoftFlagDecoderRemind	BIT(4)	//��ʾ��ռ�ý�����
#define SoftFlagDecoderMask		(SoftFlagDecoderRemind | SoftFlagDecoderApp | SoftFlagDecoderSwitch)//����������
#define SoftFlagRecording		BIT(5)	//¼�����б�ǣ���ֹ����Ȳ���ģʽ�л�������
#define SoftFlagPlayback		BIT(6)	//�طŽ��б�ǣ���ֹ����Ȳ���ģʽ�л�������

#if FLASH_BOOT_EN 
#define SoftFlagMvaInCard		BIT(7)	//�ļ�Ԥ��������SD����MVA�� ���γ�ʱ����
#define SoftFlagMvaInUDisk		BIT(8)	//�ļ�Ԥ��������U����Mva�� U�̰γ�ʱ����
#endif

#define SoftFlagMediaToRecMode	BIT(9)//0:no 1:doing

#define SoftFlagDeepSleepRequest	BIT(10)

//��� ������ʾ���׶�	����Ӧ���� �Ӵ�绰(ģʽ)���˻�ԭ��ģʽ����ϣ������ģʽ��ʾ����
#define SoftFlagRemindMask		BIT(11)

#define SoftFlagAiProcess		BIT(12)//ai�����ʵʩ
#define SoftFlagMediaRectate	BIT(13)//0: not rec 1:rec
#define SoftFlagDecRecFile		BIT(14)//0: not del file 1:del file
#define SoftFlagMediaPlayRecRemind BIT(15)//0 not play 1:playing

//ͨ��ģʽ,�����Ͽ�����,��ʱ������ʾ��,���˻ص�ÿ��ģʽʱ����
#define SoftFlagDiscDelayMask	BIT(16)
//��������ģʽ�²��Ÿ���,ͨ�����������,�ص���������ģʽ,��Ҫ�������Ÿ���
#define SoftFlagResPlayStateMask	BIT(17)

//����ϵ��һ��ѡ��ģʽǰ������Waitingplay�Ĵ��ڣ����ǵ�һ��ģʽʵʩ״̬
#define SoftFlagWaitDetect		BIT(18)		//FirstMode		BIT(17)//SoftFlagWaitDevice
#define SoftFlagWaitSysRemind	BIT(19)		//for Play sys remind:deepsleep/wakeup/PowerOn������
#define SoftFlagWaitModeMask	(SoftFlagWaitDetect | SoftFlagWaitSysRemind )//�Ǽ���Чʱ������Waitingģʽ������Ӧ�˳���

//��Ǳ��λ���Դ�Ƿ�ΪCEC����
#define SoftFlagWakeUpSouceIsCEC BIT(20)

//��Ǳ���deepsleep��Ϣ�Ƿ�������TV
#define SoftFlagDeepSleepMsgIsFromTV BIT(21)

//�������ʱ�ȴ���ʾ����������ٽ���ͨ��״̬
#define SoftFlagWaitBtRemindEnd		 BIT(22)
//�����ʱ����ͨ��״̬
#define SoftFlagDelayEnterBtHf		 BIT(23)
#ifdef CFG_FUNC_SOFT_ADJUST_IN
#define SoftFlagBtSra				BIT(24)
#endif
#define SoftFlagFrameSizeChange		BIT(25)

//�������ͨ��״̬���ӳٽ���˯��ģʽ
#define SoftFlagBtHfDelayEnterSleep	BIT(26)

//�������ʱ��¼��ǰ�������ŵ�״̬
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
