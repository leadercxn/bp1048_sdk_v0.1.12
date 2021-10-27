/*
 ******************************************************************************
 * @file    uarts.h
 * @author  Robert
 * @version V1.0.0
 * @date    2014/09/23
 * @brief	UART(Universal Asynchronous Receiver Transmitter) is serial
 *			and duplex data switch upto 3Mbps high speed transmittion.It can
 *			support datum block or tiny slice datum transfer as general uart-like.
 *
 * Changelog:
 *			2014-09-23	Borrow from O18B Uart driver by Robert
 *			2015-05-16  Optimize code by Lilu
 *			2015-10-28  Optimize code by Jerry
 *			2017-11-02	Optimize code by Messi
 *******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2014 MVSilicon </center></h2>
 */
/**
 * @addtogroup UART
 * @{
 * @defgroup uarts uarts.h
 * @{
 */

#ifndef __UARTS_H__
#define __UARTS_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include "type.h"
/**
 * @brief      ���ڶ˿�
 */
typedef enum
{
	UART_PORT0 = 0,
	UART_PORT1,
}UART_PORT_T;
/**
 * @brief      ���ڿ���ָ�
 */
typedef enum
{
	//get UART baud rate
	UART_IOCTL_BAUDRATE_GET = 1,
	//set UART baud rate,ARG2=BAUDRATE,{1,200-3,000,000/bps}
	UART_IOCTL_BAUDRATE_SET,		

	//get UART RX interrupt trigger level,FIFO depth{1-4} in bytes represented by{0-3},ARG2=0
	UART_IOCTL_RXFIFO_TRGRDEP_GET = 0x05,
	//set UART RX interrupt trigger level,FIFO depth{1-4} in bytes represented by{0-3},ARG2={0-3}
	UART_IOCTL_RXFIFO_TRGRDEP_SET,

	//clear UART RX internal FIFO forcedly,ARG2=0
	UART_IOCTL_RXFIFO_CLR = 0x0A,
	UART_IOCTL_TXFIFO_CLR ,
    
	//dis/enable UART RX ,ARG2{1,0},1 for enable,0 for disable
	UART_IOCTL_DISENRX_SET = 0x10,
	//dis/enable UART TX ,ARG2{1,0},1 for enable,0 for disable
	UART_IOCTL_DISENTX_SET,

	//dis/enable UART RX interrupt,ARG2{1,0},1 for enable,0 for disable
	UART_IOCTL_RXINT_SET = 0x15,
	//dis/enable UART TX interrupt,ARG2{1,0},1 for enable,0 for disable
	UART_IOCTL_TXINT_SET,

	//clean UART RX data or error interrupt status
	UART_IOCTL_RXINT_CLR = 0x1A,
	//clean UART TX data interrupt status
	UART_IOCTL_TXINT_CLR,

	//get UART RX status register,5`bxxxxx,{frame_err,parity_err,overrun,dat_err,dat_int}
	UART_IOCTL_RXSTAT_GET = 0x20,
	//get F/BUUART TX status register,2`bxx,{tx_busy,dat_int}
	UART_IOCTL_TXSTAT_GET,

	//set/unset RX RTS flow control,ARG2={0 for unset,1 for low active,2 for high active}
	UART_IOCTL_RXRTS_FLOWCTL_SET = 0x25,
	//set/unset TX CTS flow control,ARG2={0 for unset,1 for low active,2 for high active}
	UART_IOCTL_TXCTS_FLOWCTL_SET,
	//set/unset RX RTS flow control by software,ARG2={0 disable RTS software control,1 enable RTS software control forcedly}
	UART_IOCTL_RXRTS_FLOWCTL_BYSW_SET,
    
    //get UART RX FIFO status register,2`bxx,{rx_fifo_full,rx_fifo_empty}
	UART_IOCTL_RXFIFO_STAT_GET = 0x2A,
	//get UART TX FIFO status register,2`bxx,{tx_fifo_full,tx_fifo_empty}
	UART_IOCTL_TXFIFO_STAT_GET,
    
	//get UART baud rate clock divider fraction part
	UART_IOCTL_CLKDIV_FRAC_GET = 0x30,
	//set UART baud rate clock divider fraction part
	UART_IOCTL_CLKDIV_FRAC_SET,

	//UART configuration reset
	UART_IOCTL_CONF_RESET = 0x35,
	//UART functionality reset
	UART_IOCTL_FUNC_RESET,
	UART_IOCTL_MODULE_RESET,
    
    //Enable Uart overtime interrupt
    UART_IOCTL_OVERTIME_SET,    
    //Disable Uart overtime interrupt
    UART_IOCTL_OVERTIME_CLR,    
    //Set Uart overtime count value, its unit is baud_rate.
    UART_IOCTL_OVERTIME_NUM,

	//castle added 20151106
	//Get if Tx/Rx FIFO is empty
	UART_IOCTL_TX_FIFO_EMPTY = 0x45,
	UART_IOCTL_RX_FIFO_EMPTY,

	UART_IOCTL_DMA_TX_EN,
	UART_IOCTL_DMA_RX_EN,

	UART_IOCTL_DMA_TX_GET_EN,
	UART_IOCTL_DMA_RX_GET_EN,
	
	//castle added 20160121
	UART_IOCTL_RX_ERR_INT_EN,
	UART_IOCTL_RX_ERR_CLR,
	UART_IOCTL_RX_ERR_INT_GET,
	
	UART_IOCTL_RXINT_GET,
	UART_IOCTL_TXINT_GET,

	UART_IOCTL_OVERTIME_GET,
} UART_IOCTL_CMD_T;


