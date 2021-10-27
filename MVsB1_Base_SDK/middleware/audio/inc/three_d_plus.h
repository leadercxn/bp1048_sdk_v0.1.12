/**
 *************************************************************************************
 * @file	three_d_plus.h
 * @brief	3D+ audio effect for stereo signals
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.0.1
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __THREE_D_PLUS_H__
#define __THREE_D_PLUS_H__

#include <stdint.h>

/** error code for 3D+ */
typedef enum _THREE_D_PLUS_ERROR_CODE
{
    THREE_D_PLUS_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,	
    THREE_D_PLUS_ERROR_NON_POSITIVE_NUMBER_OF_SAMPLES,
	THREE_D_PLUS_ERROR_ILLEGAL_INTENSITY,
	THREE_D_PLUS_ERROR_PERMISSION_NOT_GRANTED,

	// No Error
	THREE_D_PLUS_ERROR_OK = 0,					/**< no error              */
} THREE_D_PLUS_ERROR_CODE;


/** 3D context */
typedef struct _ThreeDPlusContext
{
	//int32_t num_channels;
	int32_t sample_rate;	
	int32_t intensity;
	int32_t f[28];
	int32_t s[11][2][4];
} ThreeDPlusContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize 3D+ audio effect module
 * @param ct Pointer to a ThreeDPlusContext object. 
 * @param sample_rate Sample rate. 
 * @return error code. THREE_D_PLUS_ERROR_OK means successful, other codes indicate error.
 * @note Only 2 channel PCM input is supported.
 */
int32_t three_d_plus_init(ThreeDPlusContext *ct, int32_t sample_rate);


/**
 * @brief Apply 3D+ audio effect to a frame of PCM data(stereo).
 * @param ct Pointer to a ThreeDPlusContext object.
 * @param pcm_in Address of the PCM input. The PCM layout is like "L,R,L,R,..."
 * @param pcm_out Address of the PCM output. The PCM layout is like "L,R,L,R,..."
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @param n Number of PCM samples to process.
 * @param intensity 3D+ intensity. range: 0 ~ 100
 * @return error code. THREE_D_PLUS_ERROR_OK means successful, other codes indicate error.
 * @note Only 2 channel PCM input is supported.
 */
int32_t three_d_plus_apply(ThreeDPlusContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int32_t intensity);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__THREE_D_PLUS_H__
