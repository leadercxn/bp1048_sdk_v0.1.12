///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: breakpoint.c
//  ChangLog :
//			�޸�bp ģ�������ʽ2014-9-26 ��lujiangang
///////////////////////////////////////////////////////////////////////////////

#include "app_config.h"
#include "breakpoint.h"
#include "mode_switch_api.h"
#include "media_play_api.h"
#include "spi_flash.h"
#include "debug.h"
#include "file.h"
#include "ffpresearch.h"
#include "backup.h"
#include "rtos_api.h"
#include "device_service.h"
#include "main_task.h"
#include "ctrlvars.h"
#ifdef CFG_FUNC_BREAKPOINT_EN


#ifdef BP_SAVE_TO_FLASH // �������
static bool LoadBPInfoFromFlash(void);
static bool SaveBPInfoToFlash(void);
#endif
#ifdef BP_PART_SAVE_TO_NVM
static bool LoadBPPartInfoFromNVM(void);
static bool SaveBPPartInfoToNVM(void);
#endif
static BP_INFO bp_info;

// global sysInfo default init nvm value
const static BP_SYS_INFO sInitSysInfo =
{
#ifdef CFG_APP_BT_MODE_EN
	(uint8_t)AppModeBtAudioPlay,                // CurModuleId
#else
	(uint8_t)AppModeUDiskAudioPlay,             // CurModuleId
#endif

	CFG_PARA_SYS_VOLUME_DEFAULT,     			//Music Volume default value
	
#ifdef CFG_FUNC_MIC_KARAOKE_EN
    EFFECT_MODE_HunXiang,                       //Audio effect mode default value  
#else
    EFFECT_MODE_NORMAL,                         //Audio effect mode default value  
#endif

    MAX_MIC_DIG_STEP,                           //Mic Volume default value
    
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	0,                                          //Eq mode default value 
#endif

	MAX_MIC_REVB_STEP,                          //Reverb step default value                      
#ifdef CFG_FUNC_MIC_TREB_BASS_EN
	7,                                          //Bass default value    
    7,	                    					//Treb default value    
#endif

#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
	7,                                          //Bass default value    
    7,	                    					//Treb default value    
#endif
    
#ifdef CFG_FUNC_REMIND_SOUND_EN
	1,                             				// 1:Enable Alarm Remind;
	1,                            				// 1:Chinese Mode
#endif
#ifdef FUNC_BT_HF_EN
	15,								            //bt hf volume
#endif
};

#if (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN))
const static BP_PLAYER_INFO sInitPlayerInfo =
{
	25, 					// Volume
	PLAY_MODE_REPEAT_ALL,  	// PlayMode
	0, 						// LrcFlag
	{0}, 					// CardPlay
	{0}, 					// UDiskPlay
};

#endif

#ifdef CFG_APP_RADIOIN_MODE_EN // radio module default init nvm value
const static BP_RADIO_INFO sInitRadioInfo =
{
	{0}, 					// �ѱ����̨�б�/*MAX_RADIO_CHANNEL_NUM*/
	10,  					// FM����
	0,   					// ��ǰFM���η�Χ(00B��87.5~108MHz (US/Europe, China)��01B��76~90MHz (Japan)��10B��65.8~73MHz (Russia)��11B��60~76MHz
	0,  					// �ѱ����̨����
	0,   					// ��ǰ��̨Ƶ��
};
#endif

// ����CRC
uint8_t GetCrc8CheckSum(uint8_t* ptr, uint32_t len)
{
	uint32_t crc = 0;
	uint32_t i;

	while(len--)
	{
		crc ^= *ptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 0x01)
			{
				crc = ((crc >> 1) ^ 0x8C);
			}
			else
			{
				crc >>= 1;
			}
		}
	}
	return (uint8_t)crc;
}

