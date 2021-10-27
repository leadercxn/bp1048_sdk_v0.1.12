/**
 **************************************************************************************
 * @file    bt_hf_api.c
 * @brief   蓝牙通话模式
 *
 * @author  KK
 * @version V1.0.1
 *
 * $Created: 2019-3-28 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "type.h"
#include "app_config.h"
#include "app_message.h"
#include "mvintrinsics.h"
//driver
#include "chip_info.h"
#include "dac.h"
#include "gpio.h"
#include "dma.h"
#include "dac.h"
#include "audio_adc.h"
#include "debug.h"
//middleware
#include "main_task.h"
#include "audio_vol.h"
#include "rtos_api.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "bt_manager.h"
#include "resampler.h"
#include "mcu_circular_buf.h"
#include "audio_core_api.h"
#include "audio_decoder_api.h"
#include "sbcenc_api.h"
#include "bt_config.h"
#include "cvsd_plc.h"
#include "ctrlvars.h"
//application
#include "bt_hf_mode.h"
#include "decoder_service.h"
#include "audio_core_service.h"
#include "mode_switch_api.h"
#include "audio_core_api.h"
#include "bt_hf_api.h"

#ifdef CFG_FUNC_REMIND_SOUND_EN
#include "remind_sound_service.h"
#endif
#include "powercontroller.h"

extern void ResumeAudioCoreMicSource(void);

#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE))

extern uint8_t lc_sco_data_error_flag; //0=CORRECTLY_RX_FLAG; 1=POSSIBLY_INVALID_FLAG; 2=NO_RX_DATA_FLAG; 3=PARTIALLY_LOST_FLAG;

//msbc encoder
#define MSBC_CHANNE_MODE	1 		// mono
#define MSBC_SAMPLE_REATE	16000	// 16kHz
#define MSBC_BLOCK_LENGTH	15

// sco sync header H2
uint8_t sco_sync_header[4][2] = 
{
	{0x01, 0x08}, 
	{0x01, 0x38}, 
	{0x01, 0xc8}, 
	{0x01, 0xf8}
};

extern uint32_t hfModeSuspend;
extern uint32_t gSpecificDevice;
/*******************************************************************************
 * AudioCore Sink Channel2 :输出为需要发送的HF数据
 * unit: sample
 ******************************************************************************/
uint16_t BtHf_SinkScoDataSet(void* InBuf, uint16_t InLen)
{
	if(InLen == 0)
		return 0;

	osMutexLock(BtHfCt->Sink2ScoFifoMutex);
	MCUCircular_PutData(&BtHfCt->Sink2ScoFifoCircular, InBuf, InLen * 2);
	osMutexUnlock(BtHfCt->Sink2ScoFifoMutex);

	return InLen;
}

void BtHf_SinkScoDataGet(void* OutBuf, uint16_t OutLen)
{
	if(OutLen == 0)
		return;

	osMutexLock(BtHfCt->Sink2ScoFifoMutex);
	MCUCircular_GetData(&BtHfCt->Sink2ScoFifoCircular, OutBuf, OutLen*2);
	osMutexUnlock(BtHfCt->Sink2ScoFifoMutex);
}


uint16_t BtHf_SinkScoDataSpaceLenGet(void)
{
	return MCUCircular_GetSpaceLen(&BtHfCt->Sink2ScoFifoCircular) / 2;
}

uint16_t BtHf_SinkScoDataLenGet(void)
{
	return MCUCircular_GetDataLen(&BtHfCt->Sink2ScoFifoCircular) / 2;
}

/*******************************************************************************
 * MIC处理后，缓存到待发送缓存FIFO
 * unit: Bytes
 ******************************************************************************/
uint16_t BtHf_SendScoBufSet(void* InBuf, uint16_t InLen)
{
	if(InLen == 0)
		return 0;

	MCUCircular_PutData(&BtHfCt->BtScoSendCircularBuf, InBuf, InLen);
	return InLen;
}

void BtHf_SendScoBufGet(void* OutBuf, uint16_t OutLen)
{
	if(OutLen == 0)
		return;

	MCUCircular_GetData(&BtHfCt->BtScoSendCircularBuf, OutBuf, OutLen);
}


