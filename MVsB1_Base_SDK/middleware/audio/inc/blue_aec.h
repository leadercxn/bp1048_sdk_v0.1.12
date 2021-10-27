/**
 *************************************************************************************
 * @file	blue_aec.h
 * @brief	Acoustic Echo Cancellation (AEC) routines for 8kHz/16kHz voice signals
 *
 * @author	ZHAO Ying (Alfred)
 * @version	V5.7.2
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *
 *************************************************************************************
 */

#ifndef _BLUE_AEC_H
#define _BLUE_AEC_H


#include <stdint.h>
#include "blue_ns_core.h"


// BLK_LEN & BLK_BITS are defined in blue_ns_core.h
#if BLK_LEN==64
#define FILTER_L 12				// Filter length = BLK_LEN * FILTER_L
#elif BLK_LEN==128
#define FILTER_L 6					// Filter length = BLK_LEN * FILTER_L
#endif


typedef struct _BlueAECContext
{
	// matrices
	int32_t wm[FILTER_L][BLK_LEN*2];
	int32_t um[FILTER_L][BLK_LEN*2];
	int32_t uwm[FILTER_L][BLK_LEN*2];

	// vectors
	int32_t umclz[FILTER_L];
	int32_t uv[BLK_LEN*2];
	int32_t yev[BLK_LEN*2];
	int32_t phiv[BLK_LEN*2];
	float pv[BLK_LEN+1];
	
	float yp[BLK_LEN + 1];
	float sy[BLK_LEN + 1];
	float sym[BLK_LEN + 1];

	int64_t dd[BLK_LEN+1];
	int64_t uu[BLK_LEN+1];
	int64_t ee[BLK_LEN+1];
	int64_t ud[BLK_LEN*2];
	int64_t ed[BLK_LEN*2];

	int16_t cud[BLK_LEN+1];
	int16_t ced[BLK_LEN+1];
	int16_t hed[BLK_LEN+1];		

	int16_t uin_prev[BLK_LEN];
	int16_t din_prev[BLK_LEN];
	int16_t eout1_prev[BLK_LEN];
	int16_t eout2_prev[BLK_LEN];

	BlueNSCore ns_core;
	
	// scalars
	int32_t curr_blk_idx;
	int32_t blk_counter;
	int32_t maxw_idx;
	int32_t diverge;
	int32_t supp_state;
	int32_t cud_localmin;
	int32_t hlocalmin;
	int32_t hmin;
	int32_t hnewmin;
	int32_t hminctr;
	int32_t ovrd, ovrd_sm;
	int32_t es_level;
	int32_t ns_level;

} BlueAECContext;


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


/**
 * @brief  Initialization for AEC
 * @param  ct Pointer to the AEC context structure. The structure's memory should be allocated by the calling process.
 * @param  es_level Echo suppression level. 0: off, 1: minimum suppression, ..., 5: maximum suppression. The higher the suppression level, the lower the echo. However higher suppression level may affect duplex performance.
 * @param  ns_level Noise suppression level (0~5). 0: off, 1: minimum suppression, ..., 5: maximum suppression. The higher the suppression level, the lower the background noise. However higher suppression level may affect near-end speech clarity.
 * @return 0 (always success)
 */
int32_t blue_aec_init(BlueAECContext* ct, int32_t es_level, int32_t ns_level);


/**
 * @brief  AEC processing routine
 * @param  ct Pointer to the AEC context structure. The structure's memory should be allocated by the calling process.
 * @param  uin far-end voice block
 * @param  din near-end (local) voice block recorded from microphone
 * @param  eout AEC processed near-end (local) voice block for being sent to the far-end
 * @return none
 */
void blue_aec_run(BlueAECContext* ct, int16_t uin[BLK_LEN], int16_t din[BLK_LEN], int16_t eout[BLK_LEN]);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif // _BLUE_AEC_H