// ϵͳ����ʱװ��, ��Flash����BP ��Ϣ
//�������ʧ�������Ĭ��BP ֵ
//ע�⣬�ú������ܻ��ȡFlash�������Ҫ��SpiFlashInfoInit()����֮�����
void BP_LoadInfo(void)
{
	bool ret = FALSE;

	//APP_DBG("BP_LoadInfo\n");
#ifdef BP_SAVE_TO_FLASH
	ret = LoadBPInfoFromFlash();
#endif

	//��ȡBP ��Ϣʧ�ܣ���Ĭ��ֵ
	if(!ret)
	{
		//APP_DBG("BP load err, user defalut value\n");
		memcpy((void*)&bp_info.SysInfo, (void*)&sInitSysInfo, sizeof(BP_SYS_INFO));
#if (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN))
		memcpy((void*)&bp_info.PlayerInfo, (void*)&sInitPlayerInfo, sizeof(BP_PLAYER_INFO));
#endif

#ifdef CFG_APP_RADIOIN_MODE_EN
		memcpy((void*)&bp_info.RadioInfo, (void*)&sInitRadioInfo, sizeof(BP_RADIO_INFO));
#endif
		BP_SaveInfo(0);
		return;
	}
	
#ifdef BP_PART_SAVE_TO_NVM
	LoadBPPartInfoFromNVM();
#endif
	return;
}

//����bp_info �����ģ��BP ��Ϣ��ָ��
//����Ҫ����BP ��Ϣʱ��ֱ��ͨ����ָ����bp_info ������д
void* BP_GetInfo(BP_INFO_TYPE InfoType)
{
	void *pInfo = NULL;
	switch(InfoType)
	{
		case BP_SYS_INFO_TYPE:
			pInfo = (void *)&(bp_info.SysInfo);
			break;
#if (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN))
		case BP_PLAYER_INFO_TYPE:
			pInfo = (void *)&(bp_info.PlayerInfo);
			break;
#endif			
#ifdef CFG_APP_RADIOIN_MODE_EN
		case BP_RADIO_INFO_TYPE:
			pInfo = (void *)&(bp_info.RadioInfo);
			break;
#endif			
		default:
			break;
	}
	return pInfo;
}

//����BP ��Ϣ��NVM ��FLASH
//�ɹ�����TRUE�� ʧ�ܷ���FALSE
//Device == 0;дflash
//Device == 1;дNVM
bool BP_SaveInfo(uint32_t Device)
{
	uint8_t crc;

	//ASSERT(sizeof(bp_info) <= BP_MAX_SIZE);
	//APP_DBG("\n!BP_SaveInfo!\n");
	//��Save��NVM��FLASH֮ǰ�����ȼ����CRC
	crc = GetCrc8CheckSum((uint8_t*)&bp_info, sizeof(BP_INFO)-1);
	bp_info.Crc = crc;

	if(Device == 0)
	{
#ifdef BP_SAVE_TO_FLASH // �������
		if(!SaveBPInfoToFlash())
		{
			//APP_DBG("SaveBPInfoToFlash == FALSE\n");
			return FALSE;
		}
#endif
	}

	else if(Device == 1)
	{
#ifdef	BP_PART_SAVE_TO_NVM
		if(!SaveBPPartInfoToNVM())
		{
			return FALSE;
		}
#endif
	}
	//APP_DBG("BP Save success\n");
	return TRUE;
}

#ifdef BP_PART_SAVE_TO_NVM
static bool LoadBPPartInfoFromNVM(void)	
{
#if defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN)
	BP_PART_INFO BpPartInfo;
	if(BACKUP_NvmRead(10, (uint8_t *)&BpPartInfo, sizeof(BP_PART_INFO)) == FALSE)
	{
		APP_DBG("NVM load error!\n");
		return FALSE;
	}

	bp_info.PlayerInfo.PlayCardInfo.PlayTime = BpPartInfo.CardPlayTime;
	bp_info.PlayerInfo.PlayUDiskInfo.PlayTime = BpPartInfo.UDiskPlayTime;
	return TRUE;
#endif
	return FALSE;
}

static bool SaveBPPartInfoToNVM(void)
{
#if defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN)
	BP_PART_INFO BpPartInfo;


	BpPartInfo.CardPlayTime = bp_info.PlayerInfo.PlayCardInfo.PlayTime;
	BpPartInfo.UDiskPlayTime = bp_info.PlayerInfo.PlayUDiskInfo.PlayTime;

	if(BACKUP_NvmWrite(10, (uint8_t *)&BpPartInfo, sizeof(BP_PART_INFO)) == FALSE)
	{
		APP_DBG("NVM write error!\n");
		return FALSE;
	}
#endif
	//APP_DBG("NVM write OK\n");
	return TRUE;
}
#endif

