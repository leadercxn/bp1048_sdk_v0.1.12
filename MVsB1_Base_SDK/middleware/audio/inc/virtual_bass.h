/**
 *************************************************************************************
 * @file	virtual_bass.h
 * @brief	Virtual bass (bass extension).
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v4.1.4
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __VIRTUAL_BASS_H__
#define __VIRTUAL_BASS_H__

#include <stdint.h>

#define VB_MIN_CUTOFF_FREQ 30
#define VB_MAX_CUTOFF_FREQ 300
//#define MAX_CH 2
//#define MAX_N 192

/** error code for virtual bass */
typedef enum _VB_ERROR_CODE
{
    VB_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
	VB_ERROR_NUMBER_OF_CHANNELS_NOT_SUPPORTED,
	VB_ERROR_ILLEGAL_CUTOFF_FREQ,
	VB_ERROR_ILLEGAL_INTENSITY_VALUES,
	// No Error
	VB_ERROR_OK = 0,					/**< no error              */
} VB_ERROR_CODE;

/** virtual bass context */
typedef struct _VBContext
{
	int32_t sample_rate;
	int32_t num_channels;
	int32_t state;
	int32_t sd;
	int32_t cs;
	//double cs;	
	int32_t fco[7];		// gl0, gl1, gh0, gh1, a1, a2, a1'
	//int32_t fpre[3];	// gl, gh, a1
	//int32_t fop[8];		// gl0, gl1, gh0, gh1, a1, a2, a1', a2'
	//int32_t fop[9];		// g, b1, b2, a1, a2, b1, b2, a1, a2
	//int32_t fop[4];		// gl, gh, a1, a2
	int32_t ffb[4];		// gl, gh, a1, a2
	//int32_t flp[3];		// gl, gh, a1
	int32_t d[11][4];	// filter delays
} VBContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief Initialize virtual bass audio effect module
 * @param vb Pointer to a VBContext object.
 * @param num_channels number of channels. Both 1 and 2 channels are supported.
 * @param sample_rate sample rate
 * @param f_cut Cut-off frequency in Hz. Valid range: VB_MIN_CUTOFF_FREQ ~ VB_MAX_CUTOFF_FREQ Hz. Components below this cut-off frequency are attenuated and reproduced in another form (virtual effect).
 * @return error code. VB_ERROR_OK means successful, other codes indicate error.
 */
int32_t vb_init(VBContext *vb, int32_t num_channels, int32_t sample_rate, int32_t f_cut);


/**
 * @brief Apply virtual bass audio effect to a frame of PCM data.
 * @param vb Pointer to a VBContext object.
 * @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
 * @param pcm_out Address of the PCM output. The PCM layout must be the same as in Microsoft WAVE format for both mono and stereo cases.
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @param n Number of PCM samples to process. 
 * @param intensity Intensity control of virtual bass effect. Valid values: 0 ~ 100.
 * @param enhanced Switch of the enhanced mode. 0:Off, 1:On.
 * @return error code. VB_ERROR_OK means successful, other codes indicate error.
 */
int32_t vb_apply(VBContext *vb, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int32_t intensity, int32_t enhanced);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__VIRTUAL_BASS_H__
