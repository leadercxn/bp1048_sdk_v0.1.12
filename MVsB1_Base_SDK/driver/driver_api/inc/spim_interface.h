/* @file    Spim_interface.h
 * @brief   Spi  interface
 *
 * @author  Owen
 * @version V1.0.0 	initial release
 *
 * $Id$
 * $Created: 2018-11-29 17:31:10$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#ifndef SPIM_INTERFACE_H_
#define SPIM_INTERFACE_H_


/**
 * @brief
 *		DMA传输完成标志位清除
 * @param	PeripheralID	PERIPHERAL_ID_SPIM_RX接收DMA外设通道    PERIPHERAL_ID_SPIM_TX发送DMA外设通道
 * @return
 *		无
 */
void SPIM_DMA_Done_Clear(uint8_t PeripheralID);


/**
 * @brief
 *		查询全双工模式下数据接收发送是否完成
 * @param	无
 *  @return
 *		0传送未完成 1传送完成
 * @note
 */
bool SPIM_DMA_FullDone();
/**
 * @brief
 *		查询DMA模式下半双工模式下接收或发送完成状态
 * @param	PeripheralID	PERIPHERAL_ID_SPIM_RX接收 PERIPHERAL_ID_SPIM_TX发送
 * @return
 *		0数据传送未完成 1数据传送完成
 * @note
 */
bool SPIM_DMA_HalfDone(uint8_t PeripheralID);
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
void SPIM_DMA_Send_Recive_Start(uint8_t* SendBuf,uint8_t* RecvBuf, uint32_t Length);
/**
 * @brief
 *		SPIM使用DMA方式发送数据
 * @param	SendBuf		发送数据buffer
 * @param	Length		发送数据长度
 * @note
 */
void SPIM_DMA_Send_Start(uint8_t* SendBuf, uint32_t Length);
/**
 * @brief
 *		SPIM使用DMA方式接收数据
 * @param	RecvBuf		接收数据buffer
 * @param	Length		数据接收长度
 * @note
 */
void SPIM_DMA_Recv_Start(uint8_t* RecvBuf, uint32_t Length);


#endif /* SPIM_INTERFACE_H_ */
