/**
 *******************************************************************************
 * @file    bt_avrcp_api.h
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   AVRCP related api
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
* @defgroup bt_avrcp_api bt_avrcp_api.h
* @{
*/

#include "type.h"

#ifndef __BT_AVRCP_API_H__
#define __BT_AVRCP_API_H__


typedef enum{
	BT_STACK_EVENT_AVRCP_NONE = 0,
	BT_STACK_EVENT_AVRCP_CONNECTED,
	BT_STACK_EVENT_AVRCP_DISCONNECTED,
	BT_STACK_EVENT_AVRCP_ADV_PLAY_STATUS,
	BT_STACK_EVENT_AVRCP_ADV_PLAY_STATUS_CHANGED,
	BT_STACK_EVENT_AVRCP_ADV_TRACK_INFO,
	BT_STACK_EVENT_AVRCP_ADV_PLAY_POSITION,
	BT_STACK_EVENT_AVRCP_ADV_BATTERY_STATUS,
	BT_STACK_EVENT_AVRCP_ADV_APP_SETTINGS,
	BT_STACK_EVENT_AVRCP_ADV_PLAYING_CONTENT,
	BT_STACK_EVENT_AVRCP_ADV_AVAILABLE_PLAYERS,
	BT_STACK_EVENT_AVRCP_ADV_ADDRESSED_PLAYERS,
	BT_STACK_EVENT_AVRCP_ADV_UIDS,
	BT_STACK_EVENT_AVRCP_ADV_MEDIA_INFO,
	
	BT_STACK_EVENT_AVRCP_ADV_CAPABILITY_COMPANY_ID,
	BT_STACK_EVENT_AVRCP_ADV_CAPABILITY_EVENTS_SUPPORTED,

	BT_STACK_EVENT_AVRCP_ADV_VOLUME_CHANGE,
	
	BT_STACK_EVENT_AVRCP_ADV_LIST_PLAYER_SETTING_ATTRIBS,
	BT_STACK_EVENT_AVRCP_ADV_GET_PLAYER_SETTING_VALUE,
		
	//avrcp browse
	BT_STACK_EVENT_AVRCP_ADV_AVRCP_BROWSE_CONNECTED,
	BT_STACK_EVENT_AVRCP_ADV_AVRCP_BROWSE_DISCONNECT,
	BT_STACK_EVENT_AVRCP_ADV_AVRCP_SET_BROWSER_PLAYER,
	BT_STACK_EVENT_AVRCP_ADV_AVRCP_GET_FOLDER_ITEMS,
	BT_STACK_EVENT_AVRCP_ADV_AVRCP_CHANGE_PATH,
	BT_STACK_EVENT_AVRCP_ADV_AVRCP_GET_ITEM_ATTRIBUTES,
	BT_STACK_EVENT_AVRCP_ADV_AVRCP_SEARCH,

	BT_STACK_EVENT_AVRCP_PANEL_RELEASE,
	
	
}BT_AVRCP_CALLBACK_EVENT;

//////////////////////////////////////////////////////////////////////////////////
typedef uint8_t AvrcpPlayerAttrId;
	
#define AVRCP_PLAYER_EQ_STATUS        1   /* Player equalizer status */
#define AVRCP_PLAYER_REPEAT_STATUS    2   /* Player repeat status */
#define AVRCP_PLAYER_SHUFFLE_STATUS   3   /* Player Shuffle status */
#define AVRCP_PLAYER_SCAN_STATUS      4   /* Player scan status */
	
#define AVRCP_PLAYER_GET_REPEAT_STATUS		0x02
#define AVRCP_PLAYER_GET_SHUFFLE_STATUS		0x04
	
typedef uint8_t AvrcpEqValue;
	
#define AVRCP_EQ_OFF  1
#define AVRCP_EQ_ON   2
	
typedef uint8_t AvrcpRepeatValue;
	
#define AVRCP_REPEAT_OFF     1
#define AVRCP_REPEAT_SINGLE  2
#define AVRCP_REPEAT_ALL     3
#define AVRCP_REPEAT_GROUP   4
	
typedef uint8_t AvrcpShuffleValue;
	
#define AVRCP_SHUFFLE_OFF    1
#define AVRCP_SHUFFLE_ALL    2
#define AVRCP_SHUFFLE_GROUP  3
	
typedef uint8_t AvrcpScanValue;
	
