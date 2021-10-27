/**
 *************************************************************************************
 * @file	eq.h
 * @brief	Parametric EQ
 *
 * @author	ZHAO Ying (Alfred), Aissen Li
 * @version	v8.0.0
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __EQ_H__
#define __EQ_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include <stdint.h>


#define EQ_BUFFER_NUM_SAMPLES   (128)
#define MAX_EQ_FILTER_COUNT     (10 )


/**
 * @brief EQ filter type set
 */
typedef enum _EQ_FILTER_TYPE_SET
{
    EQ_FILTER_TYPE_UNKNOWN = -1,
    EQ_FILTER_TYPE_PEAKING,
    EQ_FILTER_TYPE_LOW_SHELF,
    EQ_FILTER_TYPE_HIGH_SHELF,
    EQ_FILTER_TYPE_LOW_PASS,
    EQ_FILTER_TYPE_HIGH_PASS,
    EQ_FILTER_TYPE_BAND_PASS,
    EQ_FILTER_TYPE_NOTCH,
} EQ_FILTER_TYPE_SET;

/**
 * @brief EQ filter parameters
 */
typedef struct _EQFilterParams
{
	int16_t		type;           /**< filter type, @see EQ_FILTER_TYPE_SET                               */
	uint16_t	f0;             /**< center frequency (peak) or mid-point frequency (shelf)             */
	int16_t		Q;              /**< quality factor (peak) or slope (shelf), format: Q6.10              */	
	int16_t		gain;           /**< Gain in dB, format: Q8.8. Setting gain higher than +18dB is strongly discouraged due to higher risk of overflow in calculation for large input signals */
} EQFilterParams;

/**
 * @brief EQ filter coefficients
 */
typedef struct _EQFilterCoefs
{
	union {
		int32_t b0;
		float b0f;
	};
	union {
		int32_t b1;
		float b1f;
	};
	union {
		int32_t b2;
		float b2f;
	};
	union {
		int32_t a1;
		float a1f;
	};
	union {
		int32_t a2;
		float a2f;
	};
} EQFilterCoefs;


typedef struct _EQContext
{	
	int16_t num_channels;
	int16_t use_float;
	int32_t pregain;
	int32_t filter_count;
	EQFilterCoefs filter_coef[MAX_EQ_FILTER_COUNT];
	union {
		int32_t filter_delay[MAX_EQ_FILTER_COUNT][2][4];
		float filter_delay_f[MAX_EQ_FILTER_COUNT][2][4];
	};	    
	union {
		int32_t pcm_buf[EQ_BUFFER_NUM_SAMPLES * 2];
		float pcm_buf_f[EQ_BUFFER_NUM_SAMPLES * 2];
	};	
}EQContext;


/**
 * @brief  Initialize parametric EQ module
 * @param  eq pointer to EQ context
 * @param  sample_rate sample rate.
 * @param  filter_params EQ filter parameters.
 * @param  filter_count filter count. Value range: 0 ~ MAX_EQ_FILTER_COUNT
 * @param  pregain EQ pregain in dB with Q8.8 format. e.g. 0 for 0dB, -768 for -3dB, 640 for 2.5dB. Range:-96.00 ~ +90.00dB. Pregain higher than +18dB is not recommended.
 * @param  num_channels     Number of channels. Only 1 or 2 is supported.
 * @return None
 */
void eq_init(EQContext* eq, uint16_t sample_rate, EQFilterParams* filter_params, uint32_t filter_count, int32_t pregain, int32_t num_channels);

/**
 * @brief  Initialize parametric EQ module (floating-point calculation)
 * @param  eq pointer to EQ context
 * @param  sample_rate sample rate.
 * @param  filter_params EQ filter parameters.
 * @param  filter_count filter count. Value range: 0 ~ MAX_EQ_FILTER_COUNT
 * @param  pregain EQ pregain in dB with Q8.8 format. e.g. 0 for 0dB, -768 for -3dB, 640 for 2.5dB. Range:-96.00 ~ +90.00dB. Pregain higher than +18dB is not recommended.
 * @param  num_channels     Number of channels. Only 1 or 2 is supported.
 * @return None
 */
void eq_init_float(EQContext* eq, uint16_t sample_rate, EQFilterParams* filter_params, uint32_t filter_count, int32_t pregain, int32_t num_channels);

/**
 * @brief  Configure EQ filter parameters
 * @param  eq pointer to EQ context
 * @param  sample_rate sample rate.
 * @param  filter_params EQ filter parameters
 * @param  filter_count filter count. Value range: 0 ~ MAX_EQ_FILTER_COUNT
 * @return NONE
 */
void eq_configure_filters(EQContext* eq, uint16_t sample_rate, EQFilterParams* filter_params, uint32_t filter_count);

/**
 * @brief  Set EQ pregain
 * @param  eq pointer to EQ context
 * @param  pregain EQ pregain in dB with Q8.8 format. e.g. 0 for 0dB, -768 for -3dB, 640 for 2.5dB. Range:-96.00 ~ +90.00dB. Pregain higher than +18dB is not recommended.
 * @return NONE
 */
void eq_configure_pregain(EQContext* eq, int32_t pregain);

/**
 * @brief  Apply EQ effect
 * @param  eq pointer to EQ context
 * @param  pcm_in Address of the PCM input buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
 * @param  pcm_out Address of the PCM output buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
 *         pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
 * @param  n  Number of PCM samples to process.
 * @return NONE
 */
void eq_apply(EQContext* eq, int16_t *pcm_in, int16_t *pcm_out, int32_t n);

/**
* @brief  Apply EQ effect (24-bit PCM in & out)
* @param  eq pointer to EQ context
* @param  pcm_in Address of the PCM input buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
* @param  pcm_out Address of the PCM output buffer. The PCM layout for mono is like "M0,M1,M2,..." and for stereo "L0,R0,L1,R1,L2,R2,...".
*         pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
* @param  n  Number of PCM samples to process.
* @return NONE
*/
void eq_apply24(EQContext* eq, int32_t *pcm_in, int32_t *pcm_out, int32_t n);

/**
 * @brief  Clear EQ delay buffer
 * @param  eq pointer to EQ context
 * @return None
 */
void eq_clear_delay_buffer(EQContext* eq);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__EQ_H__
