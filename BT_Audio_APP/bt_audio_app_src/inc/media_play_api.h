/**
 **************************************************************************************
 * @file    media_service_api.h
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2017-3-17 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __MEDIA_SERVICE_H__
#define __MEDIA_SERVICE_H__

#include "type.h"
#include "app_config.h"
#include "ff.h"
#include "audio_decoder_api.h"
#include "decoder_service.h"
#include "ffpresearch.h"
#ifdef CFG_FUNC_LRC_EN
#include "lrc.h"
#endif
#ifdef CFG_FUNC_RECORD_FLASHFS
#include "file.h"
#endif
// Define device ID in system.
typedef	enum _DEV_ID
{
    DEV_ID_NONE,
    DEV_ID_USB,
    DEV_ID_SD,
    DEV_ID_FLASHFS,
} DEV_ID;

typedef enum _PLAYER_STATE
{
    PLAYER_STATE_IDLE = 0,    // ����
    PLAYER_STATE_PLAYING,     // ����
    PLAYER_STATE_PAUSE,       // ��ͣ
    PLAYER_STATE_FF,          // �����
    PLAYER_STATE_FB,          // ������
    PLAYER_STATE_STOP,
} PLAYER_STATE;


typedef enum _AUDIO_PLAYER_PLAY_MODE_
{
	PLAY_MODE_REPEAT_ALL,     // ȫ��˳��ѭ������
	PLAY_MODE_RANDOM_ALL,     // ȫ���������
	PLAY_MODE_REPEAT_ONE,     // ����ѭ������
	PLAY_MODE_REPEAT_FOLDER,  // �ļ���˳��ѭ������

	PLAY_MODE_RANDOM_FOLDER,  // �ļ����������
#ifdef FUNC_BROWSER_PARALLEL_EN
	PLAY_MODE_BROWSER,
#endif
	PLAY_MODE_PREVIEW_PLAY,   // Ԥ�����ţ����ļ�����У�ѡ�и�������Ԥ�����ż����ӣ�
	PLAY_MODE_REPEAT_OFF,
	PLAY_MODE_SUM,
} AUDIO_PLAYER_MODE;

#ifdef FUNC_SPECIFY_FOLDER_PLAY_EN
typedef enum _STORY_PLAY_SELECT
{
    STORY_PLAY_CURRENT = 1,    
    STORY_PLAY_NEXT,     
    
} STORY_PLAY_SELECT;
#endif
///////////////////////////////////////////////////////
// ����ģʽ�����ݽṹ������ϵͳ�����е����ĸ���ģʽ����(����ʱ����ѭ������)
#define REPEAT_CLOSED  0 // �����ر�
#define REPEAT_A_SETED 1 // �����ø������
#define REPEAT_OPENED  2 // ������
#define REPEAT_OPENED_PAUSE 3 //�����е���ͣ

typedef struct _REPEAT_AB_
{
	//���ǵ�ʵ��Ӧ�ã�0xFFFF��ԼҲ��18��Сʱ��starttime��times��̫�����õ�uint32_t�����Ըĳ�uint16_t����
	uint16_t StartTime; // ��ʼʱ��(S)������ʱ��ѡ�񸴶�ģʽ���������
	uint16_t Times;      // ����ʱ�䳤��(S)�������ͨ�����ý���ʱ��ķ�ʽ���������յĸ���ʱ���Դ�Ϊ׼����Ԥ��ֵ
	//uint8_t LoopCount;  // ѭ����������Ԥ������(EPROM)��Ĭ��3��
	uint8_t RepeatFlag; // ��ǰ�Ƿ񸴶�ģʽ(0 - �����ر�, 1 - ���������, 2 - �������յ㣬����ʼ����)
} REPEAT_AB;

///////////////////////////////////////////////////////
typedef struct _MEDIA_PLAYER_CONTROL_
{
	uint8_t		CurPlayState;		// ������״̬ (uint8_t)
    uint8_t		CurPlayMode;		// ��ǰ����ѭ��ģʽ
	bool		SongSwitchFlag;		// �л������ķ���Ĭ��0Ϊ��һ��
    
    uint32_t	CurPlayTime;		// ����ʱ��, ms

	FATFS		gFatfs_u;   		//U��/* File system object */
	FATFS 		gFatfs_sd;   		//SD��/* File system object */

	FIL			PlayerFile;         // ��ǰ�����ļ�
	uint16_t	CurFileIndex;		// ��ǰ�ļ��ţ������ȫ��ID��
	uint16_t	TotalFileSumInDisk;	// ȫ���е��ļ�����
	
	ff_dir		PlayerFolder;		// �ļ��о�����ɼ��������ز�����Ӧ�ò�Ҫ��ֵ��
	uint16_t	CurFolderIndex;		// ��Ҫ�򿪵��ļ���ȫ��ID��  //ע:��Ϊ��ǰ�ļ������ļ��� ����Ч�ļ������
	uint16_t	ValidFolderSumInDisk;//ȫ������Ч���ļ������������˿��ļ���(�����������ļ����ļ���)

	DecoderState DecoderSync;		//����״̬�Ǽ�

