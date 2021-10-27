/**
 **************************************************************************************
 * @file    recorder_service.h
 * @brief    
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-04-28 11:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __RECORDER_SERVICE_H__
#define __RECORDER_SERVICE_H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include "type.h"



#ifdef	CFG_FUNC_RECORD_SD_UDISK
#define FILE_PATH_LEN							(strlen(MEDIA_VOLUME_STR_C)  + strlen(CFG_PARA_RECORDS_FOLDER) + 1 + 8 + 4 + 1 )//vol:3 + folder/:4 + name:8 + .ext:4
#define	FILE_INDEX_MAX							CFG_PARA_REC_MAX_FILE_NUM //录音文件最多这个数
#define FILE_NAME_MAX							65535//(1~65535) //录音文件名编号范围。根据录音先后顺序命令。
#define FILE_NAME_VALUE_SIZE					(sizeof(uint16_t))
#endif
#define MEDIA_ENCODER_SAMPLE_MAX			2000	//保护

#ifndef MEDIA_RECORDER_CHANNEL
#define MEDIA_RECORDER_CHANNEL				2
#endif
#ifndef MEDIA_RECORDER_BITRATE
#define MEDIA_RECORDER_BITRATE				96
#endif
#ifndef FILE_WRITE_FIFO_LEN
#define FILE_WRITE_FIFO_LEN					(512 * 15)//保障阻塞时 编码数据 缓冲。适应U盘、卡兼容性。
#endif

uint16_t RecFileIndex(char *string);
void IntToStrMP3Name(char *string, uint16_t number);

MessageHandle GetMediaRecorderServiceMessageHandle(void);

bool MediaRecorderServiceCreate(MessageHandle parentMsgHandle);

bool MediaRecorderServiceKill(void);

bool MediaRecorderServiceStart(void);

bool MediaRecorderServicePause(void);

bool MediaRecorderServiceResume(void);

bool MediaRecorderServiceStop(void);

void MediaRecorderRun(void);

void MediaRecorderStop(void);

void MediaRecorderEncode(void);
bool IsRecoding(void);
bool MallocRecorderCtSink1Buf(uint16_t sampleperframe);
void ReleaseRecorderCtSink1Buf(void);
void AudioRecResInit(void);
MessageHandle GetMediaRecorderMessageHandle(void);
bool MediaRecordHeapEnough(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __RECORDER_SERVICE_H__
