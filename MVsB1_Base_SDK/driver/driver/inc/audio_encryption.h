/**
 *******************************************************************************
 * @file    audio_encryption.h
 * @brief	Audio Effect Encryption driver
 
 * @author  Sam
 * @version V1.0.0
 
 * $Created: 2018-5-14 15:33:00$
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 ******************************************************************************* 
 */
 
 /**
 * @addtogroup AudioEncryption
 * @{
 * @defgroup encryption audio_encryption.h
 * @{
 */
#ifndef __AUDIOENCRYPTION_H__
#define __AUDIOENCRYPTION_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

/**
 * @brief	��Ч��������
 * @param	*Buf����Ч��������ָ��
 * @param	Len����Ч��������
 * @param	Key����Կ
 * @return	��
 * @note 	��Կ32bit,���û��ڳ������Լ�����
 */
void AudioEffectEncryption(uint8_t* Buf, uint32_t Len, uint32_t Key);

/**
 * @brief	��Ч��������
 * @param	*Buf����Ч��������ָ��
 * @param	Len����Ч��������
 * @param	Key����Կ
 * @return	��
 * @note 	��Կ32bit,���û��ڳ������Լ�����
 */
void AudioEffectDecryption(uint8_t* Buf, uint32_t Len, uint32_t key);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__AUDIOENCRYPTION_H__

/**
 * @}
 * @}
 */
 
