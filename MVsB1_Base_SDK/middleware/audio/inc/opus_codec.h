/*************************************************************************************/
/**
 * @file	opus_codec.h
 * @brief	Opus codec (encoder & decoder) API for mono signals
 *
 * @author	Zhao Ying (Alfred)
 * @version	V2.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 */
 /*************************************************************************************/

#ifndef __OPUS_CODEC_API_H__
#define __OPUS_CODEC_API_H__

#include <stdint.h>
#include "typedefine.h"
#include "song_info.h"


#define OPUS_INPUT_BUFFER_CAPACITY 2560	// capacity of input buffer in bytes
#define OPUS_MAX_CH 1					// maximum number of channels

#if OPUS_MAX_CH == 1
#define OPUS_ENCODER_CORE_SIZE 38556	// for max. 1 channel
#define OPUS_DECODER_CORE_SIZE 17900	// for max. 1 channel (17827 is found other than reported 17776)
#else
#define OPUS_DECODER_CORE_SIZE 26600	// for max. 2 channels (26547 is found other than reported 26496)
#endif
#define OPUS_MAX_FRAME_SIZE 960			// PCM samples. =20ms@48000Hz
#define OPUS_MAX_PACKET_SIZE 1276		// #encoded bytes in packet. ~510kbps for 20ms frame

/**
 * Error code of Opus Codec (encoder & decoder)
 */
//typedef enum _OPUS_CODEC_ERROR_CODE
//{
//	// The followings are unrecoverable errors
//	OPUS_ERROR_UNRECOVERABLE = -256,
//	OPUS_ERROR_NOT_SUPPORTED_SAMPLE_RATE,	/**< the specified sample rate is not supported */
//	OPUS_ERROR_NOT_SUPPORTED_CHANNELS,		/**< the specified number of channels is not supported */
//	OPUS_ERROR_NOT_SUPPORTED_BITRATE,		/**< the specified bitrate is not supported */
//	OPUS_ERROR_NOT_SUPPORTED_CODING_MODE,	/**< the specified coding mode is not supported */	
//	
//	OPUS_ERROR_INTERNAL = -192,				/**< internal error starts here */
//
//	// No Error
//	OPUS_ERROR_OK = 0,						/**< no error              */
//
//}OPUS_CODEC_ERROR_CODE;

typedef enum _OPUS_CODEC_ERROR_CODE
{
	// The followings are unrecoverable errors
	OPUS_ERROR_UNRECOVERABLE = -256,
	OPUS_ERROR_NOT_SUPPORTED_SAMPLE_RATE,			/**< the specified sample rate is not supported */
	OPUS_ERROR_NOT_SUPPORTED_CHANNELS,				/**< the specified number of channels is not supported */
	OPUS_ERROR_NOT_SUPPORTED_BITRATE,				/**< the specified bitrate is not supported */
	OPUS_ERROR_NOT_SUPPORTED_CODING_MODE,			/**< the specified coding mode is not supported */
	OPUS_ERROR_NOT_SUPPORTED_CHANNEL_MAPPING,		/**< the channel mappling family is not supported */
	OPUS_ERROR_CAPTURE_PATTERN_NOT_FOUND,			/**< the capture pattern "OggS" is not found */
	OPUS_ERROR_ILLEGAL_STREAM_STRUCTURE_VERSION,	/**< the stream structure version is illegal */
	OPUS_ERROR_OPUS_HEAD_NOT_FOUND,					/**< the Opus Head is not found */
	OPUS_ERROR_ILLEGAL_OPUS_VERSION,				/**< the Opus version is illegal */
	OPUS_ERROR_OPUS_TAGS_NOT_FOUND,					/**< the Opus Tags is not found */
	OPUS_ERROR_END_OF_STREAM,						/**< the end of stream/file is reached */

	OPUS_ERROR_INTERNAL = -192,						/**< internal error starts here */

	// The followings are recoverable errors, i.e. decoder can continue
	OPUS_ERROR_RECOVERABLE = -128,
	OPUS_ERROR_SEEKING_NOT_SUPPORTED,				/**< seeking is not supported for the time being */

	// No Error
	OPUS_ERROR_OK = 0,								/**< no error              */

}OPUS_CODEC_ERROR_CODE;

