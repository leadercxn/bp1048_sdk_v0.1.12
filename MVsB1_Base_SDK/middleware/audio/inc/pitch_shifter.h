/**
 *************************************************************************************
 * @file	pitch_shifter.h
 * @brief	Pitch Shifter
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.7.6
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __PITCH_SHIFTER_H__
#define __PITCH_SHIFTER_H__

#include <stdint.h>

#define MAX_W_SIZE 576	// Maximum w size which is related to sample rate
//#define MAX_W_SIZE 1152	// Maximum w size which is related to sample rate

#define UPPER_SEMITONE_LIMIT 120
#define LOWER_SEMITONE_LIMIT -120
#define MAX_SA_SIZE MAX_W_SIZE // Maximum sa size (down to -12 semitones @ 48kHz)
//#define MAX_SA_SIZE 1152 // Maximum sa size (down to -12 semitones @ 48kHz)
//#define LOWER_SEMITONE_LIMIT -180
//#define MAX_SA_SIZE 1632 // Maximum sa size (down to -18 semitones @ 48kHz)


/** error code for PS */
typedef enum _PS_ERROR_CODE
{
    PS_ERROR_UNSUPPORTED_NUMBER_OF_CHANNELS = -256,
	PS_ERROR_FRAME_SIZE_TOO_LARGE,
	PS_ERROR_FRAME_SIZE_NOT_EVEN,
	PS_ERROR_STEPS_OUT_OF_RANGE,
	// No Error
	PS_ERROR_OK = 0,					/**< no error              */
} PS_ERROR_CODE;


/** Pitch Shifter context */
typedef struct _PSContext
{
	int32_t num_channels;							// number of channels
	int32_t sample_rate;							// sample rate
	int32_t sa;										// input frame distance
	int32_t ss;										// output frame distance
	int32_t w;										// frame size for analysis/synthesis
	int32_t wov;									// overlap size
	int32_t shift_bits;								// shift bits
	int32_t kmax;									// max. search range for k
	int32_t xdecim;									// decimation factor for input
	int16_t inbuf[MAX_W_SIZE*4*2];					// input buffer	
	int16_t inbufc[MAX_W_SIZE*3];					// input calculation buffer
	int16_t outbuf[MAX_W_SIZE*2*2];					// output buffer	
	int16_t outbufc[MAX_W_SIZE];					// output calculation buffer
	int16_t xfwin[MAX_W_SIZE];						// overlapping-window
	int32_t ni[MAX_SA_SIZE];						// new position
	int16_t lastPCM[2];								// last PCM samples
} PSContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize Pitch Shifter (PS) module
 * @param ct Pointer to a PSContext object.
 * @param num_channels Number of channels. 
 * @param sample_rate Sample rate.
 * @param semitone_steps Number of steps to change in unit of 0.1 semitone. Range: LOWER_SEMITONE_LIMIT ~ UPPER_SEMITONE_LIMIT
 * @param w frame size for analysis/synthesis. Maximum value allowed is defined by MAX_W_SIZE. w must be an even number.
 * @return error code. PS_ERROR_OK means successful, other codes indicate error. 
 * @note Upon successful initialization, the length of input/output PCM samples = w/2.
 */
int32_t pitch_shifter_init(PSContext *ct, int32_t num_channels, int32_t sample_rate, int32_t semitone_steps, int32_t w);


/**
 * @brief Configure Pitch Shifter (PS) module
 * @param ct Pointer to a PSContext object.
 * @param semitone_steps Number of steps to change in unit of 0.1 semitone. Range: LOWER_SEMITONE_LIMIT ~ UPPER_SEMITONE_LIMIT
 */
int32_t pitch_shifter_configure(PSContext *ct, int32_t semitone_steps);

/**
 * @brief Apply pitch shifting (length of input/output PCM samples = w/2)
 * @param ct Pointer to a PSContext object.
 * @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format, i.e. for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 * @param pcm_out Address of the PCM output. The PCM layout is the same as in Microsoft WAVE format, i.e. for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 *        pcm_out can be the same as pcm_in. In this case, the PCM is changed in-place.
 * @return error code. PS_ERROR_OK means successful, other codes indicate error.
 */
int32_t pitch_shifter_apply(PSContext *ct, int16_t *pcm_in, int16_t *pcm_out);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__PITCH_SHIFTER_H__
