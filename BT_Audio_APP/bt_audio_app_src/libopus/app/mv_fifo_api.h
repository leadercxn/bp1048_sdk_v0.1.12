/*
 * mv_fifo_api.h
 *
 *  Created on: Nov 26, 2019
 *      Author: tony
 */

#ifndef _MV_FIFO_API_H_
#define _MV_FIFO_API_H_


#include <stdio.h>

void XM_AI_Mutex_init();
void mv_opus_fifo_init();
void mv_opus_fifo_deinit();
uint32_t mv_opus_get_len(void);
void mv_opus_get_send(uint8_t* buf,uint32_t size);
void mv_opus_get_data(uint8_t* buf,uint32_t size);
uint32_t xmai_resampler_init();
uint32_t xmai_resampler_apply(int16_t *pcm_in, int16_t *pcm_out, int32_t sample);


#endif /* BT_AUDIO_APP_SRC_LIBOPUS_APP_MV_FIFO_API_H_ */
