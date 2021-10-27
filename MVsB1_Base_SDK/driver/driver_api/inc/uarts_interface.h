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
 * @brief      初始化串口函数
 *             配置基础参数，波特率，数据位，校验位，停止位配置
 * @param[in]  UartNum    UART硬件模块选择，0:UART0 1:UART1
 * @param[in]  BaudRate   串口波特率配置，DPLL@288MHz或者 AUPLL@240MHz模式下:波特率范围1200bps~3000000bps；RC@12M波特率范围1200bps~115200bps
 * @param[in]  DataBits   数据位长度选择，支持5-8bit，及参数输入（5，6，7，8）；
 * @param[in]  Parity     校验位选择，输入参数分别对应 0:无校验 1:奇校验 2:偶校验
 * @param[in]  StopBits   停止位选择，输入参数分别对应1：1位停止位 2：2位停止位
 * @return     			  FALSE参数错误	TRUE参数初始化完成
 */
#define UARTS_Init(UartNum,BaudRate,DataBits,Parity,StopBits)  (*UARTS_Init_MAP[UartNum])(BaudRate,DataBits,Parity,StopBits)

/**
 * @brief      串口控制位操作函数
 *             通过选择串口控制位Cmd以及传入的参数Arg来设置相关配置或者读取相关配置
 * @param[in]  UartNum  UART硬件模块选择，0:UART0 1:UART1
 * @param[in]  Cmd  	控制指令选择，选择UART_IOCTL_CMD_T枚举量
 * @param[in]  Arg  	指令参数，根据Cmd指令写入需要的参数
 * @return     在对控制指令操作成功的前提下，会根据情况返回下列结果：
 * 				如UART_IOCTL_TXSTAT_GET等获取状态：即返回状态值
 * 				如UART_IOCTL_TXINT_SET等设置控制位，即返回0，操作成功
 * 			   返回EINVAL（22）即非法操作，请检查Cmd与Arg参数是否正确
 */
#define UARTS_IOCTL(UartNum,cmd,Arg)  (*UARTS_IOCTL_MAP[UartNum])(cmd,Arg)

/**
 * @brief      串口多字节接收函数
 * @param[in]  UartNum      UART硬件模块选择，0:UART0 1:UART1
 * @param[in]  RecvBuf  	接收数据缓冲区地址
 * @param[in]  BufLen   	接收数据缓冲区字节长度
 * @param[in]  TimeOut   	超时退出时间
 * @return     接收到的数据长度
 */
#define UARTS_Recv(UartNum, RecvBuf, BufLen,TimeOut) (*UARTS_Recv_MAP[UartNum])(RecvBuf, BufLen,TimeOut)

/**
 * @brief      串口单字节接收函数
 * @param[in]  UartNum  UART硬件模块选择，0:UART0 1:UART1
 * @param[in]  Val  	接收数据存放地址
 * @return     FALSE    未接收到数据	TRUE接收到数据
 */
#define UARTS_RecvByte(UartNum, Val) (*UARTS_RecvByte_MAP[UartNum])(Val)

/**
 * @brief      串口多字节发送函数
 * @param[in]  UartNum      UART硬件模块选择，0:UART0 1:UART1
 * @param[in]  SendBuf  	发送数据缓冲区地址
 * @param[in]  BufLen   	发送数据字节长度
 * @param[in]  TimeOut   	超时退出时间
 * @return     发送长度
 */
#define UARTS_Send(UartNum, SendBuf, BufLen ,TimeOut) (*UARTS_Send_MAP[UartNum])(SendBuf, BufLen, TimeOut)

/**
 * @brief      串口单字节发送函数
 * @param[in]  UartNum      UART硬件模块选择，0:UART0 1:UART1
 * @param[in]  SendByte  	需发送的数据
 * @return     无返回值
 */
#define UARTS_SendByte(UartNum, SendByte) (*UARTS_SendByte_MAP[UartNum])(SendByte)

/**
 * @brief      串口单字节发送
 *             注：发送后不需要等待发送是否完成，而当发送完成后，会发起发送完成中断。（在中断函数中需要注意清TX DONE中断）
 * @param[in]  UartNum      UART硬件模块选择，0:UART0 1:UART1
 * @param[in]  SendByte  	需发送的数据
 * @return     无返回值
 */
#define UARTS_SendByte_In_Interrupt(UartNum, SendByte) (*UARTS_SendByte_In_Interrupt_MAP[UartNum])(SendByte)

/*UART DMA mode*/

