/**
 **************************************************************************************
 * @file    dev_detect.c
 * @brief   
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-01-08 13:30:47$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include "type.h"
#include "app_config.h"
//driver
#include "gpio.h"
#include "sdio.h"
#include "timeout.h"
#include "otg_detect.h"
#include "delay.h"
#include "debug.h"
#include "device_detect.h"
#include "rtos_api.h" //for SDIOMutex
#include "otg_device_standard_request.h"
#include "mode_switch_api.h"
#include "irqn.h"
#include "misc.h"
#include "main_task.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"

//#include "sd_card.h"
#define		DETECT_INIT_TIME			20
#define 	DETECT_JITTER_TIME			100
#define		DETECT_OUT_WAIT_TIME		150 //拔出 屏蔽期，避免事件频繁。取代消抖，意在模式硬件重新初始化。

typedef enum _DETECT_STATE
{
	DETECT_STATE_OUT			= 0,
	DETECT_STATE_IN				= 1,
	DETECT_STATE_OUT_JITTER		= 2,
	DETECT_STATE_IN_JITTER		= 3,
	DETECT_STATE_IDLE			= 4,
} DETECT_STATE;
#ifdef CFG_FUNC_CARD_DETECT
#ifndef CARD_DETECT_GPIO
/*#define SDIO_Clk_Disable		SDIO1_ClkDisable
#define SDIO_Clk_Eable			SDIO1_ClkEnable
#define CARD_DETECT_GPIO		GPIOA6
#define CARD_DETECT_GPIO_IN		GPIO_A_IN
#define CARD_DETECT_BIT_MASK	GPIO_INDEX6
#define CARD_DETECT_GPIO_IE		GPIO_A_IE
#define CARD_DETECT_GPIO_OE		GPIO_A_OE
#define CARD_DETECT_GPIO_PU		GPIO_A_PU
#define CARD_DETECT_GPIO_PD		GPIO_A_PD
*/
#endif
static DETECT_STATE CardState = DETECT_STATE_IDLE;
TIMER	CardDetectTimer;
#endif

#ifdef CFG_FUNC_UDISK_DETECT
static DETECT_STATE UDiskState = DETECT_STATE_IDLE;
static bool IsUDiskRemoved = TRUE;
TIMER	UDiskDetectTimer;
#endif
#ifdef CFG_FUNC_USB_DEVICE_EN
static DETECT_STATE USB_Device_State = DETECT_STATE_IDLE;
#endif

#ifdef CFG_LINEIN_DET_EN
static DETECT_STATE LineInState = DETECT_STATE_IDLE;
TIMER	LineDetectTimer;
#endif

#ifdef HDMI_HPD_CHECK_DETECT_EN
static  DETECT_STATE HDMI_Device_State = DETECT_STATE_IDLE;
TIMER	HDMIDetectTimer;
TIMER	HDMIExitTimer;
uint8_t HDMIExitFlg = 0;
uint8_t HDMIResetFlg = 0;
uint8_t	HDMIDetectEnable =1;
#endif

//static uint32_t sd_in_count=0;
//static uint32_t sd_out_count=0;

#ifdef CFG_RADIO_I2C_SD_SAME_PIN_EN
extern bool i2c_work;
#endif
#ifdef CFG_FUNC_UDISK_DETECT
bool UDiskRemovedFlagGet(void)
{
	return IsUDiskRemoved;
}

