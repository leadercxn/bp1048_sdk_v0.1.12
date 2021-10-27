/**
 *************************************************************************************
 * @file	mp2enc_api.h
 * @brief	MPEG 1,2 Layer II Encoder API interface.
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v2.2.1
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __MP2ENC_API_H__
#define __MP2ENC_API_H__

#include "typedefine.h"

#define SAMPLES_PER_FRAME 1152
#define MP2_ENCODER_OUTBUF_CAPACITY	1732	// capacity (in bytes) of output buffer for encoded bitstream. Max. 1728 bytes/frame for 384kbps@32kHz. +4 for safety guard.
#define	SBLIMIT	32
#define	SCALE_BLOCK 12


 /***************************************************************************************
  Subband utility structures
 ****************************************************************************************/
typedef struct _subband_mem {
	int32_t x[2][512];	// Q32.0
	int16_t m[16][32];	// Q1.15
	int32_t off[2];
	int32_t half[2];
} subband_mem;

/**
 * Error code of MP2 Encoder
 */
typedef enum _MP2_ENCODE_ERROR_CODE
{
	// The followings are unrecoverable errors, i.e. encoder can't continue
    MP2_ERROR_UNRECOVERABLE = -256,
	MP2_ERROR_NOT_SUPPORTED_SAMPLE_RATE,	/**< the specified sample rate is not supported */
	MP2_ERROR_NOT_SUPPORTED_BITRATE,		/**< the specified bitrate is not supported */

	// The followings are recoverable errors, i.e. encoder can continue
	MP2_ERROR_RECOVERABLE = -128,			/**< decoder can continue  */
	MP2_ERROR_NOT_ENOUGH_SAMPLES,			/**< not enough PCM samples for encoding */	
	MP2_ERROR_FRAME_BITS_BROKEN,			/**< encoded bits of a frame is not correct */
	MP2_ERROR_OUT_OF_BUFFER,				/**< not enough output buffer for encoding */

	// No Error
	MP2_ERROR_OK = 0,						/**< no error              */

}MP2_ENCODE_ERROR_CODE;


///***************************************************************************************
//psycho 0 mem struct
//****************************************************************************************/
//typedef struct psycho_0_mem_struct {
//    int32_t ath_min[SBLIMIT];	
//} psycho_0_mem;


typedef struct _MP2EncoderContext
{
	int32_t sample_rate;			/**< sample rate in Hz */
	int32_t num_channels;			/**< number of channels */
	int32_t bitrate;				/**< bitrate in kbps */
	//int32_t error_code;				/**< error code */
	
	int32_t mpeg_version;		/**< MPEG-1, MPEG-2, ... */
	int32_t mpeg_mode;			/**< stereo, joint stereo, mono, ... */
	
	int8_t mpeg_mode_ext;			/**< mode extension used when mode=JOINT STEREO */	
	int8_t jsbound;					/**< first band of joint stereo coding */
    int8_t sblimit;					/**< total number of sub bands */
	int8_t vbr;						/**< VBR flag (default=0) */
	
	int8_t bitrate_index;			/**< bitrate index */
	int8_t samplerate_index;		/**< sample rate index */	
	int8_t tablenum;				/**< selected bit allocation table index */
	int8_t padding;					/**< padding flag (default=0) */	

	// Miscellaneous Options That Nobody Ever Uses
    int8_t emphasis;				/**<  default=0, i.e. No Emphasis */
    int8_t copyright;				/**< [FALSE] */
    int8_t original;				/**< [FALSE] */
    int8_t private_bit;				/**< [0] Your very own bit in the header. */
    int8_t error_protection;		/**< CRC error protection flag (default=0) */    	
	uint8_t num_blocks_received;	/**< #blocks_received x 32 PCM samples per block */	
	int8_t dummy1;
	int8_t dummy2;

	subband_mem smem;				/**< subband analysis structure */
	//psycho_0_mem p0mem;				/**< psychoacoustic model structure */

	int16_t pcm_buffer[32*2];		/**< small PCM buffer to hold samples from FIFO */
	int32_t sb_sample[2][3][SCALE_BLOCK][SBLIMIT];	/**< subband samples, equivalent to XR in decoder */
	uint8_t sf_index[2][3][SBLIMIT];/**< scalefactor index */
	uint8_t scfsi[2][SBLIMIT];		/**< scalefactor select information */
	uint8_t bit_alloc[2][SBLIMIT];	/**< bit allocation index */
	int32_t smr[2][SBLIMIT];			/**< signal-to-mask ratio */

	BufferContext bc;					/**< buffer context */
} MP2EncoderContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/*******************************************************************************
 * MP2 encoder API functions prototype. 
 *******************************************************************************/ 