#define AVRCP_SCAN_OFF    1
#define AVRCP_SCAN_ALL    2
#define AVRCP_SCAN_GROUP  3
	
typedef struct _AvrcpPlayerSetting 
{
	AvrcpPlayerAttrId	   attrId;	   /* Media Player Attribute ID */ 

	union {
		AvrcpEqValue	   eq;
		AvrcpRepeatValue   repeat;
		AvrcpShuffleValue  shuffle;
		AvrcpScanValue	   scan;
		uint8_t 	 extValue;

		uint8_t 			value;	   /* Used Internally */ 
	} setting;
}AvrcpPlayerSetting;

typedef struct _AvrcpPlayerSettingResp 
{
	uint16_t	  attrId;	  /* Media Player Attribute ID */ 

	AvrcpEqValue	   eq;
	AvrcpRepeatValue   repeat;
	AvrcpShuffleValue  shuffle;
	AvrcpScanValue	   scan;

}AvrcpPlayerSettingResp;

/*---------------------------------------------------------------------------
 * AvrcpMediaStatus type
 *
 * Defines play status of the currently playing media.
 */
typedef uint8_t AvrcpAdvMediaStatus;

#define AVRCP_ADV_MEDIA_STOPPED			0x00
#define AVRCP_ADV_MEDIA_PLAYING			0x01
#define AVRCP_ADV_MEDIA_PAUSED			0x02
#define AVRCP_ADV_MEDIA_FWD_SEEK		0x03
#define AVRCP_ADV_MEDIA_REV_SEEK		0x04
#define AVRCP_ADV_MEDIA_ERROR			0xFF


/*---------------------------------------------------------------------------
 * AvrcpMediaAttr structure
 * 
 * Used to describe media info attributes.
 */
typedef struct _OneAttrInfo
{
	uint32_t			attrId;		/**< Track attribute ID , TRACK_ATTR_XXXX */
	uint16_t			charSet;	/**< Character set ID, such as UTF8--0x006a */
	uint16_t			length;		/**< The length of property string */
	const char			*string;	/**< The property string */
} OneAttrInfo;

/**
 * AvrcpAdvMediaInfo structure
 * 
 * Used to describe media info .
 */
#define AVRCP_NUM_MEDIA_ATTRIBUTES		7
typedef struct
{
	/* The number of elements returned */ 
	uint8_t					numIds;

	/* An array of element value text information */ 
	OneAttrInfo				property[AVRCP_NUM_MEDIA_ATTRIBUTES];

} AvrcpAdvMediaInfo;

/*---------------------------------------------------------------------------
 * AvrcpAdvPlayStatus structure
 * 
 * Used to describe play status.
 */
typedef struct _AvrcpAdvPlayStatus
{
    uint32_t 				TotalLengthInMs;	//Total Song Length in ms
    uint32_t				CurPosInMs;			//Song Current Position in ms
	AvrcpAdvMediaStatus		CurPlayStatus;		//Current Play Status
}AvrcpAdvPlayStatus;

/*---------------------------------------------------------------------------
 * AvrcpAdvTranck structure
 * 
 * Used to describe media track .
 */
typedef struct _AvrcpAdvTrackStruct
{
	uint32_t			ms; /* The most significant 32 bits of the track index information.  */ 
	uint32_t			ls; /* The least significant 32 bits of the track index information.  */ 
}AvrcpAdvTrackStruct;

/*---------------------------------------------------------------------------
 * AvrcpBatteryStatus type
 *
 * Defines values for battery status.
 */
typedef uint8_t AvrcpAdvBatteryStatus;

#define AVRCP_ADV_BATT_STATUS_NORMAL		0
#define AVRCP_ADV_BATT_STATUS_WARNING		1
#define AVRCP_ADV_BATT_STATUS_CRITICAL		2
#define AVRCP_ADV_BATT_STATUS_EXTERNAL		3
#define AVRCP_ADV_BATT_STATUS_FULL_CHARGE	4

/*---------------------------------------------------------------------------
 * AvrcpAdvSystemStatus type
 *
 * Defines values for system status.
 */
typedef uint8_t AvrcpAdvSystemStatus;

#define AVRCP_ADV_SYS_POWER_ON		0
#define AVRCP_ADV_SYS_POWER_OFF		1
#define AVRCP_ADV_SYS_UNPLUGGED		2

