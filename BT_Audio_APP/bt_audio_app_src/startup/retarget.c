/*
 * retarget.c
 *
 *  Created on: Mar 8, 2017
 *      Author: peter
 */

#include <stdio.h>
#include "uarts_interface.h"
#include "type.h"
#include "remap.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
uint8_t DebugPrintPort = UART_PORT0;
uint32_t gSramEndAddr = SRAM_END_ADDR;

uint8_t IsSwUartActedAsUARTFlag = 0;
void EnableSwUartAsUART(uint8_t EnableFlag)
{
	IsSwUartActedAsUARTFlag = EnableFlag;
}

//This is used as dummy function in case that appilcation dont define this function.
__attribute__((weak))
void SwUartSend(unsigned char* Buf, unsigned int BufLen)
{

}

__attribute__((used))
int putchar(int c)
{
#if defined(CFG_APP_CONFIG)  && !defined(CFG_FUNC_DEBUG_EN)
#else
    if(IsSwUartActedAsUARTFlag)
    {
        if((unsigned char)c == '\n')
        {
            const char lfca[2] = "\r\n";
            SwUartSend((unsigned char*)lfca, 2);
        }
        else
        {
            SwUartSend((unsigned char*)&c, 1);
        }
    }
    else
    {
    	if (c == '\n')
    	{
    		UARTS_SendByte(DebugPrintPort, '\r');
    		UARTS_SendByte(DebugPrintPort, '\n');
    	}
    	else
    	{
    		UARTS_SendByte(DebugPrintPort, (uint8_t)c);
    	}
    }
#endif
	return c;
}

__attribute__((used))
void nds_write(const unsigned char *buf, int size)
{
	int i;
	for (i = 0; i < size; i++)
	{
		putchar(buf[i]);
	}
}

int DbgUartInit(int Which, unsigned int BaudRate, unsigned char DatumBits, unsigned char Parity, unsigned char StopBits)
{
	DebugPrintPort = Which;
	return UARTS_Init(Which, BaudRate, DatumBits,  Parity,  StopBits);
}