/** Opus coding modes */
typedef enum {
	CODING_MODE_VOIP = 0,				/**< Best for most VoIP/videoconference applications where listening quality and intelligibility matter most */
	CODING_MODE_AUDIO,					/**< Best for broadcast/high-fidelity application where the decoded audio should be as close as possible to the input */
	CODING_MODE_RESTRICTED_LOWDELAY,	/**< Only use when lowest-achievable latency is what matters most. Voice-optimized modes cannot be used. */	
} OPUS_CODING_MODE;

/** Opus Encoder Frame Context */
typedef struct _OpusEncoderFrameContext
{
	int32_t sample_rate;			/**< sample rate in Hz */
	int32_t num_channels;			/**< number of channels */
	int32_t bitrate;				/**< bitrate in kbps */	
	uint8_t opus_encoder[OPUS_ENCODER_CORE_SIZE];
} OpusEncoderFrameContext;

/** Opus Decoder Frame Context */
typedef struct _OpusDecoderFrameContext
{
	int32_t sample_rate;			/**< sample rate in Hz */
	int32_t num_channels;			/**< number of channels */
	uint8_t opus_decoder[OPUS_DECODER_CORE_SIZE];
} OpusDecoderFrameContext;

/** Opus Decoder Context */
typedef struct _OpusDecoderContext
{
	int32_t sample_rate;					/**< sample rate in Hz */
	int32_t num_channels;					/**< number of channels */
	int32_t input_sample_rate;				/**< original input sample rate */
	int32_t npackets_in_page;				/**< number of OPUS packets in page*/
	int32_t ipacket;						/**< current packet in process */
	int32_t packet_length_offset;			/**< current packet length offset */
	int32_t packet_length_nbytes;			/**< number of bytes used in packet length array */
	uint8_t packet_length_array[256];		/**< packet length array */
	uint8_t opus_decoder[OPUS_DECODER_CORE_SIZE];
	int16_t pcm[OPUS_MAX_FRAME_SIZE*OPUS_MAX_CH];
	
	int32_t error_code;							/**< Audio decoder return error code */
	uint8_t inbuf[OPUS_INPUT_BUFFER_CAPACITY];	/**< Input buffer */
	BufferContext bc;							/**< Buffer context pointer */
	SongInfo      song_info;					/**< Song information context pointer */
} OpusDecoderContext;

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


// -------------------- Opus Frame Encoder --------------------
/**
 * @brief  Initialize the Opus encoder.
 * @param[in] ct  Pointer to the Opus encoder frame context
 * @param[in] num_channels Number of channels. Only 1 channel is supported.
 * @param[in] sample_rate Sample rate. Supported sample rates: 8000, 12000, 16000, 24000, 48000 Hz.
 * @param[in] bitrate Bit rate in bps(bits per second). Supported bit rates: 500 ~ 512000 bps
 * @param[in] coding_mode Coding mode. Select one from OPUS_CODING_MODE.
 * @return initialization result. OPUS_ERROR_OK means OK, other codes indicate error.
 */
int32_t	opus_encoder_frame_initialize(OpusEncoderFrameContext *ct, int32_t num_channels, int32_t sample_rate, int32_t bitrate, OPUS_CODING_MODE coding_mode);

