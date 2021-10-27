///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: lrc.h
//  maintainer: Sam
///////////////////////////////////////////////////////////////////////////////

/**
* @addtogroup ����
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


#define MAX_TIME_LIST_SIZE 		25  // һ��װ��ʱ����ĸ���


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
	int32_t StartTime; // ��ʼʱ���, ms
	uint32_t Duration; // ��ǰ����г���ʱ��, ms
	uint8_t* LrcText;  // ���ָ�룬���ⲿ�����ڴ�
	int16_t MaxLrcLen; // ���Buffer���ڴ��С
} LRC_ROW;

#pragma pack()

typedef struct _LRC_TIME_POINT
{
	uint32_t FilePosition; 	// ����λ�ã���λ �ֽ� Ŀǰ���ֵ65KB��
	uint16_t TimeTag;       // ʱ���ǩ����λ 200���� ���ֵ��65535*200 = 131070s��Լ3.641 hour��
	uint16_t Len;           // ���ʵ�ʳ���
} LRC_TIME_POINT;

typedef struct _LRC_INFO
{
	FIL*	 LrcFp;     	// ����ļ����
	uint32_t LastPosition;  // �����ȡʱ���ǩ�Ľ���λ��
	uint32_t LrcFileSize; 	// ����ļ���С�����FileSize > MAX_READ_FILE_SIZE������Ϊ�Ǵ��ļ�
	int32_t  Offset;      	// ʱ�䲹��������ƫ��
	uint8_t* ReadBuffer;    // װ�ظ���ļ���Buffer

	uint16_t ReadBufSize;  	// һ�ζ�ȡ����ļ��ĳ���
	uint16_t StartTime; 	// ��һ������ʼʱ��
	uint16_t LeftItemCount; // ʣ��ʱ���ǩ�ĸ���

	TEXT_ENCODE_TYPE EncodeType;  // �ļ������ʽ

	LRC_TIME_POINT TimeList[MAX_TIME_LIST_SIZE]; // ����ض�ʱ��ε�ʱ�����б����ݵ�ǰʱ������̬װ��

	uint8_t Step;
	uint8_t FindOffsetFlag;
} LRC_INFO;

/**
 * @brief ����ʱ�����ĳ�и�ʵ�ʵ�ʳ��ȣ�����Ҳ�����ʱ����Ӧ�ĸ���򷵻� -1
 * @param input SeekTime ���ʱ���
 * @return �����ʱ����Ӧ�ĸ�ʳ��ȣ�����Ҳ�����ʣ��򷵻�-1
 * @note ���û�ϣ���ö�̬�ڴ�ķ�ʽ��ø��ʱ�����ڵ���GetLrcInfo����ǰ��
 *        �ȵ��ñ�����Ԥ��֪���ض�ʱ���ĸ�ʳ��ȣ�Ȼ��̬�����װ�ظ�������ڴ棬
 *        ��ΪGetLrcInfo�������������
 */
int32_t LrcTextLengthGet(uint32_t SeekTime);

/**
 * @brief ����ʱ����ѯ�����Ϣ����Ҫ�и����ʼʱ�䣬����ʱ�䣬�������
 * @param input SeekTime ���ʱ���
 * @param input TextOffset ���ƫ�� ���ڶԺܳ��ĸ�ʷֶλ�ȡ��ÿ�δ�ָ��ƫ�Ƶ�ַ��ʼ��һ�θ��
 * @param output LrcRow, �ú���������ʱ�������ָ����Buffer Size(LrcRow->MaxLrcLen)�����Buffer(LrcRow->LrcText)
 *        ���Buffer SizeС��ʵ�ʵĸ�ʳ��ȣ����Զ��ضϸ��
 *        ��ע�⡿�����ʹ������ضϵĸ��ĩβ�ַ���Ҫ�жϺ��ֵ������ԣ�
 *                ���ڸýӿ�ֻ������ԭ�ַ��������ұ������ͽ϶࣬���Ժ��������ж��ɽӿڵ��������д���
 *
 * @return ��ʵ�ʵ�ʳ��ȣ�����Ҳ�����ʣ�����-1��
 *        ��ע�⡿������ʵ�ʳ��ȴ��ڸ��Buffer(LrcRow->LrcText)����󳤶�(LrcRow->MaxLrcLen)��
 *                �򷵻�ֵ > LrcRow->MaxLrcLen������ ����ֵ <= LrcRow->MaxLrcLen
 */
int32_t LrcInfoGet(LRC_ROW* LrcRow, uint32_t SeekTime/*ms*/, uint32_t TextOffset);

/**
 * @brief ��õ�ǰ����ļ��ı����ʽ
 * @param ��
 * @return ��ʵı�������
 */
TEXT_ENCODE_TYPE LrcEncodeTypeGet(void);

/**
 * @brief                   lrc��ʼ������
 * @param input fp   		����ļ��ļ����
 * @param input ReadBuffer  Lrc Parser�Ĺ����ڴ�ռ䣬���û��ⲿָ���ڴ�ռ�
 * @param input ReadBufSize Lrc Parser�����ռ�Ĵ�С�������ռ����û�ָ�������ֵ���� >= 128    
 * @param input Info 		Lrc ���ݽṹ��ָ��
 * @return �ļ��򿪳ɹ�����TRUE, ʧ�ܷ���FALSE
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
