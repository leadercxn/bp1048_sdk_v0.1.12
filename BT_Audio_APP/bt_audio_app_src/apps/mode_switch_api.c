/**
 **************************************************************************************
 * @file    mode_switch_api.c
 * @brief   APP mode
 *
 * @author  halley
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "app_config.h"
#include "app_message.h"
#include "debug.h"
#include "mode_switch_api.h"
#include "main_task.h"
#include "linein_mode.h"
#include "media_play_mode.h"
#include "device_service.h"
#ifdef CFG_APP_BT_MODE_EN
#include "bt_platform_interface.h"
#include "bt_play_mode.h"
#endif
#include "hdmi_in_mode.h"
#include "usb_audio_mode.h"
#include "radio_mode.h"
#include "spdif_mode.h"
#include "waiting_mode.h"
#include "i2sin_mode.h"
#include "bt_hf_mode.h"
#include "bt_record_mode.h"
#include "rest_mode.h"

//#define NEW_APP_AUTO_WORD_TABLE
//	EN @app_config
//RADIO,	Radio)

//����ģʽ�ڴ���������к�.h�������Դλ,device_service.c��deviceServicePreSet()��̬ע�ᣬ����device_detect.c��̬ɨ��
//��main_task.c�ʹ��ļ�include XXX_mode.h�ļ���
//APPģʽ��Դ���룬ͨ��ƥ��ע��������APP�ĳ��(��Ӳ)����Ҫ��Ӳ�����������������ȼ����ģʽ
#define ResourcesAppMask_Line	(sizeof(ResourcesAppMask) / (sizeof(uint32_t) * 3))	
const uint32_t ResourcesAppMask[][3] = {
//  AppMode						Priority(0:Lowest)	ModeResourceMask	
	{AppModeWaitingPlay,		0,					AppResourceDac		},//��Ҫ: ��ģʽ�̶��ڵ�һ�У���Ҫ�䶯����Ϊȱʡֵģʽʹ�á�
	{AppModeRestPlay,			1,					AppResourceRest	},//���û����Ƶ� ������Դ(mic)ģʽ
	{AppModeLineAudioPlay,		3,					AppResourceLineIn	}, 
#ifdef CFG_APP_CARD_PLAY_MODE_EN
	{AppModeCardAudioPlay,		6,					AppResourceCard | AppResourceCardForPlay},
#endif
#ifdef CFG_APP_USB_PLAY_MODE_EN
	{AppModeUDiskAudioPlay,		6,					AppResourceUDisk | AppResourceUDiskForPlay},
#endif
#ifdef CFG_APP_BT_MODE_EN
	{AppModeBtAudioPlay,		2,					AppResourceBtPlay	},
	{AppModeBtHfPlay,			3,					AppResourceBtHf 	},
	{AppModeBtRecordPlay,		3,					AppResourceBtRecord	},
#endif
#ifdef CFG_APP_HDMIIN_MODE_EN
	{AppModeHdmiAudioPlay,		3,					AppResourceHdmiIn	}, 
#endif
#ifdef CFG_APP_USB_AUDIO_MODE_EN
	{AppModeUsbDevicePlay,		3,					AppResourceUsbDevice}, 
#endif
#ifdef CFG_APP_RADIOIN_MODE_EN
	{AppModeRadioAudioPlay,		3,					AppResourceRadio	},
#endif
#ifdef CFG_APP_OPTICAL_MODE_EN
	{AppModeOpticalAudioPlay,	3,					AppResourceSpdif	},
#endif
#ifdef CFG_APP_COAXIAL_MODE_EN
	{AppModeCoaxialAudioPlay,	3,					AppResourceSpdif	},
#endif
	{AppModeCardPlayBack,		0,					AppResourceCard		},

	{AppModeUDiskPlayBack,		0,					AppResourceUDisk	},

#ifdef	CFG_FUNC_RECORD_FLASHFS
	{AppModeFlashFsPlayBack,		0,				AppResourceFlashFs	},
#endif
#ifdef CFG_APP_I2SIN_MODE_EN
	{AppModeI2SInAudioPlay,		2,					AppResourceI2SIn	},
#endif
};
	
//modeѭ������Ӧģʽ��(������S10/S21 ?)
static const uint8_t ModeLoop[] = 
{
#ifdef CFG_APP_CARD_PLAY_MODE_EN
	AppModeCardAudioPlay,
#endif
#ifdef CFG_APP_LINEIN_MODE_EN
	AppModeLineAudioPlay,
#endif
#ifdef CFG_APP_USB_PLAY_MODE_EN
	AppModeUDiskAudioPlay,
#endif
#ifdef CFG_APP_BT_MODE_EN
	AppModeBtAudioPlay,
#endif
#ifdef CFG_APP_HDMIIN_MODE_EN
	AppModeHdmiAudioPlay,
#endif
#ifdef CFG_APP_I2SIN_MODE_EN
	AppModeI2SInAudioPlay,
#endif
#ifdef CFG_APP_USB_AUDIO_MODE_EN
	AppModeUsbDevicePlay,
#endif
#ifdef CFG_APP_RADIOIN_MODE_EN
	AppModeRadioAudioPlay,
#endif
#ifdef CFG_FUNC_SPDIF_EN
	AppModeOpticalAudioPlay,//����
	AppModeCoaxialAudioPlay,//ͬ��
#endif
#ifdef CFG_APP_REST_MODE_EN
	//AppModeRestPlay
#endif
};
	
//ע���
uint32_t		ModeResourceReady;

void ResourceRegister(uint32_t Resources)
{
	ModeResourceReady |= Resources;
	return ;
}

void ResourceDeregister(uint32_t Resources)
{
	ModeResourceReady &= ~Resources;
	return ;
}

uint32_t ResourceValue(uint32_t Resources)
{
	return ModeResourceReady & Resources;
}

//���к��������mode/appֱ����ء�
//��̬��ԴԤע�ᣬ������Դ ��̬Ԥ���ڴ���ӡ�ԭ����һ��ע��һ����Ϣ��
void ResourcePreSet(void)
{
//	MessageContext		msgSend;
	ResourceRegister(AppResourceDac);
#ifdef CFG_APP_BT_MODE_EN
	//�ϵ罫����ģʽ�Ƚ���ע��
	ResourceRegister(AppResourceBtPlay);
#endif

#if defined(CFG_APP_HDMIIN_MODE_EN) && !defined(HDMI_HPD_CHECK_DETECT_EN)
	ResourceRegister(AppResourceHdmiIn);//����̬��Ĭ�ϲ���HDMI
#endif

#ifdef CFG_APP_RADIOIN_MODE_EN
	ResourceRegister(AppResourceRadio);//����̬��
#endif
#ifdef CFG_FUNC_SPDIF_EN
	ResourceRegister(AppResourceSpdif);//����̬��
#endif
#ifdef CFG_APP_LINEIN_MODE_EN
	ResourceRegister(AppResourceLineIn);//�ײ���δ������⣬����ģ��һ���¼���Ϣ
#endif
#ifdef CFG_APP_I2SIN_MODE_EN
	ResourceRegister(AppResourceI2SIn);//�ײ���δ������⣬����ģ��һ���¼���Ϣ
#endif
#ifdef CFG_APP_REST_MODE_EN
	ResourceRegister(AppResourceRest);
#endif
}

//ּ��ɸѡģʽ������ϵ�����˯�߻��Ѻ�ָ���
bool ModeResumeMask(AppMode Mode)
{
	bool ret = FALSE;
	switch(Mode)
	{
		case AppModeUDiskPlayBack:
		case AppModeCardPlayBack:
		case AppModeFlashFsPlayBack:
		case AppModeBtHfPlay:
#ifdef CFG_APP_REST_MODE_EN
		case AppModeRestPlay:
#endif
			ret = TRUE;
			break;
		default:
			break;
	}
	return ret;
}

int32_t WaitingPlayStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = WaitingPlayCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = WaitingPlayStart();
			break;

		case ModeStop:
			ret = WaitingPlayStop();
			break;

		case ModeKill:
			ret = WaitingPlayKill();
			break;

		default:
			break;
	}
	return ret;
}

#ifdef CFG_APP_REST_MODE_EN
int32_t RestPlayStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = RestPlayCreate(GetMainMessageHandle());
			break;

		case ModeStart:
			ret = RestPlayStart();
			break;

		case ModeStop:
			ret = RestPlayStop();
			break;

		case ModeKill:
			ret = RestPlayKill();
			break;

		default:
			break;
	}
	return ret;
}
#endif

#ifdef CFG_APP_LINEIN_MODE_EN
int32_t LineInStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = LineInPlayCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = LineInPlayStart();
			break;

		case ModeStop:
			ret = LineInPlayStop();
			break;

		case ModeKill:
			ret = LineInPlayKill();
			break;

		default:
			break;
	}
	return ret;
}
#endif

#ifdef CFG_APP_I2SIN_MODE_EN
int32_t I2SInStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = I2SInPlayCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = I2SInPlayStart();
			break;

		case ModeStop:
			ret = I2SInPlayStop();
			break;

		case ModeKill:
			ret = I2SInPlayKill();
			break;

		default:
			break;
	}
	return ret;
}
#endif

#if defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN) || defined(CFG_FUNC_RECORDER_EN)

int32_t MediaPlayStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = MediaPlayCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = MediaPlayStart();
			SoftFlagRegister(SoftFlagDecoderApp);
			break;

		case ModeStop:
			ret = MediaPlayStop();
			break;

		case ModeKill:
			SoftFlagDeregister(SoftFlagDecoderApp);
			ret = MediaPlayKill();
			break;

		default:
			break;
	}
	return ret;
}
#endif

#ifdef CFG_APP_BT_MODE_EN
int32_t BtAudioStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = BtPlayCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = BtPlayStart();
			SoftFlagRegister(SoftFlagDecoderApp);
			break;

		case ModeStop:
			ret = BtPlayStop();
			break;

		case ModeKill:
			SoftFlagDeregister(SoftFlagDecoderApp);
			ret = BtPlayKill();
			break;

		default:
			break;
	}
	return ret;
}

int32_t BtHfStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = BtHfCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = BtHfStart();
			SoftFlagRegister(SoftFlagDecoderApp);
			break;

		case ModeStop:
			ret = BtHfStop();
			break;

		case ModeKill:
			SoftFlagDeregister(SoftFlagDecoderApp);
			ret = BtHfKill();
			break;

		default:
			break;
	}
	return ret;
}

int32_t BtRecordStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = BtRecordCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = BtRecordStart();
			SoftFlagRegister(SoftFlagDecoderApp);
			break;

		case ModeStop:
			ret = BtRecordStop();
			break;

		case ModeKill:
			SoftFlagDeregister(SoftFlagDecoderApp);
			ret = BtRecordKill();
			break;

		default:
			break;
	}
	return ret;
}

#endif

#ifdef CFG_APP_HDMIIN_MODE_EN
int32_t HdmiInStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = HdmiInPlayCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = HdmiInPlayStart();
			break;

		case ModeStop:
			ret = HdmiInPlayStop();
			break;

		case ModeKill:
			ret = HdmiInPlayKill();
			break;

		default:
			break;
	}
	return ret;
}
#endif

#ifdef CFG_APP_USB_AUDIO_MODE_EN
int32_t UsbAudioStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = UsbDevicePlayCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = UsbDevicePlayStart();
			break;

		case ModeStop:
			ret = UsbDevicePlayStop();
			break;

		case ModeKill:
			ret = UsbDevicePlayKill();
			break;

		default:
			break;
	}
	return ret;
}
#endif

#ifdef CFG_APP_RADIOIN_MODE_EN
int32_t RadioPlayStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = RadioPlayCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = RadioPlayStart();
			break;

		case ModeStop:
			ret = RadioPlayStop();
			break;

		case ModeKill:
			ret = RadioPlayKill();
			break;

		default:
			break;
	}
	return ret;
}
#endif

#ifdef CFG_FUNC_SPDIF_EN
int32_t SpdifPlayStateSet(ModeStateAct StateAct)
{
	int32_t ret = 0;
	switch(StateAct)
	{
		case ModeCreate:
			ret = SpdifPlayCreate(GetMainMessageHandle());
			break;
		
		case ModeStart:
			ret = SpdifPlayStart();
			break;

		case ModeStop:
			ret = SpdifPlayStop();
			break;

		case ModeKill:
			ret = SpdifPlayKill();
			break;

		default:
			break;
	}
	return ret;
}
#endif

//����ģʽ�������Api�������л������ģʽ��
int32_t ModeStateSet(ModeStateAct StateAct)//����appCurrentMode
{
	int32_t ret = 0;

	switch(GetSystemMode())
	{
		case AppModeWaitingPlay:
			ret = WaitingPlayStateSet(StateAct);
			break;
#ifdef CFG_APP_REST_MODE_EN
		case AppModeRestPlay:
			ret = RestPlayStateSet(StateAct);
			break;
#endif

#ifdef CFG_APP_LINEIN_MODE_EN
		case AppModeLineAudioPlay:
			ret = LineInStateSet(StateAct);
			break;
#endif
#ifdef CFG_APP_I2SIN_MODE_EN
		case AppModeI2SInAudioPlay:
			ret = I2SInStateSet(StateAct);
			break;
#endif
#if defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN) || defined(CFG_FUNC_RECORDER_EN)
		case AppModeCardAudioPlay:
		case AppModeCardPlayBack:
		case AppModeUDiskAudioPlay:
		case AppModeUDiskPlayBack:
		case AppModeFlashFsPlayBack:
			ret = MediaPlayStateSet(StateAct);
			break;
#endif

#ifdef CFG_APP_BT_MODE_EN
		case AppModeBtAudioPlay:
			ret = BtAudioStateSet(StateAct);
			break;
		case AppModeBtHfPlay:
			ret = BtHfStateSet(StateAct);
			break;
		case AppModeBtRecordPlay:
			ret = BtRecordStateSet(StateAct);
			break;
			
#endif
#ifdef CFG_APP_HDMIIN_MODE_EN
		case AppModeHdmiAudioPlay:
			ret = HdmiInStateSet(StateAct);
			break;
#endif
#ifdef CFG_APP_USB_AUDIO_MODE_EN
		case AppModeUsbDevicePlay:
			ret = UsbAudioStateSet(StateAct);
			break;
#endif
#ifdef CFG_APP_RADIOIN_MODE_EN
		case AppModeRadioAudioPlay:
			ret = RadioPlayStateSet(StateAct);
			break;
#endif
#ifdef CFG_FUNC_SPDIF_EN
		//case AppModeSpdifAudioPlay:
		case AppModeOpticalAudioPlay:
		case AppModeCoaxialAudioPlay:
			ret = SpdifPlayStateSet(StateAct);
			break;
#endif
		default:
			break;
	}
	return ret;
}

MessageHandle GetAppMessageHandle(void)
{
	MessageHandle ret = NULL;
	switch(GetSystemMode())
	{
		case AppModeWaitingPlay:
			ret = GetWaitingPlayMessageHandle();
			break;

#ifdef CFG_APP_LINEIN_MODE_EN
		case AppModeLineAudioPlay:
			ret = GetLineInMessageHandle();
			break;
#endif

#ifdef CFG_APP_I2SIN_MODE_EN
		case AppModeI2SInAudioPlay:
			ret = GetI2SInMessageHandle();
			break;
#endif

#if defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN) || defined(CFG_FUNC_RECORDER_EN)
		case AppModeCardAudioPlay:
		case AppModeCardPlayBack:
		case AppModeUDiskAudioPlay:
		case AppModeUDiskPlayBack:
		case AppModeFlashFsPlayBack:
			ret = GetMediaPlayMessageHandle();
			break;
#endif
#ifdef CFG_APP_BT_MODE_EN
		case AppModeBtAudioPlay:
			ret = GetBtPlayMessageHandle();
			break;
		case AppModeBtHfPlay:
			ret = GetBtHfMessageHandle();
			break;
		case AppModeBtRecordPlay:
			ret = GetBtRecordMessageHandle();
			break;
			
#endif
#ifdef CFG_APP_HDMIIN_MODE_EN
		case AppModeHdmiAudioPlay:
			ret = GetHdmiInMessageHandle();
			break;
#endif
#ifdef CFG_APP_USB_AUDIO_MODE_EN
		case AppModeUsbDevicePlay:
			ret = GetUsbDeviceMessageHandle();
			break;
#endif
#ifdef CFG_APP_RADIOIN_MODE_EN
		case AppModeRadioAudioPlay:
			ret = GetRadioPlayMessageHandle();
			break;
#endif
#ifdef CFG_FUNC_SPDIF_EN
		//case AppModeSpdifAudioPlay:
		case AppModeOpticalAudioPlay:
		case AppModeCoaxialAudioPlay:
			ret = GetSpdifPlayMessageHandle();
			break;
#endif
#ifdef CFG_APP_REST_MODE_EN
		case AppModeRestPlay:
			ret =  GetRestPlayMessageHandle();
			break;
#endif
		default:
			break;
	}
	return ret;
}


uint16_t AppStateMsgAck(uint16_t Msg)
{
	switch(Msg)
	{
		case MSG_WAITING_PLAY_MODE_CREATED:
		case MSG_REST_PLAY_MODE_CREATED:
		case MSG_LINE_AUDIO_MODE_CREATED:
		case MSG_MEDIA_PLAY_MODE_CREATED:
		case MSG_BT_PLAY_MODE_CREATED:
		case MSG_BT_HF_MODE_CREATED:
		case MSG_BT_RECORD_MODE_CREATED:
		case MSG_HDMI_AUDIO_MODE_CREATED:
		case MSG_USB_DEVICE_MODE_CREATED:
		case MSG_RADIO_AUDIO_MODE_CREATED:
		case MSG_SPDIF_AUDIO_MODE_CREATED:
		case MSG_I2SIN_AUDIO_MODE_CREATED:
			return MSG_APP_CREATED;
			break;

		case MSG_WAITING_PLAY_MODE_STARTED:
		case MSG_REST_PLAY_MODE_STARTED:
		case MSG_LINE_AUDIO_MODE_STARTED:
		case MSG_MEDIA_PLAY_MODE_STARTED:
		case MSG_BT_PLAY_MODE_STARTED:
		case MSG_BT_HF_MODE_STARTED:
		case MSG_BT_RECORD_MODE_STARTED:
		case MSG_HDMI_AUDIO_MODE_STARTED:
		case MSG_USB_DEVICE_MODE_STARTED:
		case MSG_RADIO_AUDIO_MODE_STARTED:
		case MSG_SPDIF_AUDIO_MODE_STARTED:
		case MSG_I2SIN_AUDIO_MODE_STARTED:
			return MSG_APP_STARTED;
			break;

		case MSG_MAINAPP_NEXT_MODE://main taskǿ���˳�mode
		case MSG_WAITING_PLAY_MODE_STOPPED:
		case MSG_REST_PLAY_MODE_STOPPED:
		case MSG_MEDIA_PLAY_MODE_STOPPED:
		case MSG_HDMI_AUDIO_MODE_STOPPED:
		case MSG_LINE_AUDIO_MODE_STOPPED:
		case MSG_BT_PLAY_MODE_STOPPED:
		case MSG_BT_HF_MODE_STOPPED:
		case MSG_BT_RECORD_MODE_STOPPED:
		case MSG_USB_DEVICE_MODE_STOPPED:
		case MSG_RADIO_AUDIO_MODE_STOPPED:
		case MSG_SPDIF_AUDIO_MODE_STOPPED:
		case MSG_I2SIN_AUDIO_MODE_STOPPED:
			return MSG_APP_STOPPED;
			break;

		default:
			return MSG_NONE;
	}
}

//����ResourcesAppMask[]�й�ϵ����ModeLoop�ҳ����ȼ��ߵ�ģʽ
AppMode FindPriorityMode(void)
{
	uint8_t i,j, FoundLine = 0;//�Ҳ�����Чģʽ�ͷ������У�waiting
	for(i = 0; i < sizeof(ModeLoop); i++)
	{
		for(j = 0; j < ResourcesAppMask_Line; j++)
		{
			if(ModeLoop[i] == ResourcesAppMask[j][0])//��ResourcesAppMask���ҵ�ģʽ��Ӧ�С�
			{
				if(ResourceValue(ResourcesAppMask[j][2]) == ResourcesAppMask[j][2]) //mode��Դ�߱���
				{
					if(ResourcesAppMask[FoundLine][1] <= ResourcesAppMask[j][1])//�Ѽ��������� ���ȼ��ϵ�)
					{
						FoundLine = j;
					}
				}
			}
		}
	}

	return (AppMode)ResourcesAppMask[FoundLine][0];
}

//���ݣ����룩Resources������ModeLoopʹ�ô���Դ����߼���ģʽ��
AppMode FindResourceMode(uint32_t Resources)
{
	uint8_t i, j, FoundLine = 0;
	for(j = 0; j < ResourcesAppMask_Line; j++)
	{
		if((Resources & ResourcesAppMask[j][2]) == Resources		//��Դ������
			&& ResourceValue(ResourcesAppMask[j][2]) == ResourcesAppMask[j][2])
		{
			for(i = 0; i < sizeof(ModeLoop); i++)//��ģʽ��Modeloop��
			{
				if(ModeLoop[i] == ResourcesAppMask[j][0])
				{
					if(ResourcesAppMask[FoundLine][1] <= ResourcesAppMask[j][1])//�Ѽ��������� ���ȼ��ϵ�
					{
						FoundLine = j;
					}
				}
			}

		}
	}
	return (AppMode)ResourcesAppMask[FoundLine][0];
}

bool CheckModeResource(AppMode mode)
{
	uint8_t i;
	for(i = 0; i < ResourcesAppMask_Line; i++)
	{
		if(ResourcesAppMask[i][0] == mode && ResourceValue(ResourcesAppMask[i][2]) == ResourcesAppMask[i][2])
		{
			return TRUE;
		}
	}
	return FALSE;
}

//����ģʽloop����Դע�����ѡ����һ��ģʽ������ModeLoop�ڵ�ģʽ(��waiting)����0��ʼ������
AppMode NextAppMode(AppMode mode)
{
	uint8_t i, Index;

	if(!sizeof(ModeLoop))
	{//���������
		return AppModeWaitingPlay;
	}
	Index = 0;//ȱʡ��0��ʼ
	for(i = 0; i < sizeof(ModeLoop); i++)
	{
		if(ModeLoop[i] == mode)
		{
			Index = i + 1;//�ҵ�mode��Loop��ţ�����һλ�ÿ�ʼ��
			break;
		}
	}
	for(i = 0; i < sizeof(ModeLoop) ; i++)
	{
		Index %= sizeof(ModeLoop); 
		if(CheckModeResource(ModeLoop[Index]))
		{
			return (AppMode)ModeLoop[Index];
		}
		Index++;
	}
	return AppModeWaitingPlay;//Loopģʽ��������
}

//��Դ������Ϣ ����ģʽApp���ڴ����ת�롣ͬʱ�� maintask��MsgProcessModeResource()������
AppMode FindModeByPlugInMsg(uint16_t Msg)
{
	AppMode RetMode = AppModeWaitingPlay;
	switch(Msg)
	{
		case MSG_DEVICE_SERVICE_LINE_IN:
			RetMode = FindResourceMode(AppResourceLineIn);
			break;
		case MSG_DEVICE_SERVICE_CARD_IN:
			RetMode = FindResourceMode(AppResourceCard);
			break;
		case MSG_DEVICE_SERVICE_DISK_IN:
			RetMode = FindResourceMode(AppResourceUDisk);
			break;
		case MSG_DEVICE_SERVICE_USB_DEVICE_IN:
			RetMode = FindResourceMode(AppResourceUsbDevice);
			break;
		case MSG_DEVICE_SERVICE_HDMI_IN:
			RetMode = FindResourceMode(AppResourceHdmiIn);
			break;	
	}

	return RetMode;
}

//���Ը���
char* ModeNameStr(AppMode Mode)
{
	switch(Mode)
	{
		case AppModeWaitingPlay:
			return "Waiting";
#ifdef CFG_APP_REST_MODE_EN
		case AppModeRestPlay:
			return "Rest";
#endif
		case AppModeLineAudioPlay:
			return "LinePlay";
			
		case AppModeI2SInAudioPlay:
			return "I2SINPlay";

		case AppModeCardAudioPlay:
			return "CardPlay";

		case AppModeUDiskAudioPlay:
			return "UDiskPlay";

		case AppModeCardPlayBack:
			return "CardPlayBack";

		case AppModeUDiskPlayBack:
			return "UDiskPlayBack";
			
		case AppModeFlashFsPlayBack:
			return "FlashFsPlayBack";
				
		case AppModeBtAudioPlay:
			return "BtPlay";

		case AppModeBtHfPlay:
			return "BtHfPlay";
		
		case AppModeBtRecordPlay:
			return "BtRecordPlay";

		case AppModeHdmiAudioPlay:
			return "HdmiPlay";

		case AppModeUsbDevicePlay:
			return "UsbDevicePlay";

		case AppModeRadioAudioPlay:
			return "RadioPlay";

		//case AppModeSpdifAudioPlay:
		case AppModeOpticalAudioPlay:
			return "Spdif OpticalPlay";
		case AppModeCoaxialAudioPlay:
			return "Spdif CoaxialPlay";

		default:
			return "Unknow Mode";

	}
}

