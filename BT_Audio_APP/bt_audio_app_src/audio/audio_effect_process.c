#include <string.h>
#include <nds32_intrinsic.h>
#include "debug.h"
#include "app_config.h"
#include "ctrlvars.h"
#include "watchdog.h"
#include "rtos_api.h"
#include "audio_effect.h"
#include "audio_effect_library.h"
#include "communication.h"
#include "audio_adc.h"
#include "main_task.h"
#include "math.h"
#include "bt_config.h"
#include "bt_hf_api.h"
#ifdef CFG_FUNC_AI
#include "ai.h"
#endif

#include "stdlib.h"

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
AUDIO_EFF_PARAMAS Audio_mode ;//调音参数缓存

int32_t* pcm_buf_1;
int32_t* pcm_buf_2;
int32_t* pcm_buf_3;
int32_t* pcm_buf_4;
int32_t* pcm_buf_5;
#ifdef CFG_FUNC_GUITAR_EN
int16_t* guitar_pcm = NULL;
#endif
#ifdef CFG_FUNC_ECHO_DENOISE
int16_t*  EchoAudioBuf=NULL;
#endif

void LoadAudioMode(uint16_t len,const uint8_t *buff, uint8_t init_flag);

void EffectPcmBufMalloc(uint32_t SampleLen)
{
	pcm_buf_1 = (int32_t*)osPortMallocFromEnd(SampleLen * 2 * 2);
	if(pcm_buf_1 == NULL)
	{
		APP_DBG("pcm_buf_1 malloc err\n");
		return;
	}
	pcm_buf_2 = (int32_t*)osPortMallocFromEnd(SampleLen * 2 * 2);
	if(pcm_buf_2 == NULL)
	{
		APP_DBG("pcm_buf_2 malloc err\n");
		return;
	}
	pcm_buf_3 = (int32_t*)osPortMallocFromEnd(SampleLen * 2 * 2);
	if(pcm_buf_3 == NULL)
	{
		APP_DBG("pcm_buf_3 malloc err\n");
		return;
	}
	pcm_buf_4 = (int32_t*)osPortMallocFromEnd(SampleLen * 2 * 2);
	if(pcm_buf_4 == NULL)
	{
		APP_DBG("pcm_buf_4 malloc err\n");
		return;
	}
	pcm_buf_5 = (int32_t*)osPortMallocFromEnd(SampleLen * 2 * 2);
	if(pcm_buf_5 == NULL)
	{
		APP_DBG("pcm_buf_5 malloc err\n");
		return;
	}
    #ifdef CFG_FUNC_GUITAR_EN
	guitar_pcm = (int32_t*)osPortMallocFromEnd(SampleLen * 2 * 2);
	if(guitar_pcm == NULL)
	{
		APP_DBG("guitar_pcm malloc err\n");
		return;
	}
    #endif

    #ifdef CFG_FUNC_ECHO_DENOISE
	EchoAudioBuf = (int16_t*)osPortMallocFromEnd(SampleLen * 2 * 2);
    if(EchoAudioBuf == NULL)
	{
		APP_DBG("EchoAudioBuf malloc err\n");
	}
	else
	{
		memset(EchoAudioBuf, 0, SampleLen * 2 * 2);
	}
    #endif

	APP_DBG("EffectPcmBufMalloc OK\n");
}


void EffectPcmBufRelease(void)
{
	if(pcm_buf_1 != NULL)
	{
		osPortFree(pcm_buf_1);
		pcm_buf_1 = NULL;
	}
	if(pcm_buf_2 != NULL)
	{
		osPortFree(pcm_buf_2);
		pcm_buf_2 = NULL;
	}
	if(pcm_buf_3 != NULL)
	{
		osPortFree(pcm_buf_3);
		pcm_buf_3 = NULL;
	}
	if(pcm_buf_4 != NULL)
	{
		osPortFree(pcm_buf_4);
		pcm_buf_4 = NULL;
	}
	if(pcm_buf_5 != NULL)
	{
		osPortFree(pcm_buf_5);
		pcm_buf_5 = NULL;
	}
    #ifdef CFG_FUNC_GUITAR_EN
    if(guitar_pcm != NULL)
	{
		osPortFree(guitar_pcm);
		guitar_pcm = NULL;
	}
    #endif

    #ifdef CFG_FUNC_ECHO_DENOISE
    if(EchoAudioBuf != NULL)
	{
		osPortFree(EchoAudioBuf);
		EchoAudioBuf = NULL;
	}
    #endif
}

void EffectPcmBufClear(uint32_t SampleLen)
{
	if(pcm_buf_1 != NULL)
	{
		memset(pcm_buf_1, 0, SampleLen * 2 * 2);
	}
	if(pcm_buf_2 != NULL)
	{
		memset(pcm_buf_2, 0, SampleLen * 2 * 2);
	}
	if(pcm_buf_3 != NULL)
	{
		memset(pcm_buf_3, 0, SampleLen * 2 * 2);
	}
	if(pcm_buf_4 != NULL)
	{
		memset(pcm_buf_4, 0, SampleLen * 2 * 2);
	}
	if(pcm_buf_5 != NULL)
	{
		memset(pcm_buf_5, 0, SampleLen * 2 * 2);
	}
}

/*
****************************************************************
* 音效模式选择函数
* 1.共预留10组调音参数，可由调音工具导出或下载；
* 2.每组调音参数对应1个音效模式；
****************************************************************
*/
void LoadAudioParamas (uint8_t mode)
{
	uint8_t i = 0;

	APP_DBG("FUNC_ID_EFFECT_MODE -> %d\n", mode);

	Audio_mode.EffectParamas = 0;
	Audio_mode.len           = 0;
	Audio_mode.eff_mode      = 0;
   
	if(mode < 10)//从下载的10组参数对应的flash存储空间中获取
	{
		APP_DBG(" Get Audio Effect Paramas From SPI FLASH!!!!!!\n");
		//ReadEffectParamas(mode,&Audio_mode_buff[0]);
		return;
	}
	else//从调音数组中获取（由调音工具导出)
	{
		APP_DBG("Get Audio Effect Paramas From Data.c!!!!!!\n");

		i = 0;

		while(1)
		{
			if(EFFECT_TAB[i].eff_mode == 0xff)   
			{
				APP_DBG("No Audio effect File load!!!!\n");
				return;
			} 	  

			if(EFFECT_TAB[i].eff_mode == mode)   
			{
				Audio_mode.eff_mode      = EFFECT_TAB[i].eff_mode;
				Audio_mode.EffectParamas = EFFECT_TAB[i].EffectParamas;
	            Audio_mode.len           = EFFECT_TAB[i].len;
				APP_DBG("number:%d , eff_mode:%d , len:%d name:%s\n",i,EFFECT_TAB[i].eff_mode,EFFECT_TAB[i].len,EFFECT_TAB[i].name);
	            return;
			}
			else
			{
			   i++;
			}
		}
	}
}


/*
****************************************************************
* 音效模式选择函数
* 1.共预留10组调音参数，可由调音工具导出或下载；
* 2.每组调音参数对应1个音效模式；
****************************************************************
*/
void AudioEffectModeSel(uint8_t mode, uint8_t init_flag)//0=hw,1=effect,2=hw+effect ff= no init
{
	WDG_Feed();

	LoadAudioParamas(mode);

	if(init_flag == 0xff)///no init
	{
		return;
	}

	if(Audio_mode.EffectParamas == 0)
	{
		return;
	}

	if(init_flag == 0)///only hardware init
	{
		LoadAudioMode(Audio_mode.len,Audio_mode.EffectParamas , init_flag);//0=hw,1=effect,2=hw+effect
		return;
	}

	if(!AudioEffectListJudge(Audio_mode.len,Audio_mode.EffectParamas))
	{
		return;
	}

	AudioEffectsAllDisable();

	LoadAudioMode(Audio_mode.len, Audio_mode.EffectParamas , init_flag);//0=hw,1=effect,2=hw+effect
}

#ifdef CFG_FUNC_MIC_KARAOKE_EN
/*
****************************************************************
* Music+Mic音效处理主函数
*
*
****************************************************************
*/
void AudioEffectProcess(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	int16_t  pcm;
	uint16_t n = mainAppCt.SamplesPreFrame;
	int16_t *mic_pcm    	= NULL;//pBuf->mic_in;///mic input	
	int16_t *bypass_tmp 	= NULL;//pBuf->mic_in;
	int16_t *music_pcm    	= NULL;//pBuf->music_in;///music input
	int16_t *remind_in      = NULL;//pBuf->remind_in;
	int16_t *monitor_out    = NULL;//pBuf->dac0_out; 
	int16_t *record_out     = NULL;//pBuf->dacx_out; 
	int16_t *i2s0_out       = NULL;//pBuf->i2s0_out; 
	int16_t *i2s1_out       = NULL;//pBuf->i2s1_out; 
	int16_t *usb_out        = NULL;//pBuf->usb_out; 
	int16_t *local_rec_out  = NULL;//pBuf->rec_out; 
	int16_t *echo_tmp   	= (int16_t *)pcm_buf_1;
	int16_t *reverb_tmp 	= (int16_t *)pcm_buf_2;
	int16_t *b_e_r_mix_tmp 	= (int16_t *)pcm_buf_2;
	int16_t *rec_bypass_tmp = (int16_t *)pcm_buf_3;
	int16_t *rec_effect_tmp = (int16_t *)pcm_buf_4;
	int16_t *rec_music_tmp  = (int16_t *)pcm_buf_5;
    int16_t *rec_remind_tmp = (int16_t *)pcm_buf_1;
	#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN   
	//int16_t *EqModeAudioBuf   = (int16_t *)pcm_buf_4;		
	#endif

	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Enable == TRUE)////mic buff
	{
		bypass_tmp = mic_pcm = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;//双mic输入
	}

#ifdef CFG_FUNC_RECORDER_EN
	if(GetSystemMode() == AppModeCardPlayBack || GetSystemMode() == AppModeUDiskPlayBack || GetSystemMode() == AppModeFlashFsPlayBack)
	{
		if(pAudioCore->AudioSource[PLAYBACK_SOURCE_NUM].Enable == TRUE)
		{
			music_pcm = pAudioCore->AudioSource[PLAYBACK_SOURCE_NUM].PcmInBuf;// include usb/sd source 
		}
	}
	else
#endif
	{
		if(pAudioCore->AudioSource[APP_SOURCE_NUM].Enable == TRUE)////music buff
		{
			music_pcm = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;// include line/bt/usb/sd/spdif/hdmi/i2s/radio source
		}
	}	
	
#if defined(CFG_FUNC_REMIND_SOUND_EN)	
	if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Enable == TRUE)////remind buff
	{
		remind_in = pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
	}	
#endif

	if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Enable == TRUE)   ////dacx buff
	{
		monitor_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
	}

#ifdef CFG_RES_AUDIO_DACX_EN 	
	if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Enable == TRUE)   ////dacx buff
	{
		record_out = pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
	}	
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN 	
	if(pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Enable == TRUE)	////dacx buff
	{
#if (CFG_RES_I2S_PORT==1)
		i2s1_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
#else
		i2s0_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
#endif
	}
#endif

#ifdef CFG_FUNC_RECORDER_EN
	if(pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].Enable == TRUE)
	{
		local_rec_out = pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_RES_AUDIO_USB_OUT_EN
	if(pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].Enable == TRUE)
	{
		usb_out = pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
    if(music_pcm == NULL)
	{		
		if(mainAppCt.EqModeBak != mainAppCt.EqMode)
		{
			EqModeSet(mainAppCt.EqMode);
			mainAppCt.EqModeBak = mainAppCt.EqMode;
		}
	}
#endif

#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexLock(AudioEffectMutex);
	}
#endif

    if(monitor_out)
	{
		memset(monitor_out, 0, n * 2 * 2);
    }

    if(record_out)
    {
		memset(record_out, 0, n * 2);
    }
	
    if(usb_out)
    {
		memset(usb_out, 0, n * 2 * 2);//mono*2 stereo*4
    }
	
    if(i2s0_out)
    {
		memset(i2s0_out, 0, n * 2 * 2);//mono*2 stereo*4
    }

    EffectPcmBufClear(mainAppCt.SamplesPreFrame);
	
	//伴奏信号音效处理
#ifdef CFG_AI_ENCODE_EN
	if((music_pcm)&& (!SoftFlagGet(SoftFlagAiProcess)))
#else
	if(music_pcm)
