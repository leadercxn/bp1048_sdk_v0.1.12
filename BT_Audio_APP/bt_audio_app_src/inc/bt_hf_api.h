/**
 **************************************************************************************
 * @file    bt_hf_api.h
 * @brief	����ͨ��ģʽ
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2018-7-17 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_HF_API_H__
#define __BT_HF_API_H__

#include "type.h"

//ͨ��������΢��ˮλ��ز���
#define BT_HF_LEVEL_LOW			256*2*4
#define BT_HF_LEVEL_HIGH		256*2*12

//MIC���봦��
void BtHf_MicProcess(void);

//������յ���SCO����(CVSD or MSBC)
int16_t BtHf_SaveScoData(uint8_t* data, uint16_t len);

//MSBC����
void BtHf_MsbcEncoderInit(void);
void BtHf_MsbcEncoderDeinit(void);

//MSBC����
void BtHf_MsbcMemoryReset(void);
int32_t BtHf_MsbcDecoderInit(void);
int32_t BtHf_MsbcDecoderDeinit(void);
void BtHf_MsbcDecoderStartedSet(bool flag);
bool BtHf_MsbcDecoderStartedGet(void);
uint32_t BtHf_MsbcDataLenGet(void);

//��������ͨ������
void SetBtHfSyncVolume(uint8_t gain);

//�������ݴ��� (AudioCore sink���)
uint16_t BtHf_SinkScoDataSet(void* InBuf, uint16_t InLen);
void BtHf_SinkScoDataGet(void* OutBuf, uint16_t OutLen);
uint16_t BtHf_SinkScoDataSpaceLenGet(void);
uint16_t BtHf_SinkScoDataLenGet(void);

//���յ���CVSD��������
uint16_t BtHf_ScoDataGet(void *OutBuf,uint16_t OutLen);
void BtHf_ScoDataSet(void *InBuf,uint16_t InLen);
uint16_t BtHf_ScoDataLenGet(void);
uint16_t BtHf_ScoDataSpaceLenGet(void);

//AEC����
void BtHf_AECEffectInit(void);
bool BtHf_AECInit(void);
void BtHf_AECDeinit(void);
uint32_t BtHf_AECRingDataSet(void *InBuf, uint16_t InLen);
uint32_t BtHf_AECRingDataGet(void* OutBuf, uint16_t OutLen);
int32_t BtHf_AECRingDataSpaceLenGet(void);
int32_t BtHf_AECRingDataLenGet(void);
int16_t *BtHf_AecInBuf(uint16_t OutLen);
void BtHf_AECReset(void);

//pitch shifter
void BtHf_PitchShifterEffectInit(void);
void BtHf_PitchShifterEffectConfig(uint8_t step);

//
bool BtHf_MsbcDecoderIsInitialized(void);

#endif /*__BT_HF_API_H__*/



