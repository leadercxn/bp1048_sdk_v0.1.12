/**
 **************************************************************************************
 * @file    decoder_service.h
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __DECODER_SERVICE_H__
#define __DECODER_SERVICE_H__

#include "type.h"
#include "rtos_api.h"
#include "audio_decoder_api.h"
#include "mvstdio.h"
#include "app_message.h"

typedef enum
{
	DecoderStateNone = 0,
	DecoderStateInitialized,
	DecoderStateDeinitializing,
	DecoderStateStop,
	DecoderStatePlay,
	DecoderStatePause,
	DecoderStateDecoding,
	DecoderStateBackupData,
	DecoderStateToSavePcmData,
	DecoderStateSavePcmData,

}DecoderState;


typedef enum
{
	DS_EVENT_SERVICE_INITED,
	DS_EVENT_SERVICE_STARTED,
	DS_EVENT_DECODE_INITED,
	DS_EVENT_DECODE_DECODED,
} DecoderServiceEvent;

#define DECODER_BUF_SIZE   					(1024 * 28)	//解码器开销，Unit:  BYTE，需充分测试。
#define DECODER_BUF_SIZE_MP3				(1024 * 19)
#define DECODER_BUF_SIZE_SBC				(1024 * 7)
//WMA:28536,AAC:28444,APE:25340,FLA:22580,MP3:19136,AIF:11236,WAV:11228,SBC:5624,实际块大小会变，比如adpcm的wav。

#define DECODER_FIFO_SIZE_MIN				(CFG_PARA_MAX_SAMPLES_PER_FRAME * 4)//最小 fifo 设置
#define DECODER_FIFO_SIZE_FOR_PLAYER 		(CFG_PARA_MAX_SAMPLES_PER_FRAME * 16)	//无损读取、高压缩解码阻塞情况，要求大缓冲。如：Flac24bit 1.5Mbps要改为*20;1.7Mbps要改为*24
#define	DECODER_FIFO_SIZE_FOR_SBC			(CFG_PARA_MAX_SAMPLES_PER_FRAME * 6)
#define DECODER_FIFO_SIZE_FOR_MP3			(CFG_PARA_MAX_SAMPLES_PER_FRAME * 8)

bool DecoderServiceCreate(MessageHandle parentMsgHandle, uint32_t BufSize, uint16_t FifoSize);

void DecoderServiceStart(void);

void DecoderServicePause(void);

void DecoderServiceResume(void);

void DecoderServiceStop(void);

void DecoderServiceKill(void);

void DecoderServiceMsg(void);

MessageHandle GetDecoderServiceMsgHandle(void);

TaskState GetDecoderServiceState(void);

uint16_t DecodedPcmDataGet(void * pcmData, uint16_t sampleLen);

/**
 * @func        DecodedPcmDataLenGet
 * @brief       Decoder数据输出FIFO内采样点数
 * @Output      None
 * @return      uint16_t FIFO中采样点数
 * Record
 */
uint16_t DecodedPcmDataLenGet(void);

int32_t DecoderInit(void *io_handle, int32_t ioType, int32_t decoderType);

void DecoderDeinit(void);

void DecoderPlay(void);

void DecoderReset(void);

void DecoderStop(void);

void DecoderMuteAndStop(void);

void DecoderPause(void);

void DecoderResume(void);

//注意，StepTime 单位为毫秒，实际传递值为整数秒
void DecoderFF(uint32_t StepTime);

//注意，StepTime 单位为毫秒，实际传递值为整数秒
void DecoderFB(uint32_t StepTime);

bool DecoderSeek(uint32_t Time);

uint32_t DecoderServicePlayTimeGet(void);

uint32_t GetDecoderState(void);

uint8_t DecoderSourceNumGet(void);

//解码器输出在audiocore source通路中配置的Num号
void DecoderSourceNumSet(uint8_t Num);


#endif /*__DECODER_SERVICE_H__*/