void UDiskRemovedFlagSet(bool State)
{
	IsUDiskRemoved = State;
}
#endif
bool IsCardIn(void)// 10ms 检测有问题
{
#ifdef CFG_FUNC_CARD_DETECT
	bool FindCard;
#ifdef CFG_RADIO_I2C_SD_SAME_PIN_EN
	static bool FindCard_bak;
#endif
	bool BackupPD;
	uint8_t BackupMode;

#ifdef CFG_RADIO_I2C_SD_SAME_PIN_EN
	if(i2c_work)
		return FindCard_bak;
#endif
	osMutexLock(SDIOMutex);
	SDIO_Clk_Disable();
	BackupMode = GPIO_PortAModeGet(CARD_DETECT_GPIO);
	BackupPD = GPIO_RegOneBitGet(CARD_DETECT_GPIO_PD, CARD_DETECT_BIT_MASK);	
#ifdef CFG_RADIO_I2C_SD_SAME_PIN_EN//开启IO现场恢复时，识别检测时间较长。
	bool BackupIE,BackupOE,BackupPU;
	BackupPU = GPIO_RegOneBitGet(CARD_DETECT_GPIO_PU, CARD_DETECT_BIT_MASK);
	BackupIE = GPIO_RegOneBitGet(CARD_DETECT_GPIO_IE, CARD_DETECT_BIT_MASK);
	BackupOE = GPIO_RegOneBitGet(CARD_DETECT_GPIO_OE, CARD_DETECT_BIT_MASK);
	GPIO_RegOneBitClear(CARD_DETECT_GPIO_OE, CARD_DETECT_BIT_MASK);
#endif
	GPIO_PortAModeSet(CARD_DETECT_GPIO, 0x0);
	GPIO_RegOneBitSet(CARD_DETECT_GPIO_PU, CARD_DETECT_BIT_MASK);
	GPIO_RegOneBitClear(CARD_DETECT_GPIO_PD, CARD_DETECT_BIT_MASK); 
	GPIO_RegOneBitSet(CARD_DETECT_GPIO_IE, CARD_DETECT_BIT_MASK);
#ifdef CFG_RADIO_I2C_SD_SAME_PIN_EN
	__udelay(29);//延时等电平稳定,阻值设定相关。
#endif
	//__udelay(1);//R83阻值设定相关
	if(GPIO_RegOneBitGet(CARD_DETECT_GPIO_IN, CARD_DETECT_BIT_MASK))
	{
		//APP_DBG(".\n");
		FindCard = FALSE;
	}
	else
	{
		//APP_DBG("|\n");

		FindCard = TRUE;
	}
#ifdef CFG_RADIO_I2C_SD_SAME_PIN_EN
	FindCard_bak = FindCard;
#endif
	GPIO_PortAModeSet(CARD_DETECT_GPIO, BackupMode);
	if(BackupPD)
	{
		GPIO_RegOneBitSet(CARD_DETECT_GPIO_PD, CARD_DETECT_BIT_MASK);	
	}

#ifdef CFG_RADIO_I2C_SD_SAME_PIN_EN
	if(!BackupPU)
	{
		GPIO_RegOneBitClear(CARD_DETECT_GPIO_PU, CARD_DETECT_BIT_MASK);
	}

	if(!BackupIE)
	{
		GPIO_RegOneBitClear(CARD_DETECT_GPIO_IE, CARD_DETECT_BIT_MASK);
	}
	if(BackupOE)
	{
		GPIO_RegOneBitSet(CARD_DETECT_GPIO_OE, CARD_DETECT_BIT_MASK);
	}
#endif

	SDIO_Clk_Eable(); //recover
	osMutexUnlock(SDIOMutex);

	return FindCard;

#else
	return FALSE;
#endif
}

bool IsLineInLink(void)
{
#ifdef CFG_LINEIN_DET_EN
	#define LINEIN_JETTER_TIMES		30	//连接检测消抖时间：30次

	static uint8_t LineInLinkState = 0;

	//设为输入，带上拉
	GPIO_RegOneBitClear(LINEIN_DET_GPIO_OE, LINEIN_DET_BIT_MASK);
	GPIO_RegOneBitSet(LINEIN_DET_GPIO_PU, LINEIN_DET_BIT_MASK);
	GPIO_RegOneBitClear(LINEIN_DET_GPIO_PD, LINEIN_DET_BIT_MASK);

	GPIO_RegOneBitSet(LINEIN_DET_GPIO_IE, LINEIN_DET_BIT_MASK);

	if(GPIO_RegOneBitGet(LINEIN_DET_GPIO_IN, LINEIN_DET_BIT_MASK))
	{
		LineInLinkState = 0;						//断开状态不做消抖处理
		return FALSE;
	}
	else
	{
		if(LineInLinkState < LINEIN_JETTER_TIMES)	//连接状态做消抖处理
		{
			LineInLinkState++;
		}
	}
	return (LineInLinkState >= LINEIN_JETTER_TIMES);
#else
    return TRUE;
#endif		
}

