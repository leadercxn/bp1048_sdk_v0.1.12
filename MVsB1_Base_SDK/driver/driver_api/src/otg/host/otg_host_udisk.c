/**
 *****************************************************************************
 * @file     otg_host_stor.c
 * @author   Owen
 * @version  V1.0.0
 * @date     7-September-2015
 * @brief    host mass-storage module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2015 MVSilicon </center></h2>
 */


#include <string.h>
#include "type.h"
#include "timeout.h"
#include "delay.h"
#include "debug.h"

//#include "cmsis_os.h"
//#include "app_os.h"

#include "otg_detect.h"
#include "otg_host_hcd.h"
#include "otg_host_udisk.h"

#ifdef FUNC_OS_EN
#include "rtos_api.h" //add for mutex declare
osMutexId UDiskMutex = NULL;
#endif

extern int kprintf(const char *fmt, ...);
#undef  OTG_DBG
#define	OTG_DBG(format, ...)		//printf(format, ##__VA_ARGS__)



uint16_t	gHeadLen = 0;
uint16_t	gDataLen = 0;
uint16_t	gTailLen = 0;
uint8_t		UDiskInitOK = FALSE;
UDISK 		gUDisk;
uint8_t		UDiskBuf[40];
bool 		UDiskNotReadyErrFlag = FALSE;
TIMER 		HostStorWriteTimer = {0};

CBW cbw =
{
	0x43425355,		//	uint32_t		Signature;
	0x00000000,		//	uint32_t		Tag;
	0x00000000,	 	//	uint32_t		DataTransferLength;
	0x00,	 		//	uint8_t		Flags;
	0x00,  			//	uint8_t		Lun;
	0x00,  			//	uint8_t		Length;

	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	}				//	uint8_t		CDB[16];

};

/**
 * @brief  get storage max lun
 * @param  NONE
 * @return max lun
 */
