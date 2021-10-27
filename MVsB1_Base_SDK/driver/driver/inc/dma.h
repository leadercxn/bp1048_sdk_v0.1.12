/**
 *******************************************************************************
 * @file    dma.h
 * @brief	dma module driver interface

 * @author  Sam
 * @version V1.0.0

 * $Created: 2017-10-31 17:01:05$
 * @Copyright (C) 2017, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *******************************************************************************
 */

/**
 * @addtogroup DMA
 * @{
 * @defgroup dma dma.h
 * @{
 */
 
#ifndef __DMA_H__
#define __DMA_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/*
ע�����
DMA������ɹر�����Ҫ��
1���ȹر����裻2���ٹر�DMAģ��
������һ�����ʵ���DMA�ٴ�ʹ��ʱ�쳣��
�쳣�ı���ΪDMA��������
*/

typedef void (*INT_FUNC)(void);

typedef enum
{
	DMA_STREAM_INVALID = -127,
	DMA_DIRECTION_INVALID,
	DMA_AINCR_INVALID,
	DMA_MODE_INVALID,
	DMA_CBT_LEN_INVALID,
	DMA_CHANNEL_UNALLOCATED,
	DMA_NOT_ENOUGH_SPACE,
	DMA_UNALIGNED_ADDRESS,
	DMA_INT_TYPE_INVALID,
	DMA_OK = -1
}DMA_ERROR;

typedef enum
{
	PERIPHERAL_ID_SPIS_RX = 0,		//0
	PERIPHERAL_ID_SPIS_TX,			//1
	PERIPHERAL_ID_TIMER3,			//2
	PERIPHERAL_ID_SDIO_RX,			//3
	PERIPHERAL_ID_SDIO_TX,			//4
	PERIPHERAL_ID_UART0_RX,			//5
	PERIPHERAL_ID_TIMER1,			//6
	PERIPHERAL_ID_TIMER2,			//7
	PERIPHERAL_ID_SPDIF_RX,			//8 SPDIF_RX /TX��ʹ��ͬһͨ��
	PERIPHERAL_ID_SPDIF_TX,			//9
	PERIPHERAL_ID_SPIM_RX,			//10
	PERIPHERAL_ID_SPIM_TX,			//11
	PERIPHERAL_ID_UART0_TX,			//12
	PERIPHERAL_ID_UART1_RX,			//13
	PERIPHERAL_ID_UART1_TX,			//14
	PERIPHERAL_ID_TIMER4,			//15
	PERIPHERAL_ID_TIMER5,			//16
	PERIPHERAL_ID_TIMER6,			//17
	PERIPHERAL_ID_AUDIO_ADC0_RX,	//18
	PERIPHERAL_ID_AUDIO_ADC1_RX,	//19
	PERIPHERAL_ID_AUDIO_DAC0_TX,	//20
	PERIPHERAL_ID_AUDIO_DAC1_TX,	//21
	PERIPHERAL_ID_I2S0_RX,			//22
	PERIPHERAL_ID_I2S0_TX,			//23
	PERIPHERAL_ID_I2S1_RX,			//24
	PERIPHERAL_ID_I2S1_TX,			//25
	PERIPHERAL_ID_PPWM,     		//26
	PERIPHERAL_ID_ADC,     			//27
	PERIPHERAL_ID_SOFTWARE,			//28

}DMA_PERIPHERAL_ID;

typedef enum
{
	DMA_BLOCK_MODE = 0,
	DMA_CIRCULAR_MODE
}DMA_MODE;

typedef enum
{
	DMA_CHANNEL_DIR_PERI2MEM = 0,
	DMA_CHANNEL_DIR_MEM2PERI,
	DMA_CHANNEL_DIR_MEM2MEM
}DMA_CHANNEL_DIR;

typedef enum
{
	DMA_PRIORITY_LEVEL_LOW = 0,
	DMA_PRIORITY_LEVEL_HIGH
}DMA_PRIORITY_LEVEL;

typedef enum
{
	DMA_SRC_DWIDTH_BYTE = 0,
	DMA_SRC_DWIDTH_HALF_WORD,
	DMA_SRC_DWIDTH_WORD
}DMA_SRC_DWIDTH;

