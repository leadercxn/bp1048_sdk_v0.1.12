/**
 *************************************************************************************
 * @file	three_d.h
 * @brief	3D audio effect 
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v3.3.0
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __THREE_D_H__
#define __THREE_D_H__

#include <stdint.h>

//#define MAX_CH 2
//#define MAX_N 192


/** error code for 3D */
typedef enum _THREE_D_ERROR_CODE
{
    THREE_D_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	THREE_D_ERROR_NUMBER_OF_CHANNELS_NOT_SUPPORTED,
    THREE_D_ERROR_NON_POSITIVE_NUMBER_OF_SAMPLES,
	// No Error
	THREE_D_ERROR_OK = 0,					/**< no error              */
} THREE_D_ERROR_CODE;


/** 3D context */
typedef struct _ThreeDContext
{
	int32_t num_channels;
	int32_t sample_rate;
	int32_t sample_rate_index;
	int32_t intensity;
	int32_t pre_gain;
	int32_t post_gain;	

	//double sd;	// delayed samples
	//double s0[4];	// filter delays
	//double s1[2];	// filter delays
	int32_t sd;		// delayed samples
	int32_t s0[4];	// filter delays
	int32_t s1[2];	// filter delays
	
} ThreeDContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize 3D audio effect module
 * @param td Pointer to a ThreeDContext object.
 * @param num_channels number of channels 
 * @param sample_rate Sample rate. 
 * @return error code. THREE_D_ERROR_OK means successful, other codes indicate error.
 */
int32_t three_d_init(ThreeDContext *td, int32_t num_channels, int32_t sample_rate);


/**
 * @brief Apply 3D audio effect to a frame of PCM data.
 * @param td Pointer to a ThreeDContext object.
 * @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
 * @param pcm_out Address of the PCM output. The PCM layout is the same as in Microsoft WAVE format for both mono and stereo cases.
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @param n Number of PCM samples to process.
 * @param intensity 3D intensity. range: 0 ~ 100
 * @return error code. THREE_D_ERROR_OK means successful, other codes indicate error.
 */
int32_t three_d_apply(ThreeDContext *td, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int32_t intensity);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__THREE_D_H__
