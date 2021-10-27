/**
 *************************************************************************************
 * @file	blue_ns_core.h
 * @brief	Noise Suppression Core for Mono Signals
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.4.2
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __BLUE_NS_CORE_H__
#define __BLUE_NS_CORE_H__

#include <stdint.h>

//#define BLK_LEN 64	// Block length
//#define BLK_BITS 6	// 2^BLK_BITS=BLK_LEN

#define BLK_LEN 128		// Block length
#define BLK_BITS 7		// 2^BLK_BITS=BLK_LEN


/** error code for noise suppression */
typedef enum _NS_ERROR_CODE
{
	NS_ERROR_ILLEGAL_NS_LEVEL = -256,	
	// No Error
	NS_ERROR_OK = 0,					/**< no error              */
} NS_ERROR_CODE;

/** noise suppression core structure */
typedef struct _BlueNSCore
{
	float x_abs[BLK_LEN+1];
	float noise_pow[BLK_LEN+1];
	int32_t ph1_mean[BLK_LEN+1];
	float g[BLK_LEN+1];
	float gamma[BLK_LEN+1];
} BlueNSCore;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief Initialize noise suppression core.
 * @param core Pointer to a BlueNSCore object.
 * @return none.
 * @note Only mono signals are supported.
 */
void blue_ns_core_init(BlueNSCore *core);


/**
 * @brief Run noise suppression core to a block of spectrum data (usually after windowing & FFT process)
 * @param core Pointer to a BlueNSCore object.
 * @param xfft Spectrum data (usually after windowing & FFT process). The size of xfft is equal to BLK_LEN*2. 
 *		Specaially, xfft[0] is for DC component, xfft[1] is for the highest frequency bin. 
 *		xfft[2] & xfft[3] are for the 2nd frequency bin's real and imaginary part respectively, 
 *		xfft[4] & xfft[5] are for the 3rd frequency bin's real and imaginary part respectively and so on 
 * @param ns_level Noise suppression level. Valid range: 0 ~ 5. Use 0 to disable noise suppression while 5 to apply maximum suppression. 
 * @return error code. NS_ERROR_OK means successful, other codes indicate error.
 * @note Only mono signals are supported. 
 */
int32_t blue_ns_core_run(BlueNSCore *core, int32_t *xfft, int32_t ns_level);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__BLUE_NS_CORE_H__
