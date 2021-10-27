/**
 **************************************************************************************
 * @file    audio_core.c
 * @brief   audio core 
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
#include <nds32_intrinsic.h>
#include "type.h"
#include "freertos.h"
#include "audio_core_api.h"
#include "app_config.h"
#include "debug.h"
#include "ctrlvars.h"
#include "audio_effect.h"
#include "mode_switch_api.h"
#include "main_task.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "mcu_circular_buf.h"
#include "audio_adjust.h"
#ifdef CFG_APP_BT_MODE_EN
#include "bt_config.h"
#include "bt_play_api.h"
#if (BT_HFP_SUPPORT == ENABLE)
#include "bt_hf_api.h"
#endif
#endif

typedef enum
{
	AC_RUN_CHECK,//用于检测是否需要暂停任务，如果需要暂停任务，则停留再该状态
	AC_RUN_GET,
	AC_RUN_PROC,
	AC_RUN_PUT,
}AudioCoreRunState;

static AudioCoreRunState AudioState = AC_RUN_CHECK;
AudioCoreContext		AudioCore;

/**
 * @brief       AudioCore 数据源 块获取
 * @note		只有所有使能的音频流buf数据满足1帧，才会DMA搬运数据帧
*/
static bool AudioCoreSourceDateGet(void)
{
	uint32_t i;
	uint32_t Cnt = 0;
	bool IsSourceDataEnough = TRUE; 

	//解决mic通道关闭情况一下，非流数据通道会有数据取不全导致杂音问题
	//sam,20200304
#if( CFG_RES_MIC_SELECT != 0)
	if(!AudioCore.AudioSource[MIC_SOURCE_NUM].Enable)
#endif
	{
		for(i = 0; i< AUDIO_CORE_SINK_MAX_NUM; i++)
		{
			if((AudioCore.AudioSink[i].Enable == TRUE)
			&& (AudioCore.AudioSink[i].FuncDataSpaceLenGet() < mainAppCt.SamplesPreFrame))
			{
				return FALSE;
			}
		}
	}


	for(i = 0; i< AUDIO_CORE_SOURCE_MAX_MUN; i++)
	{
		if(AudioCore.AudioSource[i].Enable == FALSE)
		{
			Cnt++;//增加一个计数器，用于统计是否所有source都被禁止
			continue;
		}
		if((AudioCore.AudioSource[i].IsSreamData == FALSE)/**/
		|| (AudioCore.AudioSource[i].FuncDataGetLen == NULL))
		{
			continue;
		}
		if(AudioCore.AudioSource[i].FuncDataGetLen() < mainAppCt.SamplesPreFrame)
		{
			IsSourceDataEnough = FALSE; 
			break;
		}
	}
	if(Cnt == AUDIO_CORE_SOURCE_MAX_MUN)//没有一个通道是能数据，退出
	{
		return FALSE;
	}
	if(!IsSourceDataEnough)
	{
		IsSourceDataEnough = TRUE;
		return FALSE;
	}

	for(i = 0; i< AUDIO_CORE_SOURCE_MAX_MUN; i++)
	{
		if(AudioCore.AudioSource[i].Enable == FALSE)
		{
			continue;
		}
		//打印用于监控
//		if((i == 1)
//		//&& (GetSystemMode() == AppModeBtAudioPlay)
//		&& (AudioCore.AudioSource[i].FuncDataGetLen() < mainAppCt.SamplesPreFrame))//Test
//		{
//			APP_DBG("E, %d, %d\n", DecodedPcmDataLenGet(), GetValidSbcDataSize());
//		}
		//长度必须是FRAME，不足填0
		memset(AudioCore.AudioSource[i].PcmInBuf, 0, mainAppCt.SamplesPreFrame * AudioCore.AudioSource[i].PcmFormat * 2);
		AudioCore.AudioSource[i].FuncDataGet(AudioCore.AudioSource[i].PcmInBuf, mainAppCt.SamplesPreFrame);
	}
	
#ifdef CFG_FUNC_FREQ_ADJUST
#ifdef CFG_FUNC_SOFT_ADJUST_IN
	if(SoftFlagGet(SoftFlagBtSra))
	{
		AudioCoreSourceSRAStepCnt();
#ifdef CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO
		AudioCoreSinkSRAStepCnt();
#endif
	}
	else
#endif
	{
		AudioCoreSourceFreqAdjust();
	}
#endif	
	return TRUE;
}

