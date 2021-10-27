/**
 *************************************************************************************
 * @file	plate_reverb.h
 * @brief	Plate class reverberation effect
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.2.6
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __PLATE_REVERB_H__
#define __PLATE_REVERB_H__

#include <stdint.h>

#define MAX_PLATE_REVERB_PREDELAY 4410 // max. 100ms @ 44.1kHz

/** error code for reverb */
typedef enum _PLATE_REVERB_ERROR_CODE
{
    PLATE_REVERB_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	PLATE_REVERB_ERROR_NUMBER_OF_CHANNELS_NOT_SUPPORTED,
    PLATE_REVERB_ERROR_ILLEGAL_PREDELAY,
	PLATE_REVERB_ERROR_ILLEGAL_DIFFUSION,
	PLATE_REVERB_ERROR_ILLEGAL_DECAY,
	PLATE_REVERB_ERROR_ILLEGAL_DAMPING,
	PLATE_REVERB_ERROR_ILLEGAL_WETDRYMIX,

	// No Error
	PLATE_REVERB_ERROR_OK = 0,					/**< no error              */
} PLATE_REVERB_ERROR_CODE;

/** reverb context */
typedef struct _PlateReverbContext
{
	int32_t sample_rate;
	int32_t num_channels;

	int32_t AP1DELAYLENGTH, AP2DELAYLENGTH, AP3DELAYLENGTH, AP4DELAYLENGTH;
	int32_t AP5DELAYLENGTH, AP6DELAYLENGTH, AP7DELAYLENGTH, AP8DELAYLENGTH;
	int32_t D2DELAYLENGTH, D3DELAYLENGTH, D4DELAYLENGTH, D5DELAYLENGTH;
	int32_t EXCURSION;
	int32_t L_D4_1, L_D4_2, L_AP8, L_D5, L_D2, L_AP6, L_D3;
	int32_t R_D2_1, R_D2_2, R_AP6, R_D3, R_D4, R_AP8, R_D5;

	int32_t modulation;
	int32_t LP1Pole;
	int32_t LP2Pole;
	int32_t LP3Pole;
    int32_t predelay;
	int32_t diffusion;
	int32_t decay;
	int32_t damping;
	int32_t wetdrymix;

    int32_t predelayptr;
    int32_t ap1ptr, ap2ptr, ap3ptr, ap4ptr;
	int32_t ap5ptr, ap6ptr, ap7ptr, ap8ptr;
	int32_t d2ptr, d3ptr, d4ptr, d5ptr;
	int32_t ap5phaseidx;
	int32_t ap7phaseidx;
	int32_t ap5zprev;
	int32_t ap7zprev;	

	int32_t lp1delayline;
	int32_t lp2delayline;
	int32_t lp3delayline;
    int16_t d2delayline[6598];
    int16_t d3delayline[5512];
    int16_t d4delayline[6249];
    int16_t d5delayline[4687];
    int16_t ap1delayline[210];
    int16_t ap2delayline[159];
    int16_t ap3delayline[562];
    int16_t ap4delayline[410];
    int16_t ap5delayline[1008 + 13 + 1];
    int16_t ap6delayline[2667];
    int16_t ap7delayline[1357 + 13 + 1];
    int16_t ap8delayline[3936];
    int16_t predelayline[MAX_PLATE_REVERB_PREDELAY];

} PlateReverbContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief Initialize reverberation audio effect module.
 * @param ct Pointer to a ReverbContext object.
 * @param num_channels Number of channels. Both 1 and 2 channels are supported.
 * @param sample_rate Sample rate.
 * @param highcut_freq Lowpass filter cutoff in the range 0 to sample_rate/2.
 * @param modulation_en Enable delay modulation. 0: disabled, 1: enabled
 * @return error code. PLATE_REVERB_ERROR_OK means successful, other codes indicate error.
 */
int32_t plate_reverb_init(PlateReverbContext *ct, int32_t num_channels, int32_t sample_rate, int32_t highcut_freq, int32_t modulation_en);


/**
 * @brief Apply reverberation effect to a frame of PCM data.
 * @param ct Pointer to a ReverbContext object.
 * @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
 * @param pcm_out Address of the PCM output. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @param n Number of PCM samples to process. 
 * @return error code. PLATE_REVERB_ERROR_OK means successful, other codes indicate error.
 */
int32_t plate_reverb_apply(PlateReverbContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n);


/**
 * @brief Configure reverberation effect's parameters.
 * @param ct Pointer to a ReverbContext object.
 * @param predelay Pre-delay in samples. Range: 0~MAX_PLATE_REVERB_PREDELAY
 * @param diffusion Density of reverb tail. Diffusion is proportional to the rate at which the reverb tail builds in density. Increasing Diffusion pushes the reflections closer together, thickening the sound. Reducing Diffusion creates more discrete echoes. Range: 0~100 for 0~100%.
 * @param decay Decay of reverb tail. Decay factor is proportional to the time it takes for reflections to run out of energy. To model a large room, use a long reverb tail. To model a small room, use a short reverb tail. Range: 0~100 for 0~100%.
 * @param damping High frequency damping. High frequency damping is proportional to the attenuation of high frequencies in the reverberation output. Setting High frequency damping to a large value makes high-frequency reflections decay faster than low-frequency reflections. Range: 0~10000 for 0.00~100.00%.
 * @param wetdrymix The ratio of wet (reverberated) signal to the mixed output (wet+dry). Range: 0~100 for 0~100%.
 * @return error code. PLATE_REVERB_ERROR_OK means successful, other codes indicate error.
 */
int32_t plate_reverb_configure(PlateReverbContext *ct, int32_t predelay, int32_t diffusion, int32_t decay, int32_t damping, int32_t wetdrymix);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__PLATE_REVERB_H__
