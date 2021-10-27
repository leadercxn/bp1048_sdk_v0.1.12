/**
 *************************************************************************************
 * @file    song_info.h
 * @brief   Definition of song information (metadata) related structures
 *
 * @author  Alfred Zhao
 * @version v1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *
 *************************************************************************************
 */

#ifndef __SONG_INFO_H__
#define __SONG_INFO_H__

#include <stdint.h>


///**
// * MPEG Version Set
// */
//typedef enum _MPEGVersionSet
//{
//	MPEG_NULL = 0,    /**< Unkown MPPEG version */
//	MPEG_VER1 = 1,    /**< MPEG version 1       */
//	MPEG_VER2 = 2,    /**< MPEG version 2       */
//	MPEG_VER2d5 = 3     /**< MPEG version 2.5     */
//}MPEGVersionSet;


///**
// * ID3 Version Set
// */
//typedef enum _ID3VersionSet
//{
//	ID3_NULL = 0,   /**< No ID3 version existed     */
//	ID3_VER1 = 1,   /**< ID3 version 1 existed      */
//	ID3_VER2 = 2,   /**< ID3 version 2 existed      */
//	ID3_V1V2 = 3    /**< Both ID3 V1 and V2 existed */
//}ID3VersionSet;


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
	STREAM_TYPE_UNKNOWN, /**< UNKNOWN stream   */
	STREAM_TYPE_MP2, /**< MP2 stream       */
	STREAM_TYPE_MP3, /**< MP3 stream       */
	STREAM_TYPE_WMA, /**< WMA stream       */
	STREAM_TYPE_SBC, /**< SBC stream       */
	STREAM_TYPE_PCM, /**< raw PCM stream   */
	STREAM_TYPE_ADPCM, /**< IMA_ADPCM stream */
	STREAM_TYPE_FLAC, /**< FLAC stream      */
	STREAM_TYPE_APE, /**< APE stream       */
	STREAM_TYPE_AAC, /**< AAC  stream      */
	STREAM_TYPE_ALAW, /**< CCITT G711 A-law */
	STREAM_TYPE_MULAW, /**< CCITT G711 u-law */
	STREAM_TYPE_AMR, /**< AMR stream       */
	STREAM_TYPE_DTS, /**< DTS stream       */
	STREAM_TYPE_OPUS, /**< OPUS stream      */
}StreamType;


/**
 * Song Information
 */
#define MAX_TAG_LEN 64				/**< 64 bytes,the last two bytes are 0x00 always */
typedef struct _SongInfo
{
	/******************************** Stream information ********************************/
	int32_t  stream_type;           /**< Stream type, must be in @code StreamType       */
	uint32_t num_channels;          /**< Number of channels                             */
	uint32_t sampling_rate;         /**< Sampling rate, unit: Hz                        */
	uint32_t bitrate;               /**< Bit rate, unit:bps                             */
	uint32_t duration;              /**< Total play time, unit:ms                       */
	uint32_t file_size;             /**< Song file size in bytes.                       */
	uint32_t vbr_flag;              /**< VBR flag, 0:CBR, 1:VBR                         */

	/*********************************** PCM information ************************************/
	uint16_t pcm_bit_width;         /**< PCM bit width.  e.g. 16/24                     */
	uint16_t pcm_data_length;       /**< PCM data length in samples after decoding (irrelevant to #channels and possibly changeable for each frame, e.g. WMA) */
	int32_t *pcm_addr;              /**< PCM start address                              */

	/*********************************** Tag information ************************************/
	int32_t char_set;               /**< Char set for tag info, @see TagCharSet, can be ISO_8859_1, UTF_16, UTF_8 */
	uint8_t title[MAX_TAG_LEN];		/**< Title   , end with "\0\0"                      */
	uint8_t artist[MAX_TAG_LEN];	/**< Author  , end with "\0\0"                      */
	uint8_t album[MAX_TAG_LEN];		/**< Album   , end with "\0\0"                      */
	uint8_t comment[MAX_TAG_LEN];   /**< Comments,                                      */
	uint8_t genre_str[MAX_TAG_LEN];	/**< Genre string for ID3v2                         */
	uint8_t year[6];                /**< Years   , year in string, e.g. '2''0''1''2'    */
	uint8_t track;                  /**< Track   , track number of music                */
	uint8_t genre;                  /**< Genre   , style index of music                 */
}SongInfo;

#endif//__SONG_INFO_H__
