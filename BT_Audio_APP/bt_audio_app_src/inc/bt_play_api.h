//bt_play_api.h
#include "type.h"

#ifndef __BT_PLAY_API_H__
#define __BT_PLAY_API_H__


typedef enum _BT_PLAYER_STATE
{
	BT_PLAYER_STATE_STOP = 0,    // 空闲
	BT_PLAYER_STATE_PLAYING,     // 播放
	BT_PLAYER_STATE_PAUSED,       // 暂停
	BT_PLAYER_STATE_FWD_SEEK,
	BT_PLAYER_STATE_REV_SEEK,
	
	BT_PLAYER_STATE_ERROR = 0xff,
} BT_PLAYER_STATE;


#define BT_SBC_PACKET_SIZE					595
#define BT_SBC_DECODER_INPUT_LEN			(10*1024)

#ifdef CFG_FUNC_SOFT_ADJUST_IN
#define BT_SOFT_ADJUST_LEVEL_L				(BT_SBC_DECODER_INPUT_LEN / 10 * 4)//4000//注意buf size，当前是10K
#define BT_SOFT_ADJUST_LEVEL_H				(BT_SBC_DECODER_INPUT_LEN / 10 * 7)//7000
#define BT_SBC_LEVEL_HIGH					(BT_SOFT_ADJUST_LEVEL_H)
#define BT_SBC_LEVEL_LOW					(BT_SOFT_ADJUST_LEVEL_L)

#define BT_AAC_LEVEL_HIGH					(BT_SOFT_ADJUST_LEVEL_H)
#define BT_AAC_LEVEL_LOW					(1024)

#define BT_AAC_FRAME_LEVEL_LOW				6
#else
#define BT_SBC_LEVEL_HIGH					(BT_SBC_DECODER_INPUT_LEN - BT_SBC_PACKET_SIZE * 4)
#define BT_SBC_LEVEL_LOW					(BT_SBC_LEVEL_HIGH  - BT_SBC_PACKET_SIZE * 3)
#define BT_AAC_LEVEL_HIGH					(BT_SBC_DECODER_INPUT_LEN - BT_SBC_PACKET_SIZE * 4)
#define BT_AAC_LEVEL_LOW					(1024)

#define BT_AAC_FRAME_LEVEL_LOW				6
#endif

uint32_t GetValidSbcDataSize(void);

uint32_t InsertDataToSbcBuffer(uint8_t * data, uint16_t dataLen);

void SetSbcDecoderStarted(bool flag);

bool GetSbcDecoderStarted(void);

int32_t SbcDecoderInit(void);

void BtSbcDecoderRefresh(void);


#ifdef CFG_APP_BT_MODE_EN
#ifdef CFG_FUNC_REMIND_SOUND_EN
#include "decoder_service.h"
bool BtPlayerBackup(void);
void SbcDecoderRefresh(void);
//void BtPlayerSetDecoderShare(DecoderShare State);
//DecoderShare BtPlayerGetDecoderShare(void);
#endif
#endif

int32_t SbcDecoderDeinit(void);

int32_t SbcDecoderStart(void);

void BtPlayerPlay(void);

void BtPlayerPause(void);

//播放&暂停
void BtPlayerPlayPause(void);

void BtAutoPlayMusic(void);
#endif

