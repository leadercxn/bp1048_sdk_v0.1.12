/**
*************************************************************************************
* @file	resampler.h
* @brief	Resampler for sample rate conversion
*
* @author	ZHAO Ying (Alfred)
* @version	v1.0.3
*
* &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
*************************************************************************************
*/

#ifndef __RESAMPLER_H__
#define __RESAMPLER_H__

#include <stdint.h>

#define MAX_FRAME_SAMPLES 128
#define MAX_RESAMPLE_FACTOR 6	// e.g. 8000->48000Hz
#define MAX_XOFF_SIZE 208		// =((65+1)/2.0) * max(1.0, MAX_RESAMPLE_FACTOR) + 10
#define MAX_IBUFF_SIZE (MAX_XOFF_SIZE * 2 + MAX_FRAME_SAMPLES)
#define MAX_OBUFF_SIZE (MAX_FRAME_SAMPLES * MAX_RESAMPLE_FACTOR + MAX_XOFF_SIZE)


/** error code for resampler */
typedef enum _RESAMPLER_ERROR_CODE
{
	RESAMPLER_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	RESAMPLER_ERROR_TARGET_SAMPLE_RATE_OUT_OF_RAGE,
	RESAMPLER_ERROR_NUMBER_OF_CHANNELS_NOT_SUPPORTED,
	RESAMPLER_ERROR_NUMBER_OF_INPUT_PCM_SAMPLES_TOO_BIG,
	RESAMPLER_ERROR_NUMBER_OF_INPUT_PCM_SAMPLES_TOO_SMALL,

	// No Error
	RESAMPLER_ERROR_OK = 0,					/**< no error              */
} RESAMPLER_ERROR_CODE;

/** reverb context */
typedef struct _ResamplerContext
{
	int32_t sr_original;
	int32_t sr_target;
	int32_t num_channels;	
	int32_t higher_filter_precision;
	int32_t higher_filter_order;
	double	factor;
	
	uint16_t LpScl;
	uint16_t Nwing;
	uint16_t Nmult;
	uint16_t Xoff;
	uint16_t Xread;
	uint16_t dummy;

	uint32_t Time;	
	uint32_t dtb;
	uint32_t dhb;
	
	int16_t *Imp;
	int16_t *ImpD;
	
	int16_t x1[MAX_IBUFF_SIZE];
	int16_t y1[MAX_OBUFF_SIZE];
	int16_t x2[MAX_IBUFF_SIZE];
	int16_t y2[MAX_OBUFF_SIZE];

} ResamplerContext;

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
* @brief Initialize resampler module.
* @param ct Pointer to a ResamplerContext object.
* @param num_channels number of channels. Both 1 and 2 channels are supported.
* @param sr_original original sample rate
* @param sr_target target sample rate. This value should not be more than sr_original x MAX_RESAMPLE_FACTOR or less than sr_original / MAX_RESAMPLE_FACTOR.
* @param higher_filter_precision enable higher precision filter coefficients. 0: disable, 1: enable higher precision
* @param higher_filter_order enable higher order filter. 0: disable, 1: enable higher order
* @return error code. RESAMPLER_ERROR_OK means successful, other codes indicate error.
*/
int32_t resampler_init(ResamplerContext *ct, int32_t num_channels, int32_t sr_original, int32_t sr_target, 
	int32_t higher_filter_precision, int32_t higher_filter_order);


/**
* @brief Apply resampling (sample rate conversion) to a frame of PCM data.
* @param ct Pointer to a ResamplerContext object.
* @param pcm_in Address of the PCM input buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
* @param pcm_out Address of the PCM output buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
*        pcm_out can be the same as pcm_in but the number of output PCM samples may not be the same as n.
* @param n Number of input PCM samples. n should not exceed MAX_FRAME_SAMPLES.
* @return Number of output PCM samples. If this value is not positive, error occurs in the resampling process and the returned value is actually the error code instead.
*/
int32_t resampler_apply(ResamplerContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__RESAMPLER_H__
