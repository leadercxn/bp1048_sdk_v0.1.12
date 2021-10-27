/**
 *******************************************************************************
 * @file    bt_tws_api.h
 * @author  Owen
 * @version V1.0.0
 * @date    19-Sep-2019
 * @brief   tws callback events and actions
 *******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

/**
* @addtogroup Bluetooth
* @{
* @defgroup bt_a2dp_api bt_a2dp_api.h
* @{
*/

#include "type.h"

#ifndef __BT_TWS_API_H__
#define __BT_TWS_API_H__
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
typedef enum{
	BT_STACK_EVENT_TWS_NONE = 0,

	BT_STACK_EVENT_TWS_CONNECTED,

	BT_STACK_EVENT_TWS_DISCONNECT,

	BT_STACK_EVENT_TWS_DATA_IND,

	//master event
	BT_STACK_EVENT_TWS_MASTER_CONNECTED,
	
	BT_STACK_EVENT_TWS_MASTER_DISCONNECT,
		
	BT_STACK_EVENT_TWS_MASTER_DATA_IND,

	BT_STACK_EVENT_TWS_ACL_DATA_LINK,
	
	BT_STACK_EVENT_TWS_ACL_DATA_UNLINK,
	
	//slave event
	BT_STACK_EVENT_TWS_SLAVE_CONNECTED,
	
	BT_STACK_EVENT_TWS_SLAVE_DISCONNECT,
		
	BT_STACK_EVENT_TWS_SLAVE_DATA_IND,

	BT_STACK_EVENT_TWS_SLAVE_STREAM_START,

	BT_STACK_EVENT_TWS_SLAVE_STREAM_STOP,

	BT_STACK_EVENT_TWS_MUTE,
	
	//当前HOST链路异常,延迟重新组网
	BT_STACK_EVENT_TWS_CONNECT_DELAY,
	//当前TWS链路存在,取消组网流程
	BT_STACK_EVENT_TWS_CONNECT_LINK_EXIST,
	
}BT_TWS_CALLBACK_EVENT;

typedef enum
{
	BT_TWS_MASTER = 0x00,
	BT_TWS_SLAVE,
	BT_TWS_UNKNOW = 0xff,
} BT_TWS_ROLE;

typedef struct _BT_TWS_CALLBACK_PARAMS
{
	BT_TWS_ROLE					role;
	uint16_t 					paramsLen;
	bool						status;
	uint16_t		 			errorCode;

	union
	{
		uint8_t					*bd_addr;
		uint8_t					*twsData;
	}params;
}BT_TWS_CALLBACK_PARAMS;

typedef void (*BTTwsCallbackFunc)(BT_TWS_CALLBACK_EVENT event, BT_TWS_CALLBACK_PARAMS * param);

void BtTwsCallback(BT_TWS_CALLBACK_EVENT event, BT_TWS_CALLBACK_PARAMS * param);

typedef struct _twsAppFeatures
{
	uint32_t			twsSimplePairingCfg;	//0=both sides;  1=one side;
	uint32_t			twsRoleCfg;		//0=random; 1=master; 2=slave
	BTTwsCallbackFunc	twsAppCallback;
}TwsAppFeatures;

//tws avrcp cmd
void tws_slave_send_cmd_play_pause(void);
void tws_slave_send_cmd_next(void);
void tws_slave_send_cmd_prev(void);

#define TWS_GET_FIFO_LEN	0
#define TWS_OPEN_CLK		1
#define TWS_FIFO_RESET		2
#define TWS_PLAY_INIT		3
int16_t tws_play_remote(uint8_t cmd);

#define TWS_M_LR__S_LR		0//主机:立体声,  从机:立体声
#define TWS_M_L__S_R		1//主机:L声道,  	  从机:R声道
#define TWS_M_R__S_L		2//主机:R声道,   从机:L声道
#define TWS_M_LR__S_MONO	3//主机:立体声,  从机:L+R混合
uint32_t tws_mem_size(uint32_t delay,uint8_t audio_mode);
uint32_t tws_mem_set(uint8_t*mem);
void tws_run_loop(void);
//void tws_start(void);
void tws_start(uint32_t delay);
/*
 * p_m主机音频数据
 * p_s从机音频数据    audio_mode为TWS_M_LR__S_MONO时候使用
 * len 音频数据的sample数
 * data_source 音频数据的通路 app或者提示音
 */
void tws_music_pcm_process(int16_t *p_m,int16_t *p_s,uint32_t len,uint8_t data_source);

//TWS组网角色
typedef enum
{
	TWS_ROLE_RANDOM = 0,
	TWS_ROLE_MASTER,
	TWS_ROLE_SLAVE,
}TWS_PAIRING_ROLE;

void tws_master_connect(unsigned char *addr);
void tws_start_pairing(TWS_PAIRING_ROLE role);
void tws_link_disconnect(void);
BT_TWS_ROLE tws_get_role(void);
void tws_link_status_set(uint8_t state);

void tws_slave_start_simple_pairing(void);
void tws_slave_stop_simple_pairing(void);
void tws_slave_simple_pairing_ready(void);
void tws_slave_simple_pairing_end(void);

void tws_vol_send(uint16_t Vol,bool MuteFlag);
void tws_eq_mode_send(uint16_t eq_mode);
void tws_music_bass_treb_send(uint16_t bass_vol,uint16_t treb_vol);
void tws_channel_mode_send(uint16_t ChMode);
void tws_master_hfp_send(void);
void tws_master_a2dp_send(void);

#endif /*__BT_TWS_API_H__*/

/**
 * @}
 * @}
 */

