/**
 **************************************************************************************
 * @file    bt_record_mode.h
 * @brief   蓝牙K歌录音模式(使用通话链路)
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2019-8-18 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_RECORD_MODE_H__
#define __BT_RECORD_MODE_H__

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

#define BT_RECORD_SOURCE_NUM			APP_SOURCE_NUM


#define BTHF_CVSD_SAMPLE_RATE		8000
#define BTHF_MSBC_SAMPLE_RATE		16000
#define BTHF_DAC_SAMPLE_RATE		16000

#define CFG_PARA_BT_RECORD_SAMPLES_PER_FRAME		512


//resample
#define BTHF_RESAMPLE_FIFO_LEN		(120 * 2 * 2)

#define BT_CVSD_SAMPLE_SIZE			60
#define BT_CVSD_PACKET_LEN			120


#define SCO_INPUT_FIFO_SIZE			120 //cvsd=120, msbc=60
#define SCO_OUTPUT_FIFO_SIZE		240

#define MSBC_DECODER_INPUT_SIZE		2*1024

#define BT_RECORD_MSBC_LEVEL_START	(57*20)//(57*3)//15ms
#define BT_MSBC_PACKET_LEN			60
#define BT_MSBC_MIC_INPUT_SAMPLE	120 //stereo:480bytes -> mono:240bytes

#define BT_RECORD_SCO_FIFO_LEN		(CFG_PARA_BT_RECORD_SAMPLES_PER_FRAME * 2 * 2 * 2) //nomo
#define BT_RESAMPLE_16K_FIFO_LEN	(1024)
#define BT_ENCODER_FIFO_LEN			(BT_MSBC_PACKET_LEN * 2 * 6) //nomo // 60 * 12 packet

typedef enum 
{
	BT_RECORD_CALLING_NONE = 0,
	BT_RECORD_CALLING_ACTIVE,
	BT_RECORD_CALLING_SUSPEND,
} BT_RECORD_CALLING_STATE;

//FIFO统计说明
//

typedef struct _BtHfRecordContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	uint8_t				codecType;

	TaskState			state;
	BT_RECORD_CALLING_STATE callingState;

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
	uint8_t*			ScoInputFifo;//接收到的数据缓存 CVSD or MSBC         //size:120B
	uint16_t			ScoInputLen;
	int16_t*			ScoOutputFifo;//将要发送的数据缓存 CVSD or MSBC
	uint16_t			ScoOutputLen;

//CVSD
	//ResamplerContext*	ScoInResamplerCt;//svcd 8K->16K
	ResamplerPolyphaseContext*	ScoInResamplerCt;
	int16_t*			ScoInSrcOutBuf;//转采样后输出BUF(接收到的cvsd数据) //size:120*2*2
	
	//ResamplerContext*	ScoOutResamplerCt;//output 16k->8k
	ResamplerPolyphaseContext*	ScoOutResamplerCt;
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
	TaskState			EncoderSync;

#endif

	uint32_t			BtHfModeRestart;

	AudioCoreSink 		*AudioCoreSinkRecordSco;

	//sink2 pcm out buf
	int16_t*			Sink2PcmOutBuf;
	
	//转采样前端FIFO
	MCU_CIRCULAR_CONTEXT	ResamplerPreFifoCircular;
	osMutexId			ResamplerPreFifoMutex;
	int16_t*			ResamplerPreFifo;
	
#ifdef BT_HFP_CALL_DURATION_DISP
	uint32_t			BtRecordTimerMsCount;
	uint32_t			BtRecordActiveTime;
	bool				BtRecordTimeUpdate;
#endif

	uint8_t				ModeKillFlag;
	uint8_t				CvsdInitFlag;
	uint8_t				MsbcInitFlag;

	//sink2 encoder后数据缓存区
	MCU_CIRCULAR_CONTEXT BtEncoderSendCircularBuf;
	uint8_t				*BtEncoderSendFifo;

	//转采样 44.1k -> 16k
	ResamplerPolyphaseContext*	AudioSink2OutResamplerCt;//output 44.1k->16k
	int16_t*			AudioSink2OutBuf;//转采样后输出BUF(需要发送的msbc数据) //size:120*2*2
	int16_t*			ResampleOutFifo;//16k转采样后的数据缓存,等待编码发送
	MCU_CIRCULAR_CONTEXT	ResampleOutFifoCircular;
	

}BtRecordContext;
extern BtRecordContext	*BtRecordCt;

MessageHandle GetBtRecordMessageHandle(void);

bool BtRecordCreate(MessageHandle parentMsgHandle);

void BtRecordTaskPause(void);

void BtRecordTaskResume(void);

bool BtRecordKill(void);

bool BtRecordStart(void);

bool BtRecordStop(void);

uint8_t BtRecordDecoderSourceNum(void);

void EnterBtRecordMode(void);

void ExitBtRecordMode(void);

void BtRecord_Timer1msCheck(void);

void BtRecordModeDeregister(void);



#endif /*__BT_RECORD_MODE_H__*/



