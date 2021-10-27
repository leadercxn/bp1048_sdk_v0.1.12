/**
 **************************************************************************************
 * @file    uarts_interface.h
 * @brief   uarts_interface
 *
 * @author  Sam
 * @version V1.1.0
 *
 * $Created: 2018-06-05 15:17:05$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __UARTS_INTERFACE_H__
#define __UARTS_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include "type.h"
#include "uarts.h"
#include "dma.h"

/*UART MCU mode*/
/**
 * @brief      ��ʼ�����ں���
 *             ���û��������������ʣ�����λ��У��λ��ֹͣλ����
 * @param[in]  UartNum    UARTӲ��ģ��ѡ��0:UART0 1:UART1
 * @param[in]  BaudRate   ���ڲ��������ã�DPLL@288MHz���� AUPLL@240MHzģʽ��:�����ʷ�Χ1200bps~3000000bps��RC@12M�����ʷ�Χ1200bps~115200bps
 * @param[in]  DataBits   ����λ����ѡ��֧��5-8bit�����������루5��6��7��8����
 * @param[in]  Parity     У��λѡ����������ֱ��Ӧ 0:��У�� 1:��У�� 2:żУ��
 * @param[in]  StopBits   ֹͣλѡ����������ֱ��Ӧ1��1λֹͣλ 2��2λֹͣλ
 * @return     			  FALSE��������	TRUE������ʼ�����
 */
#define UARTS_Init(UartNum,BaudRate,DataBits,Parity,StopBits)  (*UARTS_Init_MAP[UartNum])(BaudRate,DataBits,Parity,StopBits)

/**
 * @brief      ���ڿ���λ��������
 *             ͨ��ѡ�񴮿ڿ���λCmd�Լ�����Ĳ���Arg������������û��߶�ȡ�������
 * @param[in]  UartNum  UARTӲ��ģ��ѡ��0:UART0 1:UART1
 * @param[in]  Cmd  	����ָ��ѡ��ѡ��UART_IOCTL_CMD_Tö����
 * @param[in]  Arg  	ָ�����������Cmdָ��д����Ҫ�Ĳ���
 * @return     �ڶԿ���ָ������ɹ���ǰ���£����������������н����
 * 				��UART_IOCTL_TXSTAT_GET�Ȼ�ȡ״̬��������״ֵ̬
 * 				��UART_IOCTL_TXINT_SET�����ÿ���λ��������0�������ɹ�
 * 			   ����EINVAL��22�����Ƿ�����������Cmd��Arg�����Ƿ���ȷ
 */
#define UARTS_IOCTL(UartNum,cmd,Arg)  (*UARTS_IOCTL_MAP[UartNum])(cmd,Arg)

/**
 * @brief      ���ڶ��ֽڽ��պ���
 * @param[in]  UartNum      UARTӲ��ģ��ѡ��0:UART0 1:UART1
 * @param[in]  RecvBuf  	�������ݻ�������ַ
 * @param[in]  BufLen   	�������ݻ������ֽڳ���
 * @param[in]  TimeOut   	��ʱ�˳�ʱ��
 * @return     ���յ������ݳ���
 */
#define UARTS_Recv(UartNum, RecvBuf, BufLen,TimeOut) (*UARTS_Recv_MAP[UartNum])(RecvBuf, BufLen,TimeOut)

/**
 * @brief      ���ڵ��ֽڽ��պ���
 * @param[in]  UartNum  UARTӲ��ģ��ѡ��0:UART0 1:UART1
 * @param[in]  Val  	�������ݴ�ŵ�ַ
 * @return     FALSE    δ���յ�����	TRUE���յ�����
 */
#define UARTS_RecvByte(UartNum, Val) (*UARTS_RecvByte_MAP[UartNum])(Val)

