/**
 **************************************************************************************
 * @file    Browser.c
 * @brief   
 *
 * @author  BKD/Ken
 * @version V0.01
 *
 * $Created: 2019-9-7 
 * 
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "irqn.h"
#include "app_config.h"
#include "app_message.h"
#include "chip_info.h"
#include "dac.h"
#include "gpio.h"
#include "dma.h"
#include "dac.h"
#include "audio_adc.h"
#include "rtos_api.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "debug.h"
#include "audio_effect.h"
#include "main_task.h"
#include "audio_core_api.h"
#include "media_play_mode.h"
#include "decoder_service.h"
#include "audio_core_service.h"
#include "media_play_api.h"
#include "mode_switch_api.h"
#include "remind_sound_service.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "ctrlvars.h"
#include "audio_vol.h"
#include "timeout.h"
#include "ffpresearch.h"
#include "browser_parallel.h"

#ifdef FUNC_BROWSER_PARALLEL_EN
extern char current_vol[8];
extern MEDIA_PLAYER* gpMediaPlayer;
static BrowserStep BrowserProcess=Browser_None;
uint32_t gFolderFocusing=1;// 1-GUI_ROW_CNT_MAX
uint32_t gFileFocusing=1;// 1-GUI_ROW_CNT_MAX
uint32_t gStartFolderIndex=1;
uint32_t gStartFileIndex=1;
static uint32_t sEndFolderIndex=GUI_ROW_CNT_MAX;
static uint32_t sEndFileIndex=GUI_ROW_CNT_MAX;
static uint32_t sFolderNumber_In_screen=0;
static uint32_t sFileNumber_In_screen=0;
static BrowserPlayState sBrowserPlayState=Browser_Play_None;//0:not play 1:normal play 2:background play
static uint8_t sShowGuiTime=0;

ff_dir_win gBrowserDirWin[GUI_ROW_CNT_MAX];	
ff_file_win gBrowserFileWin[GUI_ROW_CNT_MAX];

void BrowserVarDefaultSet(void)
{
	BrowserProcess=Browser_None;
	sBrowserPlayState=Browser_Play_None;
	gFolderFocusing=1;// 1-GUI_ROW_CNT_MAX
	gFileFocusing=1;// 1-GUI_ROW_CNT_MAX
	gStartFolderIndex=1;
	sEndFolderIndex=GUI_ROW_CNT_MAX;
	gStartFileIndex=1;
	sEndFileIndex=GUI_ROW_CNT_MAX;
	sFolderNumber_In_screen=0;
	sFileNumber_In_screen=0;
}


static void ShowFolderGui(char *current_vol,uint32_t start_folder_index,uint32_t end_folder_index,uint32_t dir_focusing,bool scan_if)
{
	uint16_t  j, found_num; 
	
	sShowGuiTime=4;// 4s
	if(end_folder_index>f_dir_real_quantity())
	{
		end_folder_index=f_dir_real_quantity();
	}
	if(end_folder_index-start_folder_index>GUI_ROW_CNT_MAX-1)
		end_folder_index=start_folder_index+GUI_ROW_CNT_MAX-1;
	if(scan_if==TRUE)
	{
		memset(gBrowserDirWin, 0x00, sizeof(ff_dir_win)*GUI_ROW_CNT_MAX); 			
		f_scan_dir_by_win(current_vol, start_folder_index,end_folder_index , gBrowserDirWin,FF_LFN_BUF_TREE_BROWSER,&found_num);//f_dir_real_quantity()?????????????
		/*if(found_num<GUI_ROW_CNT_MAX&&start_folder_index>1)
			{
			if(end_folder_index-start_folder_index<GUI_ROW_CNT_MAX-1)
				{
				start_folder_index--;
				gStartFolderIndex--;
				}
			memset(gBrowserDirWin, 0x00, sizeof(ff_dir_win)*GUI_ROW_CNT_MAX); 			
			f_scan_dir_by_win(current_vol, start_folder_index,end_folder_index , gBrowserDirWin,FF_LFN_BUF_TREE_BROWSER,&found_num);//f_dir_real_quantity()?????????????
			}
		*/
		sFolderNumber_In_screen=found_num;
	}
	
	found_num=sFolderNumber_In_screen;
	if(dir_focusing>sFolderNumber_In_screen)
	{
		dir_focusing=sFolderNumber_In_screen;
		gFolderFocusing=sFolderNumber_In_screen;
	}
	APP_DBG("--------------------------------FOLDER GUI--------------------------------\n"); 	//user defifine by himself,here only for demo
	for(j=0; j<found_num; j++) 			
	{ 
		gBrowserDirWin[j].long_name[FF_LFN_BUF_TREE_BROWSER-1]=0;
		if(gBrowserDirWin[j].long_name[0]==0)// ROOT FOLDER 
		{
			gBrowserDirWin[j].long_name[0]='R';
			gBrowserDirWin[j].long_name[1]='O';
			gBrowserDirWin[j].long_name[2]='O';
			gBrowserDirWin[j].long_name[3]='T';
			gBrowserDirWin[j].long_name[4]=0;
			APP_DBG("%s Folder %ld : %s Include song number:%d\n", (dir_focusing==j+1)?">>":"  ",start_folder_index+j, gBrowserDirWin[j].long_name, gBrowserDirWin[j].valid_file_num); 	
		}
		else
			APP_DBG("%s Folder %ld : %s Include song number:%d\n", (dir_focusing==j+1)?">>":"  ",start_folder_index+j, gBrowserDirWin[j].long_name, gBrowserDirWin[j].valid_file_num); 
	} 
	APP_DBG("--------------------------------FOLDER GUI--------------------------------\n");

}


