/**
 *************************************************************************************
 * @file	howling_suppressor.h
 * @brief	Howling detection & suppression for 44.1kHz mono signals
 *
 * @author	Zhao Ying (Alfred)
 * @version	V2.0.1
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *
 *************************************************************************************
 */

#ifndef _HOWLING_SUPPRESSOR_H
#define _HOWLING_SUPPRESSOR_H

#include <stdint.h>

#define PAST_FRAMES 5

typedef struct _HowlingContext
{	
	int32_t ibuf;
	int32_t suppression_bits;
	int32_t suppression_mode;
	int32_t howling_detected;		
	int32_t pcm_win[512];
	int16_t pcm_buf[512*2];
	int32_t x[512*2];
	int64_t x_abs[512];
	uint8_t peaks[512];
	int32_t howling_candidates[PAST_FRAMES*3];
	int32_t fc[3];	
	int32_t d[3][4];
	int32_t dp[3][4];
} HowlingContext;



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize howling suppressor module
 * @param ct Pointer to a HowlingContext object. 
 * @param suppression_mode Suppression mode. 0: No suppression, 1: Precise suppression 2: Debug mode (not for normal use)
 * @return error code. 0 always, i.e. always successful.
 */
int32_t howling_suppressor_init(HowlingContext *ct, int32_t suppression_mode);

/**
 * @brief Apply howling detection & suppression to a frame of PCM data.
 * @param ct Pointer to a HowlingContext object.
 * @param pcm_in PCM input buffer.
 * @param pcm_out PCM output buffer. pcm_out can be the same as pcm_in. In this case, the PCM data is changed in-place.
 * @param n Number of PCM samples to process. It should be no more than 512. It is recommended to keep n constant between frames although it can be varied. The Less n, the higher the CPU consumption.
 * @return none.
 * @note Input signals should be always 44.1kHz mono PCM values.
 */
void howling_suppressor_apply(HowlingContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HOWLING_SUPPRESSOR_H

