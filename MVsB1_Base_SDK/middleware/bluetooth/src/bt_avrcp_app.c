/**
 *******************************************************************************
 * @file    bt_avrcp_app.c
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   Avrcp callback events and actions
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
#include "bt_manager.h"
#include "timeout.h"
#ifdef	FUNC_OS_EN
#include "rtos_api.h"
#endif
#include "bt_app_interface.h"
#ifdef CFG_APP_CONFIG
#include "bt_play_api.h"
#include "app_config.h"
#endif
#include "bt_config.h"
#include "string_convert.h"

TIMER	AvrcpConnectTimer;

#if (BT_AVRCP_SUPPORT == ENABLE)
static void SetAvrcpState(BT_AVRCP_STATE state);
extern uint8_t AudioMusicVolGet(void);
extern uint32_t gSpecificDevice;

#if (BT_AVRCP_BROWSER_FUNC == ENABLE)
//avrcp browse
//可供参考
uint32_t TotalNumItems = 0; //当前目录下文件数量(song+dirctory)
uint8_t TotalFolderDepth = 1; //当前文件夹的深度
uint8_t CurUid[8] = {0};	//当前选定的文件的UID信息(media element/folder)
uint16_t CurUidCount = 0;
uint8_t MediaPlayUid[8];
uint16_t BTFolderIndex=0;

static uint16_t B8toHost16(const uint8_t* ptr)
{
	return (uint16_t)( ((uint16_t) *ptr << 8) | ((uint16_t) *(ptr+1)) ); 
}
#endif

void BtAvrcpCallback(BT_AVRCP_CALLBACK_EVENT event, BT_AVRCP_CALLBACK_PARAMS * param)
{
	switch(event)
	{

		case BT_STACK_EVENT_AVRCP_CONNECTED:
			{
				APP_DBG("Avrcp Connected : bt address = %02x:%02x:%02x:%02x:%02x:%02x\n",
						(param->params.bd_addr)[0],
						(param->params.bd_addr)[1],
						(param->params.bd_addr)[2],
						(param->params.bd_addr)[3],
						(param->params.bd_addr)[4],
						(param->params.bd_addr)[5]);
#ifdef BT_TWS_SUPPORT				
				if((btManager.twsState == BT_TWS_STATE_CONNECTED)&&(btManager.twsRole == BT_TWS_SLAVE))
				{
					//从机已经组网成功,此时被手机连接上,则断开手机
					BTHciDisconnectCmd(param->params.bd_addr);
					break;
				}
#endif
				SetAvrcpState(BT_AVRCP_STATE_CONNECTED);

				if((param->params.bd_addr)[0] || (param->params.bd_addr)[1] || (param->params.bd_addr)[2] 
					|| (param->params.bd_addr)[3] || (param->params.bd_addr)[4] || (param->params.bd_addr)[5])
				{
					memcpy(GetBtManager()->remoteAddr, param->params.bd_addr, 6);
				}

				SetBtConnectedProfile(BT_CONNECTED_AVRCP_FLAG);

#if (BT_AUTO_PLAY_MUSIC == ENABLE)
				BtAutoPlayMusic();
#endif

#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
				GetBtManager()->avrcpSyncEnable = 0;
				GetBtManager()->avrcpSyncVol = 0xff; //初始值
				//AvrcpAdvTargetSyncVolumeConfig(AudioMusicVolGet()); //蓝牙AVRCP连接成功后，将当前的音量值同步到AVRCP adv.volume默认值
				AvrcpAdvTargetSyncVolumeConfig(BtLocalVolLevel2AbsVolme(AudioMusicVolGet()));
#endif
#ifdef BT_RECONNECTION_FUNC
				BtStartReconnectProfile();
#endif
				BtLinkStateConnect();
			}
			break;

		case BT_STACK_EVENT_AVRCP_DISCONNECTED:
			{
				APP_DBG("Avrcp disconnect\n");
				SetAvrcpState(BT_AVRCP_STATE_NONE);
				
				SetBtDisconnectProfile(BT_CONNECTED_AVRCP_FLAG);
				
				BtLinkStateDisconnect();
				//AVRCP断开机制清除
				if(btCheckEventList&BT_EVENT_AVRCP_DISCONNECT)
				{
					btCheckEventList &= ~BT_EVENT_AVRCP_DISCONNECT;
					btEventListB0Count = 0;
				}
			}
			break;

		case BT_STACK_EVENT_AVRCP_ADV_PLAY_STATUS_CHANGED:
			{
//				extern uint32_t a2dp_avrcp_connect_flag;
				APP_DBG("A2DP Play State [%d]\n", param->params.avrcpAdv.avrcpAdvMediaStatus);
#if (BT_HFP_SUPPORT == ENABLE)
				if((param->params.avrcpAdv.avrcpAdvMediaStatus == 1)&&(gSpecificDevice))
				{
					extern void SpecialDeviceFunc(void);
					SpecialDeviceFunc();
				}
#endif
				/*if((a2dp_avrcp_connect_flag)&&(param->params.avrcpAdv.avrcpAdvMediaStatus == 1))
				{
					a2dp_avrcp_connect_flag = 0;
					break;
				}*/
				
				/*if((GetA2dpState() != BT_A2DP_STATE_STREAMING))
				{
					BtMidMessageSend(MSG_BT_MID_PLAY_STATE_CHANGE, 2);
				}
				else
					*///bkd del for press key to play,but STREAMING command not received 
				{
					BtMidMessageSend(MSG_BT_MID_PLAY_STATE_CHANGE, param->params.avrcpAdv.avrcpAdvMediaStatus);
					
					if(CheckTimerStart_BtPlayStatus())
					{
					}
					else
					{
						if(param->params.avrcpAdv.avrcpAdvMediaStatus == AVRCP_ADV_MEDIA_PLAYING)
						{
							BTCtrlGetPlayStatus();
							TimerStart_BtPlayStatus();
						}
					}
				}
			}
			break;