/**
 * @brief      ���ڶ��ֽڷ��ͺ���
 * @param[in]  UartNum      UARTӲ��ģ��ѡ��0:UART0 1:UART1
 * @param[in]  SendBuf  	�������ݻ�������ַ
 * @param[in]  BufLen   	���������ֽڳ���
 * @param[in]  TimeOut   	��ʱ�˳�ʱ��
 * @return     ���ͳ���
 */
#define UARTS_Send(UartNum, SendBuf, BufLen ,TimeOut) (*UARTS_Send_MAP[UartNum])(SendBuf, BufLen, TimeOut)

/**
 * @brief      ���ڵ��ֽڷ��ͺ���
 * @param[in]  UartNum      UARTӲ��ģ��ѡ��0:UART0 1:UART1
 * @param[in]  SendByte  	�跢�͵�����
 * @return     �޷���ֵ
 */
#define UARTS_SendByte(UartNum, SendByte) (*UARTS_SendByte_MAP[UartNum])(SendByte)

/**
 * @brief      ���ڵ��ֽڷ���
 *             ע�����ͺ���Ҫ�ȴ������Ƿ���ɣ�����������ɺ󣬻ᷢ��������жϡ������жϺ�������Ҫע����TX DONE�жϣ�
 * @param[in]  UartNum      UARTӲ��ģ��ѡ��0:UART0 1:UART1
 * @param[in]  SendByte  	�跢�͵�����
 * @return     �޷���ֵ
 */
#define UARTS_SendByte_In_Interrupt(UartNum, SendByte) (*UARTS_SendByte_In_Interrupt_MAP[UartNum])(SendByte)

/*UART DMA mode*/

/**
 * @brief      UART DMA ����ģʽ��ʼ�������û������������ջ����ַ�����ջ��泤�ȣ��жϴ����ż�ֵ���ص�����
 * @param[in]  RxBufAddr  	���ջ�������ַ
 * @param[in]  RxBufLen   	���ջ�������������
 * @param[in]  ThresholdLen �жϴ����ż�ֵ���ã�ע������ֵС��RxBufLen
 * @param[in]  CallBack   	�жϻص�����
 * @return     FALSE��ʼ������ʧ��	TRUE��ʼ�����óɹ�
 */
