//#include "common/type.h"
//#include "common/debug.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "debug.h"
#include "bits.h"
#include "mp2.h"
#include "mp2dec.h"
#include "mp2dectab.h"

MPADecodeContext *pMpaDecContext;
#define priv_data				(*pMpaDecContext)



static const int16_t scale_factor_modshift[64] =
{
	 0,  1,  2,  4,  5,  6,  8,  9, 10, 12, 13, 14, 16, 17, 18, 
	20, 21, 22, 24, 25, 26, 28, 29, 30, 32, 33, 34, 36, 37, 38, 
	40, 41, 42, 44, 45, 46, 48, 49,	50, 52, 53, 54, 56, 57, 58, 
	60, 61, 62, 64, 65, 66, 68, 69, 70, 72, 73, 74, 76, 77, 78, 
	80, 81, 82, 84
};

static const int32_t scale_factor_mult[15][3] =
{
	{87380, 69353, 55045},
	{74898, 59446, 47182},
	{69904, 55482, 44036},
	{67650, 53693, 42616},
	{66576, 52841, 41940},

	{66052, 52425, 41610},
	{65792, 52219, 41446},
	{65664, 52117, 41365},
	{65600, 52066, 41325},
	{65568, 52041, 41305},

	{65552, 52028, 41295},
	{65544, 52022, 41290},
	{65540, 52019, 41287},
	{65538, 52017, 41286},
	{-65536, -52016, -41285},
};

#define SCALE_GEN(v) \
{ FIXR(1.0 * (v)), FIXR(0.7937005259 * (v)), FIXR(0.6299605249 * (v)) }

static int32_t scale_factor_mult2[3][3] =
{
    SCALE_GEN(4.0 / 3.0), /* 3 steps */
    SCALE_GEN(4.0 / 5.0), /* 5 steps */
    SCALE_GEN(4.0 / 9.0), /* 9 steps */
};