/**
 * @func        AudioCoreSinkDataSet
 * @brief       AudioCore 音效输出 推数据帧到 音频输出系统的buf
 * @param       None
 * @Output      DMA搬运数据帧到音频输出buf
 * @return      bool
 * @note		音效输出数据buf满足1帧时，dma搬运1帧数据
 * Record
*/
static bool AudioCoreSinkDataSet(void)
{
	uint32_t i;
	
	for(i = 0; i< AUDIO_CORE_SINK_MAX_NUM; i++)
	{
		if((AudioCore.AudioSink[i].Enable == TRUE)
		&& (AudioCore.AudioSink[i].FuncDataSpaceLenGet() < mainAppCt.SamplesPreFrame))
		{
			if(AudioCore.AudioSink[i].SreamDataState == 1)
			{
				AudioCore.AudioSink[i].SreamDataState = 2;
				continue;
			}

			if(GetSystemMode() == AppModeUsbDevicePlay)
			{
				continue;
			}
			return FALSE;
		}
	}

	for(i = 0; i< AUDIO_CORE_SINK_MAX_NUM; i++)
	{
		if(AudioCore.AudioSink[i].SreamDataState == 2)
		{
			AudioCore.AudioSink[i].SreamDataState = 1;
			continue;
		}
		if((AudioCore.AudioSink[i].Enable == TRUE) && (AudioCore.AudioSink[i].FuncDataSet != NULL))
		{
			AudioCore.AudioSink[i].FuncDataSet(AudioCore.AudioSink[i].PcmOutBuf, mainAppCt.SamplesPreFrame);
		}
	}
	return TRUE;
}

void AudioCoreSourcePcmFormatConfig(uint8_t Index, uint16_t Format)
{
	if(Index < AUDIO_CORE_SOURCE_MAX_MUN)
	{
		AudioCore.AudioSource[Index].PcmFormat = Format;
	}
}

void AudioCoreSourceEnable(uint8_t Index)
{
	if(Index < AUDIO_CORE_SOURCE_MAX_MUN)
	{
		AudioCore.AudioSource[Index].Enable = TRUE;
	}
}

void AudioCoreSourceDisable(uint8_t Index)
{
	if(Index < AUDIO_CORE_SOURCE_MAX_MUN)
	{
		AudioCore.AudioSource[Index].Enable = FALSE;
	}
}

void AudioCoreSourceMute(uint8_t Index, bool IsLeftMute, bool IsRightMute)
{
	if(IsLeftMute)
	{
		AudioCore.AudioSource[Index].LeftMuteFlag = TRUE;
	}
	if(IsRightMute)
	{
		AudioCore.AudioSource[Index].RightMuteFlag = TRUE;
	}
}

void AudioCoreSourceUnmute(uint8_t Index, bool IsLeftUnmute, bool IsRightUnmute)
{
	if(IsLeftUnmute)
	{
		AudioCore.AudioSource[Index].LeftMuteFlag = FALSE;
	}
	if(IsRightUnmute)
	{
		AudioCore.AudioSource[Index].RightMuteFlag = FALSE;
	}
}

void AudioCoreSourceVolSet(uint8_t Index, uint16_t LeftVol, uint16_t RightVol)
{
	AudioCore.AudioSource[Index].LeftVol = LeftVol;
	AudioCore.AudioSource[Index].RightVol = RightVol;
}

