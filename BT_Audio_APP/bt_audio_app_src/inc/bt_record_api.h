/**
 **************************************************************************************
 * @file    bt_record_api.h
 * @brief   ����K��¼��ģʽ(ʹ��ͨ����·)
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

//ͨ��������΢��ˮλ��ز���
#define BT_RECORD_LEVEL_LOW			57*2*4//256*2*4
#define BT_RECORD_LEVEL_HIGH		57*2*12//256*2*12

//MIC���봦��
void BtRecord_MicProcess(void);

//������յ���SCO����(MSBC)
int16_t BtRecord_SaveScoData(uint8_t* data, uint16_t len);

//MSBC����
void BtRecord_MsbcEncoderInit(void);
void BtRecord_MsbcEncoderDeinit(void);

//MSBC����
void BtRecord_MsbcMemoryReset(void);
int32_t BtRecord_MsbcDecoderInit(void);
int32_t BtRecord_MsbcDecoderDeinit(void);
void BtRecord_MsbcDecoderStartedSet(bool flag);
bool BtRecord_MsbcDecoderStartedGet(void);
uint32_t BtRecord_MsbcDataLenGet(void);
bool BtRecord_MsbcDecoderIsInitialized(void);

//��������ͨ������
void SetBtRecordSyncVolume(uint8_t gain);

//�������ݴ��� (AudioCore sink���)44.1K
uint16_t BtRecord_SinkScoDataSet(void* InBuf, uint16_t InLen);
void BtRecord_SinkScoDataGet(void* OutBuf, uint16_t OutLen);
uint16_t BtRecord_SinkScoDataSpaceLenGet(void);
uint16_t BtRecord_SinkScoDataLenGet(void);

//�������ݴ��� (AudioCore sink���)ת����֮��16K
uint16_t BtRecord_ResampleOutDataSet(void* InBuf, uint16_t InLen);
void BtRecord_ResampleOutDataGet(void* OutBuf, uint16_t OutLen);
uint16_t BtRecord_ResampleOutDataSpaceLenGet(void);
uint16_t BtRecord_ResampleOutDataLenGet(void);

//���յ���CVSD��������
uint16_t BtRecord_ScoDataGet(void *OutBuf,uint16_t OutLen);
void BtRecord_ScoDataSet(void *InBuf,uint16_t InLen);
uint16_t BtRecord_ScoDataLenGet(void);
uint16_t BtRecord_ScoDataSpaceLenGet(void);

#endif /*__BT_RECORD_API_H__*/