static const int16_t window[512] =
{
	     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	     0,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -2,     -2,
	    -2,     -2,     -2,     -3,     -3,     -3,     -4,     -4,     -5,     -5,
	    -6,     -6,     -7,     -8,     -9,     -9,    -10,    -11,    -12,    -13,
	   -14,    -16,    -17,    -18,    -20,    -21,    -23,    -24,    -26,    -28,
	   -29,    -31,    -33,    -35,    -37,    -38,    -40,    -42,    -44,    -46,
	   -47,    -49,    -50,    -52,     53,     55,     56,     56,     57,     57,
	    57,     57,     56,     55,     54,     52,     50,     47,     44,     41,
	    37,     32,     27,     21,     14,      7,      0,     -9,    -18,    -28,
	   -38,    -49,    -61,    -73,    -87,   -100,   -115,   -130,   -145,   -161,
	  -178,	  -195,	  -212,	  -230,	  -248,	  -266,	  -284,	  -302,	  -321,	  -339,
	  -357,	  -374,	  -392,	  -408,	  -424,	  -440,	  -454,	  -467,   -480,	  -490,
	  -500,	  -508,	  -514,	  -519,	  -521,	  -522,	  -520,	  -516,	   509,	   500,
	   488,	   473,	   456,	   435,	   411,	   384,	   354,	   320,	   283,	   243,
	   199,	   151,	   101,	    46,	   -11,	   -72,	  -136,	  -203,	  -274,	  -347,
	  -423,	  -501,	  -582,	  -666,	  -751,	  -838,	  -926,	 -1016,	 -1106,	 -1197,
	 -1288,	 -1379,	 -1470,	 -1559,	 -1647,	 -1734,	 -1818,	 -1899,	 -1977,	 -2052,
	 -2123,	 -2189,	 -2249,	 -2305,	 -2354,	 -2396,	 -2432,	 -2459,	 -2479,	 -2490,
	 -2491,	 -2484,	 -2466,	 -2437,	 -2398,	 -2347,	 -2285,	 -2210,	 -2123,	 -2023,
	 -1910,	 -1783,	  1644,	  1490,	  1322,	  1140,	   944,	   734,	   509,	   271,
	    18,	  -249,	  -530,	  -825,	 -1133,	 -1454,	 -1788,	 -2135,	 -2494,	 -2864,
	 -3245,	 -3637,	 -4039,	 -4450,	 -4869,	 -5297,	 -5732,	 -6173,	 -6620,	 -7072,
	 -7528,	 -7987,	 -8448,	 -8910,	 -9372,	 -9834,	-10294,	-10751,	-11205,	-11654,
	-12097,	-12534,	-12963,	-13383,	-13794,	-14194,	-14583,	-14959,	-15322,	-15671,
	-16005,	-16322,	-16623,	-16907,	-17173,	-17420,	-17647,	-17855,	-18042,	-18209,
	-18354,	-18477,	-18578,	-18657,	-18714,	-18748,	 18760,	 18748,	 18714,	 18657,
	 18578,	 18477,	 18354,	 18209,  18042,	 17855,	 17647,	 17420,	 17173,	 16907,
	 16623,	 16322,	 16005,	 15671,  15322,	 14959,	 14583,	 14194,	 13794,	 13383,
	 12963,	 12534,	 12097,	 11654,	 11205,	 10751,	 10294,	  9834,	  9372,	  8910,
	  8448,	  7987,	  7528,	  7072,   6620,	  6173,	  5732,	  5297,	  4869,	  4450,
	  4039,	  3637,	  3245,	  2864,	  2494,	  2135,	  1788,	  1454,	  1133,	   825,
	   530,	   249,	   -18,	  -271,	  -509,	  -734,	  -944,	 -1140,	 -1322,	 -1490,
	  1644,	  1783,	  1910,	  2023,	  2123,	  2210,	  2285,	  2347,	  2398,	  2437,
	  2466,	  2484,	  2491,	  2490,	  2479,	  2459,	  2432,	  2396,	  2354,	  2305,
	  2249,	  2189,	  2123,	  2052,	  1977,	  1899,	  1818,	  1734,	  1647,	  1559,
	  1470,	  1379,	  1288,	  1197,	  1106,	  1016,	   926,	   838,	   751,	   666,
	   582,	   501,	   423,	   347,	   274,	   203,	   136,	    72,	    11,	   -46,
	  -101,	  -151,	  -199,	  -243,	  -283,	  -320,	  -354,	  -384,	  -411,	  -435,
	  -456,	  -473,	  -488,	  -500,	   509,	   516,	   520,	   522,	   521,	   519,
	   514,	   508,	   500,	   490,	   480,    467,	   454,	   440,	   424,	   408,
	   392,	   374,	   357,	   339,	   321,    302,	   284,	   266,	   248,	   230,
	   212,	   195,	   178,	   161,	   145,    130,	   115,	   100,	    87,	    73,
	    61,	    49,	    38,	    28,	    18,      9,	     0,	    -7,	   -14,	   -21,
	   -27,	   -32,	   -37,	   -41,	   -44,    -47,	   -50,	   -52,	   -54,	   -55,
	   -56,	   -57,	   -57,	   -57,	   -57,    -56,	   -56,	   -55,	    53,	    52,
	    50,	    49,	    47,	    46,	    44,     42,	    40,	    38,	    37,	    35,
	    33,	    31,	    29,	    28,	    26,     24,	    23,	    21,	    20,	    18,
	    17,	    16,	    14,	    13,	    12,     11,	    10,	     9,	     9,	     8,
	     7,	     6,	     6,	     5,	     5,      4,	     4,	     3,	     3,      3,
	     2,	     2,	     2,	     2,	     2,      1,	     1,	     1,	     1,      1,
	     1,	     1,	     0,	     0,	     0,      0,	     0,	     0,	     0,      0,
	     0,	     0
};


/* layer 1 unscaling */
/* n = number of bits of the mantissa minus 1 */
static int l1_unscale(int n, int mant, int scale_factor)
{
    int shift, mod;
    //signed __int64 val;
    int64_t val;

    shift = scale_factor_modshift[scale_factor];
    mod = shift & 3;
    shift >>= 2;
    val = MUL64(mant + (-1 << n) + 1, scale_factor_mult[n-1][mod]);
    shift += n;
    /* NOTE: at this point, 1 <= shift >= 21 + 15 */
    return (int)((val + (1L << (shift - 1))) >> shift);
}

static int l2_unscale_group(int steps, int mant, int scale_factor)
{
    int shift, mod, val;

    shift = scale_factor_modshift[scale_factor];
    mod = shift & 3;
    shift >>= 2;

    val = (mant - (steps >> 1)) * scale_factor_mult2[steps >> 2][mod];
    /* NOTE: at this point, 0 <= shift <= 21 */
    if (shift > 0)
	{
        val = (val + (1 << (shift - 1))) >> shift;
	}
    return val;
}


