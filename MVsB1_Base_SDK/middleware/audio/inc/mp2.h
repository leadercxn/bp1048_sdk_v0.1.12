#ifndef __MP2_H__
#define __MP2_H__
#include <stdint.h>
/* max compressed frame size */
#define MPA_MAX_CODED_FRAME_SIZE 626
	
#define MPA_MAX_CHANNELS 1
	
#define SBLIMIT 32 /* number of subbands */
	
#define MPA_STEREO  0
#define MPA_JSTEREO 1
#define MPA_DUAL    2
#define MPA_MONO    3
	

#define FRAC_BITS   15   /* fractional bits for sb_samples and dct */
#define WFRAC_BITS  14   /* fractional bits for window */

#define FRAC_ONE    (1 << FRAC_BITS)

#define MULL(a,b)	(((int64_t)(a) * (int64_t)(b)) >> FRAC_BITS)
#define MUL64(a,b)	((int64_t)(a) * (int64_t)(b))

/* WARNING: only correct for positive numbers */
#define FIXR(a)		((int)((a) * FRAC_ONE + 0.5))

typedef struct MPADecodeContext 
{	
    int frame_size;
    int error_protection;
    int layer;
    int sample_rate;
    int sample_rate_index; /* between 0 and 8 */
    int bit_rate;
    int nb_channels;
    int mode;
    int mode_ext;
    int lsf;

    int16_t synth_buf[MPA_MAX_CHANNELS][512 * 2];
    int synth_buf_offset[MPA_MAX_CHANNELS];
    int32_t sb_samples[MPA_MAX_CHANNELS][36][SBLIMIT];

} MPADecodeContext;


#endif