uint16_t BtHf_SendScoBufSpaceLenGet(void)
{
	return MCUCircular_GetSpaceLen(&BtHfCt->BtScoSendCircularBuf);
}

uint16_t BtHf_SendScoBufLenGet(void)
{
	return MCUCircular_GetDataLen(&BtHfCt->BtScoSendCircularBuf);
}

/***********************************************************************************
 * msbc receive fifo
 **********************************************************************************/
uint16_t BtHf_SbcRecvBufSet(void* InBuf, uint16_t InLen)
{
	if(InLen == 0)
		return 0;

	MCUCircular_PutData(&BtHfCt->msbcRecvFifoCircular, InBuf, InLen);
	return InLen;
}

void BtHf_SbcRecvBufGet(void* OutBuf, uint16_t OutLen)
{
	if(OutLen == 0)
		return;

	MCUCircular_GetData(&BtHfCt->msbcRecvFifoCircular, OutBuf, OutLen);
}


uint16_t BtHf_SbcRecvBufSpaceLenGet(void)
{
	return MCUCircular_GetSpaceLen(&BtHfCt->msbcRecvFifoCircular);
}

uint16_t BtHf_SbcRecvBufLenGet(void)
{
	return MCUCircular_GetDataLen(&BtHfCt->msbcRecvFifoCircular);
}



/***********************************************************************************
 * msbc Encoder
 **********************************************************************************/
void BtHf_MsbcEncoderInit(void)
{
	//encoder init
	int32_t samplesPerFrame;
	int32_t ret;

	BtHfCt->sbc_encode_ct = (SBCEncoderContext*)osPortMalloc(sizeof(SBCEncoderContext));
	if(BtHfCt->sbc_encode_ct == NULL)
	{
		APP_DBG("sbc encode init malloc error\n");
	}
	memset(BtHfCt->sbc_encode_ct, 0, sizeof(SBCEncoderContext));

	//if(BtHfCt->codecType == HFP_AUDIO_DATA_PCM)
	//	return;
	
	ret = sbc_encoder_initialize(BtHfCt->sbc_encode_ct, MSBC_CHANNE_MODE, MSBC_SAMPLE_REATE, MSBC_BLOCK_LENGTH, SBC_ENC_QUALITY_MIDDLE, &samplesPerFrame);
	APP_DBG("encoder sample:%ld\n", samplesPerFrame);
	if(ret != SBC_ENC_ERROR_OK)
	{
		APP_DBG("sbc encode init error\n");
		return;
	}
	BtHfCt->mSbcEncoderStart = 1;
}

void BtHf_MsbcEncoderDeinit(void)
{
	if(BtHfCt->sbc_encode_ct)
	{
		osPortFree(BtHfCt->sbc_encode_ct);
		BtHfCt->sbc_encode_ct = NULL;
	}
	
	BtHfCt->mSbcEncoderStart = 0;
}

/*******************************************************************************
 * MSBC Decoder
 ******************************************************************************/
void BtHf_MsbcMemoryReset(void)
{
	if(BtHfCt->msbcRingBuf == NULL)
		return;
	memset(BtHfCt->msbcRingBuf, 0, MSBC_DECODER_INPUT_SIZE);
	
	BtHfCt->msbcMemHandle.addr = BtHfCt->msbcRingBuf;
	BtHfCt->msbcMemHandle.mem_capacity = MSBC_DECODER_INPUT_SIZE;
	BtHfCt->msbcMemHandle.mem_len = 0;
	BtHfCt->msbcMemHandle.p = 0;
}

int32_t BtHf_MsbcDecoderInit(void)
{
	BtHfCt->msbcRingBuf = osPortMalloc(MSBC_DECODER_INPUT_SIZE);
	if(BtHfCt->msbcRingBuf == NULL)
		return -1;
	memset(BtHfCt->msbcRingBuf, 0, MSBC_DECODER_INPUT_SIZE);
	
	BtHfCt->msbcMemHandle.addr = BtHfCt->msbcRingBuf;
	BtHfCt->msbcMemHandle.mem_capacity = MSBC_DECODER_INPUT_SIZE;
	BtHfCt->msbcMemHandle.mem_len = 0;
	BtHfCt->msbcMemHandle.p = 0;
	
	BtHfCt->msbcDecoderInitFlag = TRUE;
	BtHf_MsbcDecoderStartedSet(FALSE);

	return 0;
}

