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


//��Ƶƫ��ǯλ�������֮��,(�����������ɵ���ʹ������)
#define ADJUST_PERIOD_COUNT			(128 * 64) //ˮλ���� ����������Ƽ�16,32/64/128,���ڼ��㣬�൱��100~2000mS���ҵ���һ�Ρ�
#define ADJUST_PERIOD_BT			1000	//ms �������ʹ�� ΢��ʱ��������

//��Ƶƫֵ���λ��(����)��Ӳ��΢����Ӧregֵ;APLL 288 ��Ƶ��ȡaudioclock��12M��~24+ regΪ������С���֣���λ1/256.һ����λ����ƵƫֵΪ(288/12)*256
#define ADJUST_REG_SCALE			(24*256) //����ת���ɼĴ�������ֵ��(ADJUST_CLOCK_MAX/10000) /(1/ADJUST_REG_SCALE) ��Ӳ��΢�������ֵ��
#define ADJUST_CLOCK_MAX			5 //��λ���֮һ
#define ADJUST_SHRESHOLD			1//1~5 ���Ƽ�ֵ1��������ֵ���ߣ���low~High֮���ˮλ������������������Ӧ����Ƶ�ȼ��١�

static uint16_t AdjustLevelHigh	= 0;	//(CFG_PARA_SAMPLES_PER_FRAME * 3 / 4); //����ǰbuf��֡��С����ȡʱ���ǳ���һ֡��
static uint16_t AdjustLevelLow	= 0;	//(CFG_PARA_SAMPLES_PER_FRAME / 4);
static uint32_t	PeriodCount;			//��ǰͳ�����ڣ���λ������֡����(PeriodCount * mainAppCt.SamplesPreFrame)/CFG_PARA_SAMPLE_RATE ��ͳ�ƺ�΢�������ڡ�
static int8_t 	FreqAsyncSource = 127;	//�첽��Դͨ����127Ϊȱʡֵ��δ���á�
static int32_t  AsyncRemain;
static int16_t  AsyncRemainLast;
static int16_t  AdjustValue = 0;		//ϵͳ��Ƶ��(ƫ��)����ֵ ��С������(��λ��1/256),
static int8_t 	LastRise = 0;			//�ϴ�ˮλ�仯�����1:�� 0��ƽ -1:��
static uint32_t Count;

/**
 * @func        AudioCoreSourceFreqAdjustEnable
 * @brief       ʹ��ϵͳ��Ƶ��Ƶ΢����ʹ�ŵ�֮��ͬ��(���첽��Դ)
 * @param       uint8_t AsyncIndex   �첽��ƵԴ�����ŵ����
 * @param       uint16_t LevelLow   �첽��ƵԴ�����ŵ���ˮλ������ֵ
 * @param       uint16_t LevelHigh   �첽��ƵԴ�����ŵ���ˮλ������ֵ
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
	Clock_AudioPllClockAdjust(PLL_CLK_1, 0, 0);//�ָ�ȱʡֵ
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
	int16_t 	Adjust = 0;//Ϊ�˱��ϵ���Ƶ��ƽ�������ο�΢��1��λ��
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
			AsyncRemain = AsyncRemain / Count;//ˮλ������
			if(AsyncRemainLast != 0xffff)//not first
			{
				if(AsyncRemain > AsyncRemainLast)//ˮλ��
				{
					LevelRise = 1;
					if((AsyncRemain > AdjustLevelHigh) //bufˮλ Խ���������λ
						|| (AsyncRemain >= AdjustLevelLow && (LastRise >= ADJUST_SHRESHOLD)))//���������� ��������
					{
						Adjust = -1; //��ӿ�ϵͳʱ��/���ͷ�Ƶ����ֵ��
					}
				}
				else if(AsyncRemain < AsyncRemainLast)//ˮλ��
				{
					LevelRise = -1;
					if((AsyncRemain < AdjustLevelLow)//bufˮλ Խ���������λ
						|| ((AsyncRemain < AdjustLevelHigh) && (LastRise <= -ADJUST_SHRESHOLD))) //���������� �����µ�
					{
						Adjust = 1; //�轵��ϵͳʱ��/��߷�Ƶ����ֵ
					}
				}
				Adjust += AdjustValue; //����LastƵƫ����ֵ

#ifdef CFG_APP_BT_MODE_EN
				if(Adjust > AdjustMax && (GetSystemMode() == AppModeBtAudioPlay)) //��Ƶƫǯλ
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
				if(Adjust != AdjustValue)//��Ƶֵ�����
				{
					AdjustValue = Adjust; //���汾��Ƶƫ ��Ƶ��
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
						if(AdjustValue > 0) //���ȱʡ����߷�Ƶֵ
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
			AsyncRemain = 0;//�ۻ���������
			Count = 0;
			LastRise = LevelRise;//���ݱ���ˮλ�仯��
		}
	}
}

#ifdef CFG_FUNC_SOFT_ADJUST_IN

AudioAdjustContext*	audioSourceAdjustCt = NULL;

/*
****************************************************************
* ���΢������
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

	sra_init(&audioSourceAdjustCt->SraObj,2);//Ĭ������˫��
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
	static int8_t  AdjustFlag = 0;//����
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
					AdjustFlag = 1;//��
				}
				else if(Samples > AdjustLevelHigh)
				{
					AdjustFlag = -1;//��
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
					//���
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
					//����
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

	sra_init(&audioSinkAdjustCt->SraObj,2);//Ĭ��˫����
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

//Ŀǰ����֧��USB����ģʽ�µ����΢��
//΢�������������ȫ�෴
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
