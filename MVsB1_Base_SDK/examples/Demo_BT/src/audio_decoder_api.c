/**
 *************************************************************************************
 * @file    audio_decoder_api.c
 * @brief   Audio decoder API interface.
 *
 * @author  Alfred Zhao, Aissen Li, Lean Xiong
 * @version @see AUDIO_DECODER_LIBRARY_VERSION
 * 
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#include <string.h>
#include "audio_decoder_api.h"
#include "audio_decoder_in_use.h" // User defined configuration file for audio decoders to be used.


#ifdef  USE_MP3_DECODER
#define MP3_INPUT_BUFFER_CAPACITY 2560
#define SIZEOF_MPXContext 16132
int32_t mp3_decoder_initialize(void);
int32_t mp3_decoder_decode(void);
int32_t mp3_decoder_can_continue(void);
int32_t mp3_decoder_seek(uint32_t seek_time);
int32_t mp3_decoder_get_mpeg_version(void);
int32_t mp3_decoder_get_id3_version(void);
#endif

#ifdef  USE_WMA_DECODER
#define WMA_INPUT_BUFFER_CAPACITY 2560
#define SIZEOF_WMAContext 25532
int32_t wma_decoder_initialize(void);
int32_t wma_decoder_decode(void);
int32_t wma_decoder_can_continue(void);
int32_t wma_decoder_seek(uint32_t seek_time);
#endif

#ifdef  USE_SBC_DECODER
#define SBC_INPUT_BUFFER_CAPACITY 2560
#define SIZEOF_SBCContext 3856
int32_t sbc_decoder_initialize(void);
int32_t sbc_decoder_decode(void);
int32_t sbc_decoder_can_continue(void);
int32_t sbc_decoder_seek(uint32_t seek_time);
#endif

#ifdef  USE_WAV_DECODER
#define WAV_INPUT_BUFFER_CAPACITY 2560
#define SIZEOF_WAVContext 8224
int32_t wav_decoder_initialize(void);
int32_t wav_decoder_decode(void);
int32_t wav_decoder_can_continue(void);
int32_t wav_decoder_seek(uint32_t seek_time);
#endif

#ifdef  USE_FLAC_DECODER
#define FLAC_INPUT_BUFFER_CAPACITY 2048
#define SIZEOF_FLACContext 20088
int32_t flac_decoder_initialize(void);
int32_t flac_decoder_decode(void);
int32_t flac_decoder_can_continue(void);
int32_t flac_decoder_seek(uint32_t seek_time);
#endif

#ifdef  USE_AAC_DECODER
#define AAC_INPUT_BUFFER_CAPACITY 2560
#define SIZEOF_AACContext 25456
int32_t aac_decoder_initialize(void);
int32_t aac_decoder_decode(void);
int32_t aac_decoder_can_continue(void);
int32_t aac_decoder_seek(uint32_t seek_time);
#endif

#ifdef  USE_AIF_DECODER
#define AIF_INPUT_BUFFER_CAPACITY 2560
#define SIZEOF_AIFContext 8232
int32_t aif_decoder_initialize(void);
int32_t aif_decoder_decode(void);
int32_t aif_decoder_can_continue(void);
int32_t aif_decoder_seek(uint32_t seek_time);
#endif

#ifdef  USE_AMR_DECODER
#define AMR_INPUT_BUFFER_CAPACITY 2560
#define SIZEOF_AmrnbDecoderContext 3960
int32_t amrnb_decoder_initialize(void);
int32_t amrnb_decoder_decode(void);
int32_t amrnb_decoder_can_continue(void);
int32_t amrnb_decoder_seek(uint32_t seek_time);
#endif

#ifdef  USE_APE_DECODER
#define APE_INPUT_BUFFER_CAPACITY 2560
#define SIZEOF_APEContext 22340
int32_t ape_decoder_initialize(void);
int32_t ape_decoder_decode(void);
int32_t ape_decoder_can_continue(void);
int32_t ape_decoder_seek(uint32_t seek_time);
#endif

#ifdef USE_DTS_DECODER
#define DTS_INPUT_BUFFER_CAPACITY 2560
#define SIZEOF_DTSContext 25152
int32_t dts_decoder_initialize(void);
int32_t dts_decoder_decode(void);
int32_t dts_decoder_can_continue(void);
int32_t dts_decoder_seek(uint32_t seek_time);
#endif


/*******************************************************************************
 * Global Variable Definition
 *******************************************************************************/
