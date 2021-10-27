/**
 **************************************************************************************
 * @file    audio_adjust.c
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2020-3-2 11:17:21$
 *
 * @Copyright (C) 2020, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "app_config.h"
#include "app_message.h"
#include "dac.h"
#include "audio_adc.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "debug.h"
#include "audio_core_api.h"
#include "timeout.h"
#include "sra.h"
#include "main_task.h"
#include "bt_manager.h"
#include "audio_adjust.h"
#include "rtos_api.h"
#include "bt_play_api.h"
#include "clk.h"

#ifdef CFG_FUNC_FREQ_ADJUST


//调频偏，钳位正负万分之五,(蓝牙因阻塞可单边使用两倍)
#define ADJUST_PERIOD_COUNT			(128 * 64) //水位次数 求均消抖，推荐16,32/64/128,便于计算，相当于100~2000mS左右调整一次。
#define ADJUST_PERIOD_BT			1000	//ms 针对蓝牙使用 微调时间间隔定义

//调频偏值最大单位数(正负)，硬件微调对应reg值;APLL 288 分频获取audioclock（12M）~24+ reg为计数器小部分，单位1/256.一个单位调整频偏值为(288/12)*256
#define ADJUST_REG_SCALE			(24*256) //用于转换成寄存器调整值。(ADJUST_CLOCK_MAX/10000) /(1/ADJUST_REG_SCALE) 是硬件微调的最大值。
#define ADJUST_CLOCK_MAX			5 //单位万分之一
#define ADJUST_SHRESHOLD			1//1~5 ，推荐值1，随着数值升高，在low~High之间的水位波动性收敛变慢，响应调整频度减少。

static uint16_t AdjustLevelHigh	= 0;	//(CFG_PARA_SAMPLES_PER_FRAME * 3 / 4); //混音前buf两帧大小，获取时机是抽走一帧后。
static uint16_t AdjustLevelLow	= 0;	//(CFG_PARA_SAMPLES_PER_FRAME / 4);
static uint32_t	PeriodCount;			//当前统计周期，单位采样点帧数，(PeriodCount * mainAppCt.SamplesPreFrame)/CFG_PARA_SAMPLE_RATE 是统计和微调的周期。
static int8_t 	FreqAsyncSource = 127;	//异步音源通道，127为缺省值，未启用。
static int32_t  AsyncRemain;
static int16_t  AsyncRemainLast;
static int16_t  AdjustValue = 0;		//系统分频数(偏差)调整值 即小数部分(单位是1/256),
static int8_t 	LastRise = 0;			//上次水位变化情况，1:涨 0：平 -1:跌
static uint32_t Count;

/**
 * @func        AudioCoreSourceFreqAdjustEnable
 * @brief       使能系统音频分频微调，使信道之间同步(与异步音源)
 * @param       uint8_t AsyncIndex   异步音频源混音信道编号
 * @param       uint16_t LevelLow   异步音频源混音信道低水位采样点值
 * @param       uint16_t LevelHigh   异步音频源混音信道高水位采样点值
 * @Output      None
 * @return      None
 * @Others
 * Record
 * 1.Date        : 20180518
 *   Author      : pi.wang
 *   Modification: Created function
 */
void AudioCoreSourceFreqAdjustEnable(uint8_t AsyncIndex, uint16_t LevelLow, uint16_t LevelHigh)
{
	if(AsyncIndex >= AUDIO_CORE_SOURCE_MAX_MUN)
		return ;

	Count = 0;
	FreqAsyncSource = AsyncIndex;
	AsyncRemainLast = 0xffff;
	AsyncRemain = 0;
	AdjustValue = 0;
	AdjustLevelHigh = LevelHigh;
	AdjustLevelLow = LevelLow;

#ifdef CFG_FUNC_SOFT_ADJUST_IN
	if(SoftFlagGet(SoftFlagBtSra))
	{
		AudioCoreSourceSRAInit();
	}
	else
#endif
	{
		PeriodCount = ADJUST_PERIOD_COUNT / mainAppCt.SamplesPreFrame;
	}
}