uint8_t UDiskGetMaxLUN()
{
	USB_CTRL_SETUP_REQUEST SetupPacket;	
	uint8_t    CmdGetMaxLun[8] = {0xA1, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
	gUDisk.MaxLUN = 0;
	memcpy((uint8_t *)&SetupPacket,CmdGetMaxLun,8);
	if(OTG_HostControlRead(SetupPacket,&gUDisk.MaxLUN,1,1000) != HOST_NONE_ERR)
	{
		return gUDisk.MaxLUN;
	}
	return 0;
}



/**
 * @brief  discard packets
 * @param  nBytes 长度
 * @return 1-成功，0-失败
 */
bool UDiskDiscardPackets(uint16_t nBytes)
{
	uint8_t Buf[64];
	uint8_t Len;
	uint32_t RxLen;
	if(cbw.CDB[0] == READ_10)
	{
		while(nBytes > 0)
		{
			Len = (nBytes > 64) ? 64 : nBytes;
			nBytes -= Len;
			
			if(OTG_HostBulkRead(&gUDisk.BulkInEp, Buf, Len,&RxLen,10000) != HOST_NONE_ERR)
			{
				return FALSE;
			}
		}		 
	}
	return TRUE;
}


/**
 * @brief  storage bulk transport interface
 * @param  Buf 缓冲区指针
 * @param  Len 长度
 * @return 1-成功，0-失败
 */
uint32_t CmdErrCnt = 0;
bool UDiskBulkTransport(uint8_t* Buf, uint16_t Len)
{
	CSW csw;
//	uint8_t Retry = 2;
	uint32_t RxLen;

	cbw.DataTransferLength = CpuToLe16(Len);

	//支持扇区大小不是512字节的U盘
	if(cbw.CDB[0] == READ_10)
	{
		Len = gDataLen;
	}

	if(OTG_HostBulkWrite(&gUDisk.BulkOutEp,(uint8_t*)&cbw,sizeof(CBW),3000) != HOST_NONE_ERR)
	{
		DBG("cmd error\n");
		CmdErrCnt++;
#ifdef FUNC_OS_EN
		osTaskDelay(1);// sam add 20200916
#endif
		return FALSE;
		//continue;
	}
	if(Len)
	{
		if((cbw.Flags & 0x80) == US_BULK_FLAG_IN)
		{
			//IN PACKET
			//DBG("I");
			UDiskDiscardPackets(gHeadLen);
			if(OTG_HostBulkRead(&gUDisk.BulkInEp, Buf, Len, &RxLen, 10000) != HOST_NONE_ERR)
			{
#ifdef FUNC_OS_EN
				osTaskDelay(1);// sam add 20200916
#endif
				//DBG("IN E\n");
				return FALSE;
			}
			UDiskDiscardPackets(gTailLen);
		}
		else
		{
			//OUT PACKET
			//DBG("O");
#ifdef FUNC_OS_EN
			//osTaskDelay(1);// bkd del 2020/03/24
#else
			//DelayMs(1);
#endif

			if(OTG_HostBulkWrite(&gUDisk.BulkOutEp,Buf,Len,10000) != HOST_NONE_ERR)
			{
#ifdef FUNC_OS_EN
				osTaskDelay(1);// sam add 20200916
#endif
				//DBG("OUT E\n");
				return FALSE;
			}
		}
	}
	if(OTG_HostBulkRead(&gUDisk.BulkInEp,(uint8_t*)&csw,sizeof(CSW),&RxLen,3000) != HOST_NONE_ERR)
	{
		//DBG("OTG read block\n");
		return FALSE;
	}

	if(csw.Status == 0)
	{
		//DBG("B OK\n");
		CmdErrCnt = 0;
		return TRUE;	//SUCCESS
	}


#if 0
	while(Retry-- && OTG_PortHostIsLink()) // Modify by lean.xiong @2013-06-21 设备拔除后不需要再重试
	{
		//DBG("CMD\n");
		if(OTG_HostBulkWrite(&gUDisk.BulkOutEp,(uint8_t*)&cbw,sizeof(CBW),3000) != HOST_NONE_ERR)
		{
			continue;
		}

		if(Len)
		{
			if((cbw.Flags & 0x80) == US_BULK_FLAG_IN)
			{
				//IN PACKET
				//DBG("IN\n");
				UDiskDiscardPackets(gHeadLen);
				if(OTG_HostBulkRead(&gUDisk.BulkInEp, Buf, Len, &RxLen, 10000) != HOST_NONE_ERR)
				{
					OTG_HostBulkRead(&gUDisk.BulkInEp, Buf, Len, &RxLen, 10000);
					continue;
				}
				UDiskDiscardPackets(gTailLen);
			}
			else
			{
				//OUT PACKET
				//DBG("OUT\n");
				if(OTG_HostBulkWrite(&gUDisk.BulkOutEp,Buf,Len,10000) != HOST_NONE_ERR)
				{
					continue;
				}
			}
		}

		//IN STATUS
		//DBG("ST\n");
		if(OTG_HostBulkRead(&gUDisk.BulkInEp,(uint8_t*)&csw,sizeof(CSW),&RxLen,3000) != HOST_NONE_ERR)
		{
			//FIX BUG1056
			//客户的一个RB-539白色读卡器，第一次StorReadFormatCapacity()时，STATUS返回STALL，
			//继续接收一次STATUS，则可以接收到成功的STATUS。
			//PC在接收STATUS时，若接收到STALL，会继续接收一次。
			//if(!UsbHostRcvPacket(&gBulkInPipe, (uint8_t*)&csw, sizeof(CSW), 3000))
			if(OTG_HostBulkRead(&gUDisk.BulkInEp,(uint8_t*)&csw,sizeof(CSW),&RxLen,3000) != HOST_NONE_ERR)
			{
				continue;
			}
		}

		if(csw.Status == 0)
		{
			//DBG("B OK\n");
			return TRUE;	//SUCCESS
		}
	}
#endif
	//DBG("B ER\n");
	return FALSE;
}


/**
 * @brief  inquiry scsi command
 * @param  Lun logic unit number
 * @return 1-成功，0-失败
 */
bool UDiskInquiry(uint8_t Lun)
{
	memset(cbw.CDB, 0x00, sizeof(cbw.CDB));
	cbw.CDB[0] = INQUIRY;
	cbw.CDB[4] = 36;
	cbw.Flags = US_BULK_FLAG_IN;
	cbw.Lun = Lun;
	cbw.Length = 6;

	return UDiskBulkTransport(UDiskBuf, 36);
}


/**
 * @brief  read format capacity scsi command
 * @param  Lun logic unit number
 * @return 1-成功，0-失败
 */
bool UDiskReadFormatCapacity(uint8_t Lun)
{
	memset(cbw.CDB, 0x00, sizeof(cbw.CDB));
	cbw.CDB[0] = READ_FORMAT_CAPACITY;
	cbw.CDB[4] = 12;
	cbw.Flags = US_BULK_FLAG_IN;
	cbw.Lun = Lun;
	cbw.Length = 10;
	return UDiskBulkTransport(UDiskBuf, 12);
}


/**
 * @brief  read capacity scsi command
 * @param  Lun logic unit number
 * @return 1-成功，0-失败
 */
bool UDiskReadCapacity(uint8_t Lun)
{
//	DBG("StorReadCapacity()\n");
	memset(cbw.CDB, 0x00, sizeof(cbw.CDB));
	cbw.CDB[0] = READ_CAPACITY;
//	cbw.CDB[4] = 8;
	cbw.Flags = US_BULK_FLAG_IN;
	cbw.Lun = Lun;
	cbw.Length = 10;

	return UDiskBulkTransport(UDiskBuf, 8);

}


/**
 * @brief  request sense scsi command
 * @param  Lun logic unit number
 * @return 1-成功，0-失败
 */
bool UDiskRequestSense(uint8_t Lun)
{
//	DBG("StorRequestSense()\n");
	memset(cbw.CDB, 0x00, sizeof(cbw.CDB));
	cbw.CDB[0] = REQUEST_SENSE;
	cbw.CDB[4] = 18;
	cbw.Flags = US_BULK_FLAG_IN;
	cbw.Lun = Lun;
	cbw.Length = 12;
	if(UDiskBulkTransport(UDiskBuf, 18))
	{
		if(UDiskBuf[2] == 0x02)	//Not ready
		{
			return FALSE;
		}
	}
	return TRUE;
}


/**
 * @brief  test unit ready scsi command
 * @param  Lun logic unit number
 * @return 1-成功，0-失败
 */
bool UDiskTestUnitReady(uint8_t Lun)
{
//	DBG("StorTestUnitReady()\n");
	memset(cbw.CDB, 0x00, sizeof(cbw.CDB));
	cbw.CDB[0] = TEST_UNIT_READY;
	//cbw.CDB[4] = 0;
	cbw.Flags = US_BULK_FLAG_OUT;
	cbw.Lun = Lun;
	cbw.Length = 6;

	return UDiskBulkTransport(UDiskBuf, 0);
}



/**
 * @brief  read blocks
 * @param  BlockNum block number
 * @param  Buf 读缓冲区指针
 * @param  BlockCnt 要读的block个数
 * @return 1-成功，0-失败
 */
bool UDiskReadBlock(uint32_t BlockNum, void* Buf, uint8_t BlockCnt)
{
	uint8_t		Ret;
	uint8_t		Offset;
	uint8_t		Mult;
	uint8_t     RetryTransCnt = 3;
	
#ifdef FUNC_OS_EN
		osMutexLock(UDiskMutex);
#endif
	
	if(!UDiskInitOK || !OTG_PortHostIsLink())// Modify by lean.xiong @2013-06-21 设备拔除后不需要再重试
	{
		OTG_DBG("HostStorReadBlock() error!\n");
		
#ifdef FUNC_OS_EN
		osMutexUnlock(UDiskMutex);
#endif
		return FALSE;
	}
	gDataLen = BlockCnt * 512;
	if(gUDisk.BlockSize != 512)
	{
		Mult = (gUDisk.BlockSize >> 9);
		Offset = ((uint8_t)BlockNum & (Mult - 1));

		BlockNum /= (gUDisk.BlockSize / 512);
		gHeadLen = (Offset << 9);

		gTailLen = ((0 - Offset - BlockCnt) & (Mult - 1)) * 512;
		BlockCnt = (Offset + BlockCnt + (Mult - 1)) / Mult;
	}

	if(BlockNum > gUDisk.LastLBA)
	{
	
#ifdef FUNC_OS_EN
		osMutexUnlock(UDiskMutex);
#endif
		return FALSE;
	}

	//DBG("HostStorReadBlock(%ld, %-.4X, %d)\n", lba, (uint16_t)buf, (uint16_t)size);
	memset(cbw.CDB, 0x00, sizeof(cbw.CDB));
	cbw.CDB[0] = READ_10;
	((uint32_t*)(&cbw.CDB[2]))[0] = CpuToBe32(BlockNum);
//	cbw.CDB[2] = BlockNum>>24;
//	cbw.CDB[3] = BlockNum>>16;
//	cbw.CDB[4] = BlockNum>>8;
//	cbw.CDB[5] = BlockNum;

	cbw.CDB[8] = BlockCnt;

	cbw.Flags = US_BULK_FLAG_IN;
	cbw.Lun = gUDisk.LuNum;
	cbw.Length = 10;
	Ret = UDiskBulkTransport(Buf, gHeadLen + gDataLen + gTailLen);
	
	if(!Ret)
	{
		while(RetryTransCnt--)
		{
			//如果是设备被拔除，则不再重试。
			if(!OTG_PortHostIsLink())
			{
				OTG_DBG("UsbHostIsLink?\n");
				break;
			}
			memset(cbw.CDB, 0x00, sizeof(cbw.CDB));
			cbw.CDB[0] = READ_10;
			((uint32_t*)(&cbw.CDB[2]))[0] = CpuToBe32(BlockNum);
//			cbw.CDB[2] = BlockNum>>24;
//			cbw.CDB[3] = BlockNum>>16;
//			cbw.CDB[4] = BlockNum>>8;
//			cbw.CDB[5] = BlockNum;

			cbw.CDB[8] = BlockCnt;

			cbw.Flags = US_BULK_FLAG_IN;
			cbw.Lun = gUDisk.LuNum;
			cbw.Length = 10;
			
			Ret = UDiskBulkTransport(Buf, gHeadLen + gDataLen + gTailLen);
			
			if(Ret == TRUE)
			{
				break;
			}
		}
	}
	gHeadLen = 0;
	gDataLen = 0;
	gTailLen = 0;
	
#ifdef FUNC_OS_EN
		osMutexUnlock(UDiskMutex);
#endif
	return Ret;
}

/**
 * @brief  write blocks
 * @param  BlockNum block number
 * @param  Buf 写缓冲区指针
 * @param  BlockCnt 要写的block个数
 * @return 1-成功，0-失败
 */
bool UDiskWriteBlock(uint32_t BlockNum, void* Buf, uint8_t BlockCnt)
{
	uint8_t		Ret;
	
#ifdef FUNC_OS_EN
	osMutexLock(UDiskMutex);
#endif
	
	//DBG("UDiskWriteBlock:%08X\n",Buf);
//	if(!UDiskInitOK || !OTG_PortHostIsLink())
//	{
//		return FALSE;
//	}
//
//	if(gUDisk.BlockSize > 512)	//目前只支持扇区大小为512字节的USB设备
//	{
//		return FALSE;
//	}
//
//	if(BlockNum > gUDisk.LastLBA)
//	{
//		return FALSE;
//	}

	memset(cbw.CDB, 0x00, sizeof(cbw.CDB));
	cbw.CDB[0] = WRITE_10;
	((uint32_t*)(&cbw.CDB[2]))[0] = CpuToBe32(BlockNum);//BlockNum;
//	cbw.CDB[2] = BlockNum>>24;
//	cbw.CDB[3] = BlockNum>>16;
//	cbw.CDB[4] = BlockNum>>8;
//	cbw.CDB[5] = BlockNum;

	cbw.CDB[8] = BlockCnt;

	cbw.Flags = US_BULK_FLAG_OUT;
	cbw.Lun = gUDisk.LuNum;
	cbw.Length = 10;

	
	Ret = UDiskBulkTransport(Buf, BlockCnt * 512);
#ifdef FUNC_OS_EN
	osMutexUnlock(UDiskMutex);
#endif

	return Ret;
}

/**
 * @brief  get storage device block size
 * @param  NONE
 * @return block size
 */
uint16_t UDiskGetBlockSize(void)
{
	return gUDisk.BlockSize;
}

/**
 * @brief  get storage device last lba number
 * @param  NONE
 * @return last block number
 */
uint32_t UDiskGetLastLBA(void)
{
	return gUDisk.LastLBA;
}


bool UDiskInit()
{
	//查找USB MASS 设备
	uint32_t i;
	uint32_t MassStorDeviceCount=0;
	uint32_t MassStorDeviceInterface = 0;
	uint32_t EndPointFlag = 0;
	uint32_t EndPointCount = 0;
	bool InquiryOkFlag = FALSE;
	bool ReadFormatCapacityOkFlag = FALSE;
	TIMER		WaitStorReadyTimer1;
	TIMER		WaitStorReadyTimer2;

#ifdef FUNC_OS_EN
	if(UDiskMutex == NULL)
	{
		UDiskMutex = osMutexCreate();
	}
#endif

	//解析interface 描述符。
	for(i=0;i<OtgHostInfo.ConfigDesCriptor.bNumInterfaces;i++)
	{
		#define	InterfaceDescriptor		((PUSB_INTERFACE_DESCRIPTOR)OtgHostInfo.UsbInterface[i].pData)
		if((InterfaceDescriptor->bInterfaceClass != USB_CLASS_MASS_STORAGE)//类
		|| ((InterfaceDescriptor->bInterfaceSubClass != 5) && (InterfaceDescriptor->bInterfaceSubClass != 6))//SUB CLASS
		|| (InterfaceDescriptor->bInterfaceProtocol != 0x50))//只支持协议BBB, 不支持CBI,UAS
		{
			//DBG("MassProtocol = %08x\n",DeviceProtocol);
		}
		else
		{
			MassStorDeviceCount++;
			MassStorDeviceInterface = i;
		}
	}
	if(MassStorDeviceCount == 0)
	{
		OTG_DBG("NOT FOUNT MassStor Device Interface\n");
		return FALSE;
	}
	else if(MassStorDeviceCount > 1)
	{
		OTG_DBG("FOUNT %lu MassStor Device Interface\n",MassStorDeviceCount);
		return FALSE;
	}
	else if(MassStorDeviceCount == 1)
	{
		OTG_DBG("FOUNT %lu MassStor Device Interface\n",MassStorDeviceCount);
	}
	else
	{
		return FALSE;
	}
	//解析端点描述符,Bulk-Only协议中，至少有2个端点，一定有2个BULK端点
	#define	pBuf		(OtgHostInfo.UsbInterface[MassStorDeviceInterface].pData + 9)
	i = 0;
	while(1)
	{
		if((pBuf[i] == 0x07) && (pBuf[i+1] == 0x05) && (pBuf[i+3] == 0x02))//是一个BULK端点描述符
		{
			if(pBuf[i+2]&0x80)
			{
				//bulk in
				OTG_DBG("bulk in endpoint\n");
				EndPointFlag |= 0x01;
				EndPointCount++;
				gUDisk.BulkInEp.EpNum = pBuf[i+2];
				gUDisk.BulkInEp.MaxPacketSize = 64;//(*((uint16_t *)&pBuf[i+4])) > 64 ? 64:(*((uint16_t *)&pBuf[i+4]));
				//gUDisk.BulkInEp.MaxPacketSize = gUDisk.BulkInEp.MaxPacketSize & 0x03FF;
			}
			else
			{
				//bulk out
				OTG_DBG("bulk out endpoint\n");
				EndPointFlag |= 0x02;
				EndPointCount++;
				//save ep info
				gUDisk.BulkOutEp.EpNum = pBuf[i+2];
				gUDisk.BulkOutEp.MaxPacketSize = 64;//(*((uint16_t *)&pBuf[i+4])) > 64 ? 64:(*((uint16_t *)&pBuf[i+4]));
				//gUDisk.BulkOutEp.MaxPacketSize = gUDisk.BulkInEp.MaxPacketSize & 0x03FF;
			}
		}
		else
		{
			
		}
		i = i + pBuf[i];
		//DBG("%d\n",i);
		if(i == (OtgHostInfo.UsbInterface[MassStorDeviceInterface].Length - 9))
		{
			if((EndPointCount == 2) && (EndPointFlag == 3))
			{
				OTG_DBG("解析端点描述符OK\n");
				OTG_DBG("%02X %d\n",gUDisk.BulkInEp.EpNum,gUDisk.BulkInEp.MaxPacketSize);
				OTG_DBG("%02X %d\n",gUDisk.BulkOutEp.EpNum,gUDisk.BulkOutEp.MaxPacketSize);
				break;
			}
			return FALSE;
		}
		if(i > (OtgHostInfo.UsbInterface[MassStorDeviceInterface].Length - 9))
		{
			OTG_DBG("解析端点描述符错误\n");
			return FALSE;
		}
	}
//#ifdef FUNC_OS_EN
	//osTaskDelay(50);
//#else
	//WaitMs(50);
//#endif
	OTG_DBG("GetMaxlun\n");
	UDiskGetMaxLUN();
//#ifdef FUNC_OS_EN
	//osTaskDelay(50);
//#else
	//WaitMs(50);
//#endif
	OTG_DBG("UDiskInquiry\n");
	for(i = 0; i < (gUDisk.MaxLUN + 1) && OTG_PortHostIsLink(); ++i)
	{
		bool Ret = UDiskInquiry(i);
#ifdef FUNC_OS_EN
		osTaskDelay(5);
#else
		WaitMs(5);
#endif
		if(Ret && (UDiskBuf[0] == 0))
		{
			if(!InquiryOkFlag)
			{
				InquiryOkFlag = TRUE;
				gUDisk.LuNum = i;
			}

			//为了保证发给正常U盘的命令跟之前相同，此处只针对多LU的USB设备做处理
			if(gUDisk.MaxLUN > 0)
			{
				UDiskReadFormatCapacity(i);
				if(UDiskReadCapacity(i))
				{
					if((!ReadFormatCapacityOkFlag) && (Be32ToCpu(*(uint32_t*)&UDiskBuf[0]) > 4096))
					{
						//找到第一个正确的LU
						ReadFormatCapacityOkFlag = TRUE;
						gUDisk.LuNum = i;
					}
				}
			}
		}

		UDiskRequestSense(i);
	}
	if(!InquiryOkFlag)
	{
		OTG_DBG("InquiryOkFlag = FALSE\n");
		return FALSE;
	}
#ifdef FUNC_OS_EN
	osTaskDelay(10);
#else
	WaitMs(10);		//此处若不等待10ms，130号U盘无法识别。
#endif

	///////////////////////////////////////////////
	TimeOutSet(&WaitStorReadyTimer1, 10000);//PhoneDockingEnable ? 50000 : 10000);
	TimeOutSet(&WaitStorReadyTimer2, 2000);//PhoneDockingEnable ? 50000 : 2000);
	while(1)
	{
		if(!OTG_PortHostIsLink())	//如果设备被拔出，则退出循环
		{
			return FALSE;
		}

		if(UDiskTestUnitReady(gUDisk.LuNum))
		{
			break;
		}

		if(!UDiskRequestSense(gUDisk.LuNum))
		{
			if(IsTimeOut(&WaitStorReadyTimer2))
			{
				UDiskNotReadyErrFlag = TRUE;
				return FALSE;
			}
		}

		if(IsTimeOut(&WaitStorReadyTimer1))
		{
			return FALSE;
		}

		for(i = 0; i < 36; i++)
		{
#ifdef FUNC_OS_EN
			osTaskDelay(5);
#else
			WaitMs(5);
#endif
		}
	}

	////////////////////////////////////////////
	if(!UDiskReadCapacity(gUDisk.LuNum))
	{	
		//72号U盘第一次ReadCapacity失败，需要操作2次
		if(!UDiskReadCapacity(gUDisk.LuNum))
		{
			DBG("UDiskReadCapacity Err\n");
			return FALSE;
		}
	}
	UDiskInitOK = 1;
	gUDisk.BlockSize = Be16ToCpu(*(uint16_t*)(&UDiskBuf[6]));
	gUDisk.LastLBA = Be32ToCpu(*(uint32_t*)(&UDiskBuf[0]));
	return TRUE;
}

