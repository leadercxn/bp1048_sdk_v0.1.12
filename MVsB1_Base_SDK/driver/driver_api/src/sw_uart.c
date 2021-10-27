/**
 *******************************************************************************
 * @file    sw_uart.c
 * @brief	software uart driver. When hardware uart pins is occupied as other
 *          functions, this software uart can output debug info. This software
 *          uart only has TX function.

 * @author  Sam
 * @version V1.0.0

 * $Created: 2018-03-13 16:14:05$
 * @Copyright (C) 2014, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *******************************************************************************
 */
#include "type.h"
#include "gpio.h"

#ifdef FUNC_OS_EN
#include "rtos_api.h"
osMutexId SwUARTMutex = NULL;
#endif

#include "sw_uart.h"

#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

#ifdef SW_UART_IO_PORT

#if (CFG_SW_UART_BANDRATE == 38400)
#define SW_DELAY	136
#elif(CFG_SW_UART_BANDRATE == 57600)
#define SW_DELAY	90
#elif(CFG_SW_UART_BANDRATE == 115200)
#define SW_DELAY	45
#elif(CFG_SW_UART_BANDRATE == 256000)
#define SW_DELAY	20
#elif(CFG_SW_UART_BANDRATE == 460800)
#define SW_DELAY	11
#elif(CFG_SW_UART_BANDRATE == 1000000)
#define SW_DELAY	5
#endif


#if (SW_UART_IO_PORT == GPIO_A_IN)
#define SW_PORT			GPIO_A_IN
#define SW_OUT_REG		0x40010004
#else
#define SW_PORT			GPIO_B_IN
#define SW_OUT_REG		0x40010034
#endif

#define SW_PIN_MASK		(1<<SW_UART_IO_PORT_PIN_INDEX)


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
 * @param  BaudRate, can be 460800, 256000, 115200, 57600 or 38400
 *
 * @return None.
 */
void SwUartTxInit(uint8_t PortIndex, uint8_t PinIndex, uint32_t BaudRate)
{
	GPIO_RegOneBitSet(SW_PORT + 1, SW_PIN_MASK);//Must output high as default!
	GPIO_RegOneBitClear(SW_PORT + 5, SW_PIN_MASK);//Input disable
	GPIO_RegOneBitSet(SW_PORT + 6, SW_PIN_MASK);//Output enable
}

 /**
 * @brief  Deinit uart TX to default gpio.
 * @param  PortIndex: select which gpio bank to use
 *     @arg  SWUART_GPIO_PORT_A
 *     @arg  SWUART_GPIO_PORT_B
 *     @arg  SWUART_GPIO_PORT_C
 * @param  PinIndex:  0 ~ 31, select which gpio io to deinit.
 * @return None.
 */
void SwUartTxDeinit(uint8_t PortIndex, uint8_t PinIndex)
{
	GPIO_RegOneBitClear(SW_PORT + 1, SW_PIN_MASK);//OUTPUT = 0;
	GPIO_RegOneBitSet(SW_PORT + 5, SW_PIN_MASK);//IE = 1
	GPIO_RegOneBitClear(SW_PORT + 6, SW_PIN_MASK);//OE = 0
}

/**
 * @Brief	make sw uart baudrate automatic  adaptation
 * @Param	PreFq System Frequency before changed
 * @Param	CurFq System Frequency after changed
 */
void SWUartBuadRateAutoAdap(char PreFq, char CurFq)
{

}


/**
 * @brief  Delay to keep tx's level for some time
 * @param  void
 * @return None.
 */
uint32_t Clock_SysClockFreqGet(void); //clk.h
__attribute__((section(".tcm_section"), optimize("Og")))
void SwUartDelay(unsigned int us)//200ns
{
	int i;
	for(i=0;i<us;i++)
	{
		//200ns@320M
		__asm __volatile__(
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n""nop\n""nop\n"
		"nop\n"
		);
	}
}
inline void GIE_DISABLE(void);
inline bool GIE_STATE_GET();
inline void GIE_ENABLE(void);
/**
* @brief  Send 1 byte
* @param  c: byte to be send
* @return None
*/
__attribute__((section(".tcm_section"), optimize("Og")))
void SwUartSendByte(uint8_t c)
{
	uint8_t i;
	uint8_t Cnt = 0;
#ifndef	CFG_FUNC_LED_REFRESH
	bool ret;

	ret = GIE_STATE_GET();
	GIE_DISABLE();
#else
	InterruptLevelSet(1);
#endif
	(*(volatile unsigned long *) SW_OUT_REG) &= ~(SW_PIN_MASK);
	SwUartDelay(SW_DELAY);

	for(i=0; i<8; i++)
	{
		if(c & 0x01)
		{
			//GpioSetRegBits(PortIndex + 1, OutRegBitMsk);//OUTPUT = 0;
			(*(volatile unsigned long *) SW_OUT_REG) |= SW_PIN_MASK;
			Cnt++;
		}
		else
		{
			//GpioClrRegBits(PortIndex + 1, OutRegBitMsk);//OUTPUT = 0;
			(*(volatile unsigned long *) SW_OUT_REG) &= ~(SW_PIN_MASK);
		}
		SwUartDelay(SW_DELAY);
		c >>= 1;
	}

	if(Cnt % 2)//Å¼Êý
	{
		(*(volatile unsigned long *) SW_OUT_REG) |= SW_PIN_MASK;
	}
	else
	{
		(*(volatile unsigned long *) SW_OUT_REG) &= ~(SW_PIN_MASK);
	}
	SwUartDelay(SW_DELAY);

	(*(volatile unsigned long *) SW_OUT_REG) |= SW_PIN_MASK;
	SwUartDelay(SW_DELAY);
#ifndef	CFG_FUNC_LED_REFRESH
	if(ret)
	{
		GIE_ENABLE();
	}
#else
	InterruptLevelRestore();
#endif
}

/**
 * @brief  Send data from buffer
 * @param  Buf: Buffer address
 * @param  BufLen: Length of bytes to be send
 * @return None
 */
void SwUartSend(uint8_t* Buf, uint32_t BufLen)
{
	while(BufLen--)
	{
		SwUartSendByte(*Buf++);
	}
}
#endif

