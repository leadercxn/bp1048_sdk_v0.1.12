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
 * @brief      串口端口
 */
typedef enum
{
	UART_PORT0 = 0,
	UART_PORT1,
}UART_PORT_T;
/**
 * @brief      串口控制指令集
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
 * @brief      初始化串口
 *			        配置基础参数，波特率，数据位，校验位，停止位配置
 * @param[in]  BaudRate   串口波特率配置，DPLL@288MHz或者 AUPLL@240MHz模式下:波特率范围1200bps~3000000bps；RC@12M波特率范围1200bps~115200bps
 * @param[in]  DataBits   数据位长度选择，支持5-8bit，及参数输入（5，6，7，8）；
 * @param[in]  Parity     校验位选择，输入参数分别对应 0:无校验 1:奇校验 2:偶校验
 * @param[in]  StopBits   停止位选择，输入参数分别对应1：1位停止位 2：2位停止位
 * @return     			  FALSE参数错误	TRUE参数初始化完成
 */
bool UART0_Init(uint32_t BaudRate, uint8_t DataBits, uint8_t Parity, uint8_t StopBits);
bool UART1_Init(uint32_t BaudRate, uint8_t DataBits, uint8_t Parity, uint8_t StopBits);

/**
 * @brief      串口多字节接收函数
 *
 * @param[in]  RecvBuf  	接收数据缓冲区地址
 * @param[in]  BufLen   	接收数据缓冲区字节长度
 * @param[in]  TimeOut   	超时退出时间
 * @return     接收到的数据长度
 */
uint32_t UART0_Recv(uint8_t* RecvBuf, uint32_t BufLen,uint32_t TimeOut);
uint32_t UART1_Recv(uint8_t* RecvBuf, uint32_t BufLen,uint32_t TimeOut);


/**
 * @brief      串口单字节接收函数
 *
 * @param[in]  Val  	接收数据存放地址
 * @return     FALSE未接收到数据	TRUE接收到数据
 */
bool UART0_RecvByte(uint8_t* Val);
bool UART1_RecvByte(uint8_t* Val);

/**
 * @brief      串口多字节发送函数
 *
 * @param[in]  SendBuf  	发送数据缓冲区地址
 * @param[in]  BufLen   	发送数据字节长度
 * @param[in]  TimeOut   	超时退出时间
 * @return     发送长度
 */
uint32_t UART0_Send(uint8_t* SendBuf, uint32_t BufLen,uint32_t TimeOut);
uint32_t UART1_Send(uint8_t* SendBuf, uint32_t BufLen,uint32_t TimeOut);

/**
 * @brief      串口单字节发送函数
 * @param[in]  SendByte  	需发送的数据
 * @return     无返回值
 */
void UART0_SendByte(uint8_t SendByte);
void UART1_SendByte(uint8_t SendByte);

/**
 * @brief      串口单字节发送
 *             注：发送后不需要等待发送是否完成，而当发送完成后，会发起发送完成中断。（在中断函数中需要注意清TX DONE中断）
 * @param[in]  SendByte  	需发送的数据
 * @return     无返回值
 */
void UART0_SendByte_In_Interrupt(uint8_t SendByte);
void UART1_SendByte_In_Interrupt(uint8_t SendByte);

/**
 * @brief      串口控制位操作函数
 *             通过选择串口控制位Cmd以及传入的参数Arg来设置相关配置或者读取相关配置
 * @param[in]  Cmd  	控制指令选择，选择UART_IOCTL_CMD_T枚举量
 * @param[in]  Arg  	指令参数，根据Cmd指令写入需要的参数
 * @return     在对控制指令操作成功的前提下，会根据情况返回下列结果：
 * 				如UART_IOCTL_TXSTAT_GET等获取状态：即返回状态值
 * 				如UART_IOCTL_TXINT_SET等设置控制位，即返回0，操作成功
 * 			   返回EINVAL（22）即非法操作，请检查Cmd与Arg参数是否正确
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

