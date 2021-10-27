/**
 *************************************************************************************
 * @file    audio_decoder_api.h
 * @brief   Audio decoder API interface.
 *
 * @author  Alfred Zhao, Aissen Li, Lean Xiong
 * @version @see AUDIO_DECODER_LIBRARY_VERSION
 * 
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __AUDIO_DECODER_API_H__
#define __AUDIO_DECODER_API_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "typedefine.h"

#define AUDIO_DECODER_LIBRARY_VERSION   "7.7.13"     /**< audio decoder library version string        */
#define MAX_TAG_LEN                     64          /**< 64 bytes,the last two bytes are 0x00 always */


/**
 * Audio Decoder Type Set 
 */
typedef enum _DecoderType
{
    UNKNOWN_DECODER              = 0,       /**< Unknown decoder */
    FLAC_DECODER ,                          /**< FLAC decoder */
	APE_DECODER,                            /**< APE decoder */
	WAV_DECODER,							/**< WAV  decoder */	
    AIF_DECODER  ,                          /**< AIFF and AIFC decoder */
    AMR_DECODER,                            /**< AMR  decoder */
	AAC_DECODER,							/**< AAC  decoder */
	MP3_DECODER,                            /**< MP3  decoder */
    WMA_DECODER,                            /**< WAM  decoder */
    SBC_DECODER,                            /**< SBC  decoder */
    MSBC_DECODER,                           /**< mSBC decoder */
	DTS_DECODER,                            /**< DTS  decoder */
}DecoderType;


/**
 * MPEG Version Set
 */
typedef enum _MPEGVersionSet
{
    MPEG_NULL   = 0,    /**< Unkown MPPEG version */
    MPEG_VER1   = 1,    /**< MPEG version 1       */
    MPEG_VER2   = 2,    /**< MPEG version 2       */
    MPEG_VER2d5 = 3     /**< MPEG version 2.5     */
}MPEGVersionSet;


/**
 * ID3 Version Set
 */
typedef enum _ID3VersionSet
{
    ID3_NULL = 0,   /**< No ID3 version existed     */
    ID3_VER1 = 1,   /**< ID3 version 1 existed      */
    ID3_VER2 = 2,   /**< ID3 version 2 existed      */
    ID3_V1V2 = 3    /**< Both ID3 V1 and V2 existed */
}ID3VersionSet;


/**
 * Tag Char Set
 *
 * @note if the most significant 4 bits of char set is "0x0", it means all the strings encoded with the same chat set.
 *       if the most significant 4 bits of char set is "0x1", it means all the strings (title, artist, album, comment,
 *       genre_str) encoded with differet chat set, the other bits fileds indicates as following:
 *           bits [3:0]   means the char set of "title",
 *           bits [7:4]   means the char set of "artist",
 *           bits [11:8]  means the char set of "album",
 *           bits [15:12] means the char set of "comment",
 *           bits [19:16] means the char set of "genre_str",
 *       if the most significant 4 bits of char set is other value, it means reserved for furture use.
 */
typedef enum _TagCharSet
{
    CHAR_SET_UNKOWN,
    CHAR_SET_ISO_8859_1,
    CHAR_SET_UTF_16,
    CHAR_SET_UTF_8,
    CHAR_SET_DIVERSE = 0x10000000
}TagCharSet;


/**
 * Stream Type
 */
typedef enum _StreamType
{
    STREAM_TYPE_UNKNOWN , /**< UNKNOWN stream   */
    STREAM_TYPE_MP2     , /**< MP2 stream       */
    STREAM_TYPE_MP3     , /**< MP3 stream       */
    STREAM_TYPE_WMA     , /**< WMA stream       */
    STREAM_TYPE_SBC     , /**< SBC stream       */
    STREAM_TYPE_PCM     , /**< raw PCM stream   */
    STREAM_TYPE_ADPCM   , /**< IMA_ADPCM stream */
    STREAM_TYPE_FLAC    , /**< FLAC stream      */
	STREAM_TYPE_APE		, /**< APE stream       */
    STREAM_TYPE_AAC     , /**< AAC  stream      */
    STREAM_TYPE_ALAW    , /**< CCITT G711 A-law */
    STREAM_TYPE_MULAW   , /**< CCITT G711 u-law */
    STREAM_TYPE_AMR     , /**< AMR stream       */
	STREAM_TYPE_DTS		, /**< DTS stream       */
}StreamType;


/**
 * Song Information
 */