typedef enum
{
	DMA_DST_DWIDTH_BYTE = 0,
	DMA_DST_DWIDTH_HALF_WORD,
	DMA_DST_DWIDTH_WORD
}DMA_DST_DWIDTH;

typedef enum
{
	DMA_SRC_AINCR_NO = 0,
	DMA_SRC_AINCR_SRC_WIDTH
}DMA_SRC_AINCR;

typedef enum
{
	DMA_DST_AINCR_NO = 0,
	DMA_DST_AINCR_DST_WIDTH
}DMA_DST_AINCR;

typedef struct
{
	DMA_CHANNEL_DIR		Dir;
	DMA_MODE			Mode;
	uint16_t			ThresholdLen;//����ѭ��ģʽ����Ч
	DMA_SRC_DWIDTH		SrcDataWidth;
	DMA_DST_DWIDTH		DstDataWidth;
	DMA_SRC_AINCR		SrcAddrIncremental;
	DMA_DST_AINCR		DstAddrIncremental;
	uint32_t			SrcAddress;
	uint32_t			DstAddress;	
	uint16_t			BufferLen;
} DMA_CONFIG;

typedef enum
{
	DMA_DONE_INT = 0,
	DMA_THRESHOLD_INT,
	DMA_ERROR_INT
}DMA_INT_TYPE;

//------------------------------------------------------����Ϊͨ������API---------------------------------------------------------
/**
 * @brief	����DMA��ͨ�������
 * @param	DMAChannelMap		DMAͨ�������
 *
 * @return
 *   		NONE
 * @note 
 */
void DMA_ChannelAllocTableSet(uint8_t* DMAChannelMap);

/**
 * @brief	ʹ��DMAͨ������������
 * @param	PeripheralID	����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *   		DMA_ERROR
 * @note
 */
DMA_ERROR DMA_ChannelEnable(uint8_t PeripheralID);

/**
 * @brief	��ֹDMAͨ������������
 * @param	PeripheralID	����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *   		DMA_ERROR
 * @note
 */
DMA_ERROR DMA_ChannelDisable(uint8_t PeripheralID);

/**
 * @brief	����ָ��ͨ���жϵĻص�����
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 * @param	IntType				�ж����ͣ����DMA_INT_TYPE
 * @param	CallBack			�жϻص�����
 *								 
 * @return
 *   		DMA_ERROR
 * @note
 */
DMA_ERROR DMA_InterruptFunSet(uint8_t PeripheralID, DMA_INT_TYPE IntType, INT_FUNC CallBack);

/**
 * @brief	������ر�ָ��ͨ�����ж�
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 * @param	IntType				�ж����ͣ����DMA_INT_TYPE
 * @param	En					1:����    0:�ر�	
 *								 
 * @return
 *   		DMA_ERROR
 * @note
 */
DMA_ERROR DMA_InterruptEnable(uint8_t PeripheralID, DMA_INT_TYPE IntType, bool En);

/**
 * @brief	��ȡָ��ͨ�����жϱ�־
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 * @param	IntType				�ж����ͣ����DMA_INT_TYPE
 *								 
 * @return
 *   		�����жϱ�־��DMA_ERROR
 * @note	�жϱ�־�ڲ������жϵ������Ҳ����λ
 */
int32_t DMA_InterruptFlagGet(uint8_t PeripheralID, DMA_INT_TYPE IntType);

/**
 * @brief	���ָ��ͨ�����жϱ�־
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 * @param	IntType				�ж����ͣ����DMA_INT_TYPE
 *								 
 * @return
 *   		DMA_ERROR
 * @note	�жϱ�־�ڲ������жϵ������Ҳ����λ
 */
DMA_ERROR DMA_InterruptFlagClear(uint8_t PeripheralID, DMA_INT_TYPE IntType);

//------------------------------------------------------����ΪBlockģʽר��API-------------------------------------------------------
/**
 * @brief	��������IDָ����DMAͨ��ΪBlockģʽ
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *   		DMA_ERROR
 * @note	
 */
DMA_ERROR DMA_BlockConfig(uint8_t PeripheralID);