#if (BT_AVRCP_ADVANCED == ENABLE)
#if (BT_AVRCP_SONG_PLAY_STATE == ENABLE)
		case BT_STACK_EVENT_AVRCP_ADV_PLAY_STATUS:
			{
				uint8_t curPlayStatus = param->params.avrcpAdv.avrcpAdvPlayStatus.CurPlayStatus;
				uint32_t curPlayTimes = param->params.avrcpAdv.avrcpAdvPlayStatus.CurPosInMs;
				uint32_t tatolPlayTimes = param->params.avrcpAdv.avrcpAdvPlayStatus.TotalLengthInMs;
				
				switch(curPlayStatus)
				{
					case AVRCP_ADV_MEDIA_STOPPED:
						APP_DBG("Stopped \n");
						break;
						
					case AVRCP_ADV_MEDIA_PLAYING:
						APP_DBG("Playing > %02d:%02d / %02d:%02d \n",
								((int)curPlayTimes/1000)/60,
								((int)curPlayTimes/1000)%60,
								((int)tatolPlayTimes/1000)/60,
								((int)tatolPlayTimes/1000)%60);

						if(!(curPlayTimes/1000%5))
							BTCtrlGetMediaInfor();
						
						break;

					case AVRCP_ADV_MEDIA_PAUSED:
						APP_DBG("Paused || %02d:%02d / %02d:%02d  \n",
                        		((int)curPlayTimes/1000)/60,
                        		((int)curPlayTimes/1000)%60,
                        		((int)tatolPlayTimes/1000)/60,
                        		((int)tatolPlayTimes/1000)%60);
						break;

					case AVRCP_ADV_MEDIA_FWD_SEEK:
						APP_DBG("FF >> %02d:%02d / %02d:%02d  \n",
                        		((int)curPlayTimes/1000)/60,
                        		((int)curPlayTimes/1000)%60,
                        		((int)tatolPlayTimes/1000)/60,
                        		((int)tatolPlayTimes/1000)%60);
						break;

					case AVRCP_ADV_MEDIA_REV_SEEK:
						APP_DBG("FB << %02d:%02d / %02d:%02d  \n",
                        		((int)curPlayTimes/1000)/60,
                        		((int)curPlayTimes/1000)%60,
                        		((int)tatolPlayTimes/1000)/60,
                        		((int)tatolPlayTimes/1000)%60);
						break;

					default:
						break;
				}
			}
			break;