#ifdef CFG_FUNC_RECORD_SD_UDISK
	uint16_t*	RecFileList;		//¼���ļ����ļ�Index���򣬵�һ�����¡����256.
	DIR 		Dir;
	FILINFO 	FileInfo;
#elif defined(CFG_FUNC_RECORD_FLASHFS)
	MemHandle	FlashFsMemHandle;
	FFILE		*FlashFsFile;
#endif

	uint8_t*    AccRamBlk;        //ACC�ڴ����ָ�룬�����С��MAX_ACC_RAM_SIZE;

#ifdef CFG_FUNC_LRC_EN
	FIL			LrcFile;         	// LRC����ļ�
	uint8_t		LrcFlag;			// �����ʾ��־(�������)
	uint8_t		IsLrcRunning;
	LRC_ROW		LrcRow;
	uint8_t 	ReadBuffer[512];
	LRC_INFO	LrcInfo;
	uint8_t 	TempBuf1[128];
	uint8_t 	TempBuf2[255];
#endif

    REPEAT_AB	RepeatAB;			// ����ģʽ������Ϣ

//#ifdef PLAYER_SPECTRUM 				// Ƶ����ʾ
//	uint8_t		SpecTrumFlag;		// Ƶ�״򿪱�ʶ
//	SPECTRUM_INFO SpecTrumInfo;
//#endif
	uint16_t	ErrFileCount;		// �޷������������Ŀ�������û���folder�е���Ŀ���޷���������쳣�˳�����
//	uint16_t	error_times;		// ĳЩ�������������Ĵ������û��ָ�����
//	uint8_t		IsBPMached;			// �Ƿ�FS��ƥ���˶ϵ���Ϣ����Ҫ���ڷ�ֹ������Ч�Ķϵ���Ϣ(����ģʽ��ʼ��ʧ�ܺ���Ҫ�ı�־�ж��Ƿ񱣴�ϵ���Ϣ)

#ifdef FUNC_SPECIFY_FOLDER_PLAY_EN
	uint8_t		StorySelectPlayFlag;// 1:select curret play 2:next play
	uint8_t 	StoryPlayByIndexFlag;
#endif


} MEDIA_PLAYER;

extern MEDIA_PLAYER* gpMediaPlayer;

//�رղ����ļ�
void MediaPlayerCloseSongFile(void);
void MediaPlayerNextSong(bool IsSongPlayEnd);
void MediaPlayerPreSong(void);
void MediaPlayerPlayPause(void);
void MediaPlayerStop(void);
bool MediaPlayerInitialize(DEV_ID DeviceIndex, int32_t FileIndex, uint32_t FolderIndex);
void MediaPlayerDeinitialize(void);
void MediaPlayerSwitchPlayMode(uint8_t PlayMode);
//������ ��һ�ļ��� �趨
void MediaPlayerPreFolder(void);
//������ ��һ�ļ��� �趨
void MediaPlayerNextFolder(void);
void MediaPlayerTimeUpdate(void);
void MediaPlayerRepeatAB(void);
void MediaPlayerFastForward(void);
void MediaPlayerFastBackward(void);
void MediaPlayerFFFBEnd(void);
void MediaPlayerDecoderStop(void);
void MediaPlayerBackup(void);
bool MediaPlayerSongRefresh(void);
void MediaPlaySetDecoderState(DecoderState State);
DecoderState MediaPlayGetDecoderState(void);
uint8_t GetMediaPlayerState(void);
void SetMediaPlayerState(uint8_t state);
uint8_t GetMediaPlayMode(void);
void SetMediaPlayMode(uint8_t playmode);
void MediaPlayerBrowserEnter(void);
#if defined(FUNC_MATCH_PLAYER_BP) //&& (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN)) 
 void DiskSongSearchBPInit(void);
 void DiskSongSearchBP(DIR *dir, FILINFO *finfo, ff_acc_node *acc_node);
 uint32_t BPDiskSongNumGet(void);
 uint32_t BPDiskSongPlayTimeGet(void);
 uint32_t BPDiskSongPlayModeGet(void);
#endif
void MediaPlayerSongBrowserRefresh(void);
void MediaPlayerRepeatABClear(void);
void MediaPlayerTimerCB(void);
void MediaPlayerRepeatAB(void);


#endif /*__MEDIA_SERVICE_H__*/

