/**
 *******************************************************************************
 * @file    bt_tws_app.c
 * @author  Owen
 * @version V1.0.1
 * @date    10-Oct-2019
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

#include "type.h"
#include "debug.h"
#include "clk.h"
#include "stdlib.h"
#include "bt_manager.h"
#include "bt_app_interface.h"
#include "bt_tws_api.h"
#include "mcu_circular_buf.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#include "bt_play_mode.h"
#include "main_task.h"
#include "audio_core_api.h"
#include "ctrlvars.h"
#endif
#include "bt_avrcp_api.h"
#include "dac_interface.h"
#include "adc_interface.h"
#include "sbcenc_api.h"
#include "uarts.h"
#include "dma.h"
#include "i2s.h"
#include "audio_adc.h"
#include "i2s_interface.h"
#include "ble_api.h"
#include "irqn.h"
#include "audio_vol.h"
#include "ble_app_func.h"
#include "bt_ddb_flash.h"
#include "remind_sound_service.h"
#ifdef BT_TWS_SUPPORT
#include "bt_tws_app_func.h"
extern void tws_sound_remind_get(uint8_t *p);
extern void tws_sound_remind_status_clear();
extern uint32_t tws_sound_remind_status();

extern uint32_t gBtEnterDeepSleepFlag;
extern void TwsSlaveModeEnter(void);
extern void TwsSlaveModeExit(void);

extern void tws_master_active_cmd(void);
extern uint8_t tws_reconnect_flag_get(void);
extern void send_sniff_msg();
extern void tws_slave_fifo_clear(void);
#define TWS_ACL_LINK		0xF1
#define TWS_ACL_UNLINK		0xF2

uint32_t gBtTwsSniffLinkLoss = 0; //1=在sniff下tws linkloss,需要进行回连处理

static char *cmd_play_pause   = "CMD:AVRCP_PLAY_PAUSE";
static char *cmd_next     = "CMD:AVRCP_NEXT";
static char *cmd_prev     = "CMD:AVRCP_PREV";
static char *cmd_reset    = "CMD:TWS_RESET";
#if 0
	static char *cmd_remind   = "CMD:REMIND";
#endif
static char *cmd_sniff    = "CMD:TWS_SNIFF";
static char *cmd_repair   = "CMD:TWS_REPAIR";
static char *cmd_vol   = "CMD:TWS_VOL";
static char *cmd_mute   = "CMD:MUTE";
#if (BT_HFP_SUPPORT == ENABLE)
static char *cmd_hfp_state = "CMD:HFP_STATE";
#endif
static char *cmd_a2dp_state = "CMD:A2DP_STATE";

#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
static char *cmd_eq       = "CMD:TWS_EQ";
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
static char *cmd_music_treb_bass  = "CMD:TWS_MUSIC_TREB_BASS";
#endif
#ifdef CFG_TWS_PEER_APP
static char *cmd_ch_mode   = "CMD:TWS_MOD";
#endif
uint8_t temp[30];

uint32_t gBtTwsDelayConnectCnt = 0;//配对组网,主动发起连接异常,延时1s再次发起连接

extern uint8_t tws_reconnect_flag_get(void);

//TWS命令收发接口,按照以上内容格式发送
void tws_master_send(uint8_t *buf,uint16_t len);
void tws_slave_send(uint8_t *buf,uint16_t len);

//命令解析按照见函数
//static void BtTws_Master_RecvData(BT_TWS_CALLBACK_PARAMS * param)
//static void BtTws_Slave_RecvData(BT_TWS_CALLBACK_PARAMS * param)
void tws_vol_send(uint16_t Vol,bool MuteFlag)
{
	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
		return;
	
#ifndef TWS_VOLUME_SYNC_ENABLE
	return;
#endif

	bool ret = GIE_STATE_GET();
	if(GetBtManager()->twsState < BT_TWS_STATE_CONNECTED)
		return;
	
	GIE_DISABLE();
	memcpy(temp,cmd_vol,strlen(cmd_vol));
	temp[strlen(cmd_vol) + 1] = Vol;
	temp[strlen(cmd_vol) + 2] = MuteFlag;
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		tws_master_send(temp, (strlen(cmd_vol) + 3));
	}
	else if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		tws_slave_send(temp, (strlen(cmd_vol) + 3));
	}
	if(ret)
	{
		GIE_ENABLE();
	}
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		APP_DBG("master send cmd_vol_mute:%u %u\n",mainAppCt.MusicVolume,MuteFlag);
	}
	else if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		APP_DBG("slave send cmd_vol_mute:%u %u\n",mainAppCt.MusicVolume,MuteFlag);
	}
}

void tws_master_vol_send(uint16_t Vol,bool MuteFlag)
{
	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
		return;
	
#ifndef TWS_VOLUME_SYNC_ENABLE
	return;
#endif

	bool ret = GIE_STATE_GET();
	if(GetBtManager()->twsState < BT_TWS_STATE_CONNECTED)
		return;
	
	GIE_DISABLE();
	memcpy(temp,cmd_vol,strlen(cmd_vol));
	temp[strlen(cmd_vol) + 1] = Vol;
	temp[strlen(cmd_vol) + 2] = MuteFlag;
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		tws_master_send(temp, (strlen(cmd_vol) + 3));
	}
	if(ret)
	{
		GIE_ENABLE();
	}
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		APP_DBG("master send cmd_vol_mute:%u %u\n",mainAppCt.MusicVolume,MuteFlag);
	}
}

#if (BT_HFP_SUPPORT == ENABLE)
void tws_master_hfp_send(void)
{
	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED || GetBtManager()->twsRole == BT_TWS_SLAVE)
		return;

	bool ret = GIE_STATE_GET();
	GIE_DISABLE();
	memcpy(temp, cmd_hfp_state, strlen(cmd_hfp_state));
	temp[strlen(cmd_hfp_state) + 1] = (uint8_t)GetHfpState();
	tws_master_send(temp, (strlen(cmd_hfp_state) + 2));
	
	if(ret)
	{
		GIE_ENABLE();
	}
	APP_DBG("master send cmd_hfp_state:%d\n", temp[strlen(cmd_hfp_state) + 1]);
}
#endif

void tws_master_a2dp_send(void)
{
	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED || GetBtManager()->twsRole == BT_TWS_SLAVE)
		return;

	bool ret = GIE_STATE_GET();
	GIE_DISABLE();
	memcpy(temp, cmd_a2dp_state, strlen(cmd_a2dp_state));
	temp[strlen(cmd_a2dp_state) + 1] = (uint8_t)GetBtPlayState();
	tws_master_send(temp, (strlen(cmd_a2dp_state) + 2));

	if(ret)
	{
		GIE_ENABLE();
	}
	APP_DBG("master send cmd_a2dp_state:%d\n", temp[strlen(cmd_a2dp_state) + 1]);
}

void tws_master_mute_send(bool MuteFlag)
{

	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
		return;

	if(GetBtManager()->twsState < BT_TWS_STATE_CONNECTED || GetBtManager()->twsRole == BT_TWS_SLAVE)
		return;
	bool ret = GIE_STATE_GET();
	GIE_DISABLE();
	memcpy(temp,cmd_mute,strlen(cmd_mute));
	temp[strlen(cmd_mute) + 1] = MuteFlag;

	tws_master_send(temp, strlen(cmd_mute) + 2);

	if(ret)
	{
		GIE_ENABLE();
	}
	vTaskDelay(CFG_CMD_DELAY);
	APP_DBG("master send cmd_dac_mute: %u\n",MuteFlag);

}

#ifdef CFG_TWS_PEER_APP
void tws_channel_mode_send(uint16_t ChMode)
{
	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
		return;
	bool ret = GIE_STATE_GET();
	if(GetBtManager()->twsState < BT_TWS_STATE_CONNECTED)
		return;
	
	GIE_DISABLE();
	memcpy(temp,cmd_ch_mode,strlen(cmd_ch_mode));
	temp[strlen(cmd_ch_mode) + 1] = ChMode;
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		tws_master_send(temp, (strlen(cmd_ch_mode) + 2));
	}
	else if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		tws_slave_send(temp, (strlen(cmd_ch_mode) + 2));
	}
	if(ret)
	{
		GIE_ENABLE();
	}
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		APP_DBG("master send cmd_ch_mode:%u\n",mainAppCt.MusicVolume);
	}
	else if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		APP_DBG("slave send cmd_ch_mode:%u %u\n",mainAppCt.MusicVolume);
	}
}
#endif

#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN	 
void tws_eq_mode_send(uint16_t eq_mode)
{
	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
		return;
	
	bool ret = GIE_STATE_GET();
	if(GetBtManager()->twsState < BT_TWS_STATE_CONNECTED)
		return;
	
	GIE_DISABLE();
	memcpy(temp,cmd_eq,strlen(cmd_eq));
	temp[strlen(cmd_eq) + 1] = eq_mode;
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		tws_master_send(temp, (strlen(cmd_eq) + 2));
	}
	else if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		tws_slave_send(temp, (strlen(cmd_eq) + 2));
	}
	if(ret)
	{
		GIE_ENABLE();
	}
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		APP_DBG("master send cmd_vol_mute:%u\n",eq_mode);
	}
	else if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		APP_DBG("slave send cmd_vol_mute:%u\n",eq_mode);
	}
}

void tws_master_eq_mode_send(uint16_t eq_mode)
{

	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
		return;
	
	bool ret = GIE_STATE_GET();
	if(GetBtManager()->twsState < BT_TWS_STATE_CONNECTED)
		return;
	
	GIE_DISABLE();
	memcpy(temp,cmd_eq,strlen(cmd_eq));
	temp[strlen(cmd_eq) + 1] = eq_mode;
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		tws_master_send(temp, (strlen(cmd_eq) + 2));
	}
	if(ret)
	{
		GIE_ENABLE();
	}
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		APP_DBG("master send cmd_vol_mute:%u\n",eq_mode);
	}
}
#endif

#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
void tws_music_bass_treb_send(uint16_t bass_vol,uint16_t treb_vol)
{

	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
		return;
	
	bool ret = GIE_STATE_GET();
	if(GetBtManager()->twsState < BT_TWS_STATE_CONNECTED)
		return;
	
	GIE_DISABLE();
	memcpy(temp,cmd_music_treb_bass,strlen(cmd_music_treb_bass));
	temp[strlen(cmd_music_treb_bass) + 1] = bass_vol;
	temp[strlen(cmd_music_treb_bass) + 2] = treb_vol;
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		tws_master_send(temp, (strlen(cmd_music_treb_bass) + 3));
	}
	else if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		tws_slave_send(temp, (strlen(cmd_music_treb_bass) + 3));
	}
	if(ret)
	{
		GIE_ENABLE();
	}
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		APP_DBG("master send cmd_music_bass_treb:%u %u\n",bass_vol,treb_vol);
	}
	else if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		APP_DBG("slave send cmd_music_bass_treb:%u %u\n",bass_vol,treb_vol);
	}
}

void tws_master_music_bass_treb_send(uint16_t bass_vol,uint16_t treb_vol)
{
	if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
		return;
	
	bool ret = GIE_STATE_GET();
	if(GetBtManager()->twsState < BT_TWS_STATE_CONNECTED)
		return;
	
	GIE_DISABLE();
	memcpy(temp,cmd_music_treb_bass,strlen(cmd_music_treb_bass));
	temp[strlen(cmd_music_treb_bass) + 1] = bass_vol;
	temp[strlen(cmd_music_treb_bass) + 2] = treb_vol;
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		tws_master_send(temp, (strlen(cmd_music_treb_bass) + 3));
	}
	if(ret)
	{
		GIE_ENABLE();
	}
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		APP_DBG("master send cmd_music_bass_treb:%u %u\n",bass_vol,treb_vol);
	}
}
#endif

void tws_peer_repair(void)
{
	if(GetBtManager()->twsRole == BT_TWS_MASTER)
	{
		bool ret = GIE_STATE_GET();
		GIE_DISABLE();
		tws_master_send((uint8_t *)cmd_repair, strlen(cmd_repair));
		if(ret)
		{
			GIE_ENABLE();
		}
		APP_DBG("send cmd_repair\n");
	}
	else if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		APP_DBG("send cmd_repair\n");
		tws_slave_send((uint8_t *)cmd_repair, strlen(cmd_repair));
	}
}


int tws_cmp_paired_mac(uint8_t *addr)
{
	/*uint8_t ls_addr[6];
	ls_addr[5] = addr[0];
	ls_addr[4] = addr[1];
	ls_addr[3] = addr[2];
	ls_addr[2] = addr[3];
	ls_addr[1] = addr[4];
	ls_addr[0] = addr[5];
	return memcmp(ls_addr, btManager.btTwsDeviceAddr, 6);
	*/
	return memcmp(addr, btManager.btTwsDeviceAddr, 6);
}

