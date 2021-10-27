/**
 **************************************************************************************
 * @file    usb_audio_api.h
 * @brief   usb audio api 
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-05-08 11:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __USB_AUDIO_MODE_H__
#define __USB_AUDIO_MODE_H__


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 


#define	USB_FIFO_LEN						(256 * 2 * 2 * 8)
#define USB_AUDIO_SRC_BUF_LEN				(150 * 2 * 2)

#define CFG_RES_AUDIO_USB_IN_EN
#define CFG_RES_AUDIO_USB_OUT_EN
#define CFG_RES_AUDIO_USB_SRC_EN
#define CFG_RES_AUDIO_USB_VOL_SET_EN

#define CFG_PARA_SAMPLE_RATE	44100


bool UsbDevicePlayInit(void);

uint16_t UsbAudioSpeakerDataGet(void *Buffer,uint16_t Len);
uint16_t UsbAudioSpeakerDataLenGet(void);
uint16_t UsbAudioMicDataSet(void *Buffer,uint16_t Len);
uint16_t UsbAudioMicSpaceLenGet(void);

void UsbDeviceEnable(void);
void UsbDeviceDisable(void);
void UsbAudioTimer1msProcess(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __USB_AUDIO_MODE_H__

