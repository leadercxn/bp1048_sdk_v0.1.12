/*
 * xm_xiaoai_api.h
 *
 *  Created on: Nov 27, 2019
 *      Author: tony
 */

#ifndef _XM_XIAOAI_API_H_
#define _XM_XIAOAI_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include "type.h"

/* Typedef -------------------------------------------------------------------*/

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* API -----------------------------------------------------------------------*/
//С��AIЭ��������
void xiaoai_app_decode(uint8_t length, uint8_t *pValue);
//�����豸�˴��䷽ʽ��
void set_communicate_way(uint8_t set_flag);
uint8_t get_communicate_way(void);
//С�����ֻ��Ͽ�ʱ״̬��ʼ����
void xiaoai_app_disconn();
//��ȡ��ǰble����״̬��
uint8_t return_ble_conn_state();
//IOS�豸����A2DP֪ͨ������
void aivs_conn_edr_status(void);

//С��ͬѧ�������ݷ��ʹ���
void xm_ai_encode_data_run_loop(void);
void ai_ble_run_loop(void);

//��ȡapp�Ƿ����ӳɹ�
void xm_speech_con_set(uint8_t set);
uint8_t xm_speech_iscon();

//ble������ء�
void ble_set_data(char* set_send_data,int send_size_d);
void set_send_size(int set_send_size);
int  get_ble_send_size();
char* get_send_data_p();

//��̬����ai task�ĳ���ͨѶ��ʽ��
int32_t OpusEncodedLenGet();
void OpusEncodedLenSet(int32_t set);

void OpusEncoderTaskPro();//Ai task��������

int32_t XiaoAiTaskCreate(void* parentMsgHandle);//���񴴽�
void XiaoAiTaskDelete(void);

#ifdef __cplusplus
}
#endif




#endif /* _XM_XIAOAI_API_H_ */