int tws_cmp_local_mac(uint8_t *addr)
{
	/*uint8_t ls_addr[6];
	ls_addr[5] = addr[0];
	ls_addr[4] = addr[1];
	ls_addr[3] = addr[2];
	ls_addr[2] = addr[3];
	ls_addr[1] = addr[4];
	ls_addr[0] = addr[5];
	return memcmp(ls_addr, btManager.btDevAddr, 6);
	*/
	return memcmp(addr, btManager.btDevAddr, 6);
}

int tws_cmp_name(char* name)
{
	return memcmp(name, BT_NAME, strlen(BT_NAME)-2);
}

int tws_inquiry_cmp_name(char* name, uint8_t len)
{
#ifdef TWS_FILTER_NAME
	extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;
	uint8_t localLen = strlen(btStackConfigParams->bt_LocalDeviceName);
	if(len != localLen)
		return -1;
	return memcmp(name, &btStackConfigParams->bt_LocalDeviceName[0], localLen);
#else
	return 0;
#endif
}

int tws_filter_user_defined_infor_cmp(uint8_t *infor)
{
#ifdef TWS_FILTER_USER_DEFINED
	return memcmp(infor, &btManager.TwsFilterInfor[0], 6);
#else
	return 0;
#endif
}

BT_TWS_ROLE tws_get_role(void)
{
	return GetBtManager()->twsRole;
}

bool tws_connect_cmp(uint8_t * addr)
{
	return memcmp(&GetBtManager()->btTwsDeviceAddr[0], addr, 6);
}

unsigned char tws_role_match(unsigned char *addr)
{
#if (TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)
#include "mode_switch_api.h"
	if(GetSystemMode() == AppModeTwsSlavePlay)
		return BT_TWS_SLAVE;
	else
		return BT_TWS_MASTER;
#endif
	if(memcmp(&GetBtManager()->btTwsDeviceAddr[0], addr, 6) == 0)
	{
		return btManager.twsRole;
	}
	else
	{
		return 0xff;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
//master callback
static void BtTws_Master_Connected(BT_TWS_CALLBACK_PARAMS * param)
{
	APP_DBG("TWS_MASTER_CONNECTED:\n");
	{
		MessageContext		msgSend;
		msgSend.msgId		= MSG_BT_TWS_MASTER_CONNECTED;
		MessageSend(GetMainMessageHandle(), &msgSend);
	}
	memcpy(GetBtManager()->btTwsDeviceAddr, param->params.bd_addr, param->paramsLen);
	APP_DBG("addr: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",
		GetBtManager()->btTwsDeviceAddr[0],
		GetBtManager()->btTwsDeviceAddr[1],
		GetBtManager()->btTwsDeviceAddr[2],
		GetBtManager()->btTwsDeviceAddr[3],
		GetBtManager()->btTwsDeviceAddr[4],
		GetBtManager()->btTwsDeviceAddr[5]
		);
	GetBtManager()->twsState = BT_TWS_STATE_CONNECTED;
	GetBtManager()->twsRole = BT_TWS_MASTER;
	BtDdb_UpgradeTwsInfor(GetBtManager()->btTwsDeviceAddr);
	GetBtManager()->twsFlag = 1;
	btManager.btReconnectedFlag = 1;
	GetBtManager()->btReconnectDeviceFlag &= ~(RECONNECT_TWS);
	btManager.btReconDelayFlag = 0;

	if(btManager.btConStateProtectCnt)
		btManager.btConStateProtectCnt=0;

	if(btManager.btTwsReconnectTimer.timerFlag)
		BtStopReconnectTws();

#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)
	BtTwsExitPairingMode();

	if(btManager.btLinkState)
	{
		//蓝牙已连接手机,进入不可被连接不可被搜索状态(手机和SLAVE都已连上)
		BTSetAccessMode(BtAccessModeNotAccessible);
	}
	else
	{
		//蓝牙未连接手机,进入可被搜索可被连接状态(只有SLAVE已连上)
		BTSetAccessMode(BtAccessModeGeneralAccessible);
	}
#elif ((TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)||(TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER))
	BtTwsExitSimplePairingMode();
#else //CFG_TWS_PEER_SLAVE/CFG_TWS_PEER_MASTER
	BtTwsExitPeerPairingMode();

	if(btManager.btLinkState)
	{
		//蓝牙已连接手机,进入不可被连接不可被搜索状态(手机和SLAVE都已连上)
		BTSetAccessMode(BtAccessModeNotAccessible);
	}
	else
	{
		//蓝牙未连接手机,进入可被搜索可被连接状态(只有SLAVE已连上)
		BTSetAccessMode(BtAccessModeGeneralAccessible);
	}
#endif

#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
	if(Bt_sniff_sniff_start_state_get())
	{
		//再次使从进入sniff
		//printf("send sniff cmd to slave again\n");
		BTSetRemDevIntoSniffMode(GetBtManager()->btTwsDeviceAddr);
		return;
	}
	else
	{
		tws_master_active_cmd();
	}
#endif
	
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
	DisableAdvertising();
#endif

	tws_master_vol_send(mainAppCt.MusicVolume, IsAudioPlayerMute());

#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	tws_master_eq_mode_send(mainAppCt.EqMode);
#endif

#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
	tws_master_music_bass_treb_send(mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
#endif
}

static void BtTws_Master_Disconnected(BT_TWS_CALLBACK_PARAMS * param)
{
	GetBtManager()->twsMode=0;
	APP_DBG("TWS_MASTER_DISCONNECT:\n");
	GetBtManager()->twsState = BT_TWS_STATE_NONE;
	if(btManager.btLinkState)
	{
		//蓝牙已连接手机,进入可被连接不可被搜索状态(只有手机已连上)
		BTSetAccessMode(BtAccessModeConnectableOnly);
	}
	else
	{
		//蓝牙未连接手机,进入可被搜索可被连接状态(手机和SLAVE都未连上)
		BTSetAccessMode(BtAccessModeGeneralAccessible);
	}
	
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
	if(Bt_sniff_sniff_start_state_get() == 0)
		ble_advertisement_data_update();
	else
	{
		gBtTwsSniffLinkLoss = 1;
	}
#elif ((TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER)||(TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)||(TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM))
	if(tws_reconnect_flag_get())
	{
		//extern void BtTwsLinkLoss(void);
		//APP_DBG("tws link loss, reconnect...\n");
		//BtTwsLinkLoss();
	}
	else if(btManager.btTwsReconnectTimer.timerFlag)
	{
		BtReconnectTwsCB();
	}
#endif
}

static void BtTws_Master_RecvData(BT_TWS_CALLBACK_PARAMS * param)
{
#include "main_task.h"
#include "rtos_api.h"
#include "media_play_mode.h"
	//APP_DBG("TWS_MASTER_DATA_IND:\n");
	if(memcmp(param->params.twsData,cmd_play_pause,strlen(cmd_play_pause)) == 0)
	{
		APP_DBG("cmd_play_pause\n\n");
		
		if(GetSystemMode() == AppModeBtAudioPlay)
			BtPlayControl_PlayPause();
#if (defined(CFG_APP_USB_PLAY_MODE_EN)||defined(CFG_APP_CARD_PLAY_MODE_EN))
		else if((GetSystemMode() == AppModeCardAudioPlay)||(GetSystemMode() == AppModeUDiskAudioPlay))
		{
			MessageContext		msgSend;
			msgSend.msgId = MSG_PLAY_PAUSE;
			MessageSend(GetMediaPlayMessageHandle(), &msgSend);
		}
#endif
	}
	else if(memcmp(param->params.twsData,cmd_next,strlen(cmd_next)) == 0)
	{
		APP_DBG("cmd_next\n\n");
		
		if(GetSystemMode() == AppModeBtAudioPlay)
			BTCtrlNext();
#if (defined(CFG_APP_USB_PLAY_MODE_EN)||defined(CFG_APP_CARD_PLAY_MODE_EN))
		else if((GetSystemMode() == AppModeCardAudioPlay)||(GetSystemMode() == AppModeUDiskAudioPlay))
		{
			MessageContext		msgSend;
			msgSend.msgId = MSG_NEXT;
			MessageSend(GetMediaPlayMessageHandle(), &msgSend);
		}
#endif
	}
	else if(memcmp(param->params.twsData,cmd_prev,strlen(cmd_prev)) == 0)
	{
		APP_DBG("cmd_prev\n\n");
		
		if(GetSystemMode() == AppModeBtAudioPlay)
			BTCtrlPrev();
#if (defined(CFG_APP_USB_PLAY_MODE_EN)||defined(CFG_APP_CARD_PLAY_MODE_EN))
		else if((GetSystemMode() == AppModeCardAudioPlay)||(GetSystemMode() == AppModeUDiskAudioPlay))
		{
			MessageContext		msgSend;
			msgSend.msgId = MSG_PRE;
			MessageSend(GetMediaPlayMessageHandle(), &msgSend);
		}
#endif
	}
	else if(memcmp(param->params.twsData,cmd_reset,strlen(cmd_reset)) == 0)
	{
		APP_DBG("cmd_reset\n\n");
		tws_init();
	}
	else if(memcmp(param->params.twsData,cmd_sniff,strlen(cmd_sniff)) == 0)
	{
		APP_DBG("cmd_gotodeepsleep\n\n");
		send_sniff_msg();
	}
	else if(memcmp(param->params.twsData,cmd_repair,strlen(cmd_repair)) == 0)
	{
		APP_DBG("cmd_repair\n\n");
		BtTwsPeerEnterPairingMode();
	}
	else if(memcmp(param->params.twsData,cmd_vol,strlen(cmd_vol)) == 0)
	{
		bool ret = GIE_STATE_GET();
		GIE_DISABLE();
		uint8_t *temp = param->params.twsData;
		mainAppCt.MusicVolume = temp[strlen(cmd_vol) + 1];
		mainAppCt.gSysVol.MuteFlag = temp[strlen(cmd_vol) + 2];
		if(ret)
		{
			GIE_ENABLE();
		}
		APP_DBG("master rcv cmd_vol_mute:%u %u\n",mainAppCt.MusicVolume,mainAppCt.gSysVol.MuteFlag);
		int i;
		if(mainAppCt.gSysVol.MuteFlag)
		{
			for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM ;i++)
			{
				AudioCoreSinkMute(i, TRUE, TRUE);
			}
		}
		else
		{
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
			//add volume sync(bluetooth play mode)
			if(GetSystemMode() == AppModeBtAudioPlay)
			{
				MessageContext		msgSend;

				SetBtSyncVolume(mainAppCt.MusicVolume);

				msgSend.msgId		= MSG_BT_PLAY_VOLUME_SET;
				MessageSend(GetBtPlayMessageHandle(), &msgSend);
			}
#endif
			for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM ;i++)
			{
				AudioCoreSinkUnmute(i, TRUE, TRUE);
			}
			AudioMusicVolSet(mainAppCt.MusicVolume);
			SystemVolSet();
		}
	}
#ifdef CFG_TWS_PEER_APP	
	else if(memcmp(param->params.twsData,cmd_ch_mode,strlen(cmd_ch_mode)) == 0)
	{
		bool ret = GIE_STATE_GET();
		GIE_DISABLE();
		uint8_t *temp = param->params.twsData;
//		TwsCfg.OutMode = temp[strlen(cmd_ch_mode) + 2];
		if(ret)
		{
			GIE_ENABLE();
		}
//		APP_DBG("master rcv cmd_ch_mode:%u\n",TwsCfg.OutMode);
//		if(TwsCfg.OutMode == 0)
//		{
//			TwsCfg.MasterSound = 1;//0=L,R, 1=L, 2= R,3=L+R
//			TwsCfg.SlaverSound = 2;//0=L,R, 1=L, 2= R,3=L+R
//			APP_DBG("tws peer stereo mode\n");
//		}
//		else
//		{
//			TwsCfg.MasterSound = 3;//0=L,R, 1=L, 2= R,3=L+R
//			TwsCfg.SlaverSound = 3;//0=L,R, 1=L, 2= R,3=L+R
//			APP_DBG("tws peer mono mode\n");
//		}
	}
#endif	
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	else if(memcmp(param->params.twsData,cmd_eq,strlen(cmd_eq)) == 0)
	{
		bool ret = GIE_STATE_GET();
		GIE_DISABLE();
		uint8_t *temp = param->params.twsData;
		mainAppCt.EqMode = temp[strlen(cmd_eq) + 1];
		if(ret)
		{
			GIE_ENABLE();
		}
		APP_DBG("master rcv cmd_eq_mode = %u\n", mainAppCt.EqMode);
		#ifndef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
		EqModeSet(mainAppCt.EqMode);
		#endif		
	}
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
	else if(memcmp(param->params.twsData,cmd_music_treb_bass,strlen(cmd_music_treb_bass)) == 0)
	{
		bool ret = GIE_STATE_GET();
		GIE_DISABLE();
		uint8_t *temp = param->params.twsData;
		mainAppCt.MusicBassStep = temp[strlen(cmd_music_treb_bass) + 1];
		mainAppCt.MusicTrebStep = temp[strlen(cmd_music_treb_bass) + 2];
		if(ret)
		{
			GIE_ENABLE();
		}
		APP_DBG("master rcv cmd_music_bass_treb = %u %u\n", mainAppCt.MusicBassStep,mainAppCt.MusicTrebStep);
		MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);		
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
//slave callback
static void BtTws_Slave_Connected(BT_TWS_CALLBACK_PARAMS * param)
{
	APP_DBG("TWS_SLAVE_CONNECTED:\n");
	{
		MessageContext		msgSend;
		msgSend.msgId		= MSG_BT_TWS_SLAVE_CONNECTED;
		MessageSend(GetMainMessageHandle(), &msgSend);
	}
	memcpy(GetBtManager()->btTwsDeviceAddr, param->params.bd_addr, param->paramsLen);
	APP_DBG("addr: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",
		GetBtManager()->btTwsDeviceAddr[0],
		GetBtManager()->btTwsDeviceAddr[1],
		GetBtManager()->btTwsDeviceAddr[2],
		GetBtManager()->btTwsDeviceAddr[3],
		GetBtManager()->btTwsDeviceAddr[4],
		GetBtManager()->btTwsDeviceAddr[5]
		);
	GetBtManager()->twsState = BT_TWS_STATE_CONNECTED;
	GetBtManager()->twsRole = BT_TWS_SLAVE;
	BtDdb_UpgradeTwsInfor(GetBtManager()->btTwsDeviceAddr);
	GetBtManager()->twsFlag = 1;
	btManager.btReconnectedFlag = 1;
	btManager.btReconDelayFlag = 0;
	
	if(btManager.btReconnectTimer.timerFlag)
	{
		BtStopReconnect();
	}
	
	if(btManager.btTwsReconnectTimer.timerFlag)
		BtStopReconnectTws();
	
	if(btManager.btConStateProtectCnt)
		btManager.btConStateProtectCnt=0;
	
	//slave在tws组网成功后，进入不可被搜索不可被连接状态
	BTSetAccessMode(BtAccessModeNotAccessible);
	GetBtManager()->btReconnectDeviceFlag &= ~((RECONNECT_TWS));
	
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
	tws_slave_simple_pairing_end();
	if(Bt_sniff_sniff_start_state_get() && (gBtTwsSniffLinkLoss==0))
		Bt_sniff_sniff_stop();
#endif

#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM)
	BtTwsExitPairingMode();
#elif ((TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)||(TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER))
	BtTwsExitSimplePairingMode();
#else //CFG_TWS_PEER_SLAVE/CFG_TWS_PEER_MASTER
	BtTwsExitPeerPairingMode();
#endif

#ifdef CFG_AUTO_ENTER_TWS_SLAVE_MODE
	TwsSlaveModeEnter();
#endif
}

static void BtTws_Slave_Disconnected(BT_TWS_CALLBACK_PARAMS * param)
{
	GetBtManager()->twsMode=0;
	APP_DBG("TWS_SLAVE_DISCONNECT:\n");
	GetBtManager()->twsState = BT_TWS_STATE_NONE;
	
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
	if(Bt_sniff_sniff_start_state_get())
	{
		//BleScanParamConfig_Sniff();
		//tws_master_connect(&btManager.btTwsDeviceAddr[0]);
		BtTwsConnectApi();
		gBtTwsSniffLinkLoss = 1;
	}
	else if(!btManager.twsSbSlaveDisable)
	{
		tws_slave_simple_pairing_ready();
	}
#elif ((TWS_PAIRING_MODE == CFG_TWS_PEER_MASTER)||(TWS_PAIRING_MODE == CFG_TWS_ROLE_RANDOM))//||(TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)
	if(tws_reconnect_flag_get())
	{
		extern void BtTwsLinkLoss(void);
		APP_DBG("tws link loss, reconnect...\n");
		BtTwsLinkLoss();
	}
	else if(btManager.btTwsReconnectTimer.timerFlag)
	{
		BtReconnectTwsCB();
	}
#endif

#ifdef CFG_AUTO_ENTER_TWS_SLAVE_MODE
	TwsSlaveModeExit();
#endif
	BTSetAccessMode(BtAccessModeGeneralAccessible);
}

static void BtTws_Slave_RecvData(BT_TWS_CALLBACK_PARAMS * param)
{
	if((param->params.twsData)[0] != 'd')
	{
		if(memcmp(param->params.twsData,cmd_mute,strlen(cmd_mute)) == 0)
		{
			if((param->params.twsData)[strlen(cmd_mute) + 1])
			{
				if(!IsAudioPlayerMute())
				{
					AudioPlayerMute();
				}
				TwsSlaveFifoMuteTimeSet();

			}
			else
			{
				TwsSlaveFifoUnmuteSet();
			}
		}
#if 0	// 从机提示音直接使用主机发过来的音频数据，所以这段代码屏蔽掉
		else if(memcmp(param->params.twsData,cmd_remind,strlen(cmd_remind)) == 0)
		{
			if(IsBtTwsSlaveMode() && mainAppCt.MState == ModeStateWork)//保障 app running
			{
				RemindSoundServiceItemRequest((char *)&param->params.twsData[strlen(cmd_remind)], TRUE);
			}
		}
#endif
		else if(memcmp(param->params.twsData,cmd_repair,strlen(cmd_repair)) == 0)
		{
			APP_DBG("cmd_repair\n\n");
		
			BtTwsPeerEnterPairingMode();
		}
		else if(memcmp(param->params.twsData,cmd_vol,strlen(cmd_vol)) == 0)
		{
			bool ret = GIE_STATE_GET();
			GIE_DISABLE();
			uint8_t *temp = param->params.twsData;

			mainAppCt.MusicVolume = temp[strlen(cmd_vol) + 1];
			uint8_t MuteFlag = temp[strlen(cmd_vol) + 2];//mainAppCt.gSysVol.MuteFlag =
			if(ret)
			{
				GIE_ENABLE();
			}
			APP_DBG("slave rcv cmd_vol_mute:%u %u\n",mainAppCt.MusicVolume,mainAppCt.gSysVol.MuteFlag);

			if(MuteFlag)
			{
				if(!IsAudioPlayerMute())
				{
					AudioPlayerMute();
				}
				TwsSlaveFifoMuteTimeSet();
			}
			else 
			{
				TwsSlaveFifoUnmuteSet();
				AudioMusicVol(mainAppCt.MusicVolume);
				SystemVolSet();
			}
		}
#ifdef CFG_TWS_PEER_APP	
		else if(memcmp(param->params.twsData,cmd_ch_mode,strlen(cmd_ch_mode)) == 0)
		{
			bool ret = GIE_STATE_GET();
			GIE_DISABLE();
			uint8_t *temp = param->params.twsData;
//			TwsCfg.OutMode = temp[12];
			if(ret)
			{
				GIE_ENABLE();
			}
//			APP_DBG("slave rcv cmd_ch_mode:%u\n",TwsCfg.OutMode);
//			if(TwsCfg.OutMode == 0)
//			{
//				TwsCfg.MasterSound = 1;//0=L,R, 1=L, 2= R,3=L+R
//				TwsCfg.SlaverSound = 2;//0=L,R, 1=L, 2= R,3=L+R
//				APP_DBG("tws peer stereo mode\n");
//			}
//			else
//			{
//				TwsCfg.MasterSound = 3;//0=L,R, 1=L, 2= R,3=L+R
//				TwsCfg.SlaverSound = 3;//0=L,R, 1=L, 2= R,3=L+R
//				APP_DBG("tws peer mono mode\n");
//			}
		}
#endif	
	
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
		else if(memcmp(param->params.twsData,cmd_eq,strlen(cmd_eq)) == 0)
		{
			bool ret = GIE_STATE_GET();
			GIE_DISABLE();
			uint8_t *temp = param->params.twsData;
			mainAppCt.EqMode = temp[strlen(cmd_eq) + 1];
			if(ret)
			{
				GIE_ENABLE();
			}
			APP_DBG("slave rcv cmd_eq_mode = %u\n", mainAppCt.EqMode);
#ifndef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
			EqModeSet(mainAppCt.EqMode);
#endif		
		}
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
		else if(memcmp(param->params.twsData,cmd_music_treb_bass,strlen(cmd_music_treb_bass)) == 0)
		{
			bool ret = GIE_STATE_GET();
			GIE_DISABLE();
			uint8_t *temp = param->params.twsData;
			mainAppCt.MusicBassStep = temp[strlen(cmd_music_treb_bass) + 1];
			mainAppCt.MusicTrebStep = temp[strlen(cmd_music_treb_bass) + 2];
			if(ret)
			{
				GIE_ENABLE();
			}
			APP_DBG("slave rcv cmd_music_bass_treb = %u %u\n", mainAppCt.MusicBassStep,mainAppCt.MusicTrebStep);
			MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);		
		}
#endif
#if (BT_HFP_SUPPORT == ENABLE)
		else if(memcmp(param->params.twsData, cmd_hfp_state, strlen(cmd_hfp_state)) == 0)
		{
			bool ret = GIE_STATE_GET();
			GIE_DISABLE();
			uint8_t *temp = param->params.twsData;
			uint8_t val;
			val = temp[strlen(cmd_a2dp_state) + 1];
			if(ret)
			{
				GIE_ENABLE();
			}
			APP_DBG("slave rcv cmd_hfp_state = %d\n", val);
		}
#endif
		else if(memcmp(param->params.twsData, cmd_a2dp_state, strlen(cmd_a2dp_state)) == 0)
		{
			bool ret = GIE_STATE_GET();
			GIE_DISABLE();
			uint8_t *temp = param->params.twsData;
			uint8_t val;
			val = temp[strlen(cmd_a2dp_state) + 1];
			if(ret)
			{
				GIE_ENABLE();
			}
			APP_DBG("slave rcv cmd_a2dp_state = %d\n", val);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
//callback
void BtTwsCallback(BT_TWS_CALLBACK_EVENT event, BT_TWS_CALLBACK_PARAMS * param)
{
	//uint8_t tws_addr[6];
	switch(event)
	{
		//master
		case BT_STACK_EVENT_TWS_CONNECTED:
#if defined(CFG_APP_CONFIG) && defined(CFG_FUNC_REMIND_SOUND_EN)
			SoftFlagRegister(SoftFlagTwsRemind);
#endif
			if(param->role == BT_TWS_SLAVE)
			{
				BtTws_Slave_Connected(param);
			}
			else
			{
				BtTws_Master_Connected(param);
			}
			break;
			
		case BT_STACK_EVENT_TWS_DISCONNECT:
#if defined(CFG_APP_CONFIG) && defined(CFG_FUNC_REMIND_SOUND_EN)
			SoftFlagDeregister(SoftFlagTwsRemind);
#endif
			if(param->role == BT_TWS_SLAVE)
			{
				printf("slave dis\n");
				BtTws_Slave_Disconnected(param);
			}
			else
			{
				BtTws_Master_Disconnected(param);
			}
	#if (TWS_PAIRING_MODE == CFG_TWS_PEER_SLAVE)
			#include "main_task.h"
			if(GetSystemMode() == AppModeTwsSlavePlay)
			{
				MessageContext		msgSend;
				msgSend.msgId		= MSG_BT_TWS_DISCONNECT;
				MessageSend(GetTwsSlaveMessageHandle(), &msgSend);
			}
	#endif
			break;
			
		case BT_STACK_EVENT_TWS_DATA_IND:
			if(param->role == BT_TWS_SLAVE)
			{
				BtTws_Slave_RecvData(param);
			}
			else
			{
				BtTws_Master_RecvData(param);
			}
			break;

			
		case BT_STACK_EVENT_TWS_SLAVE_STREAM_START:
#ifdef CFG_APP_CONFIG
			tws_slave_fifo_clear();
#endif
			break;

		case BT_STACK_EVENT_TWS_SLAVE_STREAM_STOP:
			break;
			
		//当前TWS链路存在,则延时再次发起连接
		case BT_STACK_EVENT_TWS_CONNECT_DELAY:
			if(gBtTwsAppCt->btTwsPairingStart)
			{
				gBtTwsDelayConnectCnt = 50;//延时1s //20ms*50=1000ms=1s
			}
			break;
			
		default:
			break;
	}
}


//slave 接收到cmd: master当前处于active模式,需要退出sniff和deepsleep状态
void tws_master_active_mode(void)
{
	if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{
		if(Bt_sniff_sniff_start_state_get())
		{
			Bt_sniff_sniff_stop();
		}
	}
}


#include "gpio.h"
#include "irqn.h"
uint32_t tws_init_done = 0;
TaskHandle_t audio_core_handle;
char sample_buf[10];
uint8_t sample_count = 0;
uint32_t error_count = 0;
uint32_t adc_sample_count = 0;
uint32_t tws_delay;
void tws_set_delay(uint32_t delay,uint32_t sample);
uint32_t tws_audio_init(uint32_t cmd)
{
	if(cmd == 1)
	{
		//AudioDAC_DigitalMute(DAC0, TRUE, TRUE);
		tws_init_done = 0;
		tws_master_vol_send(mainAppCt.MusicVolume, IsAudioPlayerMute());
		
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
		tws_master_eq_mode_send(mainAppCt.EqMode);
#endif

#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
		tws_master_music_bass_treb_send(mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
#endif

		*(uint32_t*)0x40032008 = 0x001E0000;
		 sample_count = 0;
		 error_count = 0;
		 memset(sample_buf,0,sizeof(sample_buf));
	}
	else if(cmd == 3)
	{
		uint32_t delay= tws_get_delay();
		tws_set_delay(delay,mainAppCt.SamplesPreFrame);
		uint32_t ret = GIE_STATE_GET();
		GIE_DISABLE();
		Clock_Module1Disable(AUDIO_ADC1_CLK_EN);
		DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
		DMA_CircularFIFOClear(PERIPHERAL_ID_AUDIO_ADC1_RX);
		DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC0_RX);
		Clock_Module1Enable(AUDIO_ADC1_CLK_EN);
		tws_init_done = 1;
		adc_sample_count = 0;
		vTaskPrioritySet(audio_core_handle, 6);
		if(ret)
		{
			GIE_ENABLE();
		}
	}
	else if(cmd == 8)
	{
		return adc_sample_count + AudioADC1DataLenGet();
	}
	return 0;
}

uint32_t tws_audio_adjust(uint32_t m_len,uint32_t s_len)
{
	int tt = m_len-s_len;
	if(abs(tt) > 44)
	{
		error_count++;
		if(error_count > 5)
		{
			return 0;
		}
		return 1;
	}
	else
	{
		error_count = 0;;
	}

	//return 1;
	sample_buf[sample_count] = tt;
	sample_count++;
	if(sample_count >= 10)
	{
		sample_count = 0;
		int min = 127;
		int max = -127;
		int i;
		int sum=0;
		int min_index = 0;
		int max_index = 0;
		for(i=0;i<10;i++)
		{
			if(sample_buf[i] < min)
			{
				min = sample_buf[i];
				min_index = i;
			}
			if(sample_buf[i] > max)
			{
				max = sample_buf[i];
				max_index = i;
			}
		}
		if(min_index == 9)
		{
			sample_buf[9] = sample_buf[8];
		}
		else if(min_index == 0)
		{
			sample_buf[0] = sample_buf[1];
		}
		else
		{
			sample_buf[min_index] = (sample_buf[min_index-1] + sample_buf[min_index+1])/2;
		}

		if(max_index == 9)
		{
			sample_buf[9] = sample_buf[8];
		}
		else if(max_index == 0)
		{
			sample_buf[0] = sample_buf[1];
		}
		else
		{
			sample_buf[max_index] = (sample_buf[max_index-1] + sample_buf[max_index+1])/2;
		}
		for(i=0;i<10;i++)
		{
			printf("%d\n",sample_buf[i]);
			sum += sample_buf[i];
		}
		sum = sum/10;
		if(sum > 0)
		{
			*(uint32_t*)0x40032008 = 0x001E0000 + abs(sum)*3;
		}
		else
		{
			*(uint32_t*)0x40032008 = 0x001E0000 - abs(sum)*3;
		}
	}
	return 1;
}


bool tws_slave_switch_mode_enable(void)
{
#ifdef CFG_AUTO_ENTER_TWS_SLAVE_MODE
	return 1;
#else
	return 0;
#endif
}

//#else
//int tws_cmp_paired_mac(uint8_t *addr)
//{
//	return 1;
//}
//
//int tws_cmp_local_mac(uint8_t *addr)
//{
//	return 1;
//}
//
//int tws_inquiry_cmp_name(char* name, uint8_t len)
//{
//	return 1;
//}
//
//bool tws_connect_cmp(uint8_t * addr)
//{
//	return 1;
//}
//
//BT_TWS_ROLE tws_get_role(void)
//{
//	return BT_TWS_UNKNOW;
//}
//
//void tws_audio_init(uint32_t cmd)
//{
//}
//
//unsigned char tws_role_match(unsigned char *addr)
//{
//	return 0xff;
//}
//
//bool tws_slave_switch_mode_enable(void)
//{
//	return 0;
//}
//
//void tws_master_active_mode(void)
//{
//
//}
//int tws_filter_user_defined_infor_cmp(uint8_t *infor)
//{
//	return 0;
//}

#include "sbcenc_api.h"
#include "sbc_frame_decoder.h"

typedef struct {
	uint8_t nrof_blocks;
	uint8_t nrof_subbands;
	uint8_t mode;
	uint8_t bitpool;
}SBC_FRAME_INFO;
static const uint16_t freq_values[] =    { 16000, 32000, 44100, 48000 };
static const uint8_t block_values[] =    { 4, 8, 12, 16 };
static const uint8_t band_values[] =     { 4, 8 };
uint16_t CalculateFramelen(SBC_FRAME_INFO *frame)
{
	uint16_t nbits = frame->nrof_blocks * frame->bitpool;
	uint16_t nrof_subbands = frame->nrof_subbands;
	uint16_t result = nbits;

    if (frame->mode == 3) {
        result += nrof_subbands + (8 * nrof_subbands);
    } else {
        if (frame->mode == 1) { result += nbits; }
        if (frame->mode == 4) { result += 4*nrof_subbands; } else { result += 8*nrof_subbands; }
    }
    return 4 + (result + 7) / 8;
}

int sbc_get_fram_infor(uint8_t*data,uint32_t *fram_size,uint32_t *frequency)
{
	uint8_t d1;
	SBC_FRAME_INFO frame;
	memset(&frame,0,sizeof(SBC_FRAME_INFO));
	if(data[0] != 0x9C)
	{
		return 1;
	}
	d1 = data[1];
    frame.bitpool = data[2];///////////////////////////////
    *frequency = freq_values[((d1 & (0x80 | 0x40)) >> 6)];
	frame.nrof_blocks = block_values[((d1 & (0x20 | 0x10)) >> 4)];////////
	frame.mode = (d1 & (0x08 | 0x04)) >> 2;///////////////
	frame.nrof_subbands = band_values[(d1 & 0x01)];/////////////
	*fram_size = CalculateFramelen(&frame);
	return 0;
}


int32_t sbc_decoder_init(SBCFrameDecoderContext *ct)
{
	return sbc_frame_decoder_initialize(ct);
}

int32_t sbc_decoder_apply(SBCFrameDecoderContext *ct,uint8_t *sbc_buf,uint8_t sbc_size,int16_t *pcm_buf)
{
	int32_t ret = sbc_frame_decoder_decode(ct, sbc_buf, sbc_size);
	if(ret == 0)
	{
		memcpy(pcm_buf,ct->pcm,ct->num_channels*256);
	}
	else
	{

	}
	return ret;
}


int32_t sbc_encoder_init(SBCEncoderContext *ct,int32_t num_channels,SBC_ENC_QUALITY quality)
{
	int32_t samples_per_frame;
	return sbc_encoder_initialize(ct,num_channels,44100,16,quality,&samples_per_frame);
}
int32_t sbc_encoder_aplly(SBCEncoderContext *ct,int16_t *in_pcm,uint8_t *out_sbc,uint32_t *length)
{
	return sbc_encoder_encode(ct,in_pcm,out_sbc,length);
}

#endif
