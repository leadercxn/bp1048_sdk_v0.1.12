/**
 *****************************************************************************
 * @file     otg_device_audio.h
 * @author   Owen
 * @version  V1.0.0
 * @date     24-June-2015
 * @brief    audio device interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#ifndef __OTG_DEVICE_AUDIO_H__
#define	__OTG_DEVICE_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
	
#include "type.h"
#include "resampler_polyphase.h"
#include "mcu_circular_buf.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

#define AUDIO_MAX_VOLUME	4096

typedef struct _UsbAudio
{
	uint8_t					InitOk;
	uint8_t					AltSet;
	uint8_t 				Channel;
	uint8_t 				Mute;
	uint32_t				LeftVol;
	uint32_t				RightVol;
	uint32_t				AudioSampleRate;
	bool					AudioSampleRateFlag;
	uint32_t				FramCount;
	uint32_t				TempFramCount;

	//转采样
	ResamplerPolyphaseContext		*Resampler;
	int16_t*				SRCOutBuf;

	//缓存FIFO
	MCU_CIRCULAR_CONTEXT 	CircularBuf;
	int16_t*				PCMBuffer;
#ifdef	CFG_FUNC_SOFT_ADJUST_IN
	int16_t* 				pBufTemp;//[128*4];//SRA转换前固定buf 128 samples
	MCU_CIRCULAR_CONTEXT 	CircularBufSRA;
	uint16_t				*SRAFifo;
#endif
}UsbAudio;

bool OTG_DeviceAudioSendPcCmd(uint8_t Cmd);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