void AudioCoreSourceVolGet(uint8_t Index, uint16_t* LeftVol, uint16_t* RightVol)
{
	*LeftVol = AudioCore.AudioSource[Index].LeftCurVol;
	*RightVol = AudioCore.AudioSource[Index].RightCurVol;
}

void AudioCoreSourceConfig(uint8_t Index, AudioCoreSource* Source)
{
	memcpy(&AudioCore.AudioSource[Index], Source, sizeof(AudioCoreSource));
}

void AudioCoreSinkEnable(uint8_t Index)
{
	AudioCore.AudioSink[Index].Enable = TRUE;
}

void AudioCoreSinkDisable(uint8_t Index)
{
	AudioCore.AudioSink[Index].Enable = FALSE;
}

void AudioCoreSinkMute(uint8_t Index, bool IsLeftMute, bool IsRightMute)
{
	if(IsLeftMute)
	{
		AudioCore.AudioSink[Index].LeftMuteFlag = TRUE;
	}
	if(IsRightMute)
	{
		AudioCore.AudioSink[Index].RightMuteFlag = TRUE;
	}
}

void AudioCoreSinkUnmute(uint8_t Index, bool IsLeftUnmute, bool IsRightUnmute)
{
	if(IsLeftUnmute)
	{
		AudioCore.AudioSink[Index].LeftMuteFlag = FALSE;
	}
	if(IsRightUnmute)
	{
		AudioCore.AudioSink[Index].RightMuteFlag = FALSE;
	}
}

void AudioCoreSinkVolSet(uint8_t Index, uint16_t LeftVol, uint16_t RightVol)
{
	AudioCore.AudioSink[Index].LeftVol = LeftVol;
	AudioCore.AudioSink[Index].RightVol = RightVol;
}

void AudioCoreSinkVolGet(uint8_t Index, uint16_t* LeftVol, uint16_t* RightVol)
{
	*LeftVol = AudioCore.AudioSink[Index].LeftCurVol;
	*RightVol = AudioCore.AudioSink[Index].RightCurVol;
}

void AudioCoreSinkConfig(uint8_t Index, AudioCoreSink* Sink)
{
	memcpy(&AudioCore.AudioSink[Index], Sink, sizeof(AudioCoreSink));
}


void AudioCoreProcessConfig(AudioCoreProcessFunc AudioEffectProcess)
{
	AudioCore.AudioEffectProcess = AudioEffectProcess;
}

///**
// * @func        AudioCoreConfig
// * @brief       AudioCore参数块，本地化API
// * @param       AudioCoreContext *AudioCoreCt
// * @Output      None
// * @return      None
// * @Others      外部配置的参数块，复制一份到本地
// */
//void AudioCoreConfig(AudioCoreContext *AudioCoreCt)
//{
//	memcpy(&AudioCore, AudioCoreCt, sizeof(AudioCoreContext));
//}

bool AudioCoreInit(void)
{
	return TRUE;
}

void AudioCoreDeinit(void)
{
	AudioState = AC_RUN_CHECK;
}

/**
 * @func        AudioCoreRun
 * @brief       音源拉流->音效处理+混音->推流
 * @param       None
 * @Output      None
 * @return      None
 * @Others      当前由audioCoreservice任务保障此功能有效持续。
 * Record
 */
extern uint32_t 	IsAudioCorePause;
extern uint32_t 	IsAudioCorePauseMsgSend;
void AudioProcessMain(void);
void AudioCoreRun(void)
{
	bool ret;
	switch(AudioState)
	{
		case AC_RUN_CHECK:
			if(IsAudioCorePause == TRUE)
			{
				if(IsAudioCorePauseMsgSend == TRUE)
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_AUDIO_CORE_HOLD;
					MessageSend(GetAudioCoreServiceMsgHandle(), &msgSend);

					IsAudioCorePauseMsgSend = FALSE;
				}
				return;
			}
//			AudioState = AC_RUN_GET;
		case AC_RUN_GET:
			ret = AudioCoreSourceDateGet();
			if(ret == FALSE)
			{
				return;
			}
		case AC_RUN_PROC:
			//AudioCore.AudioProcess();
			AudioProcessMain();
			AudioState = AC_RUN_PUT;

		case AC_RUN_PUT:
			ret = AudioCoreSinkDataSet();
			if(ret == FALSE)
			{
				return;
			}
			//AudioState = AC_RUN_GET;
			AudioState = AC_RUN_CHECK;
			break;
		default:
			break;
	}
}

