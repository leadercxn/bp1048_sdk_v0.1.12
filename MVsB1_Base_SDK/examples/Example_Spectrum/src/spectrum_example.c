/**
 **************************************************************************************
 * @file    spectrum_example.c
 * @brief   spectrum_example
 *
 * @author  Castle
 * @version V1.0.0
 *
 * $Created: 2019-11-03 19:17:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "gpio.h"
#include "watchdog.h"
#include "sys.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "dac_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "spi_flash.h"
#include "remap.h"
#include "irqn.h"
#include "dac.h"
#include "chip_info.h"

#include "typedefine.h"
#include "mvstdio.h"
#include "spectrum.h"

#define SPECTRUM_SAMPLE_RATE 44100
#define SPECTRUM_SIZE 256
#define SPECTRUM_BIN_SIZE 16
#define NOISE_LEVEL 0
int16_t data_buffer1[1024]={0};
int16_t data_buffer2[1024]={0};

extern bool sine_generator(uint16_t samplerate, uint16_t frq, int8_t ampl, bool is_24bit, int32_t *dataout, uint16_t *pLen);

MemHandle cache_it;
uint8_t cache_raw_buf[1024*8];

int16_t temp_buf[SPECTRUM_SIZE];
uint32_t spectrum_out[SPECTRUM_BIN_SIZE];
uint8_t level_out[SPECTRUM_BIN_SIZE];
SpectrumContext gsc;

int16_t alf_table[16] = {0, -2, -4, -8, -10, -12, -16, -20, -24, -28, -32, -36, -38, -42, -46, -50};

void spectrum_test(void)
{
	int i,k, ii, jj;
	uint16_t *sfi;
	mv_mopen(&cache_it, cache_raw_buf, sizeof(cache_raw_buf), NULL);
	spectrum_init(&gsc, SPECTRUM_SAMPLE_RATE, SPECTRUM_SIZE, SPECTRUM_BIN_SIZE);
	sfi = spectrum_get_freq_interval(&gsc);
	for (;;)
	{
		uint16_t pLen;
		uint64_t cycle_value;
		uint32_t cpu_mips;

		sine_generator(SPECTRUM_SAMPLE_RATE, 3000, -0, FALSE, (int32_t*)data_buffer1, &pLen);
		sine_generator(SPECTRUM_SAMPLE_RATE, 4000, -0, FALSE, (int32_t*)data_buffer2, &pLen);
		if(mv_mremain(&cache_it) > pLen)
		{
			for (i = 0; i < pLen/4; i++)
			{
				// data_buffer1[i] = __nds32__clips(((int32_t)data_buffer1[i*2] + data_buffer1[i*2+1])>>1, (16)-1);
				data_buffer1[i] = __nds32__clips(((int32_t)data_buffer1[i*2] + data_buffer2[i*2])>>1, (16)-1);
			}
		
			mv_mwrite(data_buffer1, 1, pLen/2, &cache_it);
		}

		while(mv_msize(&cache_it) >= SPECTRUM_SIZE*2)
		{
			mv_mread(temp_buf, 1, SPECTRUM_SIZE*2, &cache_it);

			__nds32__mtsr(0, NDS32_SR_PFMC0);
			__nds32__mtsr(1, NDS32_SR_PFM_CTL);
			spectrum_process(&gsc, temp_buf, spectrum_out);
			spectrum_quantized_level(&gsc, spectrum_out, level_out);
			__nds32__mtsr(0, NDS32_SR_PFM_CTL);
			cycle_value = __nds32__mfsr(NDS32_SR_PFMC0);
			cpu_mips = cycle_value * (uint64_t)SPECTRUM_SAMPLE_RATE / SPECTRUM_SIZE / 1000000;
			DBG("mcps: %d\n", cpu_mips);

			DBG("\n---------------------------------\n");
			for (i = 0; i < SPECTRUM_BIN_SIZE; i++)
			{
				DBG("%02d: %05dHz--->%03d", i, sfi[i], level_out[i]);

				for (k = 0; k < level_out[i]; k++)
				{
					DBG("|");
				}
				DBG("\n");
			}
			DBG("---------------------------------\n");
		}
	}
	
}

int main(void)
{
	Chip_Init(1);
	WDG_Disable();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_APllLock(288000);
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
	DBG("|                Spectrum Example                     |\n");
	DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
	DBG("\\-----------------------------------------------------/\n");
	DBG("\n");

	spectrum_test();
	
	while(1);
}
