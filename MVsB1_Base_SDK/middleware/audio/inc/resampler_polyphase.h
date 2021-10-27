/**
*************************************************************************************
* @file	resampler_polyphase.h
* @brief	Resampler based on polyphase implementation
*
* @author	ZHAO Ying (Alfred)
* @version	v0.7.2
*
* &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
*************************************************************************************
*/

#ifndef __RESAMPLER_POLYPHASE_H__
#define __RESAMPLER_POLYPHASE_H__

#include <stdint.h>

#define MAX_FRAME_SAMPLES 128
#define MAX_NUM_TAPS_PER_PHASE 61

/* Common use of sampling rate conversion 
	Target		Original	SRC_RATIO
	44100	<--	8000	RESAMPLER_POLYPHASE_SRC_RATIO_441_80
	44100	<--	11025	RESAMPLER_POLYPHASE_SRC_RATIO_4_1
	44100	<--	12000	RESAMPLER_POLYPHASE_SRC_RATIO_147_40
	44100	<--	16000	RESAMPLER_POLYPHASE_SRC_RATIO_441_160
	44100	<--	22050	RESAMPLER_POLYPHASE_SRC_RATIO_2_1
	44100	<--	24000	RESAMPLER_POLYPHASE_SRC_RATIO_147_80
	44100	<--	32000	RESAMPLER_POLYPHASE_SRC_RATIO_441_320
	44100	<--	48000	RESAMPLER_POLYPHASE_SRC_RATIO_147_160
	44100	<--	88200	RESAMPLER_POLYPHASE_SRC_RATIO_1_2
	44100	<--	96000	RESAMPLER_POLYPHASE_SRC_RATIO_147_320
	44100	<--	176400	RESAMPLER_POLYPHASE_SRC_RATIO_1_4
	44100	<--	192000	RESAMPLER_POLYPHASE_SRC_RATIO_147_640
	44100	<--	33075	RESAMPLER_POLYPHASE_SRC_RATIO_4_3
	33075	<--	44100	RESAMPLER_POLYPHASE_SRC_RATIO_3_4
	16000	<--	48000	RESAMPLER_POLYPHASE_SRC_RATIO_1_3
	16000	<--	44100	RESAMPLER_POLYPHASE_SRC_RATIO_160_441
	16000	<--	8000	RESAMPLER_POLYPHASE_SRC_RATIO_2_1
	8000	<--	16000	RESAMPLER_POLYPHASE_SRC_RATIO_1_2
	48000	<--	8000	RESAMPLER_POLYPHASE_SRC_RATIO_6_1
	48000	<--	11025	RESAMPLER_POLYPHASE_SRC_RATIO_640_147
	48000	<--	12000	RESAMPLER_POLYPHASE_SRC_RATIO_4_1
	48000	<--	16000	RESAMPLER_POLYPHASE_SRC_RATIO_3_1
	48000	<--	22050	RESAMPLER_POLYPHASE_SRC_RATIO_320_147
	48000	<--	24000	RESAMPLER_POLYPHASE_SRC_RATIO_2_1
	48000	<--	32000	RESAMPLER_POLYPHASE_SRC_RATIO_3_2
	48000	<--	44100	RESAMPLER_POLYPHASE_SRC_RATIO_160_147
	48000	<--	88200	RESAMPLER_POLYPHASE_SRC_RATIO_80_147
	48000	<--	96000	RESAMPLER_POLYPHASE_SRC_RATIO_1_2
	48000	<--	176400	RESAMPLER_POLYPHASE_SRC_RATIO_40_147
	48000	<--	192000	RESAMPLER_POLYPHASE_SRC_RATIO_1_4
*/
/** ratio of sampling rate conversion */
typedef enum _RESAMPLER_POLYPHASE_SRC_RATIO
{
	RESAMPLER_POLYPHASE_SRC_RATIO_3_4,		// target:original = 3:4
	RESAMPLER_POLYPHASE_SRC_RATIO_4_3,		// target:original = 4:3
	RESAMPLER_POLYPHASE_SRC_RATIO_1_2,		// target:original = 1:2
	RESAMPLER_POLYPHASE_SRC_RATIO_2_1,		// target:original = 2:1
	RESAMPLER_POLYPHASE_SRC_RATIO_4_1,		// target:original = 4:1
	RESAMPLER_POLYPHASE_SRC_RATIO_1_4,		// target:original = 1:4
	RESAMPLER_POLYPHASE_SRC_RATIO_147_40,	// target:original = 147:40
	RESAMPLER_POLYPHASE_SRC_RATIO_147_80,	// target:original = 147:80
	RESAMPLER_POLYPHASE_SRC_RATIO_160_147,	// target:original = 160:147
	RESAMPLER_POLYPHASE_SRC_RATIO_147_160,	// target:original = 147:160	
	RESAMPLER_POLYPHASE_SRC_RATIO_441_320,	// target:original = 441:320
	RESAMPLER_POLYPHASE_SRC_RATIO_441_160,	// target:original = 441:160
	RESAMPLER_POLYPHASE_SRC_RATIO_441_80,	// target:original = 441:80
	RESAMPLER_POLYPHASE_SRC_RATIO_160_441,	// target:original = 160:441
	RESAMPLER_POLYPHASE_SRC_RATIO_147_320,	// target:original = 147:320
	RESAMPLER_POLYPHASE_SRC_RATIO_147_640,	// target:original = 147:640
	RESAMPLER_POLYPHASE_SRC_RATIO_6_1,		// target:original = 6:1
	RESAMPLER_POLYPHASE_SRC_RATIO_640_147,	// target:original = 640:147
	RESAMPLER_POLYPHASE_SRC_RATIO_3_1,		// target:original = 3:1
	RESAMPLER_POLYPHASE_SRC_RATIO_320_147,	// target:original = 320:147
	RESAMPLER_POLYPHASE_SRC_RATIO_3_2,		// target:original = 3:2
	RESAMPLER_POLYPHASE_SRC_RATIO_40_147,	// target:original = 40:147
	RESAMPLER_POLYPHASE_SRC_RATIO_80_147,	// target:original = 80:147
	RESAMPLER_POLYPHASE_SRC_RATIO_1_3,		// target:original = 1:3

	RESAMPLER_POLYPHASE_SRC_RATIO_UNSUPPORTED,
} RESAMPLER_POLYPHASE_SRC_RATIO;