/*---------------------------------------------------------------------------
 * AvrcpAdvEqValue type
 *
 * Defines values for the player equalizer status.
 */
typedef uint8_t AvrcpAdvEqValue;

#define AVRCP_ADV_EQ_OFF			1
#define AVRCP_ADV_EQ_ON				2

/* End of AvrcpAdvEqValue */ 

/*---------------------------------------------------------------------------
 * AvrcpAdvRepeatValue type
 *
 * Defines values for the player repeat mode status.
 */
typedef uint8_t AvrcpAdvRepeatValue;

#define AVRCP_ADV_REPEAT_OFF		1
#define AVRCP_ADV_REPEAT_SINGLE		2
#define AVRCP_ADV_REPEAT_ALL		3
#define AVRCP_ADV_REPEAT_GROUP		4

/* End of AvrcpAdvRepeatValue */ 

/*---------------------------------------------------------------------------
 * AvrcpShuffleAdvValue type
 *
 * Defines values for the player shuffle mode status.
 */
typedef uint8_t AvrcpAdvShuffleValue;

#define AVRCP_ADV_SHUFFLE_OFF		1
#define AVRCP_ADV_SHUFFLE_ALL		2
#define AVRCP_ADV_SHUFFLE_GROUP		3

/* End of AvrcpAdvShuffleValue */ 

/*---------------------------------------------------------------------------
 * AvrcpAdvScanValue type
 *
 * Defines values for the player scan mode status.
 */
typedef uint8_t AvrcpAdvScanValue;

#define AVRCP_ADV_SCAN_OFF			1
#define AVRCP_ADV_SCAN_ALL			2
#define AVRCP_ADV_SCAN_GROUP		3

/* End of AvrcpAdvScanValue */ 

typedef struct _AvrcpAdvAppSettings
{
	AvrcpAdvEqValue			eq;
	AvrcpAdvRepeatValue		repeat;
	AvrcpAdvShuffleValue	shuffle;
	AvrcpAdvScanValue		scan;
}AvrcpAdvAppSettings;

typedef struct _AvrcpAdvAddressedPlayer
{
	uint16_t		playerId;
	uint16_t		uidCounter;
}AvrcpAdvAddressedPlayer;


/*---------------------------------------------------------------------------
 * AvrcpAdvEventMask
 *
 * Bitmask of supported AVRCP events.  By default, only 
 * AVRCP_ENABLE_PLAY_STATUS_CHANGED and AVRCP_ENABLE_TRACK_CHANGED are 
 * enabled when a channel is registered.  The application must explicitly
 * enable any other supported events.

 */
typedef uint16_t AvrcpAdvEventMask;

#define AVRCP_ADV_ENABLE_PLAY_STATUS_EVENT			0x0001  /* Change in playback status */ 
#define AVRCP_ADV_ENABLE_TRACK_EVENT				0x0002  /* Current track changed */ 
#define AVRCP_ADV_ENABLE_TRACK_END_EVENT			0x0004  /* Reached end of track  */ 
#define AVRCP_ADV_ENABLE_TRACK_START_EVENT			0x0008  /* Reached track start   */ 
#define AVRCP_ADV_ENABLE_PLAY_POS_EVENT				0x0010  /* Change in playback position */ 
#define AVRCP_ADV_ENABLE_BATT_STATUS_EVENT			0x0020  /* Change in battery status */ 
#define AVRCP_ADV_ENABLE_SYS_STATUS_EVENT			0x0040  /* Change in system status */ 
#define AVRCP_ADV_ENABLE_APP_SETTING_EVENT			0x0080  /* Change in player application setting */ 
#define AVRCP_ADV_ENABLE_NOW_PLAYING_EVENT			0x0100  /* Change in the now playing list */
#define AVRCP_ADV_ENABLE_AVAIL_PLAYERS_EVENT		0x0200  /* Available players changed */
#define AVRCP_ADV_ENABLE_ADDRESSED_PLAYER_EVENT		0x0400  /* Addressed player changed */
#define AVRCP_ADV_ENABLE_UIDS_EVENT					0x0800  /* UIDS changed */
#define AVRCP_ADV_ENABLE_VOLUME_EVENT				0x1000  /* Volume Changed */

