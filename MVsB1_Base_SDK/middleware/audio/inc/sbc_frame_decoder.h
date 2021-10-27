/*************************************************************************************/
/**
 * @file	sbc_frame_decoder.h
 * @brief	SBC Frame Decoder
 *
 * @author	Zhao Ying (Alfred)
 * @version	v2.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 */
 /*************************************************************************************/

#ifndef __SBC_FRAME_DECODER_H__
#define __SBC_FRAME_DECODER_H__

#include <stdint.h>
#include "mvstdio.h"


/** SBC Frame Decoder Context */
typedef struct _SBCFrameDecoderContext
{
	int32_t sample_rate;			/**< sample rate in Hz */
	int32_t num_channels;			/**< number of channels */
	int16_t *pcm;					/**< PCM buffer*/
	int32_t pcm_length;				/**< PCM length in samples per channel in frame */	
	int32_t error_code;				/**< error code */
	uint8_t frame[3856];			// SBCContext
	uint8_t inbuf[516];				// input buffer
	uint8_t sbc_circularbuf[516];	// managed by MemHandle	
	MemHandle mh;
	BufferContext bc;
} SBCFrameDecoderContext;

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief  Initialize the SBC frame decoder.
 * @param[in] ct  Pointer to the SBC frame decoder context.
 * @return Initialization result. 0 means OK, other codes indicate error.
 */
int32_t sbc_frame_decoder_initialize(SBCFrameDecoderContext *ct);


/**
 * @brief  Decode an SBC frame.
 * @param[in] ct  Pointer to the SBC frame decoder context.
 * @param[in] data Data buffer that contains the encoded SBC frame.
 * @param[in] length Length of the encoded SBC frame in bytes, which should be no more than 513 bytes.
 * @return Decoding result. 0 means OK, other codes indicate error.
 * @note Once the SBC frame is successfully decoded, the sampling rate, number of channels, PCM buffer address, PCM length are represented
 * by sampling_rate, num_channels, pcm and pcm_length in ct structure respectively.
 * The data layout in PCM buffer for mono: M0,M1,M2,...; for stereo: L0,R0,L1,R1,L2,R2,...
 */
int32_t sbc_frame_decoder_decode(SBCFrameDecoderContext *ct, uint8_t* data, int32_t length);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif /* __SBC_FRAME_DECODER_H__ */
