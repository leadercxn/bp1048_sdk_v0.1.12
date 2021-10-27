/**
 *******************************************************************************
 * @file    sw_uart.h
 * @brief	software uart driver. When hardware uart pins is occupied as other
 *          functions, this software uart can output debug info. This software
 *          uart only has TX function.

 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2018-03-13 16:14:05$
 * @Copyright (C) 2014, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.

 * @使用注意事项：
 * 1) 如果想将SWUART的输出重定向到标准输出端口(使用DBG打印)，需要调用函数
 *    EnableSwUartAsFuart(TRUE)进行使能。
 * 2) 必须在Cache,TCM初始化之后才能正常发送数据。
 *******************************************************************************
 */


#ifndef __SW_UART_H__
#define __SW_UART_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


#include "type.h"
#include "gpio.h"

/* GPIO bank selection */
#define  SWUART_GPIO_PORT_A    GPIO_A_IN
#define  SWUART_GPIO_PORT_B    GPIO_B_IN

/**
 * @brief  Redirect SWUART to standard output(for example, DBG)
 * @param  EnableFlag: 0: not redirect, 1: redirect
 * @return None.
 */
void EnableSwUartAsFuart(bool EnableFlag);

/**
 * @brief  Init specified IO as software uart's TX.
 *
 *         Any other divided frequency is the same with last example.
 * @param  PortIndex: select which gpio bank to use
 *     @arg  SWUART_GPIO_PORT_A
 *     @arg  SWUART_GPIO_PORT_B
 * @param  PinIndex:  0 ~ 31, select which gpio io to use.
 *         for example, if PortIndex = SWUART_GPIO_PORT_A, PinIndex = 10,
 *         GPIO_A10 is used as software uart's TX.
 * @param  BaudRate, can be 460800 ,256000 ,115200 , 57600 or 38400
 *
 * @return None.
 */
void SwUartTxInit(uint8_t PortIndex, uint8_t PinIndex, uint32_t BaudRate);

/**
 * @brief  Deinit uart TX to default gpio.
 * @param  PortIndex: select which gpio bank to use
 *     @arg  SWUART_GPIO_PORT_A
 *     @arg  SWUART_GPIO_PORT_B
 * @param  PinIndex:  0 ~ 31, select which gpio io to deinit.
 * @return None.
 */
void SwUartTxDeinit(uint8_t PortIndex, uint8_t PinIndex);


/**
 * @brief  Send data from buffer
 * @param  Buf: Buffer address
 * @param  BufLen: Length of bytes to be send
 * @return None.
 * @note   This function can only be used after OSStartKernel() is called
 */
void SwUartSend(uint8_t* Buf, uint32_t BufLen);


///**
// * @Brief	make sw uart baudrate automatic  adaptation
// * @Param	PreFq System Frequency before changed
// * @Param	CurFq System Frequency after changed
// */
//void SWUartBuadRateAutoAdap(char PreFq, char CurFq);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif	//__SW_UART_H__