#endif

		case BT_STACK_EVENT_AVRCP_ADV_TRACK_INFO:
			//APP_DBG("TRACK_INFO\n");
			/*{
				APP_DBG("Playing > %02d:%02d / %02d:%02d \n",
            		(param->params.avrcpAdv.avrcpAdvTrack.ms/1000)/60,
            		(param->params.avrcpAdv.avrcpAdvTrack.ms/1000)%60,
            		(param->params.avrcpAdv.avrcpAdvTrack.ls/1000)/60,
            		(param->params.avrcpAdv.avrcpAdvTrack.ls/1000)%60);
			}*/
			break;

		case BT_STACK_EVENT_AVRCP_ADV_ADDRESSED_PLAYERS:
			//APP_DBG("AVRCP Event: BT_STACK_EVENT_AVRCP_ADV_ADDRESSED_PLAYERS\n");
			//APP_DBG("PlayId[%x],UidCount[%x]\n", param->params.avrcpAdv.avrcpAdvAddressedPlayer.playerId, param->params.avrcpAdv.avrcpAdvAddressedPlayer.uidCounter);
			break;

		case BT_STACK_EVENT_AVRCP_ADV_CAPABILITY_COMPANY_ID:
			/*APP_DBG("COMPANY_ID: ");
			{
				uint8_t len;
				for(len=0; len< param->paramsLen; len++)
				{
					APP_DBG("[0x%02x] ",param->params.avrcpAdv.avrcpAdvCompanyId[len]);
				}
			}
			APP_DBG("\n");*/
			break;
			
		case BT_STACK_EVENT_AVRCP_ADV_CAPABILITY_EVENTS_SUPPORTED:
			//APP_DBG("AVRCP SUPPORTED FEATRUE = [0x%04x]\n", param->params.avrcpAdv.avrcpAdvEventMask);
			break;

		case BT_STACK_EVENT_AVRCP_ADV_GET_PLAYER_SETTING_VALUE:
#if (BT_AVRCP_PLAYER_SETTING == ENABLE)
			APP_DBG("attrId:%x, ", param->params.avrcpAdv.avrcpAdvPlayerettingValue.attrMask);
			if(param->params.avrcpAdv.avrcpAdvPlayerettingValue.attrMask == AVRCP_ENABLE_PLAYER_EQ_STATUS)
				APP_DBG("EQ:%x\n", param->params.avrcpAdv.avrcpAdvPlayerettingValue.eq);
			else if(param->params.avrcpAdv.avrcpAdvPlayerettingValue.attrMask == AVRCP_ENABLE_PLAYER_REPEAT_STATUS)
				APP_DBG("repeat:%x\n", param->params.avrcpAdv.avrcpAdvPlayerettingValue.repeat);
			else if(param->params.avrcpAdv.avrcpAdvPlayerettingValue.attrMask == AVRCP_ENABLE_PLAYER_SHUFFLE_STATUS)
				APP_DBG("shuffle:%x\n", param->params.avrcpAdv.avrcpAdvPlayerettingValue.shuffle);
			else if(param->params.avrcpAdv.avrcpAdvPlayerettingValue.attrMask == AVRCP_ENABLE_PLAYER_SCAN_STATUS)
				APP_DBG("scan:%x\n", param->params.avrcpAdv.avrcpAdvPlayerettingValue.scan);
#endif
			break;