bool UART0_DMA_RxInit(void* RxBufAddr, uint16_t RxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
bool UART1_DMA_RxInit(void* RxBufAddr, uint16_t RxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
#define UARTS_DMA_RxInit(UartNum, RxBufAddr, RxBufLen, ThresholdLen, CallBack) (*UARTS_DMA_RxInit_MAP[UartNum])(RxBufAddr, RxBufLen, ThresholdLen, CallBack)

/**
 * @brief      UART DMA ����ģʽ��ʼ�������û������������ͻ����ַ�����ͻ��泤�ȣ��жϴ����ż�ֵ���ص�����
 * @param[in]  TxBufAddr  	���ͻ�������ַ
 * @param[in]  TxBufLen   	���ͻ�������������
 * @param[in]  ThresholdLen �жϴ����ż�ֵ���ã�ע������ֵС��RxBufLen
 * @param[in]  CallBack   	�жϻص�����
 * @return     FALSE��ʼ������ʧ��	TRUE��ʼ�����óɹ�
 */
bool UART0_DMA_TxInit(void* TxBufAddr, uint16_t TxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
bool UART1_DMA_TxInit(void* TxBufAddr, uint16_t TxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
#define UARTS_DMA_TxInit(UartNum, TxBufAddr, TxBufLen, ThresholdLen, CallBack) (*UARTS_DMA_TxInit_MAP[UartNum])(TxBufAddr, TxBufLen, ThresholdLen, CallBack)

/**
 * @brief      UART DMA ����ģʽ��ʼ�������ͻ�������ַ�����ͻ��泤�ȣ��жϴ����ż�ֵ���ص�����
 * @param[in]  TxBufAddr  	���ͻ�������ַ
 * @param[in]  TxBufLen   	�����ַ�����
 * @param[in]  TimeOut 		��ʱʱ�����ã����ﵽ���õĳ�ʱʱ�仹δ������ɣ����������
 * @return     ʵ�ʷ������ݳ���
 */
uint32_t UART0_DMA_Send(uint8_t* SendBuf, uint16_t BufLen, uint32_t TimeOut);
uint32_t UART1_DMA_Send(uint8_t* SendBuf, uint16_t BufLen, uint32_t TimeOut);
#define UARTS_DMA_Send(UartNum, SendBuf, BufLen, TimeOut) (*UARTS_DMA_Send_MAP[UartNum])(SendBuf, BufLen, TimeOut)

/**
 * @brief      UART DMA �������ݣ����ý��ջ����ַ�������ַ����ȣ����ճ�ʱ����
 * @param[in]  RecvBuf  	���ջ�������ַ
 * @param[in]  BufLen   	���ճ�������
 * @param[in]  TimeOut		��ʱʱ�����ã��涨ʱ���ڽ���û�дﵽԤ�ڳ��Ȼ���û�յ��վ����������
 * @return     ���յ����ݵ�ʵ�ʳ���
 */
uint32_t UART0_DMA_Recv(uint8_t* RecvBuf, uint16_t BufLen, uint32_t TimeOut);
uint32_t UART1_DMA_Recv(uint8_t* RecvBuf, uint16_t BufLen, uint32_t TimeOut);
#define UARTS_DMA_Recv(UartNum, RecvBuf, BufLen, TimeOut) (*UARTS_DMA_Recv_MAP[UartNum])(RecvBuf, BufLen, TimeOut)

/**
 * @brief      UART DMA �������ݣ����ô��봫�ͻ�������ַ�������ַ�����
 * @param[in]  RecvBuf  	���ͻ�������ַ
 * @param[in]  BufLen   	�����ַ���������
 * @return     �޷���ֵ
 */
void UART0_DMA_SendDataStart(uint8_t* SendBuf, uint16_t BufLen);
void UART1_DMA_SendDataStart(uint8_t* SendBuf, uint16_t BufLen);
#define UARTS_DMA_SendDataStart(UartNum, SendBuf, BufLen) (*UARTS_DMA_SendDataStart_MAP[UartNum])(SendBuf, BufLen)

/**
 * @brief      �ж����ݴ����Ƿ����
 * @return     TURE������� 	 FALSE����δ���
  */
bool UART0_DMA_TxIsTransferDone(void);
bool UART1_DMA_TxIsTransferDone(void);
#define UARTS_DMA_TxIsTransferDone(UartNum) (*UARTS_DMA_TxIsTransferDone_MAP[UartNum])()

/**
 * @brief      UART DMA 	��������ʽʹ�ܽ��գ����ý��ջ����ַ�������ַ�����
 * @param[in]  RecvBuf  	���ջ�������ַ
 * @param[in]  BufLen   	���ճ�������
 * @return     ���յ����ݵ�ʵ�ʳ���
 */
int32_t UART0_DMA_RecvDataStart(uint8_t* RecvBuf, uint16_t BufLen);
int32_t UART1_DMA_RecvDataStart(uint8_t* RecvBuf, uint16_t BufLen);
#define UARTS_DMA_RecvDataStart(UartNum, RecvBuf, BufLen) (*UARTS_DMA_RecvDataStart_MAP[UartNum])(RecvBuf, BufLen)

/**
 * @brief      �ж��Ƿ������
 * @return     TURE������� 	 FALSE����δ���
  */
bool UART0_DMA_RxIsTransferDone(void);
bool UART1_DMA_RxIsTransferDone(void);
#define UARTS_DMA_RxIsTransferDone(UartNum) (*UARTS_DMA_RxIsTransferDone_MAP[UartNum])()

/**
 * @brief      ��ѯDMA���ջ��������ݳ���
 * @return     �������ݳ���
  */
int32_t UART0_DMA_RxDataLen(void);
int32_t UART1_DMA_RxDataLen(void);
#define UARTS_DMA_RxDataLen(UartNum) (*UARTS_DMA_RxDataLen_MAP[UartNum])()

/**
 * @brief      ��ѯDMA���ͻ��������ݳ���
 * @return     �������ݳ���
  */
int32_t UART0_DMA_TxDataLen(void);
int32_t UART1_DMA_TxDataLen(void);
#define UARTS_DMA_TxDataLen(UartNum) (*UARTS_DMA_TxDataLen_MAP[UartNum])()

/**
 * @brief      ע��UART_DMAģʽ�µ��жϻص�������ע�������ж�����
 * @param[in]  IntType  	�ж���������: 1.DMA_DONE_INT���ж�    2.DMA_THRESHOLD_INT��ֵ�����ж�    3.DMA_ERROR_INT�����ж�
 * param[in]   CallBack		��ע����жϻص���������д��NULL����رջص�����
 * @return     �������ݳ���
  */
void UART0_DMA_RxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack);
void UART1_DMA_RxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack);
#define UARTS_DMA_RxSetInterruptFun(UartNum, IntType, CallBack) (*UARTS_DMA_RxSetInterruptFun_MAP[UartNum])(IntType, CallBack)
void UART0_DMA_TxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack);
void UART1_DMA_TxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack);
#define UARTS_DMA_TxSetInterruptFun(UartNum, IntType, CallBack) (*UARTS_DMA_TxSetInterruptFun_MAP[UartNum])(IntType, CallBack)