///////////////////////////////////////////////////////////////////////////////
//attrMask
#define AVRCP_ENABLE_PLAYER_EQ_STATUS			0x0001
#define AVRCP_ENABLE_PLAYER_REPEAT_STATUS		0x0002
#define AVRCP_ENABLE_PLAYER_SHUFFLE_STATUS		0x0004
#define AVRCP_ENABLE_PLAYER_SCAN_STATUS			0x0008

typedef struct _AvrcpAdvPlayerSettings
{
	/* Bitmask that describes which 
	* attributes are being reported 
	*/
	uint16_t				attrMask;

	/* The equalizer setting. */ 
	AvrcpAdvEqValue			eq;

	/* The repeat setting. */ 
	AvrcpAdvRepeatValue		repeat;

	/* The shuffle setting. */ 
	AvrcpAdvShuffleValue	shuffle;

	/* The scan setting. */ 
	AvrcpAdvScanValue		scan;
}AvrcpAdvPlayerSettings;

////////////////////////////////////////////////////////////////////////////
typedef union
{
	AvrcpAdvMediaStatus 	avrcpAdvMediaStatus;
	AvrcpAdvMediaInfo		*avrcpAdvMediaInfo;
	AvrcpAdvPlayStatus		avrcpAdvPlayStatus;
	AvrcpAdvTrackStruct 	avrcpAdvTrack;
	uint32_t				avrcpAdvPlayPosition;
	AvrcpAdvBatteryStatus	avrcpAdvBatteryStatus;
	AvrcpAdvSystemStatus	avrcpAdvSystemStatus;
	AvrcpAdvAppSettings 	avrcpAdvAppSettings;
	AvrcpAdvAddressedPlayer avrcpAdvAddressedPlayer;
	uint16_t 				avrcpAdvUidCounter;
	uint8_t					avrcpAdvVolumePercent;
	uint8_t					*avrcpAdvCompanyId;
	AvrcpAdvEventMask		avrcpAdvEventMask;
	AvrcpAdvPlayerSettings	avrcpAdvPlayerettingValue;
} AvrcpAdvParam;


///////////////////////////////////////////////////////////////////////////////////
//avrcp browse
/* Browsed Player. Valid when "advOp" is AVRCP_OP_SET_BROWSED_PLAYER */
typedef struct _AvrcpBrwsPlayer{
	uint16_t  uidCounter;  /* Current UID counter */
	uint32_t  numItems;	  /* Number of items in the current path */
	uint16_t  charSet;	  /* Character set used by the player */
	uint8_t	 fDepth;	  /* Number of folder length/name pairs to follow */
	uint8_t	*list;		  /* List of folder names */
} AvrcpBrwsPlayer;

/* Folder Items. Valid when "advOp" is AVRCP_OP_GET_FOLDER_ITEMS */
typedef struct _AvrcpFldrItems{
	uint16_t  uidCounter;  /* Current UID counter */
	uint32_t  numItems;	  /* Number of items in the current path */
	uint8_t	*list;		  /* List of items returned */
} AvrcpFldrItems;

/* Change Path. Valid when "advOp" is AVRCP_OP_CHANGE_PATH */
typedef struct _AvrcpChPath{
	uint32_t  numItems;	  /* Number of items in the current path */
} AvrcpChPath;

/* Item Attributes. Valid when "advOp" is AVRCP_OP_GET_ITEM_ATTRIBUTES */
typedef struct _AvrcpItemAttrs{
	uint8_t	 numAttrs;	  /* Number of attributes returned */
	uint8_t	*list;		  /* List of attributes returned */
} AvrcpItemAttrs;

/* Search. Valid when "advOp" is AVRCP_OP_SEARCH */
typedef struct _AvrcpSearch{
	uint16_t  uidCounter;  /* Current UID counter */
	uint32_t  numItems;	  /* Number of items found in the search */
} AvrcpSearch;

/*---------------------------------------------------------------------------
 * AvrcpItemType type
 * 
 * The type of an media item.
 */
typedef unsigned char AvrcpItemType;

#define AVRCP_ITEM_MEDIA_PLAYER  0x01
#define AVRCP_ITEM_FOLDER        0x02
#define AVRCP_ITEM_MEDIA_ELEMENT 0x03

/*---------------------------------------------------------------------------
 * 
 * 
 */
