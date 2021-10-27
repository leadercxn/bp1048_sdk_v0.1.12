/**
 *************************************************************************************
 * @file	noise_gate.h
 * @brief	Noise gate.
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.1.3
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __NOISE_GATE_H__
#define __NOISE_GATE_H__

#include <stdint.h>


/** error code for noise gate */
typedef enum _NG_ERROR_CODE
{
    NG_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	NG_ERROR_NUMBER_OF_CHANNELS_NOT_SUPPORTED,
	NG_ERROR_ILLEGAL_THRESHOLD,
	NG_ERROR_ILLEGAL_TIME,
	// No Error
	NG_ERROR_OK = 0,					/**< no error              */
} NG_ERROR_CODE;

/** noise gate context */
typedef struct _NoiseGateCT
{
	int32_t sample_rate;
	int32_t num_channels;
	int32_t ltrhold;
	int32_t utrhold;
	int32_t alpha_attack;
	int32_t alpha_release;
	//int32_t alpha_hold;
	int32_t att, rel, ht;
	int32_t invatt, invrel;
	int32_t state, g;
	int32_t lthcnt;
	int32_t uthcnt;

} NoiseGateCT;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief Initialize noise gate module
 * @param ct Pointer to a NoiseGateCT object.
 * @param num_channels Number of channels. Set 1 for mono signals and 2 for stereo signals.
 * @param sample_rate Sample rate.
 * @param lower_threshold The threshold (in dB) below which the gate will close. Allowable range: -90dB ~ 0dB.
 * @param upper_threshold The threshold (in dB) above which the gate will open. This value should be greater or equal to lower_threshold. Allowable range: -90dB ~ 0dB.
 * @param attack_time The length of time (in millisecond) the gate takes to change from closed to fully open. It is the fade-in duration.
 * @param release_time The length of time (in millisecond) the gate takes to change from open to fully closed. It is the fade-out duration. A fast release abruptly cuts off the sound, whereas a slower release smoothly attenuates the signal from open to closed, resulting in a slow fade-out. If the release time is too short a click can be heard when the gate re-opens.
 * @param hold_time The length of time (in millisecond) the gate will stay fully open after the signal falls below lower_threshold, and before the Release period is commenced. The hold control is often set to ensure the gate does not close during short pauses between words or sentences in a speech signal.
 * @return error code. NG_ERROR_OK means successful, other codes indicate error.
 */
int32_t noise_gate_init(NoiseGateCT *ct, int32_t num_channels, int32_t sample_rate, 
						int32_t lower_threshold, int32_t upper_threshold, int32_t attack_time, int32_t release_time, int32_t hold_time);


/**
 * @brief Apply noise gate to a frame of PCM data.
 * @param vb Pointer to a NoiseGateCT object.
 * @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
 * @param pcm_out Address of the PCM output. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @param n Number of PCM samples to process.
 * @return error code. NG_ERROR_OK means successful, other codes indicate error.
 */
int32_t noise_gate_apply(NoiseGateCT *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__NOISE_GATE_H__