void AudioCoreSourceFreqAdjustDisable(void)
{
	FreqAsyncSource = 127;
	AsyncRemainLast = 0xffff;
	Clock_AudioPllClockAdjust(PLL_CLK_1, 0, 0);//恢复缺省值
	Clock_AudioPllClockAdjust(PLL_CLK_2, 0, 0);
	AdjustValue = 0;
	Count = 0;
	AdjustLevelHigh = 0;//(CFG_PARA_SAMPLES_PER_FRAME * 3 / 4);
	AdjustLevelLow = 0;//(CFG_PARA_SAMPLES_PER_FRAME / 4);
	PeriodCount = ADJUST_PERIOD_COUNT / mainAppCt.SamplesPreFrame;

#ifdef CFG_FUNC_SOFT_ADJUST_IN
	if(SoftFlagGet(SoftFlagBtSra))
	{
		AudioCoreSourceSRADeinit();
	}
#endif
}

extern uint32_t GetValidSbcDataSize(void);
void AudioCoreSourceFreqAdjust(void)
{
	int16_t 	Adjust = 0;//为了保障调整频率平滑，单次可微调1单位。
	int8_t		LevelRise = 0;
	int16_t 	AdjustMax;

	AdjustMax = (uint32_t)(ADJUST_CLOCK_MAX * ADJUST_REG_SCALE) / 10000;

	if(FreqAsyncSource < AUDIO_CORE_SOURCE_MAX_MUN
	&& ((AudioCore.AudioSource[FreqAsyncSource].Enable)))// && AudioCore.AudioSource[FreqAsyncSource].FuncDataGetLen != NULL
	{
#ifdef CFG_APP_BT_MODE_EN
		if(GetSystemMode() == AppModeBtAudioPlay)
		{
			AsyncRemain += GetValidSbcDataSize();
		}
#if (BT_HFP_SUPPORT == ENABLE)
		else if(GetSystemMode() == AppModeBtHfPlay)
		{
			AsyncRemain += BtHf_SendScoBufLenGet();
			//APP_DBG(".\n");
		}
#endif
		else
#endif
		{
			AsyncRemain += AudioCore.AudioSource[FreqAsyncSource].FuncDataGetLen();
		}
		if(++Count >= PeriodCount)
		{
			AsyncRemain = AsyncRemain / Count;//水位消抖。
			if(AsyncRemainLast != 0xffff)//not first
			{
				if(AsyncRemain > AsyncRemainLast)//水位涨
				{
					LevelRise = 1;
					if((AsyncRemain > AdjustLevelHigh) //buf水位 越过缓冲高限位
						|| (AsyncRemain >= AdjustLevelLow && (LastRise >= ADJUST_SHRESHOLD)))//波动区间内 连续上涨
					{
						Adjust = -1; //需加快系统时钟/降低分频调整值。
					}
				}
				else if(AsyncRemain < AsyncRemainLast)//水位跌
				{
					LevelRise = -1;
					if((AsyncRemain < AdjustLevelLow)//buf水位 越过缓冲低限位
						|| ((AsyncRemain < AdjustLevelHigh) && (LastRise <= -ADJUST_SHRESHOLD))) //波动区间内 连续下跌
					{
						Adjust = 1; //需降低系统时钟/提高分频调整值
					}
				}
				Adjust += AdjustValue; //叠加Last频偏调整值

#ifdef CFG_APP_BT_MODE_EN
				if(Adjust > AdjustMax && (GetSystemMode() == AppModeBtAudioPlay)) //调频偏钳位
				{
					if(Adjust > AdjustMax * 2)
					{
						Adjust = AdjustMax * 2;
					}
				}
				else if(Adjust > AdjustMax)
#else
				if(Adjust > AdjustMax)
#endif
				{
					Adjust = AdjustMax;
				}
				else if(Adjust < -AdjustMax)
				{
					Adjust = -AdjustMax;
				}
				//APP_DBG("R:%d A:%d \n", (int)AsyncRemain , (int)AdjustValue);
				if(Adjust != AdjustValue)//分频值需调整
				{
					AdjustValue = Adjust; //保存本次频偏 分频。
					APP_DBG("\nR:%d A:%d \n", (int)AsyncRemain , (int)AdjustValue);
#ifdef CFG_APP_BT_MODE_EN
					if(GetSystemMode() == AppModeBtHfPlay)
					{
					if(AdjustValue > 0) //
						{
							APP_DBG("kuickly  \n");
							Clock_AudioPllClockAdjust(PLL_CLK_1, 1, AdjustValue);
							Clock_AudioPllClockAdjust(PLL_CLK_2, 1, AdjustValue);
						}
					else if(AdjustValue <0)
						{
						
							APP_DBG("SLOW\n");
							Clock_AudioPllClockAdjust(PLL_CLK_1, 0, -AdjustValue);
							Clock_AudioPllClockAdjust(PLL_CLK_2, 0, -AdjustValue);
						}
					}
					else
#endif
					{
						if(AdjustValue > 0) //相对缺省，提高分频值
						{
						Clock_AudioPllClockAdjust(PLL_CLK_1, 0, AdjustValue);
						Clock_AudioPllClockAdjust(PLL_CLK_2, 0, AdjustValue);
						}
						else
						{
						Clock_AudioPllClockAdjust(PLL_CLK_1, 1, -AdjustValue);
						Clock_AudioPllClockAdjust(PLL_CLK_2, 1, -AdjustValue);
						}
					}
				}
			}
			AsyncRemainLast = AsyncRemain;
			AsyncRemain = 0;//累积数据清零
			Count = 0;
			LastRise = LevelRise;//备份本次水位变化。
		}
	}
}

