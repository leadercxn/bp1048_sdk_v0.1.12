/**
 **************************************************************************************
 * @file    spectrum.c
 * @brief   spectrum interface
 *
 * @author  Castle
 * @version V1.0.0
 *
 * $Created: 2018-01-04 19:17:00$
 *
 * @Copyright (C) 2017, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include "type.h"

#define SPECTRUM_FFT_MAX_LEN 2048
#define SPECTRUM_BIN_MAX_LEN 16

typedef struct _spectrumContext
{
    uint16_t sample_rate;
    uint16_t spt_len;
    uint8_t bin_num;
    uint8_t level_num;
    bool is_init;

    uint16_t bin_frq[SPECTRUM_BIN_MAX_LEN];//the frequency band for every bin
    uint8_t step_table[SPECTRUM_BIN_MAX_LEN];
    int32_t fft_buf[SPECTRUM_FFT_MAX_LEN];
    int32_t hm[SPECTRUM_FFT_MAX_LEN];

}SpectrumContext;

/**
 * @brief Initialization for Spectrum
 * 
 * @param sptct A piece of memory that user provided as context for process Spectrum
 * @param sample_rate pcm sample rate 
 * @param spt_len pcm samples for every Spectrum process, can be 128,256,512..., not larger than SPECTRUM_FFT_MAX_LEN
 * @param bin_num bin numbers for output, supported 16..., not larger than SPECTRUM_BIN_MAX_LEN
 * @return true 
 * @return false 
 */
bool spectrum_init(SpectrumContext *sptct, uint16_t sample_rate, uint16_t spt_len, uint8_t bin_num);

/**
 * @brief Sprctrum processing routine
 * 
 * @param sptct Spectrum Context
 * @param pcm input pcm data, len is spt_len*2 bytes
 * @param bin_out output bin data, len is bin_num*2 bytes. sum of (real)^2+(img)^2 in the current bin
 * @return true 
 * @return false 
 */
bool spectrum_process(SpectrumContext *sptct, int16_t *pcm, uint32_t *bin_out);

/**
 * @brief Show Frequency Interval for every bin
 *
 * @param sptct Spectrum Context
 * @return uint16_t * an arry which length is bin_num. the unit is Hz
  */
uint16_t *spectrum_get_freq_interval(SpectrumContext *sptct);

/**
 * @brief quantized 16 levels for spectrum_process's output
 *
 * @param sptct Spectrum Context
 * @param *bin_value output by spectrum_process's bin_out array.
 * @param *level_out quantized level value. shows 0~15. Please check LevelTableDB for every level's dB
 * @return void
  */
void spectrum_quantized_level(SpectrumContext *sptct, uint32_t *bin_value, uint8_t *level_out);