//音效处理函数，主入口
//将mic通路数据剥离出来统一处理
//mic通路数据和具体模式无关
//提示音通路无音效，剥离后在sink端混音。
void AudioProcessMain(void)
{	
	AduioCoreSourceVolSet();

#ifdef CFG_FUNC_RECORDER_EN
	if(AudioCore.AudioSource[PLAYBACK_SOURCE_NUM].Enable == TRUE)
	{
		if(AudioCore.AudioSource[PLAYBACK_SOURCE_NUM].PcmFormat == 1)
		{
			uint16_t i;
			for(i = mainAppCt.SamplesPreFrame * 2 - 1; i > 0; i--)
			{
				AudioCore.AudioSource[PLAYBACK_SOURCE_NUM].PcmInBuf[i] = AudioCore.AudioSource[PLAYBACK_SOURCE_NUM].PcmInBuf[i / 2];
			}
		}
	}
#endif

	if(AudioCore.AudioSource[APP_SOURCE_NUM].Enable == TRUE)////music buff
	{
		#if (BT_HFP_SUPPORT == ENABLE) && defined(CFG_APP_BT_MODE_EN)
		if((GetSystemMode() != AppModeBtHfPlay) && (GetSystemMode() != AppModeBtRecordPlay))
		#endif
		{
			if(AudioCore.AudioSource[APP_SOURCE_NUM].PcmFormat == 1)
			{
				uint16_t i;
				for(i = mainAppCt.SamplesPreFrame * 2 - 1; i > 0; i--)
				{
					AudioCore.AudioSource[APP_SOURCE_NUM].PcmInBuf[i] = AudioCore.AudioSource[APP_SOURCE_NUM].PcmInBuf[i / 2];
				}
			}
		}
	}	
		
#if defined(CFG_FUNC_REMIND_SOUND_EN)	
	if(AudioCore.AudioSource[REMIND_SOURCE_NUM].Enable == TRUE)////remind buff
	{
		if(AudioCore.AudioSource[REMIND_SOURCE_NUM].PcmFormat == 1)
		{
			uint16_t i;
			for(i = mainAppCt.SamplesPreFrame * 2 - 1; i > 0; i--)
			{
				AudioCore.AudioSource[REMIND_SOURCE_NUM].PcmInBuf[i] = AudioCore.AudioSource[REMIND_SOURCE_NUM].PcmInBuf[i / 2];
			}
		}
	}	
#endif

	if(AudioCore.AudioEffectProcess != NULL)
	{
		AudioCore.AudioEffectProcess((AudioCoreContext*)&AudioCore);
	}
	
    #ifdef CFG_FUNC_BEEP_EN
    if(AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].Enable == TRUE)   ////dacx buff
	{
		Beep(AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf, mainAppCt.SamplesPreFrame);
	}
    #endif

	AduioCoreSinkVolSet();//调音量
}

//音量淡入淡出					
#define MixerFadeVolume(a, b, c)  	\
    if(a > b + c)		    \
	{						\
		a -= c;				\
	}						\
	else if(a + c < b)	   	\
	{						\
		a += c;				\
	}						\
	else					\
	{						\
		a = b;				\
	}

