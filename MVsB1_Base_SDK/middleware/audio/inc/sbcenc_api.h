/**
 *************************************************************************************
 * @file	sbcenc_api.h
 * @brief	SBC Encoder API interface.
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.1.1
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __SBCENC_API_H__
#define __SBCENC_API_H__

#include <stdint.h>

// for mSBC mode (blocks = 15), MAX_SBC_FRAME_LEN = 57 bytes always.
#define MAX_SBC_FRAME_LEN 532 // 4+(4*8*2)/8+ceil(1*8+16*(32*8)/8), ref to "bitpool" & "frame_length" concepts in A2DP reference.

/**
 * Error code of SBC Encoder
 */
typedef enum _SBC_ENC_ERROR_CODE
{
	// The followings are unrecoverable errors, i.e. encoder can't continue
    SBC_ENC_ERROR_UNRECOVERABLE = -256,
	SBC_ENC_ERROR_NOT_SUPPORTED_SAMPLE_RATE,	/**< the specified sample rate is not supported */
	SBC_ENC_ERROR_NOT_SUPPORTED_CHANNELS,		/**< the specified number of channels is not supported */
	SBC_ENC_ERROR_NOT_SUPPORTED_BLOCKS,			/**< the specified number of blocks is not supported */
	SBC_ENC_ERROR_INCORRECT_MSBC_SETTINGS,		/**< incorrect settings are used for mSBC mode */

	// The followings are recoverable errors, i.e. encoder can continue
	SBC_ENC_ERROR_RECOVERABLE = -128,
	SBC_ENC_ERROR_BAD,							/**< not enough PCM samples for encoding */	

	// No Error
	SBC_ENC_ERROR_OK = 0,						/**< no error              */

}SBC_ENC_ERROR_CODE;


/**
 * SBC encoder quality selection
 */
typedef enum _SBC_ENC_QUALITY
{
	SBC_ENC_QUALITY_HIGH,
	SBC_ENC_QUALITY_MIDDLE,
}SBC_ENC_QUALITY;


/** SBC Encoder Context */
typedef struct _SBCEncoderContext
{
	uint8_t priv_instance[4096]; // at least 4035 for encoder only, 5527 bytes for codec
	void *priv;
	void *priv_alloc_base;

	uint32_t flags;	
	uint8_t frequency;
	uint8_t blocks;
	uint8_t subbands;
	uint8_t mode;
	uint8_t allocation;
	uint8_t bitpool;
	uint8_t endian;

} SBCEncoderContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/*******************************************************************************
 * SBC encoder API functions prototype. 
 *******************************************************************************/ 
/**
 * @brief  Initialize the SBC encoder.
 * @param[in] ct  pointer to the SBC encoder context
 * @param[in] num_channels number of channels
 * @param[in] sample_rate sample rate
 * @param[in] blocks number of blocks per frame. Select from 4, 8, 12, 15, 16 only. If 15 is selected, mSBC encoding mode is implied.
 * @param[in] quality encoding quality. Select either SBC_ENC_QUALITY_HIGH (high bitrate) or SBC_ENC_QUALITY_MIDDLE (middle bitrate). This quality setting is ignored if mSBC encoding mode is used (blocks=15).
 * @param[out] samples_per_frame number of samples per frame per channel.
 *             For example if the returned samples per frame is 128, then for mono input, the required samples from input are 128 samples/frame; for 2 channels, the required samples from input are 256 samples(=128 * 2)/frame.
 * @return initialization result. SBC_ENC_ERROR_OK means OK, other codes indicate error.
 */ 
int32_t	sbc_encoder_initialize(SBCEncoderContext *ct, int32_t num_channels, int32_t sample_rate, int32_t blocks, SBC_ENC_QUALITY quality, int32_t *samples_per_frame);


/**
 * @brief  Encode an SBC frame.
 * @param[in] ct  pointer to the SBC encoder context
 * @param[in] pcm one frame of PCM data. The data layout is the same as Microsoft WAVE format. i.e. for mono, m0, m1, m2, ...; for stereo, l0, r0, l1, r1, ...
 *            The length of pcm samples should be (samples_per_frame*num_channels) as given in sbc_encoder_initialize().
 * @param[in] data  data buffer to receive the encoded SBC frame.
 * @param[out] length length of the encoded SBC frame in bytes. This value shall not exceed MAX_SBC_FRAME_LEN.
 * @return encoding result. SBC_ENC_ERROR_OK means OK, other codes indicate error.
 */
int32_t sbc_encoder_encode(SBCEncoderContext *ct, int16_t *pcm, uint8_t *data, uint32_t *length);

//int32_t sbc_encoder_finish(SBCEncoderContext *ct);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__SBCENC_API_H__