typedef struct _SongInfo
{
    /************************************ Stream information ********************************/
    int32_t     stream_type;            /**< Stream type, must be in @code StreamTypeSet    */
    uint32_t    num_channels;           /**< Number of channels                             */
    uint32_t    sampling_rate;          /**< Sampling rate, unit: Hz                        */
    uint32_t    bitrate;                /**< Bit rate, unit:bps                             */
    uint32_t    duration;               /**< Total play time, unit:ms                       */
    uint32_t    file_size;              /**< Song file size in bytes.                       */
    uint32_t    vbr_flag;               /**< VBR flag, 0:CBR, 1:VBR                         */
    
    /*********************************** PCM information ************************************/
    uint16_t    pcm_bit_width;          /**< PCM bit width.  e.g. 16/24                     */
    uint16_t    pcm_data_length;        /**< PCM data length in samples after decoding (    +/
                                             irrelevant to #channels and possibly changeable+/
                                             for each frame, e.g. WMA)                      */
    int32_t     *pcm_addr;              /**< PCM start address                              */
    
    /*********************************** Tag information ************************************/	
    int32_t    char_set;                /**< Char set for tag info, @see TagCharSet, can    +/ 
                                             be ISO_8859_1, UTF_16, UTF_8                   */
    uint8_t title[MAX_TAG_LEN]  ;       /**< Title   , end with "\0\0"                      */
    uint8_t artist[MAX_TAG_LEN] ;       /**< Author  , end with "\0\0"                      */
    uint8_t album[MAX_TAG_LEN]  ;       /**< Album   , end with "\0\0"                      */
    uint8_t comment[MAX_TAG_LEN];       /**< Comments,                                      */
    uint8_t genre_str[MAX_TAG_LEN];     /**< Genre string for ID3v2                         */    
    uint8_t year[6];                    /**< Years   , year in string, e.g. '2''0''1''2'    */
    uint8_t track;                      /**< Track   , track number of music                */
    uint8_t genre;                      /**< Genre   , style index of music                 */

}SongInfo;


/**
 * Audio Decoder Context
 */
typedef struct _AudioDecoderContext
{
    int32_t       decoder_type;         /**< Audio decoder type, value must be in @code DecoderType     */
    uint32_t      decoder_size;         /**< Audio decoder size, equal to sizeof(AudioDecoderContext)   +/
                                                                          sizeof(input_buffer_capacity) +/
                                                                          sizeof(frame_context_size)    +/
                                                                          sizeof(BufferContext)         +/
                                                                          sizeof(SongInfo)              */

    int32_t       error_code;           /**< Audio decoder return error code.                           */

    uint8_t       *inbuf;               /**< Input buffer                                               */
    void          *frame;               /**< Audio decoder state info context pointer,can be MP3Context,+/
                                             WMAContext, SBCContext, WAVContext, FLACContext            */
    BufferContext *bc;                  /**< Buffer context pointer                                     */
    SongInfo      *song_info;           /**< Song information context pointer                           */
                                           
    int32_t      (*initialize)(void);    /**< Function pointer for initializing audio decoder           */
    int32_t      (*decode)(void);        /**< Function pointer for decoding one audio frame             */
    int32_t      (*can_continue)(void);  /**< Function pointer for checking whether the audio decoder   +/
                                              can go on, if can, then decode, else abort audio decoder  */
    int32_t      (*seek)(uint32_t seek_time); /**< Function pointer for seeking to special time to play */
    
}AudioDecoderContext;

/*******************************************************************************
 * Global Variable Definition
 *******************************************************************************/
/**
 * Global AudioDecoderContext pointer
 */
extern AudioDecoderContext *audio_decoder;

/**
 * @brief  Initialize audio decoder
 * @param  ram_addr     ram address
 * @param  io_handle    IO handle
 * @param  io_type      IOType pointer, value must be in @code IOType
 * @param  decoder_type audio decoder type, value must be in @code DecoderType
 * @return initialize result, RT_FAILURE, RT_SUCCESS
 */ 
int32_t audio_decoder_initialize(uint8_t *ram_addr, void *io_handle, int32_t io_type, int32_t decoder_type);

/**
 * @brief  Decode an audio frame
 * @return RT_FAILURE, RT_SUCCESS
 */ 
int32_t audio_decoder_decode(void);

/**
 * @brief  Check whether the audio decoder can go on
 * @return check result, RT_NO, RT_YES
 */ 
int32_t audio_decoder_can_continue(void);

/**
 * @brief  Get error code from AudioDecoderContext
 * @return error code
 */ 
int32_t audio_decoder_get_error_code(void);

/**
 * @brief  Seek to file position according as the specified time. 
 * @param  seek_time   the specified time in millisecond(ms)
 * @return RT_SUCCESS, RT_FAILURE
*/
int32_t audio_decoder_seek(uint32_t seek_time);

/**
 * @brief  Get song information.
 * @return SongInfo pointer
 */
SongInfo* audio_decoder_get_song_info(void);

/**
 * @brief  Get audio decoder's entire context size in bytes.
 * @return audio decoder's entire context size in bytes.
 */
uint32_t audio_decoder_get_context_size(void);

/**
 * @brief  Get MPEG version for MP2/3 songs
 * @return MPEG version value, @see MpegVersionSet enumeration
 */
int32_t audio_decoder_get_mpeg_version(void);

/**
 * @brief  Get ID3 version
 * @return ID3 version value, @see ID3VersionSet enumeration
 */
int32_t audio_decoder_get_id3_version(void);

/**
 * @brief  Get audio decoder library version
 * @return audio decoder library version string
 */
const uint8_t* audio_decoder_get_lib_version(void);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__AUDIO_DECODER_API_H__