#define COS0_0  FIXR(0.50060299823519630134)
#define COS0_1  FIXR(0.50547095989754365998)
#define COS0_2  FIXR(0.51544730992262454697)
#define COS0_3  FIXR(0.53104259108978417447)
#define COS0_4  FIXR(0.55310389603444452782)
#define COS0_5  FIXR(0.58293496820613387367)
#define COS0_6  FIXR(0.62250412303566481615)
#define COS0_7  FIXR(0.67480834145500574602)
#define COS0_8  FIXR(0.74453627100229844977)
#define COS0_9  FIXR(0.83934964541552703873)
#define COS0_10 FIXR(0.97256823786196069369)
#define COS0_11 FIXR(1.16943993343288495515)
#define COS0_12 FIXR(1.48416461631416627724)
#define COS0_13 FIXR(2.05778100995341155085)
#define COS0_14 FIXR(3.40760841846871878570)
#define COS0_15 FIXR(10.19000812354805681150)

#define COS1_0 FIXR(0.50241928618815570551)
#define COS1_1 FIXR(0.52249861493968888062)
#define COS1_2 FIXR(0.56694403481635770368)
#define COS1_3 FIXR(0.64682178335999012954)
#define COS1_4 FIXR(0.78815462345125022473)
#define COS1_5 FIXR(1.06067768599034747134)
#define COS1_6 FIXR(1.72244709823833392782)
#define COS1_7 FIXR(5.10114861868916385802)

#define COS2_0 FIXR(0.50979557910415916894)
#define COS2_1 FIXR(0.60134488693504528054)
#define COS2_2 FIXR(0.89997622313641570463)
#define COS2_3 FIXR(2.56291544774150617881)

#define COS3_0 FIXR(0.54119610014619698439)
#define COS3_1 FIXR(1.30656296487637652785)

#define COS4_0 FIXR(0.70710678118654752439)

/* butterfly operator */
#define BF(a, b, c)\
{\
    tmp0 = tab[a] + tab[b];\
    tmp1 = tab[a] - tab[b];\
    tab[a] = tmp0;\
    tab[b] = MULL(tmp1, c);\
}

#define BF1(a, b, c, d)\
{\
    BF(a, b, COS4_0);\
    BF(c, d, -COS4_0);\
    tab[c] += tab[d];\
}

#define BF2(a, b, c, d)\
{\
    BF(a, b, COS4_0);\
    BF(c, d, -COS4_0);\
    tab[c] += tab[d];\
    tab[a] += tab[c];\
    tab[c] += tab[b];\
    tab[b] += tab[d];\
}

#define ADD(a, b) tab[a] += tab[b]