#ifdef BP_SAVE_TO_FLASH // �������
#include "spi_flash.h"
#include "flash_config.h"

//�������ʽ��NVM Flag(4Bytes "MV26") + NVM BAK ID(4Bytes) + NVM Size(2Bytes) + NVM SUB BAK ID(1Bytes) + CRC(1Bytes) + NVM Data(Sub1/Sub2/...)��
//�������ͷ��
typedef struct __BP_BAK_WRAPPER_
{
	char 		Magic[4];
	uint32_t 	BPBakId;
	uint16_t 	BPBakSize;
	uint8_t 	BPBakSubId;
	uint8_t 	Crc;
}BP_BAK_WRAPPER;

#define	MAGIC_STRING		"MVB1" //BKD CHANGE
#define NVM_BAK_OFFSET		(BP_DATA_ADDR)	// BP ��Ϣ��Flash�е���ʼ��ַ
#define NVM_BAK_CNT			(8)				// BP ��Ϣ�洢�������,С��8
#define NVM_BLOCK_SIZE		(4096)			// BP ��Ϣ���������С


// BP ��¼��ϢSize : bp_info ��¼��С+ ��¼��ͷ
#define NVM_BAK_SIZE     	(sizeof(BP_INFO) + sizeof(BP_BAK_WRAPPER))	
#define NVM_SUB_BAK_CNT  	((NVM_BLOCK_SIZE)/NVM_BAK_SIZE)

static uint32_t NVM_BAK_ADDR0   = NVM_BAK_OFFSET;	//BP INFO ��Flash�е���ʼ��ַ

// OutputBakId: ���NVM BAK ID
// OutputSubBakId: ���NVM SUB BAK ID
// return: NVM�����ַ,LJG ��������NVM��������block���׵�ַ
static uint32_t FindLatestBPBak(uint32_t* OutputBakId, uint8_t* OutputSubBakId, bool IsLoadFlash)
{
	uint32_t i, j;
	int32_t TempIndex = -1;
	uint32_t TempBakId;
	BP_BAK_WRAPPER BpBakWrapper;
	uint8_t  TempSubBakId;
	uint8_t Buf[NVM_BAK_CNT][2];

	for(i = 0; i < NVM_BAK_CNT; i++)
	{
		// read flash to get nvm bak id
		if(SpiFlashRead(NVM_BAK_ADDR0 + i * NVM_BLOCK_SIZE, (uint8_t*)&BpBakWrapper, sizeof(BpBakWrapper), 10) < 0 ||
		        memcmp(BpBakWrapper.Magic, MAGIC_STRING, 4) != 0)
		{
			//APP_DBG("no Magic\n");
			Buf[i][0] = 0xFF;
			Buf[i][1] = 0xFF;
			continue;
		}

		TempBakId    = BpBakWrapper.BPBakId;
		TempSubBakId = BpBakWrapper.BPBakSubId;
		//APP_DBG("i = %d, BckId = %d, subBakId = %d\n", i, TempBakId, TempSubBakId);
		//APP_DBG("NVM_BAK_SIZE = %d\n", NVM_BAK_SIZE);
		for(j = 0; j < (NVM_BLOCK_SIZE / NVM_BAK_SIZE); j++)
		{
			if(SpiFlashRead(NVM_BAK_ADDR0 + i * NVM_BLOCK_SIZE + j * NVM_BAK_SIZE, (uint8_t*)&BpBakWrapper, sizeof(BpBakWrapper), 10) < 0 ||
			        memcmp(BpBakWrapper.Magic, MAGIC_STRING, 4) != 0)
			{
				break;
			}

			TempBakId    = BpBakWrapper.BPBakId;;
			TempSubBakId = BpBakWrapper.BPBakSubId;
			//APP_DBG("j = %d, BckId = %d, subBakId = %d\n", j, TempBakId, TempSubBakId);
		}

		if(*OutputBakId > TempBakId)
		{
			continue;
		}

		TempIndex       = i;
		*OutputBakId    = TempBakId;//�ڼ�����
		*OutputSubBakId = TempSubBakId;//���еĵڼ���С��
		Buf[i][0] = TempBakId;
		Buf[i][1] = TempSubBakId;
		//APP_DBG("Tmp1 = %d, BckId = %d, subBakId = %d\n", TempIndex, TempBakId, TempSubBakId);
	}
	
	if(TempIndex == -1)
	{
		return 0;//û�����ҵ�������
	}
	//��ȡBP����ʱ��������Ч��
	for(i=0; i<NVM_BAK_CNT; i++)
	{
		if(Buf[i][1] < NVM_SUB_BAK_CNT - 1)
		{
			TempIndex = i;
			*OutputBakId = Buf[i][0];
			*OutputSubBakId = Buf[i][1];
			break;
		}
	}

	if((TempIndex == NVM_BAK_CNT - 1) 
		&& (*OutputBakId == TempIndex)
		&& (*OutputSubBakId == NVM_SUB_BAK_CNT - 1))
	{
		for(i=0; i<NVM_BAK_CNT; i++)
		{
			if((Buf[i][1] == 0xFF) && (Buf[i][0] == 0xFF))
			{
				if(i == 0)
				{
					TempIndex = NVM_BAK_CNT - 1;
				}
				else
				{
					TempIndex = i - 1;
				}
				*OutputBakId = TempIndex;
				*OutputSubBakId = NVM_SUB_BAK_CNT - 1;
				break;
			}
		}
	}
	//APP_DBG("Tmp22 = %d, BckId = %d, subBakId = %d\n", (int)TempIndex, (int)*OutputBakId, (int)*OutputSubBakId);
	return (NVM_BAK_ADDR0 + TempIndex * NVM_BLOCK_SIZE);
}