#ifdef CFG_FUNC_SOFT_ADJUST_IN

AudioAdjustContext*	audioSourceAdjustCt = NULL;

/*
****************************************************************
* 软件微调处理
****************************************************************
*/

void AudioCoreSourceSRAResMalloc(void)
{
	audioSourceAdjustCt = osPortMalloc(sizeof(AudioAdjustContext));//One Frame
	if(audioSourceAdjustCt == NULL)
	{
		APP_DBG("malloc err, no mem\n");
		return;
	}
}

void AudioCoreSourceSRAResRelease(void)
{
	if(audioSourceAdjustCt != NULL)
	{
		osPortFree(audioSourceAdjustCt);
		audioSourceAdjustCt = NULL;
		APP_DBG("free Audio SRA\n");
	}
}

void AudioCoreSourceSRAInit(void)
{
	APP_DBG("open SRA\n");
	if(audioSourceAdjustCt == NULL)
	{
		APP_DBG("audioSourceAdjustCt == NULL\n");
		return;
	}
	memset(audioSourceAdjustCt, 0, sizeof(AudioAdjustContext));

	sra_init(&audioSourceAdjustCt->SraObj,2);//默认蓝牙双声
	audioSourceAdjustCt->SraDecIncNum     = 0;
}


void AudioCoreSourceSRADeinit(void)
{
	APP_DBG("close SRA\n");
	if(audioSourceAdjustCt == NULL)
	{
		APP_DBG("audioSourceAdjustCt == NULL\n");
		return;
	}
	audioSourceAdjustCt->SraDecIncNum     = 0;
}


void AudioCoreSourceSRAStepCnt(void)
{
	static uint32_t i = 0;
	static int8_t  AdjustFlag = 0;//方向
	static int32_t	Samples = 0;

	if(audioSourceAdjustCt == NULL)
	{
		return;
	}

	if(FreqAsyncSource < AUDIO_CORE_SOURCE_MAX_MUN
	&& ((AudioCore.AudioSource[FreqAsyncSource].Enable)))
	{
#ifdef CFG_APP_BT_MODE_EN
		if(GetSystemMode() == AppModeBtAudioPlay)
		{
			Samples += GetValidSbcDataSize();
		}
		else
#endif
		{
			Samples += AudioCore.AudioSource[FreqAsyncSource].FuncDataGetLen();
		}
		if(i >= AUDIO_CORE_SOURCE_SOFT_ADJUST_STEP)
		{
			i = 0;
			Samples /= AUDIO_CORE_SOURCE_SOFT_ADJUST_STEP;//GetValidSbcDataSize();
			//APP_DBG("Samples = %d\n", (int)Samples);

			if(AdjustFlag == 0)
			{
				if(Samples < AdjustLevelLow)
				{
					AdjustFlag = 1;//正
				}
				else if(Samples > AdjustLevelHigh)
				{
					AdjustFlag = -1;//负
				}
				else
				{
					AdjustFlag = 0;
				}
			}
			else if(AdjustFlag == 1)
			{
				if(Samples < AdjustLevelLow)
				{
					//插点
					audioSourceAdjustCt->SraDecIncNum = 1;
					//APP_DBG("!>!");
				}
				else
				{
					AdjustFlag = 0;
				}
			}
			else if(AdjustFlag == -1)
			{
				if(Samples > AdjustLevelHigh)
				{
					//丢点
					audioSourceAdjustCt->SraDecIncNum = -1;
					//APP_DBG("!<!");
				}
				else
				{
					AdjustFlag = 0;
				}
			}
			Samples = 0;
		}
		i++;
	}
}