/* DCT32 without 1/sqrt(2) coef zero scaling. */
static void dct32(int32_t *out, int32_t *tab)
{
    int tmp0, tmp1;

    /* pass 1 */
    BF(0, 31, COS0_0);
    BF(1, 30, COS0_1);
    BF(2, 29, COS0_2);
    BF(3, 28, COS0_3);
    BF(4, 27, COS0_4);
    BF(5, 26, COS0_5);
    BF(6, 25, COS0_6);
    BF(7, 24, COS0_7);
    BF(8, 23, COS0_8);
    BF(9, 22, COS0_9);
    BF(10, 21, COS0_10);
    BF(11, 20, COS0_11);
    BF(12, 19, COS0_12);
    BF(13, 18, COS0_13);
    BF(14, 17, COS0_14);
    BF(15, 16, COS0_15);

    /* pass 2 */
    BF(0, 15, COS1_0);
    BF(1, 14, COS1_1);
    BF(2, 13, COS1_2);
    BF(3, 12, COS1_3);
    BF(4, 11, COS1_4);
    BF(5, 10, COS1_5);
    BF(6,  9, COS1_6);
    BF(7,  8, COS1_7);
    
    BF(16, 31, -COS1_0);
    BF(17, 30, -COS1_1);
    BF(18, 29, -COS1_2);
    BF(19, 28, -COS1_3);
    BF(20, 27, -COS1_4);
    BF(21, 26, -COS1_5);
    BF(22, 25, -COS1_6);
    BF(23, 24, -COS1_7);
    
    /* pass 3 */
    BF(0, 7, COS2_0);
    BF(1, 6, COS2_1);
    BF(2, 5, COS2_2);
    BF(3, 4, COS2_3);
    
    BF(8, 15, -COS2_0);
    BF(9, 14, -COS2_1);
    BF(10, 13, -COS2_2);
    BF(11, 12, -COS2_3);
    
    BF(16, 23, COS2_0);
    BF(17, 22, COS2_1);
    BF(18, 21, COS2_2);
    BF(19, 20, COS2_3);
    
    BF(24, 31, -COS2_0);
    BF(25, 30, -COS2_1);
    BF(26, 29, -COS2_2);
    BF(27, 28, -COS2_3);

    /* pass 4 */
    BF(0, 3, COS3_0);
    BF(1, 2, COS3_1);
    
    BF(4, 7, -COS3_0);
    BF(5, 6, -COS3_1);
    
    BF(8, 11, COS3_0);
    BF(9, 10, COS3_1);
    
    BF(12, 15, -COS3_0);
    BF(13, 14, -COS3_1);
    
    BF(16, 19, COS3_0);
    BF(17, 18, COS3_1);
    
    BF(20, 23, -COS3_0);
    BF(21, 22, -COS3_1);
    
    BF(24, 27, COS3_0);
    BF(25, 26, COS3_1);
    
    BF(28, 31, -COS3_0);
    BF(29, 30, -COS3_1);
    
    /* pass 5 */
    BF1(0, 1, 2, 3);
    BF2(4, 5, 6, 7);
    BF1(8, 9, 10, 11);
    BF2(12, 13, 14, 15);
    BF1(16, 17, 18, 19);
    BF2(20, 21, 22, 23);
    BF1(24, 25, 26, 27);
    BF2(28, 29, 30, 31);
    
    /* pass 6 */
    
    ADD( 8, 12);
    ADD(12, 10);
    ADD(10, 14);
    ADD(14,  9);
    ADD( 9, 13);
    ADD(13, 11);
    ADD(11, 15);

    out[ 0] = tab[0];
    out[16] = tab[1];
    out[ 8] = tab[2];
    out[24] = tab[3];
    out[ 4] = tab[4];
    out[20] = tab[5];
    out[12] = tab[6];
    out[28] = tab[7];
    out[ 2] = tab[8];
    out[18] = tab[9];
    out[10] = tab[10];
    out[26] = tab[11];
    out[ 6] = tab[12];
    out[22] = tab[13];
    out[14] = tab[14];
    out[30] = tab[15];
    
    ADD(24, 28);
    ADD(28, 26);
    ADD(26, 30);
    ADD(30, 25);
    ADD(25, 29);
    ADD(29, 27);
    ADD(27, 31);

    out[ 1] = tab[16] + tab[24];
    out[17] = tab[17] + tab[25];
    out[ 9] = tab[18] + tab[26];
    out[25] = tab[19] + tab[27];
    out[ 5] = tab[20] + tab[28];
    out[21] = tab[21] + tab[29];
    out[13] = tab[22] + tab[30];
    out[29] = tab[23] + tab[31];
    out[ 3] = tab[24] + tab[20];
    out[19] = tab[25] + tab[21];
    out[11] = tab[26] + tab[22];
    out[27] = tab[27] + tab[23];
    out[ 7] = tab[28] + tab[18];
    out[23] = tab[29] + tab[19];
    out[15] = tab[30] + tab[17];
    out[31] = tab[31];
}

#define OUT_SHIFT (WFRAC_BITS + FRAC_BITS - 15)


static int round_sample(int sum)
{
    int sum1;

    sum1 = (sum + (1 << (OUT_SHIFT - 1))) >> OUT_SHIFT;
    if (sum1 < -32768)
	{
        sum1 = -32768;
	}
    else if (sum1 > 32767)
	{
        sum1 = 32767;
	}
    return sum1;
}

/* signed 16x16 -> 32 multiply */
#define MULS(ra, rb) ((ra) * (rb))


#define SUM8(sum, op, w, p) \
{                                               \
    sum op MULS((w)[0 * 64], p[0 * 64]);\
    sum op MULS((w)[1 * 64], p[1 * 64]);\
    sum op MULS((w)[2 * 64], p[2 * 64]);\
    sum op MULS((w)[3 * 64], p[3 * 64]);\
    sum op MULS((w)[4 * 64], p[4 * 64]);\
    sum op MULS((w)[5 * 64], p[5 * 64]);\
    sum op MULS((w)[6 * 64], p[6 * 64]);\
    sum op MULS((w)[7 * 64], p[7 * 64]);\
}