int32_t BtHf_MsbcDecoderDeinit(void)
{
	BtHfCt->msbcMemHandle.addr = NULL;
	BtHfCt->msbcMemHandle.mem_capacity = 0;
	BtHfCt->msbcMemHandle.mem_len = 0;
	BtHfCt->msbcMemHandle.p = 0;
	
	BtHfCt->msbcDecoderInitFlag = FALSE;
	BtHf_MsbcDecoderStartedSet(FALSE);

	if(BtHfCt->msbcRingBuf)
	{
		osPortFree(BtHfCt->msbcRingBuf);
		BtHfCt->msbcRingBuf = NULL;
	}
	return 0;
}

void BtHf_MsbcDecoderStartedSet(bool flag)
{
	BtHfCt->msbcDecoderStarted = flag;
}

bool BtHf_MsbcDecoderStartedGet(void)
{
	return BtHfCt->msbcDecoderStarted;
}

bool BtHf_MsbcDecoderIsInitialized(void)
{
	return BtHfCt->msbcDecoderInitFlag;
}

static MemHandle * BtHf_MsbcDecoderMemHandleGet(void)
{
	if(BtHf_MsbcDecoderIsInitialized())
	{
		return &BtHfCt->msbcMemHandle;
	}
	return NULL;
}

uint32_t BtHf_MsbcDataLenGet(void)
{
	uint32_t	dataSize = 0;
	if(BtHfCt->msbcDecoderInitFlag)
	{
		dataSize = mv_msize(&BtHfCt->msbcMemHandle);
	}
	return dataSize;
}

int32_t BtHf_MsbcDecoderStart(void)
{
	int32_t 		ret = 0;
	
	ret = DecoderInit(BtHf_MsbcDecoderMemHandleGet(), (int32_t)IO_TYPE_MEMORY, MSBC_DECODER);
	if(ret != RT_SUCCESS)
	{
		APP_DBG("audio_decoder_initialize error code:%ld!\n", audio_decoder_get_error_code());
		BtHf_MsbcDecoderStartedSet(FALSE);
		return -1;
	}
	DecoderPlay();
	audio_decoder->song_info = audio_decoder_get_song_info();
//	APP_DBG("[INFO]: sample rate from %u Hz, Channel:%d, type:%d\n", (unsigned int)audio_decoder->song_info->sampling_rate, audio_decoder->song_info->num_channels, audio_decoder->song_info->stream_type);

	APP_DBG("Decoder Service Start...\n");

	//enable hfp source channel
//	AudioCoreSourceEnable(DecoderSourceNumGet());

	BtHf_MsbcDecoderStartedSet(TRUE);
	
#ifdef CFG_FUNC_REMIND_SOUND_EN
	SoftFlagDeregister(SoftFlagDecoderMask & ~SoftFlagDecoderApp);//安全性清理
#endif	
	return 0;
}

/*******************************************************************************
 * AEC 
 * 用于进行AEC的缓存远端发送数据
 * uint:sample
 ******************************************************************************/
void BtHf_AECEffectInit(void)
{
	gCtrlVars.mic_aec_unit.enable				 = 1;
	gCtrlVars.mic_aec_unit.es_level    			 = BT_HFP_AEC_ECHO_LEVEL;
	gCtrlVars.mic_aec_unit.ns_level     		 = BT_HFP_AEC_NOISE_LEVEL;
	AudioEffectAecInit(&gCtrlVars.mic_aec_unit, 1, 16000);//固定为16K采样率
}

