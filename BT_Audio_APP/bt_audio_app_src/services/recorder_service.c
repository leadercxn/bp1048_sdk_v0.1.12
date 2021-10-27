/**
 **************************************************************************************
 * @file    recorder_service.c
 * @brief   
 *
 * @author  Pi
 * @version V1.0.0
 *
 * $Created: 2018-04-26 13:06:47$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "type.h"
#include "rtos_api.h"
#include "app_message.h"
#include "app_config.h"
#include "debug.h"
#include "gpio.h"
#include "mp3enc_api.h"
#include "mode_switch_api.h"
#include "audio_core_api.h"
#include "mcu_circular_buf.h"
#include "recorder_service.h"
#include "timeout.h"
#ifdef CFG_FUNC_RECORD_SD_UDISK
#include "ff.h"
#include "sd_card.h"
#include "otg_host_hcd.h"
#include "otg_host_standard_enum.h"
#include "otg_host_udisk.h"
#elif defined(CFG_FUNC_RECORD_FLASHFS)
#include "file.h"
#endif
#include "main_task.h"
#include <nds32_intrinsic.h>
#include "device_service.h"
#ifdef	CFG_FUNC_RECORDER_EN

//static uint32_t sRecSamples    = 0;
//#define	MEDIA_RECORDER_FILE_SECOND			30	//自动截断文件再建立新文件，屏蔽后录成单个文件。

#define ENCODER_MP3_OUT_BUF_SIZE			MP3_ENCODER_OUTBUF_CAPACITY//((RecorderCt->SamplePerFrame * MEDIA_RECORDER_BITRATE * 1000 / 8) / CFG_PARA_SAMPLE_RATE + 1100)//单次编码帧（576/1152)输出的size，8为防溢出。
#if FILE_WRITE_FIFO_LEN <= 2048
#define FILE_WRITE_BUF_LEN					(512)
#else
#define FILE_WRITE_BUF_LEN					(FILE_WRITE_FIFO_LEN / 4)
#endif


#define MEDIA_RECORDER_TASK_STACK_SIZE		384// 1024
#define MEDIA_RECORDER_TASK_PRIO			2
#define MEDIA_RECORDER_RECV_MSG_TIMEOUT		1

#define MEDIA_RECORDER_NUM_MESSAGE_QUEUE	10


typedef enum
{
	RecorderStateNone = 0,
	RecorderStateEncode,	//编码
	RecorderStateWriteFile,	//写盘
	RecorderStateSaveFlie,	//文件处理，存盘或者关闭文件

}RecorderState;


typedef struct _MediaRecorderContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;
	MessageHandle		parentMsgHandle;

	TaskState			state;
	RecorderState		RecState;			

	AudioCoreSink 		*AudioCoreSinkRecorder;
	uint16_t			*Sink1Buf_Carry;//audiocore输出通道，
	uint16_t			*Sink1Fifo;//sink和recorder之间缓冲。
	MCU_CIRCULAR_CONTEXT	Sink1FifoCircular;


	/* for Encoder */
	uint16_t 			RemainSample;//编码buf当前采样点数
	int16_t				*EncodeBuf;
	MP3EncoderContext	*Mp3EncCon;
	uint8_t				*Mp3OutBuf;
	int32_t				SamplePerFrame;
	
	/* for file */
	MCU_CIRCULAR_CONTEXT FileWCircularBuf;
	int8_t				*FileWFifo;
	int8_t				*WriteBuf; 	//写盘buf
	osMutexId			FifoMutex;//录音buf 推拉流 锁
	
	bool				RecorderOn;
	bool				EncodeOn;
	uint32_t 			SampleRate; //带提示音时，如果不重采样，此处设成提示音采样率一致。
	uint32_t			sRecSamples;
	uint32_t			sRecordingTime ; // unit ms
	uint32_t			sRecordingTimePre ;
	uint32_t			SyncFileMs;//上次同步的录音长度。避免拔出设备引起 文件长度为0。


#ifdef CFG_FUNC_RECORD_SD_UDISK
	FIL					RecordFile;

	FATFS				MediaFatfs;
	uint8_t 			DiskVolume;//设备录音和回放 卷
	uint16_t			FileIndex; //录音文件编号
	uint16_t			FileNum;//录音文件计数器，限制数量以便回放排序。
	//文件操作
	FIL 				FileHandle;
	DIR 				Dir;
	FILINFO 			FileInfo;
#elif defined(CFG_FUNC_RECORD_FLASHFS)
	FFILE				*RecordFile;

#endif
}MediaRecorderContext;

static  MediaRecorderContext*		RecorderCt;
void MediaRecorderStopProcess(void);

#ifdef CFG_FUNC_RECORD_SD_UDISK
uint16_t RecFileIndex(char *string)
{
	uint32_t Index;
	if(string[5] != '.')
	{
		//APP_DBG("fileName error\n");
		return 0;
	}
	if(string[0] > '9' || string[0] < '0'|| string[1] > '9' || string[1] < '0' || string[2] > '9' || string[2] < '0' || string[3] > '9' || string[3] < '0' || string[4] > '9' || string[4] < '0')
	{
		//APP_DBG("%c%c%c%c%c", string[0], string[1], string[2], string[3], string[4]);
		return 0;
	}
	if(string[6] !=  'M' || string[7] !=  'P' || string[8] !=  '3')
	{
		//APP_DBG("not mp3");
		return 0;
	}
	Index = (string[0] - '0') * 10000 + (string[1] - '0') * 1000	+ (string[2] - '0') * 100 + (string[3] - '0') * 10 + (string[4] - '0');
	if(Index > FILE_NAME_MAX)
	{
		APP_DBG("File index too large\n");
	}
	return Index;
}

void IntToStrMP3Name(char *string, uint16_t number)
{
	if(number > FILE_NAME_MAX)
	{
		return ;
	}
	string[0] = number / 10000 + '0';
	string[1] = (number % 10000) / 1000 + '0';
	string[2] = (number % 1000) / 100 + '0';
	string[3] = (number % 100) / 10 + '0';
	string[4] = number % 10 + '0';
	string[5] = '.';
	string[6] = 'M';
	string[7] = 'P';
	string[8] = '3';
	string[9] = '\0';
}

