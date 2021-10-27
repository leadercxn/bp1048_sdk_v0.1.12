/**
 *************************************************************************************
 * @file	echo.h
 * @brief	Echo effect
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.9.0
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __ECHO_H__
#define __ECHO_H__

#include <stdint.h>

/** error code for echo effect */
typedef enum _ECHO_ERROR_CODE
{
    ECHO_ERROR_CHANNEL_NUMBER_NOT_SUPPORTED = -256,
	ECHO_ERROR_SAMPLE_RATE_NOT_SUPPORTED,
	ECHO_ERROR_ILLEGAL_MAX_DELAY,
	ECHO_ERROR_ILLEGAL_BUFFER_POINTER,
	ECHO_ERROR_DELAY_TOO_LARGE,	
	ECHO_ERROR_DELAY_NOT_POSITIVE,
	// No Error
	ECHO_ERROR_OK = 0,					/**< no error */
} ECHO_ERROR_CODE;


/** echo context */
typedef struct _EchoContext
{
	int32_t num_channels;		// number of channels
	int32_t lpb, lpa;			// filter coefficients
	int32_t lpd[2];				// filter delays
	int32_t max_delay_samples;	// maximum delay in samples	
	int32_t prev_delay_samples;	// previous delay samples	
	int32_t p;					// next position for overwriting
	int16_t in_block[32];		// input block
	int16_t delay_block[32];	// delay block
	int16_t delay_block1[32];	// delay block 1
	int16_t prevsample;			// previous samples
	int16_t prevsample1;
	int8_t  index;				// encoding index
	int8_t  index1;
	int8_t  index2;
	int8_t  index3;
	uint8_t *s;					// delay buffer
} EchoContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize echo effect module
 * @param ct Pointer to an EchoContext object.
 * @param num_channels Number of channels.
 * @param sample_rate Sample rate.
 * @param fc Cutoff frequency of the low-pass filter in Hz. Set 0 to disable the use of the low-pass filter in echo effect. Note that this value should not exceed half of the sample rate, i.e. Nyquist frequency.
 * @param max_delay_samples Maximum delay in samples. This number should be positive. For example if you'd like to have maximum 500ms delay at 44.1kHz sample rate, the max_delay_samples = delay time*sample rate = 500*44.1 = 22050.
 * @param s Delay buffer pointer. The buffer capacity is "ceil(max_delay_samples/32)*19" in bytes and allocated by the caller. For example if max_delay_samples = 22050, then s buffer should be ceil(22050/32)*19=13110 in bytes.
 * @return error code. ECHO_ERROR_OK means successful, other codes indicate error.
 */
int32_t echo_init(EchoContext *ct, int32_t num_channels, int32_t sample_rate, int32_t fc, int32_t max_delay_samples, uint8_t *s);


/**
 * @brief Apply echo effect to a frame of PCM data.
 * @param ct Pointer to a EchoContext object.
 * @param pcm_in Address of the PCM input. The data layout for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 * @param pcm_out Address of the PCM output. The data layout for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 *        pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
 * @param n Number of PCM samples to process.
 * @param attenuation attenuation coefficient. Q1.15 format to represent value in range from 0 to 1. For example, 8192 represents 0.25 as the attenuation coefficient.
 * @param delay_samples Delay in samples. Range: 1 ~ max_delay_samples.
 * @param direct_sound Direct sound on/off. 1:direct sound on. 0: direct sound off.
 * @return error code. ECHO_ERROR_OK means successful, other codes indicate error.
 */
int32_t echo_apply(EchoContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int16_t attenuation, int32_t delay_samples, int32_t direct_sound);



#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__ECHO_H__
