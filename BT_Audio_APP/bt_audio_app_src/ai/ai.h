#ifndef __AI_H__
#define __AI_H__

#include <string.h>
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

//AI初始化
bool ai_init(void);

void ai_deinit(void);
//AI数据编码  buf必须为采样率为44100的PCM数据
void ai_audio_encode(int16_t* buf,uint32_t sample,uint8_t ch);

//按键或者其他方式调用
void ai_start(void);


//BLE接收后的数据处理
void ble_rcv_data_proess(uint8_t *p,uint32_t size);

//spp接收后的数据处理
void spp_rcv_data_proess(uint8_t *p,uint32_t size);

//AI 编码开启标志。
void set_ai_encoder_f(uint8_t set_f);
uint8_t get_ai_encoder_f();
void send_data_to_phone();

//AI 无线数据发送准备.
void ai_run_loop(void);
void ble_send_data(uint8_t res_handle,uint8_t data_handle);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__ADC_KEY_H__