void AduioCoreSourceVolSet(void)
{
#if 0
	uint32_t i, j;
    
	uint16_t LeftVol, RightVol, LeftVolStep, RightVolStep;

#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT == ENABLE)
	if(GetSystemMode() == AppModeBtHfPlay)
	{
		AudioCore.AudioSource[0].PreGain = BT_HFP_MIC_DIGIT_GAIN;
		AudioCore.AudioSource[1].PreGain = BT_HFP_INPUT_DIGIT_GAIN;
	}
	else
	{
		AudioCore.AudioSource[0].PreGain = 4095;//0db
		AudioCore.AudioSource[1].PreGain = 4095;
	}
#endif
#endif

	for(j=0; j<AUDIO_CORE_SOURCE_MAX_MUN; j++)
	{
		if(!AudioCore.AudioSource[j].Enable)
		{
			continue;
		}
		if(AudioCore.AudioSource[j].LeftMuteFlag == TRUE)
		{
			LeftVol = 0;
		}
		else
		{
			LeftVol = AudioCore.AudioSource[j].LeftVol;
		}
		if(AudioCore.AudioSource[j].RightMuteFlag == TRUE)
		{
			RightVol = 0;
		}
		else
		{
			RightVol = AudioCore.AudioSource[j].RightVol;
		}

		LeftVolStep = LeftVol > AudioCore.AudioSource[j].LeftCurVol ? (LeftVol - AudioCore.AudioSource[j].LeftCurVol) : (AudioCore.AudioSource[j].LeftCurVol - LeftVol);
		LeftVolStep = LeftVolStep / mainAppCt.SamplesPreFrame + LeftVolStep % mainAppCt.SamplesPreFrame ? 1 : 0;
		RightVolStep = RightVol > AudioCore.AudioSource[j].RightCurVol ? (RightVol - AudioCore.AudioSource[j].RightCurVol) : (AudioCore.AudioSource[j].RightCurVol - RightVol);
		RightVolStep = RightVolStep / mainAppCt.SamplesPreFrame + RightVolStep % mainAppCt.SamplesPreFrame ? 1 : 0;

		if(AudioCore.AudioSource[j].PcmFormat == 2)//立体声
		{
			for(i=0; i<mainAppCt.SamplesPreFrame; i++)
			{
				AudioCore.AudioSource[j].PcmInBuf[2 * i + 0] = __nds32__clips((((((int32_t)AudioCore.AudioSource[j].PcmInBuf[2 * i + 0]) * AudioCore.AudioSource[j].LeftCurVol + 2048) >> 12) * AudioCore.AudioSource[j].PreGain) >> 12, (16)-1);//__SSAT((((((int32_t)AudioCore.AudioSource[j].PcmInBuf[2 * i] * AudioCore.AudioSource[j].LeftCurVol) >> 12) * AudioCore.AudioSource[j].PreGain) >> 12), 16);
				AudioCore.AudioSource[j].PcmInBuf[2 * i + 1] = __nds32__clips((((((int32_t)AudioCore.AudioSource[j].PcmInBuf[2 * i + 1]) * AudioCore.AudioSource[j].RightCurVol + 2048) >> 12) * AudioCore.AudioSource[j].PreGain) >> 12, (16)-1);//__SSAT((((((int32_t)AudioCore.AudioSource[j].PcmInBuf[2 * i + 1] * AudioCore.AudioSource[j].RightCurVol) >> 12) * AudioCore.AudioSource[j].PreGain) >> 12), 16);
				
				MixerFadeVolume(AudioCore.AudioSource[j].LeftCurVol, LeftVol, LeftVolStep);
				MixerFadeVolume(AudioCore.AudioSource[j].RightCurVol, RightVol, RightVolStep);
			}
		}
		else
		{
			for(i=0; i<mainAppCt.SamplesPreFrame; i++)
			{
				AudioCore.AudioSource[j].PcmInBuf[i] = __nds32__clips((((((int32_t)AudioCore.AudioSource[j].PcmInBuf[i]) * AudioCore.AudioSource[j].LeftCurVol + 2048) >> 12) * AudioCore.AudioSource[j].PreGain + 2048) >> 12, (16)-1);//__SSAT((((((int32_t)AudioCore.AudioSource[j].PcmInBuf[i] * AudioCore.AudioSource[j].LeftCurVol) >> 12) * AudioCore.AudioSource[j].PreGain) >> 12), 16);
				MixerFadeVolume(AudioCore.AudioSource[j].LeftCurVol, LeftVol, LeftVolStep);
			}
		}
	}
#endif
}
int16_t MusicVolBuf[512*2];
void AudioCoreAppSourceVolSet(uint16_t Source,int16_t *pcm_in,uint16_t n,uint16_t Channel)
{
#if 1
	uint32_t i, j;
    
	uint16_t LeftVol, RightVol,	LeftVolStep, RightVolStep;

#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT == ENABLE)
	if(GetSystemMode() == AppModeBtHfPlay)
	{
		AudioCore.AudioSource[0].PreGain = BT_HFP_MIC_DIGIT_GAIN;
		AudioCore.AudioSource[1].PreGain = BT_HFP_INPUT_DIGIT_GAIN;
	}
	else
	{
		AudioCore.AudioSource[0].PreGain = 4095;//0db
		AudioCore.AudioSource[1].PreGain = 4095;
	}
#endif
#endif

    if(pcm_in == NULL) return;
	
	j = Source;

	if(!AudioCore.AudioSource[j].Enable)
	{
		return;
	}
	if(AudioCore.AudioSource[j].LeftMuteFlag == TRUE)
	{
		LeftVol = 0;
	}
	else
	{
		LeftVol = AudioCore.AudioSource[j].LeftVol;
	}
	if(AudioCore.AudioSource[j].RightMuteFlag == TRUE)
	{
		RightVol = 0;
	}
	else
	{
		RightVol = AudioCore.AudioSource[j].RightVol;
	}

	LeftVolStep = LeftVol > AudioCore.AudioSource[j].LeftCurVol ? (LeftVol - AudioCore.AudioSource[j].LeftCurVol) : (AudioCore.AudioSource[j].LeftCurVol - LeftVol);
	LeftVolStep = LeftVolStep / mainAppCt.SamplesPreFrame + LeftVolStep % mainAppCt.SamplesPreFrame ? 1 : 0;
	RightVolStep = RightVol > AudioCore.AudioSource[j].RightCurVol ? (RightVol - AudioCore.AudioSource[j].RightCurVol) : (AudioCore.AudioSource[j].RightCurVol - RightVol);
	RightVolStep = RightVolStep / mainAppCt.SamplesPreFrame + RightVolStep % mainAppCt.SamplesPreFrame ? 1 : 0;

	if(Channel == 2)//立体声
	{
		for(i=0; i<mainAppCt.SamplesPreFrame; i++)
		{
			pcm_in[2 * i + 0] = __nds32__clips((((((int32_t)pcm_in[2 * i + 0]) * AudioCore.AudioSource[j].LeftCurVol + 2048) >> 12) * AudioCore.AudioSource[j].PreGain + 2048) >> 12, (16)-1);//__SSAT((((((int32_t)AudioCore.AudioSource[j].PcmInBuf[2 * i] * AudioCore.AudioSource[j].LeftCurVol) >> 12) * AudioCore.AudioSource[j].PreGain) >> 12), 16);
			pcm_in[2 * i + 1] = __nds32__clips((((((int32_t)pcm_in[2 * i + 1]) * AudioCore.AudioSource[j].RightCurVol + 2048) >> 12) * AudioCore.AudioSource[j].PreGain + 2048) >> 12, (16)-1);//__SSAT((((((int32_t)AudioCore.AudioSource[j].PcmInBuf[2 * i + 1] * AudioCore.AudioSource[j].RightCurVol) >> 12) * AudioCore.AudioSource[j].PreGain) >> 12), 16);
			
			MixerFadeVolume(AudioCore.AudioSource[j].LeftCurVol, LeftVol, LeftVolStep);
			MixerFadeVolume(AudioCore.AudioSource[j].RightCurVol, RightVol, RightVolStep);
		}
	}
	else
	{
		for(i=0; i<mainAppCt.SamplesPreFrame; i++)
		{
			pcm_in[i] = __nds32__clips((((((int32_t)pcm_in[i]) * AudioCore.AudioSource[j].LeftCurVol + 2048) >> 12) * AudioCore.AudioSource[j].PreGain + 2048) >> 12, (16)-1);//__SSAT((((((int32_t)AudioCore.AudioSource[j].PcmInBuf[i] * AudioCore.AudioSource[j].LeftCurVol) >> 12) * AudioCore.AudioSource[j].PreGain) >> 12), 16);
			MixerFadeVolume(AudioCore.AudioSource[j].LeftCurVol, LeftVol, LeftVolStep);
		}
	}
#endif
}