bool BtHf_AECInit(void)
{
	if(!BtHfCt->AecDelayBuf)
		BtHfCt->AecDelayBuf = (uint8_t*)osPortMalloc(DELAY_LEN_PER_BLOCK*LEN_PER_SAMPLE*MAX_DELAY_BLOCK);  //64*2*32
	
	if(BtHfCt->AecDelayBuf == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->AecDelayBuf, 0, (DELAY_LEN_PER_BLOCK*LEN_PER_SAMPLE*MAX_DELAY_BLOCK));
	
	BtHfCt->AecDelayRingBuf.addr = BtHfCt->AecDelayBuf;
	BtHfCt->AecDelayRingBuf.mem_capacity = (DELAY_LEN_PER_BLOCK*LEN_PER_SAMPLE*MAX_DELAY_BLOCK);
	BtHfCt->AecDelayRingBuf.mem_len = (DELAY_LEN_PER_BLOCK*LEN_PER_SAMPLE*DEFAULT_DELAY_BLK);
	BtHfCt->AecDelayRingBuf.p = 0;
	BtHfCt->AecDelayRingBuf.cb = NULL;
	BtHfCt->SourceBuf_Aec = (uint16_t*)osPortMalloc(CFG_BTHF_PARA_SAMPLES_PER_FRAME * 2);
	if(BtHfCt->SourceBuf_Aec == NULL)
	{
		return FALSE;
	}
	memset(BtHfCt->SourceBuf_Aec, 0, CFG_BTHF_PARA_SAMPLES_PER_FRAME * 2);
	return TRUE;
}

void BtHf_AECReset(void)
{
	memset(BtHfCt->AecDelayBuf, 0, (DELAY_LEN_PER_BLOCK*LEN_PER_SAMPLE*MAX_DELAY_BLOCK));

	BtHfCt->AecDelayRingBuf.addr = BtHfCt->AecDelayBuf;
	BtHfCt->AecDelayRingBuf.mem_capacity = (DELAY_LEN_PER_BLOCK*LEN_PER_SAMPLE*MAX_DELAY_BLOCK);
	BtHfCt->AecDelayRingBuf.mem_len = (DELAY_LEN_PER_BLOCK*LEN_PER_SAMPLE*DEFAULT_DELAY_BLK);
	BtHfCt->AecDelayRingBuf.p = 0;
	BtHfCt->AecDelayRingBuf.cb = NULL;

	memset(BtHfCt->SourceBuf_Aec, 0, CFG_BTHF_PARA_SAMPLES_PER_FRAME * 2);
}

void BtHf_AECDeinit(void)
{
	if(BtHfCt->SourceBuf_Aec)
	{
		osPortFree(BtHfCt->SourceBuf_Aec);
		BtHfCt->SourceBuf_Aec = NULL;
	}
	
	BtHfCt->AecDelayRingBuf.addr = NULL;
	BtHfCt->AecDelayRingBuf.mem_capacity = 0;
	BtHfCt->AecDelayRingBuf.mem_len = 0;
	BtHfCt->AecDelayRingBuf.p = 0;
	BtHfCt->AecDelayRingBuf.cb = NULL;
	
	if(BtHfCt->AecDelayBuf)
	{
		osPortFree(BtHfCt->AecDelayBuf);
		BtHfCt->AecDelayBuf = NULL;
	}
}


uint32_t BtHf_AECRingDataSet(void *InBuf, uint16_t InLen)
{
	if(InLen == 0)
		return 0;
	
	return mv_mwrite(InBuf, 1, InLen*2, &BtHfCt->AecDelayRingBuf);
}

uint32_t BtHf_AECRingDataGet(void* OutBuf, uint16_t OutLen)
{
	if(OutLen == 0)
		return 0;
	
	return mv_mread(OutBuf, 1, OutLen*2, &BtHfCt->AecDelayRingBuf);
}

int32_t BtHf_AECRingDataSpaceLenGet(void)
{
	return mv_mremain(&BtHfCt->AecDelayRingBuf)/2;
}

int32_t BtHf_AECRingDataLenGet(void)
{
	return mv_msize(&BtHfCt->AecDelayRingBuf)/2;
}

int16_t *BtHf_AecInBuf(uint16_t OutLen)
{
	if(BtHf_AECRingDataLenGet() >= OutLen)
	{
		BtHf_AECRingDataGet(BtHfCt->SourceBuf_Aec , OutLen);
	}
	else
	{
		memset(BtHfCt->SourceBuf_Aec, 0, OutLen * 2);
	}
	return (int16_t *)BtHfCt->SourceBuf_Aec;
}

/*******************************************************************************
 * pitch shifter 
 * 变调
 ******************************************************************************/
//table根据实际需求进行修改
const int32_t btHfPitchShifterTable[MAX_PITCH_SHIFTER_STEP] = 
{
	0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120,
	-120, -110, -100, -90, -80, -70, -60, -50, -40, -30, -20, -10
};

