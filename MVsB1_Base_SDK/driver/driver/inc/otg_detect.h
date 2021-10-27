/**
 *****************************************************************************
 * @file     otg_detect.h
 * @author   Owen
 * @version  V1.0.0
 * @date     27-03-2017
 * @brief    otg port detect module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2015 MVSilicon </center></h2>
 */

/**
 * @addtogroup OTG
 * @{
 * @defgroup otg_detect otg_detect.h
 * @{
 */
#ifndef __OTG_DETECT_H__
#define	__OTG_DETECT_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 


#include "type.h"

/**
 * @brief  Port���Ӽ�⺯��
 * @param  NONE
 * @return NONE
 */
void OTG_PortLinkCheck(void);

/**
 * @brief  ���Port�˿����Ƿ���һ��U���豸����
 * @param  NONE
 * @return 1-��U���豸���ӣ�0-��U���豸����
 */
bool OTG_PortHostIsLink(void);

/**
 * @brief  ���Port�˿����Ƿ���һ��PC����
 * @param  NONE
 * @return 1-��PC���ӣ�0-��PC����
 */
bool OTG_PortDeviceIsLink(void);


/**
 * @brief  ʹ��port�˿ڼ��U������
 * @param  HostEnable: �Ƿ���U������
 * @param  DeviceEnable: �Ƿ���PC����
 * @return NONE
 */
void OTG_PortSetDetectMode(bool HostEnable, bool DeviceEnable);

/**
 * @brief  ʹ��port�˿ڼ����ͣ
 * @param  PauseEnable: �Ƿ���ͣ���
 * @return NONE
 */
void OTG_PortSetDetectPause(bool PauseEnable);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__OTG_DETECT_H__

/**
 * @}
 * @}
 */
