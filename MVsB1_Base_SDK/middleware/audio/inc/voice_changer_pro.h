/**
 *************************************************************************************
 * @file	voice_changer_pro.h
 * @brief	Voice Changer Pro for mono signals
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v2.3.1
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __VOICE_CHANGER_PRO_H__
#define __VOICE_CHANGER_PRO_H__

#include <stdint.h>


/** error code for voice changer pro*/
typedef enum _VOICECHANGER_PRO_ERROR_CODE
{
    VOICECHANGER_PRO_ERROR_UNSUPPORTED_NUMBER_OF_CHANNELS = -256,
	VOICECHANGER_PRO_ERROR_SAMPLE_RATE_NOT_SUPPORTED,
    VOICECHANGER_PRO_ERROR_ILLEGAL_STEP_SIZE,
    VOICECHANGER_PRO_ERROR_PERMISSION_NOT_GRANTED,
	// No Error
	VOICECHANGER_PRO_ERROR_OK = 0,					/**< no error              */
} VOICECHANGER_PRO_ERROR_CODE;


/** Voice Changer Pro Context */
typedef struct _VoiceChangerProContext
{
    int32_t sample_rate;
    //int32_t num_channels;
    int32_t n1;
    int32_t n2;
    int32_t lx;
    int32_t scale_factor;    
    int32_t warp_coef;
    
    int32_t x[2048];
    int32_t xlast; // must be put here
    int16_t pcm_x[2048];
    int32_t pcm_y[4096];
    int32_t phi0[1024];
    int32_t psi[1024];
    int32_t r[1024];

} VoiceChangerProContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize the voice changer.
 * @param ct Pointer to a VoiceChangerContext object.
 * @param sample_rate Sample rate.
 * @param step_size Step size in samples. Only 256 or 128 is allowed.
 * @param pitch_ratio Ratio of new pitch over current pitch in step of 0.01. For example 180 stands for (new pitch/current pitch = 1.80)
 * @param formant_ratio Ratio of new formant over current formant in step of 0.01. For example 110 stands for (new formant/current formant = 1.10)
 * @return error code. VOICECHANGER_ERROR_OK means successful, other codes indicate error. 
 * @note Only mono signals are supported, i.e. number of channels should be 1 only.
 */
int32_t voice_changer_pro_init(VoiceChangerProContext *ct, int32_t sample_rate, int32_t step_size, int32_t pitch_ratio, int32_t formant_ratio);


/**
 * @brief Apply the change of voice
 * @param ct Pointer to a VoiceChangerContext object.
 * @param pcm_in PCM input buffer.
 * @param pcm_out PCM output buffer. pcm_out can be the same as pcm_in. In this case, the PCM data is changed in-place.
 * @return error code. VOICECHANGER_ERROR_OK means successful, other codes indicate error.
 * @note Note that the number of PCM samples per frame is equal to the step_size set in voice_changer_init(...).
 */
int32_t voice_changer_pro_apply(VoiceChangerProContext *ct, int16_t *pcm_in, int16_t *pcm_out);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__VOICE_CHANGER_PRO_H__