static void ShowFileGui(ff_dir_win *dir_win,uint32_t start_file_index,uint32_t end_file_index,uint32_t file_focusing,bool scan_if)
{	
		
	uint16_t found_num,j;
	sShowGuiTime=4;// 4s

	if(end_file_index-start_file_index>GUI_ROW_CNT_MAX-1)
			end_file_index=start_file_index+GUI_ROW_CNT_MAX-1;
	if(scan_if==TRUE)
	{
		memset(gBrowserFileWin, 0x00, sizeof(ff_file_win)*GUI_ROW_CNT_MAX); 
		f_scan_file_in_dir_by_win(&dir_win->dir_info,start_file_index,end_file_index,gBrowserFileWin,FF_LFN_BUF_TREE_BROWSER,&found_num);
		/*if(found_num<GUI_ROW_CNT_MAX&&start_file_index>1)
			{
			if(end_file_index-start_file_index<GUI_ROW_CNT_MAX-1)
				{
				start_file_index--;
				gStartFileIndex--;
				}
			memset(gBrowserFileWin, 0x00, sizeof(ff_file_win)*GUI_ROW_CNT_MAX); 
			f_scan_file_in_dir_by_win(&dir_win->dir_info,start_file_index,end_file_index,gBrowserFileWin,FF_LFN_BUF_TREE_BROWSER,&found_num);
			}*/
		sFileNumber_In_screen=found_num;
	}
	found_num=sFileNumber_In_screen;
	if(file_focusing>sFileNumber_In_screen)
	{
		file_focusing=sFileNumber_In_screen;
		gFileFocusing=sFileNumber_In_screen;
	}
	APP_DBG("--------------------------------FILE   GUI--------------------------------\n");//user defifine by himself,here only for demo
	for(j=0; j<found_num; j++) 	
	{
		gBrowserFileWin[j].long_name[FF_LFN_BUF_TREE_BROWSER-1]=0;
		APP_DBG("%s Song:%ld/%d Song name=%s\n",(file_focusing==j+1)?">>":"  ",j+start_file_index,dir_win->valid_file_num,gBrowserFileWin[j].long_name);
	}
	APP_DBG("--------------------------------FILE   GUI--------------------------------\n");
	

}


BrowserStep GetBrowserStep(void)
{
	return BrowserProcess;
}
uint8_t GetShowGuiTime(void)
{
	return sShowGuiTime;
}
void DecShowGuiTime(void)
{
	if(sShowGuiTime)
		sShowGuiTime--;
}

