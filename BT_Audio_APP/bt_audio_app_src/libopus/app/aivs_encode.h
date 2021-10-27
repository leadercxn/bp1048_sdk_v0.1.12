//
// Created by zhaojianxing on 19-01-23.
//



#ifndef _AIVS_ENCODE_H
#define _AIVS_ENCODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "opus.h"

#include "stdio.h"
#include "stdarg.h"

/*
  设备端一次请求编码流程：
aivs_opus_encode_init-->aivs_opus_encode_stream--->aivs_opus_encode_destroy

其余均为示例代码无需关注

*/

int aivs_opus_encode_init(void);

//对数据进行编码
/*
  pInputData：编码数据。采样格式为S16_LE
  inputDataLen: pInputData 数据长度
  pOutputData:编码输出结果数据.内存由调用者分配和释放。
  outputLen:编码结果pOutputData长度.用户据此长度发送内容pOutputData,或者缓存多个pOutputData一起发送。
*/

/* inputDataLen 长度除非最后一个报文 否则必须为640 防止不满帧数据插值导致音频数据杂音 */
int aivs_opus_encode_stream(const char* pInputData, int inputDataLen, unsigned char* pOutputData, int* outputLen);



void aivs_opus_encode_destroy(void);


void aivs_log(const char *format, ...);

#define LOG_SIZE  512


__attribute__((weak)) void aivs_log(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    char buffer[512];
    vsnprintf(buffer, 512, format, argptr);
    va_end(argptr);

    printf("%s\n", buffer);
}



#ifdef __cplusplus
}
#endif

#endif //_AIVS_ENCODE_H