/**
 * @brief      UART DMA 接收模式初始化，配置基础参数，接收缓存地址，接收缓存长度，中断触发门槛值，回调函数
 * @param[in]  RxBufAddr  	接收缓冲区地址
 * @param[in]  RxBufLen   	接收缓存区长度设置
 * @param[in]  ThresholdLen 中断触发门槛值设置，注意设置值小于RxBufLen
 * @param[in]  CallBack   	中断回调函数
 * @return     FALSE初始化配置失败	TRUE初始化设置成功
 */
bool UART0_DMA_RxInit(void* RxBufAddr, uint16_t RxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
bool UART1_DMA_RxInit(void* RxBufAddr, uint16_t RxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
#define UARTS_DMA_RxInit(UartNum, RxBufAddr, RxBufLen, ThresholdLen, CallBack) (*UARTS_DMA_RxInit_MAP[UartNum])(RxBufAddr, RxBufLen, ThresholdLen, CallBack)

/**
 * @brief      UART DMA 传送模式初始化，配置基础参数，发送缓存地址，发送缓存长度，中断触发门槛值，回调函数
 * @param[in]  TxBufAddr  	传送缓冲区地址
 * @param[in]  TxBufLen   	传送缓存区长度设置
 * @param[in]  ThresholdLen 中断触发门槛值设置，注意设置值小于RxBufLen
 * @param[in]  CallBack   	中断回调函数
 * @return     FALSE初始化配置失败	TRUE初始化设置成功
 */
bool UART0_DMA_TxInit(void* TxBufAddr, uint16_t TxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
bool UART1_DMA_TxInit(void* TxBufAddr, uint16_t TxBufLen, uint16_t ThresholdLen, INT_FUNC CallBack);
#define UARTS_DMA_TxInit(UartNum, TxBufAddr, TxBufLen, ThresholdLen, CallBack) (*UARTS_DMA_TxInit_MAP[UartNum])(TxBufAddr, TxBufLen, ThresholdLen, CallBack)

/**
 * @brief      UART DMA 传送模式初始化，发送缓存区地址，发送缓存长度，中断触发门槛值，回调函数
 * @param[in]  TxBufAddr  	传送缓冲区地址
 * @param[in]  TxBufLen   	传送字符长度
 * @param[in]  TimeOut 		超时时间设置，即达到设置的超时时间还未发送完成，则放弃发送
 * @return     实际发送数据长度
 */
uint32_t UART0_DMA_Send(uint8_t* SendBuf, uint16_t BufLen, uint32_t TimeOut);
uint32_t UART1_DMA_Send(uint8_t* SendBuf, uint16_t BufLen, uint32_t TimeOut);
#define UARTS_DMA_Send(UartNum, SendBuf, BufLen, TimeOut) (*UARTS_DMA_Send_MAP[UartNum])(SendBuf, BufLen, TimeOut)

/**
 * @brief      UART DMA 接收数据，设置接收缓存地址，接收字符长度，接收超时设置
 * @param[in]  RecvBuf  	接收缓冲区地址
 * @param[in]  BufLen   	接收长度设置
 * @param[in]  TimeOut		超时时间设置，规定时间内接收没有达到预期长度或者没收到收据则放弃接收
 * @return     接收到数据的实际长度
 */
uint32_t UART0_DMA_Recv(uint8_t* RecvBuf, uint16_t BufLen, uint32_t TimeOut);
uint32_t UART1_DMA_Recv(uint8_t* RecvBuf, uint16_t BufLen, uint32_t TimeOut);
#define UARTS_DMA_Recv(UartNum, RecvBuf, BufLen, TimeOut) (*UARTS_DMA_Recv_MAP[UartNum])(RecvBuf, BufLen, TimeOut)

/**
 * @brief      UART DMA 传送数据，设置传入传送缓存区地址，传送字符长度
 * @param[in]  RecvBuf  	传送缓冲区地址
 * @param[in]  BufLen   	传送字符长度设置
 * @return     无返回值
 */
void UART0_DMA_SendDataStart(uint8_t* SendBuf, uint16_t BufLen);
void UART1_DMA_SendDataStart(uint8_t* SendBuf, uint16_t BufLen);
#define UARTS_DMA_SendDataStart(UartNum, SendBuf, BufLen) (*UARTS_DMA_SendDataStart_MAP[UartNum])(SendBuf, BufLen)

/**
 * @brief      判断数据传送是否完成
 * @return     TURE传送完成 	 FALSE传送未完成
  */
bool UART0_DMA_TxIsTransferDone(void);
bool UART1_DMA_TxIsTransferDone(void);
#define UARTS_DMA_TxIsTransferDone(UartNum) (*UARTS_DMA_TxIsTransferDone_MAP[UartNum])()

/**
 * @brief      UART DMA 	非阻塞方式使能接收，设置接收缓存地址，接收字符长度
 * @param[in]  RecvBuf  	接收缓冲区地址
 * @param[in]  BufLen   	接收长度设置
 * @return     接收到数据的实际长度
 */
int32_t UART0_DMA_RecvDataStart(uint8_t* RecvBuf, uint16_t BufLen);
int32_t UART1_DMA_RecvDataStart(uint8_t* RecvBuf, uint16_t BufLen);
#define UARTS_DMA_RecvDataStart(UartNum, RecvBuf, BufLen) (*UARTS_DMA_RecvDataStart_MAP[UartNum])(RecvBuf, BufLen)

/**
 * @brief      判断是否传输完成
 * @return     TURE传送完成 	 FALSE传送未完成
  */
bool UART0_DMA_RxIsTransferDone(void);
bool UART1_DMA_RxIsTransferDone(void);
#define UARTS_DMA_RxIsTransferDone(UartNum) (*UARTS_DMA_RxIsTransferDone_MAP[UartNum])()

/**
 * @brief      查询DMA接收缓存区数据长度
 * @return     返回数据长度
  */
int32_t UART0_DMA_RxDataLen(void);
int32_t UART1_DMA_RxDataLen(void);
#define UARTS_DMA_RxDataLen(UartNum) (*UARTS_DMA_RxDataLen_MAP[UartNum])()

/**
 * @brief      查询DMA发送缓存区数据长度
 * @return     返回数据长度
  */
int32_t UART0_DMA_TxDataLen(void);
int32_t UART1_DMA_TxDataLen(void);
#define UARTS_DMA_TxDataLen(UartNum) (*UARTS_DMA_TxDataLen_MAP[UartNum])()

/**
 * @brief      注册UART_DMA模式下的中断回调函数，注意配置中断类型
 * @param[in]  IntType  	中断类型设置: 1.DMA_DONE_INT无中断    2.DMA_THRESHOLD_INT阈值触发中断    3.DMA_ERROR_INT错误中断
 * param[in]   CallBack		需注册的中断回掉函数，若写入NULL，则关闭回调功能
 * @return     返回数据长度
  */
void UART0_DMA_RxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack);
void UART1_DMA_RxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack);
#define UARTS_DMA_RxSetInterruptFun(UartNum, IntType, CallBack) (*UARTS_DMA_RxSetInterruptFun_MAP[UartNum])(IntType, CallBack)
void UART0_DMA_TxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack);
void UART1_DMA_TxSetInterruptFun(DMA_INT_TYPE IntType, INT_FUNC CallBack);
#define UARTS_DMA_TxSetInterruptFun(UartNum, IntType, CallBack) (*UARTS_DMA_TxSetInterruptFun_MAP[UartNum])(IntType, CallBack)