void BtHf_PitchShifterEffectInit(void)
{
#if ((CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN)&&defined(BT_HFP_MIC_PITCH_SHIFTER_FUNC))
	gCtrlVars.pitch_shifter_unit.enable = 1;
	gCtrlVars.pitch_shifter_unit.semitone_steps = 0;
	gCtrlVars.pitch_shifter_unit.channel = 1;

	AudioEffectPitchShifterInit(&gCtrlVars.pitch_shifter_unit, 1, 16000);
#endif
}

void BtHf_PitchShifterEffectConfig(uint8_t step)
{
#if ((CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN)&&defined(BT_HFP_MIC_PITCH_SHIFTER_FUNC))
	gCtrlVars.pitch_shifter_unit.semitone_steps = btHfPitchShifterTable[step];

	AudioEffectPitchShifterConfig(&gCtrlVars.pitch_shifter_unit);
#endif
}

/*******************************************************************************
 * hf volume sync
 ******************************************************************************/
void SetBtHfSyncVolume(uint8_t gain)
{
	BT_MANAGER_ST *	tempBtManager = NULL;
	uint32_t volume = gain;
	//BtHfCt->BtSyncVolume = volume;
	
	tempBtManager = GetBtManager();

	if(tempBtManager == NULL)
		return;

	tempBtManager->volGain = (uint8_t)((volume*15)/CFG_PARA_MAX_VOLUME_NUM);
	APP_DBG("hf volume:%d\n", tempBtManager->volGain);
	HfpSpeakerVolume(tempBtManager->volGain);
}

uint8_t GetBtHfSyncVolume(void)
{
	return BtHfCt->BtSyncVolume;
}


/***********************************************************************************
 * get sco data(CVSD)
 **********************************************************************************/
//Len/Length都是Samples(单声道)
uint16_t BtHf_ScoDataGet(void *OutBuf,uint16_t OutLen)
{
	uint16_t NumSamples = 0;
	NumSamples = MCUCircular_GetData(&BtHfCt->BtScoCircularBuf,OutBuf,OutLen*2);
	return NumSamples/2;
}

void BtHf_ScoDataSet(void *InBuf,uint16_t InLen)
{
	MCUCircular_PutData(&BtHfCt->BtScoCircularBuf,InBuf,InLen*2);
}

uint16_t BtHf_ScoDataLenGet(void)
{
	uint16_t NumSamples = 0;
	NumSamples = MCUCircular_GetDataLen(&BtHfCt->BtScoCircularBuf);
	return NumSamples/2;
}

uint16_t BtHf_ScoDataSpaceLenGet(void)
{
	uint16_t NumSamples = 0;
	NumSamples = MCUCircular_GetSpaceLen(&BtHfCt->BtScoCircularBuf);
	return NumSamples/2;
}

/***********************************************************************************
 * 接收到HFP SCO数据,插入缓存fifo
 **********************************************************************************/
