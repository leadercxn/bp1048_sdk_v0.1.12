/**
 *************************************************************************************
 * @file	auto_wah.h
 * @brief	Auto-wah effect for mono signals
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __AUTO_WAH_H__
#define __AUTO_WAH_H__

#include <stdint.h>

#define AUTOWAH_MAX_NUM_F0 400

/** error code for auto wah */
typedef enum _AUTOWAH_ERROR_CODE
{
	AUTOWAH_ERROR_UNSUPPORTED_NUMBER_OF_CHANNELS = -256,
	AUTOWAH_ERROR_ILLEGAL_SAMPLE_RATE,
	AUTOWAH_ERROR_ILLEGAL_MODULATION_RATE,	
	AUTOWAH_ERROR_ILLEGAL_FREQUENCY_MIN,
	AUTOWAH_ERROR_ILLEGAL_FREQUENCY_MAX,
	AUTOWAH_ERROR_ILLEGAL_DEPTH,
	AUTOWAH_ERROR_ILLEGAL_DRY,
	AUTOWAH_ERROR_ILLEGAL_WET,
	// No Error
	AUTOWAH_ERROR_OK = 0,					/**< no error              */
} AUTOWAH_ERROR_CODE;


/** Auto Wah Context */
typedef struct _AutoWahContext
{
	//int32_t num_channels;
	int32_t sample_rate;
	int32_t mod_count;	
	int32_t mod_rate;
	int32_t mod_rate_step;
	int32_t freq_mid;
	int32_t freq_delta_max;
	int32_t depth;	
	int32_t d[4];
	int32_t num_f0;
	int32_t i_f0;
	uint16_t F0[AUTOWAH_MAX_NUM_F0];
	int16_t B[AUTOWAH_MAX_NUM_F0];
	int16_t A[AUTOWAH_MAX_NUM_F0];
} AutoWahContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize the Auto-Wah module.
 * @param ct Pointer to an AutoWahContext object.
 * @param sample_rate Sample rate.
 * @param mod_rate Modulation rate in 0.1Hz. For example, 2 for 0.2Hz, 10 for 1.0Hz, 100 for 10.0Hz. Range: 0.0~10.0Hz in step of 0.1Hz
 * @param freq_min Minimum frequency in the modulation's sweep range. Range: 100~500Hz.
 * @param freq_max Maximum frequency in the modulation's sweep range. Range: 500~5000Hz.
 * @param depth Depth of the effect. Range: 1~100.
 * @return error code. AUTOWAH_ERROR_OK means successful, other codes indicate error. 
 */
int32_t auto_wah_init(AutoWahContext *ct, int32_t sample_rate, int32_t mod_rate, int32_t freq_min, int32_t freq_max, int32_t depth);


/**
 * @brief Apply the Auto-Wah effect to a frame of PCM data (mono only)
 * @param ct Pointer to an AutoWahContext object.
 * @param pcm_in PCM input buffer.
 * @param pcm_out PCM output buffer.
 *        pcm_out can be the same as pcm_in. In this case, the PCM data is changed in-place.
 * @param n Number of PCM samples to process.
 * @param dry The level of dry(direct) signals in the output. Range: 0%~100%.
 * @param wet The level of wet(effect) signals in the output. Range: 0%~100%.
 * @param mod_rate Modulation rate in 0.1Hz. For example, 2 for 0.2Hz, 10 for 1.0Hz, 100 for 10.0Hz. Range: 0.0~10.0Hz in step of 0.1Hz
 * @return error code. AUTOWAH_ERROR_OK means successful, other codes indicate error.
 * @note Note that only mono signals are accepted.
 */
int32_t auto_wah_apply(AutoWahContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int32_t dry, int32_t wet, int32_t mod_rate);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__AUTO_WAH_H__