/**
 * @brief	����blockģʽ�Ĵ���buffer
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 * @param	Ptr					����buffer�׵�ַ
 * @param	Len					����buffer����
 *
 * @return	
 *			DMA_ERROR
 * @note
*/
DMA_ERROR DMA_BlockBufSet(uint8_t PeripheralID, void* Ptr, uint16_t Len);

/**
 * @brief	��ȡblockģʽ�Ĵ���buffer���Ѿ���������ݳ���
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 *
 * @return	
 *			�Ѿ���������ݳ��Ȼ�DMA_ERROR
 * @note
*/
int32_t DMA_BlockDoneLenGet(uint8_t PeripheralID);

//-----------------------------------------------------����ΪCircularģʽר��API----------------------------------------------------
/**
 * @brief	��������IDָ����DMAͨ��ΪCircularģʽ
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 * @param	CircularThreshold	ѭ��FIFOˮλ����󳤶�65535����λByte (����������ѭ��ģʽ����Ч)
 * @param	CircularFifoAddr	ѭ��FIFO�׵�ַ �����ַ�����룬�᷵��DMA_UNALIGNED_ADDRESS��(����������ѭ��ģʽ����Ч)
 * @param	CircularFifoLen		ѭ��FIFO���ȣ���󳤶�65535����λByte (����������ѭ��ģʽ����Ч)
 *
 * @return
 *   		DMA_ERROR
 * @note	
 */
DMA_ERROR DMA_CircularConfig(uint8_t PeripheralID, uint16_t CircularThreshold, void* CircularFifoAddr, uint16_t CircularFifoLen);

/**
 * @brief	��ѭ��FIFO����������
 * @param	PeripheralID	����ID�����DMA_PERIPHERAL_ID
 * @param	Ptr				�������ݵ��׵�ַ
 * @param	Len				�������ݳ��ȣ���λByte
 *
 * @return
 *   		�������������ʵ�ʳ���
 * @note	ѭ��ģʽ�£�ʣ��ռ䳤��С��Len����ʱ���Ὣbuffer����������ʵ������ĳ���
 */
int32_t DMA_CircularDataPut(uint8_t PeripheralID, void* Ptr, uint16_t Len);

/**
 * @brief	��ѭ��FIFO��ȡ������
 * @param	PeripheralID	����ID�����DMA_PERIPHERAL_ID
 * @param	Ptr				ȡ�����ݴ�ŵ��׵�ַ
 * @param	MaxLen			����ȡ�����ݵ���󳤶ȣ���λByte
 *
 * @return
 *   		����ȡ�����ݵ�ʵ�ʳ���
 * @note	ѭ��ģʽ�£�ʣ�����ݳ���С��Lenʱ���Ὣbufferȡ�գ�����ʵ������ĳ���
 */
int32_t DMA_CircularDataGet(uint8_t PeripheralID, void* Ptr, uint16_t MaxLen);

/**
 * @brief	����ѭ��FIFO��ˮλֵ�����������������ˮλʱ��λDMA_THRESHOLD_FLAG
 * @param	PeripheralID	����ID�����DMA_PERIPHERAL_ID
 * @param	Len				ˮλֵ����λByte��
 *
 * @return
 *   		DMA_ERROR
 * @note	
 */
DMA_ERROR DMA_CircularThresholdLenSet(uint8_t PeripheralID, uint16_t Len);

/**
 * @brief	��ȡѭ��FIFO��ˮλֵ�����������������ˮλʱ��λDMA_THRESHOLD_FLAG
 * @param	PeripheralID	����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *   		����ˮλֵ��DMA_ERROR
 * @note	
 */
int32_t DMA_CircularThresholdLenGet(uint8_t PeripheralID);

/**
 * @brief	��ȡѭ��FIFO�е����ݳ��ȣ���λByte
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *    		����buffer�е����ݳ���(��λByte) �� DMAC_ERROR
 * @note
 */
int32_t DMA_CircularDataLenGet(uint8_t PeripheralID);

/**
 * @brief	��ȡѭ��FIFO�е�ʣ��ռ䳤�ȣ���λByte
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *   		����buffer��ʣ��ռ䳤�Ȼ� DMAC_ERROR
 * @note
 */
int32_t DMA_CircularSpaceLenGet(uint8_t PeripheralID);

