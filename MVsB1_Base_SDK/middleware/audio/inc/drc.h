/**
 *************************************************************************************
 * @file	drc.h
 * @brief	DRC (Dynamic Range Compression) fixed-point implementation
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v3.1.0
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __DRC_H__
#define __DRC_H__

#include <stdint.h>


/** error code for DRC */
typedef enum _DRC_ERROR_CODE
{
    DRC_ERROR_SAMPLE_RATE_NOT_SUPPORTED = -256,
    DRC_ERROR_NUM_CHANNELS_NOT_SUPPORTED,
	DRC_ERROR_THRESHOLD_OUT_OF_RANGE,
	DRC_ERROR_RATIO_OUT_OF_RANGE,
	// No Error
	DRC_ERROR_OK = 0,					/**< no error              */
} DRC_ERROR_CODE;


/** DRC mode */
typedef enum _DRC_MODE
{
	DRC_MODE_FULLBAND,					/** full band */
	
	DRC_MODE_CF_B1,						/** 2-band only (Butterworth filters, order = 1) */
	DRC_MODE_CF_LR4,					/** 2-band only (Linkwitz-Riley filters, order = 4) */
	DRC_MODE_CF_Q4,						/** 2-band only (Q controlled filters, order = 4) */
	
	DRC_MODE_CF_B1_FULLBAND,			/** 2-band (Butterworth filters, order = 1) + full band */
	DRC_MODE_CF_LR4_FULLBAND,			/** 2-band (Linkwitz-Riley filters, order = 4) + full band  */
	DRC_MODE_CF_Q4_FULLBAND,			/** 2-band (Q controlled filters, order = 4) + full band */

} DRC_MODE;


/** DRC context */
typedef struct _DRCContext
{
	int32_t num_channels;
	int32_t sample_rate;
	int32_t fc;
	int32_t mode;
	int32_t filter_order;
	int32_t fullband_on;
	//int32_t knee;				// knee width in dB (0: hard knee; other values: soft knee)
	//int32_t maxdb;
	//int32_t ratio;			// compression ratio
	//int32_t attack_tc;		// attack time constant in millisecond
	//int32_t release_tc;		// release time constant in millisecond		
	int32_t slope[3];
	int32_t alpha_attack[3];
	int32_t alpha_release[3];
	int32_t threshold[3];
	int32_t state[3];
	int32_t bl[2];
	int32_t bh[2];
	int32_t a[2];
	int32_t ah[2];
	int32_t d[2][14];			// filter delays
} DRCContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief Initialize Dynamic Range Compression (DRC) module
 * @param ct Pointer to a DRCContext object.
 * @param num_channels Number of channels. 
 * @param sample_rate Sample rate.
 * @param fc Crossover frequency in Hz to seperate the full band into 2 subbands. Set 0 to apply DRC for the full band only. Set other values (20 ~ 20000) to apply DRC for the 2 subbands seperatedly.
 * @param mode The mode as listed in DRC_MODE.
 * @param q[2] Q values in Q6.10 format (e.g 717 stands for 0.70) of the Q controlled crossover filters if selected. q[0] is for low-pass filter and q[1] is for high-pass filter when mode = DRC_MODE_CF_Q4 or DRC_MODE_CF_Q4_FULLBAND, otherwise the Q parameters are ignored and can be set NULL.
 * @param threshold[3] A compressor reduces the level of an audio signal if its amplitude exceeds a certain threshold. Set the threshold in step of 0.01dB. Allowable range: -9000 ~ 0 to cover -90.00dB ~ 0.00dB. For example, -2550 stands for -25.50dB threshold. threshold[0] is for lower band, threshold[1] is for upper band and threshold[2] is for full band.
 * @param ratio[3] The amount of gain reduction is determined by ratio: a ratio of 4:1 means that if input level is 4 dB over the threshold, the output signal level will be 1 dB over the threshold. The gain (level) has been reduced by 3 dB. ratio[0] is for lower band, ratio[1] is for upper band, ratio[2] is for full band.
 * @param attack_tc[3] Amount of time (in millisecond) it will take for the gain to decrease to a set level. attack_tc[0] is for lower band, attack_tc[1] is for upper band and attack_tc[2] is for full band.
 * @param release_tc[3] Amount of time (in millisecond) it will take for the gain to restore to 0dB. release_tc[0] is for lower band, release_tc[1] is for upper band and release_tc[2] is for full band.
 * @return error code. DRC_ERROR_OK means successful, other codes indicate error.
 */
