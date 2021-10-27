/**
 *************************************************************************************
 * @file	pingpong.h
 * @brief	Ping-Pong Delay
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.4.1
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __PINGPONG_H__
#define __PINGPONG_H__

#include <stdint.h>


/** error code for echo effect */
typedef enum _PINGPONG_ERROR_CODE
{
	PINGPONG_ERROR_ILLEGAL_MAX_DELAY = -128,
	PINGPONG_ERROR_ILLEGAL_BUFFER_POINTER,
	PINGPONG_ERROR_ILLEGAL_WETDRYMIX,
	PINGPONG_ERROR_DELAY_TOO_LARGE,
	PINGPONG_ERROR_DELAY_NOT_POSITIVE,	

	// No Error
	PINGPONG_ERROR_OK = 0,					/**< no error */
} PINGPONG_ERROR_CODE;


/** echo context */
typedef struct _PingPongContext
{
	int32_t wetdrymix;			// ratio of wet/mix
	int32_t wet_scale;			// wet scale
	int32_t p;					// next position for overwriting
	int32_t max_delay_samples;	// maximum delay in samples
	int32_t high_quality;		// high quality switch
	
	int16_t in_block[64];		// input block
	int16_t delay_block[64];	// delay block
	int16_t prevsample[2];		// previous samples
	int8_t index[4];			// encoding index
	uint8_t *s;					// delay buffer
} PingPongContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize ping-pong delay module
 * @param ct Pointer to an PingPongContext object.
 * @param max_delay_samples Maximum delay in samples. For example if you want to have maximum 500ms delay at 44.1kHz sample rate, the max_delay_samples = delay time*sample rate = 500*44.1 = 22050.
 * @param high_quality High quality switch. If high_quality is set 1, the delay values are losslessly saved for high quality output, otherwise (high_quality = 0) the delay values are compressed for longer maximum delay.
 * @param s Delay buffer pointer. This buffer should be allocated by the caller and its capacity depends on both "high_quality" and "max_delay_samples". 
 *        If high_quality is set 1, the buffer capacity = "max_delay_samples*4" in bytes. For example if max_delay_samples = 22050, then the buffer capacity should be 88200 bytes (22050*4=88200)
 *        If high_quality is set 0, the buffer capacity = "ceil(max_delay_samples/32)*38" in bytes. For example if max_delay_samples = 22050, then the buffer capacity should be 26220 bytes (ceil(22050/32)*38=690*38=26220)
 * @return error code. PINGPONG_ERROR_OK means successful, other codes indicate error.
 */
int32_t pingpong_init(PingPongContext *ct, int32_t max_delay_samples, int32_t high_quality, uint8_t *s);


/**
 * @brief Apply ping-pong delay to a frame of PCM data (must be stereo, i.e. 2 channels)
 * @param ct Pointer to a PingPongContext object.
 * @param pcm_in Address of the PCM input. The data layout for stereo: L0,R0,L1,R1,L2,R2,...
 * @param pcm_out Address of the PCM output. The data layout for stereo: L0,R0,L1,R1,L2,R2,...
 *        pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
 * @param n Number of PCM samples to process.
 * @param attenuation attenuation coefficient. Q1.15 format to represent value in range from 0 to 1. For example, 8192 represents 0.25 as the attenuation coefficient.
 * @param delay_samples Delay in samples. Range: 1 ~ max_delay_samples.
 * @param wetdrymix The ratio of wet (ping-pong delay) signal to the mixed output (wet+dry). Range: 0~100 for 0~100%.
 * @return error code. PINGPONG_ERROR_OK means successful, other codes indicate error.
 * @note Only stereo (2 channels) PCM signals are supported for processing.
 */
int32_t pingpong_apply(PingPongContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int16_t attenuation, int32_t delay_samples, int32_t wetdrymix);



#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__PINGPONG_H__