bool IsHDMILink(void)
{
#ifdef HDMI_HPD_CHECK_DETECT_EN
	if(GPIO_RegOneBitGet(HDMI_HPD_CHECK_STATUS_IO, HDMI_HPD_CHECK_STATUS_IO_PIN))
	{
		return TRUE;
	}
	return FALSE;
#else
	return FALSE;
#endif
}
/**
 * @func        DeviceDetectCard
 * @brief       DeviceDetectCard no jitter
 * @param       void  
 * @Output      None
 * @return      PlugEvent
 * @Others      
 * Record
 * 1.Date        : 20180122
 *   Author      : pi.wang
 *   Modification: Created function
*/
#ifdef CFG_FUNC_CARD_DETECT
//static uint8_t sd_card_detect_xms_polling=0; //bkd add 2019.4.27
#endif

#ifdef CFG_COMMUNICATION_BY_USB
static uint32_t sDevice_Init_state=0;// 1:inited    0:not init
uint8_t sDevice_Inserted_Flag = 0; //实现USB在线调音下的后插先播功能
#endif

#ifdef CFG_APP_USB_AUDIO_MODE_EN
static uint8_t sUSB_Audio_Exit_Flag=0;
#endif

extern void NVIC_DisableIRQ(IRQn_Type IRQn);
extern uint32_t CmdErrCnt;

