/**
 **************************************************************************************
 * @file    fft_api.c
 * @brief   fft_api
 *
 * @author  Castle
 * @version V1.0.0
 *
 * $Created: 2018-04-24 16:15:00$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <math.h>
#include <nds32_intrinsic.h>
#include "type.h"
#include "fft.h"
#include "fft_api.h"


#define USING_HW_FFT

#define ACC_PI                                  (3.1415926535)
#define BFLOOR_Q1d31(x)                         __nds32__clips((x), 31)
#define BFLOOR_Q1d15(x)                         __nds32__clips((x), 15)

#define ACC_FFT_MAX_SIZE        (2048)
#define ACC_COS_TAB_FRAC_BITS   (15)

enum CFFT_TYPE
{
    CFFT  = 0,
    CIFFT = 1,
};

enum RFFT_TYPE
{
    RFFT  = 0,
    RIFFT = 1,
};

typedef struct _ComplexData
{
    int32_t real;
    int32_t imag;
}ComplexData;

int32_t rfft_sw(uint32_t Type, int32_t* DataInOut, uint32_t DataCount);
int32_t cfft_sw(uint32_t Type, int32_t* DataInOut, uint32_t DataCount);
void DataCacheRangeInvalid(uint32_t start, uint32_t size);

const static int32_t COS1024_TAB[] =
{
    0x8000, 0x7FFF, 0x7FFF, 0x7FFE, 0x7FFD, 0x7FFC, 0x7FFA, 0x7FF8, 0x7FF6, 0x7FF3, 0x7FF0, 0x7FED, 0x7FE9, 0x7FE5, 0x7FE1, 0x7FDD,
    0x7FD8, 0x7FD3, 0x7FCE, 0x7FC8, 0x7FC2, 0x7FBC, 0x7FB5, 0x7FAE, 0x7FA7, 0x7F9F, 0x7F97, 0x7F8F, 0x7F87, 0x7F7E, 0x7F75, 0x7F6B,
    0x7F62, 0x7F58, 0x7F4D, 0x7F43, 0x7F38, 0x7F2D, 0x7F21, 0x7F15, 0x7F09, 0x7EFD, 0x7EF0, 0x7EE3, 0x7ED5, 0x7EC8, 0x7EBA, 0x7EAB,
    0x7E9D, 0x7E8E, 0x7E7F, 0x7E6F, 0x7E5F, 0x7E4F, 0x7E3F, 0x7E2E, 0x7E1D, 0x7E0C, 0x7DFA, 0x7DE8, 0x7DD6, 0x7DC3, 0x7DB0, 0x7D9D,
    0x7D8A, 0x7D76, 0x7D62, 0x7D4E, 0x7D39, 0x7D24, 0x7D0F, 0x7CF9, 0x7CE3, 0x7CCD, 0x7CB7, 0x7CA0, 0x7C89, 0x7C71, 0x7C5A, 0x7C42,
    0x7C29, 0x7C11, 0x7BF8, 0x7BDF, 0x7BC5, 0x7BAC, 0x7B92, 0x7B77, 0x7B5D, 0x7B42, 0x7B26, 0x7B0B, 0x7AEF, 0x7AD3, 0x7AB6, 0x7A9A,
    0x7A7D, 0x7A5F, 0x7A42, 0x7A24, 0x7A05, 0x79E7, 0x79C8, 0x79A9, 0x798A, 0x796A, 0x794A, 0x792A, 0x7909, 0x78E8, 0x78C7, 0x78A6,
    0x7884, 0x7862, 0x7840, 0x781D, 0x77FA, 0x77D7, 0x77B4, 0x7790, 0x776C, 0x7747, 0x7723, 0x76FE, 0x76D9, 0x76B3, 0x768E, 0x7668,
    0x7641, 0x761B, 0x75F4, 0x75CC, 0x75A5, 0x757D, 0x7555, 0x752D, 0x7504, 0x74DB, 0x74B2, 0x7489, 0x745F, 0x7435, 0x740B, 0x73E0,
    0x73B5, 0x738A, 0x735F, 0x7333, 0x7307, 0x72DB, 0x72AF, 0x7282, 0x7255, 0x7227, 0x71FA, 0x71CC, 0x719E, 0x716F, 0x7141, 0x7112,
    0x70E2, 0x70B3, 0x7083, 0x7053, 0x7023, 0x6FF2, 0x6FC1, 0x6F90, 0x6F5F, 0x6F2D, 0x6EFB, 0x6EC9, 0x6E96, 0x6E63, 0x6E30, 0x6DFD,
    0x6DCA, 0x6D96, 0x6D62, 0x6D2D, 0x6CF9, 0x6CC4, 0x6C8F, 0x6C59, 0x6C24, 0x6BEE, 0x6BB8, 0x6B81, 0x6B4A, 0x6B13, 0x6ADC, 0x6AA5,
    0x6A6D, 0x6A35, 0x69FD, 0x69C4, 0x698C, 0x6953, 0x6919, 0x68E0, 0x68A6, 0x686C, 0x6832, 0x67F7, 0x67BD, 0x6782, 0x6746, 0x670B,
    0x66CF, 0x6693, 0x6657, 0x661A, 0x65DD, 0x65A0, 0x6563, 0x6526, 0x64E8, 0x64AA, 0x646C, 0x642D, 0x63EF, 0x63B0, 0x6371, 0x6331,
    0x62F2, 0x62B2, 0x6271, 0x6231, 0x61F1, 0x61B0, 0x616F, 0x612D, 0x60EC, 0x60AA, 0x6068, 0x6026, 0x5FE3, 0x5FA0, 0x5F5E, 0x5F1A,
    0x5ED7, 0x5E93, 0x5E50, 0x5E0B, 0x5DC7, 0x5D83, 0x5D3E, 0x5CF9, 0x5CB4, 0x5C6E, 0x5C29, 0x5BE3, 0x5B9D, 0x5B56, 0x5B10, 0x5AC9,
    0x5A82, 0x5A3B, 0x59F3, 0x59AC, 0x5964, 0x591C, 0x58D4, 0x588B, 0x5842, 0x57F9, 0x57B0, 0x5767, 0x571D, 0x56D4, 0x568A, 0x5640,
    0x55F5, 0x55AB, 0x5560, 0x5515, 0x54CA, 0x547E, 0x5433, 0x53E7, 0x539B, 0x534E, 0x5302, 0x52B5, 0x5269, 0x521C, 0x51CE, 0x5181,
    0x5133, 0x50E5, 0x5097, 0x5049, 0x4FFB, 0x4FAC, 0x4F5E, 0x4F0F, 0x4EBF, 0x4E70, 0x4E21, 0x4DD1, 0x4D81, 0x4D31, 0x4CE1, 0x4C90,
    0x4C3F, 0x4BEF, 0x4B9E, 0x4B4C, 0x4AFB, 0x4AA9, 0x4A58, 0x4A06, 0x49B4, 0x4961, 0x490F, 0x48BC, 0x4869, 0x4816, 0x47C3, 0x4770,
    0x471C, 0x46C9, 0x4675, 0x4621, 0x45CD, 0x4578, 0x4524, 0x44CF, 0x447A, 0x4425, 0x43D0, 0x437B, 0x4325, 0x42D0, 0x427A, 0x4224,
    0x41CE, 0x4177, 0x4121, 0x40CA, 0x4073, 0x401D, 0x3FC5, 0x3F6E, 0x3F17, 0x3EBF, 0x3E68, 0x3E10, 0x3DB8, 0x3D60, 0x3D07, 0x3CAF,
    0x3C56, 0x3BFD, 0x3BA5, 0x3B4C, 0x3AF2, 0x3A99, 0x3A40, 0x39E6, 0x398C, 0x3932, 0x38D8, 0x387E, 0x3824, 0x37CA, 0x376F, 0x3714,
    0x36BA, 0x365F, 0x3604, 0x35A8, 0x354D, 0x34F2, 0x3496, 0x343A, 0x33DE, 0x3382, 0x3326, 0x32CA, 0x326E, 0x3211, 0x31B5, 0x3158,
    0x30FB, 0x309E, 0x3041, 0x2FE4, 0x2F87, 0x2F29, 0x2ECC, 0x2E6E, 0x2E11, 0x2DB3, 0x2D55, 0x2CF7, 0x2C98, 0x2C3A, 0x2BDC, 0x2B7D,
    0x2B1F, 0x2AC0, 0x2A61, 0x2A02, 0x29A3, 0x2944, 0x28E5, 0x2886, 0x2826, 0x27C7, 0x2767, 0x2707, 0x26A8, 0x2648, 0x25E8, 0x2588,
    0x2528, 0x24C7, 0x2467, 0x2407, 0x23A6, 0x2345, 0x22E5, 0x2284, 0x2223, 0x21C2, 0x2161, 0x2100, 0x209F, 0x203E, 0x1FDC, 0x1F7B,
    0x1F19, 0x1EB8, 0x1E56, 0x1DF5, 0x1D93, 0x1D31, 0x1CCF, 0x1C6D, 0x1C0B, 0x1BA9, 0x1B47, 0x1AE4, 0x1A82, 0x1A20, 0x19BD, 0x195B,
    0x18F8, 0x1896, 0x1833, 0x17D0, 0x176D, 0x170A, 0x16A8, 0x1645, 0x15E2, 0x157F, 0x151B, 0x14B8, 0x1455, 0x13F2, 0x138E, 0x132B,
    0x12C8, 0x1264, 0x1201, 0x119D, 0x1139, 0x10D6, 0x1072, 0x100E, 0x0FAB, 0x0F47, 0x0EE3, 0x0E7F, 0x0E1B, 0x0DB7, 0x0D53, 0x0CEF,
    0x0C8B, 0x0C27, 0x0BC3, 0x0B5F, 0x0AFB, 0x0A97, 0x0A33, 0x09CE, 0x096A, 0x0906, 0x08A2, 0x083D, 0x07D9, 0x0775, 0x0710, 0x06AC,
    0x0647, 0x05E3, 0x057F, 0x051A, 0x04B6, 0x0451, 0x03ED, 0x0388, 0x0324, 0x02BF, 0x025B, 0x01F6, 0x0192, 0x012D, 0x00C9, 0x0064,
    0x0000,
};
	
#define LOG2(x) (log(x)/log(2))
static inline uint8_t __CLZ(uint32_t value)
{
    if(value)
    {
        return 31 - (uint8_t)LOG2(value);
    }
    else
    {
        return 32;
    }
}

static inline uint8_t rfft_get_shift_bits(int n)
{
	uint8_t ret = 0;
	switch(n)
	{
		case 64:
			ret = 5;
			break;
		
		case 128:
			ret = 6;
			break;
		
		case 256:
			ret = 7;
			break;
		
		case 512:
			ret = 8;
			break;
		
		case 1024:
			ret = 9;
			break;
		
		case 2048:
			ret = 10;
			break;

		default:
			break;
	}

	return ret;
}

static inline uint8_t cfft_get_shift_bits(int n)
{

	uint8_t ret = 0;
	switch(n)
	{
		case 32:
			ret = 5;
			break;
		
		case 64:
			ret = 6;
			break;
		
		case 128:
			ret = 7;
			break;
		
		case 256:
			ret = 8;
			break;
		
		case 512:
			ret = 9;
			break;
		
		case 1024:
			ret = 10;
			break;

		default:
			break;
	}

	return ret;
}


static inline FFT_SIZE fft_get_length_type(int n)
{
	FFT_SIZE fft_size = FFT_SIZE_32;
	switch(n)
	{
		case 32:
			fft_size = FFT_SIZE_32;
			break;
			
		case 64:
			fft_size = FFT_SIZE_64;
			break;
		
		case 128:
			fft_size = FFT_SIZE_128;
			break;
		
		case 256:
			fft_size = FFT_SIZE_256;
			break;
		
		case 512:
			fft_size = FFT_SIZE_512;
			break;
		
		case 1024:
			fft_size = FFT_SIZE_1024;
			break;
		
		case 2048:
			fft_size = FFT_SIZE_2048;
			break;
		
		default:
			break;
			
	}

	return fft_size;
}

int rfft_api(int32_t* x, int data_count, int scale_down)
{
	int shift_bits, i;

	if( scale_down == 0)
	{
		shift_bits = rfft_get_shift_bits(data_count);
		for(i=0; i<data_count; i++)
			x[i] = x[i] << shift_bits;
	}
	
#ifdef USING_HW_FFT
	if(FFT_IsWorking())
		return 0;

	FFT_Init(FFT_TYPE_RFFT, x, fft_get_length_type(data_count));
	FFT_Start();
	while(!FFT_IsDone());
	DataCacheRangeInvalid((uint32_t)x, data_count * sizeof(int32_t));
#else
	rfft_sw(RFFT, x, data_count);
#endif
	return 1;

}

int rifft_api(int32_t* x, int data_count)
{
#ifdef USING_HW_FFT
	if(FFT_IsWorking())
		return 0;
	
	FFT_Init(FFT_TYPE_RIFFT, x, fft_get_length_type(data_count));
	FFT_Start();
	while(!FFT_IsDone());
	DataCacheRangeInvalid((uint32_t)x, data_count * sizeof(int32_t));
#else
	rfft_sw(RIFFT, x, data_count);
#endif
	return 1;
}

int cfft_api(int32_t* x, int data_count, int scale_down)
{
	int shift_bits, i;
	
	if( scale_down == 0)
	{
		shift_bits = cfft_get_shift_bits(data_count);
		for(i=0; i<data_count*2; i++)
			x[i] = x[i] << shift_bits;
	}
#ifdef USING_HW_FFT
	if(FFT_IsWorking())
		return 0;

	FFT_Init(FFT_TYPE_CFFT, x, fft_get_length_type(data_count));
	FFT_Start();
	while(!FFT_IsDone());
	DataCacheRangeInvalid((uint32_t)x, data_count * sizeof(int32_t) * 2);
#else
	cfft_sw(CFFT, x, data_count);
#endif
	return 1;
}

int cifft_api(int32_t* x, int data_count)
{
#ifdef USING_HW_FFT
	if(FFT_IsWorking())
		return 0;
	
	FFT_Init(FFT_TYPE_CIFFT, x, fft_get_length_type(data_count));
	FFT_Start();
	while(!FFT_IsDone());
	DataCacheRangeInvalid((uint32_t)x, data_count * sizeof(int32_t) * 2);
#else
	cfft_sw(CIFFT, x, data_count);
#endif
	return 1;
}


//Radix-2 N points real DFT/IDFT
//Type: 0-RFFT, 1-RIFFT, see #RFFT_TYPE
int32_t rfft_sw(uint32_t Type, int32_t* DataInOut, uint32_t DataCount)
{
    uint32_t k;
    int32_t  r1, r2, i1, i2;
    int32_t  C, S;
    int64_t  temp;

    ComplexData* d = (ComplexData*)DataInOut;

    if(Type == RFFT)
    {
        cfft_sw(CFFT, DataInOut, DataCount / 2);

        for(k = 1; k < DataCount / 4; k++)
        {
            C = COS1024_TAB[k * ACC_FFT_MAX_SIZE / DataCount];
            S = COS1024_TAB[ACC_FFT_MAX_SIZE / 4 - k * ACC_FFT_MAX_SIZE / DataCount];

            r1 = BFLOOR_Q1d31( ((int64_t)d[k].real + (int64_t)d[DataCount / 2 - k].real) >> 1);
            i1 = BFLOOR_Q1d31( ((int64_t)d[k].imag - (int64_t)d[DataCount / 2 - k].imag) >> 1);
            r2 = BFLOOR_Q1d31( ((int64_t)d[k].imag + (int64_t)d[DataCount / 2 - k].imag) >> 1);
            i2 = BFLOOR_Q1d31(-((int64_t)d[k].real - (int64_t)d[DataCount / 2 - k].real) >> 1);

            temp  = BFLOOR_Q1d31(((int64_t)r2 * C + (int64_t)i2 * S) >> ACC_COS_TAB_FRAC_BITS);
            d[k].real                 = BFLOOR_Q1d31((int64_t)r1 + (int64_t)temp);
            d[DataCount / 2 - k].real = BFLOOR_Q1d31((int64_t)r1 - (int64_t)temp);

            temp  = BFLOOR_Q1d31(((int64_t)i2 * C - (int64_t)r2 * S) >> ACC_COS_TAB_FRAC_BITS);
            d[k].imag                 = BFLOOR_Q1d31((int64_t)i1   + (int64_t)temp);
            d[DataCount / 2 - k].imag = BFLOOR_Q1d31((int64_t)temp - (int64_t)i1);
        }

        d[DataCount / 4].real =   d[DataCount / 4].real;
        d[DataCount / 4].imag = - d[DataCount / 4].imag;
        r1 = d[0].real;
        i1 = d[0].imag;
        d[0].real = BFLOOR_Q1d31((int64_t)r1 + (int64_t)i1);
        d[0].imag = BFLOOR_Q1d31((int64_t)r1 - (int64_t)i1);
    }
    else if(Type == RIFFT)
    {
        for(k = 1; k < DataCount / 4; k++)
        {
            C = COS1024_TAB[k * ACC_FFT_MAX_SIZE / DataCount];
            S = COS1024_TAB[ACC_FFT_MAX_SIZE / 4 - k * ACC_FFT_MAX_SIZE / DataCount];

            r1 = BFLOOR_Q1d31( ((int64_t)d[k].real + (int64_t)d[DataCount / 2 - k].real) >> 1);
            i1 = BFLOOR_Q1d31( ((int64_t)d[k].imag - (int64_t)d[DataCount / 2 - k].imag) >> 1);
            r2 = BFLOOR_Q1d31( ((int64_t)d[k].imag + (int64_t)d[DataCount / 2 - k].imag) >> 1);
            i2 = BFLOOR_Q1d31(+((int64_t)d[k].real - (int64_t)d[DataCount / 2 - k].real) >> 1);

            temp  = BFLOOR_Q1d31(((int64_t)r2 * C + (int64_t)i2 * S) >> ACC_COS_TAB_FRAC_BITS);
            d[k].real                 = BFLOOR_Q1d31((int64_t)r1 - (int64_t)temp);
            d[DataCount / 2 - k].real = BFLOOR_Q1d31((int64_t)r1 + (int64_t)temp);

            temp  = BFLOOR_Q1d31(((int64_t)i2 * C - (int64_t)r2 * S) >> ACC_COS_TAB_FRAC_BITS);
            d[k].imag                 = BFLOOR_Q1d31((int64_t)i1   + (int64_t)temp);
            d[DataCount / 2 - k].imag = BFLOOR_Q1d31((int64_t)temp - (int64_t)i1);
        }

        r1 = d[0].real;
        r2 = d[0].imag;
        d[0].real = BFLOOR_Q1d31(((int64_t)r1 + (int64_t)r2) >> 1);
        d[0].imag = BFLOOR_Q1d31(((int64_t)r1 - (int64_t)r2) >> 1);
        d[DataCount / 4].real =  d[DataCount / 4].real;
        d[DataCount / 4].imag = -d[DataCount / 4].imag;

        cfft_sw(CIFFT, DataInOut, DataCount / 2);
    }

    return FFT_OK;
}

//Radix-2 N points complex DFT/IDFT
//Type: 0-CFFT, 1-CIFFT, see #CFFT_TYPE
int32_t cfft_sw(uint32_t Type, int32_t* DataInOut, uint32_t DataCount)
{
    uint32_t i, j, k, M, L, B;
    int32_t  C, S;
    int32_t  idx, offset;
    int32_t  t1, t2;

    ComplexData* d = (ComplexData*)DataInOut;

    M = 32 - 1 - __CLZ(DataCount);

    for(L = 0; L < M; L++)
    {
        for(B = 0; B < (1U << L); B++)
        {
            idx = B * DataCount / (1 << L);

            for(j = 0; j < (DataCount / (1 << L)) / 2; j++)
            {
                offset = idx + j + DataCount / (1 << L) / 2;

                if(j * (1 << L) * 4 > DataCount)
                {
                    C = -COS1024_TAB[ACC_FFT_MAX_SIZE / 2 - j * (1 << L) * ACC_FFT_MAX_SIZE / DataCount];
                    S =  COS1024_TAB[j * (1 << L) * ACC_FFT_MAX_SIZE / DataCount - ACC_FFT_MAX_SIZE / 4];
                }
                else
                {
                    C = COS1024_TAB[j * (1 << L) * ACC_FFT_MAX_SIZE / DataCount];
                    S = COS1024_TAB[ACC_FFT_MAX_SIZE / 4 - j * (1 << L) * ACC_FFT_MAX_SIZE / DataCount];
                }

                t1 = d[idx + j].real;
                t2 = d[offset ].real;
                d[idx + j].real = BFLOOR_Q1d31(((int64_t)t1 + (int64_t)t2) >> 1);
                d[offset ].real = BFLOOR_Q1d31(((int64_t)t1 - (int64_t)t2) >> 1);
                t1 = d[idx + j].imag;
                t2 = d[offset ].imag;
                d[idx + j].imag = BFLOOR_Q1d31(((int64_t)t1 + (int64_t)t2) >> 1);
                d[offset ].imag = BFLOOR_Q1d31(((int64_t)t1 - (int64_t)t2) >> 1);

                t1 = d[offset].real;
                t2 = d[offset].imag;

                if(Type == CFFT)
                {
                    d[offset].real = BFLOOR_Q1d31(((int64_t)t1 * C + (int64_t)t2 * S) >> ACC_COS_TAB_FRAC_BITS);
                    d[offset].imag = BFLOOR_Q1d31(((int64_t)t2 * C - (int64_t)t1 * S) >> ACC_COS_TAB_FRAC_BITS);
                }
                else
                {
                    d[offset].real = BFLOOR_Q1d31(((int64_t)t1 * C - (int64_t)t2 * S) >> ACC_COS_TAB_FRAC_BITS);
                    d[offset].imag = BFLOOR_Q1d31(((int64_t)t2 * C + (int64_t)t1 * S) >> ACC_COS_TAB_FRAC_BITS);
                }
            }
        }
    }

    //bit reversal
    j = 0;
    for(i = 1; i < DataCount - 1; i++)
    {
        k = DataCount / 2;

        while(k <= j)
        {
            j = j - k;
            k = k / 2;
        }

        j = j + k;

        if(i < j)
        {
            t1 = d[j].real;
            t2 = d[j].imag;
            d[j].real = d[i].real;
            d[j].imag = d[i].imag;
            d[i].real = t1;
            d[i].imag = t2;
        }
    }

    return FFT_OK;
}

