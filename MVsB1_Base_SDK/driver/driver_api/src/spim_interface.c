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
 *		DMA传输完成标志位清除
 * @param	PeripheralID	PERIPHERAL_ID_SPIM_RX接收DMA外设通道    PERIPHERAL_ID_SPIM_TX发送DMA外设通道
 * @return
 *		无
 */
void SPIM_DMA_Done_Clear(uint8_t PeripheralID)
{
	DMA_InterruptFlagClear(PeripheralID,DMA_DONE_INT);
}

/**
 * @brief
 *		查询全双工模式下数据接收发送是否完成
 * @param	无
 *  @return
 *		0传送未完成 1传送完成
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
 *		查询DMA模式下半双工模式下接收或发送完成状态
 * @param	PeripheralID	PERIPHERAL_ID_SPIM_RX接收 PERIPHERAL_ID_SPIM_TX发送
 * @return
 *		0数据传送未完成 1数据传送完成
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
 *		SPIM全双工模式下使用DMA方式接收与发送数据
 * @param	SendBuf		发送数据buffer
 * @param	RecvBuf		接收数据buffer
 * @param	Length		数据传送长度
 * @return
 *		无
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
 *		SPIM使用DMA方式发送数据
 * @param	SendBuf		发送数据buffer
 * @param	Length		发送数据长度
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
 *		SPIM使用DMA方式接收数据
 * @param	RecvBuf		接收数据buffer
 * @param	Length		数据接收长度
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
