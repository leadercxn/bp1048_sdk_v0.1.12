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
extern bool GetWhetherRecMusic(void);
extern uint8_t DecoderTaskState;

#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
int16_t EqModeAudioBuf[512*2];
EQContext EqBufferBak;
#endif

void du_efft_fadein_sw(int16_t* pcm_in, uint16_t pcm_length, uint16_t ch)
{
	int16_t* temp = (int16_t *)pcm_in;
	int32_t n = 0, w = 0, step = 32768/pcm_length;

	if(ch == 2)
	{
		for(n = 0;	n < pcm_length; n++)
		{
			temp[n * 2] = (int16_t)(((int32_t)temp[n * 2] * w + 16384) >> 15);
			temp[n * 2 + 1] = (int16_t)(((int32_t)temp[n * 2 + 1] * w + 16384) >> 15);
			w += step;
		}
	}
	else if(ch == 1)
	{
		for(n = 0;	n < pcm_length; n++)
		{
			temp[n] = (int16_t)(((int32_t)temp[n] * w + 16384) >> 15);
			w += step;
		}
	}
}

void du_efft_fadeout_sw(int16_t* pcm_in, uint16_t pcm_length, uint16_t ch)
{
	int16_t* temp = (int16_t *)pcm_in;
	int32_t n = 0, w = (32768/pcm_length)*(pcm_length-1), step = 32768/pcm_length;

	if(ch == 2)
	{
		for(n = 0; n < pcm_length; n++)
		{
			temp[n * 2] = (int16_t)(((int32_t)temp[n * 2] * w + 16384) >> 15);
			temp[n * 2 + 1] = (int16_t)(((int32_t)temp[n * 2 + 1] * w + 16384) >> 15);
			w -= step;
		}
	}
	else if(ch == 1)
	{
		for(n = 0; n < pcm_length; n++)
		{
			temp[n] = (int16_t)(((int32_t)temp[n] * w + 16384) >> 15);
			w -= step;
		}
	}
}

/*
****************************************************************
* 关闭所有音效功能
*
*
****************************************************************
*/
void AudioEffectsAllDisable(void)
{
	uint16_t i;
    
	APP_DBG("AudioEffectsAllDisable \n");
	gCtrlVars.auto_tune_unit.enable 	  = 0;
	gCtrlVars.echo_unit.enable			  = 0;
	gCtrlVars.pitch_shifter_unit.enable   = 0;
	#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
	gCtrlVars.pitch_shifter_pro_unit.enable = 0;
	#endif
	gCtrlVars.voice_changer_unit.enable   = 0;
	#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	gCtrlVars.voice_changer_pro_unit.enable = 0;
	#endif
	gCtrlVars.reverb_unit.enable		  = 0;
	gCtrlVars.plate_reverb_unit.enable	  = 0;
    #if CFG_AUDIO_EFFECT_PINGPONG_EN
    gCtrlVars.ping_pong_unit.enable       = 0;
    #endif
	#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	gCtrlVars.reverb_pro_unit.enable      = 0;
	#endif
	gCtrlVars.freq_shifter_unit.enable	  = 0;
	gCtrlVars.howling_dector_unit.enable  = 0;
	#if CFG_AUDIO_EFFECT_MIC_AEC_EN
	gCtrlVars.mic_aec_unit.enable         = 0;
	#endif
	gCtrlVars.MicAudioSdct_unit.enable    = 0;
	gCtrlVars.MusicAudioSdct_unit.enable  = 0;
	#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	gCtrlVars.music_threed_unit.enable    = 0;
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
	gCtrlVars.music_threed_plus_unit.enable= 0;
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	gCtrlVars.music_vb_unit.enable        = 0;
	#endif
	#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
	gCtrlVars.rec_vb_unit.enable          = 0;
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	gCtrlVars.music_vb_classic_unit.enable = 0;
	#endif
    #if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
	gCtrlVars.stereo_winden_unit.enable   = 0;
    #endif
    #if CFG_AUDIO_EFFECT_AUTOWAH_EN
    gCtrlVars.auto_wah_unit.enable        = 0;
    #endif
	#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
	gCtrlVars.music_delay_unit.enable     = 0;
	#endif
	gCtrlVars.music_exciter_unit.enable   = 0;	
	gCtrlVars.vocal_cut_unit.enable       = 0;
	gCtrlVars.vocal_remove_unit.enable    = 0;
	gCtrlVars.chorus_unit.enable          = 0;
	gCtrlVars.phase_control_unit.enable   = 0;

	//expand
	for(i = 0; i < sizeof(expander_unit_aggregate)/sizeof(expander_unit_aggregate[0]); i++)
	{
		expander_unit_aggregate[i]->enable = 0;
	}
    //drc
	for(i = 0; i < sizeof(drc_unit_aggregate)/sizeof(drc_unit_aggregate[0]); i++)
	{
		drc_unit_aggregate[i]->enable		= 0;
	}
	//eq
	for(i = 0; i < sizeof(eq_unit_aggregate)/sizeof(eq_unit_aggregate[0]); i++)
	{
		eq_unit_aggregate[i]->enable		= 0;
	}
    //gain control
	for(i = 0; i < sizeof(gain_unit_aggregate)/sizeof(gain_unit_aggregate[0]); i++)
	{
		gain_unit_aggregate[i]->enable		= 0;
	}
	//eq
	for(i = 0; i < sizeof(eq_unit_aggregate)/sizeof(eq_unit_aggregate[0]); i++)
	{
		eq_unit_aggregate[i]->enable 	= 0;
	}
}

/*
****************************************************************
* 音效模块反初始化
*
*
****************************************************************
*/
void AudioEffectsDeInit_HFP(void)
{
#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexLock(AudioEffectMutex);
	}
#endif
	if(gCtrlVars.mic_expander_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.mic_expander_unit.ct);
	}
	if(gCtrlVars.mic_drc_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.mic_drc_unit.ct);
	}
	if(gCtrlVars.mic_out_eq_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.mic_out_eq_unit.ct);
	}
	//if(gCtrlVars.mic_reverb_gain_control_unit.ct != NULL)
	//{
	//	osPortFree(gCtrlVars.mic_reverb_gain_control_unit.ct);
	//}
	if(gCtrlVars.mic_aec_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.mic_aec_unit.ct);
	}
#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexUnlock(AudioEffectMutex);
	}
#endif

}

void AudioEffectsDeInit(void)
{
	uint8_t i;
#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexLock(AudioEffectMutex);
	}