//注意Vol 为卷号，0 或1；
static void FilePathStringGet(char *FilePath, uint32_t FileIndex, uint8_t Vol)
{
	if(Vol == MEDIA_VOLUME_U)
	{
		strcpy(FilePath, MEDIA_VOLUME_STR_U);
	}
	else
	{
		strcpy(FilePath, MEDIA_VOLUME_STR_C);
	}
	strcat(FilePath, CFG_PARA_RECORDS_FOLDER);

	strcat(FilePath + strlen(FilePath), "/");

	IntToStrMP3Name(FilePath + strlen(FilePath), FileIndex);
}
void DelRecFileRecSevice(uint32_t current_rec_index);// 
//文件序号初始化，需要先初始化卷号。
static bool FileIndexInit(void)
{
#ifdef CFG_FUNC_RECORD_SD_UDISK
	uint32_t Index = 0;
	char FilePath[FILE_PATH_LEN];
	uint16_t RecFileList = 0;

	if(RecorderCt->DiskVolume == MEDIA_VOLUME_U)
	{
		strcpy(FilePath, MEDIA_VOLUME_STR_U);
		f_chdrive(MEDIA_VOLUME_STR_U);
	}
	else
	{
		strcpy(FilePath, MEDIA_VOLUME_STR_C);
		f_chdrive(MEDIA_VOLUME_STR_C);
	}
//	strcpy(FilePath, MEDIA_VOLUME_STR);
//	FilePath[0] += Volume;
//	f_chdrive(FilePath);
	strcat(FilePath, CFG_PARA_RECORDS_FOLDER);
	RecorderCt->FileIndex = 0;
	RecorderCt->FileNum = 0;
	if(FR_OK != f_opendir(&RecorderCt->Dir, FilePath))
	{
		APP_DBG("f_opendir failed: %s\n", FilePath);
		if(f_mkdir((TCHAR*)FilePath) != FR_OK)
		{
			MediaRecorderStopProcess();
			return FALSE;
		}
		else
		{
			RecorderCt->FileIndex = 1;
			RecorderCt->FileNum = 0;
			return TRUE;
		}
	}
	else
	{
		while(((f_readdir(&RecorderCt->Dir, &RecorderCt->FileInfo)) == FR_OK) && RecorderCt->FileInfo.fname[0])
		{
			if(RecorderCt->FileInfo.fattrib & AM_ARC)
			{
				Index = RecFileIndex(RecorderCt->FileInfo.fname);
				if(Index > 0 && Index <= FILE_NAME_MAX)
				{
					RecorderCt->FileNum++;
					if(Index > RecFileList)
					{
						RecFileList = Index;//找已存在的录音文件最大编号
					}
				}
			}
		}
		if(RecFileList < FILE_NAME_MAX && RecorderCt->FileNum < FILE_INDEX_MAX)
		{
			RecorderCt->FileIndex = RecFileList + 1;//最大编号下一个编号作为新录音文件编号
			return TRUE;
		}
#ifdef AUTO_DEL_REC_FILE_FUNCTION
		else
		{
			uint32_t i_count=FILE_INDEX_MAX;
			while(i_count>=1)
			{
				APP_DBG("del i_count=%d\n",i_count);
				DelRecFileRecSevice(i_count);
				i_count--;
			}
			RecorderCt->FileIndex = 1;
			RecorderCt->FileNum = 0;
			return TRUE;
		}
#endif
	}
	return FALSE;
#endif
}

#ifdef MEDIA_RECORDER_FILE_SECOND
static bool MediaRecorderNextFileIndex()
{
	bool Ret = FALSE;
	char FilePath[FILE_PATH_LEN + 1];
	
	while(!Ret)
	{
		RecorderCt->FileIndex++;
		if(RecorderCt->FileIndex > FILE_NAME_MAX || RecorderCt->FileNum >= FILE_INDEX_MAX)
		{
			return FALSE;
		}
		FilePathStringGet(FilePath, RecorderCt->FileIndex, RecorderCt->DiskVolume);
		if((f_open(&RecorderCt->FileHandle, (TCHAR*)FilePath, FA_READ | FA_OPEN_EXISTING)) == FR_OK)
		{
			f_close(&RecorderCt->FileHandle);
		}
		else
		{
			Ret = TRUE;
		}
	}
	return Ret;
}
#endif
#endif //CFG_FUNC_RECORD_SD_UDISK

