#ifndef __AI_H__
#define __AI_H__

#include <string.h>
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

//AI��ʼ��
bool ai_init(void);

void ai_deinit(void);
//AI���ݱ���  buf����Ϊ������Ϊ44100��PCM����
void ai_audio_encode(int16_t* buf,uint32_t sample,uint8_t ch);

//��������������ʽ����
void ai_start(void);


//BLE���պ�����ݴ���
void ble_rcv_data_proess(uint8_t *p,uint32_t size);

//spp���պ�����ݴ���
void spp_rcv_data_proess(uint8_t *p,uint32_t size);

//AI ���뿪����־��
void set_ai_encoder_f(uint8_t set_f);
uint8_t get_ai_encoder_f();
void send_data_to_phone();

//AI �������ݷ���׼��.
void ai_run_loop(void);
void ble_send_data(uint8_t res_handle,uint8_t data_handle);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__ADC_KEY_H__