/** error code for resampler */
typedef enum _RESAMPLER_POLYPHASE_ERROR_CODE
{
	RESAMPLER_POLYPHASE_ERROR_NUMBER_OF_CHANNELS_NOT_SUPPORTED = -256,
	RESAMPLER_POLYPHASE_ERROR_UNSUPPORTED_RATIO,	

	// No Error
	RESAMPLER_POLYPHASE_ERROR_OK = 0,					/**< no error              */
} RESAMPLER_POLYPHASE_ERROR_CODE;

/** ResamplerPolyphase Context */
typedef struct _ResamplerPolyphaseContext
{
	int32_t num_channels;
	int32_t current_phase;
	int32_t zpos;
	int32_t L, M;
	int32_t NUM_TAPS_PER_PHASE;
	const int16_t *FILTER_COEF;
	int32_t zbuf[2 * MAX_NUM_TAPS_PER_PHASE]; // fit for 16-bit/24-bit

} ResamplerPolyphaseContext;

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
* @brief Initialize resampler module.
* @param ct Pointer to a ResamplerPolyphaseContext object.
* @param num_channels number of channels. Both 1 and 2 channels are supported.
* @param src_ratio Choose one from RESAMPLER_POLYPHASE_SRC_RATIO enumeration except RESAMPLER_POLYPHASE_SRC_RATIO_UNSUPPORTED.
* @return error code. RESAMPLER_POLYPHASE_ERROR_OK means successful, other codes indicate error.
*/
int32_t resampler_polyphase_init(ResamplerPolyphaseContext *ct, int32_t num_channels, int32_t src_ratio);


/**
* @brief Apply resampling (sample rate conversion) to a frame of PCM data.
* @param ct Pointer to a ResamplerPolyphaseContext object.
* @param pcm_in Address of the PCM input buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
* @param pcm_out Address of the PCM output buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
*        pcm_out CANNOT be the same as pcm_in and the number of output PCM samples may not be the same as n.
* @param n Number of input PCM samples.
* @return Number of output PCM samples. If this value is not positive, error occurs in the resampling process and the returned value is actually the error code instead.
*/
int32_t resampler_polyphase_apply(ResamplerPolyphaseContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


/**
* @brief Apply resampling (sample rate conversion) to a frame of PCM data (24-bit).
* @param ct Pointer to a ResamplerPolyphaseContext object.
* @param pcm_in Address of the PCM input buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
* @param pcm_out Address of the PCM output buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
*        pcm_out CANNOT be the same as pcm_in and the number of output PCM samples may not be the same as n.
* @param n Number of input PCM samples.
* @return Number of output PCM samples. If this value is not positive, error occurs in the resampling process and the returned value is actually the error code instead.
*/
int32_t resampler_polyphase_apply24(ResamplerPolyphaseContext *ct, int32_t *pcm_in, int32_t *pcm_out, int32_t n);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__RESAMPLER_POLYPHASE_H__
