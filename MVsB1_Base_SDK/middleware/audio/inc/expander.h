/**
*************************************************************************************
* @file	expander.h
* @brief	Expander
*
* @author	ZHAO Ying (Alfred)
* @version	v1.2.0
*
* &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
*************************************************************************************
*/

#ifndef __EXPANDER_H__
#define __EXPANDER_H__

#include <stdint.h>


/** error code for expander */
typedef enum _EXPANDER_ERROR_CODE
{
	EXPANDER_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	EXPANDER_ERROR_NUM_CHANNELS_NOT_SUPPORTED,
	EXPANDER_ERROR_THRESHOLD_OUT_OF_RANGE,
	EXPANDER_ERROR_RATIO_OUT_OF_RANGE,
	// No Error
	EXPANDER_ERROR_OK = 0,					/**< no error              */
} EXPANDER_ERROR_CODE;


/** Expander context */
typedef struct _ExpanderContext
{
	int32_t num_channels;
	int32_t sample_rate;
	//int32_t knee;				// knee width in dB (0: hard knee; other values: soft knee)
	int32_t threshold;
	int32_t slope;
	int32_t alpha_attack;
	int32_t alpha_release;	
	int32_t state;
} ExpanderContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize Expander module
 * @param ct Pointer to a ExpanderContext object.
 * @param num_channels Number of channels. 
 * @param sample_rate Sample rate.
 * @param threshold The level below which gain is applied to the input signal. Set the threshold in step of 0.01dB. Allowable range: -9000 ~ 0 to cover -90.00dB ~ 0.00dB. For example, -2550 stands for -25.50dB threshold.
 * @param ratio  The output/input ratio for signals that undershoot the operation threshold.
 * @param attack_time Amount of time (in millisecond) it takes applied gain to ramp up.
 * @param release_time Amount of time (in millisecond) it takes applied gain to ramp down.
 * @return error code. EXPANDER_ERROR_OK means successful, other codes indicate error.
 */
int32_t expander_init(ExpanderContext *ct, int32_t num_channels, int32_t sample_rate,
				 int32_t threshold, int32_t ratio, int32_t attack_time, int32_t release_time);


/**
 * @brief Apply Dynamic Range Expansion to a frame of PCM data.
 * @param ct Pointer to a ExpanderContext object.
 * @param pcm_in Address of the PCM input buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
 * @param pcm_out Address of the PCM output buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
 *        pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
 * @param n Number of PCM samples to process.
 * @return error code. EXPANDER_ERROR_OK means successful, other codes indicate error.
 */
int32_t expander_apply(ExpanderContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


/**
 * @brief Apply Dynamic Range Expansion to a frame of PCM data (24-bit PCM in & out).
 * @param ct Pointer to a ExpanderContext object.
 * @param pcm_in Address of the PCM input buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
 * @param pcm_out Address of the PCM output buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
 *        pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
 * @param n Number of PCM samples to process.
 * @return error code. EXPANDER_ERROR_OK means successful, other codes indicate error.
 */
int32_t expander_apply24(ExpanderContext *ct, int32_t *pcm_in, int32_t *pcm_out, int32_t n);


/**
* @brief Set the threshold of expander.
* @param ct Pointer to a ExpanderContext object.
* @param threshold The level below which gain is applied to the input signal. Set the threshold in step of 0.01dB. Allowable range: -9000 ~ 0 to cover -90.00dB ~ 0.00dB. For example, -2550 stands for -25.50dB threshold.
* @return error code. EXPANDER_ERROR_OK means successful, other codes indicate error.
*/
int32_t expander_set_threshold(ExpanderContext *ct, int32_t threshold);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif
