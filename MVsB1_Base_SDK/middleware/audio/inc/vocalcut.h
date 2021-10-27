/**
 **************************************************************************************
 * @file    vocalcut.h
 * @brief   Vocal Cut Effect Module
 *
 * @author  Cecilia Wang
 * @version V1.6.1
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
#ifndef __VOCALCUT_H__
#define __VOCALCUT_H__

#include <stdint.h>
#include "eq.h"

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#define MAX_FILTER_COUNT	6
    
/** error code for VOCAL */
typedef enum _VOCALCUT_ERROR_CODE
{

    VOCALCUT_CONTEXT_ERROR = -256,
    
    //No Error
    VOCALCUT_ERROR_OK = 0,
    
} VOCALCUT_ERROR_CODE; 

    
/** vocal context */ 
typedef struct _VOCALCUTContext
{
    int16_t 		pcm_low[128 * 2];
    int16_t 		pcm_high[128 * 2];
    int16_t 		pcm_band[128 * 2];
        
    EQContext  		eq1;
    EQContext  		eq2;
    EQContext  		eq3;
	EQContext  		eq4;

    EQFilterParams  filter_params1[MAX_FILTER_COUNT];
    EQFilterParams  filter_params2[MAX_FILTER_COUNT];
    EQFilterParams  filter_params3[MAX_FILTER_COUNT];
	EQFilterParams  filter_params4[MAX_FILTER_COUNT];

} VocalCutContext;


/**
 * @brief  Initialize vocal cut module
 * @param  ct Pointer to a VocalCutContext object.
 * @param  samplerate sample rate
 * @return error code. VOCALCUT_ERROR_OK means successful, other codes indicate error.
 * @note   Input signals should be always stereo format.
 */
int32_t vocal_cut_init(VocalCutContext *ct, int32_t samplerate);


/**
 * @brief  Apply vocal cut effect to a frame of PCM data.
 * @param  pcm_in Address of the PCM input. The PCM layout must be stereo format : L0, R0, L1, R1, L2, R2, ... 
 * @param  pcm_out Address of the PCM output. The PCM layout is stereo format, stereo: L0, R0, L1, R1, L2, R2, ... 
 *         pcm_out can be the same as pcm_in. In this case, the PCM signals are changed in-place.
 * @param  samples Number of PCM samples to process
 * @param  wetdrymix The ratio of wet(vocal cut) signal to the mixed output (wet+dry). Range: 0~100 for 0~100%.
 * @return error code. VOCALCUT_ERROR_OK means successful, other codes indicate error.
 * @note   Input signals should be always stereo format.
 */
int32_t vocal_cut_apply(VocalCutContext *ct, int16_t* pcm_in, int16_t* pcm_out, int32_t samples, int32_t wetdrymix);



#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__VOCALCUT_H__
