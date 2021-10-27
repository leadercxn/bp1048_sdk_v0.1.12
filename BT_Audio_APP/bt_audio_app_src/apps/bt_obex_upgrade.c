///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: bt_obex_upgrade.c
//  maintainer: KK
///////////////////////////////////////////////////////////////////////////////
#include "type.h"
#include "debug.h"
#include "rom.h"
#include "bt_config.h"
#include "spi_flash.h"
#include "flash_config.h"
#include "bt_obex_api.h"
#include "mcu_circular_buf.h"
#include "watchdog.h"
#include "portmacro.h"

#ifdef MVA_BT_OBEX_UPDATE_FUNC_SUPPORT


#define FLASH_MVA_UPDATE_START_OFFSET 0x100000////0x100000  /*MUST be based on the real condition!!!*/

#define OBEX_STATUS_NONE        0x00
#define OBEX_STATUS_CONNECTED   0x01
#define OBEX_STATUS_MVA_FOUND   0x02

static unsigned char ObexStautsFlag = OBEX_STATUS_NONE;
static uint32_t MvaFileTotalSize = 0;
static uint32_t FileTotalLen = 0;
uint8_t *opp_update_fifo=NULL;//[63*1024];
#define OPP_UPDATE_FITO_SIZE 63*1024
MCU_CIRCULAR_CONTEXT opp_update_handle;
uint8_t opp_update_buf[4096];
uint32_t erase_addr = 0;
uint32_t write_addr = 0;
uint32_t block_erase_num = 0;
//uint32_t sec_erase_num = 0;
uint32_t volatile obex_start = 0;
uint32_t obex_cont = 0;
//uint8_t obex_fifo_temp[4096];
uint8_t sAllocMemFirst=1;
//CRC algorithm
 