extern uint32_t BtHfModeExitFlag;
int16_t BtHf_SaveScoData(uint8_t* data, uint16_t len)
{
	uint32_t	insertLen = 0;
	int32_t 	remainLen = 0;
	uint16_t	msbcLen = 0;
	int32_t SRCDoneLen;	//sample number

	if(BtHfModeExitFlag)
		return -1;

	if((BtHfCt->ModeKillFlag)||(BtHfCt->state != TaskStateRunning)||(!BtHfCt->hfModeAudioResInitOk))
		return -1;
	
#if (defined(CFG_BT_NUMBER_REMIND))
	if((GetHfpState() == BT_HFP_STATE_INCOMING)&&(BtHfCt->btPhoneCallRing == 0))
		return -1;
#endif

	BtHfCt->btPhoneCallRing = 1;
	
	if((DecoderSourceNumGet()!=BT_HF_SOURCE_NUM)
#ifdef CFG_FUNC_REMIND_SOUND_EN
#ifdef CFG_BT_NUMBER_REMIND
		|| (BtHfCt->PhoneNumberRemindStart)
#endif
		|| SoftFlagGet(SoftFlagDecoderRemind)
#endif
		)
		return -1;

	BtHfCt->ScoInputLen = len;

	//延时开启source
	if(BtHfCt->btHfResumeCnt)
	{
		BtHfCt->btHfResumeCnt++;
		if(BtHfCt->btHfResumeCnt == 10)
		{
			AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
		}
		
		if(BtHfCt->btHfResumeCnt>=35)
		{
			BtHfCt->btHfResumeCnt=0;
			AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
		}
	}

	if(BtHfCt->codecType == HFP_AUDIO_DATA_PCM)
	{
		if(gSpecificDevice)
		{
			if(memcmp(&BtHfCt->scoSpecificFifo[BtHfCt->scoSpecificIndex][0],data, 120) == 0)
			{
				BtHfCt->scoSpecificCount++;
			}
			else
			{
				if(BtHfCt->scoSpecificCount>=20)
				{
					APP_DBG("unmute-resume...\n");
					AudioCoreSourceUnmute(BT_HF_SOURCE_NUM, TRUE, TRUE);
				}
				BtHfCt->scoSpecificCount=0;
			}
			memcpy(&BtHfCt->scoSpecificFifo[BtHfCt->scoSpecificIndex][0],data, 120);
			BtHfCt->scoSpecificIndex++;
			if(BtHfCt->scoSpecificIndex>=3)
				BtHfCt->scoSpecificIndex=0;

			if(BtHfCt->scoSpecificCount == 20)
			{
				APP_DBG("cvsd-mute\n");
				AudioCoreSourceMute(BT_HF_SOURCE_NUM, TRUE, TRUE);
			}
		}
		//cvsd - plc
		if(lc_sco_data_error_flag)
		{
			//APP_DBG("sco_error:%d\n", lc_sco_data_error_flag);
			lc_sco_data_error_flag=0;
			cvsd_plc_bad_frame(BtHfCt->cvsdPlcState, (int16_t *)BtHfCt->ScoInputFifo);
		}
		else
		{
			cvsd_plc_good_frame(BtHfCt->cvsdPlcState, (int16_t *)data, (int16_t *)BtHfCt->ScoInputFifo);
		}
		
		BtHfCt->ScoInputLen = len;
		
		//CVSD : 转采样 8k->16k mono
		//在此进行转采样,会带来300-400us的任务占用时间
		//SRCDoneLen = resampler_apply(BtHfCt->ScoInResamplerCt, (int16_t*)BtHfCt->ScoInputFifo, (int16_t*)BtHfCt->ScoInSrcOutBuf, BtHfCt->ScoInputLen/2);
		SRCDoneLen = resampler_polyphase_apply(BtHfCt->ScoInResamplerCt, (int16_t*)BtHfCt->ScoInputFifo, (int16_t*)BtHfCt->ScoInSrcOutBuf, BtHfCt->ScoInputLen/2);
		if(SRCDoneLen>0)
		{
			if(BtHf_ScoDataSpaceLenGet() > SRCDoneLen)
				BtHf_ScoDataSet(BtHfCt->ScoInSrcOutBuf, SRCDoneLen);
		}

		{
			//send data(120Bytes)
			if(BtHf_SendScoBufLenGet() > BT_CVSD_PACKET_LEN)
			{
				memset(BtHfCt->scoSendBuf,0,BT_CVSD_PACKET_LEN);
				BtHf_SendScoBufGet(BtHfCt->scoSendBuf, BT_CVSD_PACKET_LEN);
				HfpSendScoData(BtHfCt->scoSendBuf, BT_CVSD_PACKET_LEN);
			}
		}
	}
	else
	{
		if(BtHfCt->ScoInputLen == BT_MSBC_PACKET_LEN)
		{
			//send data(60Bytes)
			if(BtHf_SendScoBufLenGet() > BT_MSBC_PACKET_LEN)
			{
				memset(BtHfCt->scoSendBuf,0,BT_MSBC_PACKET_LEN);
				BtHf_SendScoBufGet(BtHfCt->scoSendBuf,BT_MSBC_PACKET_LEN);
				HfpSendScoData(BtHfCt->scoSendBuf, BT_MSBC_PACKET_LEN);
				//printf("s");
			}
		}
		
		//接收到的数据需要做如下判断
		//1.全0 丢弃
		//2.长度非60的，需要放入缓存
		//3.长度为60的直接处理
		if((!lc_sco_data_error_flag)&&(!BtHf_MsbcDecoderStartedGet()))
		{
			//为0数据丢弃
			if((data[0]==0)&&(data[1]==0)&&(data[2]==0)&&(data[3]==0)&&(data[len-2]==0)&&(data[len-1]==0))
				return -1;
		}

		//长度
		if(BtHfCt->ScoInputLen != BT_MSBC_PACKET_LEN)
		{
			if((data[0] != 0x01)||(data[2] != 0xad)||(data[3] != 0x00))
			{
				APP_DBG("msbc discard...\n");
				return -1;
			}
		}

		{
			//数据格式: 0-1:sync header;  2-58:msbc data(57bytes - 有效数据); 59:tail
			memcpy(BtHfCt->ScoInputFifo, data, 60);
			
			//接收到错误的msbc数据包,则将该次收到的数据全部清空为0
			if(lc_sco_data_error_flag)
			{
				//APP_DBG("msbc e:%d\n",lc_sco_data_error_flag);
				lc_sco_data_error_flag=0;

				BtHfCt->ScoInputFifo[0] = 0x01;
				BtHfCt->ScoInputFifo[1] = 0x08;
				
				memset(&BtHfCt->ScoInputFifo[2], 0, 58);

				//if((!BtHf_MsbcDecoderStartedGet())||(lc_sco_data_error_flag == 2))
				if(!BtHf_MsbcDecoderStartedGet())
					return -1;

				BtHfCt->msbcPlcCnt = 2;
			}

			if(BtHf_SbcRecvBufSpaceLenGet()<=60)
			{
				APP_DBG("msbc recv fifo full-1\n");
				return -1;
			}

			BtHf_SbcRecvBufSet(BtHfCt->ScoInputFifo, 60);

			if(BtHf_SbcRecvBufLenGet()<60*2)
				return -1;

			if(BtHfCt->msbcPlcCnt == 0)
			{
				uint8_t i;
				for(i=0;i<60;i++)
				{
					BtHf_SbcRecvBufGet(&BtHfCt->ScoInputFifo[0], 1);
					if(BtHfCt->ScoInputFifo[0] == 0x01)
					{
						BtHf_SbcRecvBufGet(&BtHfCt->ScoInputFifo[0], 1);
						BtHfCt->ScoInputFifo[0]&=0x0f;
						if(BtHfCt->ScoInputFifo[0] == 0x08)
						{
							BtHf_SbcRecvBufGet(&BtHfCt->ScoInputFifo[0], 58);
							break;
						}
					}
				}

				if(i==60)
					return -1;
			}
			else
			{
				BtHfCt->msbcPlcCnt--;
				BtHf_SbcRecvBufGet(&BtHfCt->ScoInputFifo[0], 60);

				memset(&BtHfCt->ScoInputFifo[0], 0, 60);
			}
		}

		msbcLen = 57;
		
		remainLen = mv_mremain(&BtHfCt->msbcMemHandle);
		if(remainLen <= msbcLen)
		{
			APP_DBG("msbc receive fifo is full\n");
			return -1;
		}
		insertLen = mv_mwrite(BtHfCt->ScoInputFifo, msbcLen, 1, &BtHfCt->msbcMemHandle);
		if(insertLen != msbcLen)
		{
			APP_DBG("insert data len err! i:%ld,d:%d\n", insertLen, msbcLen);
		}

		//decoder start
		if(!BtHf_MsbcDecoderStartedGet())
		{
			if(BtHf_MsbcDataLenGet()>BT_MSBC_LEVEL_START)
			{
				int32_t ret=0;
				ret = BtHf_MsbcDecoderStart();
				if(ret == 0)
				{
					APP_DBG("msbc decoder start success\n");
					BtHfCt->DecoderSync = TaskStateStarting;
					BtHf_MsbcDecoderStartedSet(TRUE);
					AudioCoreSourceEnable(BT_HF_SOURCE_NUM);
				}
				else
				{
					APP_DBG("msbc decoder start fail\n");
				}
				
			}
		}
	}

	return 0;
}

