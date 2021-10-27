/**
 *************************************************************************************
 * @file	stereo_widener.h
 * @brief	Stereo widening effect
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.2.0
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __STEREO_WIDENER_H__
#define __STEREO_WIDENER_H__

#include <stdint.h>


/** error code for stereo widener */
typedef enum _STEREO_WIDENER_ERROR_CODE
{
    STEREO_WIDENER_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	STEREO_WIDENER_ERROR_NON_POSITIVE_NUMBER_OF_SAMPLES,
	// No Error
	STEREO_WIDENER_ERROR_OK = 0,					/**< no error              */
} STEREO_WIDENER_ERROR_CODE;


/** Stereo widener context */
typedef struct _StereoWidenerContext
{
	//int32_t num_channels;
	//int32_t sample_rate;
	int32_t shaping;
	int32_t *LS_COEF;
	int32_t *HS_COEF;
	int32_t dp;
	int32_t dlen;
	int16_t sd[1200];
	int32_t sls[2][4];
	int32_t shs[2][4];	
} StereoWidenerContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize stereo widening effect module
 * @param ct Pointer to a StereoWidenerContext object. 
 * @param sample_rate Sample rate.
 * @param shaping Set to apply spectrum shaping or not. 0:no shaping, 1:shaping
 * @return error code. STEREO_WIDENER_ERROR_OK means successful, other codes indicate error.
 */
int32_t stereo_widener_init(StereoWidenerContext *ct, int32_t sample_rate, int32_t shaping);


/**
 * @brief Apply stereo widening effect to a frame of PCM data (2 channels)
 * @param ct Pointer to a StereoWidenerContext object.
 * @param pcm_in Address of the PCM input. The PCM layout is like "L,R,L,R,..."
 * @param pcm_out Address of the PCM output. The PCM layout is like "L,R,L,R,..."
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @param n Number of PCM samples to process. Each pair of (L,R) is counted as 1.
 * @return error code. STEREO_WIDENER_ERROR_OK means successful, other codes indicate error.
 * @note Only stereo (2 channels) PCM signals are supported for processing.
 */
int32_t stereo_widener_apply(StereoWidenerContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__STEREO_WIDENER_H__