int32_t drc_init(DRCContext *ct, int32_t num_channels, int32_t sample_rate, 
				 int32_t fc, DRC_MODE mode, uint16_t q[2],
				 int32_t threshold[3], int32_t ratio[3], int32_t attack_tc[3], int32_t release_tc[3]);


/**
 * @brief Apply Dynamic Range Compression (DRC) to a frame of PCM data.
 * @param ct Pointer to a DRCContext object.
 * @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format, i.e. for mono: M0,M1,M2,... ("mono-S"); for stereo: L0,R0,L1,R1,L2,R2,... ("stereo-R+L")
 * @param pcm_out Address of the PCM output. The PCM layout is the same as in Microsoft WAVE format, i.e. for mono: M0,M1,M2,... ("mono-S"); for stereo: L0,R0,L1,R1,L2,R2,... ("stereo-R+L")
 *        pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
 * @param n Number of PCM samples to process.
 * @param pregain1 Pregain applied to full-band or lower-band in case of 2-band DRC before DRC processing. Q4.12 format to represent value in range [0.000244140625, 8) to cover -72dB ~ 18dB. For example, 2052 represents x0.501 (-6dB), 2900 represents x0.708 (-3dB), 4096 represents x1.0 (0dB),  5786 represents x1.413 (+3dB), 16306 represents x3.981 (+12dB)
 * @param pregain2 Pregain applied to upper-band in case of 2-band DRC before DRC processing. For full-band DRC, this parameter is ignored. Q4.12 format to represent value in range [0.000244140625, 8) to cover -72dB ~ 18dB. For example, 2052 represents x0.501 (-6dB), 2900 represents x0.708 (-3dB), 4096 represents x1.0 (0dB),  5786 represents x1.413 (+3dB), 16306 represents x3.981 (+12dB)
 * @return error code. DRC_ERROR_OK means successful, other codes indicate error.
 */
int32_t drc_apply(DRCContext *ct, int16_t *pcm_in, int16_t *pcm_out, int32_t n, int32_t pregain1, int32_t pregain2);


/**
 * @brief Apply Dynamic Range Compression (DRC) to a frame of PCM data (24-bit PCM in & out).
 * @param ct Pointer to a DRCContext object.
 * @param pcm_in Address of the PCM input. The PCM layout must be the same as in Microsoft WAVE format, i.e. for mono: M0,M1,M2,... ("mono-S"); for stereo: L0,R0,L1,R1,L2,R2,... ("stereo-R+L")
 * @param pcm_out Address of the PCM output. The PCM layout is the same as in Microsoft WAVE format, i.e. for mono: M0,M1,M2,... ("mono-S"); for stereo: L0,R0,L1,R1,L2,R2,... ("stereo-R+L")
 *        pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
 * @param n Number of PCM samples to process.
 * @param pregain1 Pregain applied to full-band or lower-band in case of 2-band DRC before DRC processing. Q4.12 format to represent value in range [0.000244140625, 8) to cover -72dB ~ 18dB. For example, 2052 represents x0.501 (-6dB), 2900 represents x0.708 (-3dB), 4096 represents x1.0 (0dB),  5786 represents x1.413 (+3dB), 16306 represents x3.981 (+12dB)
 * @param pregain2 Pregain applied to upper-band in case of 2-band DRC before DRC processing. For full-band DRC, this parameter is ignored. Q4.12 format to represent value in range [0.000244140625, 8) to cover -72dB ~ 18dB. For example, 2052 represents x0.501 (-6dB), 2900 represents x0.708 (-3dB), 4096 represents x1.0 (0dB),  5786 represents x1.413 (+3dB), 16306 represents x3.981 (+12dB)
 * @return error code. DRC_ERROR_OK means successful, other codes indicate error.
 */
int32_t drc_apply24(DRCContext *ct, int32_t *pcm_in, int32_t *pcm_out, int32_t n, int32_t pregain1, int32_t pregain2);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__DRC_H__
