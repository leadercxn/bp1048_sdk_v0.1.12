/**
 **************************************************************************************
 * @file    fft.h
 * @brief   FFT API
 *
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * $Created: 2017-05-11 13:32:38$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

/**
 * @addtogroup FFT
 * @{
 * @defgroup fft fft.h
 * @{
 */

#ifndef __FFT_H__
#define __FFT_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

/**
 * @brief FFT错误码
 */
typedef enum __FFT_ERROR_CODE
{
    //FFT Error Code
	FFT_EC_INVALID_FFT_ADDR = -200,
	FFT_EC_UNSUPPORTED_FFT_TYPE,
	FFT_EC_UNSUPPORTED_FFT_SIZE,
	FFT_EC_HW_FFT_BUSY,

    //NO ERROR
    FFT_OK = 0
} FFT_ERROR_CODE;

/**
 * @brief FFT类型
 */
typedef enum __FFT_TYPE
{
    FFT_TYPE_RFFT  = 0, /**< 实数FFT   */
    FFT_TYPE_RIFFT = 1, /**< 实数IFFT  */
    FFT_TYPE_CFFT  = 2, /**< 复数FFT   */
    FFT_TYPE_CIFFT = 3, /**< 复数IFFT  */
} FFT_TYPE;

/**
 * @brief FFT块大小
 */
typedef enum __FFT_SIZE
{
    FFT_SIZE_32   = 5, 
    FFT_SIZE_64   = 6, 
    FFT_SIZE_128  = 7, 
    FFT_SIZE_256  = 8, 
    FFT_SIZE_512  = 9, 
    FFT_SIZE_1024 = 10,
    FFT_SIZE_2048 = 11,
} FFT_SIZE;


/**
 * @brief  初始化FFT模块
 *
 * @param[in]       Type       FFT类型
 * @param[in,out]   DataInOut  输入输出数据地址，宽度为32bit.
 *							   FFT_TYPE_RFFT:
 * 							       输入数据宽度为32bit，建议将输入的16bit的PCM数据左移16bit，保留最多的精度。
 * 							       输出数据宽度为32bit，输出格式为：Re(0), Re(DataCount/2), Re(1), Im(1), Re(2), Im(2)....Re(DataCount/2 -1), Im(DataCount/2 -1)
 *                                 其中Re(0)为直流分量
 *
 *							   FFT_TYPE_RIFFT:
 *								   输入数据宽度为32bit, 输入格式为：Re(0), Re(DataCount/2), Re(1), Im(1), Re(2), Im(2)....Re(DataCount/2 -1), Im(DataCount/2 -1)
 *								   输出数据宽度为32bit, Re(0), Re(1), Re(2), Re(3)...Re(N-1)
 *
 *							   FFT_TYPE_CFFT和FFT_TYPE_CIFFT:
 *								   输入数据格式：Re(0), Im(0), Re(1), Im(1),...Re(N-1),Im(N-1)
 *								   输出数据格式：Re(0), Im(0), Re(1), Im(1),...Re(N-1),Im(N-1)
 *                                                      
 * @param[in]       Size       FFT块大小
 *							   Type = FFT_TYPE_RFFT or FFT_TYPE_RIFFT：FFT_SIZE_64 ~ FFT_SIZE_2048
 *							   Type = FFT_TYPE_CFFT or FFT_TYPE_CIFFT: FFT_SIZE_32 ~ FFT_SIZE_1024
 *
 * @return 返回初始化的错误码
 */
FFT_ERROR_CODE FFT_Init(FFT_TYPE Type, int32_t* DataInOut, FFT_SIZE Size);

/**
 * @brief  开启FFT运算
 *
 * @param  无
 *
 * @return 返回错误码
 */
FFT_ERROR_CODE FFT_Start(void);

/**
 * @brief  判断FFT是否运算完成
 *
 * @param  NONE
 *
 * @return TRUE: 完成    FALSE:运算中
 */
bool FFT_IsDone(void);

/**
 * @brief  软件清除done标志，该标志也会在fft_start函数时硬件自动清除
 *
 * @param  NONE
 *
 * @return TRUE: 完成    FALSE:运算中
 */
void FFT_DoneFlagClear(void);

/**
 * @brief  判断FFT是否处于忙状态
 *
 * @param  NONE
 *
 * @return TRUE：忙状态   FALSE: 空闲状态
 */
bool FFT_IsWorking(void);

/**
 * @brief  使能FFT中断
 *
 * @return 错误码，详见 #FFT_ERROR_CODE
 */
FFT_ERROR_CODE FFT_InterruptEnable(void);

/**
 * @brief  禁能FFT中断
 *
 * @return 错误码，详见 #FFT_ERROR_CODE
 */
FFT_ERROR_CODE FFT_InterruptDisable(void);


/**
 * @brief  获取FFT的中断标志
 *
 * @return TRUE：中断发生  FALSE: 无中断产生
 */
bool FFT_InterruptFlagGet(void);

/**
 * @brief  清除FFT中断标志
 *
 * @return 无
 */
void FFT_InterruptFlagClear(void);


/**
 * @brief  复位FFT模块
 *
 * @param[in]  无
 *
 * @return 无
 */
void FFT_Reset(void);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__FFT_H__

/**
 * @}
 * @}
 */
