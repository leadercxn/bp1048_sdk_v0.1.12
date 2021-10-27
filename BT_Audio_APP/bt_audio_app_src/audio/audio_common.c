/**
 **************************************************************************************
 * @file    audio_common.c
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2020-8-6 13:17:21$
 *
 * @Copyright (C) 2020, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "app_config.h"
#include "resampler_polyphase.h"

static const int32_t SamplerRatio[][3] =
{
	{8000,  RESAMPLER_POLYPHASE_SRC_RATIO_441_80,	RESAMPLER_POLYPHASE_SRC_RATIO_6_1},
	{11025, RESAMPLER_POLYPHASE_SRC_RATIO_4_1,		RESAMPLER_POLYPHASE_SRC_RATIO_640_147},
	{12000, RESAMPLER_POLYPHASE_SRC_RATIO_147_40,	RESAMPLER_POLYPHASE_SRC_RATIO_4_1},
	{16000, RESAMPLER_POLYPHASE_SRC_RATIO_441_160,	RESAMPLER_POLYPHASE_SRC_RATIO_3_1},
	{22050, RESAMPLER_POLYPHASE_SRC_RATIO_2_1,		RESAMPLER_POLYPHASE_SRC_RATIO_320_147},
	{24000, RESAMPLER_POLYPHASE_SRC_RATIO_147_80,	RESAMPLER_POLYPHASE_SRC_RATIO_2_1},
	{32000, RESAMPLER_POLYPHASE_SRC_RATIO_441_320, 	RESAMPLER_POLYPHASE_SRC_RATIO_3_2},
	{44100, 0,										RESAMPLER_POLYPHASE_SRC_RATIO_160_147},
	{48000, RESAMPLER_POLYPHASE_SRC_RATIO_147_160,	0},
	{88200, RESAMPLER_POLYPHASE_SRC_RATIO_1_2,		RESAMPLER_POLYPHASE_SRC_RATIO_80_147},
	{96000, RESAMPLER_POLYPHASE_SRC_RATIO_147_320,	RESAMPLER_POLYPHASE_SRC_RATIO_1_2},
	{176400,RESAMPLER_POLYPHASE_SRC_RATIO_1_4,		RESAMPLER_POLYPHASE_SRC_RATIO_40_147},
	{192000,RESAMPLER_POLYPHASE_SRC_RATIO_147_640,	RESAMPLER_POLYPHASE_SRC_RATIO_1_4},
	{33075, RESAMPLER_POLYPHASE_SRC_RATIO_4_3,		RESAMPLER_POLYPHASE_SRC_RATIO_4_3},
};

int32_t Get_Resampler_Polyphase(uint32_t resampler)
{
	int32_t res = 0;
	uint32_t i;
	for(i=0; i<14; i++)
	{
		if(SamplerRatio[i][0] == resampler)
		{
			if(CFG_PARA_SAMPLE_RATE == 44100)
			{
				res = SamplerRatio[i][1];
				break;
			}
			else if(CFG_PARA_SAMPLE_RATE == 48000)
			{
				res = SamplerRatio[i][2];
				break;
			}
			else
			{
				res = 0;
			}
		}
	}
	//APP_DBG("res = %d\n", res);		
	return res;
}