static void ShowBrowserHomeGui(void)
{

	if(sFolderNumber_In_screen==0)
		sEndFolderIndex=gStartFolderIndex+GUI_ROW_CNT_MAX-1;
	else
		sEndFolderIndex=gStartFolderIndex+sFolderNumber_In_screen-1;
	ShowFolderGui(current_vol,gStartFolderIndex,gStartFolderIndex+GUI_ROW_CNT_MAX-1,gFolderFocusing,TRUE);
	sEndFolderIndex=gStartFolderIndex+sFolderNumber_In_screen-1;//correct sEndFolderIndex

}
void BrowserUp(BrowserStep BrowserProcessTemp)
 {
    uint32_t public_temp_var=0;

    if(BrowserProcessTemp==Browser_Folder)
    {
            APP_DBG("MSG_MEDIA_PLAY_BROWER_UP\n");
            public_temp_var=0;
            
            if(gFolderFocusing>1)
                gFolderFocusing--;
            else
            {
                public_temp_var=1;//out of range ,need scan again
                if(gStartFolderIndex>1)
                {
                    gStartFolderIndex--;
                }
                else
                {
	                if(gpMediaPlayer->ValidFolderSumInDisk>=GUI_ROW_CNT_MAX)
	                {
		                gFolderFocusing=GUI_ROW_CNT_MAX;
		                gStartFolderIndex=gpMediaPlayer->ValidFolderSumInDisk-GUI_ROW_CNT_MAX+1;
	                }
					else
					{
						gStartFolderIndex=1;
						gFolderFocusing=gpMediaPlayer->ValidFolderSumInDisk;
					}
                }
				
				APP_DBG("UP gpMediaPlayer->ValidFolderSumInDisk=%d\n",gpMediaPlayer->ValidFolderSumInDisk);
            }
			
			
            ShowFolderGui(current_vol,gStartFolderIndex,gStartFolderIndex+GUI_ROW_CNT_MAX-1,gFolderFocusing,public_temp_var?TRUE:FALSE);// ?????
            sEndFolderIndex=gStartFolderIndex+sFolderNumber_In_screen-1;//correct sEndFolderIndex
    }
    else if(BrowserProcessTemp==Browser_File)
    {
        	APP_DBG("MSG_MEDIA_PLAY_BROWER_UP\n");
        	public_temp_var=0;
            if(gFileFocusing>1)
                gFileFocusing--;
            else
            {
                public_temp_var=1;//out of range ,need scan again
                if(gStartFileIndex>1)
                {
                    gStartFileIndex--;
                }
                else
                {
	                if(gBrowserDirWin[gFolderFocusing-1].valid_file_num>=GUI_ROW_CNT_MAX)
	                {
		                gFileFocusing=GUI_ROW_CNT_MAX;
		                gStartFileIndex=gBrowserDirWin[gFolderFocusing-1].valid_file_num-GUI_ROW_CNT_MAX+1;//
		             }
					else
					{
						gStartFileIndex=1;
						gFileFocusing=gBrowserDirWin[gFolderFocusing-1].valid_file_num;
					}
                }
				
				APP_DBG("UP valid_file_num in folder=%d\n",gBrowserDirWin[gFolderFocusing-1].valid_file_num);
            }

			
            ShowFileGui(&gBrowserDirWin[gFolderFocusing-1],gStartFileIndex,gStartFileIndex+GUI_ROW_CNT_MAX-1,gFileFocusing,public_temp_var?TRUE:FALSE);
            sEndFileIndex=gStartFileIndex+sFileNumber_In_screen-1;//correct sEndFilerIndex
    }
}
void BrowserDn(BrowserStep BrowserProcessTemp)
{
    uint32_t public_temp_var=0;
    if(BrowserProcessTemp==Browser_Folder)
        {
            APP_DBG("MSG_MEDIA_PLAY_BROWER_DN\n");
            public_temp_var=0;
            if(sFolderNumber_In_screen<GUI_ROW_CNT_MAX)
            {
                if(gFolderFocusing<sFolderNumber_In_screen)
                    gFolderFocusing++;
				else
					gFolderFocusing=1;
            }
            else
            {// full display the last time
                if(gFolderFocusing<GUI_ROW_CNT_MAX)
                    gFolderFocusing++;
                else
                {
                    public_temp_var=1;//out of range ,need scan again
                    if(gStartFolderIndex+GUI_ROW_CNT_MAX-1<gpMediaPlayer->ValidFolderSumInDisk)// bkd
                    {
                        gStartFolderIndex++;
                    }
                    else
                    {
                        gFolderFocusing=1;
                        gStartFolderIndex=1;
                    }
                }
				
				APP_DBG("DN gpMediaPlayer->ValidFolderSumInDisk=%d\n",gpMediaPlayer->ValidFolderSumInDisk);
            }
            ShowFolderGui(current_vol,gStartFolderIndex,gStartFolderIndex+GUI_ROW_CNT_MAX-1,gFolderFocusing,public_temp_var?TRUE:FALSE);// ?????
            sEndFolderIndex=gStartFolderIndex+sFolderNumber_In_screen-1;//correct sEndFolderIndex
        }
    else if(BrowserProcessTemp==Browser_File)
        {
            APP_DBG("MSG_MEDIA_PLAY_BROWER_DN\n");
            public_temp_var=0;
            if(sFileNumber_In_screen<GUI_ROW_CNT_MAX)
            {
                if(gFileFocusing<sFileNumber_In_screen)
                    gFileFocusing++;
				else
					gFileFocusing=1;
            }
            else
            {// full display the last time
                if(gFileFocusing<GUI_ROW_CNT_MAX)
                    gFileFocusing++;
                else
                {
                     public_temp_var=1;//out of range ,need scan again
                    if(gStartFileIndex+GUI_ROW_CNT_MAX-1<gBrowserDirWin[gFolderFocusing-1].valid_file_num)//f_dir_with_song_real_quantity())
                    {
                        //ndFileIndex++;
                        gStartFileIndex++;
                    }
                    else
                    {
                        //if(sBrowserPlayState==Browser_Play_Normal)
                        {
                            gFileFocusing=1;
                            gStartFileIndex=1;
                        }
                    }
                }
				
				APP_DBG("DN valid_file_num in folder=%d\n",gBrowserDirWin[gFolderFocusing-1].valid_file_num);
            }
            ShowFileGui(&gBrowserDirWin[gFolderFocusing-1],gStartFileIndex,gStartFileIndex+GUI_ROW_CNT_MAX-1,gFileFocusing,public_temp_var?TRUE:FALSE);
            sEndFileIndex=gStartFileIndex+sFileNumber_In_screen-1;//correct sEndFileIndex
        }
}