/**
 * Global AudioDecoderContext pointer
 */
AudioDecoderContext* audio_decoder = NULL;
/*******************************************************************************/

// Declare here to avoid compiler warning.
extern void buffer_context_init(BufferContext *bc, void *io_handle, int32_t io_type, uint8_t *buffer, int32_t buffer_capcity, int32_t buffer_type);

int32_t audio_decoder_initialize(uint8_t *ram_addr, void *io_handle, int32_t io_type, int32_t decoder_type)
{    
    int32_t buffer_capacity;
    int32_t frame_context_size;
    
    if(ram_addr == NULL)
        return RT_FAILURE;

    audio_decoder = (AudioDecoderContext*)ram_addr;

    memset(audio_decoder, 0, sizeof(AudioDecoderContext));

    switch(decoder_type)
    {
#ifdef  USE_MP3_DECODER
    case MP3_DECODER:
		buffer_capacity     = MP3_INPUT_BUFFER_CAPACITY;
		frame_context_size  = SIZEOF_MPXContext;
        audio_decoder->initialize   = mp3_decoder_initialize;
        audio_decoder->decode       = mp3_decoder_decode;
        audio_decoder->can_continue = mp3_decoder_can_continue;
        audio_decoder->seek         = mp3_decoder_seek;
        break;
#endif
    
#ifdef  USE_WMA_DECODER
    case WMA_DECODER:
        buffer_capacity     = WMA_INPUT_BUFFER_CAPACITY;
        frame_context_size  = SIZEOF_WMAContext;
        audio_decoder->initialize   = wma_decoder_initialize;
        audio_decoder->decode       = wma_decoder_decode;
        audio_decoder->can_continue = wma_decoder_can_continue;
        audio_decoder->seek         = wma_decoder_seek;
        break;
#endif
        
#ifdef  USE_SBC_DECODER
    case SBC_DECODER:
    case MSBC_DECODER:
        buffer_capacity     = SBC_INPUT_BUFFER_CAPACITY;
        frame_context_size  = SIZEOF_SBCContext;
        audio_decoder->initialize   = sbc_decoder_initialize;
        audio_decoder->decode       = sbc_decoder_decode;
        audio_decoder->can_continue = sbc_decoder_can_continue;
        audio_decoder->seek         = sbc_decoder_seek;
        break;
#endif
        
#ifdef  USE_WAV_DECODER
    case WAV_DECODER:
        buffer_capacity     = WAV_INPUT_BUFFER_CAPACITY;
        frame_context_size  = SIZEOF_WAVContext;
        audio_decoder->initialize   = wav_decoder_initialize;
        audio_decoder->decode       = wav_decoder_decode;
        audio_decoder->can_continue = wav_decoder_can_continue;
        audio_decoder->seek         = wav_decoder_seek;
        break;
    #endif
    
#ifdef  USE_FLAC_DECODER
    case FLAC_DECODER:
        buffer_capacity     = FLAC_INPUT_BUFFER_CAPACITY;
        frame_context_size  = SIZEOF_FLACContext;
        audio_decoder->initialize   = flac_decoder_initialize;
        audio_decoder->decode       = flac_decoder_decode;
        audio_decoder->can_continue = flac_decoder_can_continue;
        audio_decoder->seek         = flac_decoder_seek;
        break;
    #endif

#ifdef  USE_AAC_DECODER
    case AAC_DECODER:
        buffer_capacity     = AAC_INPUT_BUFFER_CAPACITY;
        frame_context_size  = SIZEOF_AACContext;
        audio_decoder->initialize   = aac_decoder_initialize;
        audio_decoder->decode       = aac_decoder_decode;
        audio_decoder->can_continue = aac_decoder_can_continue;
        audio_decoder->seek         = aac_decoder_seek;
        break;
#endif

#ifdef  USE_AIF_DECODER
    case AIF_DECODER:
        buffer_capacity     = AIF_INPUT_BUFFER_CAPACITY;
        frame_context_size  = SIZEOF_AIFContext;
        audio_decoder->initialize   = aif_decoder_initialize;
        audio_decoder->decode       = aif_decoder_decode;
        audio_decoder->can_continue = aif_decoder_can_continue;
        audio_decoder->seek         = aif_decoder_seek;
        break;
#endif

#ifdef  USE_AMR_DECODER
    case AMR_DECODER:
        buffer_capacity     = AMR_INPUT_BUFFER_CAPACITY;
        frame_context_size  = SIZEOF_AmrnbDecoderContext;
        audio_decoder->initialize   = amrnb_decoder_initialize;
        audio_decoder->decode       = amrnb_decoder_decode;
        audio_decoder->can_continue = amrnb_decoder_can_continue;
        audio_decoder->seek         = amrnb_decoder_seek;
        break;
#endif

#ifdef  USE_APE_DECODER
	case APE_DECODER:
		buffer_capacity		= APE_INPUT_BUFFER_CAPACITY;
		frame_context_size	= SIZEOF_APEContext;
		audio_decoder->initialize = ape_decoder_initialize;
		audio_decoder->decode = ape_decoder_decode;
		audio_decoder->can_continue = ape_decoder_can_continue;
		audio_decoder->seek = ape_decoder_seek;
		break;
#endif

#ifdef  USE_DTS_DECODER
	case DTS_DECODER:
		buffer_capacity = DTS_INPUT_BUFFER_CAPACITY;
		frame_context_size = SIZEOF_DTSContext;
		audio_decoder->initialize = dts_decoder_initialize;
		audio_decoder->decode = dts_decoder_decode;
		audio_decoder->can_continue = dts_decoder_can_continue;
		audio_decoder->seek = dts_decoder_seek;
		break;
#endif
    
	default:
        return RT_FAILURE;
    }

    audio_decoder->decoder_type = decoder_type;
    audio_decoder->decoder_size = (sizeof(AudioDecoderContext) + buffer_capacity + frame_context_size + sizeof(BufferContext) + sizeof(SongInfo) + 3) & 0xFFFFFFFC;
    
    // memory allocation
    audio_decoder->inbuf     = (uint8_t*)audio_decoder + sizeof(AudioDecoderContext);
    audio_decoder->frame     = (void*)(audio_decoder->inbuf + buffer_capacity);
    audio_decoder->bc        = (BufferContext*)((uint8_t *)audio_decoder->frame + frame_context_size);
    audio_decoder->song_info = (SongInfo*)((uint8_t*)audio_decoder->bc + sizeof(BufferContext));	
    
    // clear memory
	memset(audio_decoder->frame, 0, frame_context_size+sizeof(BufferContext)+sizeof(SongInfo));
    
    buffer_context_init(audio_decoder->bc, io_handle, io_type, audio_decoder->inbuf, buffer_capacity, BUFFER_FOR_INPUT);

    // initialize genre index
    audio_decoder->song_info->genre = 255; // 255: "unknown genre" (assumed). 0: blue, so it can't be used as default.
    audio_decoder->song_info->pcm_bit_width = 16; // 16 bits per sample by default

	return audio_decoder->initialize();
}


