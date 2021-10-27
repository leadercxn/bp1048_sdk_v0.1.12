/**
 **************************************************************************************
 * @file    bt_record_api.h
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

#ifndef __BT_RECORD_API_H__
#define __BT_RECORD_API_H__

#include "type.h"

//通话采样率微调水位监控参数
#define BT_RECORD_LEVEL_LOW			57*2*4//256*2*4
#define BT_RECORD_LEVEL_HIGH		57*2*12//256*2*12

//MIC输入处理
void BtRecord_MicProcess(void);

//处理接收到的SCO数据(MSBC)
int16_t BtRecord_SaveScoData(uint8_t* data, uint16_t len);

//MSBC编码
void BtRecord_MsbcEncoderInit(void);
void BtRecord_MsbcEncoderDeinit(void);

//MSBC解码
void BtRecord_MsbcMemoryReset(void);
int32_t BtRecord_MsbcDecoderInit(void);
int32_t BtRecord_MsbcDecoderDeinit(void);
void BtRecord_MsbcDecoderStartedSet(bool flag);
bool BtRecord_MsbcDecoderStartedGet(void);
uint32_t BtRecord_MsbcDataLenGet(void);
bool BtRecord_MsbcDecoderIsInitialized(void);

//设置蓝牙通话音量
void SetBtRecordSyncVolume(uint8_t gain);

//缓存数据处理 (AudioCore sink输出)44.1K
uint16_t BtRecord_SinkScoDataSet(void* InBuf, uint16_t InLen);
void BtRecord_SinkScoDataGet(void* OutBuf, uint16_t OutLen);
uint16_t BtRecord_SinkScoDataSpaceLenGet(void);
uint16_t BtRecord_SinkScoDataLenGet(void);

//缓存数据处理 (AudioCore sink输出)转采样之后16K
uint16_t BtRecord_ResampleOutDataSet(void* InBuf, uint16_t InLen);
void BtRecord_ResampleOutDataGet(void* OutBuf, uint16_t OutLen);
uint16_t BtRecord_ResampleOutDataSpaceLenGet(void);
uint16_t BtRecord_ResampleOutDataLenGet(void);

//接收到的CVSD缓存数据
uint16_t BtRecord_ScoDataGet(void *OutBuf,uint16_t OutLen);
void BtRecord_ScoDataSet(void *InBuf,uint16_t InLen);
uint16_t BtRecord_ScoDataLenGet(void);
uint16_t BtRecord_ScoDataSpaceLenGet(void);

#endif /*__BT_RECORD_API_H__*/



