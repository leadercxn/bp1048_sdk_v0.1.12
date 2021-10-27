/**
 **************************************************************************************
 * @file    i2s_interface.h
 * @brief   audio i2s interface
 *
 * @author  
 * @version V1.1.0
 *
 * $Created: 2019-01-04 14:01:05$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef		__I2S_INTERFACE_H__
#define		__I2S_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include "i2s.h"

typedef struct __I2SParamCt
{
	uint8_t          IsMasterMode;    //0: master, 1: slave

	uint32_t         SampleRate;      //sample rate

	I2S_DATA_FORMAT  I2sFormat;       //I2S_DATA_FORMAT

	I2S_DATA_LENGTH  I2sBits;         //I2S_DATA_LENGTH
	
	uint8_t          I2sTxRxEnable;   // 0: disable, 1: only tx enable, 2: only rx enable, 3: tx & rx enable
	
	//DMA
	uint8_t          TxPeripheralID;  //i2s tx peripheralID

	void            *TxBuf;           //i2s tx buf

	uint16_t         TxLen;           //i2s tx len
	
	uint8_t          RxPeripheralID;  //i2s rx peripheralID

	void            *RxBuf;           //i2s rx buf

	uint16_t         RxLen;           //i2s rx len
	

} I2SParamCt;

void 	 AudioI2S_Init(I2S_MODULE Module, I2SParamCt *ct);
void 	 AudioI2S_DeInit(I2S_MODULE Module);
uint16_t AudioI2S_DataLenGet(I2S_MODULE Module);
uint16_t AudioI2S_DataGet(I2S_MODULE Module, void* Buf, uint16_t Len);
uint16_t AudioI2S_DataSpaceLenGet(I2S_MODULE Module);
void     AudioI2S_DataSet(I2S_MODULE Module, void *Buf, uint32_t Len);


uint16_t AudioI2S0_DataLenGet(void);
uint16_t AudioI2S1_DataLenGet(void);
uint16_t AudioI2S0_DataGet(void* Buf, uint16_t Len);
uint16_t AudioI2S1_DataGet(void* Buf, uint16_t Len);
uint16_t AudioI2S0_DataSpaceLenGet(void);
uint16_t AudioI2S1_DataSpaceLenGet(void);
uint16_t AudioI2S0_DataSet(void *Buf, uint16_t Len);
uint16_t AudioI2S1_DataSet(void *Buf, uint16_t Len);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__ADC_INTERFACE_H__