//�޸ķ���ֵ
//-3: flash��ȡ����
//-2: crc����ֵ���󣬼���ṹ�峤��û�з����仯������û�б䣬��ȡ����������Ӧ�����Ϸ��Լ�飩
//-1: crc����ֵ���󣬼���ṹ�峤�ȷ����˱仯����Ҫ�Լ���������һ��ȫ������
//0: crc����ֵ��ȷ
static int32_t LoadBakBPInfo(uint32_t BakNvmAddr, uint8_t SubBakId)
{
	uint8_t CrcVal;
	uint32_t i;
	
	BP_BAK_WRAPPER BpBakWrapper;
	
	if(SpiFlashRead((uint32_t)(BakNvmAddr + SubBakId * NVM_BAK_SIZE), (uint8_t*)&BpBakWrapper, sizeof(BpBakWrapper), 10) < 0)
	{
		return -3;//FALSE;
	}
	if(SpiFlashRead((uint32_t)(BakNvmAddr + SubBakId * NVM_BAK_SIZE + sizeof(BpBakWrapper)), (uint8_t*)&bp_info, sizeof(bp_info), 10) < 0)
	{
		return -3;//FALSE;
	}
	
	CrcVal = GetCrc8CheckSum((uint8_t*)&bp_info, sizeof(BP_INFO));
	if(CrcVal != BpBakWrapper.Crc)
	{
		if(BpBakWrapper.BPBakSize != sizeof(BP_INFO))
		{
			APP_DBG("BPBakSize changed, erase all\n");
			for(i = 0; i < NVM_BAK_CNT; i++)
			{
				SpiFlashErase(SECTOR_ERASE, (NVM_BAK_OFFSET + i * NVM_BLOCK_SIZE) /4096 , 1);
			}
			return -1;
		}
		//return FALSE;
		return -2;
	}

	return 0;
	//return TRUE;
}

