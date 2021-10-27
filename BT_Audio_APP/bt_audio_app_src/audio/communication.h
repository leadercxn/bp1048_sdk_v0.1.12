/**
 **************************************************************************************
 * @file    communication.h
 * @brief
 *
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdint.h>
#include "ctrlvars.h"


#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#ifdef CFG_COMMUNICATION_BY_UART
void UART1_Communication_Init(uint8_t *s_tx_buf, uint32_t s_tx_len, uint8_t *s_rx_buf, uint32_t s_rx_len);

void UART1_Communication_Receive_Process(void);

void UART_Communication_Heart_Message(void);

#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++//
void AudioClkConfig(uint32_t sample_rate, uint8_t mclk_source);
void HIDUsb_Tx(uint8_t *buf,uint16_t len);
void HIDUsb_Rx(uint8_t *buf,uint16_t len);
void hid_send_data(void);

//+++++++++++++++++++++++++++++++++++++++++++++++++++//


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__COMMUNICATION_H__
