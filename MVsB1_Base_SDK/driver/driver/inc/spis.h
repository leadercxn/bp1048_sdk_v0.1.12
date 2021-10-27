/**
  *****************************************************************************
  * @file:			spis.h
  * @author			grayson
  * @version		V1.0.0
  * @data			18-June-2017
  * @Brief			SPI Slave driver header file.
  * @note			For sdio and spi can't access memory simultaneously
  ******************************************************************************
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
  * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
  */
  
/**
 * @addtogroup SPIS
 * @{
 * @defgroup spis spis.h
 * @{
 */
#ifndef __SPIS_H__
#define __SPIS_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus



#define	SPIS_PORT0_A0A1A2A3			0
#define	SPIS_PORT1_A20A21A22A23		1


/**
 * @brief
 *    SPIS����IO��ʼ��
 * @param	PortSel
 *					0 - SPIS_PORT0_A0A1A2A3
 *					1 - SPIS_PORT1_A20A21A22A23
 * @return
 *   ��
 * @note
 */
void SPIS_IoConfig(uint8_t PortSel);
/**
 * @brief
 *    ��ʼ��SPISģ��
 * @param	ClkMode
 *					0 - CPOL = 0 & CPHA = 0, idleΪ�͵�ƽ,��һ�����ز���(������)
 *					1 - CPOL = 0 & CPHA = 1, idleΪ�͵�ƽ,�ڶ������ز���(�½���)
 *					2 - CPOL = 1 & CPHA = 0, idleΪ�ߵ�ƽ,��һ�����ز���(�½���)
 *					3 - CPOL = 1 & CPHA = 1, idleΪ�ߵ�ƽ,��һ�����ز���(������)
 * @return
 *   ��
 * @note
 */
void SPIS_Init(uint8_t ClkMode);


//--------------------------------MCU MODE-------------------------------------
/**
 * @brief
 *    MCU��ʽ�������ݣ���TimeOutʱ���ڵȴ�������ɣ���ʱ����ʵ�ʷ������ݸ�������λByte
 * @param	SendBuf			�����׵�ַ
 * @param	BufLen			�������ݳ���
 * @param	TimeOut			��ʱʱ��
 *
 * @return
 *   ����ʵ�ʷ������ݳ���
 * @note
 */
uint32_t SPIS_Send(uint8_t* SendBuf, uint32_t BufLen, uint32_t TimeOut);

/**
 * @brief
 *    MCU��ʽ�������ݣ���TimeOutʱ���ڵȴ�������ɣ���ʱ����ʵ�ʽ������ݸ�������λByte
 * @param	SendBuf			�����׵�ַ
 * @param	BufLen			�������ݳ���
 * @param	TimeOut			��ʱʱ��
 *
 * @return
 *   ����ʵ�ʽ������ݳ���
 * @note
 */
uint32_t SPIS_Recv(uint8_t* RecvBuf, uint32_t BufLen, uint32_t TimeOut);

/**
 * @brief
 *    MCU��ʽ����1�ֽ�����
 * @param	SendBuf			���ֽ�����
 *
 * @return
 *   ��
 * @note
 */
void SPIS_SendByte(uint8_t SendByte);

/**
 * @brief
 *    MCU��ʽ����1�ֽ�����
 * @param	SendBuf			���ֽ�����
 *
 * @return
 *   ��
 * @note
 */
uint8_t SPIS_RecvByte(void);

//--------------------------------DMA MODE-------------------------------------
/**
 * @brief
 *    ��ʼ��SPIS DMA����
 * @param	RxBufferAddr  ���ջ������׵�ַ
 * @param	RxBufferLen   ���ջ���������
 * @param	TxBufferAddr  ���ͻ������׵�ַ
 * @param   TxBufferLen   ���ͻ���������
 * @return
 *   TRUE   �ɹ���  FALSE ʧ��
 * @note
 */
bool SPIS_DMA_Init(void* RxBufferAddr, uint16_t RxBufferLen, void* TxBufferAddr, uint16_t TxBufferLen);


/**
 * @brief
 *    DMA��ʽ�������ݣ���FIFO���������ݣ���TimeOutʱ���ڵȴ�������ɣ���ʱ����ʵ�ʷ������ݸ�������λByte
 * @param	SendBuf			�����׵�ַ
 * @param	BufLen			�������ݳ���
 * @param	TimeOut			��ʱʱ��
 *
 * @return
 *   ����ʵ�ʷ������ݳ���
 * @note
 */
uint32_t SPIS_DMA_Send(uint8_t* SendBuf, uint32_t BufLen, uint32_t TimeOut);

/**
 * @brief
 *    DMA��ʽ�������ݣ���FIFO��ȡ�����ݣ���TimeOutʱ���ڵȴ�������ɣ���ʱ����ʵ�ʽ������ݸ�������λByte
 * @param	SendBuf			�����׵�ַ
 * @param	BufLen			�������ݳ���
 * @param	TimeOut			��ʱʱ��
 *
 * @return
 *   ����ʵ�ʽ������ݳ���
 * @note
 */
