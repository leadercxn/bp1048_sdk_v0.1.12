///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: breakpoint.h
//  ChangLog :
//			�޸�bp ģ�������ʽ2014-9-26 ��lujiangang
///////////////////////////////////////////////////////////////////////////////
#ifndef __BREAKPOINT_H__
#define __BREAKPOINT_H__

#include "app_config.h"
#include "mode_switch_api.h"

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


#ifdef CFG_FUNC_BREAKPOINT_EN


#define BP_NVM_MAX_SIZE		16		//BP NVM ��Ϣ����ֽ���(B1)
#define BP_SET_ELEMENT(a, b) a = b
#define BP_GET_ELEMENT(x) x
#pragma pack(1)

//**************************************************************************************************************
//	PLAYER ��ضϵ�������ݽṹ
//	ע��:: �޸�BP_PLAYER_INFO �������sInitPlayerInfo ����
//**************************************************************************************************************

typedef struct _BP_PLAY_DISK_INFO_
{
	uint32_t DirSect; 		// �ļ�Ŀ¼������
	uint32_t FirstClust; 	// �ļ��״غ�
	uint32_t FileSize; 		// ��ʹ���ļ���С// �ļ���У����
	uint16_t PlayTime; 		// ����ʱ��

} BP_PLAY_DISK_INFO;

typedef struct _BP_PART_INFO_
{
	uint16_t CardPlayTime; 		// ����ʱ��
	uint16_t UDiskPlayTime; 	// ����ʱ��
}BP_PART_INFO;

typedef struct _BP_PLAYER_INFO_ // ����ģʽ ���ݴ洢�ṹ
{
	// ��������
	uint8_t	PlayerVolume; // Volume:0--32
	// ��ǰ����ģʽ
	uint8_t	PlayMode : 7; // Play mode
	// ��ʿ���
	uint8_t LrcFlag  : 1; // Lrc
	// ��������Ϣ

	BP_PLAY_DISK_INFO PlayCardInfo;
	BP_PLAY_DISK_INFO PlayUDiskInfo;
} BP_PLAYER_INFO;



//**************************************************************************************************************
//	Radio ��ضϵ�������ݽṹ
//	ע��:: �޸�BP_RADIO_INFO �������sInitRadioInfo ����
//**************************************************************************************************************
#ifdef CFG_APP_RADIOIN_MODE_EN
typedef struct _BP_RADIO_INFO_
{
	uint8_t		StationList[50]; 		// �ѱ����̨�б�/*MAX_RADIO_CHANNEL_NUM*/
	uint8_t		RadioVolume: 6;  		// FM����
	uint8_t		CurBandIdx : 2;  		// ��ǰFM���η�Χ(00B��87.5~108MHz (US/Europe, China)��01B��76~90MHz (Japan)��10B��65.8~73MHz (Russia)��11B��60~76MHz
	uint8_t		StationCount;    		// �ѱ����̨����
	uint16_t	CurFreq;         		// ��ǰ��̨Ƶ��
} BP_RADIO_INFO;
#endif


//**************************************************************************************************************
//	SYSTEM ��ضϵ�������ݽṹ
//	ע��:: �޸�BP_SYS_INFO �������sInitSysInfo ����
//**************************************************************************************************************
typedef struct _BP_SYS_INFO_
{
	// ��ǰӦ��ģʽ
	uint8_t 	CurModuleId; // system function mode.

	uint8_t  	MusicVolume;

	uint8_t     EffectMode;
    uint8_t     MicVolume;
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	uint8_t     EqMode;
#endif
    uint8_t  	ReverbStep;	
#ifdef CFG_FUNC_MIC_TREB_BASS_EN	
	uint8_t 	MicBassStep;
    uint8_t 	MicTrebStep;
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN	
	uint8_t 	MusicBassStep;
    uint8_t 	MusicTrebStep;
#endif	    
#ifdef CFG_FUNC_REMIND_SOUND_EN
	uint8_t 	SoundRemindOn;
	uint8_t 	LanguageMode;
#endif
#ifdef CFG_APP_BT_MODE_EN
	uint8_t 	HfVolume;
#endif
} BP_SYS_INFO;


//**************************************************************************************************************
//	Breakpoint �ϵ�������ݽṹ
//**************************************************************************************************************
typedef struct	_BP_INFO_
{
	BP_SYS_INFO    	SysInfo;
	
#if (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN)) // ����ģʽ���ݴ洢����
	BP_PLAYER_INFO PlayerInfo;
#endif

#ifdef CFG_APP_RADIOIN_MODE_EN // FMģʽ���ݴ洢����
	BP_RADIO_INFO 	RadioInfo;
#endif

	uint8_t 			Crc;
	
} BP_INFO;


typedef enum _BP_INFO_TYPE_
{
	BP_SYS_INFO_TYPE,
	BP_PLAYER_INFO_TYPE,
	BP_RADIO_INFO_TYPE,
}BP_INFO_TYPE;

typedef enum _BACKUP_MODE_
{
	BACKUP_SYS_INFO = 0,
	BACKUP_PLAYER_INFO,
#ifdef BP_PART_SAVE_TO_NVM
	BACKUP_PLAYER_INFO_2NVM, //�ϵ���䣬Ŀǰֻ�洢����ʱ��
#endif
	BACKUP_RADIO_INFO,
	BACKUP_ALL_INFO,
}BACKUP_MODE;
	

#pragma pack()

uint32_t BPDiskSongPlayModeGet(void);
void BP_InfoLog(void);
void BP_LoadInfo(void);
void* BP_GetInfo(BP_INFO_TYPE InfoType);
bool BP_SaveInfo(uint32_t Device);
uint32_t BPDiskFileResearch(AppMode DiskMode, uint16_t *PlayTime);
void BackupInfoUpdata(BACKUP_MODE mode);

#endif
#ifdef __cplusplus
}
#endif//__cplusplus

#endif/*__BREAKPOINT_H_*/

