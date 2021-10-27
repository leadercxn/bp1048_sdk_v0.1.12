/**
 **************************************************************************************
 * @file    spectrum.c
 * @brief   spectrum interface
 *
 * @author  Castle
 * @version V1.0.0
 *
 * $Created: 2018-01-04 19:17:00$
 *
 * @Copyright (C) 2017, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "type.h"
#include "fft_api.h"
#include <math.h>
#include "spectrum.h"

#define M_2PI 6.283185307179586476925286766559005

static const uint8_t StepTable64[] =
{
	1,	1,	1,	1,	2,	2,	3,	3,	5,	4,	4,	5,	7,	10,	14,	1
};

static const int16_t LevelTableDB[16] =
{
	0, -2, -4, -8, -10, -12, -16, -20, -24, -28, -32, -36, -38, -42, -46, -50
};

static const uint32_t LevelTable[16] =
{
	1175,
	2842,
	6957,
	17265,
	27204,
	67897,
	169618,
	425512,
	1066583,
	2674237,
	6712487,
	10634771,
	16855124,
	42320625,
	67069215,
	106283615,
};

void spectrum_quantized_level(SpectrumContext *sptct, uint32_t *bin_value, uint8_t *level_out)
{
	int i,j;
	for (i = 0; i < sptct->bin_num; i++)
	{
		for (j = 0; j < 16; j++)
		{
			if(bin_value[i] < LevelTable[j])
				break;
		}
		level_out[i] = (j>15) ? (15):(j);
	}
}

bool spectrum_init(SpectrumContext *sptct, uint16_t sample_rate, uint16_t spt_len, uint8_t bin_num)
{
	int i, sum = 0;
	float frq_one_unit;
    if(spt_len > SPECTRUM_FFT_MAX_LEN)
        return FALSE;

    if(bin_num > SPECTRUM_BIN_MAX_LEN)
        return FALSE;
    
    memset(sptct, 0x00, sizeof(SpectrumContext));
    sptct->sample_rate = sample_rate;
    sptct->spt_len = spt_len;
    sptct->bin_num = bin_num;
    sptct->is_init = TRUE;

    for (i = 0; i < sptct->spt_len; i++)
    {
    	sptct->hm[i] = (int32_t)((0.54 - 0.46 * cos(M_2PI * i / (sptct->spt_len-1))) * 32767);
    }

    for(i = 0; i<sptct->bin_num; i++)
    {
    	sptct->step_table[i] = StepTable64[i] * (spt_len/64/2);
    }

    frq_one_unit = (float)sample_rate/2/(spt_len/2);
    for(i=0; i<bin_num; i++)
    {
    	sptct->bin_frq[i] = frq_one_unit*(sptct->step_table[i] + sum);
    	sum += sptct->step_table[i];
    }
    return TRUE;
}

bool spectrum_process(SpectrumContext *sptct, int16_t *pcm, uint32_t *bin_out)
{
    int i, j, k;

    memset(bin_out, 0x00, sizeof(uint32_t) * sptct->bin_num);
    for (i = 0; i < sptct->spt_len; i++)
    {
    	sptct->fft_buf[i] = (pcm[i] * sptct->hm[i]) >> 15;
    }

    if(rfft_api(sptct->fft_buf, sptct->spt_len, 1) == 1)
    {
        sptct->fft_buf[1] = 0;

        for (i = 0, j = 0, k = 0; i < sptct->spt_len; i+=2, j++)
        {
            int64_t temp = (sptct->fft_buf[i] * sptct->fft_buf[i]) + (sptct->fft_buf[i+1] * sptct->fft_buf[i+1]);
            if(j >= sptct->step_table[k])
            {
            	j -= sptct->step_table[k];
            	k++;
            }
            bin_out[k] += temp;
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

uint16_t *spectrum_get_freq_interval(SpectrumContext *sptct)
{
	return sptct->bin_frq;
}