/***********************************************************************************
 * 处理HFP需要发送的数据
 **********************************************************************************/
//2-EV3 = 60bytes/packet
//处理发送数据函数入口
void BtHf_MicProcess(void)
{
	int32_t SRCDoneLen;	//sample number
	int32_t return_flag=0;
	//APP_DBG("S1\n");//bkd
	if(BtHfCt->ModeKillFlag)
		return;
	
	if(gSpecificDevice)
	{
		if((!BtHfCt->CvsdInitFlag)&&(!BtHfCt->MsbcInitFlag))
		{
			return_flag=1;
		
		}
		if(GetHfpState() < BT_HFP_STATE_CONNECTED)
		{
			return_flag=1;
		}
	}
	else
	{
		if(GetHfpState() < BT_HFP_STATE_ACTIVE)
		{
			return_flag=1;
		}
	}
	if(return_flag)
	{
		AudioCoreSinkDisable(AUDIO_HF_SCO_SINK_NUM);
		return;
	}

	if(BtHfCt->btHfScoSendReady)
	{
		BtHfCt->btHfScoSendReady = 0;
		BtHfCt->btHfScoSendStart = 1;
		return_flag=1;
	}

	if(!BtHfCt->btHfScoSendStart)
	{
		return_flag=1;
	}

	if(return_flag)
	{
		AudioCoreSinkDisable(AUDIO_HF_SCO_SINK_NUM);
		return;
	}
	//sink2 channel
	if(BtHfCt->AudioCoreSinkHfSco->Enable == 0)
		BtHfCt->AudioCoreSinkHfSco->Enable = 1;
	
	AudioCoreSinkEnable(AUDIO_HF_SCO_SINK_NUM);
	
	if(BtHfCt->codecType == HFP_AUDIO_DATA_PCM)
	{
		//CVSD
		if(BtHf_SinkScoDataLenGet()>BT_CVSD_PACKET_LEN) //16K:120sample -> 8K:60sample
		{
			//1.转采样 16K -> 8K
			BtHf_SinkScoDataGet(BtHfCt->ScoOutputFifo, BT_CVSD_PACKET_LEN);
			//SRCDoneLen = resampler_apply(BtHfCt->ScoOutResamplerCt, (int16_t*)BtHfCt->ScoOutputFifo, (int16_t*)BtHfCt->ScoOutSrcOutBuf, BT_CVSD_PACKET_LEN);
			SRCDoneLen = resampler_polyphase_apply(BtHfCt->ScoOutResamplerCt, (int16_t*)BtHfCt->ScoOutputFifo, (int16_t*)BtHfCt->ScoOutSrcOutBuf, BT_CVSD_PACKET_LEN);

			//2.put data to send fifo
			if(SRCDoneLen>0)
			{
				if(BtHf_SendScoBufSpaceLenGet() > SRCDoneLen*2)
					BtHf_SendScoBufSet(BtHfCt->ScoOutSrcOutBuf, SRCDoneLen*2);
			}
		}
	}
	else
	{
		if(BtHfCt->mSbcEncoderStart)
		{
			//msbc encoder: 120 Sample
			if(BtHf_SinkScoDataLenGet()>BT_MSBC_MIC_INPUT_SAMPLE)
			{
				//1.encoder
				BtHf_SinkScoDataGet(BtHfCt->ScoOutputFifo, BT_MSBC_MIC_INPUT_SAMPLE);
				memset(BtHfCt->msbcEncoderFifo, 0, BT_MSBC_PACKET_LEN);
				
				//2. add sync header
				if(BtHfCt->msbcSyncCount > 3)
				{
					BtHfCt->msbcSyncCount = 0;
				}
				memcpy(&BtHfCt->msbcEncoderFifo[0], sco_sync_header[BtHfCt->msbcSyncCount], 2);
				BtHfCt->msbcSyncCount++;
				BtHfCt->msbcSyncCount %= 4;

				//3. encode
				sbc_encoder_encode(BtHfCt->sbc_encode_ct, BtHfCt->ScoOutputFifo, &BtHfCt->msbcEncoderFifo[2], &BtHfCt->msbc_encoded_data_length);

				//4.put data to send fifo
				if(BtHf_SendScoBufSpaceLenGet() > BT_MSBC_PACKET_LEN)
					BtHf_SendScoBufSet(BtHfCt->msbcEncoderFifo, BT_MSBC_PACKET_LEN);
			}
		}
	}
}
#endif