uint32_t SPIS_DMA_Recv(uint8_t* RecvBuf, uint32_t BufLen, uint32_t TimeOut);

/**
 * @brief
 *    �����ж�ʹ��
 * @param	En			1�������жϣ�0���ر��ж�
 *
 * @return
 *   ��
 * @note
 *	 ÿ����һ��Byte����һ���ж�
 */
void SPIS_RxIntEn(bool En);

/**
 * @brief
 *    ��ȡ�����жϱ�־
 * @param	
 *
 * @return
 *   ���ؽ����жϱ�־
 * @note
 *	 ÿ����һ��Byte����һ���ж�
 */
bool SPIS_GetRxIntFlag(void);

/**
 * @brief
 *   ��������жϱ�־
 * @param	
 *
 * @return
 *   ��
 * @note
 *	 ÿ����һ��Byte����һ���ж�
 */
void SPIS_ClrRxIntFlag(void);

/**
 * @brief
 *    ��ȡ���������־
 * @param	
 *
 * @return
 *   ���ؽ��������־
 * @note
 *	 SPIS �ڲ�4Byte Ӳ��Rx FIFO�����ٴ��յ�һ��Byte������������־
 */
bool SPIS_GetRxOverrunFlag(void);

/**
 * @brief
 *    ������������־
 * @param	
 *
 * @return
 *   ��
 * @note
 *	 SPIS �ڲ�4Byte Ӳ��Rx FIFO�����ٴ��յ�һ��Byte������������־
 */
void SPIS_ClrRxOverrunFlag(void);


/**
 * @brief
 *    DMA��ʽ��������ģʽ����
 * @param	1--ʹ��DMA�������ݣ�
 *          0--��ʹ��DMA��������
 * @return
 *   ��
 * @note
 *
 */
void SPIS_RxDmaModeSet(bool EN);

/**
 * @brief
 *    LSB/MSB��������
 * @param	1--LSB���д���
 *          0--MSB���д���
 * @return
 *   ��
 * @note
 *   ��SPIS_Init()֮�����ã�����������Ĭ��MSB���д���
 */
void SPIS_LSBFirstSet(bool EN);

/**
 * @brief
 *   ��ս��ջ�����FIFO
 * @param
 *   ��
 * @return
 *   ��
 * @note
 *
 */
void SPIS_ClrRxFIFO(void);
/**
 * @brief
 *   ��շ��ͻ�����FIFO
 * @param
 *   ��
 * @return
 *   ��
 * @note
 *
 */
void SPIS_ClrTxFIFO(void);



//���
/**
 * @brief
 *   ��ʼ��SPIS�շ���BlockDMAģʽ
 * @param
 *   ��
 * @return
 *   ��
 * @note
 *
 */
bool SPIS_BlockDMA_Init();
/**
 * @brief
 *   SPIS����DMA��ʽ��������
 * @param
 *   SendBuf���ͻ����ַ
 *   BufLen�������ݳ��ȣ�byte��
 * @return
 *   ��
 * @note
 *
 */
void SPIS_DMA_StartSend(uint8_t* SendBuf, uint32_t BufLen);
/**
 * @brief
 *   ��ѯSPISʹ��DMA��ʽ��������״̬
 * @param
 * ��
 * @return
 *   0��û�в���DMA���ݴ�����ɱ�־     1������DMA���ݴ�����ɱ�־
 * @note
 *
 */
bool SPIS_DMA_SendState();
/**
 * @brief
 *   SPIS����DMA��ʽ��������
 * @param
 *   RecvBuf���ջ����ַ
 *   BufLen�������ݳ��ȣ�byte��
 * @return
 *   ��
 * @note
 *
 */
void SPIS_DMA_StartRecv(uint8_t* RecvBuf, uint32_t BufLen);
/**
 * @brief
 *   ��ѯSPISʹ��DMA��ʽ��������״̬
 * @param
 * ��
 * @return
 *   0��û�в���DMA���ݽ�����ɱ�־  1������DMA���ݽ�����ɱ�־
 * @note
 *
 */
bool SPIS_DMA_RecvState();
/**
 * @brief
 *   	SPIS����DMA��ʽ�շ����ݣ������������뷢������ͬʱ����
 * @param
 * 		SendBuf�������ݵ�ַ
 * 		RecvBuf�������ݴ�ŵ�ַ
 * 		BufLen���ݳ���
 * @return
 *   0��û�в���DMA���ݽ�����ɱ�־  1������DMA���ݽ�����ɱ�־
 * @note
 *
 */
void SPIS_DMA_StartSendRecv(uint8_t* SendBuf,uint8_t* RecvBuf, uint32_t BufLen);
/**
 * @brief
 *   ��ѯSPISʹ��DMA��ʽ���պͷ�������״̬
 * @param
 * ��
 * @return
 *   0��û�в���DMA���ݽ�����ɱ�־  1������DMA���ݽ��պͷ�����ɱ�־
 * @note
 *
 */
bool SPIS_DMA_SendRecvState();

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__SPIS_H__

/**
 * @}
 * @}
 */