/**
 * @brief	���ѭ��FIFO
 * @param	PeripheralID	����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *   		����DMA_ERROR
 * @note	ѭ��FIFO������ǽ���дָ��ָ��ͬһ��ַ
 */
DMA_ERROR DMA_CircularFIFOClear(uint8_t PeripheralID);

/**
 * @brief	��ȡѭ��FIFO��дָ���λ�ã���λByte
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *    		����ѭ��FIFO��дָ��λ�û�DMA_ERROR
 * @note
 */
int32_t DMA_CircularWritePtrGet(uint8_t PeripheralID);

/**
 * @brief	��ȡѭ��FIFO�Ķ�ָ���λ�ã���λByte
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *    		����ѭ��FIFO�ж�ָ��λ�û�DMA_ERROR
 * @note
 */
int32_t DMA_CircularReadPtrGet(uint8_t PeripheralID);

/**
 * @brief	����ѭ��FIFO��дָ���λ�ã���λByte
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 * @param	Ptr					ѭ��FIFO��дָ��λ��
 *
 * @return
 *    		DMA_ERROR
 * @note
 */
DMA_ERROR DMA_CircularWritePtrSet(uint8_t PeripheralID, uint16_t Ptr);

/**
 * @brief	����ѭ��FIFO��дָ���λ�ã���λByte
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 * @param	Ptr					ѭ��FIFO�ж�ָ��λ��
 *
 * @return
 *    		DMA_ERROR
 * @note
 */
DMA_ERROR DMA_CircularReadPtrSet(uint8_t PeripheralID, uint16_t Ptr);

/**
 * @brief	�ر�DMAͨ������������
 * @param	PeripheralID	����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *   		DMA_ERROR
 * @note
 */
DMA_ERROR DMA_ChannelClose(uint8_t PeripheralID);

/**
 * @brief	��ȡDMAͨ����
 * @param	PeripheralID	����ID�����DMA_PERIPHERAL_ID
 *
 * @return
 *   		DMA ͨ���ţ���Чֵ��0~~7��
 * @note
 */
uint32_t DMA_ChannelNumGet(uint8_t PeripheralID);

/**
 * @brief	��������IDָ����DMAͨ��Ϊblockģʽ
 * @param	SrcAddr		blockģʽ�Ĵ���bufferԴ��ַ
 * @param	DstAddr	    blockģʽ�Ĵ���bufferĿ�ĵ�ַ
 * @param	Length	    blockģʽ�Ĵ���buffer�ĳ���
 * @param	SrcDwidth	Դ��ַ�İ��˵�λ��WORD;HALF WORD;BYTE��
 * @param   DstDwidth	Ŀ�ĵ�ַ�İ��˵�λ��WORD;HALF WORD;BYTE��
 * @param   SrcAincr	Դ��ַ��ָ�����ӵ�λ��һ�����SrcDwidth����
 * @param   DstAincr	Ŀ�ĵ�ַ��ָ�����ӵ�λ��һ�����DstDwidth����
 *
 * @return
 *   		DMA_OK
 * @note
 */
DMA_ERROR DMA_MemCopy(uint32_t SrcAddr, uint32_t DstAddr, uint16_t Length, uint8_t SrcDwidth, uint8_t DstDwidth, uint8_t SrcAincr, uint8_t DstAincr);

//--------------------------------------------------------����ΪTimerר��API-------------------------------------------------------
/**
 * @brief	��������IDָ����DMAͨ��������һ����޶�ΪTIMER��
 * @param	PeripheralID		����ID�����DMA_PERIPHERAL_ID
 * @param	DMA_CONFIG			DMA���ýṹ�壬���DMA_CONFIG �����ַ�����룬�᷵��DMA_UNALIGNED_ADDRESS��
 *
 * @return
 *   		DMA_ERROR
 * @note
 */
DMA_ERROR DMA_TimerConfig(uint8_t PeripheralID, DMA_CONFIG* Config);


DMA_ERROR DMA_ConfigModify(uint8_t PeripheralID, int8_t SrcDwidth, uint8_t DstDwidth, uint8_t SrcAincr, uint8_t DstAincr);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif 


/**
 * @}
 * @}
 */
