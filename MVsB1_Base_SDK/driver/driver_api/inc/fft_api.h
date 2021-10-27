/**
 **************************************************************************************
 * @file    fft_api.h
 * @brief   wrapper function for FFT (hardware/software)
 *
 * @author  Castle, Alfred
 * @version V1.0.0
 *
 * $Created: 2018-04-24 16:15:00$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __FFT_API_H__
#define __FFT_API_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 


/**
 * @brief      Real FFT
 *
 * @param[in]  x for input/output buffer
 * @param[in]  data_count for data length, only valid with 64,128,256,512,1024,2048
 * @param[in]  scale_down , scale down enable. 0: disable scaling down,  1: enable scaling down. If scaling down is disabled, the output is not scaled down, which is the same as Matlab's result. However the output may run into the risk of overflow. If scaling down is enabled, the output is scaled down, which prevents output overflow. It is suggested to disable scaling down if the maximum absolute value of the input is no more that 2^(31-n)-1, where 2^n = data_count. For example if all your input data is in the range of signed short (16-bit), which is the typcial case for audio PCM values, you can disable scaling down.
 * @return     1 for success, 0 for failure.
 */
int rfft_api(int32_t* x, int data_count, int scale_down);


/**
 * @brief      Real Inverse FFT
 *
 * @param[in]  x for input/output buffer
 * @param[in]  data_count for data length, only valid with 64,128,256,512,1024,2048
 * @return     1 for success, 0 for failure.
 */
int rifft_api(int32_t* x, int data_count);

/**
 * @brief      Complex FFT
 *
 * @param[in]  x for input/output buffer
 * @param[in]  data_count for data length, only valid with 32,64,128,256,512,1024
 * @param[in]  scale_down , scale down enable. 0: disable scaling down,  1: enable scaling down. If scaling down is disabled, the output is not scaled down, which is the same as Matlab's result. However the output may run into the risk of overflow. If scaling down is enabled, the output is scaled down, which prevents output overflow. It is suggested to disable scaling down if the maximum absolute value of the input is no more that 2^(31-n)-1, where 2^n = data_count. For example if all your input data is in the range of signed short (16-bit), which is the typcial case for audio PCM values, you can disable scaling down.
 * @return     1 for success, 0 for failure.
 */
int cfft_api(int32_t* x, int data_count, int scale_down);

/**
 * @brief      Complex Inverse FFT
 *
 * @param[in]  x for input/output buffer
 * @param[in]  data_count for data length, only valid with 32,64,128,256,512,1024
 * @return     1 for success, 0 for failure.
 */
int cifft_api(int32_t* x, int data_count);



#ifdef __cplusplus
}
#endif // __cplusplus 

#endif//__FFT_API_H__

