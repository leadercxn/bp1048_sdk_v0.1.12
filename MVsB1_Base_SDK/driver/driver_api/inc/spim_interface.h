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
 *		DMA������ɱ�־λ���
 * @param	PeripheralID	PERIPHERAL_ID_SPIM_RX����DMA����ͨ��    PERIPHERAL_ID_SPIM_TX����DMA����ͨ��
 * @return
 *		��
 */
void SPIM_DMA_Done_Clear(uint8_t PeripheralID);


/**
 * @brief
 *		��ѯȫ˫��ģʽ�����ݽ��շ����Ƿ����
 * @param	��
 *  @return
 *		0����δ��� 1�������
 * @note
 */
bool SPIM_DMA_FullDone();
/**
 * @brief
 *		��ѯDMAģʽ�°�˫��ģʽ�½��ջ������״̬
 * @param	PeripheralID	PERIPHERAL_ID_SPIM_RX���� PERIPHERAL_ID_SPIM_TX����
 * @return
 *		0���ݴ���δ��� 1���ݴ������
 * @note
 */
bool SPIM_DMA_HalfDone(uint8_t PeripheralID);
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
void SPIM_DMA_Send_Recive_Start(uint8_t* SendBuf,uint8_t* RecvBuf, uint32_t Length);
/**
 * @brief
 *		SPIMʹ��DMA��ʽ��������
 * @param	SendBuf		��������buffer
 * @param	Length		�������ݳ���
 * @note
 */
void SPIM_DMA_Send_Start(uint8_t* SendBuf, uint32_t Length);
/**
 * @brief
 *		SPIMʹ��DMA��ʽ��������
 * @param	RecvBuf		��������buffer
 * @param	Length		���ݽ��ճ���
 * @note
 */
void SPIM_DMA_Recv_Start(uint8_t* RecvBuf, uint32_t Length);


#endif /* SPIM_INTERFACE_H_ */