#define SUM8P2(sum1, op1, sum2, op2, w1, w2, p) \
{                                               \
    int tmp;\
    tmp = p[0 * 64];\
    sum1 op1 MULS((w1)[0 * 64], tmp);\
    sum2 op2 MULS((w2)[0 * 64], tmp);\
    tmp = p[1 * 64];\
    sum1 op1 MULS((w1)[1 * 64], tmp);\
    sum2 op2 MULS((w2)[1 * 64], tmp);\
    tmp = p[2 * 64];\
    sum1 op1 MULS((w1)[2 * 64], tmp);\
    sum2 op2 MULS((w2)[2 * 64], tmp);\
    tmp = p[3 * 64];\
    sum1 op1 MULS((w1)[3 * 64], tmp);\
    sum2 op2 MULS((w2)[3 * 64], tmp);\
    tmp = p[4 * 64];\
    sum1 op1 MULS((w1)[4 * 64], tmp);\
    sum2 op2 MULS((w2)[4 * 64], tmp);\
    tmp = p[5 * 64];\
    sum1 op1 MULS((w1)[5 * 64], tmp);\
    sum2 op2 MULS((w2)[5 * 64], tmp);\
    tmp = p[6 * 64];\
    sum1 op1 MULS((w1)[6 * 64], tmp);\
    sum2 op2 MULS((w2)[6 * 64], tmp);\
    tmp = p[7 * 64];\
    sum1 op1 MULS((w1)[7 * 64], tmp);\
    sum2 op2 MULS((w2)[7 * 64], tmp);\
}


/* 32 sub band synthesis filter. Input: 32 sub band samples, Output:
   32 samples. */
/* XXX: optimize by avoiding ring buffer usage */
static void synth_filter(int ch, int16_t *samples, int incr, int32_t sb_samples[SBLIMIT])
{
	int32_t tmp[32];
	int16_t *synth_buf;
    const int16_t *w, *w2, *p;
    int j, offset, v;
    int16_t *samples2;
    int sum, sum2;
    
//	DBG(("001\n"));
	//1.4ms
	dct32(tmp, sb_samples);
    
//    DBG(("002\n"));
	offset = priv_data.synth_buf_offset[ch];
    synth_buf = priv_data.synth_buf[ch] + offset;

//	DBG(("003\n"));
//	DBG(("offset: %d\n", offset));
	
	//0.3ms
    for(j=0;j<32;j++) 
	{
        v = tmp[j];
        /* NOTE: can cause a loss in precision if very high amplitude
           sound */
        if (v > 32767)
		{
            v = 32767;
		}
        else if (v < -32768)
		{
            v = -32768;
		}
        synth_buf[j] = v;
    }
//    DBG(("004\n"));
	
	/* copy to avoid wrap */
    memcpy(synth_buf + 512, synth_buf, 32 * sizeof(int16_t));

//	DBG(("005\n"));

    samples2 = samples + 31 * incr;
    w = window;
    w2 = window + 31;

    sum = 0;
    p = synth_buf + 16;
    SUM8(sum, +=, w, p);
    p = synth_buf + 48;
    SUM8(sum, -=, w + 32, p);
    *samples = round_sample(sum);
    samples += incr;
    w++;

    /* we calculate two samples at the same time to avoid one memory
       access per two sample */
    //2.7ms
    for(j=1;j<16;j++) 
	{
        sum = 0;
        sum2 = 0;
        p = synth_buf + 16 + j;
        SUM8P2(sum, +=, sum2, -=, w, w2, p);
        p = synth_buf + 48 - j;
        SUM8P2(sum, -=, sum2, -=, w + 32, w2 + 32, p);

        *samples = round_sample(sum);
        samples += incr;
        *samples2 = round_sample(sum2);
        samples2 -= incr;
        w++;
        w2--;
    }
    
    p = synth_buf + 32;
    sum = 0;
    SUM8(sum, -=, w + 32, p);
    *samples = round_sample(sum);

//	DBG(("offset 2: %d\n", offset));
    offset = (offset - 32) & 511;
//	DBG(("offset 3: %d\n", offset));

    priv_data.synth_buf_offset[ch] = offset;
//	DBG(("offset 4: %d  %d\n", offset, priv_data.synth_buf_offset[ch]));
}


