/**
 *************************************************************************************
 * @file	voice_changer.h
 * @brief	Voice changer for 32/44.1/48kHz mono signals
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.6.3
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __VOICE_CHANGER_H__
#define __VOICE_CHANGER_H__

#include <stdint.h>


#define VC_FRAME_SIZE 512		// Frame size for voice changer
#define MAX_NUM_MARKS 64		// VC_FRAME_SIZE*4/MIN_LAG
#define MAX_LAG_AT_MAX_FS 762	// Maximum lag @ maximum sample frequency, i.e. 48kHz

/** error code for voice changer */
typedef enum _VOICECHANGER_ERROR_CODE
{
    VOICECHANGER_ERROR_UNSUPPORTED_NUMBER_OF_CHANNELS = -256,
	VOICECHANGER_ERROR_SAMPLE_RATE_NOT_SUPPORTED,
	VOICECHANGER_ERROR_STEPS_OUT_OF_RANGE,
	// No Error
	VOICECHANGER_ERROR_OK = 0,					/**< no error              */
} VOICECHANGER_ERROR_CODE;


/** Voice Changer Context */
typedef struct _VoiceChangerContext
{
	int32_t num_channels;
	int32_t sample_rate;
	int32_t frame_size;
	int32_t pitch_scale;
	int32_t formant_scale;	
	int32_t lp_b1,lp_a2,lp_a3;
	int32_t lp_d0,lp_d1,lp_d2,lp_d3;
	int32_t hp_d0, hp_d1;
	int32_t pitch_weight;
	int32_t num_pitch_lags;
	int32_t xdecim;
	int32_t shift_bits;	
	int32_t pcm_min;
	int32_t pcm_max;
	int32_t last_syn;
	int32_t last_mark;
	int32_t num_marks;
	//int32_t pitch_ref;
	int32_t ref_error;
	int32_t hit_index;
	int32_t hit_count;
	int32_t hit_count_th;
	const int16_t *pitch_lag_table;	
	int16_t *old_unit;
	int16_t *new_unit;
	int32_t old_unit_len;
	int32_t new_unit_len;
	int32_t pitch_trend[5];	
	//int32_t pitch_frame[5];
	
	int32_t ac[328];
	int16_t marks[MAX_NUM_MARKS];
	int16_t xlp[VC_FRAME_SIZE*3];
	int16_t x[VC_FRAME_SIZE*5];
	int16_t y[VC_FRAME_SIZE*5]; // NOT allowed although maximum 3106 is found in tests (male -> more male), thus still 5, not 7.
	int16_t unit0[MAX_LAG_AT_MAX_FS*2];
	int16_t unit1[MAX_LAG_AT_MAX_FS*2];
	int16_t syn_unit_r[MAX_LAG_AT_MAX_FS*2+MAX_LAG_AT_MAX_FS]; // formant scale should to be >= 2/3
	int16_t syn_unit[MAX_LAG_AT_MAX_FS*2];
	
} VoiceChangerContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize the voice changer.
 * @param ct Pointer to a VoiceChangerContext object.
 * @param num_channels Number of channels. 
 * @param sample_rate Sample rate. Although other sample rates can be set, 44.1kHz is the only sample rate that is currently designed for.
 * @param pitch_ratio Ratio of new pitch over current pitch in step of 0.01. For example 180 stands for (new pitch/current pitch = 1.80)
 * @param formant_ratio Ratio of new formant over current formant in step of 0.01. For example 110 stands for (new formant/current formant = 1.10)
 * @return error code. VOICECHANGER_ERROR_OK means successful, other codes indicate error. 
 * @note Due to the voice property and computation limit, although 2 channels are supported, this module treats stereo signals like mono signals by mixing left and right channel first and make mono output in both left and right channels, i.e. the left channel has the same output as the right channel.
 */
int32_t voice_changer_init(VoiceChangerContext *ct, int32_t num_channels, int32_t sample_rate, int32_t pitch_ratio, int32_t formant_ratio);


/**
 * @brief Apply the change of voice
 * @param ct Pointer to a VoiceChangerContext object.
 * @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format, i.e. for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 * @param pcm_out Address of the PCM output. The PCM layout is the same as in Microsoft WAVE format, i.e. for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @return error code. VOICECHANGER_ERROR_OK means successful, other codes indicate error.
 * @note The length of PCM samples = num_channels * VC_FRAME_SIZE.
 */
int32_t voice_changer_apply(VoiceChangerContext *ct, int16_t *pcm_in, int16_t *pcm_out);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__VOICE_CHANGER_H__