#endif
    APP_DBG("AudioEffectsDeInit \n");

	if(gCtrlVars.freq_shifter_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.freq_shifter_unit.ct);
	}
	if(gCtrlVars.howling_dector_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.howling_dector_unit.ct);
	}
	if(gCtrlVars.pitch_shifter_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.pitch_shifter_unit.ct);
	}
	#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
	if(gCtrlVars.pitch_shifter_pro_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.pitch_shifter_pro_unit.ct);
	}
	#endif
	if(gCtrlVars.auto_tune_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.auto_tune_unit.ct);
	}
	if(gCtrlVars.voice_changer_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.voice_changer_unit.ct);
	}
	#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	if(gCtrlVars.voice_changer_pro_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.voice_changer_pro_unit.ct);
	}
	#endif
	if(gCtrlVars.echo_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.echo_unit.ct);
	}
	if(gCtrlVars.reverb_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.reverb_unit.ct);
	}
	if(gCtrlVars.plate_reverb_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.plate_reverb_unit.ct);
	}
    #if CFG_AUDIO_EFFECT_PINGPONG_EN
    if(gCtrlVars.ping_pong_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.ping_pong_unit.ct);
	}
    #endif
	#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	if(gCtrlVars.reverb_pro_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.reverb_pro_unit.ct);
	}
	#endif
	#if CFG_AUDIO_EFFECT_MIC_AEC_EN
	if(gCtrlVars.mic_aec_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.mic_aec_unit.ct);
	}
	#endif
	if(gCtrlVars.MicAudioSdct_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.MicAudioSdct_unit.ct);
	}
	if(gCtrlVars.MusicAudioSdct_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.MusicAudioSdct_unit.ct);
	}
	if(gCtrlVars.vocal_cut_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.vocal_cut_unit.ct);
	}
	if(gCtrlVars.vocal_remove_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.vocal_remove_unit.ct);
	}
	#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	if(gCtrlVars.music_threed_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.music_threed_unit.ct);
	}
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
	if(gCtrlVars.music_threed_plus_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.music_threed_plus_unit.ct);
	}
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	if(gCtrlVars.music_vb_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.music_vb_unit.ct);
	}
	#endif
	#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
	if(gCtrlVars.rec_vb_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.rec_vb_unit.ct);
	}
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	if(gCtrlVars.music_vb_classic_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.music_vb_classic_unit.ct);
	}
	#endif
    #if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
    if(gCtrlVars.stereo_winden_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.stereo_winden_unit.ct);
	}
    #endif
    #if CFG_AUDIO_EFFECT_AUTOWAH_EN
    if(gCtrlVars.auto_wah_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.auto_wah_unit.ct);
	}
    #endif
	#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
	if(gCtrlVars.music_delay_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.music_delay_unit.ct);
	}
	#endif
	if(gCtrlVars.music_exciter_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.music_exciter_unit.ct);
	}
	if(gCtrlVars.chorus_unit.ct != NULL)
	{
		osPortFree(gCtrlVars.chorus_unit.ct);
	}
    //expand
	for(i = 0; i < sizeof(expander_unit_aggregate)/sizeof(expander_unit_aggregate[0]); i++)
	{
		if(expander_unit_aggregate[i]->ct != NULL)
		{
			osPortFree(expander_unit_aggregate[i]->ct);
		}
	}
    //drc
    for(i = 0; i < sizeof(drc_unit_aggregate)/sizeof(drc_unit_aggregate[0]); i++)
	{
		if(drc_unit_aggregate[i]->ct != NULL)
		{
    		osPortFree(drc_unit_aggregate[i]->ct);
		}
	}
	 //eq
    for(i = 0; i < sizeof(eq_unit_aggregate)/sizeof(eq_unit_aggregate[0]); i++)
	{
		if(eq_unit_aggregate[i]->ct != NULL)
		{
    		osPortFree(eq_unit_aggregate[i]->ct);
		}
	}

	gCtrlVars.freq_shifter_unit.ct   =   NULL;
	gCtrlVars.howling_dector_unit.ct =   NULL;
	gCtrlVars.pitch_shifter_unit.ct  =   NULL;
	#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
	gCtrlVars.pitch_shifter_pro_unit.ct  =   NULL;
	#endif
	gCtrlVars.auto_tune_unit.ct		 =   NULL;
	gCtrlVars.voice_changer_unit.ct  =   NULL;
	#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	gCtrlVars.voice_changer_pro_unit.ct  =   NULL;
	#endif
	gCtrlVars.echo_unit.ct			 =   NULL;
	gCtrlVars.echo_unit.s_buf		 =   NULL;
	gCtrlVars.reverb_unit.ct		 =   NULL;
	gCtrlVars.plate_reverb_unit.ct	 =   NULL;
    #if CFG_AUDIO_EFFECT_PINGPONG_EN
    gCtrlVars.ping_pong_unit.ct      =   NULL;
    gCtrlVars.ping_pong_unit.s       =   NULL;
    #endif
	#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	gCtrlVars.reverb_pro_unit.ct     =   NULL;
	#endif
	#if CFG_AUDIO_EFFECT_MIC_AEC_EN
	gCtrlVars.mic_aec_unit.ct        =   NULL;
	#endif
	gCtrlVars.MicAudioSdct_unit.ct   =   NULL;
	gCtrlVars.MusicAudioSdct_unit.ct =   NULL;
	gCtrlVars.vocal_cut_unit.ct	     =   NULL;
	gCtrlVars.vocal_remove_unit.ct	 =   NULL;
	#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	gCtrlVars.music_threed_unit.ct   =   NULL;
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
	gCtrlVars.music_threed_plus_unit.ct =NULL;
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	gCtrlVars.music_vb_unit.ct       =   NULL;
	#endif
	#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
	gCtrlVars.rec_vb_unit.ct         =   NULL;
	#endif
	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	gCtrlVars.music_vb_classic_unit.ct = NULL;
	#endif
    #if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
    gCtrlVars.stereo_winden_unit.ct  =   NULL;
    #endif
    #if CFG_AUDIO_EFFECT_AUTOWAH_EN
    gCtrlVars.auto_wah_unit.ct       =   NULL;
    #endif
	#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
	gCtrlVars.music_delay_unit.ct	 =   NULL;
	#endif
	gCtrlVars.music_exciter_unit.ct	 =   NULL;
	gCtrlVars.chorus_unit.ct         =   NULL;
	
    //expand
	for(i = 0; i < sizeof(expander_unit_aggregate)/sizeof(expander_unit_aggregate[0]); i++)
	{
		expander_unit_aggregate[i]->ct		= NULL;
	}
    //drc
    for(i = 0; i < sizeof(drc_unit_aggregate)/sizeof(drc_unit_aggregate[0]); i++)
	{
		drc_unit_aggregate[i]->ct		    = NULL;
	}
	 //eq
    for(i = 0; i < sizeof(eq_unit_aggregate)/sizeof(eq_unit_aggregate[0]); i++)
	{
		eq_unit_aggregate[i]->ct		    = NULL;
	}

#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexUnlock(AudioEffectMutex);
	}
#endif
}

/*
****************************************************************
* 音效模块初始化
*
*
****************************************************************
*/
void AudioEffectsInit(void)
{
#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexLock(AudioEffectMutex);
	}
