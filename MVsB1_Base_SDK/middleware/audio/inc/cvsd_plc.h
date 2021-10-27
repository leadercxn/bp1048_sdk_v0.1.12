/**
 *************************************************************************************
 * @file	cvsd_plc.h
 * @brief	Packet Loss Concealment (PLC) for CVSD
 *
 * @author	Zhao Ying (Alfred)
 * @version	V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *
 *************************************************************************************
 */

#ifndef _CVSD_PLC_H
#define _CVSD_PLC_H

#include <stdint.h>

#if defined __cplusplus
extern "C" {
#endif


#define CVSD_FS 60           /* CVSD Frame Size */
#define CVSD_N 128           /* 16ms - Window Length for pattern matching */ 
#define CVSD_M 32            /* Template for matching */
#define CVSD_LHIST (CVSD_N+CVSD_FS-1)  /* Length of history buffer required */ 
#define CVSD_RT 12        /*  Reconvergence Time (samples) */
#define CVSD_OLAL 40         /* OverLap-Add Length (samples) */

/* PLC State Information */
typedef struct _CVSD_PLC_State {
    int16_t hist[CVSD_LHIST+CVSD_FS+CVSD_RT+CVSD_OLAL];
    int16_t bestlag;
    int     nbf;

    // summary of processed good and bad frames
    int good_frames_nr;
    int bad_frames_nr;
    int frame_count;
} CVSD_PLC_State;


/**
 * @brief Initialize Packet Loss Concealment (PLC) module
 * @param plc_state Pointer to a CVSD_PLC_State object. 
 * @return None.
 */
void cvsd_plc_init(CVSD_PLC_State *plc_state);


/**
 * @brief Generate output for a bad frame.
 * @param plc_state Pointer to a CVSD_PLC_State object.
 * @param out PCM output buffer¡£
 * @return None.
 * @note the output buffer should be able to hold CVSD_FS samples.
 */
void cvsd_plc_bad_frame(CVSD_PLC_State *plc_state, int16_t *out); 


/**
 * @brief Generate output for a good frame.
 * @param plc_state Pointer to a CVSD_PLC_State object.
 * @param in PCM input buffer.
 * @param out PCM output buffer. The input & output buffer can be the same, i.e. out==in. In this case, the PCM data is changed in-place.
 * @return None.
 * @note the input/output buffer should be able to hold CVSD_FS samples.
 */
void cvsd_plc_good_frame(CVSD_PLC_State *plc_state, int16_t *in, int16_t *out);


/**
 * @brief Generate output for a frame automatically.
 * @param plc_state Pointer to a CVSD_PLC_State object.
 * @param in PCM input buffer.
 * @param out PCM output buffer. The input & output buffer can be the same, i.e. out==in. In this case, the PCM data is changed in-place.
 * @return None.
 * @note the input/output buffer should be able to hold CVSD_FS samples.
 */
void cvsd_plc_process_frame(CVSD_PLC_State *plc_state, int16_t *in, int16_t *out);


#if defined __cplusplus
}
#endif

#endif // _CVSD_PLC_H