typedef struct _BT_AVRCP_CALLBACK_PARAMS
{
	uint16_t			paramsLen;
	bool				status;
	uint16_t			errorCode;

	union
	{
		uint8_t					*bd_addr;
		uint16_t				panelKey;
		AvrcpAdvParam			avrcpAdv;
		AvrcpBrwsPlayer			*brwsPlayer;
		AvrcpFldrItems			*fldItems;
		AvrcpChPath				*chPath;
		AvrcpItemAttrs			*itemAttrs;
		AvrcpSearch				*search;
	}params;
}BT_AVRCP_CALLBACK_PARAMS;

typedef void (*BTAvrcpCallbackFunc)(BT_AVRCP_CALLBACK_EVENT event, BT_AVRCP_CALLBACK_PARAMS * param);

void BtAvrcpCallback(BT_AVRCP_CALLBACK_EVENT event, BT_AVRCP_CALLBACK_PARAMS * param);

typedef struct _AvrcpAppFeatures
{
	bool		supportAdvanced;	/*!< Advanced AVRCP includes songs info and playing status report...*/
	bool		supportTgSide;		/*!< TG side includes volume synchronization (remote volume control) */
	bool		supportSongTrackInfo;
	bool		supportPlayStatusInfo;
	BTAvrcpCallbackFunc		avrcpAppCallback;
}AvrcpAppFeatures;


/**
 * @brief  User Command: AVRCP connect command.
 * @param  addr The address of bt device to connect.
 * @return True for the command implement successful 
 * @Note If the device has been connected with a2dp profile,
 * 		the event BT_STACK_EVENT_AVRCP_CONNECTED will be received in the callback.
 * 		Otherwise BT_STACK_EVENT_PAGE_TIMEOUT will be received.
 *
 */
bool AvrcpConnect(uint8_t * addr);


/**
 * @brief  User Command: AVRCP disconnect command.
 * @param  None
 * @return True for the command implement successful 
 * @Note If the device has been disconnected.
 * 		The event BT_STACK_EVENT_AVRCP_DISCONNECTED will be received in the callback.
 */
bool AvrcpDisconnect(void);

/**
 * @brief  User Command: Send 'Play' command to the target
 * @param  None
 * @return True for the command implement successful 
 * @Note 
 */
bool AvrcpCtrlPlay(void);
#define BTCtrlPlay		AvrcpCtrlPlay

/**
 * @brief  User Command: Send 'Pause' command to the target
 * @param  None
 * @return True for the command implement successful 
 * @Note 
 */
bool AvrcpCtrlPause(void);
#define BTCtrlPause		AvrcpCtrlPause

/**
 * @brief  User Command: Send 'Stop' command to the target
 * @param  None
 * @return True for the command implement successful 
 * @Note 
 */
bool AvrcpCtrlStop(void);
#define BTCtrlStop		AvrcpCtrlStop

/**
 * @brief  User Command: Send 'Next' command to the target
 * @param  None
 * @return True for the command implement successful 
 * @Note 
 */
bool AvrcpCtrlNext(void);
#define BTCtrlNext		AvrcpCtrlNext

/**
 * @brief  User Command: Send 'Previous' command to the target
 * @param  None
 * @return True for the command implement successful 
 * @Note 
 */
bool AvrcpCtrlPrev(void);
#define BTCtrlPrev		AvrcpCtrlPrev

/**
 * @brief  User Command: Send 'FF' press command to the target
 * @param  None
 * @return True for the command implement successful 
 * @Note 
 */
bool AvrcpCtrlPressFastFoward(void);
#define BTCtrlFF		AvrcpCtrlPressFastFoward

/**
 * @brief  User Command: Send 'FF' release command to the target
 * @param  None
 * @return True for the command implement successful 
 * @Note 
 */
bool AvrcpCtrlReleaseFastFoward(void);
#define BTCtrlEndFF		AvrcpCtrlReleaseFastFoward


/**
 * @brief  User Command: Send 'FB' press command to the target
 * @param  None
 * @return True for the command implement successful 
 * @Note 
 */
bool AvrcpCtrlFastBackward(void);
#define BTCtrlFB		AvrcpCtrlFastBackward

/**
 * @brief  User Command: Send 'FB' release command to the target
 * @param  None
 * @return True for the command implement successful 
 * @Note 
 */
bool AvrcpCtrlReleaseFastBackward(void);
#define BTCtrlEndFB		AvrcpCtrlReleaseFastBackward

/**
 * @brief  User Command: Get Play Status information
 * @param  None
 * @return True for the command implement successful 
 * @Note   callback info includes: Play Status, Total Song Length, Song Current Position
 */
