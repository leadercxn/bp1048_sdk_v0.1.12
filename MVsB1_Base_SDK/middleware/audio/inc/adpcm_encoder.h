/**
 **************************************************************************************
 * @file    adpcm_encoder.h
 * @brief   IMA ADPCM Encoder
 *
 * @author  Aissen Li, ZHAO Ying (Alfred)
 * @version V2.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __ADPCM_ENCODER_H__
#define __ADPCM_ENCODER_H__

#include <stdint.h>

 /**
  * Error code of ADPCM Encoder
  */
typedef enum _ADPCM_ENCODE_ERROR_CODE
{
	// The followings are unrecoverable errors, i.e. encoder can't continue
	ADPCM_ENC_ERROR_UNRECOVERABLE = -256,
	ADPCM_ENC_ERROR_NOT_SUPPORTED_SAMPLE_RATE,	/**< the specified sample rate is not supported */
	ADPCM_ENC_ERROR_NOT_SUPPORTED_BLOCKSIZE,	/**< the specified block size is not supported */
	ADPCM_ENC_ERROR_NOT_SUPPORTED_CHANNELS,		/**< the specified number of channels is not supported */

	// The followings are recoverable errors, i.e. encoder can continue
	ADPCM_ENC_ERROR_RECOVERABLE = -128,

	// No Error
	ADPCM_ENC_ERROR_OK = 0,						/**< no error              */

}ADPCM_ENCODE_ERROR_CODE;


typedef struct _ADPCMHeader
{
    uint8_t     chunk_id[4];		    /**<  0: 'R','I','F','F'	                                                    */
    uint32_t    chunk_size;			    /**<  4: equal to (filesize-8)                                                  */
    uint8_t     wave_id[4]; 		    /**<  8: 'W','A','V','E'                                                        */
    uint8_t     fmt_chunk_id[4];	    /**< 12: 'f','m','t',' '	                                                    */
    uint32_t    fmt_chunk_size;		    /**< 16: 16 for PCM, 20 for IMA-ADPCM                                           */
    uint16_t    format_tag;		        /**< 20: 0x0001 for PCM, 0x0011 for IMA-ADPCM & DVI-ADPCM   			        */
    uint16_t    num_channels;		    /**< 22: number of channels                                                     */
    uint32_t    sample_rate; 		    /**< 24: sample rate in Hz                                                      */
    uint32_t    byte_rate;			    /**< 28: average byterate, equal to (SampleRate*NumChannels*(BitsPerSample/8))  */
    uint16_t    block_align;		    /**< 32: equal to (NumChannels * BitsPerSample/8) 			                    */
    uint16_t    bits_per_sample;	    /**< 34: bits per samples, 16 for PCM, 4 for IMA-ADPCM                          */
    uint16_t    cb_size;                /**< 36: size of extension                                                      */
    uint16_t    samples_per_block;      /**< 38: samples per block                                                      */
    uint8_t     fact_chunk_id[4];       /**< 40: 'f','a','c','t'                                                        */
    uint32_t    fact_chunk_size;        /**< 44: fact chunk size                                                        */
    uint32_t    num_encoded_samples;    /**< 48: number of encoded samples                                              */
    uint8_t     data_chunk_id[4];		/**< 52: 'd','a','t','a'                                                        */
    uint32_t    data_chunk_size;		/**< 56: equal to (filesize-60)                                                 */
}ADPCMHeader;

/**
 * @brief ADPCM Encoder Context
 */
typedef struct _ADPCMEncoderContext
{
    ADPCMHeader  header;            /**< IMA-ADPCM header                                           */

    uint32_t sampling_rate;			/**< Sampling rate                                              */
    uint16_t num_channels;			/**< Number of channels                                         */
    uint16_t samples_per_block;		/**< Samples per block_size for each channel                    */
    uint16_t block_size;			/**< Block size, length of encoded data to be output in bytes   */
    uint16_t block_encoded_size;	/**< Encoded block size                                         */
    int32_t  total_encoded_blocks;	/**< Total encoded blocks				                        */
    int32_t  pre_sample[2];			/**< Predict sample value                                       */

	int8_t   step_table_index[2];	/**< Step size table index                                      */
	int8_t   reserved[2];           /**< Reserved for word align                                    */

    //uint8_t* outbuf;                /**< Encoder output buffer for one block                        */

}ADPCMEncoderContext;


#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief  Initialize ADPCM encoder
 * @param[in] adpcm ADPCM encoder struct pointer
 * @param[in] num_channels number of channels
 * @param[in] sampling_rate sampling rate
 * @param[in] block_size size of encoded output block in bytes. Choose from 256/512/1024 or set 0 to let encoder choose for you according to the sampling rate. This value affects the size of input PCM frame, i.e. adpcm->samples_per_block.
 * @return initialization result. ADPCM_ENC_ERROR_OK means OK, other codes indicate error.
 */
int32_t adpcm_encoder_initialize(ADPCMEncoderContext* adpcm, int32_t num_channels, int32_t sampling_rate, int32_t block_size);


/**
 * @brief  Encode PCM data using ADPCM algorithm
 * @param[in]  adpcm         ADPCM encoder struct pointer
 * @param[in]  pcm           a frame of PCM data (adpcm->samples_per_block * adpcm->num_channels). The data layout is the same as Microsoft WAVE format, i.e. L,R,L,R,... for stereo ; M,M,M,... for mono.
 * @param[out] data			 Data buffer to receive the encoded ADPCM frame.
 * @param[out] length        length of encoded data in bytes. This is equal to block_size specified in adpcm_encoder_initialize().
 * @return encoding result. ADPCM_ENC_ERROR_OK means OK, other codes indicate error.
 * @note adpcm->samples_per_block is valid only after successfully calling adpcm_encoder_initialize().
 */
int32_t adpcm_encoder_encode(ADPCMEncoderContext* adpcm, int16_t* pcm, uint8_t* data, uint32_t* length);


/**
 * @brief  Get the pointer to the ADPCMHeader structure
 * @param[in]   adpcm   ADPCM encoder struct pointer
 * @return pointer to the ADPCMHeader structure
 * @note the returned ADPCMHeader structure can be used directly to update the header of ADPCM wave file.
 */
ADPCMHeader * adpcm_encoder_get_file_header(ADPCMEncoderContext* adpcm);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__ADPCM_ENCODER_H__