uint32_t DeviceDetect(void)
{
	uint32_t Ret = 0;
	bool InOrOut;

	HWDeviceDected();
#ifdef CFG_LINEIN_DET_EN
	InOrOut = IsLineInLink();
	if(InOrOut)
	{
		Ret |= LINEIN_STATE_BIT;
	}
	switch(LineInState)
	{
		case DETECT_STATE_IDLE:
			if(InOrOut)
			{
				LineInState = DETECT_STATE_IN_JITTER;
				TimeOutSet(&LineDetectTimer, DETECT_JITTER_TIME);
			}
			else
			{
				Ret |= LINEIN_EVENT_BIT;
				LineInState = DETECT_STATE_OUT_JITTER;
				TimeOutSet(&LineDetectTimer, DETECT_OUT_WAIT_TIME);
			}
			break;
		case DETECT_STATE_IN:
			if(!InOrOut)
			{
				Ret |= LINEIN_EVENT_BIT;
				LineInState = DETECT_STATE_OUT_JITTER;
				TimeOutSet(&LineDetectTimer, DETECT_OUT_WAIT_TIME);
			}
			break;
		case DETECT_STATE_OUT_JITTER:
			if(InOrOut)
			{
				LineInState = DETECT_STATE_IN_JITTER;
				TimeOutSet(&LineDetectTimer, DETECT_JITTER_TIME);
			}
			else if(IsTimeOut(&LineDetectTimer))//卡弹出消抖完成
			{
				LineInState = DETECT_STATE_OUT;
			}
			break;
		case DETECT_STATE_OUT:
			if(InOrOut)
			{
				LineInState = DETECT_STATE_IN_JITTER;
				TimeOutSet(&LineDetectTimer, DETECT_JITTER_TIME);
			}
			break;
		case DETECT_STATE_IN_JITTER://卡插入消抖完成
			if(!InOrOut)
			{
				LineInState = DETECT_STATE_OUT_JITTER;
				TimeOutSet(&LineDetectTimer, DETECT_OUT_WAIT_TIME);
			}
			else if(IsTimeOut(&LineDetectTimer))
			{
				Ret |= LINEIN_EVENT_BIT;
				LineInState = DETECT_STATE_IN;
			}
	}
#endif

#ifdef HDMI_HPD_CHECK_DETECT_EN
		InOrOut = IsHDMILink();
		if(InOrOut)
		{
			Ret |= HDMI_HPD_STATE_BIT;
		}

		switch(HDMI_Device_State)
		{
			case DETECT_STATE_IDLE:
				if(InOrOut)
				{
					HDMI_Device_State = DETECT_STATE_IN_JITTER;
					TimeOutSet(&HDMIDetectTimer, DETECT_JITTER_TIME);
				}
				else
				{
					HDMI_Device_State = DETECT_STATE_OUT_JITTER;
					TimeOutSet(&HDMIDetectTimer, DETECT_JITTER_TIME);
				}
				break;
			case DETECT_STATE_IN:
				if(!InOrOut)
				{
					HDMI_Device_State = DETECT_STATE_OUT_JITTER;
					TimeOutSet(&HDMIDetectTimer, DETECT_JITTER_TIME);//800
				}
				break;
			case DETECT_STATE_OUT_JITTER:
				if(InOrOut)
				{
					HDMI_Device_State = DETECT_STATE_IN_JITTER;
					TimeOutSet(&HDMIDetectTimer, DETECT_JITTER_TIME);
				}
				else if(IsTimeOut(&HDMIDetectTimer))//卡弹出消抖完成
				{
					HDMI_Device_State = DETECT_STATE_OUT;
					TimeOutSet(&HDMIExitTimer, 800);
					HDMIExitFlg = 1;
					mainAppCt.hdmiResetFlg = 1;
					Ret |= HDMI_HPD_EVENT_BIT;
				}
				break;
			case DETECT_STATE_OUT:
				if(InOrOut)
				{
					HDMIExitFlg = 0;
					HDMI_Device_State = DETECT_STATE_IN_JITTER;
					TimeOutSet(&HDMIDetectTimer, DETECT_JITTER_TIME);
				}
				break;
			case DETECT_STATE_IN_JITTER://卡插入消抖完成
				if(!InOrOut)
				{
					HDMI_Device_State = DETECT_STATE_OUT_JITTER;
					TimeOutSet(&HDMIDetectTimer, DETECT_JITTER_TIME);//800
				}
				else if(IsTimeOut(&HDMIDetectTimer))
				{
					Ret |= HDMI_HPD_EVENT_BIT;
					HDMI_Device_State = DETECT_STATE_IN;
				}
				break;
		}
#endif

	//CARD插拔检测
#ifdef CFG_FUNC_CARD_DETECT
//if(sd_card_detect_xms_polling<6)sd_card_detect_xms_polling++;
//else
	{
	
		//sd_card_detect_xms_polling=0;
		
		InOrOut = IsCardIn();
		if(InOrOut)
		{
			Ret |= CARDIN_STATE_BIT;
		}
		switch(CardState)
		{
			case DETECT_STATE_IDLE:
				if(InOrOut)
				{
					CardState = DETECT_STATE_IN_JITTER;
					TimeOutSet(&CardDetectTimer, DETECT_JITTER_TIME);
					//				APP_DBG("Detect:CardPlugInJitter\n");
				}
				else
				{
					Ret |= CARDIN_EVENT_BIT;
					CardState = DETECT_STATE_OUT_JITTER;
					TimeOutSet(&CardDetectTimer, DETECT_OUT_WAIT_TIME);
					//				APP_DBG("Detect:CardPlugOutJitter\n");
				}
				break;
			case DETECT_STATE_IN:
				if(!InOrOut)
				{
					Ret |= CARDIN_EVENT_BIT;
					CardState = DETECT_STATE_OUT_JITTER;
					TimeOutSet(&CardDetectTimer, DETECT_OUT_WAIT_TIME);
					//				APP_DBG("Detect:CardPlugOutJitter\n");
				}
				break;
			case DETECT_STATE_OUT_JITTER:
				if(InOrOut)
				{
					CardState = DETECT_STATE_IN_JITTER;
					TimeOutSet(&CardDetectTimer, DETECT_JITTER_TIME);
					//				APP_DBG("CardPlugInJitter\n");
				}
				else if(IsTimeOut(&CardDetectTimer))//卡弹出消抖完成
				{
					//				APP_DBG("Detect:CardPlugOut\n");
					CardState = DETECT_STATE_OUT;
				}
				break;
			case DETECT_STATE_OUT:
				if(InOrOut)
				{
					CardState = DETECT_STATE_IN_JITTER;
					TimeOutSet(&CardDetectTimer, DETECT_JITTER_TIME);
					//				APP_DBG("Detect:CardPlugInJitter\n");
				}
				break;
			case DETECT_STATE_IN_JITTER://卡插入消抖完成
				if(!InOrOut)
				{
					CardState = DETECT_STATE_OUT_JITTER;
					TimeOutSet(&CardDetectTimer, DETECT_OUT_WAIT_TIME);
					//				APP_DBG("Detect:CardPlugOutJitter\n");
				}
				else if(IsTimeOut(&CardDetectTimer))
				{
					//				APP_DBG("Detect:CardPlugIn\n");
					Ret |= CARDIN_EVENT_BIT;
					CardState = DETECT_STATE_IN;
				}
				break;
		}
	}
#endif

#ifdef CFG_FUNC_UDISK_DETECT
#include "otg_host_hcd.h"
#ifdef CFG_FUNC_USB_DEVICE_EN
	if(USB_Device_State != DETECT_STATE_IN)
#endif
	{
		//USB插拔检测
		InOrOut = IsUDiskLink();
		if(InOrOut)
		{
			Ret |= UDISKIN_STATE_BIT;
		}	
		switch(UDiskState)
		{
			case DETECT_STATE_IDLE:
				if(InOrOut)
				{
					OTG_HostControlInit();

					UDiskState = DETECT_STATE_IN_JITTER;
					TimeOutSet(&UDiskDetectTimer, DETECT_JITTER_TIME);
				}
				else
				{
					UDiskState = DETECT_STATE_OUT;
					APP_DBG("Detect:NoUDisk\n");
				}

				break;
			case DETECT_STATE_IN:
				if(!InOrOut)
				{
					Ret |= UDISKIN_EVENT_BIT;
					UDiskState = DETECT_STATE_OUT;
					APP_DBG("Detect:UDiskOut\n");
					IsUDiskRemoved = TRUE;
					CmdErrCnt = 0;
				}			
				break;
			case DETECT_STATE_OUT:
				if(InOrOut)
				{
					OTG_HostControlInit();//for U盘兼容性，快速回应U盘上电。
					TimeOutSet(&UDiskDetectTimer, DETECT_JITTER_TIME);
					UDiskState = DETECT_STATE_IN_JITTER;
				}
				break;
			case DETECT_STATE_IN_JITTER:
				if(InOrOut)
				{
					if(IsTimeOut(&UDiskDetectTimer))
					{
						UDiskState = DETECT_STATE_IN;
						Ret |= UDISKIN_EVENT_BIT;
						APP_DBG("Detect:UDiskInPlug\n");
					}
				}
				else
				{
					UDiskState = DETECT_STATE_OUT;
					APP_DBG("Detect:UDiskOut\n");
				}
				break;
			default:
				break;
		}
	}
#endif

#ifdef CFG_FUNC_USB_DEVICE_EN
#ifdef CFG_FUNC_UDISK_DETECT
	if(UDiskState != DETECT_STATE_IN && UDiskState != DETECT_STATE_IN_JITTER)
#endif
	{		
		//USB插拔检测
		if(GetSystemMode()<AppModeLineAudioPlay)
			return Ret;
			
		InOrOut = OTG_PortDeviceIsLink();
		if(InOrOut)
		{
			Ret |= USB_DEVICE_STATE_BIT;
		}
		switch(USB_Device_State)
		{
			case DETECT_STATE_IDLE:
				if(InOrOut)
				{
					USB_Device_State = DETECT_STATE_IN;
					Ret |= USB_DEVICE_EVENT_BIT;
					APP_DBG("Detect:Is usb device\n");

#ifdef CFG_COMMUNICATION_BY_USB
					if(sDevice_Init_state==0)
					{
						if(GetSystemMode() == AppModeUsbDevicePlay)
							OTG_DeviceModeSel(CFG_PARA_USB_MODE, USB_VID, USBPID(CFG_PARA_USB_MODE));
						else
							OTG_DeviceModeSel(HID, USB_VID, USBPID(HID));

						OTG_DeviceInit();
						NVIC_EnableIRQ(Usb_IRQn);
						sDevice_Init_state=1;
					}
#endif
				}
				else
				{
					USB_Device_State = DETECT_STATE_OUT;
					APP_DBG("Detect:Not usb device\n");

#ifdef CFG_COMMUNICATION_BY_USB
					if(sDevice_Init_state==1)
					{
						NVIC_DisableIRQ(Usb_IRQn);
						OTG_DeviceDisConnect();
						sDevice_Init_state=0;
					}
#endif
				}
				break;
			case DETECT_STATE_IN:
				if(!InOrOut)
				{
					Ret |= USB_DEVICE_EVENT_BIT;
					USB_Device_State = DETECT_STATE_OUT;
					APP_DBG("Detect:Out as usb device\n");

#ifdef CFG_COMMUNICATION_BY_USB
                    sDevice_Inserted_Flag = 0;
					if(sDevice_Init_state==1)
					{
						NVIC_DisableIRQ(Usb_IRQn);
						OTG_DeviceDisConnect();
						sDevice_Init_state=0;
					}
#endif
				}
				break;
			case DETECT_STATE_OUT:
				if(InOrOut)
				{
					Ret |= USB_DEVICE_EVENT_BIT;
					USB_Device_State = DETECT_STATE_IN;
					APP_DBG("Detect:Plus as usb device\n");

#ifdef CFG_COMMUNICATION_BY_USB
					if(sDevice_Init_state==0)
					{
						if(GetSystemMode() == AppModeUsbDevicePlay)
							OTG_DeviceModeSel(CFG_PARA_USB_MODE, USB_VID, USBPID(CFG_PARA_USB_MODE));
						else
							OTG_DeviceModeSel(HID, USB_VID, USBPID(HID));

						OTG_DeviceInit();
						NVIC_EnableIRQ(Usb_IRQn);
						sDevice_Init_state=1;
					}
#endif
				}
				break;
			default:
				break;
		}
	}
#endif

	return Ret;
}