/* header decoding. MUST check the header before because no
   consistency check is done there. Return 1 if free format found and
   that the frame size must be computed externally */
static bool decode_header(uint32_t header)
{
    int sample_rate, frame_size, mpeg25, padding;
    int sample_rate_index, bitrate_index;


	/* header */
    if ((header & 0xffe00000) != 0xffe00000)
	{
		return FALSE;
	}

    /* layer check */
    if (((header >> 17) & 3) == 0)
	{
		return FALSE;
	}

    /* bit rate */
    if (((header >> 12) & 0xf) == 0xf)
	{
		return FALSE;
	}

    /* frequency */
    if (((header >> 10) & 3) == 3)
	{
		return FALSE;
	}


    if (header & (1<<20)) 
	{
        priv_data.lsf = (header & (1<<19)) ? 0 : 1;
        mpeg25 = 0;
    } 
	else 
	{
        priv_data.lsf = 1;
        mpeg25 = 1;
    }
    
    priv_data.layer = 4 - ((header >> 17) & 3);
    /* extract frequency */
    sample_rate_index = (header >> 10) & 3;
    sample_rate = mpa_freq_tab[sample_rate_index] >> (priv_data.lsf + mpeg25);
    sample_rate_index += 3 * (priv_data.lsf + mpeg25);
    priv_data.sample_rate_index = sample_rate_index;
    priv_data.error_protection = ((header >> 16) & 1) ^ 1;
    priv_data.sample_rate = sample_rate;

    bitrate_index = (header >> 12) & 0xf;
    padding = (header >> 9) & 1;
    priv_data.mode = (header >> 6) & 3;
    priv_data.mode_ext = (header >> 4) & 3;

    if (priv_data.mode == MPA_MONO)
	{
        priv_data.nb_channels = 1;
	}
    else
	{
        priv_data.nb_channels = 2;
	}
    
    if (bitrate_index != 0) 
	{
        frame_size = mpa_bitrate_tab[priv_data.lsf][priv_data.layer - 1][bitrate_index];
        priv_data.bit_rate = frame_size * 1000;
        switch(priv_data.layer) 
		{
        case 1:
            frame_size = (frame_size * 12000) / sample_rate;
            frame_size = (frame_size + padding) * 4;
            break;
        case 2:
            frame_size = (frame_size * 144000) / sample_rate;
            frame_size += padding;
            break;
        default:
        case 3:
            frame_size = (frame_size * 144000) / (sample_rate << priv_data.lsf);
            frame_size += padding;
            break;
        }
        priv_data.frame_size = frame_size;
    } 
	else 
	{
		return FALSE;
    }
    
    return TRUE;
}


/* bitrate is in kb/s */
int l2_select_table(int bitrate, int nb_channels, int freq, int lsf)
{
    int ch_bitrate, table;
    
    ch_bitrate = bitrate / nb_channels;
    if (!lsf) 
	{
        if ((freq == 48000 && ch_bitrate >= 56) || (ch_bitrate >= 56 && ch_bitrate <= 80)) 
		{
            table = 0;
		}
        else if (freq != 48000 && ch_bitrate >= 96)
		{
            table = 1;
		}
        else if (freq != 32000 && ch_bitrate <= 48) 
		{
            table = 2;
		}
        else 
		{
            table = 3;
		}
    } 
	else 
	{
        table = 4;
    }
    return table;
}

