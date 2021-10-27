/**
 *************************************************************************************
 * @file	freqshifter.h
 * @brief	Frequency shifter for 44.1kHz mono signals
 *
 * @author	ZHAO Ying (Alfred)
 * @version	V1.6.3
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *
 *************************************************************************************
 */

#ifndef _FREQSHIFTER_H
#define _FREQSHIFTER_H

#include <stdint.h>


typedef struct _FreqShifterContext
{
	int32_t d[4*4*2+1];		// delay buffer	
	//int32_t fs;				// sample rate
	int32_t deltaf;			// shift frequency in Hz
	int32_t costab_size;
	const int16_t *costab;
	int32_t n;				// sample counter
	//int32_t nscale;			// scale factor
	//int32_t s[4];			// filter delay	
} FreqShifterContext;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize frequency shifter module
 * @param ct Pointer to a FreqShifterContext object. 
 * @param deltaf Frequency shift in Hz. Allowable value: -7, -5, -3, +3, +5, +7
 * @return error code. 0 means successful, other codes indicate error.
 * @note Input signals should be always 44.1kHz mono PCM values.
 */
int32_t freqshifter_init(FreqShifterContext* ct, int32_t deltaf);


/**
 * @brief Apply frequency shifting to a frame of PCM data.
 * @param ct Pointer to a FreqShifterContext object.
 * @param pcm_in Address of the PCM input.
 * @param pcm_out Address of the PCM output.
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @param pcm_n Number of PCM samples to process. 
 * @return none.
 * @note Input signals should be always 44.1kHz mono PCM values.
 */
void freqshifter_apply(FreqShifterContext* ct, int16_t *pcm_in, int16_t *pcm_out, int32_t pcm_n);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _FREQSHIFTER_H