/**
 * @brief      注册UART_DMA模式下的传输完成的中断回调函数，注意配置中断类型
 *             注：该函数等同于，上面的函数的特殊情况->中断类型设置	DMA_DONE_INT
 * param[in]   CallBack		需注册的中断回掉函数，若写入NULL，则关闭回调功能
 * @return     返回数据长度
  */
void UART0_DMA_TxSetTransmitDoneFun(INT_FUNC CallBack);
void UART1_DMA_TxSetTransmitDoneFun(INT_FUNC CallBack);
#define UARTS_DMA_TxSetTransmitDoneFun(UartNum, CallBack) (*UARTS_DMA_TxSetTransmitDoneFun_MAP[UartNum])(CallBack)
void UART0_DMA_RxSetTransmitDoneFun(INT_FUNC CallBack);
void UART1_DMA_RxSetTransmitDoneFun(INT_FUNC CallBack);
#define UARTS_DMA_RxSetTransmitDoneFun(UartNum, CallBack) (*UARTS_DMA_RxSetTransmitDoneFun_MAP[UartNum])(CallBack)

/**
 * @brief      UART_DMA使能函数
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
 * @brief      UART_DMA禁用函数
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
 * @brief      UART_DMA 中断标志位清除
 * @param[in]   IntType  	中断类型设置	DMA_DONE_INT无中断DMA_THRESHOLD_INT阈值触发中断DMA_ERROR_INT错误中断
 * @return  	DMA_ERROR
 * @note	中断标志在不开启中断的情况下也会置位
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