/**
 * @brief  Encode an Opus frame.
 * @param[in] ct  Pointer to the Opus encoder frame context
 * @param[in] pcm One frame of PCM input data. The data layout is the same as Microsoft WAVE format, i.e. for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 * @param[in] frame_size Number of samples per channel in the frame. Range: (2.5ms ~ 20ms) * sample_rate. For example at 48kHz, the frame size can be set between 120 (=2.5*48)  and 960 (=20*48) samples. This value can vary between frames.
 * @param[out] data Data buffer to receive the encoded Opus frame.
 * @param[out] length Length of the encoded Opus frame in bytes. This value will be no more than OPUS_MAX_PACKET_SIZE.
 * @param[in] bitrate Bit rate in kbps. Supported bit rates: 6 ~ 510 kbps. This value can vary between frames.
 * @return encoding result. OPUS_ERROR_OK means OK, other codes indicate error.
 * @note Usually the larger frame_size is, the better the encoding quality. Select 20ms@48kHz==960 samples as your first choice unless you expect lower delay in certain applications.
 */
int32_t opus_encoder_frame_encode(OpusEncoderFrameContext *ct, int16_t *pcm, int32_t frame_size, uint8_t* data, uint32_t *length, int32_t bitrate);


// -------------------- OPUS Frame Decoder --------------------
/**
 * @brief  Initialize the Opus decoder.
 * @param[in] ct  Pointer to the Opus decoder frame context
 * @param[in] num_channels Number of channels. Only 1 channel is supported.
 * @param[in] sample_rate Sample rate. Supported sample rates: 8000, 12000, 16000, 24000, 48000 Hz. 
 * @return initialization result. OPUS_ERROR_OK means OK, other codes indicate error.
 */
int32_t	opus_decoder_frame_initialize(OpusDecoderFrameContext *ct, int32_t num_channels, int32_t sample_rate);

/**
 * @brief  Decode an Opus frame.
 * @param[in] ct  Pointer to the Opus decoder frame context
 * @param[in] data Data buffer to receive the encoded Opus frame.
 * @param[in] length Length of the encoded Opus frame in bytes. This value will be no more than OPUS_MAX_PACKET_SIZE.
 * @param[out] pcm One frame of PCM output data. The data layout is the same as Microsoft WAVE format, i.e. for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 * @param[out] frame_size Number of samples per channel in the frame. Range: (2.5ms ~ 20ms) * sample_rate. For example at 48kHz, the frame size is between 120 (=2.5*48)  and 960 (=20*48) samples.
 * @return encoding result. OPUS_ERROR_OK means OK, other codes indicate error.
 * @note The decoded frame_size can vary between frames. The minimum value is 2.5ms x sample_rate, the maximum value is 20ms x sample_rate.
 */
int32_t opus_decoder_frame_decode(OpusDecoderFrameContext *ct, uint8_t* data, int32_t length, int16_t *pcm, int32_t *frame_size);



// -------------------- OPUS Stream Decoder --------------------
/**
 * @brief  Initialize the Opus decoder.
 * @param  ct           Pointer to the structure of decoder context
 * @param  io_handle    I/O handle. This is either a file handle or memory handle depending on io_type
 * @param  io_type      I/O type. The value should be of @code IOType (IO_TYPE_FILE or IO_TYPE_MEMORY£©
 * @return Initialization result equal to either RT_FAILURE or RT_SUCCESS.
 */
int32_t opus_decoder_initialize(OpusDecoderContext *ct, void *io_handle, int32_t io_type);

/**
 * @brief  Decode an Opus frame from stream
 * @param  ct           Pointer to the structure of decoder context
 * @return Decoding result equal to either RT_FAILURE or RT_SUCCESS.
 */
int32_t opus_decoder_decode(OpusDecoderContext *ct);

/**
 * @brief  Check whether the decoder can go on
 * @param  ct           Pointer to the structure of decoder context
 * @return Check result equal to either RT_NO or RT_YES
 */
int32_t opus_decoder_can_continue(OpusDecoderContext *ct);

/**
 * @brief  Change the file handle to a new position according to the specified time.
 * @param  ct           Pointer to the structure of decoder context
 * @param  seek_time    the specified time in millisecond(ms)
 * @return Seek result equal to either RT_FAILURE or RT_SUCCESS.
*/
int32_t opus_decoder_seek(OpusDecoderContext *ct, uint32_t seek_time);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif /* __OPUS_CODEC_API_H__ */
