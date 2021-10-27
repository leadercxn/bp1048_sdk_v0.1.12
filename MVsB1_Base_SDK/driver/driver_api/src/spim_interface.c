////////////////////////////////////////////////////////////////////////////////
//                   Mountain View Silicon Tech. Inc.
//		Copyright 2012, Mountain View Silicon Tech. Inc., ShangHai, China
//                   All rights reserved.
//
//		Filename	:spim_interface.h
//
//		Description	:
//
//
//		Changelog	:	
//					2019-5-20	create basic frame
///////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "type.h"
#include "dma.h"
#include "spim.h"



/**
 * @brief
 *		DMA������ɱ�־λ���
 * @param	PeripheralID	PERIPHERAL_ID_SPIM_RX����DMA����ͨ��    PERIPHERAL_ID_SPIM_TX����DMA����ͨ��
 * @return
 *		��
 */
void SPIM_DMA_Done_Clear(uint8_t PeripheralID)
{
	DMA_InterruptFlagClear(PeripheralID,DMA_DONE_INT);
}

/**
 * @brief
 *		��ѯȫ˫��ģʽ�����ݽ��շ����Ƿ����
 * @param	��
 *  @return
 *		0����δ��� 1�������
 * @note
 */
bool SPIM_DMA_FullDone()
{
	if((DMA_InterruptFlagGet(PERIPHERAL_ID_SPIM_RX,DMA_DONE_INT))&&(SPIM_GetIntFlag())&&DMA_InterruptFlagGet(PERIPHERAL_ID_SPIM_TX,DMA_DONE_INT))
	{
		SPIM_DMA_Done_Clear(PERIPHERAL_ID_SPIM_RX);
		SPIM_DMA_Done_Clear(PERIPHERAL_ID_SPIM_TX);
		SPIM_SetDmaEn(0);
		SPIM_ClrIntFlag();
		return 1;
	}
	return 0;
}

/**
 * @brief
 *		��ѯDMAģʽ�°�˫��ģʽ�½��ջ������״̬
 * @param	PeripheralID	PERIPHERAL_ID_SPIM_RX���� PERIPHERAL_ID_SPIM_TX����
 * @return
 *		0���ݴ���δ��� 1���ݴ������
 * @note
 */
bool SPIM_DMA_HalfDone(uint8_t PeripheralID)
{
	if((DMA_InterruptFlagGet(PeripheralID,DMA_DONE_INT))&&(SPIM_GetIntFlag()))
	{
		SPIM_DMA_Done_Clear(PeripheralID);
		SPIM_SetDmaEn(0);
		SPIM_ClrIntFlag();
		return 1;
	}
	return 0;
}



/**
 * @brief
 *		SPIMȫ˫��ģʽ��ʹ��DMA��ʽ�����뷢������
 * @param	SendBuf		��������buffer
 * @param	RecvBuf		��������buffer
 * @param	Length		���ݴ��ͳ���
 * @return
 *		��
 *		@note
 */
void SPIM_DMA_Send_Recive_Start(uint8_t* SendBuf,uint8_t* RecvBuf, uint32_t Length)
{
	DMA_ChannelDisable(PERIPHERAL_ID_SPIM_RX);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPIM_RX, DMA_DONE_INT);
	DMA_BlockConfig(PERIPHERAL_ID_SPIM_RX);
	DMA_BlockBufSet(PERIPHERAL_ID_SPIM_RX, RecvBuf, Length);
	DMA_ChannelEnable(PERIPHERAL_ID_SPIM_RX);

	DMA_ChannelDisable(PERIPHERAL_ID_SPIM_TX);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPIM_TX, DMA_DONE_INT);
	DMA_BlockConfig(PERIPHERAL_ID_SPIM_TX);
	DMA_BlockBufSet(PERIPHERAL_ID_SPIM_TX, SendBuf, Length);
	DMA_ChannelEnable(PERIPHERAL_ID_SPIM_TX);

	SPIM_DMAStart(Length,1,1);
}

/**
 * @brief
 *		SPIMʹ��DMA��ʽ��������
 * @param	SendBuf		��������buffer
 * @param	Length		�������ݳ���
 * @note
 */
void SPIM_DMA_Send_Start(uint8_t* SendBuf, uint32_t Length)
{
	DMA_ChannelDisable(PERIPHERAL_ID_SPIM_TX);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPIM_TX, DMA_DONE_INT);
	DMA_BlockConfig(PERIPHERAL_ID_SPIM_TX);
	DMA_BlockBufSet(PERIPHERAL_ID_SPIM_TX, SendBuf, Length);
	DMA_ChannelEnable(PERIPHERAL_ID_SPIM_TX);

	SPIM_DMAStart(Length,1,0);
}


/**
 * @brief
 *		SPIMʹ��DMA��ʽ��������
 * @param	RecvBuf		��������buffer
 * @param	Length		���ݽ��ճ���
 * @note
 */
void SPIM_DMA_Recv_Start(uint8_t* RecvBuf, uint32_t Length)
{
	DMA_ChannelDisable(PERIPHERAL_ID_SPIM_RX);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPIM_RX, DMA_DONE_INT);
	DMA_BlockConfig(PERIPHERAL_ID_SPIM_RX);
	DMA_BlockBufSet(PERIPHERAL_ID_SPIM_RX, RecvBuf, Length);
	DMA_ChannelEnable(PERIPHERAL_ID_SPIM_RX);

	SPIM_DMAStart(Length,0,1);
}