unsigned char bit_alloc[MPA_MAX_CHANNELS][SBLIMIT];
unsigned char scale_code[MPA_MAX_CHANNELS][SBLIMIT];
unsigned char scale_factors[MPA_MAX_CHANNELS][SBLIMIT][3], *sf;
static int mp_decode_layer2(void)
{
    int sblimit; /* number of used subbands */
    const unsigned char *alloc_table;
    int table, bit_alloc_bits, i, j, ch, bound, v;
    int scale, qindex, bits, steps, k, l, m, b;

    /* select decoding table */
    table = l2_select_table(priv_data.bit_rate / 1000, priv_data.nb_channels, priv_data.sample_rate, priv_data.lsf);
    sblimit = sblimit_table[table];
    alloc_table = alloc_tables[table];

    if (priv_data.mode == MPA_JSTEREO) 
	{
        bound = (priv_data.mode_ext + 1) * 4;
	}
    else
	{
        bound = sblimit;
	}

    //printf("bound=%d sblimit=%d\n", bound, sblimit);

    /* sanity check */
    if( bound > sblimit )
	{
		bound = sblimit;
	}

    /* parse bit allocation */
    j = 0;
    for(i=0;i<bound;i++) 
	{
        bit_alloc_bits = alloc_table[j];
        for(ch=0;ch<priv_data.nb_channels;ch++) 
		{
            bit_alloc[ch][i] = get_bits(bit_alloc_bits);
        }
        j += 1 << bit_alloc_bits;
    }
    for(i=bound;i<sblimit;i++) 
	{
        bit_alloc_bits = alloc_table[j];
        v = get_bits(bit_alloc_bits);
        bit_alloc[0][i] = v;
        bit_alloc[1][i] = v;
        j += 1 << bit_alloc_bits;
    }

    /* scale codes */
    for(i=0;i<sblimit;i++) 
	{
        for(ch=0;ch<priv_data.nb_channels;ch++) 
		{
            if (bit_alloc[ch][i]) 
			{
                scale_code[ch][i] = get_bits(2);
			}
        }
    }
    
    /* scale factors */
    for(i=0;i<sblimit;i++) 
	{
        for(ch=0;ch<priv_data.nb_channels;ch++) 
		{
            if (bit_alloc[ch][i]) 
			{
                sf = scale_factors[ch][i];
                switch(scale_code[ch][i]) 
				{
                default:
                case 0:
                    sf[0] = get_bits(6);
                    sf[1] = get_bits(6);
                    sf[2] = get_bits(6);
                    break;
                case 2:
                    sf[0] = get_bits(6);
                    sf[1] = sf[0];
                    sf[2] = sf[0];
                    break;
                case 1:
                    sf[0] = get_bits(6);
                    sf[2] = get_bits(6);
                    sf[1] = sf[0];
                    break;
                case 3:
                    sf[0] = get_bits(6);
                    sf[2] = get_bits(6);
                    sf[1] = sf[2];
                    break;
                }
            }
        }
    }

   /* samples */
    for(k=0;k<3;k++) 
	{
        for(l=0;l<12;l+=3) 
		{
            j = 0;
            for(i=0;i<bound;i++) 
			{
                bit_alloc_bits = alloc_table[j];
                for(ch=0;ch<priv_data.nb_channels;ch++) 
				{
                    b = bit_alloc[ch][i];
                    if (b) 
					{
                        scale = scale_factors[ch][i][k];
                        qindex = alloc_table[j+b];
                        bits = quant_bits[qindex];
                        if (bits < 0) 
						{
                            /* 3 values at the same time */
                            v = get_bits(-bits);
                            steps = quant_steps[qindex];
                            priv_data.sb_samples[ch][k * 12 + l + 0][i] = l2_unscale_group(steps, v % steps, scale);
                            v = v / steps;
                            priv_data.sb_samples[ch][k * 12 + l + 1][i] = l2_unscale_group(steps, v % steps, scale);
                            v = v / steps;
                            priv_data.sb_samples[ch][k * 12 + l + 2][i] = l2_unscale_group(steps, v, scale);
                        } 
						else 
						{
                            for(m=0;m<3;m++) 
							{
                                v = get_bits(bits);
                                v = l1_unscale(bits - 1, v, scale);
                                priv_data.sb_samples[ch][k * 12 + l + m][i] = v;
                            }
                        }
                    } 
					else 
					{
                        priv_data.sb_samples[ch][k * 12 + l + 0][i] = 0;
                        priv_data.sb_samples[ch][k * 12 + l + 1][i] = 0;
                        priv_data.sb_samples[ch][k * 12 + l + 2][i] = 0;
                    }
                }
                /* next subband in alloc table */
                j += 1 << bit_alloc_bits; 
            }
            /* XXX: find a way to avoid this duplication of code */
            for(i=bound;i<sblimit;i++) 
			{
                bit_alloc_bits = alloc_table[j];
                b = bit_alloc[0][i];
                if (b) 
				{
                    int mant, scale0, scale1;
                    scale0 = scale_factors[0][i][k];
                    scale1 = scale_factors[1][i][k];
                    qindex = alloc_table[j+b];
                    bits = quant_bits[qindex];
                    if (bits < 0) 
					{
                        /* 3 values at the same time */
                        v = get_bits(-bits);
                        steps = quant_steps[qindex];
                        mant = v % steps;
                        v = v / steps;
                        priv_data.sb_samples[0][k * 12 + l + 0][i] = l2_unscale_group(steps, mant, scale0);
                        priv_data.sb_samples[1][k * 12 + l + 0][i] = l2_unscale_group(steps, mant, scale1);
                        mant = v % steps;
                        v = v / steps;
                        priv_data.sb_samples[0][k * 12 + l + 1][i] = l2_unscale_group(steps, mant, scale0);
                        priv_data.sb_samples[1][k * 12 + l + 1][i] = l2_unscale_group(steps, mant, scale1);
                        priv_data.sb_samples[0][k * 12 + l + 2][i] = l2_unscale_group(steps, v, scale0);
                        priv_data.sb_samples[1][k * 12 + l + 2][i] = l2_unscale_group(steps, v, scale1);
                    } 
					else 
					{
                        for(m=0;m<3;m++) 
						{
                            mant = get_bits(bits);
                            priv_data.sb_samples[0][k * 12 + l + m][i] = l1_unscale(bits - 1, mant, scale0);
                            priv_data.sb_samples[1][k * 12 + l + m][i] = l1_unscale(bits - 1, mant, scale1);
                        }
                    }
                } 
				else 
				{
                    priv_data.sb_samples[0][k * 12 + l + 0][i] = 0;
                    priv_data.sb_samples[0][k * 12 + l + 1][i] = 0;
                    priv_data.sb_samples[0][k * 12 + l + 2][i] = 0;
                    priv_data.sb_samples[1][k * 12 + l + 0][i] = 0;
                    priv_data.sb_samples[1][k * 12 + l + 1][i] = 0;
                    priv_data.sb_samples[1][k * 12 + l + 2][i] = 0;
                }
                /* next subband in alloc table */
                j += 1 << bit_alloc_bits; 
            }
            /* fill remaining samples to zero */
            for(i=sblimit;i<SBLIMIT;i++) 
			{
                for(ch=0;ch<priv_data.nb_channels;ch++) 
				{
                    priv_data.sb_samples[ch][k * 12 + l + 0][i] = 0;
                    priv_data.sb_samples[ch][k * 12 + l + 1][i] = 0;
                    priv_data.sb_samples[ch][k * 12 + l + 2][i] = 0;
                }
            }
        }
    }
    return 3 * 12;
}