#ifdef CFG_FUNC_UDISK_DETECT
bool IsUDiskLink(void)
{
//extern osMutexId MuteReadBlock;// = NULL;
//extern osMutexId MuteWriteBlock;// = NULL;

	bool IsUDiskLink;

	IsUDiskLink = OTG_PortHostIsLink();

//    if(IsUDiskLink == FALSE)
//    {
//        osMutexUnlock(MuteWriteBlock);
//        osMutexUnlock(MuteReadBlock);
//    }

    return IsUDiskLink;
}
#endif



// 上电时，硬件扫描消抖
void InitDeviceDetect(void)
{
#ifdef FUNC_OS_EN
	if(SDIOMutex == NULL)
		SDIOMutex = osMutexCreate();
#endif

/*
 * give 20 ms enough to device detection
 */

//#ifdef CFG_FUNC_CARD_DETECT
//	IsCardInFlag = IsCardIn();
//#endif

//#ifdef CFG_FUNC_UDISK_DETECT
//	UDiskState = DETECT_STATE_IDLE;
//	IsUDiskLinkFlag = IsUDiskLink();
//	APP_DBG("IsUDiskLinkFlag = %d\n", IsUDiskLinkFlag);
//#endif
//		IsLineInLink();
#ifdef FUNC_MIC_DET_EN
	IsMicLinkFlag = IsMicInLink();
#endif

#ifdef HDMI_HPD_CHECK_DETECT_EN
	HDMI_HPD_CHECK_IO_INIT();
#endif

	HWDeviceDected_Init();
}

