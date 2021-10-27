/**
*************************************************************************************
* @file	    chorus.h
* @brief	Chorus effect for mono signals
*
* @author	ZHAO Ying (Alfred)
* @version	v1.1.1
*
* &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
*************************************************************************************
*/

#ifndef __CHORUS_H__
#define __CHORUS_H__

#include <stdint.h>

#define MAX_CHORUS_DELAY_SAMPLES (25*2*48) // maximum delay in samples 25ms*2@48kHz

/** error code for chorus */
typedef enum _CHORUS_ERROR_CODE
{
	CHORUS_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	CHORUS_ERROR_ILLEGAL_SAMPLE_RATE,
	CHORUS_ERROR_ILLEGAL_DELAY_LENGTH,
	CHORUS_ERROR_ILLEGAL_MODULATION_DEPTH,
	CHORUS_ERROR_ILLEGAL_MODULATION_RATE,
	CHORUS_ERROR_ILLEGAL_FEEDBACK,
	CHORUS_ERROR_ILLEGAL_DRY,
	CHORUS_ERROR_ILLEGAL_WET,

	// No Error
	CHORUS_ERROR_OK = 0,					/**< no error              */
} CHORUS_ERROR_CODE;

/** chorus context */
typedef struct _ChorusContext
{
	int32_t sample_rate;
	//int32_t num_channels;
	int32_t delay_length;
	int32_t delay_length_samples;
	int32_t dp;
	int32_t mod_count;
	int32_t mod_depth_samples;	
	int32_t mod_rate;
	int32_t mod_rate_step;	
	int16_t delay[MAX_CHORUS_DELAY_SAMPLES];
} ChorusContext;
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
* @brief Initialize the chorus module.
* @param ct Pointer to a ChorusContext object.
* @param sample_rate Sample rate.
* @param delay_length Delay length in millisecond. Range: 1~25ms
* @param mod_depth Modulation depth in millisecond, which should be less than delay_length. Range: 0~(delay_length-1)ms
* @param mod_rate Modulation rate in 0.1Hz. For example, 2 for 0.2Hz, 10 for 1.0Hz, 100 for 10.0Hz. Range: 0.0~10.0Hz in step of 0.1Hz
* @return error code. CHORUS_ERROR_OK means successful, other codes indicate error.
*/
int32_t chorus_init(ChorusContext *ct, int32_t sample_rate, int32_t delay_length, int32_t mod_depth, int32_t mod_rate);


/**
* @brief Apply chorus effect to a frame of PCM data (mono only)
* @param ct Pointer to a ChorusContext object.
* @param pcm_in PCM input buffer.
* @param pcm_out PCM output buffer.
*        pcm_out can be the same as pcm_in. In this case, the PCM data is changed in-place.
* @param n Number of PCM samples to process.
* @param feedback Feedback percentage in use. Range: 0~50%
* @param dry The level of dry(direct) signals in the output. Range: 0%~100%.
* @param wet The level of wet(effect) signals in the output. Range: 0%~100%.
* @param mod_rate Modulation rate in 0.1Hz. For example, 2 for 0.2Hz, 10 for 1.0Hz, 100 for 10.0Hz. Range: 0.0~10.0Hz in step of 0.1Hz
* @return error code. CHORUS_ERROR_OK means successful, other codes indicate error.
* @note Note that only mono signals are accepted.
*/
int32_t chorus_apply(ChorusContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int32_t feedback, int32_t dry, int32_t wet, int32_t mod_rate);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif // __CHORUS_H__