static int mp_decode_frame(short *samples)
{
    int i, nb_frames, ch;
    short *samples_ptr;
    
    nb_frames = 0;
    /* skip error protection field */
    if (priv_data.error_protection)
	{
        get_bits(16);
	}

    //printf("frame %d:\n", s->frame_count);
    switch(priv_data.layer) 
	{
    case 2:
        nb_frames = mp_decode_layer2();
        break;
    }
    /* apply the synthesis filter */
    for(ch=0;ch<priv_data.nb_channels;ch++) 
	{
        samples_ptr = samples + ch;
        for(i=0;i<nb_frames;i++) 
		{
            //DBG(("ch: %d, i: %d\n", ch, i));
			synth_filter(ch, samples_ptr, priv_data.nb_channels, priv_data.sb_samples[ch][i]);
            samples_ptr += 32 * priv_data.nb_channels;
        }
    }
    return nb_frames * 32 * sizeof(short) * priv_data.nb_channels;
}

bool MP2_decode_frame(void* PcmData, uint8_t* Mp2Data)
{
	//查找合法的帧头，本应用中可以不用查找，每次传进来的就恰好是一帧	
	if(!decode_header((Mp2Data[0] << 24) | (Mp2Data[1] << 16) | (Mp2Data[2] << 8) | Mp2Data[3])) 
	{
		DBG("decode_header() error!\n");
		return FALSE;
	}
	
	//解码
	init_get_bits(&Mp2Data[4]);
	mp_decode_frame((short*)PcmData);
	return TRUE;
}


void MP2_decode_init(void* MemAddr)
{	
	pMpaDecContext = MemAddr;
	memset(&priv_data, 0, sizeof(MPADecodeContext));
	//DBG("sizeof(MPADecodeContext): %d\n", (uint16_t)sizeof(MPADecodeContext));
}