#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
		case BT_STACK_EVENT_AVRCP_ADV_VOLUME_CHANGE:
			//APP_DBG("BTVOL_CHANGE = [%d]\n", param->params.avrcpAdv.avrcpAdvVolumePercent);
			GetBtManager()->avrcpSyncEnable = 1;
			{
				uint16_t VolumePercent = 0;//param->params.avrcpAdv.avrcpAdvVolumePercent;
				//VolumePercent = VolumePercent*CFG_PARA_MAX_VOLUME_NUM/100;

				//GetBtManager()->avrcpSyncVol = VolumePercent;
				VolumePercent = BtAbsVolume2VolLevel(param->params.avrcpAdv.avrcpAdvVolumePercent);
				APP_DBG("BTVOL_CHANGE = [%d]\n", VolumePercent);
				
				BtMidMessageSend(MSG_BT_MID_VOLUME_CHANGE, (uint8_t)VolumePercent);
			}
			break;
#endif

#if (BT_AVRCP_SONG_TRACK_INFOR == ENABLE)
		case BT_STACK_EVENT_AVRCP_ADV_MEDIA_INFO:
			APP_DBG("AVRCP Event: BT_STACK_EVENT_AVRCP_ADV_MEDIA_INFO\n");

			if(GetMediaInfo)
				GetMediaInfo((void*)param->params.avrcpAdv.avrcpAdvMediaInfo);

			break;
#endif
#endif

