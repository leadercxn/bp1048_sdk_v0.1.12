/**
*************************************************************************************
* @file		exciter.h
* @brief	Harmonic Exciter.
*
* @author	ZHAO Ying (Alfred)
* @version	v1.1.2
*
* &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
*************************************************************************************
*/

#ifndef __EXCITER_H__
#define __EXCITER_H__

#include <stdint.h>

#define EXCITER_MIN_CUTOFF_FREQ 1000
#define EXCITER_MAX_CUTOFF_FREQ 10000

/** error code for exciter */
typedef enum _EXCITER_ERROR_CODE
{
	EXCITER_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	EXCITER_ERROR_NUMBER_OF_CHANNELS_NOT_SUPPORTED,
	EXCITER_ERROR_ILLEGAL_CUTOFF_FREQ,
	EXCITER_ERROR_ILLEGAL_AMOUNT,
	// No Error
	EXCITER_ERROR_OK = 0,					/**< no error              */
} EXCITER_ERROR_CODE;

/** exciter context */
typedef struct _ExciterContext
{
	int32_t sample_rate;
	int32_t num_channels;
	int32_t fco[7];
	int32_t d[2][6];
	int32_t threshold, slope, alpha_attack, alpha_release;
	int32_t state[2];
} ExciterContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief Initialize exciter module.
 * @param ct Pointer to the ExciterContext object.
 * @param num_channels Number of channels. Both 1 and 2 channels are supported.
 * @param sample_rate Sample rate
 * @param f_cut Cut-off frequency in Hz. Range: EXCITER_MIN_CUTOFF_FREQ ~ EXCITER_MAX_CUTOFF_FREQ in Hz. Components higher than this cut-off frequency are excited with harmonics.
 * @return error code. EXCITER_ERROR_OK means successful, other codes indicate error.
 */
int32_t exciter_init(ExciterContext *ct, int32_t num_channels, int32_t sample_rate, int32_t f_cut);


/**
* @brief Apply harmonic exciter audio effect to a frame of PCM data.
* @param ct Pointer to the ExciterContext object.
* @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
* @param pcm_out Address of the PCM output. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
*        pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
* @param n Number of PCM samples to process.
* @param dry Amount of the original signal in final mix. Range: 0 ~ 100
* @param wet Amount of the exciter signal in final mix. Range: 0 ~ 100
* @return error code. EXCITER_ERROR_OK means successful, other codes indicate error.
*/
int32_t exciter_apply(ExciterContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int32_t dry, int32_t wet);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__EXCITER_H__