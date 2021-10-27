///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: lrc.h
//  maintainer: Sam
///////////////////////////////////////////////////////////////////////////////

/**
* @addtogroup 驱动
* @{
* @defgroup LRC LRC
* @{
*/

#ifndef __LRC_H__
#define __LRC_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "ff.h"


#define MAX_TIME_LIST_SIZE 		25  // 一次装载时间轴的个数


#pragma pack(1)

typedef enum _TEXT_ENCODE_TYPE
{
    ENCODE_UNKNOWN,
    ENCODE_ANSI,
    ENCODE_UTF8,
    ENCODE_UTF16, // unicode
    ENCODE_UTF16_BIG,
    ENCODE_GBK,
} TEXT_ENCODE_TYPE;

typedef struct _LRC_ROW
{
	int32_t StartTime; // 起始时间点, ms
	uint32_t Duration; // 当前歌词行持续时间, ms
	uint8_t* LrcText;  // 歌词指针，由外部分配内存
	int16_t MaxLrcLen; // 歌词Buffer的内存大小
} LRC_ROW;

#pragma pack()

typedef struct _LRC_TIME_POINT
{
	uint32_t FilePosition; 	// 所在位置（单位 字节 目前最大值65KB）
	uint16_t TimeTag;       // 时间标签（单位 200毫秒 最大值：65535*200 = 131070s，约3.641 hour）
	uint16_t Len;           // 歌词实际长度
} LRC_TIME_POINT;

typedef struct _LRC_INFO
{
	FIL*	 LrcFp;     	// 歌词文件句柄
	uint32_t LastPosition;  // 最近获取时间标签的结束位置
	uint32_t LrcFileSize; 	// 歌词文件大小，如果FileSize > MAX_READ_FILE_SIZE，则认为是大文件
	int32_t  Offset;      	// 时间补偿，整体偏移
	uint8_t* ReadBuffer;    // 装载歌词文件的Buffer

	uint16_t ReadBufSize;  	// 一次读取歌词文件的长度
	uint16_t StartTime; 	// 第一句歌词起始时间
	uint16_t LeftItemCount; // 剩余时间标签的个数

	TEXT_ENCODE_TYPE EncodeType;  // 文件编码格式

	LRC_TIME_POINT TimeList[MAX_TIME_LIST_SIZE]; // 歌词特定时间段的时间轴列表，根据当前时间点而动态装载

	uint8_t Step;
	uint8_t FindOffsetFlag;
} LRC_INFO;

/**
 * @brief 根据时间点获得某行歌词的实际长度，如果找不到该时间点对应的歌词则返回 -1
 * @param input SeekTime 歌词时间点
 * @return 输出与时间点对应的歌词长度，如果找不到歌词，则返回-1
 * @note 当用户希望用动态内存的方式获得歌词时，可在调用GetLrcInfo函数前，
 *        先调用本函数预先知道特定时间点的歌词长度，然后动态分配好装载歌词所需内存，
 *        作为GetLrcInfo函数的输入参数
 */
int32_t LrcTextLengthGet(uint32_t SeekTime);

/**
 * @brief 根据时间点查询歌词信息，主要有歌词起始时间，持续时间，歌词内容
 * @param input SeekTime 歌词时间点
 * @param input TextOffset 歌词偏移 用于对很长的歌词分段获取，每次从指定偏移地址开始读一段歌词
 * @param output LrcRow, 该函数输出歌词时，会根据指定的Buffer Size(LrcRow->MaxLrcLen)，填充Buffer(LrcRow->LrcText)
 *        如果Buffer Size小于实际的歌词长度，则自动截断歌词
 *        【注意】如果歌词过长，截断的歌词末尾字符需要判断汉字的完整性，
 *                由于该接口只输出歌词原字符串，并且编码类型较多，所以汉字完整判断由接口调用者自行处理
 *
 * @return 歌词的实际长度，如果找不到歌词，返回-1，
 *        【注意】如果歌词实际长度大于歌词Buffer(LrcRow->LrcText)的最大长度(LrcRow->MaxLrcLen)，
 *                则返回值 > LrcRow->MaxLrcLen，否则 返回值 <= LrcRow->MaxLrcLen
 */
int32_t LrcInfoGet(LRC_ROW* LrcRow, uint32_t SeekTime/*ms*/, uint32_t TextOffset);

/**
 * @brief 获得当前歌词文件的编码格式
 * @param 无
 * @return 歌词的编码类型
 */
TEXT_ENCODE_TYPE LrcEncodeTypeGet(void);

/**
 * @brief                   lrc初始化操作
 * @param input fp   		歌词文件文件句柄
 * @param input ReadBuffer  Lrc Parser的工作内存空间，由用户外部指定内存空间
 * @param input ReadBufSize Lrc Parser工作空间的大小，工作空间由用户指定，则该值必须 >= 128    
 * @param input Info 		Lrc 数据结构体指针
 * @return 文件打开成功返回TRUE, 失败返回FALSE
 */
bool LrcInit(FIL* fp, uint8_t* ReadBuffer, uint32_t ReadBufSize, LRC_INFO* Info);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif

/**
 * @}
 * @}
 */