#if (BT_AVRCP_BROWSER_FUNC == ENABLE)
///////////////////////////////////////////////////////////////////////////////////////////
//avrcp browse
		case BT_STACK_EVENT_AVRCP_ADV_AVRCP_BROWSE_CONNECTED:
			APP_DBG("avrcp browse is connected\n");
			//连接成功后重置browse相关寄存器
			TotalNumItems = 0;
			TotalFolderDepth = 1;
			CurUidCount = 0;
			memset(CurUid, 0, 8);
			break;

		case BT_STACK_EVENT_AVRCP_ADV_AVRCP_BROWSE_DISCONNECT:
			APP_DBG("avrcp browse disconnect\n");
			break;
			
		case BT_STACK_EVENT_AVRCP_ADV_AVRCP_SET_BROWSER_PLAYER:
			APP_DBG("UI : BT_UI_CB_STATUS_ADV_AVRCP_SET_BROWSER_PLAYER\n");
			{
				uint16_t listLen=0;
				#define StringMaxLen 30
				uint8_t ConvertStringData[StringMaxLen];
				uint8_t StringData[StringMaxLen]={0};
				
				uint16_t offset=0;
				uint8_t i;
				
				APP_DBG("UID Count:0x%04x, Number of Items:0x%08x, CharSet:0x%04x, Folder Depth:%x\n",
	                    			param->params.brwsPlayer->uidCounter,
	                    			param->params.brwsPlayer->numItems,
	                    			param->params.brwsPlayer->charSet,
	                    			param->params.brwsPlayer->fDepth);
	                    			
				TotalNumItems = param->params.brwsPlayer->numItems;
				TotalFolderDepth = param->params.brwsPlayer->fDepth;
				
	            //显示当前文件夹的名称(最深层次)
	            //1.获取最后1个文件夹 offset
	            offset = 0;
	            if(param->params.brwsPlayer->fDepth>1)
	            {
		            for(i=0; i<(param->params.brwsPlayer->fDepth-1);i++)
		            {
		            	offset += B8toHost16(param->params.brwsPlayer->list+offset) + 2;
		            }
		        }

	            //2.解析文件夹信息
	            listLen = B8toHost16(param->params.brwsPlayer->list+offset); 
	            APP_DBG("Folder Length:%d \nName:", listLen);
	            memcpy(StringData, (param->params.brwsPlayer->list+2), listLen);
	            #ifdef CFG_FUNC_STRING_CONVERT_EN
					StringConvert(ConvertStringData, StringMaxLen, StringData, listLen ,UTF8_TO_GBK);
					APP_DBG("%s\n", ConvertStringData);
				#endif
			}
			break;

		case BT_STACK_EVENT_AVRCP_ADV_AVRCP_GET_FOLDER_ITEMS:
			APP_DBG(" UI : BT_UI_CB_STATUS_ADV_AVRCP_GET_FOLDER_ITEMS\n");
			{
				uint16_t offset=0;
				AvrcpItemType itemType;
				uint8_t i;
				#define StringMaxLen 60
				uint8_t ConvertStringData[StringMaxLen];
				uint8_t StringData[StringMaxLen]={0};
				uint16_t listLen=0;
				
				APP_DBG("uidCount:%x, numItems:%x\n", param->params.fldItems->uidCounter, param->params.fldItems->numItems);

				if(param->params.fldItems->numItems == 0)
					break;
				
				//if(info->numItems == 1)
				{
					//解析数据
					itemType = param->params.fldItems->list[offset++];
				    //itemLen = B8toHost16(info->list+offset);
				    offset += 2;
				    
					if(itemType == AVRCP_ITEM_FOLDER)
					{
						APP_DBG("TYPE:DIRECTORY\n");

						memcpy(CurUid, param->params.fldItems->list+offset, 8);
						APP_DBG("FolderUid:");
						for(i=0;i<8;i++)
						{
							APP_DBG("%02x ", CurUid[i]);
						}
						APP_DBG("\n");
						
				        offset += 8;
				        //mediaItem.item.folder.folderType = info->list[offset++];
				        offset++;
				        //mediaItem->item.folder.isPlayable = info->list[offset++];
				        offset++;
				        //mediaItem->item.folder.charSet = BEtoHost16(info->list+offset);
				        offset += 2;
				        
				        //folder name length
				        listLen = B8toHost16(param->params.fldItems->list+offset);
				        offset += 2;
				        
				        //folder name info
        				memcpy(StringData, (param->params.fldItems->list+offset), listLen);

						//listLen = mediaItem.item.folder.nameLen;
						APP_DBG("Name Len:%x\n", listLen);
						//memcpy(StringData, mediaItem.item.folder.name, listLen);
						#ifdef CFG_FUNC_STRING_CONVERT_EN
							StringConvert(ConvertStringData, StringMaxLen, StringData, listLen ,UTF8_TO_GBK);
							APP_DBG("%s\n", ConvertStringData);
						#endif
						
					}
					else if(itemType == AVRCP_ITEM_MEDIA_ELEMENT)
					{
						APP_DBG("TYPE:SONG\n");
						
						memcpy(CurUid, param->params.fldItems->list+offset, 8);
				        APP_DBG("MediaElementUid:");
						for(i=0;i<8;i++)
						{
							APP_DBG("%02x ", CurUid[i]);
						}
						APP_DBG("\n");
						offset += 8;
				        //mediaItem->item.element.mediaType = list[offset++];
				        offset++;
				        //mediaItem->item.element.charSet = BEtoHost16(list+offset);
				        offset += 2;

				        //media element length
				        listLen = B8toHost16(param->params.fldItems->list+offset);
				        offset += 2;

				        //media element info
        				memcpy(StringData, (param->params.fldItems->list+offset), listLen);

						#ifdef CFG_FUNC_STRING_CONVERT_EN
							StringConvert(ConvertStringData, StringMaxLen, StringData, listLen ,UTF8_TO_GBK);
							APP_DBG("%s\n", ConvertStringData);
						#endif
					}
				}
			}
			break;

		case BT_STACK_EVENT_AVRCP_ADV_AVRCP_CHANGE_PATH:
			APP_DBG("UI : BT_UI_CB_STATUS_ADV_AVRCP_CHANGE_PATH\n");
			APP_DBG("numItems:%x\n", param->params.chPath->numItems);
			break;
			
		case BT_STACK_EVENT_AVRCP_ADV_AVRCP_GET_ITEM_ATTRIBUTES:
			APP_DBG("UI : BT_UI_CB_STATUS_ADV_AVRCP_GET_ITEM_ATTRIBUTES\n");
			APP_DBG("numAttrs:%x\n", param->params.itemAttrs->numAttrs);
			break;
			
		case BT_STACK_EVENT_AVRCP_ADV_AVRCP_SEARCH:
			APP_DBG("UI : BT_UI_CB_STATUS_ADV_AVRCP_SEARCH\n");
			APP_DBG("uidCount:%x, numItems:%x\n", param->params.search->uidCounter, param->params.search->numItems);
			break;