#if (defined(CFG_APP_USB_AUDIO_MODE_EN))&&(defined(CFG_COMMUNICATION_BY_USB))
void SetDeviceDetectVarReset(void)
{
#ifdef CFG_FUNC_UDISK_DETECT
	UDiskState=DETECT_STATE_IDLE;
#endif
#ifdef CFG_FUNC_USB_DEVICE_EN
	USB_Device_State=DETECT_STATE_IDLE;
#endif
	sDevice_Init_state=0;
}

void SetDeviceDetectVarReset_Deepsleep(void)
{
#ifdef CFG_FUNC_USB_DEVICE_EN
	USB_Device_State=DETECT_STATE_IDLE;
#endif
	sDevice_Init_state=0;
}
#endif

#ifdef CFG_COMMUNICATION_BY_USB
void SetDeviceInitState(uint32_t init_state)
{
	sDevice_Init_state=init_state;
}

uint32_t GetDeviceInitState(void)
{
	return sDevice_Init_state; 	
}
#endif

#if (defined(CFG_APP_USB_AUDIO_MODE_EN))&&(defined(CFG_COMMUNICATION_BY_USB))
void SetUSBAudioExitFlag(uint8_t flag)
{
	sUSB_Audio_Exit_Flag=flag;
}
uint8_t GetUSBAudioExitFlag(void)
{
	return sUSB_Audio_Exit_Flag;
}
#endif



