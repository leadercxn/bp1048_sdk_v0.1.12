/**
 **************************************************************************************
 * @file    cache_downsize_example.c
 * @brief   cache downsize example
 *
 * @author  Peter
 * @version V1.0.0
 *
 * $Created: 2019-05-22 19:17:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "spi_flash.h"
#include "remap.h"
#include "irqn.h"
#include "chip_info.h"
#include "watchdog.h"
#include "sys.h"

void I16K_D16K_Set(void);
int main(void)
{
	Chip_Init(1);
	WDG_Disable();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(PLL_CLK_MODE);

	GPIO_PortAModeSet(GPIOA24, 1);//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	SysTickInit();

	Remap_InitTcm(0, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     CacheDownSize Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

    I16K_D16K_Set();

	while(1);
}