#endif //(BT_AVRCP_BROWSER_FUNC == ENABLE)
#if BT_AVRCP_VOLUME_SYNC
			case BT_STACK_EVENT_AVRCP_PANEL_RELEASE:
				BtMidMessageSend(MSG_BT_MID_AVRCP_PANEL_KEY, (uint8_t)param->params.panelKey);
				//APP_DBG("AVRCP PANEL key=%d\n",param->params.panelKey);
			break;
#endif		

///////////////////////////////////////////////////////////////////////////////////////////
		default:
			break;
	}
}

static void SetAvrcpState(BT_AVRCP_STATE state)
{
	GetBtManager()->avrcpState = state;
}
#endif

BT_AVRCP_STATE GetAvrcpState(void)
{
	return GetBtManager()->avrcpState;
}


bool IsAvrcpConnected(void)
{
	return (GetAvrcpState() == BT_AVRCP_STATE_CONNECTED);
}

void BtAvrcpConnect(uint8_t *addr)
{
	if(!IsAvrcpConnected())
	{
		AvrcpConnect(addr);
	}
}


//针对部分手机不能主动连接AVRCP协议,在A2DP连接成功2S后主动发起AVRCP连接
void BtAvrcpConnectStart(void)
{
	if((!GetBtManager()->avrcpConnectStart)&&(!IsAvrcpConnected()))
	{
		GetBtManager()->avrcpConnectStart = 1;
		TimeOutSet(&AvrcpConnectTimer, 2000);
	}
}

/*
void CheckBtAvrcpState(void)
{
	if(GetBtManager()->avrcpConnectStart)
	{
		if(IsTimeOut(&AvrcpConnectTimer))
		{
			GetBtManager()->avrcpConnectStart=0;

			if(!IsAvrcpConnected())
			{
				memcpy(GetBtManager()->remoteAvrcpAddr, GetBtManager()->remoteA2dpAddr, 6);
				BtAvrcpConnect(GetBtManager()->remoteAvrcpAddr);
			}
		}
	}
}
*/

//AVRCP handle
void BtAvrcpConnectedHandle(uint8_t *addr)
{
	SetAvrcpState(BT_AVRCP_STATE_CONNECTED);
}


void BtAvrcpDisconnectHandle(void)
{
	SetAvrcpState(BT_AVRCP_STATE_NONE);
}

//timer: get bt play status
void TimerStart_BtPlayStatus(void)
{
	TimeOutSet(&GetBtManager()->avrcpPlayStatusTimer.timerHandle, 900);
	GetBtManager()->avrcpPlayStatusTimer.timerFlag = 1;
}

void TimerStop_BtPlayStatus(void)
{
	GetBtManager()->avrcpPlayStatusTimer.timerFlag = 0;
}

uint8_t CheckTimerStart_BtPlayStatus(void)
{
	return GetBtManager()->avrcpPlayStatusTimer.timerFlag;
}

#if (BT_AVRCP_BROWSER_FUNC == ENABLE)
/////////////////////////////////////////////////////////////////////////////
//avrcp browser
//范例:
void BtSetBrowsedPlayer(void)
{
	//1:Media player virtual file system
	AvrcpCtrlAdvAvrcpSetBrowsedPlayer(1);
}

void BtGetFolderItems(void)
{
	//获取文件名称
	AvrcpCtrlAdvAvrcpGetFolderItems(BTFolderIndex,BTFolderIndex);
}

void BtFolderForwardCtrl(void)
{
	//进入下一个目录
	AvrcpCtrlAdvAvrcpChangePath(0x01, CurUid);//CurUid:folder uid

	BTFolderIndex = 0;
}

void BtFolderBackwardCtrl(void)
{
	//返回上一个目录
	if(TotalFolderDepth>1)
	{
		AvrcpCtrlAdvAvrcpChangePath(0, CurUid);
	}
}

void BtPlaySongByUid(void)
{
	//播放指定的音乐文件
	AvrcpCtrlAdvAvrcpPlayItem(CurUid);//CurUid:song uid
}

#endif