/**
 * @brief      ��ʼ������
 *			        ���û��������������ʣ�����λ��У��λ��ֹͣλ����
 * @param[in]  BaudRate   ���ڲ��������ã�DPLL@288MHz���� AUPLL@240MHzģʽ��:�����ʷ�Χ1200bps~3000000bps��RC@12M�����ʷ�Χ1200bps~115200bps
 * @param[in]  DataBits   ����λ����ѡ��֧��5-8bit�����������루5��6��7��8����
 * @param[in]  Parity     У��λѡ����������ֱ��Ӧ 0:��У�� 1:��У�� 2:żУ��
 * @param[in]  StopBits   ֹͣλѡ����������ֱ��Ӧ1��1λֹͣλ 2��2λֹͣλ
 * @return     			  FALSE��������	TRUE������ʼ�����
 */
bool UART0_Init(uint32_t BaudRate, uint8_t DataBits, uint8_t Parity, uint8_t StopBits);
bool UART1_Init(uint32_t BaudRate, uint8_t DataBits, uint8_t Parity, uint8_t StopBits);

/**
 * @brief      ���ڶ��ֽڽ��պ���
 *
 * @param[in]  RecvBuf  	�������ݻ�������ַ
 * @param[in]  BufLen   	�������ݻ������ֽڳ���
 * @param[in]  TimeOut   	��ʱ�˳�ʱ��
 * @return     ���յ������ݳ���
 */
uint32_t UART0_Recv(uint8_t* RecvBuf, uint32_t BufLen,uint32_t TimeOut);
uint32_t UART1_Recv(uint8_t* RecvBuf, uint32_t BufLen,uint32_t TimeOut);


/**
 * @brief      ���ڵ��ֽڽ��պ���
 *
 * @param[in]  Val  	�������ݴ�ŵ�ַ
 * @return     FALSEδ���յ�����	TRUE���յ�����
 */
bool UART0_RecvByte(uint8_t* Val);
bool UART1_RecvByte(uint8_t* Val);

/**
 * @brief      ���ڶ��ֽڷ��ͺ���
 *
 * @param[in]  SendBuf  	�������ݻ�������ַ
 * @param[in]  BufLen   	���������ֽڳ���
 * @param[in]  TimeOut   	��ʱ�˳�ʱ��
 * @return     ���ͳ���
 */
uint32_t UART0_Send(uint8_t* SendBuf, uint32_t BufLen,uint32_t TimeOut);
uint32_t UART1_Send(uint8_t* SendBuf, uint32_t BufLen,uint32_t TimeOut);

/**
 * @brief      ���ڵ��ֽڷ��ͺ���
 * @param[in]  SendByte  	�跢�͵�����
 * @return     �޷���ֵ
 */
void UART0_SendByte(uint8_t SendByte);
void UART1_SendByte(uint8_t SendByte);

/**
 * @brief      ���ڵ��ֽڷ���
 *             ע�����ͺ���Ҫ�ȴ������Ƿ���ɣ�����������ɺ󣬻ᷢ��������жϡ������жϺ�������Ҫע����TX DONE�жϣ�
 * @param[in]  SendByte  	�跢�͵�����
 * @return     �޷���ֵ
 */
void UART0_SendByte_In_Interrupt(uint8_t SendByte);
void UART1_SendByte_In_Interrupt(uint8_t SendByte);

/**
 * @brief      ���ڿ���λ��������
 *             ͨ��ѡ�񴮿ڿ���λCmd�Լ�����Ĳ���Arg������������û��߶�ȡ�������
 * @param[in]  Cmd  	����ָ��ѡ��ѡ��UART_IOCTL_CMD_Tö����
 * @param[in]  Arg  	ָ�����������Cmdָ��д����Ҫ�Ĳ���
 * @return     �ڶԿ���ָ������ɹ���ǰ���£����������������н����
 * 				��UART_IOCTL_TXSTAT_GET�Ȼ�ȡ״̬��������״ֵ̬
 * 				��UART_IOCTL_TXINT_SET�����ÿ���λ��������0�������ɹ�
 * 			   ����EINVAL��22�����Ƿ�����������Cmd��Arg�����Ƿ���ȷ
 */
int32_t UART0_IOCtl(UART_IOCTL_CMD_T Cmd, uint32_t Arg);
int32_t UART1_IOCtl(UART_IOCTL_CMD_T Cmd, uint32_t Arg);

#ifdef __cplusplus
}
#endif // __cplusplus 


#endif//__UARTS_H__

/**
 * @}
 * @}
 */

