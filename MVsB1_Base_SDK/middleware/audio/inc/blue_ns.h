/**
 *************************************************************************************
 * @file	blue_ns.h
 * @brief	Noise Suppression for Mono Signals
 *
 * @author	ZHAO Ying (Alfred)
 * @version	v1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __BLUE_NS_H__
#define __BLUE_NS_H__

#include <stdint.h>
#include "blue_ns_core.h"


/** noise suppression structure */
typedef struct _BlueNS
{
	BlueNSCore ns_core;
	int16_t xin_prev[BLK_LEN];
	int16_t xout_prev[BLK_LEN];
	int32_t xv[BLK_LEN * 2];
} BlueNSContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**
 * @brief Initialize noise suppression module.
 * @param ct Pointer to a BlueNSContext object.
 * @return none.
 * @note Only mono signals are supported.
 */
void blue_ns_init(BlueNSContext *ct);


/**
 * @brief Run noise suppression to a block of PCM data
 * @param ct Pointer to a BlueNSContext object.
 * @param xin Input PCM data. The size of xin is equal to BLK_LEN.
 * @param xout Output PCM data. The size of xout is equal to BLK_LEN. 
 *        xout can be the same as xin. In this case, the PCM is changed in-place.
 * @param ns_level Noise suppression level. Valid range: 0 ~ 5. Use 0 to disable noise suppression while 5 to apply maximum suppression. 
 * @return error code. NS_ERROR_OK means successful, other codes indicate error.
 * @note Only mono signals are supported. 
 */
int32_t blue_ns_run(BlueNSContext *ct, int16_t xin[BLK_LEN], int16_t xout[BLK_LEN], int32_t ns_level);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif//__BLUE_NS_H__