static void BrowserClearScreen(void)//user defifine by himself,here only for demo
{

	APP_DBG("clear file and folder gui \n");
 	
}

BrowserPlayState GetBrowserPlay_state(void)//0:not play 1:normal play 2:background play
{
	return sBrowserPlayState;
}

void EnterBrowserPlayMode(void)
{

	BrowserVarDefaultSet();
	ShowBrowserHomeGui();
	SetMediaPlayMode(PLAY_MODE_BROWSER);
	BrowserProcess=Browser_Folder;

}
void ExitBrowserPlayMode(void)
{
	sShowGuiTime=0;
	BrowserClearScreen();
	BrowserProcess=Browser_None;						
	sBrowserPlayState=Browser_Play_None;
	gpMediaPlayer->CurPlayMode++;
	gpMediaPlayer->CurPlayMode %= PLAY_MODE_SUM;

}
void BrowserMsgProcess(uint16_t msgId)
{
//uint32_t public_temp_var=0;

    switch(BrowserProcess)
    {
    case Browser_None:
    	{
    	if(msgId==MSG_BROWSE)
    		{
    		APP_DBG("MSG_MEDIA_PLAY_BROWER_SWITCH\n");
    		//if(GetMediaPlayMode()!=PLAY_MODE_BROWSER)
    			{
    			EnterBrowserPlayMode();
    			}
    		
    		}
    	}
    break;
    case Browser_Folder:
    	{
    	switch(msgId)
        	{
        	case MSG_MEDIA_PLAY_BROWER_UP:
        		BrowserUp(Browser_Folder);
        		break;
        	case MSG_MEDIA_PLAY_BROWER_DN:
        		BrowserDn(Browser_Folder);
        		break;
        	case MSG_MEDIA_PLAY_BROWER_ENTER:
        		gStartFileIndex=1;
        		sEndFileIndex=GUI_ROW_CNT_MAX;
        		gFileFocusing=1;
        		ShowFileGui(&gBrowserDirWin[gFolderFocusing-1],gStartFileIndex,sEndFileIndex,gFileFocusing,TRUE);
        		gpMediaPlayer->ErrFileCount=gBrowserDirWin[gFolderFocusing - 1].valid_file_num;
        		BrowserProcess=Browser_File;
        		break;
        	case MSG_MEDIA_PLAY_BROWER_RETURN:
        	case MSG_BROWSE:
        		ExitBrowserPlayMode();
        		break;
        	}
    	}
    break;
    case Browser_File:
    	{
    	switch(msgId)
    		{
    		case MSG_MEDIA_PLAY_BROWER_UP:
    			BrowserUp(Browser_File);
    			break;
    		case MSG_MEDIA_PLAY_BROWER_DN:
    			BrowserDn(Browser_File);
    			break;
    		case MSG_MEDIA_PLAY_BROWER_ENTER:
    			sShowGuiTime=0;
    			MediaPlayerBrowserEnter();
    			sBrowserPlayState=Browser_Play_Normal;
    			break;
    		case MSG_MEDIA_PLAY_BROWER_RETURN:
    			BrowserProcess=Browser_Folder;
    			if(sBrowserPlayState==Browser_Play_Normal)
    				sBrowserPlayState=Browser_Play_BackGround;
    			ShowBrowserHomeGui();
    			break;
    		case MSG_BROWSE:
    			ExitBrowserPlayMode();
    			break;
    		}
    	}
    break;
    default:
    	break;


    }

}


#endif