static bool SaveBakBPInfo(uint32_t BakNvmAddr, uint32_t NvmBakId, uint8_t SubBakId)	//SaveBakNVM
{
	uint8_t CrcVal;
	uint16_t NvmSize = sizeof(BP_INFO);
	uint8_t Temp[sizeof(BP_BAK_WRAPPER)+sizeof(BP_INFO)];
	BP_BAK_WRAPPER *pBakWrapper;

	CrcVal = GetCrc8CheckSum((uint8_t*)&bp_info, sizeof(BP_INFO)-1);
	bp_info.Crc = CrcVal;	//��������bp_info�ڲ�CRC

	//��������bp_info��crc
	CrcVal = GetCrc8CheckSum((uint8_t*)&bp_info, sizeof(BP_INFO));
	pBakWrapper = (BP_BAK_WRAPPER*)&Temp[0];
	memcpy((void*)&pBakWrapper->Magic, (void*)MAGIC_STRING, 4);
	pBakWrapper->BPBakId = NvmBakId;
	pBakWrapper->BPBakSize = NvmSize;
	pBakWrapper->BPBakSubId = SubBakId;
	pBakWrapper->Crc = CrcVal;
	memcpy((void*)&Temp[sizeof(BP_BAK_WRAPPER)],(void*)&bp_info, sizeof(BP_INFO));

	if(SubBakId == 0)
	{
		uint8_t i;
		uint8_t buf[4];
//		uint32_t NextBakAddr;

		//APP_DBG("\nSubBak Idex == 0\n");
		SpiFlashRead(BakNvmAddr, buf, 4, 10);
		if(buf[0] !=0xFF ||buf[1] !=0xFF ||buf[2] !=0xFF ||buf[3] !=0xFF)
		{
			for(i=0; i<NVM_BAK_CNT; i++)
			{
				SpiFlashErase(SECTOR_ERASE, (NVM_BAK_OFFSET + i * NVM_BLOCK_SIZE) /4096 , 1);
				//APP_DBG("Erase Flash ok (%d)\n", i);
			}
		}
		if(SpiFlashWrite(BakNvmAddr, Temp, sizeof(Temp), 1) < 0)
		{
			return FALSE;
		}
		return TRUE;
	}

	if(SpiFlashWrite((uint32_t)(BakNvmAddr + SubBakId * NVM_BAK_SIZE), Temp, sizeof(Temp), 1) < 0)
	{
		return FALSE;
	}
	return TRUE;
}

static bool LoadBPInfoFromFlash(void)	
{
	uint32_t i;
	uint32_t LoadNvmBakId = 0;
	uint32_t NvmBakAddr;
	uint8_t  NvmSubBakId;
	int32_t BakBPInfoLoadState;

	NVM_BAK_ADDR0 = NVM_BAK_OFFSET;
	
	APP_DBG("Load BP INFO START\n");

	NvmBakAddr = FindLatestBPBak(&LoadNvmBakId, &NvmSubBakId, TRUE);
	//APP_DBG("NvmBakAddr = %x, LoadNvmBakId = %d\n", NvmBakAddr, LoadNvmBakId);
	
	if(NvmBakAddr == 0)//����û�б�������鿴һ���Ƿ���Ҫ����flash,һ��ֻ����4K,һ��Blcok
	{
		uint8_t i;
		uint8_t buf[4];
//		uint32_t NextBakAddr;

		if(SpiFlashRead(NVM_BAK_ADDR0, buf, 4, 10) < 0)
		{
			return FALSE;//error
		}
		if(buf[0] != 0xFF ||buf[1] != 0xFF || buf[0] != 0xFF || buf[0] != 0xFF )
		{
			for(i=0; i<NVM_BAK_CNT; i++)
			{
				SpiFlashErase(SECTOR_ERASE, (NVM_BAK_OFFSET + i * NVM_BLOCK_SIZE) /4096 , 1);
				//APP_DBG("Frist PowerOn Erase Flash ok (%d)\n", i);
			}
		}
		return FALSE;
	}

	BakBPInfoLoadState = LoadBakBPInfo(NvmBakAddr, NvmSubBakId);
	if(BakBPInfoLoadState == 0)
	{
		return TRUE;
	}
	else if(BakBPInfoLoadState == -2)
	{
		//APP_DBG("load back err\n");
		//���´�����©����ֻ������ÿ��block�����һ������
		// �����������ʧ�ܣ����԰��Ⱥ�˳�����ε�����������ֱ�������ɹ�����������еľ���ʧ��
		for(i = (NvmBakAddr - NVM_BAK_ADDR0) / NVM_BLOCK_SIZE; i < NVM_BAK_CNT; i++)
		{
			if(LoadBakBPInfo(NVM_BAK_ADDR0 + i * NVM_BLOCK_SIZE, (NVM_SUB_BAK_CNT - 1)) == 0)
			{
				return TRUE;
			}
		}

		for(i = 0; i < (NvmBakAddr - NVM_BAK_ADDR0) / NVM_BLOCK_SIZE; i++)
		{
			if(LoadBakBPInfo(NVM_BAK_ADDR0 + i * NVM_BLOCK_SIZE, (NVM_SUB_BAK_CNT - 1)) == 0)
			{
				return TRUE;
			}
		}
		//�����˵����������м���飬��Ϊ���ҵ���Ч����飬����Ϊ���쳣������һ�ε����������ȫ������
		{
			APP_DBG("BPBakSize changed, erase all\n");
			for(i = 0; i < NVM_BAK_CNT; i++)
			{
				SpiFlashErase(SECTOR_ERASE, (NVM_BAK_OFFSET + i * NVM_BLOCK_SIZE) /4096 , 1);
			}
		}
	}

	APP_DBG("Load BP From Flash Fail\n");
	return FALSE;
}