/**
 * @brief      ע��UART_DMAģʽ�µĴ�����ɵ��жϻص�������ע�������ж�����
 *             ע���ú�����ͬ�ڣ�����ĺ������������->�ж���������	DMA_DONE_INT
 * param[in]   CallBack		��ע����жϻص���������д��NULL����رջص�����
 * @return     �������ݳ���
  */
void UART0_DMA_TxSetTransmitDoneFun(INT_FUNC CallBack);
void UART1_DMA_TxSetTransmitDoneFun(INT_FUNC CallBack);
#define UARTS_DMA_TxSetTransmitDoneFun(UartNum, CallBack) (*UARTS_DMA_TxSetTransmitDoneFun_MAP[UartNum])(CallBack)
void UART0_DMA_RxSetTransmitDoneFun(INT_FUNC CallBack);
void UART1_DMA_RxSetTransmitDoneFun(INT_FUNC CallBack);
#define UARTS_DMA_RxSetTransmitDoneFun(UartNum, CallBack) (*UARTS_DMA_RxSetTransmitDoneFun_MAP[UartNum])(CallBack)

/**
 * @brief      UART_DMAʹ�ܺ���
 * @param[in]
 * @return
  */
void UART0_DMA_RxChannelEn(void);
void UART1_DMA_RxChannelEn(void);
#define UARTS_DMA_RxChannelEn(UartNum) (*UARTS_DMA_RxChannelEn_MAP[UartNum])()
void UART0_DMA_TxChannelEn(void);
void UART1_DMA_TxChannelEn(void);
#define UARTS_DMA_TxChannelEn(UartNum) (*UARTS_DMA_TxChannelEn_MAP[UartNum])()

/**
 * @brief      UART_DMA���ú���
 * @param[in]
 * @return
  */
void UART0_DMA_RxChannelDisable(void);
void UART1_DMA_RxChannelDisable(void);
#define UARTS_DMA_RxChannelDisable(UartNum) (*UARTS_DMA_RxChannelDisable_MAP[UartNum])()
void UART0_DMA_TxChannelDisable(void);
void UART1_DMA_TxChannelDisable(void);
#define UARTS_DMA_TxChannelDisable(UartNum) (*UARTS_DMA_TxChannelDisable_MAP[UartNum])()

/**
 * @brief      UART_DMA �жϱ�־λ���
 * @param[in]   IntType  	�ж���������	DMA_DONE_INT���ж�DMA_THRESHOLD_INT��ֵ�����ж�DMA_ERROR_INT�����ж�
 * @return  	DMA_ERROR
 * @note	�жϱ�־�ڲ������жϵ������Ҳ����λ
  */
