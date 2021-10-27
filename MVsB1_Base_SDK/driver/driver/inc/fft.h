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
 * @brief FFT������
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
 * @brief FFT����
 */
typedef enum __FFT_TYPE
{
    FFT_TYPE_RFFT  = 0, /**< ʵ��FFT   */
    FFT_TYPE_RIFFT = 1, /**< ʵ��IFFT  */
    FFT_TYPE_CFFT  = 2, /**< ����FFT   */
    FFT_TYPE_CIFFT = 3, /**< ����IFFT  */
} FFT_TYPE;

/**
 * @brief FFT���С
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
 * @brief  ��ʼ��FFTģ��
 *
 * @param[in]       Type       FFT����
 * @param[in,out]   DataInOut  ����������ݵ�ַ�����Ϊ32bit.
 *							   FFT_TYPE_RFFT:
 * 							       �������ݿ��Ϊ32bit�����齫�����16bit��PCM��������16bit���������ľ��ȡ�
 * 							       ������ݿ��Ϊ32bit�������ʽΪ��Re(0), Re(DataCount/2), Re(1), Im(1), Re(2), Im(2)....Re(DataCount/2 -1), Im(DataCount/2 -1)
 *                                 ����Re(0)Ϊֱ������
 *
 *							   FFT_TYPE_RIFFT:
 *								   �������ݿ��Ϊ32bit, �����ʽΪ��Re(0), Re(DataCount/2), Re(1), Im(1), Re(2), Im(2)....Re(DataCount/2 -1), Im(DataCount/2 -1)
 *								   ������ݿ��Ϊ32bit, Re(0), Re(1), Re(2), Re(3)...Re(N-1)
 *
 *							   FFT_TYPE_CFFT��FFT_TYPE_CIFFT:
 *								   �������ݸ�ʽ��Re(0), Im(0), Re(1), Im(1),...Re(N-1),Im(N-1)
 *								   ������ݸ�ʽ��Re(0), Im(0), Re(1), Im(1),...Re(N-1),Im(N-1)
 *                                                      
 * @param[in]       Size       FFT���С
 *							   Type = FFT_TYPE_RFFT or FFT_TYPE_RIFFT��FFT_SIZE_64 ~ FFT_SIZE_2048
 *							   Type = FFT_TYPE_CFFT or FFT_TYPE_CIFFT: FFT_SIZE_32 ~ FFT_SIZE_1024
 *
 * @return ���س�ʼ���Ĵ�����
 */
FFT_ERROR_CODE FFT_Init(FFT_TYPE Type, int32_t* DataInOut, FFT_SIZE Size);

/**
 * @brief  ����FFT����
 *
 * @param  ��
 *
 * @return ���ش�����
 */
FFT_ERROR_CODE FFT_Start(void);

/**
 * @brief  �ж�FFT�Ƿ��������
 *
 * @param  NONE
 *
 * @return TRUE: ���    FALSE:������
 */
bool FFT_IsDone(void);

/**
 * @brief  ������done��־���ñ�־Ҳ����fft_start����ʱӲ���Զ����
 *
 * @param  NONE
 *
 * @return TRUE: ���    FALSE:������
 */
void FFT_DoneFlagClear(void);

/**
 * @brief  �ж�FFT�Ƿ���æ״̬
 *
 * @param  NONE
 *
 * @return TRUE��æ״̬   FALSE: ����״̬
 */
bool FFT_IsWorking(void);

/**
 * @brief  ʹ��FFT�ж�
 *
 * @return �����룬��� #FFT_ERROR_CODE
 */
FFT_ERROR_CODE FFT_InterruptEnable(void);

/**
 * @brief  ����FFT�ж�
 *
 * @return �����룬��� #FFT_ERROR_CODE
 */
FFT_ERROR_CODE FFT_InterruptDisable(void);


/**
 * @brief  ��ȡFFT���жϱ�־
 *
 * @return TRUE���жϷ���  FALSE: ���жϲ���
 */
bool FFT_InterruptFlagGet(void);

/**
 * @brief  ���FFT�жϱ�־
 *
 * @return ��
 */
void FFT_InterruptFlagClear(void);


/**
 * @brief  ��λFFTģ��
 *
 * @param[in]  ��
 *
 * @return ��
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