//name must be a short name!!!
static bool MediaRecorderOpenDataFile(void)//uint8_t SongType)
{
#ifdef CFG_FUNC_RECORD_SD_UDISK
	char FilePath[FILE_PATH_LEN + 1];
	if(RecorderCt->FileIndex > FILE_NAME_MAX)
	{
		return FALSE;
	}
	FilePathStringGet(FilePath, RecorderCt->FileIndex, RecorderCt->DiskVolume);
	APP_DBG("Name:%s", FilePath);
	if((f_open(&RecorderCt->RecordFile, (TCHAR*)FilePath, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		RecorderCt->RecorderOn = FALSE;
		APP_DBG("Open file error!\n");
		return FALSE;
	}
	else
	{
		APP_DBG("Open File ok!\n");
		RecorderCt->FileNum++;//以文件创建为基准，增加文件计数器
		return TRUE;
	}
#elif defined(CFG_FUNC_RECORD_FLASHFS)
	if((RecorderCt->RecordFile = Fopen(CFG_PARA_FLASHFS_FILE_NAME, "w")) == NULL)
	{
		APP_DBG("Open file error!\n");
		return FALSE;
	}
	else
	{
		APP_DBG("Open File ok!\n");
		return TRUE;
	}
#endif
}


uint16_t MediaRecorderDataSpaceLenGet(void)
{
	return MCUCircular_GetSpaceLen(&RecorderCt->Sink1FifoCircular) / (MEDIA_RECORDER_CHANNEL* 2);
}

//Len: Unit: SampleS
uint16_t MediaRecorderDataSet(void* Buf, uint16_t Len)
{

	int16_t* pBuf = Buf;
	if(Len == 0)
	{
		return 0;
	}
	if(RecorderCt->RecorderOn)
	{
		if(MCUCircular_GetSpaceLen(&RecorderCt->Sink1FifoCircular) / (MEDIA_RECORDER_CHANNEL* 2) <= Len)
			APP_DBG("*");
#if (MEDIA_RECORDER_CHANNEL == 1)
		uint16_t i;
		for(i=0; i<Len; i++)
		{
			pBuf[i] = __nds32__clips((int32_t)pBuf[2 * i] + pBuf[2 * i + 1], (16)-1);//__SSAT((((int32_t)pBuf[2 * i] + (int32_t)pBuf[2 * i + 1]) / 2), 16);
		}
#endif

		MCUCircular_PutData(&RecorderCt->Sink1FifoCircular, pBuf, Len * MEDIA_RECORDER_CHANNEL * 2);

	}
	return 0;
}

bool encoder_init(int32_t num_channels, int32_t sample_rate, int32_t *samples_per_frame)
{
	*samples_per_frame = (sample_rate > 32000)?(1152):(576);
	if(sample_rate < 24000)
	{
		return mp3_encoder_initialize(RecorderCt->Mp3EncCon, num_channels, sample_rate, MEDIA_RECORDER_BITRATE, 0);
	}
	else
	{
		return mp3_encoder_initialize(RecorderCt->Mp3EncCon, num_channels, sample_rate, MEDIA_RECORDER_BITRATE, 12000);
	}
}

bool encoder_encode(int16_t *pcm_in, uint8_t *data_out, uint32_t *plength)
{
	return mp3_encoder_encode(RecorderCt->Mp3EncCon, pcm_in, data_out, plength);
}

void MediaRecorderEncode(void)
{
	uint32_t Len;
	if(RecorderCt != NULL && RecorderCt->RecorderOn)
	{
		RecorderCt->EncodeOn = TRUE;
		Len = MCUCircular_GetDataLen(&RecorderCt->Sink1FifoCircular);
		if(Len / (MEDIA_RECORDER_CHANNEL * 2) + RecorderCt->RemainSample  >= RecorderCt->SamplePerFrame)
		{

			MCUCircular_GetData(&RecorderCt->Sink1FifoCircular, 
							RecorderCt->EncodeBuf + RecorderCt->RemainSample * MEDIA_RECORDER_CHANNEL,
							(RecorderCt->SamplePerFrame - RecorderCt->RemainSample) * MEDIA_RECORDER_CHANNEL * 2);
			if(!encoder_encode(RecorderCt->EncodeBuf, RecorderCt->Mp3OutBuf, &Len))// len=313
			{
				if(MCUCircular_GetSpaceLen(&RecorderCt->FileWCircularBuf) < Len)
				{
					APP_DBG("Encoder:fifo Error!\n");//兼容性错误警告。录音丢数据了，FILE_WRITE_FIFO_LEN不适配。
				}
				osMutexLock(RecorderCt->FifoMutex);
				MCUCircular_PutData(&RecorderCt->FileWCircularBuf, RecorderCt->Mp3OutBuf, Len);
				osMutexUnlock(RecorderCt->FifoMutex);
				RecorderCt->sRecSamples += RecorderCt->SamplePerFrame;
				//APP_DBG("RecorderCt->RecSampleS = %d\n", RecorderCt->sRecSamples);
			}
			else
			{
				APP_DBG("Encoder Error!\n");
				RecorderCt->EncodeOn = FALSE;
				return ;//编码帧错误，丢弃。
			}
			RecorderCt->RemainSample = 0;
		}
		else if(Len >= MEDIA_RECORDER_CHANNEL * 2)
		{
			MCUCircular_GetData(&RecorderCt->Sink1FifoCircular, 
							RecorderCt->EncodeBuf + RecorderCt->RemainSample * MEDIA_RECORDER_CHANNEL,
							Len);
			RecorderCt->RemainSample += Len /(MEDIA_RECORDER_CHANNEL * 2);
		}
		RecorderCt->EncodeOn = FALSE;
	}
}

void MediaRecorderStopProcess(void)
{
	MessageContext	msgSend;
	uint32_t 		Times;

	Times = ((uint64_t)RecorderCt->sRecSamples * 1000) / RecorderCt->SampleRate;
	while(RecorderCt->EncodeOn);
	AudioCoreSinkDisable(AUDIO_RECORDER_SINK_NUM);
	if(RecorderCt->RecorderOn)
	{
		APP_DBG("Close\n");
#ifdef CFG_FUNC_RECORD_SD_UDISK
		f_close(&RecorderCt->RecordFile);
#elif defined(CFG_FUNC_RECORD_FLASHFS) 
		if(RecorderCt->RecordFile)
		{
			Fclose(RecorderCt->RecordFile);
			RecorderCt->RecordFile = NULL;
		}
#endif
	}
#ifdef CFG_FUNC_RECORDS_MIN_TIME
	if(Times < CFG_FUNC_RECORDS_MIN_TIME && RecorderCt->RecorderOn)
	{
#ifdef CFG_FUNC_RECORD_SD_UDISK
		char FilePath[FILE_PATH_LEN];
		FilePathStringGet(FilePath, RecorderCt->FileIndex, RecorderCt->DiskVolume);
		f_unlink(FilePath);
		RecorderCt->FileNum--;
#elif defined(CFG_FUNC_RECORD_FLASHFS) 
		Remove(CFG_PARA_FLASHFS_FILE_NAME);
#endif
		APP_DBG("Delete file , too short!\n");
	}
//#ifdef CFG_FUNC_RECORD_SD_UDISK
//	else //录盘条件下，文件没有删，需切到下一首，准备好录下一首。
//	{
//		
//	}
//#endif //#ifdef CFG_FUNC_RECORD_SD_UDISK
#endif //#ifdef CFG_FUNC_RECORDS_MIN_TIME

	Times /= 1000;//转换为秒
	RecorderCt->RecorderOn = FALSE;

	if(Times)
	{
		APP_DBG("Recorded %d M %d S\n", (int)(Times / 60), (int)(Times % 60));
	}
	RecorderCt->sRecSamples = 0;
	RecorderCt->RemainSample = 0;
	RecorderCt->SyncFileMs = 0;
	RecorderCt->state = TaskStatePaused;
	msgSend.msgId = MSG_MEDIA_RECORDER_STOPPED;
	MessageSend(RecorderCt->parentMsgHandle, &msgSend);
//	SoftFlagDeregister(SoftFlagRecording);//清理
}

static void MediaRecorderServicesCreateProcess(void)
{
	// Create service task
}

static void MediaRecorderServiceStartProcess(void)
{
	RecorderCt->state = TaskStateStarting;
}

//All of services is created
//Send CREATED message to parent
//this task only audio core service
static void MsgProcessServiceCreated(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_MEDIA_RECORDER_SERVICE_CREATED;
	MessageSend(RecorderCt->parentMsgHandle, &msgSend);

	RecorderCt->state = TaskStateReady;
}

static void MsgProcessServiceStarted(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_MEDIA_RECORDER_SERVICE_STARTED;
	MessageSend(RecorderCt->parentMsgHandle, &msgSend);
	RecorderCt->state = TaskStateRunning;
	
}
#ifdef CFG_FUNC_UDISK_DETECT
extern bool UDiskRemovedFlagGet(void);
extern void UDiskRemovedFlagSet(bool State);
#endif
#ifdef CFG_FUNC_RECORD_SD_UDISK
bool RecordDiskMount(void)
{
	bool ret = TRUE;

#ifdef  CFG_FUNC_RECORD_UDISK_FIRST
#ifdef CFG_FUNC_UDISK_DETECT
	if(ResourceValue(AppResourceUDisk))
	{
		if(UDiskRemovedFlagGet())
		{
			OTG_HostControlInit();
			OTG_HostEnumDevice();
			if(UDiskInit() == FALSE)
			{
				ret = FALSE;
			}				
		}
		UDiskRemovedFlagSet(0);
		APP_DBG("枚举MASSSTORAGE接口OK\n");
		if(ret == TRUE)
		{
			if(f_mount(&RecorderCt->MediaFatfs, MEDIA_VOLUME_STR_U, 1) == 0)
			{
				RecorderCt->DiskVolume = 1;
				APP_DBG("U盘卡挂载到 1:/--> 成功\n");
			}
			else
			{
				ret = FALSE;
				APP_DBG("U盘卡挂载到 1:/--> 失败\n");
			}
		}
	}
	else
#endif
	{
		ret = FALSE;
	}
	if(!ret)
	{
		ret = TRUE;
		if(ResourceValue(AppResourceCard))
		{
			CardPortInit(CFG_RES_CARD_GPIO);
			if(SDCard_Init() == 0)
			{
				APP_DBG("SDCard Init Success!\n");
				if(f_mount(&RecorderCt->MediaFatfs, MEDIA_VOLUME_STR_C, 1) == 0)
				{
					RecorderCt->DiskVolume = 0;
					APP_DBG("SD mount 0:/--> ok\n");
				}
				else
				{
					ret = FALSE;
					APP_DBG("SD mount 0:/--> fail\n");
				}
			}
			else
			{
				ret = FALSE;
				APP_DBG("SdInit Failed!\n");
			}
		}
		else
		{
			ret = FALSE;
		}
	}
#else
	if(ResourceValue(AppResourceCard))
	{
		CardPortInit(CFG_RES_CARD_GPIO);
		if(SDCard_Init() == 0)
		{
			APP_DBG("SDCard Init Success!\n");
			if(f_mount(&RecorderCt->MediaFatfs, MEDIA_VOLUME_STR_C, 1) == 0)
			{
				RecorderCt->DiskVolume = 0;
				APP_DBG("SD mount 0:/--> ok\n");
			}
			else
			{
				ret = FALSE;
				APP_DBG("SD mount 0:/--> fail\n");
			}
		}
		else
		{
			ret = FALSE;
			APP_DBG("SdInit Failed!\n");
		}
	}
	else
	{
		ret = FALSE;
	}
#ifdef CFG_FUNC_UDISK_DETECT
	if(!ret)
	{
		ret = TRUE;
		if(ResourceValue(AppResourceUDisk))
		{
			if(UDiskRemovedFlagGet())
			{
				OTG_HostControlInit();
				OTG_HostEnumDevice();
				if(UDiskInit() == FALSE)
				{
					ret = FALSE;
				}				
			}
			UDiskRemovedFlagSet(0);
			APP_DBG("枚举MASSSTORAGE接口OK\n");
			if(ret == TRUE)
			{
				if(f_mount(&RecorderCt->MediaFatfs, MEDIA_VOLUME_STR_U, 1) == 0)
				{
					RecorderCt->DiskVolume = 1;
					APP_DBG("U盘卡挂载到 1:/--> 成功\n");
				}
				else
				{
					ret = FALSE;
					APP_DBG("U盘卡挂载到 1:/--> 失败\n");
				}
			}
		}
		else
		{
			ret = FALSE;
		}
	}
#endif
#endif

	return ret;
}
#endif

void ShowRecordingTime(void);
static bool MediaRecorderDataProcess(void)
{
	uint32_t LenFile, Len = 0;

	switch(RecorderCt->RecState)
	{
		case RecorderStateWriteFile:
			//写卡处理
			osMutexLock(RecorderCt->FifoMutex);
			if(RecorderCt->FileWCircularBuf.W < RecorderCt->FileWCircularBuf.R)// 读指针在buf末尾
			{
				RecorderCt->WriteBuf = &RecorderCt->FileWCircularBuf.CircularBuf[RecorderCt->FileWCircularBuf.R];
				Len = RecorderCt->FileWCircularBuf.BufDepth - RecorderCt->FileWCircularBuf.R;
			}
			else if((LenFile = MCUCircular_GetDataLen(&RecorderCt->FileWCircularBuf)) < FILE_WRITE_BUF_LEN)
			{
				osMutexUnlock(RecorderCt->FifoMutex);
				break;
			}
			else
			{
				RecorderCt->WriteBuf = &RecorderCt->FileWCircularBuf.CircularBuf[RecorderCt->FileWCircularBuf.R];
				Len = ((RecorderCt->FileWCircularBuf.W - RecorderCt->FileWCircularBuf.R) / 512) * 512;
			}
			osMutexUnlock(RecorderCt->FifoMutex);

			unsigned int RetLen;
#ifdef CFG_FUNC_RECORD_SD_UDISK			
			//录音媒介 兼容性测试 请分析此行执行阻塞情况。
			FRESULT Ret = f_write(&RecorderCt->RecordFile,RecorderCt->WriteBuf, Len, &RetLen);
			if((Ret != FR_OK) || (Ret == FR_OK && Len != RetLen))
#elif defined(CFG_FUNC_RECORD_FLASHFS)
			if(!Fwrite(RecorderCt->WriteBuf, Len, 1, RecorderCt->RecordFile))
#endif
			{
				APP_DBG("File Write Error!\n");
				RecorderCt->FileWCircularBuf.R = (RecorderCt->FileWCircularBuf.R + RetLen) % RecorderCt->FileWCircularBuf.BufDepth;
#ifdef CFG_FUNC_RECORD_SD_UDISK
				if(RecorderCt->MediaFatfs.free_clst <= 1) //fat缺省0xFFFFFFFF, 当系统更新此参数有效时，输出log
				{
					APP_DBG("No disk space for record\n");
				}
#endif
				MessageContext		msgSend;
				msgSend.msgId		= MSG_MEDIA_RECORDER_ERROR;
				MessageSend(RecorderCt->parentMsgHandle, &msgSend);
				return FALSE;//写入错误，停止，应该加上磁盘信息。比如空间满。
			}
			RecorderCt->FileWCircularBuf.R = (RecorderCt->FileWCircularBuf.R + Len) % RecorderCt->FileWCircularBuf.BufDepth;
			RecorderCt->RecState = RecorderStateSaveFlie;
			break;
		case RecorderStateSaveFlie:
			
			RecorderCt->sRecordingTime=((uint64_t)RecorderCt->sRecSamples * 1000) / RecorderCt->SampleRate;
			//if(GetRecEncodeState()!=TaskStatePausing)
			ShowRecordingTime();
			
			//存盘处理
			//此处设置2秒钟同步一次文件，以防突然拔卡操作/断电。
			if(((uint64_t)RecorderCt->sRecSamples * 1000) / RecorderCt->SampleRate - RecorderCt->SyncFileMs >= 2000
				|| RecorderCt->SyncFileMs == 0 ) {
				//同步是写入fat表和文件长度信息。媒介兼容性测试时，需关注此行执行阻塞情况。
#ifdef CFG_FUNC_RECORD_SD_UDISK
				if(f_sync(&RecorderCt->RecordFile) == FR_OK)
#elif defined(CFG_FUNC_RECORD_FLASHFS)
				Fflush( RecorderCt->RecordFile);
#endif
				{
					RecorderCt->SyncFileMs = ((uint64_t)RecorderCt->sRecSamples * 1000) / RecorderCt->SampleRate;
					//APP_DBG(" File Ms = %d\n", (int)RecorderCt->SyncFileMs);
				}
			}
			
#if defined(MEDIA_RECORDER_FILE_SECOND) && defined(CFG_FUNC_RECORD_SD_UDISK)
			uint32_t Times;
			Times = (RecorderCt->sRecSamples) / RecorderCt->SampleRate;

			if(Times >= MEDIA_RECORDER_FILE_SECOND) //文件长度取决于预设时长。
			{
				APP_DBG("File Recorded %d M %d S\n", (int)(Times / 60), (int)(Times % 60));
				RecorderCt->sRecSamples = 0;
				RecorderCt->SyncFileMs = 0;
				f_close(&RecorderCt->RecordFile);
				if(MediaRecorderNextFileIndex())
				{
					if(MediaRecorderOpenDataFile())
					{
						return TRUE;//打开成功则继续录音
					}
				}
				return FALSE;
			}
#endif
			RecorderCt->RecState = RecorderStateWriteFile;
			break;
		default:
			break;
	}
	return TRUE;
}

static void MediaRecorderServiceStopProcess(void)
{
	if(RecorderCt->state != TaskStateStopping
			&& RecorderCt->state != TaskStateStopped
			&& RecorderCt->state != TaskStateNone)
	{
		if(RecorderCt->state != TaskStatePaused)
		{
			MediaRecorderStopProcess();
		}
		RecorderCt->state = TaskStateStopping;
	}
}

static void MsgProcessServiceStopped(void)
{
	MessageContext		msgSend;
	
	//clear msg
	MessageClear(RecorderCt->msgHandle);
	if(RecorderCt->state != TaskStateStopped
			&& RecorderCt->state != TaskStateNone)
	{
		//Set state
		RecorderCt->state = TaskStateStopped;
		//reply
		msgSend.msgId		= MSG_MEDIA_RECORDER_SERVICE_STOPPED;
		MessageSend(RecorderCt->parentMsgHandle, &msgSend);
	}
}

/**
 * @func        MediaRecorder_Init
 * @brief       MediaRecorder模式参数配置，资源初始化
 * @param       MessageHandle parentMsgHandle
 * @Output      None
 * @return      int32_t
 * @Others      任务块、Adc、Dac、AudioCore配置
 * @Others      数据流从Adc到audiocore配有函数指针，audioCore到Dac同理，由audiocoreService任务按需驱动
 * Record
 */


bool MallocRecorderCtSink1Buf(uint16_t sampleperframe)
{
	if(RecorderCt!=NULL&&RecorderCt->Sink1Buf_Carry == NULL)
	{
		RecorderCt->Sink1Buf_Carry = (uint16_t*)osPortMallocFromEnd(sampleperframe * 2 * 2);//DAC fifo
		if(RecorderCt->Sink1Buf_Carry != NULL)
		{
			memset(RecorderCt->Sink1Buf_Carry, 0, sampleperframe * 2 * 2);
			return TRUE;
		}
		else
		{
			APP_DBG("malloc RecorderCt Sink1Buf  error\n");
			return FALSE;
		}
	}
	return TRUE;
}

void ReleaseRecorderCtSink1Buf(void)
{
	if(RecorderCt!=NULL&&RecorderCt->Sink1Buf_Carry!=NULL)
	{
		APP_DBG("Release RecorderCt Sink1Buf \n");
		osPortFree(RecorderCt->Sink1Buf_Carry);
		RecorderCt->Sink1Buf_Carry = NULL;
	}
}
void AudioRecResInit(void)
{
	if(RecorderCt!=NULL&&RecorderCt->Sink1Buf_Carry!=NULL)
		RecorderCt->AudioCoreSinkRecorder->PcmOutBuf = (int16_t *)RecorderCt->Sink1Buf_Carry;
}

static bool MediaRecorder_Init(MessageHandle parentMsgHandle)
{
	bool ret = TRUE;

	RecorderCt = (MediaRecorderContext*)osPortMalloc(sizeof(MediaRecorderContext));
	if(RecorderCt == NULL)
	{
		return FALSE;
	}
	//Task config & Service para
	memset(RecorderCt, 0, sizeof(MediaRecorderContext));
	RecorderCt->msgHandle = MessageRegister(MEDIA_RECORDER_NUM_MESSAGE_QUEUE);
	if(RecorderCt->msgHandle == NULL)
	{
		return FALSE;
	}
	APP_DBG("R:%d\n", (int)xPortGetFreeHeapSize());
	RecorderCt->parentMsgHandle = parentMsgHandle;
	RecorderCt->state = TaskStateCreating;
	//para
	RecorderCt->SampleRate = CFG_PARA_SAMPLE_RATE; //注意采样率同步，从sink1获取。
	RecorderCt->RecorderOn = FALSE;
	RecorderCt->EncodeOn = FALSE;

	//编码控制 句柄
	RecorderCt->Mp3EncCon = (MP3EncoderContext*)osPortMalloc(sizeof(MP3EncoderContext));
	if(RecorderCt->Mp3EncCon  == NULL)
	{
		return FALSE;
	}
//	if((RecorderCt->Mp3EncCon = (MP3EncoderContext*)osPortMalloc(sizeof(MP3EncoderContext))) == NULL)
//	{
//		return FALSE;
//	}

	encoder_init(MEDIA_RECORDER_CHANNEL, RecorderCt->SampleRate, &(RecorderCt->SamplePerFrame));
	if(RecorderCt->SamplePerFrame > MEDIA_ENCODER_SAMPLE_MAX)
	{
		APP_DBG("Encoder frame error!!!");//保护性检测
	}

	// Audio core map;
//	ReleaseRecorderCtSink1Buf();

	if(!MallocRecorderCtSink1Buf(mainAppCt.SamplesPreFrame)) //One frame
	{
		return FALSE;
	}

	RecorderCt->Sink1Fifo = (uint16_t*)osPortMalloc(MEDIA_RECORDER_FIFO_LEN);
	if(RecorderCt->Sink1Fifo  == NULL)
	{
		return FALSE;
	}
//	if((RecorderCt->Sink1Fifo = (uint16_t*)osPortMalloc(MEDIA_RECORDER_FIFO_LEN)) == NULL)
//	{
//		return FALSE;
//	}
	MCUCircular_Config(&RecorderCt->Sink1FifoCircular, RecorderCt->Sink1Fifo, MEDIA_RECORDER_FIFO_LEN);

	
	//Core	Sink1 Para
	RecorderCt->AudioCoreSinkRecorder =  &AudioCore.AudioSink[AUDIO_RECORDER_SINK_NUM];

	RecorderCt->AudioCoreSinkRecorder->Enable = 0;
	RecorderCt->AudioCoreSinkRecorder->PcmFormat = 2;//stereo
	RecorderCt->AudioCoreSinkRecorder->FuncDataSet = MediaRecorderDataSet;
	RecorderCt->AudioCoreSinkRecorder->FuncDataSpaceLenGet = MediaRecorderDataSpaceLenGet;
	RecorderCt->AudioCoreSinkRecorder->PcmOutBuf = (int16_t *)RecorderCt->Sink1Buf_Carry;


	//编码输入buf
	if(RecorderCt->EncodeBuf == NULL) 
	{
		RecorderCt->EncodeBuf = (int16_t*)osPortMalloc(RecorderCt->SamplePerFrame * MEDIA_RECORDER_CHANNEL * 2);
		if(RecorderCt->EncodeBuf == NULL)
		{
			return FALSE;
		}

//		if(((RecorderCt->EncodeBuf = (int16_t*)osPortMalloc(RecorderCt->SamplePerFrame * MEDIA_RECORDER_CHANNEL * 2))) == NULL)
//		{
//			return FALSE;
//		}
	}
	//编码输出buf
	RecorderCt->Mp3OutBuf = osPortMalloc(ENCODER_MP3_OUT_BUF_SIZE);
	if(RecorderCt->Mp3OutBuf == NULL)
	{
		return FALSE;
	}

//	if((RecorderCt->Mp3OutBuf = osPortMalloc(ENCODER_MP3_OUT_BUF_SIZE)) == NULL)
//	{
//		return FALSE;
//	}

	if(RecorderCt->FileWFifo == NULL)
	{
		RecorderCt->FileWFifo = (int8_t*)osPortMalloc(FILE_WRITE_FIFO_LEN);
		if(RecorderCt->FileWFifo == NULL)//MP3 和pcm size 需求不同
		{
			return FALSE;
		}

//		if(((RecorderCt->FileWFifo = (int8_t*)osPortMalloc(FILE_WRITE_FIFO_LEN))) == NULL)//MP3 和pcm size 需求不同
//		{
//			return FALSE;
//		}
	}
	MCUCircular_Config(&RecorderCt->FileWCircularBuf, RecorderCt->FileWFifo, FILE_WRITE_FIFO_LEN);

	if((RecorderCt->FifoMutex = xSemaphoreCreateMutex()) == NULL)
	{
		return FALSE;
	}

	//录音模式下，写文件时的搬数据buf ********注释此段，借用fifo空间，写入文件系统，节约ram************
//	if((RecorderCt->WriteBuf = osPortMalloc(FILE_WRITE_BUF_LEN)) == NULL)
//	{
//		return FALSE;
//	}

#ifdef CFG_FUNC_RECORD_SD_UDISK
	RecorderCt->FileNum = 0;
#endif
	return ret;
}

static void MediaRecorderEntrance(void * param)
{
	MessageContext		msgRecv;
	
	// Create services
	APP_DBG("RecorderEntrance \n");
	MediaRecorderServicesCreateProcess();//空
	APP_DBG("MediaRecorder Service\n");
	MsgProcessServiceCreated();//无service，直接返回created
	SoftFlagRegister(SoftFlagRecording);//登记录音状态，用于屏蔽后插先播
	SetRecState(0);
	while(1)
	{
		MessageRecv(RecorderCt->msgHandle, &msgRecv, MEDIA_RECORDER_RECV_MSG_TIMEOUT);
		switch(msgRecv.msgId)
		{
			case MSG_TASK_CREATE://API, not msg, only happy
				break;

			case MSG_TASK_START:
				MediaRecorderServiceStartProcess();
				MsgProcessServiceStarted();//无service，直接向上级返回
				break;
			
			case MSG_TASK_STOP:
				MediaRecorderServiceStopProcess();
				MsgProcessServiceStopped();//无service，直接向上级返回
				break;

			case MSG_MEDIA_RECORDER_RUN:
				if(!RecorderCt->RecorderOn)
				{
#ifdef CFG_FUNC_RECORD_SD_UDISK
					//bool	Result = FALSE;
					if(RecordDiskMount())
					{	
						if(RecorderCt->MediaFatfs.free_clst != 0xFFFFFFFF) //fat系统 缺省值，即存在有效值时，输出log
						{
							APP_DBG("Space Remain: %u (Clst) * %u Sector\n", (unsigned int)RecorderCt->MediaFatfs.free_clst, (unsigned int)RecorderCt->MediaFatfs.csize);
						}
						
						if(FileIndexInit() == FALSE)      
						{       
							RecorderCt->RecorderOn = FALSE;       
							RecorderCt->RecState = RecorderStateNone;       
							APP_DBG("Rec Number Full!\n");       
							break;      
						} 
					}

#endif//注意：flashfs dev初始化上电时进行。
					RecorderCt->RecorderOn = TRUE;
					RecorderCt->RecState = RecorderStateWriteFile;//RecorderStateEncode;//sam add
					if(MediaRecorderOpenDataFile())
					{
						//MCUCircular_Config(&RecorderCt->CircularHandle, RecorderCt->SinkRecorderFIFO, MEDIA_RECORDER_FIFO_LEN);
						AudioCoreSinkEnable(AUDIO_RECORDER_SINK_NUM);// BKD CHANGE ,DO not change again please
						APP_DBG("Audio core sink 1 enable\n");
						break;
					}
					MediaRecorderStopProcess();
					APP_DBG("MediaRecorder Init buf & Disk Space error!\n");
				}
				break;

			case MSG_MEDIA_RECORDER_STOP://
				if(RecorderCt->state != TaskStatePaused)
				{
					MediaRecorderStopProcess();
				}
				break;
			default:
				break;
		}
		if(RecorderCt->RecorderOn)
		{
			if(MediaRecorderDataProcess() == FALSE)
			{
				if(RecorderCt->state != TaskStatePaused)
				{
					MediaRecorderStopProcess();
				}
			}
		}
	}
}

/***************************************************************************************
 *
 * APIs
 *
 */
bool MediaRecorderServiceCreate(MessageHandle parentMsgHandle)
{
	bool		ret = TRUE;
	APP_DBG("MediaRecorderServiceCreate\n");
	ret = MediaRecorder_Init(parentMsgHandle);
	if(ret)
	{
		RecorderCt->taskHandle = NULL;
		xTaskCreate(MediaRecorderEntrance,
					"Recorder",
					MEDIA_RECORDER_TASK_STACK_SIZE,
					NULL,
					MEDIA_RECORDER_TASK_PRIO,
					&RecorderCt->taskHandle);
		if(RecorderCt->taskHandle == NULL)
		{
			ret = FALSE;
		}
	}
	APP_DBG("R:%d\n", (int)xPortGetFreeHeapSize());
	if(!ret)
	{
		MediaRecorderServiceKill();
		APP_DBG("Media Recorder service: create fail!\n");
	}
	return ret;
}

bool MediaRecorderServiceStart(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_START;
	if(RecorderCt == NULL)
	{
		return FALSE;
	}
	return MessageSend(RecorderCt->msgHandle, &msgSend);
}


bool MediaRecorderServicePause(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_PAUSE;
	if(RecorderCt == NULL)
	{
		return FALSE;
	}
	return MessageSend(RecorderCt->msgHandle, &msgSend);
}

bool MediaRecorderServiceResume(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_RESUME;
	if(RecorderCt == NULL)
	{
		return FALSE;
	}
	return MessageSend(RecorderCt->msgHandle, &msgSend);
}

bool MediaRecorderServiceStop(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_TASK_STOP;
	if(RecorderCt == NULL)
	{
		return FALSE;
	}
	return MessageSend(RecorderCt->msgHandle, &msgSend);
}

bool MediaRecorderServiceKill(void)
{
	SoftFlagDeregister(SoftFlagRecording);//清理
	//Kill used services
	if(RecorderCt == NULL)
	{
		return FALSE;
	}
	AudioCoreSinkDisable(AUDIO_RECORDER_SINK_NUM);
	//task
	if(RecorderCt->taskHandle != NULL)
	{
		vTaskDelete(RecorderCt->taskHandle);
		RecorderCt->taskHandle = NULL;
	}
	//Msgbox
	if(RecorderCt->msgHandle != NULL)
	{
		MessageDeregister(RecorderCt->msgHandle);
		RecorderCt->msgHandle = NULL;
	}

#ifdef CFG_FUNC_RECORD_SD_UDISK
		if(RecorderCt->DiskVolume == MEDIA_VOLUME_U)
		{
			f_unmount(MEDIA_VOLUME_STR_U);
			ResourceRegister(AppResourceUDisk | AppResourceUDiskForPlay);
		}
		else
		{
			f_unmount(MEDIA_VOLUME_STR_C);
			ResourceRegister(AppResourceCard | AppResourceCardForPlay);
			if(SDIOMutex)
			{
				osMutexLock(SDIOMutex);
			}
			SDCardDeinit(CFG_RES_CARD_GPIO);
			{
				osMutexUnlock(SDIOMutex);
			}
		}
#endif
	//PortFree
	if(RecorderCt->Mp3EncCon != NULL)
	{
		osPortFree(RecorderCt->Mp3EncCon);
		RecorderCt->Mp3EncCon = NULL;
	}
	if(RecorderCt->EncodeBuf != NULL)
	{
		osPortFree(RecorderCt->EncodeBuf);
		RecorderCt->EncodeBuf = NULL;
	}
	if(RecorderCt->Mp3OutBuf != NULL)
	{
		osPortFree(RecorderCt->Mp3OutBuf);
		RecorderCt->Mp3OutBuf = NULL;
	}
//	if(RecorderCt->WriteBuf != NULL)
//	{
//		osPortFree(RecorderCt->WriteBuf);
//		RecorderCt->WriteBuf = NULL;
//	}
	if(RecorderCt->FifoMutex != NULL)
	{
		vSemaphoreDelete(RecorderCt->FifoMutex);
		RecorderCt->FifoMutex = NULL;
	}
	if(RecorderCt->FileWFifo != NULL)
	{
		osPortFree(RecorderCt->FileWFifo);
		RecorderCt->FileWFifo = NULL;
	}
	if(RecorderCt->Sink1Fifo != NULL)
	{
		osPortFree(RecorderCt->Sink1Fifo);
		RecorderCt->Sink1Fifo = NULL;
	}
	RecorderCt->AudioCoreSinkRecorder = NULL;
	if(RecorderCt->Sink1Buf_Carry != NULL)
	{
		osPortFree(RecorderCt->Sink1Buf_Carry);
		RecorderCt->Sink1Buf_Carry = NULL;
	}
	osPortFree(RecorderCt);
	RecorderCt = NULL;
	return TRUE;
}


MessageHandle GetMediaRecorderMessageHandle(void)
{
	APP_DBG("'H'");
	if(RecorderCt != NULL)
	{
		return RecorderCt->msgHandle;
	}
	else
	{
		return NULL;
	}
}


void MediaRecorderRun(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_MEDIA_RECORDER_RUN;
	if(RecorderCt != NULL)
	{
		MessageSend(RecorderCt->msgHandle, &msgSend);
	}
}

void MediaRecorderStop(void)
{
	MessageContext		msgSend;

	msgSend.msgId		= MSG_MEDIA_RECORDER_STOP;
	if(RecorderCt != NULL)
	{
		MessageSend(RecorderCt->msgHandle, &msgSend);
	}
}




void ShowRecordingTime(void)
{
#ifdef CFG_FUNC_RECORD_SD_UDISK
 if((RecorderCt->sRecordingTime / 1000 - RecorderCt->sRecordingTimePre / 1000 >= 1))
	 {
	  APP_DBG("%u.mp3 Recording(%lds)\n",RecorderCt->FileIndex,RecorderCt->sRecordingTime / 1000);//can use IntToStrMP3Name()
	  RecorderCt->sRecordingTimePre = RecorderCt->sRecordingTime;
	 }
#endif 
}

bool IsRecoding(void)
{
	if(RecorderCt != NULL && RecorderCt->RecorderOn)
		return TRUE;
	else
		return FALSE;

}
#define MALLOC_REAL_SIZE(a)		((a) % 32 ? ((a) / 32 + 2) * 32 : ((a) / 32 + 1) * 32)
bool MediaRecordHeapEnough(void)
{
	uint32_t TotalSize;
	/******OS********/
	TotalSize = (256 + 192);//TCB 和 mailbox，注意前者也可能是128.
	TotalSize += MALLOC_REAL_SIZE(MEDIA_RECORDER_TASK_STACK_SIZE * 4);//Stack
	/*********PCM FIFO************/
	TotalSize += MALLOC_REAL_SIZE(MEDIA_RECORDER_FIFO_LEN);
	/**********Encode buf****************/
	TotalSize += MALLOC_REAL_SIZE(((CFG_PARA_SAMPLE_RATE > 32000)?(1152):(576)) * 2 * MEDIA_RECORDER_CHANNEL);
	/************Encoder**************************/
	TotalSize += MALLOC_REAL_SIZE(sizeof(MP3EncoderContext));
	/**************Encode out*********************/
	TotalSize += MALLOC_REAL_SIZE(ENCODER_MP3_OUT_BUF_SIZE);
	/**************MP3 Data FIFO for write************************/
	TotalSize += MALLOC_REAL_SIZE(FILE_WRITE_FIFO_LEN);
	/*************Recoder********************/
	TotalSize += MALLOC_REAL_SIZE(sizeof(MediaRecorderContext));
	/*************Sink Frame**************************/
	TotalSize += 4 * mainAppCt.SamplesPreFrame + 32;
	if(xPortGetFreeHeapSize() < TotalSize)
	{
		APP_DBG("Error: Need %d， ram not enough!\n", (int)TotalSize);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}



void DelRecFileRecSevice(uint32_t current_rec_index)// 
{
#ifdef CFG_FUNC_RECORD_SD_UDISK
	char current_vol_rec[8];
	char FilePath[FILE_PATH_LEN];
	if(RecorderCt->DiskVolume)	
	{
		strcpy(current_vol_rec, MEDIA_VOLUME_STR_U);
	}
	else
	{
		strcpy(current_vol_rec, MEDIA_VOLUME_STR_C);
	}

	strcpy(FilePath, current_vol_rec);
	strcat(FilePath, CFG_PARA_RECORDS_FOLDER);
	strcat(FilePath,"/");
	IntToStrMP3Name(FilePath + strlen(FilePath),current_rec_index);
	if(f_unlink(FilePath))
	{
		APP_DBG("%dREC.mp3 DEL failure\n",current_rec_index);
	}
		
	
#endif	
}

#endif //ifdef CFG_FUNC_RECORDER_EN