/**
 * @brief  Initialize the MP2 encoder.
 * @param[in] ct  Pointer to the MP2 encoder context
 * @param[in] num_channels Number of channels. Only 1 or 2 channels are supported.
 * @param[in] sample_rate Sample rate. Supported sample rates: 16000, 22050, 24000,	32000, 44100, 48000 Hz.
 * @param[in] bitrate Bit rate in kbps. Supported bit rates: 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 192, 224, 256, 320, 384 kbps.
 * @return initialization result. MP2_ERROR_OK means OK, other codes indicate error.
 * @note Not all combinations of sample rates & bit rates are supported according to the MPEG 1 & 2 specificaitons (ISO 11172-3 & ISO 13818-3)
 * The following table shows all the supported combinations of sample rates & bitrates ("o" means OK)
  *	---------------------------------------------------------------------
 *	| Bitrate(kbps) |                   Sample Rate (Hz)                |
 *	|--------------------------------------------------------------------
 *	|               |  16000   22050   24000  |  32000   44100   48000  |
 *	|               | 1ch 2ch 1ch 2ch 1ch 2ch | 1ch 2ch 1ch 2ch 1ch 2ch |
 *	|    8          |  o       o       o      |                         |
 *	|    16         |  o   o   o   o   o   o  |                         |
 *	|    24         |  o   o   o   o   o   o  |                         |
 *	|    32         |  o   o   o   o   o   o  |  o       o       o      |
 *	|    40         |  o   o   o   o   o   o  |                         |
 *	|    48         |  o   o   o   o   o   o  |  o       o       o      |
 *	|    56         |  o   o   o   o   o   o  |  o       o       o      |
 *	|    64         |  o   o   o   o   o   o  |  o   o   o   o   o   o  |
 *	|    80         |  o   o   o   o   o   o  |  o   o   o   o   o   o  |
 *	|    96         |      o       o       o  |  o   o   o   o   o   o  |
 *	|    112        |      o       o       o  |  o   o   o   o   o   o  |
 *	|    128        |      o       o       o  |  o   o   o   o   o   o  |
 *	|    144        |      o       o       o  |                         |
 *	|    160        |      o       o       o  |  o   o   o   o   o   o  |
 *	|    192        |                         |  o   o   o   o   o   o  |
 *	|    224        |                         |      o       o       o  |
 *	|    256        |                         |      o       o       o  |
 *	|    320        |                         |      o       o       o  |
 *	|    384        |                         |      o       o       o  |
 *	---------------------------------------------------------------------
 */
int32_t	mp2_encoder_initialize(MP2EncoderContext *ct, int32_t num_channels, int32_t sample_rate, int32_t bitrate);


/**
 * @brief  Encode an MP2 frame.
 * @param[in] ct  Pointer to the MP2 encoder context
 * @param[in] pcm One frame of PCM input data. The data layout is the same as Microsoft WAVE format, i.e. for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 * @param[out] data Data buffer to receive the encoded MP2 frame.
 * @param[out] length Length of encoded data in bytes
 * @return encoding result. MP2_ERROR_OK means OK, other codes indicate error.
 * @note no. of samples per frame = 1152 for all sample rates
 *		 For PCM input to the buffer, no. of samples required = no. of samples per frame x no. of channels.
 */
int32_t mp2_encoder_encode(MP2EncoderContext *ct, int16_t pcm[SAMPLES_PER_FRAME * 2], uint8_t* data, uint32_t *length);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__MP2ENC_API_H__