#endif
	{
		if((GetSystemMode() == AppModeCardPlayBack) || (GetSystemMode() == AppModeUDiskPlayBack)  || (GetSystemMode() == AppModeFlashFsPlayBack))
		{
			#if CFG_AUDIO_EFFECT_USB_IN_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.usb_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
			
			#if CFG_AUDIO_EFFECT_AUX_GAIN_CONTROL_EN
			{
				#ifdef CFG_FUNC_SHUNNING_EN
				uint32_t gain;
				gain = gCtrlVars.aux_gain_control_unit.gain;
				gCtrlVars.aux_gain_control_unit.gain = gCtrlVars.aux_out_dyn_gain;//闪避功能打开时用到
				#endif
				AudioEffectPregainApply(&gCtrlVars.aux_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
				#ifdef CFG_FUNC_SHUNNING_EN
				gCtrlVars.aux_gain_control_unit.gain = gain;
				#endif
			}		
			#endif			
			
			for(s = 0; s < n; s++)
			{
				rec_music_tmp[2*s + 0] = music_pcm[2*s + 0];
				rec_music_tmp[2*s + 1] = music_pcm[2*s + 1];
			}
			
			AudioCoreAppSourceVolSet(PLAYBACK_SOURCE_NUM, music_pcm, n, 2);
		}
		else
		{
			if(gCtrlVars.adc_line_channel_num == 1)
			{
				for(s = 0; s < n; s++)
				{
					music_pcm[s] = __nds32__clips((((int32_t)music_pcm[2 * s + 0] + (int32_t)music_pcm[2 * s + 1]) ), 16-1);
				}
			}
			
			#if CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN
			AudioEffectExpanderApply(&gCtrlVars.music_expander_unit, music_pcm, music_pcm, n);
			#endif
			
			#if CFG_AUDIO_EFFECT_MUSIC_SILENCE_DECTOR_EN
			AudioEffectSilenceDectorApply(&gCtrlVars.MusicAudioSdct_unit,  music_pcm,  n);
			#endif

			if((GetSystemMode() == AppModeOpticalAudioPlay) || (GetSystemMode() == AppModeCoaxialAudioPlay) || (GetSystemMode() == AppModeHdmiAudioPlay))
			{
				#if CFG_AUDIO_EFFECT_SPDIF_IN_GAIN_CONTROL_EN
				AudioEffectPregainApply(&gCtrlVars.spdif_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
				#endif
			}
			else if(GetSystemMode() == AppModeI2SInAudioPlay)
			{
				#if CFG_AUDIO_EFFECT_I2S_IN_GAIN_CONTROL_EN
				AudioEffectPregainApply(&gCtrlVars.i2s_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
				#endif
			}
			else if((GetSystemMode() == AppModeCardAudioPlay) || (GetSystemMode() == AppModeUDiskAudioPlay) || (GetSystemMode() == AppModeUsbDevicePlay))
			{
				#if CFG_AUDIO_EFFECT_USB_IN_GAIN_CONTROL_EN
				AudioEffectPregainApply(&gCtrlVars.usb_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
				#endif
			}
			else if(GetSystemMode() == AppModeBtAudioPlay)
			{
				#if CFG_AUDIO_EFFECT_BT_IN_GAIN_CONTROL_EN
				AudioEffectPregainApply(&gCtrlVars.bt_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
				#endif
			}
			
			#if CFG_AUDIO_EFFECT_AUX_GAIN_CONTROL_EN
			{
				#ifdef CFG_FUNC_SHUNNING_EN
				uint32_t gain;
				gain = gCtrlVars.aux_gain_control_unit.gain;
				gCtrlVars.aux_gain_control_unit.gain = gCtrlVars.aux_out_dyn_gain;//闪避功能打开时用到
				#endif
				AudioEffectPregainApply(&gCtrlVars.aux_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
				#ifdef CFG_FUNC_SHUNNING_EN
				gCtrlVars.aux_gain_control_unit.gain = gain;
				#endif
			}		
			#endif			
			
			#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN 
			AudioEffectPitchShifterProApply(&gCtrlVars.pitch_shifter_pro_unit, music_pcm, music_pcm, n);
			#endif	
			
			#if CFG_AUDIO_EFFECT_VOCAL_CUT_EN
	        AudioEffectVocalCutApply(&gCtrlVars.vocal_cut_unit, music_pcm, music_pcm, n);
			#endif

			#if CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN
	        AudioEffectVocalRemoveApply(&gCtrlVars.vocal_remove_unit,  music_pcm, music_pcm, n);
	        #endif			
		
			#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	        AudioEffectVBApply(&gCtrlVars.music_vb_unit, music_pcm, music_pcm, n);
			#endif

			#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	        AudioEffectVBClassicApply(&gCtrlVars.music_vb_classic_unit, music_pcm, music_pcm, n);
			#endif
			
	        #if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
	        AudioEffectStereoWidenerApply(&gCtrlVars.stereo_winden_unit, music_pcm, music_pcm, n);
	        #endif

	        #if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	        AudioEffectThreeDApply(&gCtrlVars.music_threed_unit, music_pcm, music_pcm, n);
	        #endif	

			#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
	        AudioEffectThreeDPlusApply(&gCtrlVars.music_threed_plus_unit, music_pcm, music_pcm, n);
	        #endif

			#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
			AudioEffectPcmDelayApply(&gCtrlVars.music_delay_unit, music_pcm, music_pcm, n);
			#endif

			#if CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN
			AudioEffectExciterApply(&gCtrlVars.music_exciter_unit, music_pcm, music_pcm, n);
			#endif

			#if CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN
			AudioEffectEQApply(&gCtrlVars.music_pre_eq_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif

			for(s = 0; s < n; s++)
			{
				rec_music_tmp[2*s + 0] = music_pcm[2*s + 0];
				rec_music_tmp[2*s + 1] = music_pcm[2*s + 1];
			}

			#if CFG_AUDIO_EFFECT_MUSIC_DRC_EN
			AudioEffectDRCApply(&gCtrlVars.music_drc_unit, music_pcm, music_pcm, n);
			#endif

			#if CFG_AUDIO_EFFECT_MUSIC_OUT_EQ_EN		
			AudioEffectEQApply(&gCtrlVars.music_out_eq_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif

			AudioCoreAppSourceVolSet(APP_SOURCE_NUM, music_pcm, n, 2);
			
			if(gCtrlVars.adc_line_channel_num == 1)
			{
				memcpy(&music_pcm[n], music_pcm, n*2);
				for(s = 0; s < n; s++)
				{
					music_pcm[2*s + 0] = music_pcm[2*s + 1] = music_pcm[n+s];
				}
			}
		}
	}
    //MIC信号音效处理
	if(mic_pcm)
	{	
		//pre eq
		#if CFG_AUDIO_EFFECT_MIC_PRE_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_pre_eq_unit, mic_pcm, mic_pcm, n, gCtrlVars.adc_mic_channel_num);
		#endif
		
		#if CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN
		AudioEffectExpanderApply(&gCtrlVars.mic_expander_unit, mic_pcm, mic_pcm, n);
		#endif

		#ifdef CFG_FUNC_GUITAR_EN
		//吉他信号处理
		if(guitar_pcm)
		{
			for(s = 0; s < n; s++)
			{
				guitar_pcm[s]  = mic_pcm[2 * s + 1];//吉他取MIC的右声道
			}
			for(s = 0; s < n; s++)
			{
				mic_pcm[s*2 + 1] = mic_pcm[s*2 + 0]; 
			}
			#if CFG_AUDIO_EFFECT_GUITAR_EQ_EN
	        //AudioEffectEQApply(&gCtrlVars.guitar_eq_unit, guitar_pcm, guitar_pcm, n, 2);
	        #endif

	        #if CFG_AUDIO_EFFECT_AUTOWAH_EN
	        AudioEffectAutoWahApply(&gCtrlVars.auto_wah_unit, guitar_pcm, guitar_pcm, n);
	        #endif

	        #if CFG_AUDIO_EFFECT_CHORUS_EN
	        AudioEffectChorusApply(&gCtrlVars.chorus_unit,  guitar_pcm, guitar_pcm, n);
	        #endif
			
            //********************* mono 2 stereo ****************************
			for(s = 0; s < n; s++)
			{
			    rec_bypass_tmp[s] = guitar_pcm[s];
			}
			for(s = 0; s < n; s++)
			{
			    pcm   =rec_bypass_tmp[s];//
				guitar_pcm[2 * s + 0] = pcm; 
				guitar_pcm[2 * s + 1] = pcm; 
			}	
			
	        #if CFG_AUDIO_EFFECT_PINGPONG_EN				
	        AudioEffectPinPongApply(&gCtrlVars.ping_pong_unit, guitar_pcm, guitar_pcm, n);		
	        #endif

	        #if CFG_AUDIO_EFFECT_GUITAR_ECHO_EN
	        //AudioEffectEchoApply(&gCtrlVars.guitar_echo_unit, guitar_pcm, guitar_pcm, n);
	        #endif

	        #if CFG_AUDIO_EFFECT_GUITAR_GAIN_EN
	        AudioEffectPregainApply(&gCtrlVars.guitar_gain_control_unit, guitar_pcm, guitar_pcm, n, 2);
	        #endif
		}
        #else
		
	    #ifdef CFG_FUNC_DETECT_MIC_EN
		if(gCtrlVars.MicOnlin && (!gCtrlVars.Mic2Onlin))
		{
			#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
			if(gCtrlVars.MicSegment == 3)
			{
				for(s = 0; s < n; s++)
				{
					mic_pcm[s*2 + 1] = mic_pcm[s*2 + 0] = 0; 
				}
			}
			else
			#endif
			{
				for(s = 0; s < n; s++)
				{
					mic_pcm[s*2 + 0] = mic_pcm[s*2 + 1]; 
				}
			}
		}
		else if((!gCtrlVars.MicOnlin) && gCtrlVars.Mic2Onlin)
		{
			for(s = 0; s < n; s++)
			{
				mic_pcm[s*2 + 1] = mic_pcm[s*2 + 0]; 
			}
		}
		else if(gCtrlVars.MicOnlin && gCtrlVars.Mic2Onlin)
		{
			#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
			if(gCtrlVars.MicSegment == 3)
			{
				for(s = 0; s < n; s++)
				{
					mic_pcm[s*2 + 1] = mic_pcm[s*2 + 0]; 
				}
			}
			else
			#endif
			{
				for(s = 0; s < n; s++)
				{
					pcm	= __nds32__clips((((int32_t)mic_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 1]) ), 16-1); 
					mic_pcm[2 * s + 0] = pcm; 
					mic_pcm[2 * s + 1] = pcm; 
				}
			}
		}
		#else
		if( (gCtrlVars.line3_l_mic1_en) && (gCtrlVars.line3_r_mic2_en) )
		{
			for(s = 0; s < n; s++)
			{
				pcm	= __nds32__clips((((int32_t)mic_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 1]) ), 16-1); 
				mic_pcm[2 * s + 0] = pcm; 
				mic_pcm[2 * s + 1] = pcm; 
			}
		}
		else if(gCtrlVars.line3_l_mic1_en )
		{
			for(s = 0; s < n; s++)
			{
				mic_pcm[s*2 + 1] = mic_pcm[s*2 + 0]; 
			}
		}
		else if( gCtrlVars.line3_r_mic2_en )
		{
			for(s = 0; s < n; s++)
			{
				mic_pcm[s*2 + 0] = mic_pcm[s*2 + 1]; 
			}
		}
		#endif
        else
		{
			for(s = 0; s < n; s++)
			{
				mic_pcm[s*2 + 0] = 0;
				mic_pcm[s*2 + 1] = 0; 
			}
		}
		#endif
		
		if(gCtrlVars.adc_mic_channel_num == 1)
		{
			for(s = 0; s < n; s++)
			{
				pcm	= __nds32__clips((((int32_t)mic_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 1]) ), 16-1);
				mic_pcm[s] = pcm;
			}
		}
		
		#ifdef CFG_AI_ENCODE_EN
		if(SoftFlagGet(SoftFlagAiProcess))
		{
#ifdef	 CFG_FUNC_AI_EN
			ai_audio_encode(mic_pcm,n,gCtrlVars.adc_mic_channel_num);
#endif

#ifdef	CFG_XIAOAI_AI_EN
			opus_encode_data(mic_pcm,n,gCtrlVars.adc_mic_channel_num);
#endif
		}
		#endif
		
        #if CFG_AUDIO_EFFECT_MIC_SILENCE_DECTOR_EN
		AudioEffectSilenceDectorApply(&gCtrlVars.MicAudioSdct_unit,  mic_pcm,  n);
		#endif
		
		#if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN || CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN || CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN 
		//freq shifter，howling只支持单声道数据	
		 //********************* stereo 2 mono ****************************  
		if((gCtrlVars.adc_mic_channel_num == 2) && (gCtrlVars.freq_shifter_unit.enable 
			|| gCtrlVars.howling_dector_unit.enable
			#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
			|| gCtrlVars.voice_changer_pro_unit.enable
			#endif
			))
		{
			for(s = 0; s < n; s++)
			{
				// pcm	= __nds32__clips((((int32_t)mic_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 1]) ), 16-1);
				mic_pcm[s] = mic_pcm[2 * s + 0];
			}
		}
		#endif
				
        #if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN
		AudioEffectFreqShifterApply(&gCtrlVars.freq_shifter_unit, mic_pcm, mic_pcm, n);
		#endif
		
        #if CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN
		AudioEffectHowlingSuppressorApply(&gCtrlVars.howling_dector_unit, mic_pcm, mic_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN 
		AudioEffectVoiceChangerProApply(&gCtrlVars.voice_changer_pro_unit, mic_pcm, mic_pcm, n);
		#endif
		
		#if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN || CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN || CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN 
		 //********************* mono 2 stereo ****************************
		if((gCtrlVars.adc_mic_channel_num == 2) && (
				gCtrlVars.freq_shifter_unit.enable 
			|| gCtrlVars.howling_dector_unit.enable
			#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
			|| gCtrlVars.voice_changer_pro_unit.enable
			#endif
			))
		{	
		    for(s = 0; s < n; s++)
			{
			    rec_bypass_tmp[s] = mic_pcm[s];
			}
			for(s = 0; s < n; s++)
			{
			    pcm   =rec_bypass_tmp[s];//
				mic_pcm[2 * s + 0] = pcm; 
				mic_pcm[2 * s + 1] = pcm; 
			}		
		}
		#endif

		#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN 
		AudioEffectVoiceChangerApply(&gCtrlVars.voice_changer_unit, mic_pcm, mic_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN 
		AudioEffectPitchShifterApply(&gCtrlVars.pitch_shifter_unit, mic_pcm, mic_pcm, n, gCtrlVars.adc_mic_channel_num);
		#endif					
		
		#if CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN
		AudioEffectAutoTuneApply(&gCtrlVars.auto_tune_unit, mic_pcm, mic_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_ECHO_EN
		AudioEffectEchoApply(&gCtrlVars.echo_unit, mic_pcm, echo_tmp, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_REVERB_EN
		AudioEffectReverbApply(&gCtrlVars.reverb_unit, mic_pcm, reverb_tmp, n);
		#endif

        #if CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN
		AudioEffectPlateReverbApply(&gCtrlVars.plate_reverb_unit, mic_pcm, reverb_tmp, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
		AudioEffectReverbProApply(&gCtrlVars.reverb_pro_unit, mic_pcm, reverb_tmp, n);
		#endif
		
		//bypass eq
		#if CFG_AUDIO_EFFECT_MIC_BYPASS_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_bypass_eq_unit, bypass_tmp, bypass_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif

        //get bypass data for record
		for(s = 0; s < n; s++)
		{
			rec_bypass_tmp[2*s + 0] = bypass_tmp[2*s + 0];
			rec_bypass_tmp[2*s + 1] = bypass_tmp[2*s + 1];
		}
		
        //echo eq
		#if CFG_AUDIO_EFFECT_MIC_ECHO_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_echo_eq_unit, echo_tmp, echo_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif		
		
        //reverb eq
		#if CFG_AUDIO_EFFECT_MIC_REVERB_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_reverb_eq_unit, reverb_tmp, reverb_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif

        //get echo+reverb data for record 
		for(s = 0; s < n; s++)
		{
			int32_t Data_L = (int32_t)echo_tmp[2*s+0] + (int32_t)reverb_tmp[2*s+0];
			int32_t Data_R = (int32_t)echo_tmp[2*s+1] + (int32_t)reverb_tmp[2*s+1];

			rec_effect_tmp[2 * s + 0] = __nds32__clips((Data_L >> 0), 16-1);
			rec_effect_tmp[2 * s + 1] = __nds32__clips((Data_R >> 0), 16-1);
		}
		
        //bypass pregain
		#if CFG_AUDIO_EFFECT_MIC_BYPASS_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.mic_bypass_gain_control_unit, bypass_tmp, bypass_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif
		
        //echo pregain
		#if CFG_AUDIO_EFFECT_MIC_ECHO_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.mic_echo_control_unit, echo_tmp, echo_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif

		//reverb pregain
		#if CFG_AUDIO_EFFECT_MIC_REVERB_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.mic_reverb_gain_control_unit, reverb_tmp, reverb_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif
		
		//mux
		for(s = 0; s < n; s++)
		{
			if(gCtrlVars.adc_mic_channel_num == 2)
			{
				int32_t Data_L = (int32_t)bypass_tmp[2*s+0] + (int32_t)echo_tmp[2*s+0] + (int32_t)reverb_tmp[2*s+0];
				int32_t Data_R = (int32_t)bypass_tmp[2*s+1] + (int32_t)echo_tmp[2*s+1] + (int32_t)reverb_tmp[2*s+1];

				b_e_r_mix_tmp[2 * s + 0] = __nds32__clips((Data_L >> 0), 16-1);
				b_e_r_mix_tmp[2 * s + 1] = __nds32__clips((Data_R >> 0), 16-1);
			}
			else
			{
				int32_t Data_M = (int32_t)bypass_tmp[s] + (int32_t)echo_tmp[s] + (int32_t)reverb_tmp[s];
				b_e_r_mix_tmp[s] = __nds32__clips((Data_M >> 0), 16-1);
			}
		}

		if(gCtrlVars.adc_mic_channel_num == 1)
		{
			memcpy(&b_e_r_mix_tmp[n], b_e_r_mix_tmp, n*2);
			memcpy(&bypass_tmp[n], bypass_tmp, n*2);
			for(s = 0; s < n; s++)
			{
				b_e_r_mix_tmp[2*s + 0] = b_e_r_mix_tmp[2*s + 1] = b_e_r_mix_tmp[n+s];
				bypass_tmp[2*s + 0]    = bypass_tmp[2*s + 1]    = bypass_tmp[n+s];
			}
		}
		
		#if CFG_AUDIO_EFFECT_MIC_DRC_EN
		AudioEffectDRCApply(&gCtrlVars.mic_drc_unit, b_e_r_mix_tmp, b_e_r_mix_tmp, n);
		#endif
	}	

	#ifdef CFG_FUNC_REMIND_SOUND_EN
	//提示音音效处理
	if(remind_in)
	{
		if(gCtrlVars.remind_type == REMIND_TYPE_BACKGROUND)
		{
		    //get background remind data for record 
	        for(s = 0; s < n; s++)
			{
				rec_remind_tmp[2*s + 0] = remind_in[2*s + 0];
				rec_remind_tmp[2*s + 1] = remind_in[2*s + 1];
			}  
			#ifdef CFG_FUNC_SHUNNING_EN			
			uint32_t gain;
			gain = gCtrlVars.remind_effect_gain_control_unit.gain;
			gCtrlVars.remind_effect_gain_control_unit.gain = gCtrlVars.remind_out_dyn_gain/2;//闪避功能打开时用到
			#endif
			AudioEffectPregainApply(&gCtrlVars.remind_effect_gain_control_unit, remind_in, remind_in, n, 2);
			#ifdef CFG_FUNC_SHUNNING_EN
			gCtrlVars.remind_effect_gain_control_unit.gain = gain;
			#endif			
			
			//if(gCtrlVars.remind_effect_gain_control_unit.enable == 0) remind_in = NULL;
		}
	    else
    	{
			AudioEffectPregainApply(&gCtrlVars.remind_key_gain_control_unit, remind_in, remind_in, n, 2);
			//if(gCtrlVars.remind_key_gain_control_unit.enable == 0) remind_in = NULL;
    	}	

		AudioCoreAppSourceVolSet(REMIND_SOURCE_NUM, remind_in, n, 2);
	}
    #endif

	//DAC0立体声监听音效处理
	#ifdef CFG_AI_ENCODE_EN
	if((monitor_out)&& (!SoftFlagGet(SoftFlagAiProcess)))
	#else
	if(monitor_out)
	#endif
	{			
		#if CFG_AUDIO_EFFECT_MIC_OUT_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_out_eq_unit, b_e_r_mix_tmp, b_e_r_mix_tmp, n, 2);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_OUT_GAIN_CONTROL_EN
		#if 0//CFG_USB_MODE == AUDIO_GAME_HEADSET//电竞耳机上用到
		{
			gCtrlVars.mic_out_gain_control_unit.enable = 1;
	        gCtrlVars.mic_out_gain_control_unit.mute = 0;
	        gCtrlVars.mic_out_gain_control_unit.gain   = DigVolTab_64[gCtrlVars.UsbMicToSpeakerVolume];  		
	        if(gCtrlVars.UsbMicToSpeakerMute) gCtrlVars.mic_out_gain_control_unit.gain = 0;
		}
		#endif
		AudioEffectPregainApply(&gCtrlVars.mic_out_gain_control_unit, b_e_r_mix_tmp, b_e_r_mix_tmp, n, 2);
		#endif		
		
		#ifdef CFG_FUNC_RECORDER_EN
		if(local_rec_out)
		{
			for(s = 0; s < n; s++)
			{
				if(GetWhetherRecMusic() && music_pcm)
				{
					local_rec_out[2*s + 0] = __nds32__clips((((int32_t)music_pcm[2*s + 0] + (int32_t)b_e_r_mix_tmp[2*s + 0])), 16-1);
					local_rec_out[2*s + 1] = __nds32__clips((((int32_t)music_pcm[2*s + 1] + (int32_t)b_e_r_mix_tmp[2*s + 1])), 16-1);
				}
				else
				{
					local_rec_out[2*s + 0] = b_e_r_mix_tmp[2*s + 0];
					local_rec_out[2*s + 1] = b_e_r_mix_tmp[2*s + 1];
				}
			}
		}
		#endif

		AudioCoreAppSourceVolSet(MIC_SOURCE_NUM, b_e_r_mix_tmp, n, 2);

		for(s = 0; s < n; s++)
		{
			#if defined(CFG_FUNC_REMIND_SOUND_EN)//提示音音效处理,若需要linein模式下和提示音同时输出，需要屏蔽掉此部分
			if(remind_in)
			{
				monitor_out[2*s + 0] = __nds32__clips((((int32_t)b_e_r_mix_tmp[2*s + 0] + (int32_t)remind_in[2*s + 0])), 16-1);
				monitor_out[2*s + 1] = __nds32__clips((((int32_t)b_e_r_mix_tmp[2*s + 1] + (int32_t)remind_in[2*s + 1])), 16-1);
			}
			else
	        #endif
        	{
				if(music_pcm)
				{
					monitor_out[2*s + 0] = __nds32__clips((((int32_t)music_pcm[2*s + 0] + (int32_t)b_e_r_mix_tmp[2*s + 0])), 16-1);
					monitor_out[2*s + 1] = __nds32__clips((((int32_t)music_pcm[2*s + 1] + (int32_t)b_e_r_mix_tmp[2*s + 1])), 16-1);
				}
				else
				{
					monitor_out[2*s + 0] = b_e_r_mix_tmp[2*s + 0];
					monitor_out[2*s + 1] = b_e_r_mix_tmp[2*s + 1];
				}
        	}
		}	
		
		//#if defined(CFG_FUNC_REMIND_SOUND_EN)//提示音音效处理，若需要linein模式下和提示音同时输出，需要恢复此部分
		//if(remind_in)
		//{
		//	for(s = 0; s < n; s++)
		//	{
		//		monitor_out[2*s + 0] = __nds32__clips((((int32_t)monitor_out[2*s + 0] + (int32_t)remind_in[2*s + 0])), 16-1);
		//		monitor_out[2*s + 1] = __nds32__clips((((int32_t)monitor_out[2*s + 1] + (int32_t)remind_in[2*s + 1])), 16-1);
		//	}
		//}
        //#endif

		#ifdef CFG_FUNC_GUITAR_EN
		if(guitar_pcm)
		{
			for(s = 0; s < n; s++)
			{
				monitor_out[2*s + 0] = __nds32__clips((((int32_t)monitor_out[2*s + 0] + (int32_t)guitar_pcm[2*s + 0])), 16-1);
				monitor_out[2*s + 1] = __nds32__clips((((int32_t)monitor_out[2*s + 1] + (int32_t)guitar_pcm[2*s + 1])), 16-1);
			}
		}
		#endif
		 
		if(i2s0_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s0_out[2*s + 0] = monitor_out[2*s + 0];
				i2s0_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}
		
		if(i2s1_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s1_out[2*s + 0] = monitor_out[2*s + 0];
				i2s1_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}					
	}

	
	#ifdef CFG_RES_AUDIO_DACX_EN
	//DAC X单声道录音音效处理
	if(record_out)
	{
	    //if(gCtrlVars.reverb_pro_unit.enable)
		//{
		//	memset(rec_bypass_tmp,   0, n * 4);
		//}

		#if CFG_AUDIO_EFFECT_REC_BYPASS_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.rec_bypass_gain_control_unit, rec_bypass_tmp, rec_bypass_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif
		
		#if CFG_AUDIO_EFFECT_REC_EFFECT_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.rec_effect_gain_control_unit, rec_effect_tmp, rec_effect_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif

		#if CFG_AUDIO_EFFECT_REC_AUX_GAIN_CONTROL_EN
		if(music_pcm) AudioEffectPregainApply(&gCtrlVars.rec_aux_gain_control_unit, rec_music_tmp, rec_music_tmp, n, gCtrlVars.adc_line_channel_num);
		#endif

		#ifdef CFG_FUNC_REMIND_SOUND_EN
		if(remind_in && (gCtrlVars.remind_type == REMIND_TYPE_BACKGROUND)) AudioEffectPregainApply(&gCtrlVars.rec_remind_gain_control_unit, rec_remind_tmp, rec_remind_tmp, n, gCtrlVars.adc_line_channel_num);
		#endif
		
		if(music_pcm)
		{
			for(s = 0; s < n; s++)
			{
				record_out[s] = __nds32__clips((((int32_t)rec_effect_tmp[2*s+0] + (int32_t)rec_bypass_tmp[2*s+0] + (int32_t)rec_music_tmp[2*s+0]           
									    + (int32_t)rec_effect_tmp[2*s+1] + (int32_t)rec_bypass_tmp[2*s+1] + (int32_t)rec_music_tmp[2*s+1])), 16-1);
			}
		}
		else
		{
			for(s = 0; s < n; s++)
			{
				record_out[s] = __nds32__clips((((int32_t)rec_effect_tmp[2*s+0] + (int32_t)rec_bypass_tmp[2*s+0] + 0           
						                + (int32_t)rec_effect_tmp[2*s+1] + (int32_t)rec_bypass_tmp[2*s+1] + 0)), 16-1);
			}
		}

		#ifdef CFG_FUNC_REMIND_SOUND_EN
		if(remind_in && (gCtrlVars.remind_type == REMIND_TYPE_BACKGROUND))
		{
			for(s = 0; s < n; s++)
			{
				pcm = __nds32__clips( ((int32_t)rec_remind_tmp[2*s+0]/2 + (int32_t)rec_remind_tmp[2*s+1]/2),16-1); 
				pcm = __nds32__clips( ((int32_t)pcm + (int32_t)record_out[s]),16-1); 
				record_out[s] = pcm;
			}
		}
		#endif	 
		
        #ifdef CFG_FUNC_GUITAR_EN
        if(guitar_pcm)
		{
			for(s = 0; s < n; s++)
			{
				pcm = __nds32__clips( ((int32_t)guitar_pcm[2*s+0] + (int32_t)guitar_pcm[2*s+1]),16-1);
				pcm = __nds32__clips( ((int32_t)pcm + (int32_t)record_out[s]),16-1);
				record_out[s] = pcm;
			}
		}
        #endif				
		
		#if CFG_AUDIO_EFFECT_REC_EQ_EN
		AudioEffectEQApply(&gCtrlVars.rec_eq_unit, record_out, record_out, n, 1);
		#endif

		#if CFG_AUDIO_EFFECT_REC_DRC_EN
		AudioEffectDRCApply(&gCtrlVars.rec_drc_unit, record_out, record_out, n);
		#endif

        #if CFG_AUDIO_EFFECT_PHASE_EN
		AudioEffectPhaseApply(&gCtrlVars.phase_control_unit, record_out, record_out, n, 1);
		#endif
	}
	#endif

	#ifdef CFG_RES_AUDIO_USB_OUT_EN
	if(usb_out)
	{
	    if(!record_out)
		{
			#if CFG_AUDIO_EFFECT_REC_BYPASS_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.rec_bypass_gain_control_unit, rec_bypass_tmp, rec_bypass_tmp, n, gCtrlVars.adc_mic_channel_num);
			#endif
			
			#if CFG_AUDIO_EFFECT_REC_EFFECT_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.rec_effect_gain_control_unit, rec_effect_tmp, rec_effect_tmp, n, gCtrlVars.adc_mic_channel_num);
			#endif

			#if 0//CFG_AUDIO_EFFECT_REC_AUX_GAIN_CONTROL_EN
			if(music_pcm) AudioEffectPregainApply(&gCtrlVars.rec_aux_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif

			#if CFG_AUDIO_EFFECT_REC_REMIND_GAIN_CONTROL_EN
			if(remind_in && (gCtrlVars.remind_type == REMIND_TYPE_BACKGROUND)) AudioEffectPregainApply(&gCtrlVars.rec_remind_gain_control_unit, rec_remind_tmp, rec_remind_tmp, n, gCtrlVars.adc_line_channel_num);
			#endif

    	}
		for(s = 0; s < n; s++)
		{
			//if(music_pcm)
			//{
			//	usb_out[2*s + 0] = __nds32__clips((((int32_t)music_pcm[2*s + 0] + (int32_t)rec_effect_tmp[2*s + 0]  + (int32_t)rec_bypass_tmp[2*s + 0])), 16-1);
			//	usb_out[2*s + 1] = __nds32__clips((((int32_t)music_pcm[2*s + 1] + (int32_t)rec_effect_tmp[2*s + 1]  + (int32_t)rec_bypass_tmp[2*s + 1])), 16-1);
			//}
			//else
			{
				usb_out[2*s + 0] = __nds32__clips((((int32_t)rec_effect_tmp[2*s + 0]  + (int32_t)rec_bypass_tmp[2*s + 0])), 16-1);
				usb_out[2*s + 1] = __nds32__clips((((int32_t)rec_effect_tmp[2*s + 1]  + (int32_t)rec_bypass_tmp[2*s + 1])), 16-1);
			}
		}

		#ifdef CFG_FUNC_REMIND_SOUND_EN
		if(remind_in && (gCtrlVars.remind_type == REMIND_TYPE_BACKGROUND))
		{
			for(s = 0; s < n; s++)
			{
				usb_out[2*s + 0] = __nds32__clips((((int32_t)usb_out[2*s + 0]  + (int32_t)rec_remind_tmp[2*s + 0])), 16-1);
				usb_out[2*s + 1] = __nds32__clips((((int32_t)usb_out[2*s + 1]  + (int32_t)rec_remind_tmp[2*s + 1])), 16-1);
			}
		}
		#endif	
		
		#if CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN
		#if CFG_SUPPORT_USB_VOLUME_SET
        gCtrlVars.rec_usb_out_gain_control_unit.enable = 1;
        gCtrlVars.rec_usb_out_gain_control_unit.mute = 0;
        gCtrlVars.rec_usb_out_gain_control_unit.gain   = DigVolTab_64[gCtrlVars.UsbMicVolume];  		
        if(gCtrlVars.UsbMicMute) gCtrlVars.rec_usb_out_gain_control_unit.gain = 0;
        AudioEffectPregainApply(&gCtrlVars.rec_usb_out_gain_control_unit, usb_out, usb_out, n, 2);			
		#endif
		#endif
		
		#if CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.rec_out_gain_control_unit, usb_out, usb_out, n, 2);
		#endif
	}
	#endif
	#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexUnlock(AudioEffectMutex);
	}
    #endif

}
#else
void AudioMusicProcess(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	int16_t  pcm;
	uint16_t n = mainAppCt.SamplesPreFrame;
	int16_t *music_pcm    	= NULL;//pBuf->music_in;///music input
	int16_t *remind_in      = NULL;//pBuf->remind_in;
	int16_t *monitor_out    = NULL;//pBuf->dac0_out; 
	int16_t *record_out     = NULL;//pBuf->dacx_out; 
	int16_t *i2s0_out       = NULL;//pBuf->i2s0_out; 
	int16_t *i2s1_out       = NULL;//pBuf->i2s1_out; 
	int16_t *usb_out        = NULL;//pBuf->usb_out; 
	int16_t *local_rec_out  = NULL;//pBuf->rec_out; 
	int16_t *rec_music_tmp  = (int16_t *)pcm_buf_1;
	
#ifdef CFG_FUNC_RECORDER_EN
	if(GetSystemMode() == AppModeCardPlayBack || GetSystemMode() == AppModeUDiskPlayBack || GetSystemMode() == AppModeFlashFsPlayBack)
	{
		if(pAudioCore->AudioSource[PLAYBACK_SOURCE_NUM].Enable == TRUE)
		{
			music_pcm = pAudioCore->AudioSource[PLAYBACK_SOURCE_NUM].PcmInBuf;// include usb/sd source 
		}
	}
	else
#endif
	{
		if(pAudioCore->AudioSource[APP_SOURCE_NUM].Enable == TRUE)////music buff
		{
			music_pcm = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;// include line/bt/usb/sd/spdif/hdmi/i2s/radio source
		}
	}	
	
#if defined(CFG_FUNC_REMIND_SOUND_EN)	
	if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Enable == TRUE)////remind buff
	{
		remind_in = pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
	}	
#endif

    if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Enable == TRUE)   ////dacx buff
	{
		monitor_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
	}	
#ifdef CFG_RES_AUDIO_DACX_EN 	
	if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Enable == TRUE)   ////dacx buff
	{
		record_out = pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
	}	
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN 	
	if(pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Enable == TRUE)	////dacx buff
	{
#if (CFG_RES_I2S_PORT==1)
		i2s1_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
#else
		i2s0_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
#endif
	}
#endif

#ifdef CFG_FUNC_RECORDER_EN
	if(pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].Enable == TRUE)
	{
		local_rec_out = pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_RES_AUDIO_USB_OUT_EN
	if(pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].Enable == TRUE)
	{
		usb_out = pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
    if(music_pcm == NULL)
	{		
		if(mainAppCt.EqModeBak != mainAppCt.EqMode)
		{
			EqModeSet(mainAppCt.EqMode);
			mainAppCt.EqModeBak = mainAppCt.EqMode;
		}
	}
#endif

#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexLock(AudioEffectMutex);
	}
#endif

    if(monitor_out)
	{
		memset(monitor_out, 0, n * 2 * 2);
    }

    if(record_out)
    {
		memset(record_out, 0, n * 2);
    }
	
    if(usb_out)
    {
		memset(usb_out, 0, n * 2 * 2);//mono*2 stereo*4
    }
	
    if(i2s0_out)
    {
		memset(i2s0_out, 0, n * 2 * 2);//mono*2 stereo*4
    }

    EffectPcmBufClear(mainAppCt.SamplesPreFrame);
	
	//伴奏信号音效处理
	if(music_pcm)
	{
		if(gCtrlVars.adc_line_channel_num == 1)
		{
			for(s = 0; s < n; s++)
			{
				music_pcm[s] = __nds32__clips((((int32_t)music_pcm[2 * s + 0] + (int32_t)music_pcm[2 * s + 1]) ), 16-1);
			}
		}
		
		if(GetSystemMode() == AppModeLineAudioPlay)
		{
			#if CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN
			AudioEffectExpanderApply(&gCtrlVars.music_expander_unit, music_pcm, music_pcm, n);
			#endif
		}
		
		#if CFG_AUDIO_EFFECT_MUSIC_SILENCE_DECTOR_EN
		AudioEffectSilenceDectorApply(&gCtrlVars.MusicAudioSdct_unit,  music_pcm,  n);
		#endif

		for(s = 0; s < n; s++)
		{
			rec_music_tmp[2*s + 0] = music_pcm[2*s + 0];
			rec_music_tmp[2*s + 1] = music_pcm[2*s + 1];
		}	
		if((GetSystemMode() == AppModeOpticalAudioPlay) || (GetSystemMode() == AppModeCoaxialAudioPlay) || (GetSystemMode() == AppModeHdmiAudioPlay))
		{
			#if CFG_AUDIO_EFFECT_SPDIF_IN_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.spdif_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
		}
		else if(GetSystemMode() == AppModeI2SInAudioPlay)
		{
			#if CFG_AUDIO_EFFECT_I2S_IN_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.i2s_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
		}
		else if((GetSystemMode() == AppModeCardAudioPlay) || (GetSystemMode() == AppModeUDiskAudioPlay) || (GetSystemMode() == AppModeUsbDevicePlay))
		{
			#if CFG_AUDIO_EFFECT_USB_IN_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.usb_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
		}
		else if(GetSystemMode() == AppModeBtAudioPlay)
		{
			#if CFG_AUDIO_EFFECT_BT_IN_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.bt_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
		}
		#if CFG_AUDIO_EFFECT_AUX_GAIN_CONTROL_EN
		{
			#ifdef CFG_FUNC_SHUNNING_EN
			uint32_t gain;
			gain = gCtrlVars.aux_gain_control_unit.gain;
			gCtrlVars.aux_gain_control_unit.gain = gCtrlVars.aux_out_dyn_gain;//闪避功能打开时用到
			#endif
			AudioEffectPregainApply(&gCtrlVars.aux_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#ifdef CFG_FUNC_SHUNNING_EN
			gCtrlVars.aux_gain_control_unit.gain = gain;
			#endif
		}		
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN		
		AudioEffectEQApply(&gCtrlVars.music_pre_eq_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN 
		AudioEffectPitchShifterProApply(&gCtrlVars.pitch_shifter_pro_unit, music_pcm, music_pcm, n);
		#endif	
		
		#if CFG_AUDIO_EFFECT_VOCAL_CUT_EN
        AudioEffectVocalCutApply(&gCtrlVars.vocal_cut_unit, music_pcm, music_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN
        AudioEffectVocalRemoveApply(&gCtrlVars.vocal_remove_unit,  music_pcm, music_pcm, n);
        #endif			
	
		#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
        AudioEffectVBApply(&gCtrlVars.music_vb_unit, music_pcm, music_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
        AudioEffectVBClassicApply(&gCtrlVars.music_vb_classic_unit, music_pcm, music_pcm, n);
		#endif

        #if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
        AudioEffectStereoWidenerApply(&gCtrlVars.stereo_winden_unit, music_pcm, music_pcm, n);
        #endif

		#if CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN
		AudioEffectExciterApply(&gCtrlVars.music_exciter_unit, music_pcm, music_pcm, n);
		#endif

        #if CFG_AUDIO_EFFECT_MUSIC_3D_EN
        AudioEffectThreeDApply(&gCtrlVars.music_threed_unit, music_pcm, music_pcm, n);
        #endif	

		#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
        AudioEffectThreeDPlusApply(&gCtrlVars.music_threed_plus_unit, music_pcm, music_pcm, n);
        #endif

		#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
		AudioEffectPcmDelayApply(&gCtrlVars.music_delay_unit, music_pcm, music_pcm, n);
		#endif
		
		#if CFG_AUDIO_EFFECT_MUSIC_DRC_EN
		AudioEffectDRCApply(&gCtrlVars.music_drc_unit, music_pcm, music_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_OUT_EQ_EN		
		AudioEffectEQApply(&gCtrlVars.music_out_eq_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
		#endif

        #if CFG_AUDIO_EFFECT_PHASE_EN
		AudioEffectPhaseApply(&gCtrlVars.phase_control_unit, music_pcm, music_pcm, n, 1);
		#endif

		AudioCoreAppSourceVolSet(APP_SOURCE_NUM, music_pcm, n, gCtrlVars.adc_line_channel_num);

		if(gCtrlVars.adc_line_channel_num == 1)
		{
			memcpy(&music_pcm[n], music_pcm, n*2);
			for(s = 0; s < n; s++)
			{
				music_pcm[2*s + 0] = music_pcm[2*s + 1] = music_pcm[n+s];
			}
		}
	}		
	//DAC0立体声监听音效处理
	if(monitor_out)
	{	
		#ifdef CFG_FUNC_RECORDER_EN
		if(local_rec_out && music_pcm)
		{
			for(s = 0; s < n; s++)
			{
				local_rec_out[2*s + 0] = music_pcm[2*s + 0];
				local_rec_out[2*s + 1] = music_pcm[2*s + 1];
			}
		}
		#endif
        #if defined(CFG_FUNC_REMIND_SOUND_EN)//提示音音效处理,若需要linein模式下和提示音同时输出，需要屏蔽掉此部分
		if(remind_in)
		{
			AudioCoreAppSourceVolSet(REMIND_SOURCE_NUM, remind_in, n, 2);
		}
		#endif
		for(s = 0; s < n; s++)
		{
			#if defined(CFG_FUNC_REMIND_SOUND_EN)//提示音音效处理,若需要linein模式下和提示音同时输出，需要屏蔽掉此部分
			if(remind_in)
			{
				monitor_out[2*s + 0] = remind_in[2*s + 0];
				monitor_out[2*s + 1] = remind_in[2*s + 1];
			}
			else
			#endif
			{
				if(music_pcm)
				{
					monitor_out[2*s + 0] = music_pcm[2*s + 0];
					monitor_out[2*s + 1] = music_pcm[2*s + 1];
				}
				else
				{
					monitor_out[2*s + 0] = 0;
					monitor_out[2*s + 1] = 0;
				}
			}
		}		

		//#if defined(CFG_FUNC_REMIND_SOUND_EN)//提示音音效处理,若需要linein模式下和提示音同时输出，需要恢复此部分
		//if(remind_in)
		//{
		//	for(s = 0; s < n; s++)
		//	{
		//		monitor_out[2*s + 0] = __nds32__clips((((int32_t)monitor_out[2*s + 0] + (int32_t)remind_in[2*s + 0])), 16-1);
		//		monitor_out[2*s + 1] = __nds32__clips((((int32_t)monitor_out[2*s + 1] + (int32_t)remind_in[2*s + 1])), 16-1);
		//	}
		//}
        //#endif
		
		if(i2s0_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s0_out[2*s + 0] = monitor_out[2*s + 0];
				i2s0_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}
		
		if(i2s1_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s1_out[2*s + 0] = monitor_out[2*s + 0];
				i2s1_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}	
	}
		
	#ifdef CFG_RES_AUDIO_DACX_EN
	//DAC X单声道录音音效处理
	if(record_out)
	{		
		if(music_pcm)
		{
			for(s = 0; s < n; s++)
			{
				record_out[s] = __nds32__clips((((int32_t)rec_music_tmp[2*s+0]/2 + (int32_t)rec_music_tmp[2*s+1]/2)), 16-1);
			}
		}
		else
		{
			for(s = 0; s < n; s++)
			{
				record_out[s] = 0;
			}
		}
		
		#if CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.rec_out_gain_control_unit, record_out, record_out, n, 1);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_OUT_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_out_eq_unit, record_out, record_out, n, 1);
		#endif
				
		#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
		AudioEffectVBApply(&gCtrlVars.rec_vb_unit, record_out, record_out, n);
		#endif

		#if CFG_AUDIO_EFFECT_REC_DRC_EN
		AudioEffectDRCApply(&gCtrlVars.rec_drc_unit, record_out, record_out, n);
		#endif
		
		#if CFG_AUDIO_EFFECT_REC_EQ_EN	
		AudioEffectEQApply(&gCtrlVars.rec_eq_unit, record_out, record_out, n, 1);
		#endif		
	}
	#endif

	#ifdef CFG_RES_AUDIO_USB_OUT_EN
	if(usb_out)
	{
		for(s = 0; s < n; s++)
		{
			if(monitor_out)
			{
				usb_out[2*s + 0] = monitor_out[2*s + 0];
				usb_out[2*s + 1] = monitor_out[2*s + 1];
			}
			else
			{
				usb_out[2*s + 0] = 0;
				usb_out[2*s + 1] = 0;
			}
		}
				
		#if CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN
		#if CFG_SUPPORT_USB_VOLUME_SET
        gCtrlVars.rec_usb_out_gain_control_unit.enable = 1;
        gCtrlVars.rec_usb_out_gain_control_unit.mute = 0;
        gCtrlVars.rec_usb_out_gain_control_unit.gain   = DigVolTab_64[gCtrlVars.UsbMicVolume];  		
        if(gCtrlVars.UsbMicMute) gCtrlVars.rec_usb_out_gain_control_unit.gain = 0;
        AudioEffectPregainApply(&gCtrlVars.rec_usb_out_gain_control_unit, usb_out, usb_out, n, 2);			
		#endif
		#endif
		
		#if CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.rec_out_gain_control_unit, usb_out, usb_out, n, 2);
		#endif
	}
	#endif
	#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexUnlock(AudioEffectMutex);
	}
    #endif
}
#endif

#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE))
/*
****************************************************************
* HFP+Mic音效处理主函数
* 只用于蓝牙通话下K歌录音功能
****************************************************************
*/
#ifdef BT_RECORD_FUNC_ENABLE
void AudioEffectProcessBTRecord(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	int16_t  pcm;
	uint16_t n = mainAppCt.SamplesPreFrame;
	int16_t *mic_pcm		= NULL;//pBuf->mic_in;///mic input	
	int16_t *bypass_tmp 	= NULL;//pBuf->mic_in;
	int16_t *music_pcm		= NULL;//pBuf->music_in;///music input
	int16_t *remind_in		= NULL;//pBuf->remind_in;
	int16_t *monitor_out	= NULL;//pBuf->dac0_out; 
	int16_t *record_out 	= NULL;//pBuf->dacx_out; 
	int16_t *i2s0_out		= NULL;//pBuf->i2s0_out; 
	int16_t *i2s1_out		= NULL;//pBuf->i2s1_out; 
	int16_t *usb_out		= NULL;//pBuf->usb_out; 
	int16_t *local_rec_out	= NULL;//pBuf->rec_out; 
	int16_t *hf_pcm_out     = NULL;//pBuf->hf_pcm_out;//蓝牙通话上传buffer
	
	int16_t *echo_tmp		= (int16_t *)pcm_buf_1;
	int16_t *reverb_tmp 	= (int16_t *)pcm_buf_2;
	int16_t *b_e_r_mix_tmp	= (int16_t *)pcm_buf_2;
	int16_t *rec_bypass_tmp = (int16_t *)pcm_buf_3;
	int16_t *rec_effect_tmp = (int16_t *)pcm_buf_4;
	int16_t *rec_remind_tmp = (int16_t *)pcm_buf_1;

	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Enable == TRUE)////mic buff
	{
		bypass_tmp = mic_pcm = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;//双mic输入
	}

	if(pAudioCore->AudioSource[APP_SOURCE_NUM].Enable == TRUE)////music buff
	{
		music_pcm = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;// include line/bt/usb/sd/spdif/hdmi/i2s/radio source
	}
	
	#if defined(CFG_FUNC_REMIND_SOUND_EN)
	if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Enable == TRUE)////remind buff
	{
		remind_in = pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
	}
	#endif

	if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Enable == TRUE)	////dacx buff
	{
		monitor_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
		
		//hfp send 
		hf_pcm_out = pAudioCore->AudioSink[AUDIO_HF_SCO_SINK_NUM].PcmOutBuf;
	}

	#ifdef CFG_RES_AUDIO_DACX_EN 	
	if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Enable == TRUE)	////dacx buff
	{
		record_out = pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
	}	
	#endif

	#ifdef CFG_FUNC_RECORDER_EN
	if(pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].Enable == TRUE)
	{
		local_rec_out = pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].PcmOutBuf;
	}
	#endif

	#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexLock(AudioEffectMutex);
	}
	#endif

	if(monitor_out)
	{
		memset(monitor_out, 0, n * 2 * 2);
	}

	if(record_out)
	{
		memset(record_out, 0, n * 2);
	}
	
	if(usb_out)
	{
		memset(usb_out, 0, n * 2 * 2);//mono*2 stereo*4
	}
	
	if(i2s0_out)
	{
		memset(i2s0_out, 0, n * 2 * 2);//mono*2 stereo*4
	}

	if(hf_pcm_out) //nomo
	{
		memset(hf_pcm_out, 0, n * 2);
    }
	
	EffectPcmBufClear(mainAppCt.SamplesPreFrame);
	
	//伴奏信号音效处理
	if(music_pcm)
	{
		//nomo -> stereo
		for(s = n; s > 0; s--)
		{
			music_pcm[2*(s-1)] = music_pcm[s-1];
			music_pcm[2*(s-1)+1] = music_pcm[s-1];
		}
		
		#if CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN
		AudioEffectExpanderApply(&gCtrlVars.music_expander_unit, music_pcm, music_pcm, n);
		#endif
		
		#if CFG_AUDIO_EFFECT_MUSIC_SILENCE_DECTOR_EN
		AudioEffectSilenceDectorApply(&gCtrlVars.MusicAudioSdct_unit,  music_pcm,  n);
		#endif

		if((GetSystemMode() == AppModeOpticalAudioPlay) || (GetSystemMode() == AppModeCoaxialAudioPlay) || (GetSystemMode() == AppModeHdmiAudioPlay))
		{
			#if CFG_AUDIO_EFFECT_SPDIF_IN_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.spdif_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
		}
		else if(GetSystemMode() == AppModeI2SInAudioPlay)
		{
			#if CFG_AUDIO_EFFECT_I2S_IN_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.i2s_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
		}
		else if((GetSystemMode() == AppModeCardAudioPlay) || (GetSystemMode() == AppModeUDiskAudioPlay) || (GetSystemMode() == AppModeUsbDevicePlay))
		{
			#if CFG_AUDIO_EFFECT_USB_IN_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.usb_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
		}
		else if(GetSystemMode() == AppModeBtAudioPlay)
		{
			#if CFG_AUDIO_EFFECT_BT_IN_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.bt_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
		}
		
		#if CFG_AUDIO_EFFECT_AUX_GAIN_CONTROL_EN
		{
			#ifdef CFG_FUNC_SHUNNING_EN
			uint32_t gain;
			gain = gCtrlVars.aux_gain_control_unit.gain;
			gCtrlVars.aux_gain_control_unit.gain = gCtrlVars.aux_out_dyn_gain;//闪避功能打开时用到
			#endif
			AudioEffectPregainApply(&gCtrlVars.aux_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#ifdef CFG_FUNC_SHUNNING_EN
			gCtrlVars.aux_gain_control_unit.gain = gain;
			#endif
		}		
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN 
		AudioEffectPitchShifterProApply(&gCtrlVars.pitch_shifter_pro_unit, music_pcm, music_pcm, n);
		#endif	
		
		#if CFG_AUDIO_EFFECT_VOCAL_CUT_EN
		AudioEffectVocalCutApply(&gCtrlVars.vocal_cut_unit, music_pcm, music_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN
		AudioEffectVocalRemoveApply(&gCtrlVars.vocal_remove_unit,  music_pcm, music_pcm, n);
    	#endif			
	
		#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
		AudioEffectVBApply(&gCtrlVars.music_vb_unit, music_pcm, music_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
		AudioEffectVBClassicApply(&gCtrlVars.music_vb_classic_unit, music_pcm, music_pcm, n);
		#endif
		
    	#if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
        AudioEffectStereoWidenerApply(&gCtrlVars.stereo_winden_unit, music_pcm, music_pcm, n);
     	#endif

    	#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
		AudioEffectThreeDApply(&gCtrlVars.music_threed_unit, music_pcm, music_pcm, n);
    	#endif	

		#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
		AudioEffectThreeDPlusApply(&gCtrlVars.music_threed_plus_unit, music_pcm, music_pcm, n);
    	#endif			

		#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
		AudioEffectPcmDelayApply(&gCtrlVars.music_delay_unit, music_pcm, music_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN
		AudioEffectExciterApply(&gCtrlVars.music_exciter_unit, music_pcm, music_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN
		AudioEffectEQApply(&gCtrlVars.music_pre_eq_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_DRC_EN
		AudioEffectDRCApply(&gCtrlVars.music_drc_unit, music_pcm, music_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MUSIC_OUT_EQ_EN
		AudioEffectEQApply(&gCtrlVars.music_out_eq_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
		#endif

		AudioCoreAppSourceVolSet(APP_SOURCE_NUM, music_pcm, n, 2);
		
		if(gCtrlVars.adc_line_channel_num == 1)
		{
			memcpy(&music_pcm[n], music_pcm, n*2);
			for(s = 0; s < n; s++)
			{
				music_pcm[2*s + 0] = music_pcm[2*s + 1] = music_pcm[n+s];
			}
		}
	}
    #ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
    else
	{		
		if(mainAppCt.EqModeBak != mainAppCt.EqMode)
		{
			EqModeSet(mainAppCt.EqMode);
			mainAppCt.EqModeBak = mainAppCt.EqMode;
		}
	}
	#endif
	
	//MIC信号音效处理
	if(mic_pcm)
	{	
    	#ifdef CFG_FUNC_DETECT_MIC_EN
		if(gCtrlVars.MicOnlin && (!gCtrlVars.Mic2Onlin))
		{
			#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
			if(gCtrlVars.MicSegment == 3)
			{
				for(s = 0; s < n; s++)
				{
					mic_pcm[s*2 + 1] = mic_pcm[s*2 + 0] = 0; 
				}
			}
			else
			#endif
			{
				for(s = 0; s < n; s++)
				{
					mic_pcm[s*2 + 0] = mic_pcm[s*2 + 1]; 
				}
			}
		}
		else if((!gCtrlVars.MicOnlin) && gCtrlVars.Mic2Onlin)
		{
			for(s = 0; s < n; s++)
			{
				mic_pcm[s*2 + 1] = mic_pcm[s*2 + 0]; 
			}
		}
		else if(gCtrlVars.MicOnlin && gCtrlVars.Mic2Onlin)
		{
			#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
			if(gCtrlVars.MicSegment == 3)
			{
				for(s = 0; s < n; s++)
				{
					mic_pcm[s*2 + 1] = mic_pcm[s*2 + 0]; 
				}
			}
			else
			#endif
			{
				for(s = 0; s < n; s++)
				{
					pcm = __nds32__clips((((int32_t)mic_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 1]) ), 16-1); 
					mic_pcm[2 * s + 0] = pcm; 
					mic_pcm[2 * s + 1] = pcm; 
				}
			}
		}
		#else
		if( (gCtrlVars.line3_l_mic1_en) && (gCtrlVars.line3_r_mic2_en) )
		{
			for(s = 0; s < n; s++)
			{
				pcm = __nds32__clips((((int32_t)mic_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 1]) ), 16-1); 
				mic_pcm[2 * s + 0] = pcm; 
				mic_pcm[2 * s + 1] = pcm; 
			}
		}
		else if(gCtrlVars.line3_l_mic1_en )
		{
			for(s = 0; s < n; s++)
			{
				mic_pcm[s*2 + 1] = mic_pcm[s*2 + 0]; 
			}
		}
		else if( gCtrlVars.line3_r_mic2_en )
		{
			for(s = 0; s < n; s++)
			{
				mic_pcm[s*2 + 0] = mic_pcm[s*2 + 1]; 
			}
		}
		#endif
		else
		{
			for(s = 0; s < n; s++)
			{
				mic_pcm[s*2 + 0] = 0;
				mic_pcm[s*2 + 1] = 0; 
			}
		}
		if(gCtrlVars.adc_mic_channel_num == 1)
		{
			for(s = 0; s < n; s++)
			{
				pcm = __nds32__clips((((int32_t)mic_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 1]) ), 16-1);
				mic_pcm[s] = pcm;
			}
		}
		
		//pre eq
		#if CFG_AUDIO_EFFECT_MIC_PRE_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_pre_eq_unit, mic_pcm, mic_pcm, n, gCtrlVars.adc_mic_channel_num);
		#endif
		
		#if CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN
		AudioEffectExpanderApply(&gCtrlVars.mic_expander_unit, mic_pcm, mic_pcm, n);
		#endif
		
    	#if CFG_AUDIO_EFFECT_MIC_SILENCE_DECTOR_EN
		AudioEffectSilenceDectorApply(&gCtrlVars.MicAudioSdct_unit,  mic_pcm,  n);
		#endif
		
		#if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN || CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN || CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
		//freq shifter，howling只支持单声道数据	
		 //********************* stereo 2 mono ****************************  
		if((gCtrlVars.adc_mic_channel_num == 2) && (gCtrlVars.freq_shifter_unit.enable 
			|| gCtrlVars.howling_dector_unit.enable
			#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
			|| gCtrlVars.voice_changer_pro_unit.enable
			#endif
			)
			)
		{
			for(s = 0; s < n; s++)
			{
				// pcm	= __nds32__clips((((int32_t)mic_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 1]) ), 16-1);
				mic_pcm[s] = mic_pcm[2 * s + 0];
			}
		}
		#endif
				
    	#if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN
		AudioEffectFreqShifterApply(&gCtrlVars.freq_shifter_unit, mic_pcm, mic_pcm, n);
		#endif
		
    	#if CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN
		AudioEffectHowlingSuppressorApply(&gCtrlVars.howling_dector_unit, mic_pcm, mic_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN 
		AudioEffectVoiceChangerProApply(&gCtrlVars.voice_changer_pro_unit, mic_pcm, mic_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN || CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN || CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
		 //********************* mono 2 stereo ****************************
		if((gCtrlVars.adc_mic_channel_num == 2) && (
				gCtrlVars.freq_shifter_unit.enable 
			|| gCtrlVars.howling_dector_unit.enable
			#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
			|| gCtrlVars.voice_changer_pro_unit.enable
			#endif
			))
		{	
			for(s = 0; s < n; s++)
			{
				rec_bypass_tmp[s] = mic_pcm[s];
			}
			for(s = 0; s < n; s++)
			{
				pcm   =rec_bypass_tmp[s];//
				mic_pcm[2 * s + 0] = pcm; 
				mic_pcm[2 * s + 1] = pcm; 
			}		
		}
		#endif

		#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN 
		AudioEffectVoiceChangerApply(&gCtrlVars.voice_changer_unit, mic_pcm, mic_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN 
		AudioEffectPitchShifterApply(&gCtrlVars.pitch_shifter_unit, mic_pcm, mic_pcm, n, gCtrlVars.adc_mic_channel_num);
		#endif					
		
		#if CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN
		AudioEffectAutoTuneApply(&gCtrlVars.auto_tune_unit, mic_pcm, mic_pcm, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_ECHO_EN
		AudioEffectEchoApply(&gCtrlVars.echo_unit, mic_pcm, echo_tmp, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_REVERB_EN
		AudioEffectReverbApply(&gCtrlVars.reverb_unit, mic_pcm, reverb_tmp, n);
		#endif

    	#if CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN
		AudioEffectPlateReverbApply(&gCtrlVars.plate_reverb_unit, mic_pcm, reverb_tmp, n);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
		AudioEffectReverbProApply(&gCtrlVars.reverb_pro_unit, mic_pcm, reverb_tmp, n);
		#endif
		
		//bypass eq
		#if CFG_AUDIO_EFFECT_MIC_BYPASS_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_bypass_eq_unit, bypass_tmp, bypass_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif

		//get bypass data for record 
		for(s = 0; s < n; s++)
		{
			rec_bypass_tmp[2*s + 0] = bypass_tmp[2*s + 0];
			rec_bypass_tmp[2*s + 1] = bypass_tmp[2*s + 1];
		}
		
		//echo eq
		#if CFG_AUDIO_EFFECT_MIC_ECHO_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_echo_eq_unit, echo_tmp, echo_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif		
		
		//reverb eq
		#if CFG_AUDIO_EFFECT_MIC_REVERB_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_reverb_eq_unit, reverb_tmp, reverb_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif

		//get echo+reverb data for record 
		for(s = 0; s < n; s++)
		{
			int32_t Data_L = (int32_t)echo_tmp[2*s+0] + (int32_t)reverb_tmp[2*s+0];
			int32_t Data_R = (int32_t)echo_tmp[2*s+1] + (int32_t)reverb_tmp[2*s+1];

			rec_effect_tmp[2 * s + 0] = __nds32__clips((Data_L >> 0), 16-1);
			rec_effect_tmp[2 * s + 1] = __nds32__clips((Data_R >> 0), 16-1);
		}
		
		//bypass pregain
		#if CFG_AUDIO_EFFECT_MIC_BYPASS_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.mic_bypass_gain_control_unit, bypass_tmp, bypass_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif
		
		//echo pregain
		#if CFG_AUDIO_EFFECT_MIC_ECHO_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.mic_echo_control_unit, echo_tmp, echo_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif

		//reverb pregain
		#if CFG_AUDIO_EFFECT_MIC_REVERB_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.mic_reverb_gain_control_unit, reverb_tmp, reverb_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif
		
		//mux
		for(s = 0; s < n; s++)
		{
			if(gCtrlVars.adc_mic_channel_num == 2)
			{
				int32_t Data_L = (int32_t)bypass_tmp[2*s+0] + (int32_t)echo_tmp[2*s+0] + (int32_t)reverb_tmp[2*s+0];
				int32_t Data_R = (int32_t)bypass_tmp[2*s+1] + (int32_t)echo_tmp[2*s+1] + (int32_t)reverb_tmp[2*s+1];

				b_e_r_mix_tmp[2 * s + 0] = __nds32__clips((Data_L >> 0), 16-1);
				b_e_r_mix_tmp[2 * s + 1] = __nds32__clips((Data_R >> 0), 16-1);
			}
			else
			{
				int32_t Data_M = (int32_t)bypass_tmp[s] + (int32_t)echo_tmp[s] + (int32_t)reverb_tmp[s];
				b_e_r_mix_tmp[s] = __nds32__clips((Data_M >> 0), 16-1);
			}
		}

		if(gCtrlVars.adc_mic_channel_num == 1)
		{
			memcpy(&b_e_r_mix_tmp[n], b_e_r_mix_tmp, n*2);
			memcpy(&bypass_tmp[n], bypass_tmp, n*2);
			for(s = 0; s < n; s++)
			{
				b_e_r_mix_tmp[2*s + 0] = b_e_r_mix_tmp[2*s + 1] = b_e_r_mix_tmp[n+s];
				bypass_tmp[2*s + 0]    = bypass_tmp[2*s + 1]	= bypass_tmp[n+s];
			}
		}
		
		#if CFG_AUDIO_EFFECT_MIC_DRC_EN
		AudioEffectDRCApply(&gCtrlVars.mic_drc_unit, b_e_r_mix_tmp, b_e_r_mix_tmp, n);
		#endif
	}	

	if(hf_pcm_out)
	{
		for(s=0;s<n;s++)
		{
			hf_pcm_out[s] = __nds32__clips((((int32_t)b_e_r_mix_tmp[2 * s + 0] + (int32_t)b_e_r_mix_tmp[2 * s + 1])), 16-1);
		}
	}
	
	//DAC0立体声监听音效处理
	if(monitor_out)
	{			
		#if CFG_AUDIO_EFFECT_MIC_OUT_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_out_eq_unit, b_e_r_mix_tmp, b_e_r_mix_tmp, n, 2);
		#endif

		#if CFG_AUDIO_EFFECT_MIC_OUT_GAIN_CONTROL_EN
		#if 0//CFG_USB_MODE == AUDIO_GAME_HEADSET//电竞耳机上用到
		{
			gCtrlVars.mic_out_gain_control_unit.enable = 1;
			gCtrlVars.mic_out_gain_control_unit.mute = 0;
			gCtrlVars.mic_out_gain_control_unit.gain   = DigVolTab_64[gCtrlVars.UsbMicToSpeakerVolume]; 		
			if(gCtrlVars.UsbMicToSpeakerMute) gCtrlVars.mic_out_gain_control_unit.gain = 0;
		}
		#endif
		AudioEffectPregainApply(&gCtrlVars.mic_out_gain_control_unit, b_e_r_mix_tmp, b_e_r_mix_tmp, n, 2);
		#endif

		#ifdef CFG_FUNC_RECORDER_EN
		if(local_rec_out)
		{
			for(s = 0; s < n; s++)
			{
				if(GetWhetherRecMusic() && music_pcm)
				{
					local_rec_out[2*s + 0] = __nds32__clips((((int32_t)music_pcm[2*s + 0] + (int32_t)b_e_r_mix_tmp[2*s + 0])), 16-1);
					local_rec_out[2*s + 1] = __nds32__clips((((int32_t)music_pcm[2*s + 1] + (int32_t)b_e_r_mix_tmp[2*s + 1])), 16-1);
				}
				else
				{
					local_rec_out[2*s + 0] = b_e_r_mix_tmp[2*s + 0];
					local_rec_out[2*s + 1] = b_e_r_mix_tmp[2*s + 1];
				}
			}
		}
		#endif

		AudioCoreAppSourceVolSet(MIC_SOURCE_NUM, b_e_r_mix_tmp, n, 2);

		for(s = 0; s < n; s++)
		{
			#if defined(CFG_FUNC_REMIND_SOUND_EN)//提示音音效处理,若需要linein模式下和提示音同时输出，需要屏蔽掉此部分
			if(remind_in)
			{
				AudioCoreAppSourceVolSet(REMIND_SOURCE_NUM, remind_in, n, 2);
				
				monitor_out[2*s + 0] = __nds32__clips((((int32_t)b_e_r_mix_tmp[2*s + 0] + (int32_t)remind_in[2*s + 0])), 16-1);
				monitor_out[2*s + 1] = __nds32__clips((((int32_t)b_e_r_mix_tmp[2*s + 1] + (int32_t)remind_in[2*s + 1])), 16-1);
			}
			else
	    	#endif
    		{
				if(music_pcm)
				{
					monitor_out[2*s + 0] = __nds32__clips((((int32_t)music_pcm[2*s + 0] + (int32_t)b_e_r_mix_tmp[2*s + 0])), 16-1);
					monitor_out[2*s + 1] = __nds32__clips((((int32_t)music_pcm[2*s + 1] + (int32_t)b_e_r_mix_tmp[2*s + 1])), 16-1);
				}
				else
				{
					monitor_out[2*s + 0] = b_e_r_mix_tmp[2*s + 0];
					monitor_out[2*s + 1] = b_e_r_mix_tmp[2*s + 1];
				}
    		}
		}		
		
    	//#if defined(CFG_FUNC_REMIND_SOUND_EN)//提示音音效处理,若需要linein模式下和提示音同时输出，需要恢复此部分
		//if(remind_in)
		//{
		//	for(s = 0; s < n; s++)
		//	{
		//		monitor_out[2*s + 0] = __nds32__clips((((int32_t)monitor_out[2*s + 0] + (int32_t)remind_in[2*s + 0])), 16-1);
		//		monitor_out[2*s + 1] = __nds32__clips((((int32_t)monitor_out[2*s + 1] + (int32_t)remind_in[2*s + 1])), 16-1);
		//	}
		//}
    	//#endif

		if(i2s0_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s0_out[2*s + 0] = monitor_out[2*s + 0];
				i2s0_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}
		
		if(i2s1_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s1_out[2*s + 0] = monitor_out[2*s + 0];
				i2s1_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}	
	}
		
	#ifdef CFG_RES_AUDIO_DACX_EN
	//DAC X单声道录音音效处理
	if(record_out)
	{
		//if(gCtrlVars.reverb_pro_unit.enable)
		//{
		//	memset(rec_bypass_tmp,	 0, n * 4);
		//}

		#if CFG_AUDIO_EFFECT_REC_BYPASS_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.rec_bypass_gain_control_unit, rec_bypass_tmp, rec_bypass_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif
		
		#if CFG_AUDIO_EFFECT_REC_EFFECT_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.rec_effect_gain_control_unit, rec_effect_tmp, rec_effect_tmp, n, gCtrlVars.adc_mic_channel_num);
		#endif

		#if CFG_AUDIO_EFFECT_REC_AUX_GAIN_CONTROL_EN
		if(music_pcm) AudioEffectPregainApply(&gCtrlVars.rec_aux_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
		#endif
		
		if(music_pcm)
		{
			for(s = 0; s < n; s++)
			{
				record_out[s] = __nds32__clips((((int32_t)rec_effect_tmp[2*s+0] + (int32_t)rec_bypass_tmp[2*s+0] + (int32_t)music_pcm[2*s+0]		   
										+ (int32_t)rec_effect_tmp[2*s+1] + (int32_t)rec_bypass_tmp[2*s+1] + (int32_t)music_pcm[2*s+1])), 16-1);
			}
		}
		else
		{
			for(s = 0; s < n; s++)
			{
				record_out[s] = __nds32__clips((((int32_t)rec_effect_tmp[2*s+0] + (int32_t)rec_bypass_tmp[2*s+0] + 0		   
										+ (int32_t)rec_effect_tmp[2*s+1] + (int32_t)rec_bypass_tmp[2*s+1] + 0)), 16-1);
			}
		}
				
		#if CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.rec_out_gain_control_unit, record_out, record_out, n, 1);
		#endif
		
		#if CFG_AUDIO_EFFECT_REC_EQ_EN
		AudioEffectEQApply(&gCtrlVars.rec_eq_unit, record_out, record_out, n, 1);
		#endif

		#if CFG_AUDIO_EFFECT_REC_DRC_EN
		AudioEffectDRCApply(&gCtrlVars.rec_drc_unit, record_out, record_out, n);
		#endif

    	#if CFG_AUDIO_EFFECT_PHASE_EN
		AudioEffectPhaseApply(&gCtrlVars.phase_control_unit, record_out, record_out, n, 1);
		#endif
	}
	#endif

	#if CFG_USB_OUT_EN
	#if CFG_USB_OUT_STEREO_EN
	if(usb_out)
	{
		if(!record_out)
		{
			#if CFG_AUDIO_EFFECT_REC_BYPASS_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.rec_bypass_gain_control_unit, rec_bypass_tmp, rec_bypass_tmp, n, gCtrlVars.adc_mic_channel_num);
			#endif
			
			#if CFG_AUDIO_EFFECT_REC_EFFECT_GAIN_CONTROL_EN
			AudioEffectPregainApply(&gCtrlVars.rec_effect_gain_control_unit, rec_effect_tmp, rec_effect_tmp, n, gCtrlVars.adc_mic_channel_num);
			#endif

			#if CFG_AUDIO_EFFECT_REC_AUX_GAIN_CONTROL_EN
			if(music_pcm) AudioEffectPregainApply(&gCtrlVars.rec_aux_gain_control_unit, music_pcm, music_pcm, n, gCtrlVars.adc_line_channel_num);
			#endif
		}
		for(s = 0; s < n; s++)
		{
			if(music_pcm)
			{
				usb_out[2*s + 0] = __nds32__clips((((int32_t)music_pcm[2*s + 0] + (int32_t)rec_effect_tmp[2*s + 0]	+ (int32_t)rec_bypass_tmp[2*s + 0])), 16-1);
				usb_out[2*s + 1] = __nds32__clips((((int32_t)music_pcm[2*s + 1] + (int32_t)rec_effect_tmp[2*s + 1]	+ (int32_t)rec_bypass_tmp[2*s + 1])), 16-1);
			}
			else
			{
				usb_out[2*s + 0] = __nds32__clips((((int32_t)rec_effect_tmp[2*s + 0]  + (int32_t)rec_bypass_tmp[2*s + 0])), 16-1);
				usb_out[2*s + 1] = __nds32__clips((((int32_t)rec_effect_tmp[2*s + 1]  + (int32_t)rec_bypass_tmp[2*s + 1])), 16-1);
			}
		}
				
		#if CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN
		#if CFG_SUPPORT_USB_VOLUME_SET
		gCtrlVars.rec_usb_out_gain_control_unit.enable = 1;
		gCtrlVars.rec_usb_out_gain_control_unit.mute = 0;
		gCtrlVars.rec_usb_out_gain_control_unit.gain   = DigVolTab_64[gCtrlVars.UsbMicVolume];			
		if(gCtrlVars.UsbMicMute) gCtrlVars.rec_usb_out_gain_control_unit.gain = 0;
		AudioEffectPregainApply(&gCtrlVars.rec_usb_out_gain_control_unit, usb_out, usb_out, n, 2);			
		#endif
		#endif
		
		#if CFG_AUDIO_EFFECT_REC_OUT_GAIN_CONTROL_EN
		AudioEffectPregainApply(&gCtrlVars.rec_out_gain_control_unit, usb_out, usb_out, n, 2);
		#endif
	}
	#endif
	#endif
	#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexUnlock(AudioEffectMutex);
	}
	#endif
}
#endif
/*
****************************************************************
* HFP+Mic音效处理主函数
* 用于正常蓝牙通话
****************************************************************
*/
void AudioEffectProcessBTHF(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	uint16_t n = mainAppCt.SamplesPreFrame;

	int16_t *remind_in      = NULL;//pBuf->remind_in;
	int16_t *monitor_out    = NULL;//pBuf->dac0_out;
	int16_t *record_out     = NULL;//pBuf->dacx_out;
	int16_t *i2s0_out       = NULL;//pBuf->i2s0_out;
	int16_t *i2s1_out       = NULL;//pBuf->i2s1_out;
	int16_t *usb_out        = NULL;//pBuf->usb_out;
#ifdef CFG_FUNC_RECORDER_EN
//	int16_t *local_rec_out  = NULL;//pBuf->rec_out;
#endif
	int16_t  *hf_mic_in     = NULL;//pBuf->hf_mic_in;//蓝牙通话mic采样buffer
	int16_t  *hf_pcm_in     = NULL;//pBuf->hf_pcm_in;//蓝牙通话下传buffer
	int16_t  *hf_aec_in		= NULL;//pBuf->hf_aec_in;;//蓝牙通话下传delay buffer,专供aec算法处理
	int16_t  *hf_pcm_out    = NULL;//pBuf->hf_pcm_out;//蓝牙通话上传buffer
	int16_t  *hf_dac_out    = NULL;//pBuf->hf_dac_out;//蓝牙通话DAC的buffer
	int16_t  *hf_rec_out    = NULL;//pBuf->hf_rec_out;//蓝牙通话送录音的buffer
	int16_t  *u_pcm_tmp     = (int16_t *)pcm_buf_3;
	int16_t  *d_pcm_tmp     = (int16_t *)pcm_buf_4;


	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Enable == TRUE)////mic buff
	{
		hf_mic_in = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;
	}

	if(pAudioCore->AudioSource[APP_SOURCE_NUM].Enable == TRUE)////music buff
	{
		//hf sco: nomo
		hf_pcm_in = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;

		//aec process:push the new data, pop the old data
		if(BtHf_AECRingDataSpaceLenGet() > n)
			BtHf_AECRingDataSet(hf_pcm_in, n);
		hf_aec_in = BtHf_AecInBuf(n);
	}

#if defined(CFG_FUNC_REMIND_SOUND_EN)
	if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Enable == TRUE)////remind buff
	{
		remind_in = pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
	}
#endif

    if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Enable == TRUE)   ////dacx buff
	{
		//hf mode
		hf_dac_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
		hf_pcm_out = pAudioCore->AudioSink[AUDIO_HF_SCO_SINK_NUM].PcmOutBuf;
	}

#ifdef CFG_RES_AUDIO_DACX_EN
	if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Enable == TRUE)   ////dacx buff
	{
	#if (BT_HFP_SUPPORT == ENABLE)
		if(GetSystemMode() == AppModeBtHfPlay)
		{
			record_out = NULL;
		}
		else
	#endif
		{
			record_out = pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
		}
	}
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	if(pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Enable == TRUE)	////dacx buff
	{
#if (CFG_RES_I2S_PORT==1)
		i2s1_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
#else
		i2s0_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
#endif
	}
#endif

#ifdef CFG_FUNC_RECORDER_EN
//	if(pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].Enable == TRUE)
//	{
//		local_rec_out = pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].PcmOutBuf;
//	}
#endif

    if(monitor_out)
	{
		memset(monitor_out, 0, n * 2 * 2);
    }

    if(record_out)
    {
		memset(record_out, 0, n * 2);
    }

    if(usb_out)
    {
		memset(usb_out, 0, n * 2 * 2);//mono*2 stereo*4
    }

    if(i2s0_out)
    {
		memset(i2s0_out, 0, n * 2 * 2);//mono*2 stereo*4
	}

	if(hf_pcm_out)
	{
		memset(hf_pcm_out, 0, n * 2);
	}

	if(hf_dac_out)
	{
		memset(hf_dac_out, 0, n * 2 * 2);
	}

	if(hf_rec_out)
	{
		memset(hf_rec_out, 0, n * 2);
	}

	if(hf_mic_in && hf_pcm_in && hf_pcm_out && hf_dac_out)
	{
		//hf_pcm_in: 16K nomo //256*2
		//hf_mic_in: 16K stereo (需要转成nomo)  //256*2*2
		//hf_pcm_out: 16K nomo //256*2
		//hf_dac_out: 16K stereo //256*2*2

		//手机端通话音频送DAC 16K nomo -> stereo
		/*for(s = 0; s < n; s++)
		{
			hf_dac_out[s*2 + 0] = hf_pcm_in[s];
			hf_dac_out[s*2 + 1] = hf_pcm_in[s];
		}*/

		AudioCoreAppSourceVolSet(APP_SOURCE_NUM, hf_pcm_in, n, 1);

#ifdef CFG_FUNC_REMIND_SOUND_EN//提示音音效处理
		if(remind_in)
		{
			AudioCoreAppSourceVolSet(REMIND_SOURCE_NUM, remind_in, n, 2);
			
			for(s = 0; s < n; s++)
			{
				hf_dac_out[2*s + 0] = __nds32__clips((((int32_t)hf_pcm_in[s] + (int32_t)remind_in[2*s + 0])), 16-1);
				hf_dac_out[2*s + 1] = __nds32__clips((((int32_t)hf_pcm_in[s] + (int32_t)remind_in[2*s + 1])), 16-1);
			}
		}
		else
#endif
		{
			for(s = 0; s < n; s++)
			{
				hf_dac_out[s*2 + 0] = hf_pcm_in[s];
				hf_dac_out[s*2 + 1] = hf_pcm_in[s];
			}
		}

        //本地MIC采样做降噪处理
		#if CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN
		//AudioEffectExpanderApply(&gCtrlVars.mic_expander_unit, hf_mic_in, hf_mic_in, n);
		#endif

		AudioCoreAppSourceVolSet(MIC_SOURCE_NUM, hf_mic_in, n, 2);

		#if CFG_AUDIO_EFFECT_MIC_BYPASS_EQ_EN
		AudioEffectEQApply(&gCtrlVars.mic_bypass_eq_unit, hf_mic_in, hf_mic_in, n, 2);
		#endif

		//本地MIC采样及手机端通话音频做AEC处理
		//only mic input : stereo -> mono
	    for(s = 0; s < n; s++)
		{
			d_pcm_tmp[s] = __nds32__clips((((int32_t)hf_mic_in[2 * s + 0] + (int32_t)hf_mic_in[2 * s + 1])), 16-1);
			u_pcm_tmp[s] = hf_aec_in[s];
		}		
		
	    #if CFG_AUDIO_EFFECT_MIC_AEC_EN
		AudioEffectAecApply(&gCtrlVars.mic_aec_unit, u_pcm_tmp , d_pcm_tmp, hf_pcm_out, n);
	    #else
		for(s = 0; s < n; s++)
		{
		    hf_pcm_out[s] = d_pcm_tmp[s];
		}
	    #endif

		/*for(s = 0; s < n; s++)
		{
		    pcm   =rec_bypass_tmp[s];//
			hf_pcm_out[2 * s + 0] = pcm;
			hf_pcm_out[2 * s + 1] = pcm;
		}*/

		//AEC处理之后的数据做变调处理
		#if ((CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN)&&defined(BT_HFP_MIC_PITCH_SHIFTER_FUNC))
		AudioEffectPitchShifterApply(&gCtrlVars.pitch_shifter_unit, hf_pcm_out, hf_pcm_out, n, 1);//nomo
		#endif
		
	}
	else if(hf_dac_out)
	{
	#ifdef CFG_FUNC_REMIND_SOUND_EN//提示音音效处理
		if(remind_in)
		{
			AudioCoreAppSourceVolSet(REMIND_SOURCE_NUM, remind_in, n, 2);
			
			for(s = 0; s < n; s++)
			{
				hf_dac_out[2*s + 0] = remind_in[2*s + 0];
				hf_dac_out[2*s + 1] = remind_in[2*s + 1];
			}
		}
	#endif
	}
	
	//DAC0立体声监听音效处理
	if(hf_dac_out)
	{
		if(i2s0_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s0_out[2*s + 0] = hf_dac_out[2*s + 0];
				i2s0_out[2*s + 1] = hf_dac_out[2*s + 1];
			}
		}
		if(i2s1_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s1_out[2*s + 0] = hf_dac_out[2*s + 0];
				i2s1_out[2*s + 1] = hf_dac_out[2*s + 1];
			}
		}
	}

	#ifdef CFG_RES_AUDIO_DACX_EN
	//DAC X单声道录音音效处理
	if(record_out)
	{
		for(s = 0; s < n; s++)
		{
			record_out[s] = hf_dac_out[s];
		}

	}
	#endif
}
#endif

#else
int32_t pcm_buf_3[512];
int32_t pcm_buf_4[512];

/*
****************************************************************
* 无音效处理主函数
*
*
****************************************************************
*/
void AudioBypassProcess(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	uint16_t n = mainAppCt.SamplesPreFrame;
	int16_t *mic_pcm    	= NULL;//pBuf->mic_in;///mic input	
	int16_t *music_pcm    	= NULL;//pBuf->music_in;///music input
	int16_t *remind_in      = NULL;//pBuf->remind_in;
	int16_t *monitor_out    = NULL;//pBuf->dac0_out; 
	int16_t *record_out     = NULL;//pBuf->dacx_out; 
	int16_t *i2s0_out       = NULL;//pBuf->i2s0_out; 
	int16_t *i2s1_out       = NULL;//pBuf->i2s1_out; 
	int16_t *usb_out        = NULL;//pBuf->usb_out; 
	int16_t *local_rec_out  = NULL;//pBuf->rec_out; 
	#if (BT_HFP_SUPPORT == ENABLE)
	int16_t  *hf_mic_in     = NULL;//pBuf->hf_mic_in;//蓝牙通话mic采样buffer
	int16_t  *hf_pcm_in     = NULL;//pBuf->hf_pcm_in;//蓝牙通话下传buffer
	int16_t  *hf_aec_in		= NULL;//pBuf->hf_aec_in;;//蓝牙通话下传delay buffer,专供aec算法处理
	int16_t  *hf_pcm_out    = NULL;//pBuf->hf_pcm_out;//蓝牙通话上传buffer
	int16_t  *hf_dac_out    = NULL;//pBuf->hf_dac_out;//蓝牙通话DAC的buffer	
	int16_t  *hf_rec_out    = NULL;//pBuf->hf_rec_out;//蓝牙通话送录音的buffer	
	int16_t  *u_pcm_tmp     = (int16_t *)pcm_buf_3;
	int16_t  *d_pcm_tmp     = (int16_t *)pcm_buf_4;
	#endif

	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Enable == TRUE)////mic buff
	{
#if (BT_HFP_SUPPORT == ENABLE)
		if(GetSystemMode() == AppModeBtHfPlay)
		{
			//hf mode
			hf_mic_in = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;
		}
		else
#endif
		{
			mic_pcm = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;//双mic输入
		}
	}

#ifdef CFG_FUNC_RECORDER_EN
	if(GetSystemMode() == AppModeCardPlayBack || GetSystemMode() == AppModeUDiskPlayBack || GetSystemMode() == AppModeFlashFsPlayBack)
	{
		if(pAudioCore->AudioSource[PLAYBACK_SOURCE_NUM].Enable == TRUE)
		{
			music_pcm = pAudioCore->AudioSource[PLAYBACK_SOURCE_NUM].PcmInBuf;// include usb/sd source
		}
	}
	else
#endif
	{
		if(pAudioCore->AudioSource[APP_SOURCE_NUM].Enable == TRUE)////music buff
		{
#if (BT_HFP_SUPPORT == ENABLE) && defined(CFG_APP_BT_MODE_EN)
			if(GetSystemMode() == AppModeBtHfPlay)
			{
				//hf sco: nomo
				hf_pcm_in = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;

				//aec process:push the new data, pop the old data
				/*if(BtHf_AECRingDataSpaceLenGet() > CFG_BTHF_PARA_SAMPLES_PER_FRAME)
					BtHf_AECRingDataSet(hf_pcm_in, CFG_BTHF_PARA_SAMPLES_PER_FRAME);
				hf_aec_in = BtHf_AecInBuf();*/
			}
			else
#endif
			{
				music_pcm = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;// include line/bt/usb/sd/spdif/hdmi/i2s/radio source
			}
		}
	}	
	
#if defined(CFG_FUNC_REMIND_SOUND_EN)	
	if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Enable == TRUE)////remind buff
	{
		remind_in = pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
	}	
#endif

    if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Enable == TRUE)   ////dacx buff
	{
	#if defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)
		if(GetSystemMode() == AppModeBtHfPlay)
		{
			//hf mode
			hf_dac_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
			hf_pcm_out = pAudioCore->AudioSink[AUDIO_HF_SCO_SINK_NUM].PcmOutBuf;
		}
		else
	#endif
		{
			monitor_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
		}
	}
	
#ifdef CFG_RES_AUDIO_DACX_EN 	
	if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Enable == TRUE)   ////dacx buff
	{
		record_out = pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
	}	
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN 	
	if(pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Enable == TRUE)	////dacx buff
	{
#if (CFG_RES_I2S_PORT==1)
		i2s1_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
#else
		i2s0_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
#endif
	}
#endif

#ifdef CFG_FUNC_RECORDER_EN
	if(pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].Enable == TRUE)
	{
		local_rec_out = pAudioCore->AudioSink[AUDIO_RECORDER_SINK_NUM].PcmOutBuf;
	}
#endif

    if(monitor_out)
	{
		memset(monitor_out, 0, n * 2 * 2);
    }

    if(record_out)
    {
		memset(record_out, 0, n * 2);
    }
	
    if(usb_out)
    {
		memset(usb_out, 0, n * 2 * 2);//mono*2 stereo*4
    }
	
    if(i2s0_out)
    {
		memset(i2s0_out, 0, n * 2 * 2);//mono*2 stereo*4
	}
	
#if (BT_HFP_SUPPORT == ENABLE)
	if(hf_pcm_out)
	{
		memset(hf_pcm_out, 0, n * 2);
	}
	
	if(hf_dac_out)
	{
		memset(hf_dac_out, 0, n * 2 * 2);
	}
	
	if(hf_rec_out)
	{
		memset(hf_rec_out, 0, n * 2);
	}

	if(hf_mic_in && hf_pcm_in && hf_pcm_out && hf_dac_out)
	{
		//hf_pcm_in: 16K nomo //256*2
		//hf_mic_in: 16K stereo (需要转成nomo)  //256*2*2
		//hf_pcm_out: 16K nomo //256*2
		//hf_dac_out: 16K stereo //256*2*2
		
		//手机端通话音频送DAC 16K nomo -> stereo
		for(s = 0; s < n; s++)
		{
			hf_dac_out[s*2 + 0] = hf_pcm_in[s];
			hf_dac_out[s*2 + 1] = hf_pcm_in[s]; 
		}

		//如需AEC,则需要开启CFG_APP_USB_AUDIO_MODE_EN
		for(s = 0; s < n; s++)
		{
			hf_pcm_out[s] = __nds32__clips((((int32_t)hf_mic_in[2*s + 0] + (int32_t)hf_mic_in[2*s + 1])), 16-1);
		}
	}
    //本地MIC采样及手机端通话音频做录音处理
	if(hf_rec_out)
	{
		for(s = 0; s < n; s++)
		{
			hf_rec_out[2*s + 0] = __nds32__clips((((int32_t)hf_mic_in[2*s + 0] + (int32_t)hf_pcm_in[2*s + 0])), 16-1);
			hf_rec_out[2*s + 1] = __nds32__clips((((int32_t)hf_mic_in[2*s + 1] + (int32_t)hf_pcm_in[2*s + 1])), 16-1);
		}
	}
#endif

	//DAC0立体声监听音效处理
	if(monitor_out)
	{		
		for(s = 0; s < n; s++)
		{
			if(mic_pcm && music_pcm)
			{
				monitor_out[2*s + 0] = __nds32__clips((((int32_t)mic_pcm[2*s + 0] + (int32_t)music_pcm[2*s + 0] )), 16-1);
				monitor_out[2*s + 1] = __nds32__clips((((int32_t)mic_pcm[2*s + 1] + (int32_t)music_pcm[2*s + 1] )), 16-1);
			}
			else if(mic_pcm)
			{				
				monitor_out[2*s + 0] = mic_pcm[2*s + 0];
				monitor_out[2*s + 1] = mic_pcm[2*s + 1];
			}
			else if(music_pcm)
			{				
				monitor_out[2*s + 0] = music_pcm[2*s + 0];
				monitor_out[2*s + 1] = music_pcm[2*s + 1];
			}
			else
			{				
				monitor_out[2*s + 0] = 0;
				monitor_out[2*s + 1] = 0;
			}
		}
		
        #if defined(CFG_FUNC_REMIND_SOUND_EN)//提示音音效处理
		if(remind_in)
		{
			for(s = 0; s < n; s++)
			{
				monitor_out[2*s + 0] = __nds32__clips((((int32_t)monitor_out[2*s + 0] + (int32_t)remind_in[2*s + 0])), 16-1);
				monitor_out[2*s + 1] = __nds32__clips((((int32_t)monitor_out[2*s + 1] + (int32_t)remind_in[2*s + 1])), 16-1);
			}
		}
        #endif

		if(i2s0_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s0_out[2*s + 0] = monitor_out[2*s + 0];
				i2s0_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}
		if(i2s1_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s1_out[2*s + 0] = monitor_out[2*s + 0];
				i2s1_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}	
	}


    #ifdef CFG_FUNC_RECORDER_EN
	if(local_rec_out)
	{
		for(s = 0; s < n; s++)
		{
			local_rec_out[2*s + 0] = monitor_out[2*s + 0];
			local_rec_out[2*s + 1] = monitor_out[2*s + 1];
		}
	}
	#endif
	
	#ifdef CFG_RES_AUDIO_DACX_EN
	//DAC X单声道录音音效处理
	if(record_out)
	{		
		if(music_pcm)
		{
			for(s = 0; s < n; s++)
			{
				record_out[s] = __nds32__clips((( (int32_t)music_pcm[2*s+0] + (int32_t)music_pcm[2*s+1])), 16-1);
									    
			}
		}
	}
	#endif
}

#endif

/*
****************************************************************
* 无App通路音频处理主函数
*
*
****************************************************************
*/
void AudioNoAppProcess(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	uint16_t n = mainAppCt.SamplesPreFrame;
	int16_t *mic_pcm    	= NULL;//pBuf->mic_in;///mic input

	int16_t *monitor_out    = NULL;//pBuf->dac0_out;

	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Enable == TRUE)////mic buff
	{
		mic_pcm = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;//双mic输入
	}

    if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Enable == TRUE)   ////dacx buff
	{

    	monitor_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
	}

#ifdef CFG_RES_AUDIO_DACX_EN
//	if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Enable == TRUE)   ////dacx buff
//	{
//		record_out = pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
//	}
#endif

    if(monitor_out)
	{
		memset(monitor_out, 0, n * 2 * 2);
    }

	//DAC0立体声监听音效处理
	if(monitor_out)
	{
		for(s = 0; s < n; s++)
		{
			if(mic_pcm)
			{
				monitor_out[2*s + 0] = mic_pcm[2*s + 0];
				monitor_out[2*s + 1] = mic_pcm[2*s + 1];
			}
			else
			{
				monitor_out[2*s + 0] = 0;
				monitor_out[2*s + 1] = 0;
			}
		}
	}
}
