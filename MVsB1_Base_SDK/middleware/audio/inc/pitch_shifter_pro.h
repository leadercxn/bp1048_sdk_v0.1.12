/**
 *************************************************************************************
 * @file	pitch_shifter_pro.h
 * @brief	Pitch Shifter Pro
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v2.1.1
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __PITCH_SHIFTER_PRO_H__
#define __PITCH_SHIFTER_PRO_H__

#include <stdint.h>


/** error code for pitch shifter pro*/
typedef enum _PITCHSHIFTER_PRO_ERROR_CODE
{
    PITCHSHIFTER_PRO_ERROR_UNSUPPORTED_NUMBER_OF_CHANNELS = -256,
    PITCHSHIFTER_PRO_ERROR_SAMPLE_RATE_NOT_SUPPORTED,
    PITCHSHIFTER_PRO_ERROR_SEMITONE_STEPS_OUT_OF_RANGE,
    PITCHSHIFTER_PRO_ERROR_ILLEGAL_STEP_SIZE,
    PITCHSHIFTER_PRO_ERROR_PERMISSION_NOT_GRANTED,
	// No Error
    PITCHSHIFTER_PRO_ERROR_OK = 0,					/**< no error              */
} PITCHSHIFTER_PRO_ERROR_CODE;


/** Pitch Shifter Pro Context */
typedef struct _PitchShifterProContext
{
    int32_t sample_rate;
    int32_t num_channels;
    int32_t n1;
    int32_t n2;
    int32_t lx;
    int32_t scale_factor;
    //int32_t warp_coef;
    
    int32_t x[2048];
    int32_t xlast; // must be put here
    int16_t pcm_x[2][2048];
    int32_t pcm_y[2][4096];
    int32_t phi0[2][1024];
    int32_t psi[2][1024];
    int32_t r[1024];

} PitchShifterProContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize the pitch shifter.
 * @param ct Pointer to a PitchShifterProContext object.
 * @param num_channels Number of channels.
 * @param sample_rate Sample rate.
 * @param semitone_steps Number of steps to change in unit of 0.1 semitone. Range: -120 ~ 120 for -12.0 ~ 12.0 semitones
 * @param step_size Step size in samples. Only 512, 256 or 128 is allowed. This is the number of samples to process in each loop.
 * Choose 256 or 128 for better effect, lower delay but higher MCPS (Mega Cycles Per Second).
 * Choose 512 for lower MCPS consumption, but it is only suitable for certain semitone_steps. It imposes higher delay as well.
 * @return error code. PITCHSHIFTER_PRO_ERROR_OK means successful, other codes indicate error. 
 * @note Choice of step_size may affect the MCPS consumption as well as the performance of effect. 
 */
int32_t pitch_shifter_pro_init(PitchShifterProContext *ct, int32_t num_channels, int32_t sample_rate, int32_t semitone_steps, int32_t step_size);


/**
 * @brief Apply the pitch shifting.
 * @param ct Pointer to a PitchShifterProContext object.
 * @param pcm_in PCM input buffer.
 * @param pcm_out PCM output buffer. pcm_out can be the same as pcm_in. In this case, the PCM data is changed in-place.
 * @return error code. PITCHSHIFTER_PRO_ERROR_OK means successful, other codes indicate error.
 * @note Note that the number of PCM samples per frame is equal to the step_size set in pitch_shifter_pro_init(...).
 */
int32_t pitch_shifter_pro_apply(PitchShifterProContext *ct, int16_t *pcm_in, int16_t *pcm_out);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__PITCH_SHIFTER_PRO_H__
