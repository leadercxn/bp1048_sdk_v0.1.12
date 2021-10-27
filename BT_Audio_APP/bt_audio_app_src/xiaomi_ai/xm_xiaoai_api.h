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
//小爱AI协议解析入口
void xiaoai_app_decode(uint8_t length, uint8_t *pValue);
//设置设备端传输方式。
void set_communicate_way(uint8_t set_flag);
uint8_t get_communicate_way(void);
//小爱与手机断开时状态初始化。
void xiaoai_app_disconn();
//获取当前ble连接状态。
uint8_t return_ble_conn_state();
//IOS设备连接A2DP通知函数。
void aivs_conn_edr_status(void);

//小爱同学编码数据发送处理。
void xm_ai_encode_data_run_loop(void);
void ai_ble_run_loop(void);

//获取app是否连接成功
void xm_speech_con_set(uint8_t set);
uint8_t xm_speech_iscon();

//ble发送相关。
void ble_set_data(char* set_send_data,int send_size_d);
void set_send_size(int set_send_size);
int  get_ble_send_size();
char* get_send_data_p();

//动态申请ai task的长度通讯方式。
int32_t OpusEncodedLenGet();
void OpusEncodedLenSet(int32_t set);

void OpusEncoderTaskPro();//Ai task主函数。

int32_t XiaoAiTaskCreate(void* parentMsgHandle);//任务创建
void XiaoAiTaskDelete(void);

#ifdef __cplusplus
}
#endif




#endif /* _XM_XIAOAI_API_H_ */