void AduioCoreSinkVolSet(void)
{
	uint32_t i;
	uint8_t j;

	uint16_t LeftVol, RightVol,	LeftVolStep, RightVolStep;

	for(j=0; j<AUDIO_CORE_SINK_MAX_NUM; j++)
	{
		if(AudioCore.AudioSink[j].Enable == TRUE)
		{
			if(AudioCore.AudioSink[j].LeftMuteFlag == TRUE)
			{
				LeftVol = 0;
			}
			else
			{
				LeftVol = AudioCore.AudioSink[j].LeftVol;
			}
			if(AudioCore.AudioSink[j].RightMuteFlag == TRUE)
			{
				RightVol = 0;
			}
			else
			{
				RightVol = AudioCore.AudioSink[j].RightVol;
			}

			LeftVolStep = LeftVol > AudioCore.AudioSink[j].LeftCurVol ? (LeftVol - AudioCore.AudioSink[j].LeftCurVol) : (AudioCore.AudioSink[j].LeftCurVol - LeftVol);
			LeftVolStep = LeftVolStep / mainAppCt.SamplesPreFrame + LeftVolStep % mainAppCt.SamplesPreFrame ? 1 : 0;
			RightVolStep = RightVol > AudioCore.AudioSink[j].RightCurVol ? (RightVol - AudioCore.AudioSink[j].RightCurVol) : (AudioCore.AudioSink[j].RightCurVol - RightVol);
			RightVolStep = RightVolStep / mainAppCt.SamplesPreFrame + RightVolStep % mainAppCt.SamplesPreFrame ? 1 : 0;

			if(AudioCore.AudioSink[j].PcmFormat == 2)
			{
				for(i=0; i<mainAppCt.SamplesPreFrame; i++)
				{
					AudioCore.AudioSink[j].PcmOutBuf[2 * i + 0] = __nds32__clips((((int32_t)AudioCore.AudioSink[j].PcmOutBuf[2 * i + 0]) * AudioCore.AudioSink[j].LeftCurVol + 2048) >> 12, (16)-1);
					AudioCore.AudioSink[j].PcmOutBuf[2 * i + 1] = __nds32__clips((((int32_t)AudioCore.AudioSink[j].PcmOutBuf[2 * i + 1]) * AudioCore.AudioSink[j].RightCurVol + 2048) >> 12, (16)-1);

					MixerFadeVolume(AudioCore.AudioSink[j].LeftCurVol, LeftVol, LeftVolStep);
					MixerFadeVolume(AudioCore.AudioSink[j].RightCurVol, RightVol, RightVolStep);
				}
			}
			else if(AudioCore.AudioSink[j].PcmFormat == 1)
			{
				for(i=0; i<mainAppCt.SamplesPreFrame; i++)
				{
					AudioCore.AudioSink[j].PcmOutBuf[i] = __nds32__clips((((int32_t)AudioCore.AudioSink[j].PcmOutBuf[i]) * AudioCore.AudioSink[j].LeftCurVol + 2048) >> 12, (16)-1);

					MixerFadeVolume(AudioCore.AudioSink[j].LeftCurVol, LeftVol, LeftVolStep);
				}
			}
		}
	}
}