static bool SaveBPInfoToFlash(void)
{
	int32_t i;
	uint32_t NvmBakAddr;//���Լ������
	
	uint32_t SaveNvmBakId = 0;
	uint8_t  SaveNvmSubBakId = NVM_SUB_BAK_CNT - 1;

	NVM_BAK_ADDR0 = NVM_BAK_OFFSET;

	// ���������NVM����
	NvmBakAddr = FindLatestBPBak(&SaveNvmBakId, &SaveNvmSubBakId, FALSE);

	//APP_DBG("NvmBakAddr = %x, SaveNvmSubBakId = %d\n", NvmBakAddr, SaveNvmSubBakId);
	//APP_DBG("SaveNvmBakId = %d\n", SaveNvmBakId);

	if(NvmBakAddr == 0)
	{
		NvmBakAddr = NVM_BAK_OFFSET;
		SaveNvmBakId = 0;
		SaveNvmSubBakId = 0;
	}
	else
	{
		SaveNvmSubBakId++;
		if(SaveNvmSubBakId == NVM_SUB_BAK_CNT)
		{
			SaveNvmSubBakId = 0;
			SaveNvmBakId++;
			if(SaveNvmBakId == NVM_BAK_CNT)
			{
				SaveNvmBakId = 0;
				NvmBakAddr = NVM_BAK_OFFSET;
			}
			else
			{
				NvmBakAddr += NVM_BLOCK_SIZE;
			}
		}		
	}

	// ����NVM����
	//APP_DBG("NvmBakAddr = %x, SaveNvmBakId = %d, SaveNvmSubBakId = %d\n", (unsigned int)NvmBakAddr, (int)SaveNvmBakId, SaveNvmSubBakId);
	if(SaveBakBPInfo(NvmBakAddr, SaveNvmBakId, SaveNvmSubBakId))
	{
		return TRUE;
	}
	else
	{
		//APP_DBG("\n!back err\n");
		// ������뾵��ʧ�ܣ����԰��Ⱥ�˳�����ε��뵽������������ֱ������ɹ�����������еľ�������ʧ��
		for(i = ((NvmBakAddr - NVM_BAK_ADDR0) / NVM_BLOCK_SIZE - 1); i >= 0; i--)
		{
			if(SaveBakBPInfo(NVM_BAK_ADDR0 + i * NVM_BLOCK_SIZE, SaveNvmBakId, 0))
			{
				return TRUE;
			}
		}

		for(i = (NVM_BAK_CNT - 1); i >= (NvmBakAddr - NVM_BAK_ADDR0) / NVM_BLOCK_SIZE; i--)
		{
			if(SaveBakBPInfo(NVM_BAK_ADDR0 + i * NVM_BLOCK_SIZE, SaveNvmBakId, 0))
			{
				return TRUE;
			}
		}
	}

	APP_DBG("Save BP To Flash Fail\n");
	return FALSE;
}
#endif

void BackupInfoUpdata(BACKUP_MODE mode)
{
	MessageContext		msgSend;

	switch(mode)
	{
	case BACKUP_SYS_INFO:
		msgSend.msgId	= MSG_DEVICE_SERVICE_BP_SYS_INFO;
		break;
	case BACKUP_PLAYER_INFO:
		msgSend.msgId	= MSG_DEVICE_SERVICE_BP_PLAYER_INFO;
		break;
#ifdef BP_PART_SAVE_TO_NVM
	case BACKUP_PLAYER_INFO_2NVM:
		msgSend.msgId	= MSG_DEVICE_SERVICE_BP_PLAYER_INFO_2NVM;
		break;
#endif
	case BACKUP_RADIO_INFO:
		msgSend.msgId	= MSG_DEVICE_SERVICE_BP_RADIO_INFO;
		break;
	case BACKUP_ALL_INFO:
		msgSend.msgId	= MSG_DEVICE_SERVICE_BP_ALL_INFO;
		break;
	default:
		break;
	}
	MessageSend(GetDeviceMessageHandle(), &msgSend);
}

