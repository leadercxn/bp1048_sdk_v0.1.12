/**
 *************************************************************************************
 * @file	reverb.h
 * @brief	Reverberation effect
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.9.2
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __REVERB_H__
#define __REVERB_H__

#include <stdint.h>

#define COMB_NUM 8
#define COMB1L_SIZE 1116
#define COMB2L_SIZE 1188
#define COMB3L_SIZE 1277
#define COMB4L_SIZE 1356
#define COMB5L_SIZE 1422
#define COMB6L_SIZE 1491
#define COMB7L_SIZE 1557
#if COMB_NUM == 8
#define COMB8L_SIZE 1617
#else
#define COMB8L_SIZE 0
#endif

#define ALLPASS_NUM 4
#define ALLPASS1L_SIZE 556
#define ALLPASS2L_SIZE 441
#define ALLPASS3L_SIZE 341
#define ALLPASS4L_SIZE 225

#define STEREO_SPREAD 23
//#define DELAYL_TOTAL_SIZE (COMB1L_SIZE+COMB2L_SIZE+COMB3L_SIZE+COMB4L_SIZE+COMB5L_SIZE+COMB6L_SIZE+COMB7L_SIZE+COMB8L_SIZE+ALLPASS1L_SIZE+ALLPASS2L_SIZE+ALLPASS3L_SIZE+ALLPASS4L_SIZE)
//#define DELAYR_TOTAL_SIZE (DELAYL_TOTAL_SIZE+STEREO_SPREAD*(COMB_NUM+ALLPASS_NUM))
#define DELAYL8_COMB_SIZE (COMB1L_SIZE+COMB2L_SIZE+COMB3L_SIZE+COMB4L_SIZE+COMB5L_SIZE+COMB6L_SIZE+COMB7L_SIZE+COMB8L_SIZE)
#define DELAYL8_ALLPASS_SIZE (ALLPASS1L_SIZE+ALLPASS2L_SIZE+ALLPASS3L_SIZE+ALLPASS4L_SIZE)
#define DELAYR8_COMB_SIZE (DELAYL8_COMB_SIZE+STEREO_SPREAD*8)
#define DELAYR8_ALLPASS_SIZE (DELAYL8_ALLPASS_SIZE+STEREO_SPREAD*ALLPASS_NUM)

/** error code for reverb */
typedef enum _REVERB_ERROR_CODE
{
    REVERB_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	REVERB_ERROR_NUMBER_OF_CHANNELS_NOT_SUPPORTED,	
	REVERB_ERROR_ILLEGAL_DRY_SCALE,
	REVERB_ERROR_ILLEGAL_WET_SCALE,
	REVERB_ERROR_ILLEGAL_WIDTH_SCALE,
	REVERB_ERROR_ILLEGAL_ROOMSIZE_SCALE,
	REVERB_ERROR_ILLEGAL_DAMP_SCALE,

	// No Error
	REVERB_ERROR_OK = 0,					/**< no error              */
} REVERB_ERROR_CODE;

/** reverb context */
typedef struct _ReverbContext
{
	int32_t sample_rate;
	int32_t num_channels;	
	int32_t dry;
	int32_t wet, wet1, wet2;
	int32_t roomsize;
	int32_t damping;
	//int16_t d0[DELAYL_TOTAL_SIZE];
	//int16_t d1[DELAYR_TOTAL_SIZE];
	int16_t d0c[DELAYL8_COMB_SIZE];
	int16_t d1c[DELAYR8_COMB_SIZE];
	int32_t d0a[DELAYL8_ALLPASS_SIZE];
	int32_t d1a[DELAYR8_ALLPASS_SIZE];
	int32_t s0[COMB_NUM];
	int32_t s1[COMB_NUM];
	int32_t comb_bufidx0[COMB_NUM];
	int32_t comb_bufidx1[COMB_NUM];
	int32_t allpass_bufidx0[ALLPASS_NUM];
	int32_t allpass_bufidx1[ALLPASS_NUM];
} ReverbContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief Initialize reverberation audio effect module.
 * @param ct Pointer to a ReverbContext object.
 * @param num_channels number of channels. Both 1 and 2 channels are supported.
 * @param sample_rate sample rate
 * @return error code. REVERB_ERROR_OK means successful, other codes indicate error.
 */
int32_t reverb_init(ReverbContext *ct, int32_t num_channels, int32_t sample_rate);


/**
 * @brief Apply reverberation effect to a frame of PCM data.
 * @param ct Pointer to a ReverbContext object.
 * @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
 * @param pcm_out Address of the PCM output. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @param n Number of PCM samples to process. 
 * @return error code. REVERB_ERROR_OK means successful, other codes indicate error.
 */
int32_t reverb_apply(ReverbContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


/**
 * @brief Configure reverberation effect's parameters.
 * @param ct Pointer to a ReverbContext object.
 * @param dry_scale Scale of the dry part in the output. Range: 0 ~ 200 for 0 ~ 200% of the original input.
 * @param wet_scale Scale of the wet part in the output. Range: 0 ~ 300 for 0 ~ 300% of the reverb effect.
 * @param width_scale Scale of the stereo seperation in the output. Range: 0 ~ 100 for 0 ~ 100% seperation.
 * @param roomsize_scale Scale of the room size. Range: 0 ~ 100 for 0 ~ 100% of the room size
 * @param damping_scale Scale of the damping factor. Range: 0 ~ 100 for 0 ~ 100% of the damping factor.
 * @return error code. REVERB_ERROR_OK means successful, other codes indicate error.
 */
int32_t reverb_configure(ReverbContext *ct, int32_t dry_scale, int32_t wet_scale, int32_t width_scale, int32_t roomsize_scale, int32_t damping_scale);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__REVERB_H__