#endif
	
	#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	AudioEffectReverbProInit(&gCtrlVars.reverb_pro_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif
	
	#if CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN
	AudioEffectExpanderInit(&gCtrlVars.music_expander_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

    #if CFG_AUDIO_EFFECT_MUSIC_SILENCE_DECTOR_EN
	gCtrlVars.MusicAudioSdct_unit.enable = 1;
	AudioEffectSilenceDectorInit(&gCtrlVars.MusicAudioSdct_unit,gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
    #endif

	#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
	AudioEffectPcmDelayInit(&gCtrlVars.music_delay_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN
	AudioEffectExciterInit(&gCtrlVars.music_exciter_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
	AudioEffectPitchShifterProInit(&gCtrlVars.pitch_shifter_pro_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif
	
	#if CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN
	AudioEffectExpanderInit(&gCtrlVars.mic_expander_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif

	#if 0//CFG_AUDIO_EFFECT_MIC_AEC_EN
	gCtrlVars.mic_aec_unit.enable = 1;
	//AudioEffectAecInit(&gCtrlVars.mic_aec_unit, 1, gCtrlVars.sample_rate);
	AudioEffectAecInit(&gCtrlVars.mic_aec_unit, 1, 16000);//固定为16K采样率
	#endif
	
    #if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN
	AudioEffectFreqShifterInit(&gCtrlVars.freq_shifter_unit);
	#endif

	#if CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN
	AudioEffectHowlingSuppressorInit(&gCtrlVars.howling_dector_unit);
	#endif
	
    #if CFG_AUDIO_EFFECT_MIC_SILENCE_DECTOR_EN
	gCtrlVars.MicAudioSdct_unit.enable = 1;
	AudioEffectSilenceDectorInit(&gCtrlVars.MicAudioSdct_unit,gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
    #endif

	#if CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN
	AudioEffectPitchShifterInit(&gCtrlVars.pitch_shifter_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN
	AudioEffectAutoTuneInit(&gCtrlVars.auto_tune_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN
	AudioEffectVoiceChangerInit(&gCtrlVars.voice_changer_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif

    #if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	AudioEffectVoiceChangerProInit(&gCtrlVars.voice_changer_pro_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif
	
	#if CFG_AUDIO_EFFECT_MIC_ECHO_EN
	AudioEffectEchoInit(&gCtrlVars.echo_unit,  gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MIC_REVERB_EN
	AudioEffectReverbInit(&gCtrlVars.reverb_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif

    #if CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN
	AudioEffectPlateReverbInit(&gCtrlVars.plate_reverb_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif	

    #if CFG_AUDIO_EFFECT_PINGPONG_EN
    AudioEffectPingPongInit(&gCtrlVars.ping_pong_unit,gCtrlVars.sample_rate);
    #endif

	#if CFG_AUDIO_EFFECT_VOCAL_CUT_EN
	AudioEffectVocalCutInit(&gCtrlVars.vocal_cut_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN
    AudioEffectVocalRemoveInit(&gCtrlVars.vocal_remove_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
    #endif

	#if CFG_AUDIO_EFFECT_CHORUS_EN
    AudioEffectChorusInit(&gCtrlVars.chorus_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
    #endif
	
	#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
	AudioEffectThreeDInit(&gCtrlVars.music_threed_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
	AudioEffectThreeDPlusInit(&gCtrlVars.music_threed_plus_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
	AudioEffectVBInit(&gCtrlVars.music_vb_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_REC_VIRTUAL_BASS_EN
	AudioEffectVBInit(&gCtrlVars.rec_vb_unit, 1, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
	AudioEffectVBClassicInit(&gCtrlVars.music_vb_classic_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif
	
    #if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
	AudioEffectStereoWidenerInit(&gCtrlVars.stereo_winden_unit,gCtrlVars.sample_rate);
    #endif

    #if CFG_AUDIO_EFFECT_AUTOWAH_EN
    AudioEffectAutoWahInit(&gCtrlVars.auto_wah_unit,gCtrlVars.sample_rate);
    #endif

	#if CFG_AUDIO_EFFECT_MUSIC_DRC_EN
	AudioEffectDRCInit(&gCtrlVars.music_drc_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

    #if CFG_AUDIO_EFFECT_MIC_DRC_EN
	AudioEffectDRCInit(&gCtrlVars.mic_drc_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif
	
	#if CFG_AUDIO_EFFECT_REC_DRC_EN
	AudioEffectDRCInit(&gCtrlVars.rec_drc_unit, 1, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MUSIC_PRE_EQ_EN
	AudioEffectEQInit(&gCtrlVars.music_pre_eq_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MUSIC_OUT_EQ_EN
	AudioEffectEQInit(&gCtrlVars.music_out_eq_unit, gCtrlVars.adc_line_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MIC_PRE_EQ_EN
	AudioEffectEQInit(&gCtrlVars.mic_pre_eq_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif
	
	#if CFG_AUDIO_EFFECT_MIC_BYPASS_EQ_EN
	AudioEffectEQInit(&gCtrlVars.mic_bypass_eq_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MIC_ECHO_EQ_EN
	AudioEffectEQInit(&gCtrlVars.mic_echo_eq_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MIC_REVERB_EQ_EN
	AudioEffectEQInit(&gCtrlVars.mic_reverb_eq_unit, gCtrlVars.adc_mic_channel_num, gCtrlVars.sample_rate);
	#endif

	#if CFG_AUDIO_EFFECT_MIC_OUT_EQ_EN
	#ifdef CFG_FUNC_MIC_KARAOKE_EN
	AudioEffectEQInit(&gCtrlVars.mic_out_eq_unit, 2, gCtrlVars.sample_rate);
	#else
	AudioEffectEQInit(&gCtrlVars.mic_out_eq_unit, 1, gCtrlVars.sample_rate);
	#endif
	#endif

	#if CFG_AUDIO_EFFECT_REC_EQ_EN
	AudioEffectEQInit(&gCtrlVars.rec_eq_unit, 1, gCtrlVars.sample_rate);
	#endif

#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexUnlock(AudioEffectMutex);
	}
#endif
}

#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
/*
****************************************************************
* Pcm Delay音效初始化
*
*
****************************************************************
*/
void AudioEffectPcmDelayInit(PcmDelayUnit *unit, uint8_t channel, uint32_t sample_rate)
{
//	uint32_t size;
	
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

    if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		uint32_t s_buff_size;
		if(unit->high_quality)
		{
			s_buff_size = unit->max_delay_samples*2*channel;
		}
		else
		{
			s_buff_size = ceil(unit->max_delay_samples/32)*19*channel+64;;
			       
		}
        unit->ct = (PCMDelay *)osPortMallocFromEnd(PCM_DELAY_SIZE + s_buff_size);
		
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("PCMDelay mem malloc err! %ld\n",PCM_DELAY_SIZE);
			return;
		}
		unit->s_buf = (uint8_t *)unit->ct + PCM_DELAY_SIZE;
	}
	
	if(unit->ct != NULL)
	{
		pcm_delay_init(unit->ct, channel, unit->max_delay_samples, unit->high_quality, unit->s_buf);
	}	
}
#endif

#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
/*
****************************************************************
* Pcm Delay音效配置
*
*
****************************************************************
*/
void AudioEffectPcmDelayConfig(PcmDelayUnit *unit, uint8_t channel, uint32_t sample_rate)
{

    if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct != NULL)
	{
		pcm_delay_init(unit->ct, channel, unit->max_delay_samples, unit->high_quality, unit->s_buf);
	}
}
#endif	
/*
****************************************************************
* Exciter音效初始化
*
*
****************************************************************
*/
void AudioEffectExciterInit(ExciterUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

    if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (ExciterContext *)osPortMallocFromEnd(EXCITER_SIZE);	
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("ExciterContext mem malloc err! %ld\n",EXCITER_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		exciter_init(unit->ct, channel, sample_rate, unit->f_cut);
	}
#endif
}
/*
****************************************************************
* Exciter音效配置
*
*
****************************************************************
*/
void AudioEffectExciterConfig(ExciterUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MUSIC_EXCITER_EN
    if(!unit->enable)
	{
		return;
	}
		
	if(unit->ct != NULL)
	{
		exciter_init(unit->ct, channel, sample_rate, unit->f_cut);
	}
#endif
}

/*
****************************************************************
* Aec音效初始化
*
*
****************************************************************
*/
void AudioEffectAecInit(AecUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_AEC_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
//    if(unit->channel == 0)
//    {
//		unit->channel = channel;
//    }
//
//	channel = unit->channel;

    if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{	
		unit->ct = (BlueAECContext *)osPortMallocFromEnd(AEC_SIZE);
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("BlueAECContext mem malloc err! %ld\n",AEC_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		blue_aec_init(unit->ct, unit->es_level, unit->ns_level);
	}
#endif	
}
/*
****************************************************************
* Expander音效初始化
*
*
****************************************************************
*/
void AudioEffectExpanderInit(ExpanderUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN || CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

    if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (ExpanderContext *)osPortMallocFromEnd(EXPANDER_SIZE);
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("ExpanderContext mem malloc err! %ld\n",AEC_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		expander_init(unit->ct,  channel, sample_rate,
								 unit->threshold,
								 unit->ratio,
								 unit->attack_time,
								 unit->release_time);
	}
#endif
}
/*
****************************************************************
* Expander音效INIT参数配置
*
*
****************************************************************
*/
void AudioEffectExpanderConfig(ExpanderUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN || CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN
    if(!unit->enable)
	{
		return;
	}
		
	if(unit->ct != NULL)
	{
		expander_init(unit->ct,  channel, sample_rate,
								 unit->threshold,
								 unit->ratio,
								 unit->attack_time,
								 unit->release_time);
	}
#endif
}
/*
****************************************************************
* Expander音效配置函数
* 1，适用于实时调节的场合
*
****************************************************************
*/
void AudioEffectExpanderThresholdConfig(ExpanderUnit *unit)
{
#if CFG_AUDIO_EFFECT_MUSIC_NOISE_SUPPRESSOR_EN || CFG_AUDIO_EFFECT_MIC_NOISE_SUPPRESSOR_EN
	
	if(!unit->enable)
	{
		return;
	}	
	
	if(unit->ct != NULL)
	{
		expander_set_threshold(unit->ct,  unit->threshold);
	}
#endif
}
/*
****************************************************************
* FreqShifter音效初始化
*
*
****************************************************************
*/
void AudioEffectFreqShifterInit(FreqShifterUnit *unit)
{
#if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
    if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (FreqShifterContext *)osPortMallocFromEnd(FREQSHIFTER_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("FreqShifterContext mem malloc err! %ld\n",FREQSHIFTER_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		freqshifter_init(unit->ct,unit->deltaf);  
	}
#endif
}
/*
****************************************************************
* FreqShifter音效配置
*
*
****************************************************************
*/
void AudioEffectFreqShifterConfig(FreqShifterUnit *unit)
{
#if CFG_AUDIO_EFFECT_MIC_FREQ_SHIFTER_EN
    if(!unit->enable)
	{
		return;
	}
		
	if(unit->ct != NULL)
	{
		freqshifter_init(unit->ct,unit->deltaf);  
	}
#endif
}
/*
****************************************************************
* HowlingDector音效初始化
*
*
****************************************************************
*/
void AudioEffectHowlingSuppressorInit(HowlingDectorUnit *unit)
{
#if CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
    if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (HowlingContext *)osPortMallocFromEnd(HOWLING_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("HowlingContext mem malloc err! %ld\n",HOWLING_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		howling_suppressor_init(unit->ct, unit->suppression_mode);
	}
#endif
}
/*
****************************************************************
* HowlingDector音效配置
*
*
****************************************************************
*/
void AudioEffectHowlingSuppressorConfig(HowlingDectorUnit *unit)
{
#if CFG_AUDIO_EFFECT_MIC_HOWLING_DECTOR_EN
    if(!unit->enable)
	{
		return;
	}
		
	if(unit->ct != NULL)
	{
		howling_suppressor_init(unit->ct, unit->suppression_mode);
	}
#endif
}

/*
****************************************************************
* SilenceDector音效初始化
*
*
****************************************************************
*/
void AudioEffectSilenceDectorInit(SilenceDetectorUnit *unit,uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MUSIC_SILENCE_DECTOR_EN || CFG_AUDIO_EFFECT_MIC_SILENCE_DECTOR_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

    if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (SilenceDetectorContext *)osPortMallocFromEnd(SILENCE_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("SilenceDetectorContext mem malloc err! %ld\n",SILENCE_SIZE);
		}	
	}
		
	if(unit->ct != NULL)
	{
	   init_silence_detector(unit->ct, channel, sample_rate);
	}
#endif
}
/*
****************************************************************
* PitchShifter音效初始化
*
*
****************************************************************
*/
void AudioEffectPitchShifterInit(PitchShifterUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (PSContext *)osPortMallocFromEnd(PS_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("PSContext mem malloc err! %ld\n",PS_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		//unit->ct->w = 300;//////改善延时!!!!!!!!!!!!!
		pitch_shifter_init(unit->ct, channel, sample_rate, unit->semitone_steps, CFG_MIC_PITCH_SHIFTER_FRAME_SIZE);//512
		//gCtrlVars.SamplesPerFrame = CFG_MIC_PITCH_SHIFTER_FRAME_SIZE / 2;
	}
#endif
}
#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
/*
****************************************************************
* PitchShifterPro音效初始化
*
*
****************************************************************
*/
void AudioEffectPitchShifterProInit(PitchShifterProUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if 0
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (PitchShifterProContext *)osPortMallocFromEnd(PS_PRO_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("PitchShifterProContext mem malloc err! %ld\n",PS_PRO_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		pitch_shifter_pro_init(unit->ct, channel, sample_rate, unit->semitone_steps, CFG_MUSIC_PITCH_SHIFTER_PRO_FRAME_SIZE);
		gCtrlVars.SamplesPerFrame = CFG_MUSIC_PITCH_SHIFTER_PRO_FRAME_SIZE;
	}
#endif
}
#endif
/*
****************************************************************
* PitchShifter音效参数配置
* 1，适用于实时调节的场合
*
****************************************************************
*/
void AudioEffectPitchShifterConfig(PitchShifterUnit *unit)
{
#if CFG_AUDIO_EFFECT_MIC_PITCH_SHIFTER_EN
	/*if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}*/
	
	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct != NULL)
	{
		pitch_shifter_configure(unit->ct, unit->semitone_steps);
	}
#endif
}
#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
/*
****************************************************************
* PitchShifterPro音效参数配置
* 1，适用于实时调节的场合
*
****************************************************************
*/
void AudioEffectPitchShifterProConfig(PitchShifterProUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if 0
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct != NULL)
	{
		pitch_shifter_pro_init(unit->ct, channel, sample_rate, unit->semitone_steps, CFG_MUSIC_PITCH_SHIFTER_PRO_FRAME_SIZE);
	}
#endif
}
#endif
/*
****************************************************************
* AutoTune音效初始化
*
*
****************************************************************
*/
void AudioEffectAutoTuneInit(AutoTuneUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
    	 unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (AutoTuneContext *)osPortMallocFromEnd(AUTOTUNE_SIZE);
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("AutoTuneContext mem malloc err! %ld\n",AUTOTUNE_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		auto_tune_init(unit->ct, channel, sample_rate, unit->key, mainAppCt.SamplesPreFrame);
	}
#endif
}

/*
****************************************************************
* AutoTune音效配置
*
*
****************************************************************
*/
void AudioEffectAutoTuneConfig(AutoTuneUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_AUTO_TUNE_EN
	if(!unit->enable)
	{
		return;
	}		
	if(unit->ct != NULL)
	{
		auto_tune_init(unit->ct, channel, sample_rate, unit->key, mainAppCt.SamplesPreFrame);
	}
#endif
}

/*
****************************************************************
* VoiceChanger音效初始化
*
*
****************************************************************
*/
void AudioEffectVoiceChangerInit(VoiceChangerUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (VoiceChangerContext *)osPortMallocFromEnd(VOICECHANGER_SIZE);			
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("VoiceChangerContext mem malloc err! %ld\n",VOICECHANGER_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		voice_changer_init(unit->ct, channel, sample_rate, unit->pitch_ratio, unit->formant_ratio);//512

	}
#endif
}
/*
****************************************************************
* VoiceChanger音效配置
*
*
****************************************************************
*/
void AudioEffectVoiceChangerConfig(VoiceChangerUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_EN
	if(!unit->enable)
	{
		return;
	}
		
	if(unit->ct != NULL)
	{
		voice_changer_init(unit->ct, channel, sample_rate, unit->pitch_ratio, unit->formant_ratio);//512

	}
#endif
}
#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
/*
****************************************************************
* VoiceChangerPro音效初始化
*
*
****************************************************************
*/
void AudioEffectVoiceChangerProInit(VoiceChangerProUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if 0//CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (VoiceChangerProContext *)osPortMallocFromEnd(VOICECHANGER_PRO_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("VoiceChangerProContext mem malloc err! %ld\n",VOICECHANGER_PRO_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		voice_changer_pro_init(unit->ct, sample_rate, 256, unit->pitch_ratio, unit->formant_ratio);///256				
	}
#endif
}
#endif

#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
/*
****************************************************************
* VoiceChangerPro音效配置
*
*
****************************************************************
*/
void AudioEffectVoiceChangerProConfig(VoiceChangerProUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if 0
	if(!unit->enable)
	{
		return;
	}
		
	if(unit->ct != NULL)
	{
		voice_changer_pro_init(unit->ct, sample_rate, 256, unit->pitch_ratio, unit->formant_ratio);///256				
	}
#endif
}
#endif
/*
****************************************************************
* Echo音效初始化
*
*
****************************************************************
*/
void AudioEffectEchoInit(EchoUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_ECHO_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}		
	if(unit->ct == NULL)
	{
        uint32_t s_buff_size;
        s_buff_size = ceil(unit->max_delay_samples/32)*19+64;
        unit->ct = (EchoContext *)osPortMallocFromEnd(ECHO_SIZE + s_buff_size);
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("EchoContext mem malloc err! %ld\n",ECHO_SIZE);
			return;
		}
        unit->s_buf = (uint8_t *)unit->ct + ECHO_SIZE; //(uint8_t *)osPortMallocFromEnd(size);
	}
	if(unit->ct != NULL)
	{
		echo_init(unit->ct, channel, sample_rate, unit->fc, unit->max_delay_samples, unit->s_buf);
	}
#endif
}

/*
****************************************************************
* Echo音效配置
*
*
****************************************************************
*/
void AudioEffectEchoConfig(EchoUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_ECHO_EN

	if(!unit->enable)
	{
		return;
	}
		
	if(unit->ct != NULL)
	{
		echo_init(unit->ct, channel, sample_rate, unit->fc, unit->max_delay_samples, unit->s_buf);
	}
#endif
}
/*
****************************************************************
* Reverb音效初始化
*
*
****************************************************************
*/
void AudioEffectReverbInit(ReverbUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_REVERB_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (ReverbContext *)osPortMallocFromEnd(REVERB_SIZE);
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("ReverbContext mem malloc err! %ld\n",REVERB_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		reverb_init(unit->ct, channel, sample_rate);
		reverb_configure(unit->ct, unit->dry_scale, unit->wet_scale, unit->width_scale, unit->roomsize_scale, unit->damping_scale);
	}
#endif
}
/*
****************************************************************
* Reverb音效配置函数
* 1，适用于实时调节的场合
*
****************************************************************
*/
void AudioEffectReverbConfig(ReverbUnit *unit)
{
#if CFG_AUDIO_EFFECT_MIC_REVERB_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
	//	return;
	}
	
	if(!unit->enable)
	{
		return;
	}	
	
	if(unit->ct != NULL)
	{
		reverb_configure(unit->ct, unit->dry_scale, unit->wet_scale, unit->width_scale, unit->roomsize_scale, unit->damping_scale);
	}
#endif
}
/*
****************************************************************
* PlateReverb音效初始化
*
*
****************************************************************
*/
void AudioEffectPlateReverbInit(PlateReverbUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (PlateReverbContext *)osPortMallocFromEnd(PLATE_REVERB_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("PlateReverbContext mem malloc err! %ld\n",PLATE_REVERB_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
	    plate_reverb_init(unit->ct, channel, sample_rate,unit->highcut_freq,unit->modulation_en);		
        plate_reverb_configure(unit->ct, unit->predelay, unit->diffusion, unit->decay, unit->damping, unit->wetdrymix);		
	}
#endif
}
/*
****************************************************************
* PlateReverb音效modulatio配置
*
*
****************************************************************
*/
void AudioEffectPlateReverbHighcutModulaConfig(PlateReverbUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN
	if(!unit->enable)
	{
		return;
	}
		
	if(unit->ct != NULL)
	{
	    plate_reverb_init(unit->ct, channel, sample_rate,unit->highcut_freq,unit->modulation_en);		
	}
#endif
}

void AudioEffectPlateReverbConfig(PlateReverbUnit *unit)
{
#if CFG_AUDIO_EFFECT_MIC_PLATE_REVERB_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
	//	return;
	}
	
	if(!unit->enable)
	{
		return;
	}	
	
	if(unit->ct != NULL)
	{
		plate_reverb_configure(unit->ct, unit->predelay, unit->diffusion, unit->decay, unit->damping, unit->wetdrymix);		
	}
#endif
}
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
/*
****************************************************************
* ReverbPro音效初始化
*
*
****************************************************************
*/
void AudioEffectReverbProInit(ReverbProUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
#if defined(CFG_CHIP_BP1048P2)  || defined(CFG_CHIP_BP1048P4) ||defined(CFG_CHIP_BP1064A2)
	int32_t real_size=0,len=0;

	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel==0)
    {
    	 unit->channel = channel;
    }

	channel = unit->channel;
	
	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		if(sample_rate>44100)
		{
			APP_DBG("Pro Reverb sample must <= 44100\n");
		}
		if(sample_rate == 44100)
		{
			len = MAX_REVERB_PRO_CONTEXT_SIZE_44100;
			unit->ct = (uint8_t *)osPortMallocFromEnd(REVERB_PRO_SIZE);						
			if(unit->ct == NULL)
			{
				unit->enable = 0;
				APP_DBG("ReverbPro 44k mem malloc err! %d\n",REVERB_PRO_SIZE);
			}			
		}
		else if(sample_rate == 32000)
		{
			len = MAX_REVERB_PRO_CONTEXT_SIZE_32000;
			unit->ct = (uint8_t *)osPortMallocFromEnd(REVERB_PRO_32K_SIZE);		
			if(unit->ct == NULL)
			{
				unit->enable = 0;
				APP_DBG("ReverbPro 32k mem malloc err! %u\n",REVERB_PRO_32K_SIZE);
			}
		}
	}
		
	if(unit->ct != NULL)
	{
		reverb_pro_init(unit->ct,sample_rate,unit->dry,unit->wet,unit->erwet,unit->erfactor,
			            unit->erwidth,unit->ertolate,unit->rt60,unit->delay,unit->width,unit->wander,
			            unit->spin,unit->inputlpf,unit->damplpf,unit->basslpf,unit->bassb,unit->outputlpf);

		real_size  = reverb_pro_context_size(unit->ct);
		len = len - real_size;
	}
#endif
#endif
}
#endif
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
/*
****************************************************************
* ReverProb音效配置函数
* 1，适用于实时调节的场合
* 2.暂时只调节干声或湿声
****************************************************************
*/
void AudioEffectReverProbConfig(ReverbProUnit *unit,uint32_t sample_rate)
{
#if defined(CFG_CHIP_BP1048P2)  || defined(CFG_CHIP_BP1048P4) ||defined(CFG_CHIP_BP1064A2)
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
	if((!unit->enable)||(unit->ct == NULL))
	{
		return;
	}	
	
	reverb_pro_init(unit->ct,sample_rate,unit->dry,unit->wet,unit->erwet,unit->erfactor,
							unit->erwidth,unit->ertolate,unit->rt60,unit->delay,unit->width,unit->wander,
							unit->spin,unit->inputlpf,unit->damplpf,unit->basslpf,unit->bassb,unit->outputlpf);
#endif
}
#endif
/*
****************************************************************
* VocalCut音效初始化
*
*
****************************************************************
*/
void AudioEffectVocalCutInit(VocalCutUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_VOCAL_CUT_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;
	
	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (VocalCutContext *)osPortMallocFromEnd(VOCALCUT_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("VocalCutContext mem malloc err! %ld\n",VOCALCUT_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		vocal_cut_init(unit->ct, sample_rate);			
	}
#endif
}
/*
****************************************************************
* VocalRemove音效初始化
*
*
****************************************************************
*/
void AudioEffectVocalRemoveInit(VocalRemoveUnit *unit,  uint8_t channel, uint32_t sample_rate)
{
#if 0//CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
	if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;
	
	if(!unit->enable)
	{
		return;
	}

	if(unit->ct == NULL)
	{
	    unit->ct = (VocalRemoverContext *)osPortMallocFromEnd(VOCALREMOVE_SIZE);					
	    if(unit->ct == NULL)
	    {
			unit->enable = 0;
			APP_DBG("VocalRemoverContext mem malloc err! %ld\n",VOCALREMOVE_SIZE);
	    }
	}
		
	if(unit->ct != NULL)
	{
		vocal_remover_init(unit->ct, sample_rate,unit->lower_freq,unit->higher_freq,unit->step_size);
	}
#endif
}
/*
****************************************************************
* VocalRemove音效配置
*
*
****************************************************************
*/
void AudioEffectVocalRemoveConfig(VocalRemoveUnit *unit,  uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_VOCAL_REMOVE_EN	
	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct != NULL)
	{
		vocal_remover_init(unit->ct, sample_rate,unit->lower_freq,unit->higher_freq,unit->step_size);
	}
#endif
}
/*
****************************************************************
* Chorus音效初始化
*
*
****************************************************************
*/
void AudioEffectChorusInit(ChorusUnit *unit,  uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_CHORUS_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
	if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;
	
	if(!unit->enable)
	{
		return;
	}
	if(unit->ct == NULL)
	{
	    unit->ct = (ChorusContext *)osPortMallocFromEnd(CHORUS_SIZE);					
	    if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("ChorusContext mem malloc err! %ld\n",CHORUS_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		chorus_init(unit->ct, sample_rate, unit->delay_length, unit->mod_depth, unit->mod_rate);
	}
#endif
}
#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
/*
****************************************************************
* ThreeD音效初始化
*
*
****************************************************************
*/
void AudioEffectThreeDInit(ThreeDUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (ThreeDContext *)osPortMallocFromEnd(THREED_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("ThreeDContext mem malloc err! %ld\n",THREED_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		three_d_init(unit->ct, channel,	sample_rate);	
	}
}
#endif
#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
/*
****************************************************************
* ThreeD Plus音效初始化
*
*
****************************************************************
*/
void AudioEffectThreeDPlusInit(ThreeDPlusUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if defined(CFG_CHIP_BP1048P2)  || defined(CFG_CHIP_BP1048P4) ||defined(CFG_CHIP_BP1064A2)
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (ThreeDPlusContext *)osPortMallocFromEnd(THREED_PLUS_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("ThreeDPlusContext mem malloc err! %ld\n",THREED_PLUS_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		three_d_plus_init(unit->ct, sample_rate);	
	}
#endif
}
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
/*
****************************************************************
* VB音效初始化
*
*
****************************************************************
*/
void AudioEffectVBInit(VBUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (VBContext *)osPortMallocFromEnd(VB_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("VBContext mem malloc err! %ld\n",VB_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		vb_init(unit->ct, channel,	sample_rate, unit->f_cut);	
	}
}
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
/*
****************************************************************
* VB音效配置
*
*
****************************************************************
*/
void AudioEffectVBConfig(VBUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	if(!unit->enable)
	{
		return;
	}
		
	if(unit->ct != NULL)
	{
		vb_init(unit->ct, channel,	sample_rate, unit->f_cut);	
	}
}
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
/*
****************************************************************
* VBClassic音效初始化
*
*
****************************************************************
*/
void AudioEffectVBClassicInit(VBClassicUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = osPortMallocFromEnd(VBCLASSIC_SIZE);//(VBContext *)				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("VBContext mem malloc err! %ld\n",VBCLASSIC_SIZE);
		}
	}
		
	if(unit->ct != NULL)
	{
		vb_classic_init(unit->ct, channel,	sample_rate, unit->f_cut);	
	}
}
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
/*
****************************************************************
* VBClassic音效配置
*
*
****************************************************************
*/
void AudioEffectVBClassicConfig(VBClassicUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct != NULL)
	{
		vb_classic_init(unit->ct, channel,	sample_rate, unit->f_cut);	
	}
}
#endif
/*
****************************************************************
* DRC音效初始化
*
*
****************************************************************
*/
void AudioEffectDRCInit(DRCUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MUSIC_DRC_EN || CFG_AUDIO_EFFECT_MIC_DRC_EN || CFG_AUDIO_EFFECT_REC_DRC_EN
	uint16_t q[2];

	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (DRCContext *)osPortMallocFromEnd(DRC_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("DRCContext mem malloc err! %ldu\n",DRC_SIZE);
		}
	}
	
	if(unit->ct != NULL)
	{
		q[0] = unit->q[0];
		q[1] = unit->q[1];
		drc_init(unit->ct, channel, sample_rate, unit->fc, unit->mode, q, unit->threshold, unit->ratio,unit->attack_tc, unit->release_tc);
	}
#endif
}

/*
****************************************************************
* DRC音效配置函数
*
*
****************************************************************
*/
void AudioEffectDRCConfig(DRCUnit *unit, uint8_t channel, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_MUSIC_DRC_EN || CFG_AUDIO_EFFECT_MIC_DRC_EN || CFG_AUDIO_EFFECT_REC_DRC_EN
	uint16_t q[2];

	if(!unit->enable)
	{
		return;
	}	
	
	if(unit->ct != NULL)
	{
		q[0] = unit->q[0];
		q[1] = unit->q[1];
		drc_init(unit->ct, channel, sample_rate, unit->fc, unit->mode, q, unit->threshold, unit->ratio,unit->attack_tc, unit->release_tc);
	}
#endif
}
/*
****************************************************************
* EQ音效初始化
*
*
****************************************************************
*/
void AudioEffectEQInit(EQUnit *unit, uint8_t channel, uint32_t sample_rate)
{
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}
	
    if(unit->channel == 0)
    {
		unit->channel = channel;
    }

	channel = unit->channel;

	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct == NULL)
	{
		unit->ct = (EQContext *)osPortMallocFromEnd(EQ_SIZE);				
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("EQContext mem malloc err! %ld\n",EQ_SIZE);
		}
	}
	
	if(unit->ct != NULL)
	{
		eq_clear_delay_buffer(unit->ct);
		eq_init(unit->ct, sample_rate, unit->filter_params, unit->filter_count, unit->pregain, channel);
	}

}
/*
****************************************************************
* EQ预增益配置函数
*
*
****************************************************************
*/
void AudioEffectEQPregainConfig(EQUnit *unit)
{
	if(!gCtrlVars.audio_effect_init_flag)
	{
		//return;
	}
	
	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct != NULL)
	{
		eq_configure_pregain(unit->ct, unit->pregain);
	}
}
/*
****************************************************************
* EQ滤波器配置函数
*
*
****************************************************************
*/
void AudioEffectEQFilterConfig(EQUnit *unit, uint32_t sample_rate)
{
	if(!gCtrlVars.audio_effect_init_flag)
	{
		//return;
	}
	
	if(!unit->enable)
	{
		return;
	}
	
	if(unit->ct != NULL)
	{
		eq_configure_filters(unit->ct, sample_rate, unit->filter_params, unit->filter_count);
	}
}
/*
****************************************************************
* StereoWidener音效初始化
*
*
****************************************************************
*/
void AudioEffectStereoWidenerInit(StereoWindenUnit *unit, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}

    if(!unit->enable)
	{
		return;
	}

	if(unit->ct == NULL)
	{
		unit->ct = (StereoWidenerContext *)osPortMallocFromEnd(STEREO_WIDEN_SIZE);
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("stereo_widener_init mem malloc err! %ld\n",STEREO_WIDEN_SIZE);
		}
	}

	if(unit->ct != NULL)
	{
		stereo_widener_init(unit->ct, sample_rate, unit->shaping);
	}
#endif
}
/*
****************************************************************
* AutoWah音效初始化
*
*
****************************************************************
*/
void AudioEffectAutoWahInit(AutoWahUnit *unit, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_AUTOWAH_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}

    if(!unit->enable)
	{
		return;
	}

	if(unit->ct == NULL)
	{
		unit->ct = (AutoWahContext *)osPortMallocFromEnd(AUTOWAH_SIZE);

		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("AutoWahContext mem malloc err! %ld\n",AUTOWAH_SIZE);
			return;
		}
	}

	if(unit->ct != NULL)
	{
		auto_wah_init(unit->ct, sample_rate, unit->modulation_rate, unit->min_frequency, unit->max_frequency,unit->depth);
	}
#endif
}
/*
****************************************************************
* PingPong音效初始化
*
*
****************************************************************
*/
void AudioEffectPingPongInit(PingPongUnit *unit, uint32_t sample_rate)
{
#if CFG_AUDIO_EFFECT_PINGPONG_EN
	if(!gCtrlVars.audio_effect_init_flag)
	{
		return;
	}

    if(!unit->enable)
	{
		return;
	}

    if(unit->ct == NULL)
	{
        uint32_t s_buff_size;		

		if(unit->high_quality)
		{
			s_buff_size = unit->max_delay_samples*4;
		}
		else
		{
			s_buff_size = ceilf(unit->max_delay_samples/32.0f) * 38;
		}

        unit->ct = (PingPongContext *)osPortMallocFromEnd(PINGPONG_SIZE + s_buff_size);
		if(unit->ct == NULL)
		{
			unit->enable = 0;
			APP_DBG("PingPongContext mem malloc err! %ld\n",PINGPONG_SIZE);
			return;
		}
        unit->s = (uint8_t *)unit->ct + PINGPONG_SIZE; //(uint8_t *)osPortMallocFromEnd(size);
	}

	if(unit->ct != NULL)
	{
		pingpong_init(unit->ct, unit->max_delay_samples, unit->high_quality,unit->s);
	}
#endif
}

/*
****************************************************************
* Pcm Delay主循环处理函数
*
*
****************************************************************
*/
#if CFG_AUDIO_EFFECT_MUSIC_DELAY_EN
void AudioEffectPcmDelayApply(PcmDelayUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		pcm_delay_apply(unit->ct, pcm_in, pcm_out, n, unit->delay_samples);
	}
}
#endif
#if CFG_AUDIO_EFFECT_MIC_AEC_EN
/*
****************************************************************
* Aec主循环处理函数
*
*
****************************************************************
*/
void AudioEffectAecApply(AecUnit *unit, int16_t *u_pcm_in, int16_t *d_pcm_in, int16_t *pcm_out, uint32_t n)
{
    uint16_t fram,i/*,samples*/;

	fram = n/BLK_LEN;
	if(fram == 0)
	{
		//samples = 64;
		fram = 1;
	}
	else
	{
		//samples = 64;
	}
			
	if((unit->enable) && (unit->ct != NULL))
	{
        for(i = 0; i < fram; i++)	
    	{
			blue_aec_run(unit->ct,  (int16_t *)(u_pcm_in + i * BLK_LEN), (int16_t *)(d_pcm_in + i * BLK_LEN), (int16_t *)(pcm_out + i * BLK_LEN));
    	}
	}
}
#endif
/*
****************************************************************
* Exciter主循环处理函数
*
*
****************************************************************
*/
void AudioEffectExciterApply(ExciterUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		exciter_apply(unit->ct, pcm_in, pcm_out, n, unit->dry, unit->wet);		
	}
}
/*
****************************************************************
* Expander主循环处理函数
*
*
****************************************************************
*/
void AudioEffectExpanderApply(ExpanderUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		expander_apply(unit->ct, pcm_in, pcm_out, n);
	}
}
/*
****************************************************************
* FreqShifter主循环处理函数
*
*
****************************************************************
*/
void AudioEffectFreqShifterApply(FreqShifterUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{	
	int16_t  s;

	switch (unit->fade_step)
	{
		case 0://normal apply
			if((unit->enable) && (unit->ct != NULL))
			{
				freqshifter_apply(unit->ct, pcm_in, pcm_out, n);
			}
			break;

		case 1://fadeout before off apply, need triggle outside
			if((unit->enable) && (unit->ct != NULL))
			{
				freqshifter_apply(unit->ct, pcm_in, pcm_out, n);
			}
			du_efft_fadeout_sw(pcm_out,n,1);
			unit->fade_step = 2;
			break;

		case 2://zero
			for(s = 0; s < n; s++)
			{
				pcm_out[s] = 0;
			}
			unit->fade_step = 3;
			break;

		case 3://fadein with no apply
			du_efft_fadein_sw(pcm_out,n,1);
			unit->fade_step = 4;
			break;

		case 4://no apply
			break;

		case 5://fadeout before on apply, , need triggle outside
			du_efft_fadeout_sw(pcm_in,n,1);
			unit->fade_step = 6;
            break;
		case 6://zero
			for(s = 0; s < n; s++)
			{
				pcm_out[s] = 0;
			}
			unit->fade_step = 7;
			break;

		case 7://fadein with apply
			if((unit->enable) && (unit->ct != NULL))
			{
				freqshifter_apply(unit->ct, pcm_in, pcm_out, n);
			}
			du_efft_fadein_sw(pcm_out,n,1);
			unit->fade_step = 0;
			break;
	}
}
/*
****************************************************************
* HowlingDector主循环处理函数
*
*
****************************************************************
*/
void AudioEffectHowlingSuppressorApply(HowlingDectorUnit *unit, int16_t *pcm_in, int16_t *pcm_out, int32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		howling_suppressor_apply(unit->ct,  pcm_in, pcm_out, n);
	}
}
/*
****************************************************************
* SilenceDector主循环处理函数
*
*
****************************************************************
*/
#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
uint32_t  Silence_Power_Off_Time = SILENCE_POWER_OFF_DELAY_TIME;
#endif
void AudioEffectSilenceDectorApply(SilenceDetectorUnit *unit, int16_t *pcm_in, int32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		unit->level = apply_silence_detector(unit->ct, pcm_in,n);
		#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
		if(unit->level > SILENCE_THRESHOLD)
			Silence_Power_Off_Time = SILENCE_POWER_OFF_DELAY_TIME;
		#endif
	}
}
/*
****************************************************************
* Phase主循环处理函数
*
*
****************************************************************
*/
void AudioEffectPhaseApply(PhaseControlUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n,  uint8_t channel)
{
	int32_t s;

    if(unit->channel == 0)
    {
    	 unit->channel = channel;
    }


	if(unit->enable && unit->phase_difference)
	{
		channel = unit->channel;

		for(s=0; s < n * channel; s++)		
		{			
			pcm_in[s] = __nds32__clips(((int32_t)pcm_in[s] * (-1)), (16)-1);		
		}	
	}
}

/*
****************************************************************
* Pregain主循环处理函数
*
*
****************************************************************
*/
void AudioEffectPregainApply(GainControlUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n,  uint8_t channel)
{
	int32_t s;
	int32_t pregain;

    if(unit->channel == 0)
    {
    	 unit->channel = channel;
    }

	if(unit->enable)
	{
		channel = unit->channel;

		pregain = unit->mute? 0 : unit->gain;

		for(s = 0; s < n; s++)
		{
			if(channel == 2)
			{
				pcm_out[2 * s + 0] = __nds32__clips((((int32_t)pcm_in[2 * s + 0] * pregain + 2048) >> 12), 16-1);
				pcm_out[2 * s + 1] = __nds32__clips((((int32_t)pcm_in[2 * s + 1] * pregain + 2048) >> 12), 16-1);
			}
			else
			{
				pcm_out[s] = __nds32__clips((((int32_t)pcm_in[s] * pregain + 2048) >> 12), 16-1);
			}
		}
	}
}
/*
****************************************************************
* PitchShifter主循环处理函数
*
*
****************************************************************
*/
void AudioEffectPitchShifterApply(PitchShifterUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n, uint8_t channel)
{

	if((unit->enable) && (unit->ct != NULL))
	{
		uint32_t PSSample = (CFG_MIC_PITCH_SHIFTER_FRAME_SIZE >> 1);
		uint32_t Cnt = n / PSSample;
		uint16_t iIdx;

		channel = unit->channel;

		for(iIdx = 0; iIdx < Cnt; iIdx++)
		{
			pitch_shifter_apply(unit->ct, (int16_t *)(pcm_in  + (PSSample * channel) * iIdx),
									  	  (int16_t *)(pcm_out + (PSSample * channel) * iIdx));
		}
	}
}
#if CFG_AUDIO_EFFECT_MUSIC_PITCH_SHIFTER_PRO_EN
/*
****************************************************************
* PitchShifterPro主循环处理函数
*
*
****************************************************************
*/
void AudioEffectPitchShifterProApply(PitchShifterProUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{

	if((unit->enable) && (unit->ct != NULL))
	{
		//pitch_shifter_pro_apply(unit->ct, pcm_in, pcm_out);
	}
}
#endif
/*
****************************************************************
* AutoTune主循环处理函数
*
*
****************************************************************
*/
void AudioEffectAutoTuneApply(AutoTuneUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		auto_tune_apply(unit->ct, pcm_in, pcm_out, unit->snap_mode,	unit->key,unit->pitch_accuracy);
	}
}
/*
****************************************************************
* VoiceChanger主循环处理函数
*
*
****************************************************************
*/
void AudioEffectVoiceChangerApply(VoiceChangerUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		voice_changer_apply(unit->ct, pcm_in, pcm_out);
	}
}
#if CFG_AUDIO_EFFECT_MIC_VOICE_CHANGER_PRO_EN
/*
****************************************************************
* VoiceChanger主循环处理函数
*
*
****************************************************************
*/
void AudioEffectVoiceChangerProApply(VoiceChangerProUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		//voice_changer_pro_apply(unit->ct, pcm_in, pcm_out);
	}
}
#endif
/*
****************************************************************
* Echo主循环处理函数
*
*
****************************************************************
*/
#ifdef CFG_FUNC_ECHO_DENOISE
extern int16_t*  EchoAudioBuf;
#endif
void AudioEffectEchoApply(EchoUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	uint32_t s;

	uint16_t channel = 2;

	#ifdef CFG_FUNC_ECHO_DENOISE
	if((unit->enable) && (unit->ct != NULL) && (EchoAudioBuf != NULL))
	#else
	if(unit->enable && (unit->ct != NULL))
	#endif
	{
		//unit->delay_samples = unit->delay * gCtrlVars.sample_rate / 1000;

		#ifdef CFG_FUNC_ECHO_DENOISE
		if( (unit->delay != unit->delay_bakup))// && (EchoAudioBuf != NULL) )
		{
			//channel = unit->channel;
			memcpy(EchoAudioBuf, pcm_in, n * 2 * channel);

			unit->delay_samples = unit->delay_bakup * gCtrlVars.sample_rate / 1000;
			echo_apply(unit->ct, pcm_in, pcm_out, n, unit->attenuation, unit->delay_samples, unit->direct_sound);
			du_efft_fadeout_sw(pcm_out, n, channel);

			unit->delay_samples = unit->delay * gCtrlVars.sample_rate / 1000;
			echo_apply(unit->ct, EchoAudioBuf, EchoAudioBuf, n, unit->attenuation, unit->delay_samples, unit->direct_sound);
			du_efft_fadein_sw(EchoAudioBuf, n, channel);

			if(channel == 2)
			{
				for(s = 0; s < n; s++)
				{
					pcm_out[2*s + 0] = __nds32__clips(((int32_t)pcm_out[2*s + 0] + (int32_t)EchoAudioBuf[2*s + 0]), 16-1);
					pcm_out[2*s + 1] = __nds32__clips(((int32_t)pcm_out[2*s + 1] + (int32_t)EchoAudioBuf[2*s + 1]), 16-1);
				}
			}
			else
			{
				for(s = 0; s < n; s++)
				{
					pcm_out[s] = __nds32__clips(((int32_t)pcm_out[s] + (int32_t)EchoAudioBuf[s]), 16-1);
				}
			}

			unit->delay_bakup = unit->delay;
		}
		else
        #endif
		{
			echo_apply(unit->ct, pcm_in, pcm_out, n, unit->attenuation, unit->delay_samples, unit->direct_sound);
		}
	}
}
/*
****************************************************************
* Reverb主循环处理函数
*
*
****************************************************************
*/
void AudioEffectReverbApply(ReverbUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		reverb_apply(unit->ct, pcm_in, pcm_out, n);
	}
}
/*
****************************************************************
* PlateReverb主循环处理函数
*
*
****************************************************************
*/
void AudioEffectPlateReverbApply(PlateReverbUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		plate_reverb_apply(unit->ct, pcm_in, pcm_out, n);
	}
}
/*
****************************************************************
* PinPong主循环处理函数
*
*
****************************************************************
*/
void AudioEffectPinPongApply(PingPongUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
#if CFG_AUDIO_EFFECT_PINGPONG_EN
	if((unit->enable) && (unit->ct != NULL))
	{
		pingpong_apply(unit->ct, pcm_in, pcm_out, n,unit->attenuation,unit->delay_samples,unit->wetdrymix);
	}
#endif
}
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
/*
****************************************************************
* ReverbPro主循环处理函数
*
*
****************************************************************
*/
void AudioEffectReverbProApply(ReverbProUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
#if defined(CFG_CHIP_BP1048P2)  || defined(CFG_CHIP_BP1048P4) ||defined(CFG_CHIP_BP1064A2)
	if((unit->enable) && (unit->ct != NULL))
	{
		reverb_pro_apply(unit->ct, pcm_in, pcm_out, n);
	}
#endif
}
#endif
/*
****************************************************************
* VocalCut主循环处理函数
*
*
****************************************************************
*/
void AudioEffectVocalCutApply(VocalCutUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
    if(unit->vocal_cut_en == 0) return;
	
	if((unit->enable) && (unit->ct != NULL))
	{
		vocal_cut_apply(unit->ct, pcm_in, pcm_out, n,	unit->wetdrymix);
	}
}
/*
****************************************************************
* VocalRemovet主循环处理函数
*
*
****************************************************************
*/
void AudioEffectVocalRemoveApply(VocalRemoveUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
    if(unit->vocal_remove_en == 0) return;

	if((unit->enable) && (unit->ct != NULL))
	{
		//vocal_remover_apply(unit->ct, pcm_in, pcm_out);
	}
}
/*
****************************************************************
* Chorus主循环处理函数
*
*
****************************************************************
*/
void AudioEffectChorusApply(ChorusUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		chorus_apply(unit->ct, pcm_in, pcm_out, n, unit->feedback, unit->dry, unit->wet, unit->mod_rate);
	}
}
#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
/*
****************************************************************
* ThreeD主循环处理函数
*
*
****************************************************************
*/
void AudioEffectThreeDApply(ThreeDUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{	
    if(unit->three_d_en == 0) return;
	if((unit->enable) && (unit->ct != NULL))
	{
		three_d_apply(unit->ct, pcm_in, pcm_out, n, unit->intensity);
	}
}
#endif
#if CFG_AUDIO_EFFECT_MUSIC_3D_PLUS_EN
/*
****************************************************************
* ThreeD Plus主循环处理函数
*
*
****************************************************************
*/
void AudioEffectThreeDPlusApply(ThreeDPlusUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
#if defined(CFG_CHIP_BP1048P2)  || defined(CFG_CHIP_BP1048P4) || defined(CFG_CHIP_BP1064A2)
    if(unit->three_d_en == 0) return;
	if((unit->enable) && (unit->ct != NULL))
	{
		three_d_plus_apply(unit->ct, pcm_in, pcm_out, n, unit->intensity);
	}
#endif
}
#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
extern int16_t MusicVolBuf[512*2];
/*
****************************************************************
* VB主循环处理函数
*
*
****************************************************************
*/
void AudioEffectVBApply(VBUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{    
	uint16_t i;
	if(unit->vb_en == 0)
	{
		if(unit->vb_en_bak)
		{
			unit->vb_en_bak = 0;
			if((unit->enable) && (unit->ct != NULL))
			{
				DBG("vb fadeOut\n");
				memcpy(MusicVolBuf, pcm_in, n * 2 * unit->channel);

				vb_apply(unit->ct, pcm_in, pcm_out, n, unit->intensity, unit->enhanced);
				du_efft_fadeout_sw(pcm_out, n, unit->channel);

				du_efft_fadein_sw((int16_t *)MusicVolBuf, n, unit->channel);

				for(i = 0; i < n; i++)
				{
					pcm_out[2*i + 0] = __nds32__clips(((int32_t)pcm_out[2*i + 0] + (int32_t)MusicVolBuf[2*i + 0]), 16-1);
					pcm_out[2*i + 1] = __nds32__clips(((int32_t)pcm_out[2*i + 1] + (int32_t)MusicVolBuf[2*i + 1]), 16-1);
				}
			}
		}
		return;
	}
	if((unit->enable) && (unit->ct != NULL))
	{
		if(unit->vb_en_bak==0)
		{
			DBG("vb fadeIn\n");
			unit->vb_en_bak = 1;

			memcpy(MusicVolBuf, pcm_in, n * 2 * unit->channel);
			memcpy(pcm_out, pcm_in, n * 2 * unit->channel);

			du_efft_fadeout_sw(pcm_out, n, unit->channel);

			vb_apply(unit->ct, MusicVolBuf, MusicVolBuf, n, unit->intensity, unit->enhanced);
			du_efft_fadein_sw((int16_t *)MusicVolBuf, n, unit->channel);

			for(i = 0; i < n; i++)
			{
				pcm_out[2*i + 0] = __nds32__clips(((int32_t)pcm_out[2*i + 0] + (int32_t)MusicVolBuf[2*i + 0]), 16-1);
				pcm_out[2*i + 1] = __nds32__clips(((int32_t)pcm_out[2*i + 1] + (int32_t)MusicVolBuf[2*i + 1]), 16-1);
			}
		}
		else
		{
			vb_apply(unit->ct, pcm_in, pcm_out, n, unit->intensity, unit->enhanced);
		}
	}
 }

#endif
#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_CLASSIC_EN
/*
****************************************************************
* VBClassic主循环处理函数
*
*
****************************************************************
*/
void AudioEffectVBClassicApply(VBClassicUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{	
    if(unit->vb_en == 0) return;
	if((unit->enable) && (unit->ct != NULL))
	{
		vb_classic_apply(unit->ct, pcm_in, pcm_out, n, unit->intensity);
	}
}
#endif
/*
****************************************************************
* DRC主循环处理函数
*
*
****************************************************************
*/
void AudioEffectDRCApply(DRCUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
	if((unit->enable) && (unit->ct != NULL))
	{
		drc_apply(unit->ct, pcm_in, pcm_out, n, unit->pregain1, unit->pregain2);
	}
}
/*
****************************************************************
* StereoWidener主循环处理函数
*
*
****************************************************************
*/
void AudioEffectStereoWidenerApply(StereoWindenUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
#if CFG_AUDIO_EFFECT_STEREO_WIDEN_EN
	if((unit->enable) && (unit->ct != NULL))
	{
		stereo_widener_apply(unit->ct, pcm_in, pcm_out, n);
	}
#endif
}
/*
****************************************************************
* AutoWahA主循环处理函数
*
*
****************************************************************
*/
void AudioEffectAutoWahApply(AutoWahUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n)
{
#if CFG_AUDIO_EFFECT_AUTOWAH_EN
	if((unit->enable) && (unit->ct != NULL))
	{
		auto_wah_apply(unit->ct, pcm_in, pcm_out, n,unit->dry,unit->wet, unit->modulation_rate);
	}
#endif
}
/*
****************************************************************
* EQ主循环处理函数
*
*
****************************************************************
*/
void AudioEffectEQApply(EQUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n, uint8_t channel)
{
	uint32_t cnt = n / EQ_BUFFER_NUM_SAMPLES;
	uint32_t RemainLen = n - cnt * EQ_BUFFER_NUM_SAMPLES;
	uint32_t i = 0;

	if((unit->enable) && (unit->ct != NULL))
	{
		channel = unit->channel;

		#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
		if(&gCtrlVars.music_out_eq_unit == unit)
		{
			if(mainAppCt.EqModeBak != mainAppCt.EqMode)
			{
				memcpy(EqModeAudioBuf, pcm_in, n * 2 * channel);
				memcpy(&EqBufferBak, unit->ct, sizeof(EQContext));
				// AudioEffectEQApply(&gCtrlVars.music_out_eq_unit, music_pcm, music_pcm, n, channel);
				for(i = 0; i < cnt; i++)
				{
					eq_apply(unit->ct, (int16_t *)(pcm_in + i * EQ_BUFFER_NUM_SAMPLES * channel), (int16_t *)(pcm_out + i * EQ_BUFFER_NUM_SAMPLES * channel), EQ_BUFFER_NUM_SAMPLES);
				}
				if(RemainLen > 0)
				{
					eq_apply(unit->ct, (int16_t *)(pcm_in + i * EQ_BUFFER_NUM_SAMPLES * channel), (int16_t *)(pcm_out + i * EQ_BUFFER_NUM_SAMPLES * channel), RemainLen);
				}

				du_efft_fadeout_sw(pcm_out, n, channel);
				//eq_clear_delay_buffer(unit->ct);//若前后EQ模式参数中filter数目差异大，需要打开这行代码

				memcpy(unit->ct, &EqBufferBak,sizeof(EQContext));
				#ifdef FUNC_OS_EN
				osMutexUnlock(AudioEffectMutex);
				#endif
				EqModeSet(mainAppCt.EqMode);
				#ifdef FUNC_OS_EN
				osMutexLock(AudioEffectMutex);
				#endif

				//AudioEffectEQApply(&gCtrlVars.music_out_eq_unit, (int16_t *)EqModeAudioBuf, (int16_t *)EqModeAudioBuf, n, channel);
				for(i = 0; i < cnt; i++)
				{
					eq_apply(unit->ct, (int16_t *)(EqModeAudioBuf + i * EQ_BUFFER_NUM_SAMPLES * channel), (int16_t *)(EqModeAudioBuf + i * EQ_BUFFER_NUM_SAMPLES * channel), EQ_BUFFER_NUM_SAMPLES);
				}
				if(RemainLen > 0)
				{
					eq_apply(unit->ct, (int16_t *)(EqModeAudioBuf + i * EQ_BUFFER_NUM_SAMPLES * channel), (int16_t *)(EqModeAudioBuf + i * EQ_BUFFER_NUM_SAMPLES * channel), RemainLen);
				}

				du_efft_fadein_sw((int16_t *)EqModeAudioBuf, n, channel);

				for(i = 0; i < n; i++)
				{
					pcm_out[2*i + 0] = __nds32__clips(((int32_t)pcm_out[2*i + 0] + (int32_t)EqModeAudioBuf[2*i + 0]), 16-1);
					pcm_out[2*i + 1] = __nds32__clips(((int32_t)pcm_out[2*i + 1] + (int32_t)EqModeAudioBuf[2*i + 1]), 16-1);
				}

				mainAppCt.EqModeBak = mainAppCt.EqMode;

				return;
			}
		}
		#endif

		for(i = 0; i < cnt; i++)
		{
			eq_apply(unit->ct, (int16_t *)(pcm_in + i * EQ_BUFFER_NUM_SAMPLES * channel), (int16_t *)(pcm_out + i * EQ_BUFFER_NUM_SAMPLES * channel), EQ_BUFFER_NUM_SAMPLES);
		}
		if(RemainLen > 0)
		{
			eq_apply(unit->ct, (int16_t *)(pcm_in + i * EQ_BUFFER_NUM_SAMPLES * channel), (int16_t *)(pcm_out + i * EQ_BUFFER_NUM_SAMPLES * channel), RemainLen);
		}
	}
}
/*
****************************************************************
* 实时获取reverb相关参数，以此为电位器调节的最大值
* 只是在装载参数时，才可以获取最大值，用于调节参数
* 用户根据需要获取相应的值
****************************************************************
*/
void GetAudioEffectMaxValue(void)
{
	//if(StartWriteCmd == LOAD_BUSY)//////
	{	
		gCtrlVars.max_chorus_wet               = gCtrlVars.chorus_unit.wet;
		
	    gCtrlVars.max_plate_reverb_roomsize    = gCtrlVars.plate_reverb_unit.decay;

		gCtrlVars.max_plate_reverb_wetdrymix   = gCtrlVars.plate_reverb_unit.wetdrymix;

		gCtrlVars.max_reverb_wet_scale         = gCtrlVars.reverb_unit.wet_scale;

		gCtrlVars.max_reverb_roomsize    = gCtrlVars.reverb_unit.roomsize_scale;
		
		gCtrlVars.max_echo_delay        = gCtrlVars.echo_unit.delay_samples;
	
        gCtrlVars.max_echo_depth        = gCtrlVars.echo_unit.attenuation;
	
		//gCtrlVars.ReverbRoom            = 0;

		//gCtrlVars.max_dac0_dig_l_vol    = gCtrlVars.dac0_dig_l_vol;

		//gCtrlVars.max_dac0_dig_r_vol    = gCtrlVars.dac0_dig_r_vol;

		//gCtrlVars.max_dac1_dig_vol      = gCtrlVars.dac1_dig_vol;  
        #if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
		gCtrlVars.max_reverb_pro_wet     = gCtrlVars.reverb_pro_unit.wet ;
		gCtrlVars.max_reverb_pro_erwet   = gCtrlVars.reverb_pro_unit.erwet ;
		#endif

    }
}
/*
****************************************************************
* mic treb,bass调节函数
*
*
****************************************************************
*/
#ifdef CFG_FUNC_MIC_TREB_BASS_EN
void MicBassTrebAjust(int16_t 	BassGain,	int16_t TrebGain)
{
	gCtrlVars.mic_out_eq_unit.filter_params[0].gain = gCtrlVars.mic_out_eq_unit.eq_params[1].gain = BassTrebGainTable[BassGain]; 
	gCtrlVars.mic_out_eq_unit.filter_params[1].gain = gCtrlVars.mic_out_eq_unit.eq_params[0].gain=  BassTrebGainTable[TrebGain];
	AudioEffectEQFilterConfig(&gCtrlVars.mic_out_eq_unit, gCtrlVars.sample_rate);
}
#endif
/*
****************************************************************
* music treb,bass调节函数
*
*
****************************************************************
*/
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
void MusicBassTrebAjust(int16_t 	BassGain,	int16_t TrebGain)
{
	gCtrlVars.music_pre_eq_unit.filter_params[0].gain = gCtrlVars.music_pre_eq_unit.eq_params[0].gain = BassTrebGainTable[BassGain]; 
	gCtrlVars.music_pre_eq_unit.filter_params[1].gain = gCtrlVars.music_pre_eq_unit.eq_params[1].gain=  BassTrebGainTable[TrebGain];
	AudioEffectEQFilterConfig(&gCtrlVars.music_pre_eq_unit, gCtrlVars.sample_rate);
}
#endif
/*************************************************
 *  混响大小调节函数
 *
 *
 ***************************************************/
#ifdef CFG_FUNC_MIC_KARAOKE_EN
void ReverbStepSet(uint8_t ReverbStep)
{
	uint16_t step,r;
	
    step = gCtrlVars.max_chorus_wet / MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		gCtrlVars.chorus_unit.wet = gCtrlVars.max_chorus_wet;
	}
	else
	{
		gCtrlVars.chorus_unit.wet = ReverbStep * step;
	}
	step = gCtrlVars.max_plate_reverb_roomsize / MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		gCtrlVars.plate_reverb_unit.decay = gCtrlVars.max_plate_reverb_roomsize;
	}
	else
	{
		gCtrlVars.plate_reverb_unit.decay = ReverbStep * step;
	}
	//APP_DBG("mic_wetdrymix   = %d\n",gCtrlVars.max_plate_reverb_wetdrymix);
	step = gCtrlVars.max_plate_reverb_wetdrymix / MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		gCtrlVars.plate_reverb_unit.wetdrymix = gCtrlVars.max_plate_reverb_wetdrymix;
	}
	else
	{
		gCtrlVars.plate_reverb_unit.wetdrymix = ReverbStep * step;
	}			
    step = gCtrlVars.max_echo_delay / MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		gCtrlVars.echo_unit.delay_samples = gCtrlVars.max_echo_delay;
	}
	else
	{
		gCtrlVars.echo_unit.delay_samples = ReverbStep * step;
	}
	step = gCtrlVars.max_echo_depth/ MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		gCtrlVars.echo_unit.attenuation= gCtrlVars.max_echo_depth;
	}
	else
	{
		gCtrlVars.echo_unit.attenuation = ReverbStep * step;
	}

    step = gCtrlVars.max_reverb_wet_scale/ MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		gCtrlVars.reverb_unit.wet_scale = gCtrlVars.max_reverb_wet_scale;
	}
	else
	{
		gCtrlVars.reverb_unit.wet_scale = ReverbStep * step;
	}
    step = gCtrlVars.max_reverb_roomsize/ MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		gCtrlVars.reverb_unit.roomsize_scale = gCtrlVars.max_reverb_roomsize;
	}
	else
	{
		gCtrlVars.reverb_unit.roomsize_scale = ReverbStep * step;
	}
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	//+0  ~~~ -70
    r = abs(gCtrlVars.max_reverb_pro_wet);
    r = 70-r;
    step = r / MAX_MIC_REVB_STEP;

	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		gCtrlVars.reverb_pro_unit.wet = gCtrlVars.max_reverb_pro_wet;
	}
	else
	{
		r = MAX_MIC_REVB_STEP - 1 - ReverbStep;

		r*= step;

		gCtrlVars.reverb_pro_unit.wet = gCtrlVars.max_reverb_pro_wet - r;

		if(ReverbStep == 0) gCtrlVars.reverb_pro_unit.wet = -70;
	}


    r = abs(gCtrlVars.max_reverb_pro_erwet);
    r = 70-r;
    step = r / MAX_MIC_REVB_STEP;

	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		gCtrlVars.reverb_pro_unit.erwet = gCtrlVars.max_reverb_pro_erwet;
	}
	else
	{
		r = MAX_MIC_REVB_STEP - 1 - ReverbStep;

		r*= step;

		gCtrlVars.reverb_pro_unit.erwet = gCtrlVars.max_reverb_pro_erwet - r;

		if(ReverbStep == 0) gCtrlVars.reverb_pro_unit.erwet = -70;
	}
#endif

	AudioEffectReverbConfig(&gCtrlVars.reverb_unit);
    AudioEffectPlateReverbConfig(&gCtrlVars.plate_reverb_unit);
#if CFG_AUDIO_EFFECT_MIC_REVERB_PRO_EN
	AudioEffectReverProbConfig(&gCtrlVars.reverb_pro_unit,gCtrlVars.sample_rate);
#endif
}
#endif

#else
/*
****************************************************************
* Aec音效初始化
*
*
****************************************************************
*/
void AudioEffectAecInit(AecUnit *unit, uint8_t channel, uint32_t sample_rate)
{
}

/*
****************************************************************
* 音效模块反初始化
*
*
****************************************************************
*/
void AudioEffectsDeInit(void)
{
}

/*
****************************************************************
* 音效模块初始化
*
*
****************************************************************
*/
void AudioEffectsInit(void)
{
}


#endif
