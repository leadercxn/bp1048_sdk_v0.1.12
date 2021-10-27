/**
 *************************************************************************************
 * @file	pcm_delay.h
 * @brief	Delay of PCM samples
 *
 * @author	ZHAO Ying (Alfred)
 * @version	V2.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *
 *************************************************************************************
 */

#ifndef _PCM_DELAY_H
#define _PCM_DELAY_H

#include <stdint.h>


 /** error code for noise gate */
typedef enum _PCMDELAY_ERROR_CODE
{
    PCMDELAY_ERROR_NUMBER_OF_CHANNELS_NOT_SUPPORTED = -256,
	PCMDELAY_ERROR_ILLEGAL_MAX_DELAY,
	PCMDELAY_ERROR_ILLEGAL_BUFFER_POINTER,
    PCMDELAY_ERROR_INVALID_DELAY_SAMPLES,
    // No Error
    PCMDELAY_ERROR_OK = 0,					/**< no error              */
} PCMDELAY_ERROR_CODE;

typedef struct _PCMDelay
{
	int32_t num_channels;		// number of channels
	int32_t p;					// next position for overwriting
	int32_t max_delay_samples;	// maximum delay in samples
	int32_t high_quality;		// high quality switch
	int16_t in_block[64];		// input block
	int16_t delay_block[64];	// delay block
	int16_t prevsample[2];		// previous samples
	int8_t index[4];			// encoding index
	uint8_t *s;					// delay buffer
} PCMDelay;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief Initialization of PCM delay unit.
 * @param ct Pointer to the PCM delay context structure. The structure's memory should be allocated by the calling process.
 * @param num_channels Number of channels. Both 1 channel and 2 channels are supported.
 * @param max_delay_samples Maximum delay in samples. For example if you want to have maximum 500ms delay at 44.1kHz sample rate, the max_delay_samples = delay time*sample rate = 500*44.1 = 22050.
 * @param high_quality High quality switch. If high_quality is set 1, the delay values are losslessly saved for high quality output, otherwise (high_quality = 0) the delay values are compressed for longer maximum delay.
 * @param s Delay buffer pointer. This buffer should be allocated by the caller and its capacity depends on "num_channels", "high_quality" and "max_delay_samples".
 *        If high_quality is set 1, the buffer capacity = "max_delay_samples*2*num_channels" in bytes. For example if max_delay_samples = 22050, then the buffer capacity should be 88200 bytes (22050*2*2) for stereo input or 44100 bytes (22050*2*1) for mono input.
 *        If high_quality is set 0, the buffer capacity = "ceil(max_delay_samples/32)*19*num_channels" in bytes. For example if max_delay_samples = 22050, then the buffer capacity should be 26220 bytes (ceil(22050/32)*19*2) for stereo input or 13110 (ceil(22050/32)*19*1) for mono input.
 * @return error code. PCMDELAY_ERROR_OK means successful, other codes indicate error.
 */
int32_t pcm_delay_init(PCMDelay* ct, int32_t num_channels, int32_t max_delay_samples, int32_t high_quality, uint8_t *s);


/**
 * @brief Apply PCM delay to a frame of PCM data.
 * @param ct Pointer to a PCMDelay object.
 * @param pcm_in Address of the PCM input. The data layout for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 * @param pcm_out Address of the PCM output. The data layout for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 *        pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
 * @param n Number of PCM samples to process.
 * @param delay_samples Delay in samples. e.g. 40ms delay @ 44.1kHz = 40*44.1 = 1764 samples. This value should not be greater than max_delay_samples or less than 0.
 * @return error code. PCMDELAY_ERROR_OK means successful, other codes indicate error.
 */
int32_t pcm_delay_apply(PCMDelay* ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int32_t delay_samples);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif // _PCM_DELAY_H
