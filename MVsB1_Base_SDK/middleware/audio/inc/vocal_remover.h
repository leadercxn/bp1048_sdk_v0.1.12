/**
*************************************************************************************
* @file	    vocal_remover.h
* @brief	Remove vocal component from stereo recordings
*
* @author	ZHAO Ying (Alfred)
* @version	v1.3.2
*
* &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
*************************************************************************************
*/

#ifndef __VOCAL_REMOVER_H__
#define __VOCAL_REMOVER_H__

#include <stdint.h>


/** error code for vocal remover */
typedef enum _VOCAL_REMOVER_ERROR_CODE
{
    VOCAL_REMOVER_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
    VOCAL_REMOVER_ERROR_ILLEGAL_VOCAL_RANGE,
    VOCAL_REMOVER_ERROR_ILLEGAL_STEP_SIZE,
    VOCAL_REMOVER_ERROR_PERMISSION_NOT_GRANTED,

    // No Error
    VOCAL_REMOVER_ERROR_OK = 0,					/**< no error              */
} VOCAL_REMOVER_ERROR_CODE;

/** vocal remover context */
typedef struct _VocalRemoverContext
{
    int32_t sample_rate;
    //int32_t num_channels;
    int32_t step_size;
    int32_t scale_factor;
    int32_t kl, kh;
    int32_t x[2][2048];
    int16_t pcm_x[2][2048];
    int32_t pcm_y[2][2048];

} VocalRemoverContext;
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
* @brief Initialize the module of vocal remover.
* @param ct Pointer to a VocalRemoverContext object.
* @param sample_rate Sample rate.
* @param lower_freq Lower limit frequency of the vocal detect range in Hz.
* @param higher_freq Higher limit frequency of the vocal detect range in Hz.
* @param step_size Step size in samples. Only 512, 256, or 128 is allowed.
* @return error code. VOCAL_REMOVER_ERROR_OK means successful, other codes indicate error.
*/
int32_t vocal_remover_init(VocalRemoverContext *ct, int32_t sample_rate, int32_t lower_freq, int32_t higher_freq, int32_t step_size);


/**
* @brief Apply vocal remover effect to a frame of PCM data (must be stereo)
* @param ct Pointer to a VocalRemoverContext object.
* @param pcm_in PCM input buffer. The PCM layout is like "L,R,L,R,..."
* @param pcm_out PCM output buffer. The PCM layout is like "L,R,L,R,..."
*        pcm_out can be the same as pcm_in. In this case, the PCM data is changed in-place.
* @return error code. VOCAL_REMOVER_ERROR_OK means successful, other codes indicate error.
* @note Note that the number of PCM samples per frame is equal to the step_size set in vocal_remover_init(...).
*/
int32_t vocal_remover_apply(VocalRemoverContext *ct, int16_t *pcm_in, int16_t *pcm_out);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif // __VOCAL_REMOVER_H__