int32_t UART0_DMA_TxIntFlgClr(DMA_INT_TYPE IntType);
int32_t UART1_DMA_TxIntFlgClr(DMA_INT_TYPE IntType);
#define UARTS_DMA_TxIntFlgClr(UartNum, IntType) (*UARTS_DMA_TxIntFlgClr_MAP[UartNum])(IntType)
//RX flag clear
int32_t UART0_DMA_RxIntFlgClr(DMA_INT_TYPE IntType);
int32_t UART1_DMA_RxIntFlgClr(DMA_INT_TYPE IntType);
#define UARTS_DMA_RxIntFlgClr(UartNum, IntType) (*UARTS_DMA_RxIntFlgClr_MAP[UartNum])(IntType)
//add new APIs here...

/*UART MCU MAP*/
bool (*UARTS_Init_MAP[2])(uint32_t BaudRate, uint8_t DataBits, uint8_t Parity, uint8_t StopBits);
uint32_t (*UARTS_Recv_MAP[2])(uint8_t* RecvBuf, uint32_t BufLen,uint32_t TimeOut);
bool (*UARTS_RecvByte_MAP[2])(uint8_t* Val);
uint32_t (*UARTS_Send_MAP[2])(uint8_t* SendBuf, uint32_t BufLen,uint32_t TimeOut);
void (*UARTS_SendByte_MAP[2])(uint8_t SendByte);
void (*UARTS_SendByte_In_Interrupt_MAP[2])(uint8_t SendByte);
int32_t (*UARTS_IOCTL_MAP[2])(UART_IOCTL_CMD_T Cmd, uint32_t Arg);

/*UART DMA MAP*/
bool (*UARTS_DMA_RxInit_MAP[2])(void* RxBufAddr, uint16_t RxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
bool (*UARTS_DMA_TxInit_MAP[2])(void* TxBufAddr, uint16_t TxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
uint32_t (*UARTS_DMA_Send_MAP[2])(uint8_t* SendBuf, uint16_t BufLen, uint32_t TimeOut);
uint32_t (*UARTS_DMA_Recv_MAP[2])(uint8_t* RecvBuf, uint16_t BufLen, uint32_t TimeOut);
void (*UARTS_DMA_SendDataStart_MAP[2])(uint8_t* SendBuf, uint16_t BufLen);
bool (*UARTS_DMA_TxIsTransferDone_MAP[2])(void);
int32_t (*UARTS_DMA_RecvDataStart_MAP[2])(uint8_t* RecvBuf, uint16_t BufLen);
bool (*UARTS_DMA_RxIsTransferDone_MAP[2])(void);
int32_t (*UARTS_DMA_RxDataLen_MAP[2])(void);
int32_t (*UARTS_DMA_TxDataLen_MAP[2])(void);
void (*UARTS_DMA_RxSetInterruptFun_MAP[2])(DMA_INT_TYPE IntType, INT_FUNC CallBack);
void (*UARTS_DMA_TxSetInterruptFun_MAP[2])(DMA_INT_TYPE IntType, INT_FUNC CallBack);
void (*UARTS_DMA_RxChannelEn_MAP[2])(void);
void (*UARTS_DMA_TxChannelEn_MAP[2])(void);
void (*UARTS_DMA_RxChannelDisable_MAP[2])(void);
void (*UARTS_DMA_TxChannelDisable_MAP[2])(void);
int32_t (*UARTS_DMA_TxIntFlgClr_MAP[2])(DMA_INT_TYPE IntType);
int32_t (*UARTS_DMA_RxIntFlgClr_MAP[2])(DMA_INT_TYPE IntType);
void (*UARTS_DMA_TxSetTransmitDoneFun_MAP[2])(INT_FUNC CallBack);
void (*UARTS_DMA_RxSetTransmitDoneFun_MAP[2])(INT_FUNC CallBack);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif//__UARTS_INTERFACE_H__