bool AvrcpCtrlGetPlayStatusInfo(void);
#define BTCtrlGetPlayStatus		AvrcpCtrlGetPlayStatusInfo

/**
 * @brief  User Command: Set Volume
 * @param  volume percent
 * @return True for the command implement successful 
 * @Note   callback info includes: 
 */
bool AvrcpCtrlSetVolume(uint8_t volumePercent);
#define BTCtrlSetVol		AvrcpCtrlSetVolume

/**
 * @brief  User Command: Get Media Information
 * @param  None
 * @return True for the command implement successful 
 * @Note   the event BT_STACK_EVENT_AVRCP_ADV_MEDIA_INFO will be received in the callback.
 * 			callback info includes: title, artist, album, number, genre
 */
bool AvrcpCtrlGetMediaInfo(void);
#define BTCtrlGetMediaInfor		AvrcpCtrlGetMediaInfo
 

/**
 * @brief  sync volume(avrcp target)
 * @param  volume (range: 0-0x7f)
 * @return NONE
 * @Note   
 */
void AvrcpAdvTargetSyncVolumeConfig(uint8_t volume);

//////////////////////////////////////////////////////////////////////////////
/**
 * @brief  Set Player Setting Values
 * @param  repeat mode / mode param
 * @return True for the command implement successful 
 * @Note   callback info includes: 
 */
bool AvrcpCtrlSetPlayerSettingValues(uint8_t attrId, uint8_t setting);
#define BTCtrlSetPlayerSettingValues		AvrcpCtrlSetPlayerSettingValues

/**
 * @brief  Get Player Setting Values
 * @param  attribute mask
 * @return True for the command implement successful 
 * @Note   callback info includes: 
 */
bool AvrcpCtrlGetPlayerSettingValues(uint16_t attrMask);
#define BTCtrlGetPlayerSettingValues		AvrcpCtrlGetPlayerSettingValues

//////////////////////////////////////////////////////////////////////////////
//avrcp browse
/**
 * @brief
 *  set brower player
 *	获取当前文件文件夹名称/深度
 *
 * @param 
 *		playerId(default:1)
 *
 * @return
 *		success
 *
 * @note
 *		通过BT_STACK_EVENT_AVRCP_ADV_AVRCP_SET_BROWSER_PLAYER反馈数据
 *	
 */
unsigned char AvrcpCtrlAdvAvrcpSetBrowsedPlayer(unsigned short playerId);
#define BTCtrlAdvAvrcpSetBrowsedPlayer		AvrcpCtrlAdvAvrcpSetBrowsedPlayer

/**
 * @brief
 *  get folder items
 *	获取当前文件夹内文件的信息
 *
 * @param 
 *		startItem/endItem (保持一致,每次获取1个)
 *
 * @return
 *		success
 *
 * @note
 *		通过BT_STACK_EVENT_AVRCP_ADV_AVRCP_GET_FOLDER_ITEMS反馈数据
 *
 */
unsigned char AvrcpCtrlAdvAvrcpGetFolderItems(unsigned int startItem, unsigned int endItem);
#define BTCtrlAdvAvrcpGetFolderItems		AvrcpCtrlAdvAvrcpGetFolderItems

/**
 * @brief
 *  change path
 *	改变文件目录
 *
 * @param 
 *		direction: 0x01=down, 0x00=up
 *		uid:folder uid信息(进入下一个目录时有效,返回上一个目录时可以随意)
 *
 * @return
 *		success
 *
 * @note
 *		通过BT_STACK_EVENT_AVRCP_ADV_AVRCP_CHANGE_PATH反馈数据
 *
 */
unsigned char AvrcpCtrlAdvAvrcpChangePath(unsigned char direction, unsigned char *uid);
#define BTCtrlAdvAvrcpChangePath		AvrcpCtrlAdvAvrcpChangePath

/**
 * @brief
 *  play item
 *	通过UID信息播放歌曲
 *
 * @param 
 *		uid:media element uid信息
 *
 * @return
 *		success
 *
 * @note
 *		
 *
 */
unsigned char AvrcpCtrlAdvAvrcpPlayItem(unsigned char *uid);
#define BTCtrlAdvAvrcpPlayItem		AvrcpCtrlAdvAvrcpPlayItem



#endif //__BT_AVRCP_API_H__

/**
 * @}
 * @}
 */
 

