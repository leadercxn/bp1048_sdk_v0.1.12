/**
 **************************************************************************************
 * @file    bt_hf_mode.h
 * @brief	蓝牙通话模式
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2018-7-17 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_HF_MODE_H__
#define __BT_HF_MODE_H__

#include "type.h"
#include "rtos_api.h"

#include "audio_vol.h"
#include "rtos_api.h"
#include "resampler_polyphase.h"
#include "mcu_circular_buf.h"
#include "audio_core_api.h"
#include "audio_decoder_api.h"
#include "sbcenc_api.h"
#include "bt_config.h"
#include "cvsd_plc.h"
#include "ctrlvars.h"
#include "blue_ns_core.h"

#define BT_HF_SOURCE_NUM			APP_SOURCE_NUM


#define BTHF_CVSD_SAMPLE_RATE		8000
#define BTHF_MSBC_SAMPLE_RATE		16000
#define BTHF_DAC_SAMPLE_RATE		16000

//resample
#define BTHF_RESAMPLE_FIFO_LEN		(120 * 2 * 2)//(CFG_BTHF_PARA_SAMPLES_PER_FRAME * 2 * 2 * 2)

#define BT_SCO_FIFO_LEN				(CFG_BTHF_PARA_SAMPLES_PER_FRAME * 2 * 2 * 2) //nomo
#define BT_SCO_SEND_BUFFER_HIGH_LEVEL	60*25
#define BT_SCO_SEND_BUFFER_LOW_LEVEL	60*4


//#define	ADC_FIFO_LEN				(CFG_BTHF_PARA_SAMPLES_PER_FRAME * 2 * 2 * 2) //stereo 256*2=512sample

#define SCO_INPUT_FIFO_SIZE			120 //cvsd=120, msbc=60
#define SCO_OUTPUT_FIFO_SIZE		240

#define BT_CVSD_SAMPLE_SIZE			60
#define BT_CVSD_PACKET_LEN			120

#define MSBC_DECODER_INPUT_SIZE		2*1024

#define BT_MSBC_LEVEL_START			(57*20)
#define BT_MSBC_PACKET_LEN			60
#define BT_MSBC_MIC_INPUT_SAMPLE	120 //stereo:480bytes -> mono:240bytes

#define BT_MSBC_ENCODER_FIFO_LEN	(CFG_BTHF_PARA_SAMPLES_PER_FRAME * 2 * 2 * 2)

//sbc receive fifo
#define BT_SBC_RECV_FIFO_SIZE		(60*6)


//AEC
#define FRAME_SIZE					BLK_LEN
#define AEC_SAMPLE_RATE				16000
#define LEN_PER_SAMPLE				2 //mono
#define DELAY_LEN_PER_BLOCK				(64)
#define MAX_DELAY_BLOCK				BT_HFP_AEC_MAX_DELAY_BLK
#define DEFAULT_DELAY_BLK			BT_HFP_AEC_DELAY_BLK

//PITCH SHIFTER
#define MAX_PITCH_SHIFTER_STEP		25

typedef enum 
{
	BT_HF_CALLING_NONE = 0,
	BT_HF_CALLING_ACTIVE,
	BT_HF_CALLING_SUSPEND,
} BT_HFP_CALLING_STATE;

typedef struct _btHfContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	uint8_t				codecType;

	TaskState			state;
	BT_HFP_CALLING_STATE callingState;

	QueueHandle_t 		audioMutex;
	QueueHandle_t		pcmBufMutex;

	uint8_t 			DecoderSourceNum;

	uint8_t				hfModeAudioResInitOk;
	
	uint32_t 			SampleRate;
	uint8_t				BtSyncVolume;

	//adc fifo
	uint32_t			*ADCFIFO;	//adc input fifo (stereo) //

	//sco fifo 
	MCU_CIRCULAR_CONTEXT BtScoCircularBuf;
	uint8_t*			ScoInputFifo;//接收到的数据缓存 CVSD or MSBC         //cvsd=120, msbc=60
	uint16_t			ScoInputLen;
	int16_t*			ScoOutputFifo;//将要发送的数据缓存 CVSD or MSBC
	uint16_t			ScoOutputLen;

//CVSD
	//ResamplerContext*	ScoInResamplerCt;//svcd 8K->16K
	ResamplerPolyphaseContext*  ScoInResamplerCt;
	int16_t*			ScoInSrcOutBuf;//转采样后输出BUF(接收到的cvsd数据) //size:120*2*2
	
	//ResamplerContext*	ScoOutResamplerCt;//output 16k->8k
	ResamplerPolyphaseContext*  ScoOutResamplerCt;
	int16_t*			ScoOutSrcOutBuf;//转采样后输出BUF(需要发送的cvsd数据) //size:120*2*2
	
	//cvsd plc
	CVSD_PLC_State		*cvsdPlcState;
	
	uint16_t 			*Source1Buf_ScoData;

	AudioCoreContext 	*AudioCoreBtHf;

	
	int8_t*				ScoBuffer;
	uint8_t*			ScoEncoderBuffer;

	//sco send to phone buffer 120bytes
	uint8_t				scoSendBuf[BT_CVSD_PACKET_LEN];
	
#if	defined(CFG_FUNC_REMIND_SOUND_EN)
	uint16_t*			SourceDecoder;
	uint8_t				PhoneNumber[20];
	uint8_t				PhoneNumberLen;
#ifdef CFG_BT_NUMBER_REMIND
	uint8_t 			PhoneNumberRemindStart; // 1=开始播放提示音
	uint8_t 			RemindCnt;
	uint8_t				PhoneNumberPos;
#endif
#endif

	//MSBC
	//used Service
	TaskState			DecoderSync;
	
	//msbc decoder
	uint8_t				msbcDecoderStarted;
	uint8_t				msbcDecoderInitFlag;
	MemHandle 			msbcMemHandle;
	uint8_t				*msbcRingBuf;

	//msbc encoder
	uint8_t				mSbcEncoderStart;
	uint8_t				mSbcEncoderSendStart;
	uint8_t				msbcSyncCount;
	uint8_t				msbcEncoderFifo[BT_MSBC_PACKET_LEN];
	uint32_t			msbc_encoded_data_length;
	uint32_t			test_msbc_encoded_data_length;
	
	uint8_t				btHfScoSendStart;//正式处理需要发送的数据
	uint8_t				btHfScoSendReady;//可以开始发送数据了,初始化发送的相关参数
	
	SBCEncoderContext	*sbc_encode_ct;

#ifdef CFG_FUNC_RECORDER_EN
	TaskState			RecorderSync;
#endif

	uint32_t			BtHfModeRestart;

	AudioCoreSink 		*AudioCoreSinkHfSco;
	
	MCU_CIRCULAR_CONTEXT	Sink2ScoFifoCircular;
	osMutexId			Sink2ScoFifoMutex;
	int16_t*			Sink2ScoFifo;
	int16_t*			Sink2PcmFifo;
	
#ifdef BT_HFP_CALL_DURATION_DISP
	uint32_t			BtHfTimerMsCount;
	uint32_t			BtHfActiveTime;
	bool				BtHfTimeUpdate;
#endif

	uint8_t				ModeKillFlag;
	uint8_t				CvsdInitFlag;
	uint8_t				MsbcInitFlag;
	
	MCU_CIRCULAR_CONTEXT BtScoSendCircularBuf;
	uint8_t				*BtScoSendFifo;

	//aec
	uint8_t				*AecDelayBuf;		//BLK_LEN*LEN_PER_SAMPLE*MAX_DELAY_BLOCK
	MemHandle			AecDelayRingBuf;
	uint16_t			*SourceBuf_Aec;

#if ((CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN)&&defined(BT_HFP_MIC_PITCH_SHIFTER_FUNC))
	//pitch shifter
	uint8_t				PitchShifterStep;
#endif
	uint8_t				btHfResumeCnt;
	uint8_t				btPhoneCallRing; //0=本地铃声  1=手机铃声

	//SBC receive buffer
	MCU_CIRCULAR_CONTEXT	msbcRecvFifoCircular;
	uint8_t*				msbcRecvFifo;
	uint8_t					msbcPlcCnt;

	//specific device(如WIN7 PC用于处理HFP数据-cvsd)
	uint8_t				scoSpecificFifo[3][120];
	uint32_t			scoSpecificIndex;
	uint32_t			scoSpecificCount;
	
	uint32_t			flagDelayExitBtHf;	//在语音标志关闭退出时，延时1s退出bt hf模式

	uint32_t 			SystemEffectMode;//用于标记system当前的音效模式，HFP音效单独

}BtHfContext;
extern BtHfContext	*BtHfCt;

MessageHandle GetBtHfMessageHandle(void);

bool BtHfCreate(MessageHandle parentMsgHandle);

void BtHfTaskPause(void);

void BtHfTaskResume(void);

bool BtHfKill(void);

bool BtHfStart(void);

bool BtHfStop(void);

uint8_t BtHfDecoderSourceNum(void);

void EnterBtHfMode(void);

void ExitBtHfMode(void);

void BtHf_Timer1msCheck(void);

void BtHfRemindNumberStop(void);

void BtHfCodecTypeUpdate(uint8_t codecType);

void BtHfModeRunningResume(void);

void DelayExitBtHfModeSet(void);

void DelayExitBtHfModeCancel(void);

#endif /*__BT_HF_MODE_H__*/