//#endif/*CFG_FUNC_BREAKPOINT_EN*/

//#if defined(FUNC_MATCH_PLAYER_BP) && (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN)) 

static bool 	IsFindBpSongFlag = FALSE;
static uint32_t PlaySoneNum = 0;
static uint32_t PlayTime = 0;

void DiskSongSearchBPInit(void)
{
	PlaySoneNum = 0;
	PlayTime = 0;
	IsFindBpSongFlag = FALSE;
}


//typedef void (*ff_presearch_callback)(DIR *dir, FILINFO *finfo, ff_acc_node *acc_node);
void DiskSongSearchBP(DIR *dir, FILINFO *finfo, ff_acc_node *acc_node)
{
	static BP_PLAYER_INFO *BpPlayInfo;
	//static BpPlayInfo = BP_GetInfo(BP_PLAYER_INFO_TYPE);

//	PlaySoneNum = 0;
//	PlayTime = 0;
//	IsFindBpSongFlag = FALSE;
	BpPlayInfo = BP_GetInfo(BP_PLAYER_INFO_TYPE);

	if(IsFindBpSongFlag == FALSE)
	{
		if(GetSystemMode() == AppModeCardAudioPlay)
		{
			//APP_DBG("\n");
			//APP_DBG("BP file size = %d, sect = %d, clust = %d\n", BpPlayInfo->PlayCardInfo.FileSize, BpPlayInfo->PlayCardInfo.DirSect, BpPlayInfo->PlayCardInfo.FirstClust);
			if((BpPlayInfo->PlayCardInfo.DirSect == (uint32_t)dir->sect)
			&& (BpPlayInfo->PlayCardInfo.FirstClust == (uint32_t)finfo->fcl)
			&& (BpPlayInfo->PlayCardInfo.FileSize == (uint32_t)finfo->fsize))
			{
				IsFindBpSongFlag = TRUE;
				PlaySoneNum = acc_node->prior_files_amount;//only test
				PlayTime = BpPlayInfo->PlayCardInfo.PlayTime;
				APP_DBG("Find BP Song, num = %d\n", (int)PlaySoneNum);
			}
		}
		else
		{
			//APP_DBG("\n");
			//APP_DBG("BP file size = %d, sect = %d, clust = %d\n", BpPlayInfo->PlayUDiskInfo.FileSize, BpPlayInfo->PlayUDiskInfo.DirSect, BpPlayInfo->PlayUDiskInfo.FirstClust);
			if((BpPlayInfo->PlayUDiskInfo.DirSect == (uint32_t)dir->sect)
			&& (BpPlayInfo->PlayUDiskInfo.FirstClust == (uint32_t)finfo->fcl)
			&& (BpPlayInfo->PlayUDiskInfo.FileSize == (uint32_t)finfo->fsize))
			{
				IsFindBpSongFlag = TRUE;
				PlaySoneNum = acc_node->prior_files_amount;//only test
				PlayTime = BpPlayInfo->PlayUDiskInfo.PlayTime;
				APP_DBG("Find BP Song, num = %d\n", (int)PlaySoneNum);
			}
		}
	}
}

uint32_t BPDiskSongNumGet(void)
{
	return PlaySoneNum;
}

uint32_t BPDiskSongPlayTimeGet(void)
{
	return PlayTime;
}

uint32_t BPDiskSongPlayModeGet(void)
{
	BP_PLAYER_INFO *pBpMediaPlayInfo;
	uint8_t PlayMode;

	pBpMediaPlayInfo = (BP_PLAYER_INFO *)BP_GetInfo(BP_PLAYER_INFO_TYPE);

	PlayMode = BP_GET_ELEMENT(pBpMediaPlayInfo->PlayMode);

	if(PlayMode >= PLAY_MODE_SUM)
	{
		PlayMode = PLAY_MODE_REPEAT_ALL;
	}

	return PlayMode;
}



#endif //defined(FUNC_MATCH_PLAYER_BP) && (defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN)) 