static const uint16_t CrcCCITTTable[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

//init CRC with 0x0000
static uint16_t CRC16(uint8_t* Buf,uint32_t BufLen,uint16_t CRC)
{
	register uint32_t i;

	for (i = 0 ; i < BufLen ; i++) {
		CRC = (CRC << 8) ^ CrcCCITTTable[((CRC >> 8) ^ *(uint8_t*)Buf ++) & 0x00FF];
	}
	return CRC;
}

static void DualBankUpdateReboot(void)
{
    ROM_BankBUpgradeApply(1, FLASH_MVA_UPDATE_START_OFFSET); //write addr for update usage!!!
    //reboot
    ROM_SysReset();
}

static uint8_t MvaCrcCheck(void)
{
    //Total Size
    uint32_t Addr = FLASH_MVA_UPDATE_START_OFFSET;
    uint32_t i;
    uint16_t Crc16 = 0,T;
    uint8_t Tmp[4];
    
    if(ObexStautsFlag != OBEX_STATUS_MVA_FOUND)
    {
        APP_DBG("NO MVA file is found.\n");
        return 0; 
    }
    
    for(i = 0 ; i < MvaFileTotalSize - 4 ; i ++)
    {
        SpiFlashRead(Addr, Tmp, 1, 0);
        Crc16 = CRC16(Tmp,1,Crc16);
        Addr ++;
    }
    
    SpiFlashRead(Addr,   Tmp,   1, 0);
    SpiFlashRead(Addr+1, Tmp+1, 1, 0);
    SpiFlashRead(Addr+2, Tmp+2, 1, 0);
    SpiFlashRead(Addr+3, Tmp+3, 1, 0);
    
    if(!(Tmp[2] | Tmp[3]))
    {
        T = Tmp[1];
        T <<= 8;
        T |= Tmp[0];
        
        if(T != Crc16)
        {
            APP_DBG("MVA file CRC failed, please check...\n");
        }
        else
        {
            APP_DBG("MVA file CRC OK. Now one can reboot to update....\n");
            return 1;//OK to update....
        }
    }
    APP_DBG("MVA file CRC failed...\n");
    return 0; //error    
}

void CheckMvaFileName(uint8_t* Data ,uint16_t Len)
{
    int i;
    
    for(i = 0 ; i < Len ; i ++)
    {
        APP_DBG("%c",Data[i]);
    }
    APP_DBG("\n");
    
    if(Len > 10)
    {
        if((Data[Len-3] == 'A') || (Data[Len-3] == 'a'))
        {
            if((Data[Len-5] == 'V') || (Data[Len-5] == 'v'))
            {
                if((Data[Len-7] == 'M') || (Data[Len-7] == 'm'))
                {
                    if(Data[Len-9] == '.')//found .mva files
                    {
                        ObexStautsFlag = OBEX_STATUS_MVA_FOUND;  
                        APP_DBG("Receiving MVA file...\n");
                    }
                }
            }
        }
    }
}

void ObexUpdateProc(const BT_OBEX_CALLBACK_PARAMS *Info)
{    
    switch(Info->type)
    {
        case BT_OBEX_NAME:        
            CheckMvaFileName(Info->segdata, Info->seglen);
            //init 
            MvaFileTotalSize = 0;
            FileTotalLen = 0;
            //MCUCircular_Config(&opp_update_handle, opp_update_fifo, sizeof(opp_update_fifo));
            obex_start = 0;
        	break;

		case BT_OBEX_LENGTH:
			{
				uint32_t i;
				APP_DBG("type:%d\n", Info->type);
				APP_DBG("total len:%d\n", Info->total);
				APP_DBG("seg off:%d\n", Info->segoff);
				APP_DBG("seg len:%d\n", Info->seglen);
				APP_DBG("seg data:");
				for(i=0;i<Info->seglen;i++)
				{
					APP_DBG("%02x ", Info->segdata[i]);
				}
				APP_DBG("\n ====\n");
				FileTotalLen = 0;
				FileTotalLen = (Info->segdata[0]<<24) +  (Info->segdata[1]<<16) + (Info->segdata[2]<<8) + Info->segdata[3];
				block_erase_num = FileTotalLen/65536;//64K
				if(FileTotalLen%65536)
					block_erase_num++;
				/*sec_erase_num = ( FileTotalLen%(64*1024))/4096;
				APP_DBG("sec_erase_num:%u\n",sec_erase_num);
				if(FileTotalLen%4096)
				{
					sec_erase_num++;
				}
				APP_DBG("sec_erase_num:%u\n",sec_erase_num);
				*/
	            erase_addr = FLASH_MVA_UPDATE_START_OFFSET;
	            write_addr = FLASH_MVA_UPDATE_START_OFFSET;

				erase_addr += 65536;
				block_erase_num--;
	            obex_start = 1;
			
				AudioCoreServicePause();
				APP_DBG("FileTotalLens:%u\n", FileTotalLen);
				APP_DBG("block_erase_num:%u\n", block_erase_num);
				
			}
			break;
        
        case BT_OBEX_BODY:
        case BT_OBEX_BODY_END:
			if(Info->type == BT_OBEX_BODY_END)
				APP_DBG("Info->type = %d",Info->type);

			if(sAllocMemFirst)
			{
				opp_update_fifo=osPortMallocFromEnd(OPP_UPDATE_FITO_SIZE);
				if(opp_update_fifo!= NULL)
					MCUCircular_Config(&opp_update_handle, opp_update_fifo,OPP_UPDATE_FITO_SIZE);
				else
				{
				APP_DBG("Alloc Mem Error\n");
				}
			}

			sAllocMemFirst=0;

			
			if(MCUCircular_GetSpaceLen(&opp_update_handle)>= Info->seglen)
			{
				MCUCircular_PutData(&opp_update_handle,Info->segdata,Info->seglen);
			}
			else
			{
				APP_DBG("opp fifo error=%d\n",MCUCircular_GetSpaceLen(&opp_update_handle));
			}
            MvaFileTotalSize += Info->seglen;
            //APP_DBG("+%d=%d\n", Info->seglen, MvaFileTotalSize);
            break;
        
        default:
            break;
    }
}

void ObexUpgradeStart(void)
{
	ObexStautsFlag = OBEX_STATUS_CONNECTED;
}

void ObexUpgradeEnd(void)
{
	obex_start = 0;
	if(ObexStautsFlag == OBEX_STATUS_MVA_FOUND)//end of mva data transfering, then I should do the CRC and report the result if error found.
	{
	    APP_DBG("start check MVA file...\n");
	    if(MvaCrcCheck())
	    {
	        APP_DBG("legal MVA file found..... \n");
	        
	        //add reboot related code, one MUST double check the boot version(V4.2.2 and later) consistence. 
	        DualBankUpdateReboot();
	    }
	}
	ObexStautsFlag = OBEX_STATUS_NONE;
}



void bt_obex_upgrate(void)
{

	sAllocMemFirst=1;

	while(1)
	{
		int temp = 0;
		start:
		if(obex_start == 0)
		{
			vTaskDelay(1);
			goto start;
		}
		if(obex_start == 1)
		{

			SpiFlashErase(BLOCK_ERASE, FLASH_MVA_UPDATE_START_OFFSET/65536, 1);
			vTaskDelay(1);
			obex_start = 2;
			WDG_Enable(WDG_STEP_4S);
			goto start;
		}
		WDG_Feed();
		obex_cont++;
		//if(obex_cont)
		{
		
			if(MCUCircular_GetDataLen(&opp_update_handle)>= 4096)
			{

			
				if(erase_addr >= write_addr)
				{
					MCUCircular_GetData(&opp_update_handle,opp_update_buf,4096);
					temp = 1;
				}
			}
			if(temp)
			{
				APP_DBG("w:%06X\n",write_addr);
				SpiFlashWrite(write_addr, opp_update_buf, 4096, 1);
				/*SpiFlashRead(write_addr,  obex_fifo_temp, 4096, 1);
				if(memcmp(opp_update_buf,obex_fifo_temp,4096) != 0)
				{
					SpiFlashWrite(write_addr, opp_update_buf, 4096, 1);
					SpiFlashRead(write_addr,obex_fifo_temp, 4096, 1);
					if(memcmp(opp_update_buf,obex_fifo_temp,4096) != 0)
					{
						//printf("error 1\n");
						SpiFlashWrite(write_addr, opp_update_buf, 4096, 1);
						SpiFlashRead(write_addr,obex_fifo_temp, 4096, 1);
						if(memcmp(opp_update_buf,obex_fifo_temp,4096) != 0)
						{
							printf("error 1\n");
							//goto start1;
						}
					}
				}
				*/
				write_addr +=4096;
			}
			if( (FileTotalLen - (write_addr-FLASH_MVA_UPDATE_START_OFFSET)) == (FileTotalLen%4096))
			{
				int len = MCUCircular_GetDataLen(&opp_update_handle);
				if(len == (FileTotalLen%4096))
				{
					MCUCircular_GetData(&opp_update_handle,opp_update_buf,len);
					APP_DBG("qw:%06X %u\n",write_addr,len);
					SpiFlashWrite(write_addr, opp_update_buf, len, 1);
					/*SpiFlashRead(write_addr,obex_fifo_temp, len, 1);
					if(memcmp(opp_update_buf,obex_fifo_temp,len) != 0)
					{
						SpiFlashWrite(write_addr, opp_update_buf, len, 1);
						SpiFlashRead(write_addr,obex_fifo_temp, len, 1);
						if(memcmp(opp_update_buf,obex_fifo_temp,len) != 0)
						{
							//printf("error\n");
							SpiFlashWrite(write_addr, opp_update_buf, len, 1);
							SpiFlashRead(write_addr,obex_fifo_temp, len, 1);
							if(memcmp(opp_update_buf,obex_fifo_temp,len) != 0)
							{
								printf("error\n");
								//goto start;
							}
						}
					}
					*/
					write_addr +=len;
					APP_DBG("qw:%06X end\n",write_addr);
				}
			}
		}
		if(obex_cont > 15)
		{
			//²Á³ýFLASH
			if(block_erase_num)
			{
				APP_DBG("e:%06X\n",erase_addr);
				SpiFlashErase(BLOCK_ERASE, erase_addr/65536, 1);
				erase_addr += 65536;
				block_erase_num--;
			}

			/*else
				{
				if(sec_erase_num)
				{
					APP_DBG("e:%06X\n",erase_addr);
					SpiFlashErase(SECTOR_ERASE, erase_addr/4096, 1);
					erase_addr += 4096;
					sec_erase_num--;
				}
			}
			*/
			obex_cont = 0;
		}
		vTaskDelay(1);
	}
}



#endif