int32_t audio_decoder_decode(void)
{
    if(NULL == audio_decoder)
        return RT_FAILURE;
    
    return audio_decoder->decode();
}


int32_t audio_decoder_can_continue(void)
{
    if(NULL == audio_decoder)
        return RT_NO;
    
    return audio_decoder->can_continue();
}


int32_t audio_decoder_get_error_code(void)
{
    if(NULL == audio_decoder)
        return 0; // no error is found.
    
    return audio_decoder->error_code;	
}


int32_t audio_decoder_seek(uint32_t seek_time)
{
    if(NULL == audio_decoder)
        return RT_FAILURE;
    
    return audio_decoder->seek(seek_time > audio_decoder->song_info->duration ? audio_decoder->song_info->duration : seek_time);
}


SongInfo* audio_decoder_get_song_info(void)
{
    if(NULL == audio_decoder)
        return NULL;
    
    return audio_decoder->song_info;
}


uint32_t audio_decoder_get_context_size(void)
{
    if(NULL == audio_decoder)
        return 0;
    
    return audio_decoder->decoder_size;
}


int32_t audio_decoder_get_mpeg_version(void)
{
#ifdef  USE_MP3_DECODER
	return mp3_decoder_get_mpeg_version();
#else
	return MPEG_NULL;
#endif
}


int32_t audio_decoder_get_id3_version(void)
{
#ifdef  USE_MP3_DECODER
	return mp3_decoder_get_id3_version();
#else
	return ID3_NULL;
#endif
}

static const uint8_t audio_decoder_lib_version[] = AUDIO_DECODER_LIBRARY_VERSION " build @ " __DATE__ " " __TIME__;

const uint8_t* audio_decoder_get_lib_version(void)
{
    return audio_decoder_lib_version;
}