uint16_t AudioSourceSRAProcess(int16_t *InBuf, uint16_t InLen)
{
	int16_t    AdjustVal;

	if(audioSourceAdjustCt == NULL)
	{
		return 0;
	}

	AdjustVal = audioSourceAdjustCt->SraDecIncNum;
	audioSourceAdjustCt->SraDecIncNum     = 0;

	if(sra_apply(&audioSourceAdjustCt->SraObj, (int16_t *)InBuf, (int16_t *)audioSourceAdjustCt->SraPcmOutnBuf, AdjustVal) != SRA_ERROR_OK)
	{
		return 0;
	}
  //debug only
//	if(AdjustVal != 0)
//	{
//	APP_DBG("* Adjust= %d\n", AdjustVal);
//	}
	return InLen + AdjustVal;
}

#ifdef CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO

AudioAdjustContext*	audioSinkAdjustCt = NULL;
void AudioCoreSinkSRAResMalloc(void)
{
	audioSinkAdjustCt = osPortMalloc(sizeof(AudioAdjustContext));//One Frame
	if(audioSinkAdjustCt == NULL)
	{
		APP_DBG("malloc err, no mem\n");
		return;
	}
}

void AudioCoreSinkSRAResRelease(void)
{
	if(audioSinkAdjustCt != NULL)
	{
		osPortFree(audioSinkAdjustCt);
		audioSinkAdjustCt = NULL;
		APP_DBG("free Audio SRA\n");
	}
}

void AudioCoreSinkSRAInit(void)
{
	APP_DBG("open SRA\n");
	if(audioSinkAdjustCt == NULL)
	{
		APP_DBG("audioSourceAdjustCt == NULL\n");
		return;
	}
	memset(audioSinkAdjustCt, 0, sizeof(AudioAdjustContext));

	sra_init(&audioSinkAdjustCt->SraObj,2);//默认双声道
	audioSinkAdjustCt->SraDecIncNum     = 0;

}


void AudioCoreSinkSRADeinit(void)
{
	APP_DBG("close SRA\n");
	if(audioSinkAdjustCt == NULL)
	{
		APP_DBG("audioSourceAdjustCt == NULL\n");
		return;
	}
	audioSinkAdjustCt->SraDecIncNum     = 0;

}

//目前仅仅支持USB声卡模式下的输出微调
//微调方向和输入完全相反
void AudioCoreSinkSRAStepCnt(void)
{
	if(audioSinkAdjustCt == NULL)
	{
		return;
	}

	if(audioSourceAdjustCt->SraDecIncNum == -1)
	{
		audioSinkAdjustCt->SraDecIncNum = 1;
		//APP_DBG("$>$");
	}
	else if(audioSourceAdjustCt->SraDecIncNum == 1)
	{
		audioSinkAdjustCt->SraDecIncNum = -1;
		//APP_DBG("$<$");
	}
	else
	{
		audioSinkAdjustCt->SraDecIncNum = -0;
	}
}

uint16_t AudioSinkSRAProcess(int16_t *InBuf, uint16_t InLen)
{
	int16_t    AdjustVal;

	if(audioSourceAdjustCt == NULL)
	{
		return 0;
	}

	AdjustVal = audioSinkAdjustCt->SraDecIncNum;
	audioSinkAdjustCt->SraDecIncNum     = 0;

	if(sra_apply(&audioSinkAdjustCt->SraObj, (int16_t *)InBuf, (int16_t *)audioSinkAdjustCt->SraPcmOutnBuf, AdjustVal) != SRA_ERROR_OK)
	{
		return 0;
	}

	return InLen + AdjustVal;
}
#endif
#endif //CFG_FUNC_SOFT_ADJUST_IN

#endif
