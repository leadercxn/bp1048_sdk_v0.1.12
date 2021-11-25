#include <stdio.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "type.h"
#include "watchdog.h"
#include "irqn.h"
#include "remap.h"
#include "core_d1088.h"
#include "flash_boot.h"
#include "mode_switch_api.h"
#include "sys.h"
#include "app_config.h"
#include "rom.h"
#if FLASH_BOOT_EN


//******以下全为升级失败
//未检测到升级接口
#define NO_DEVICE_LINK			3
#define NO_UDISK_LINK			4
#define NO_SDCARD_LINK			5
#define NO_PC_LINK				6
#define NO_BT_LINK				7
//文件系统出错
#define FS_OPEN_MVA_ERR			8
#define FS_SDCARD_ERR			9
#define FS_UDISK_ERR			10
//升级数据合法性检查失败
#define MVA_HEADER_ERR			11
#define MVA_MAGIC_ERR			12
#define MVA_BOOT_LEN_ERR		13
#define MVA_ENCRYPTION_ERR		14
#define MVA_CODE_LEN_ERR		15
#define MVA_CONST_LEN_ERR		16
#define MVA_CONFIG_LEN_ERR		17
#define MVA_CONST_OFFSET_ERR	18
#define MVA_CONFIG_OFFEST_ERR	19
//升级过程中数据传输或数据写入失败
#define PROCESS_BOOT_ERR		21
#define PROCESS_CODE_ERR		22
#define PROCESS_CONST_ERR		23
#define PROCESS_CONFIG_ERR		24
#define FLASH_UNLOCK_ERR		25
#define BT_INFO_ERR				26

#define     ADR_FLASH_BOOT_FLAGE                                             (0x4000100C)
typedef struct _ST_FLASH_BOOT_FLAGE {
	volatile  unsigned long UDisk                      :  1; /**< enable U盘 mode  */
	volatile  unsigned long PC                         :  1; /**< enable PC升级 mode  */
	volatile  unsigned long sdcard                     :  2; /**< enable SD卡 mode  */
	volatile  unsigned long bt                         :  1; /**< bt  */
	volatile  unsigned long updata                     :  1; /**< 保留其他  */
	volatile  unsigned long flag                       :  2; /**< enable  */
	volatile  unsigned long RSV                        :  1; /**< 保留  */
	volatile  unsigned long ERROR_CODE                 :  8; /**< error code  */
	volatile  unsigned long POR_CODE                   :  8; /**< error code  */

} ST_FLASH_BOOT_FLAGE __ATTRIBUTE__(BITBAND);
#define SREG_FLASH_BOOT_FLAGE                    (*(volatile ST_FLASH_BOOT_FLAGE *) ADR_FLASH_BOOT_FLAGE)

void report_up_grate()
{
	uint16_t err_code=0,clear_data=0;
	err_code = ROM_UserRegisterGet();//V2.1.4版本及以上的flashboot，升级标志请以ROM_UserRegisterGet获取数据为准
	clear_data = ROM_UserRegisterGet();;
	err_code = (err_code&0x1f);//低5bit用于传递升级结果
	if(err_code!=0)
	{
		if(err_code == USER_CODE_RUN_START)
		{
			APP_DBG("正常启动\n");
		}
		else if(err_code == UPDAT_OK)
		{
			APP_DBG("升级OK\n");
		}
		else if(err_code == NEEDLESS_UPDAT)
		{
			APP_DBG("代码无需升级\n");
		}
		else
		{
			APP_DBG("升级失败 error code %d\n", err_code);
		}
		clear_data = (clear_data&0xffe0);
		ROM_UserRegisterSet(clear_data);
		//ROM_UserRegisterClear();
	}
	else
	{
		if(SREG_FLASH_BOOT_FLAGE.ERROR_CODE == USER_CODE_RUN_START)
		{
			APP_DBG("正常启动,你好\n");
		}
		else if(SREG_FLASH_BOOT_FLAGE.ERROR_CODE == UPDAT_OK)
		{
			APP_DBG("升级OK\n");
		}
		else if(SREG_FLASH_BOOT_FLAGE.ERROR_CODE == NEEDLESS_UPDAT)
		{
			APP_DBG("代码无需升级\n");
		}
		else
		{
			APP_DBG("升级失败 error code %lu\n", (uint32_t)SREG_FLASH_BOOT_FLAGE.ERROR_CODE);
		}
	}
}

uint8_t Report_Error_Code(void)
{
	return SREG_FLASH_BOOT_FLAGE.ERROR_CODE;
}

void Clear_Error_Code(void)
{
	SREG_FLASH_BOOT_FLAGE.ERROR_CODE = USER_CODE_RUN_START;
}

uint8_t Reset_FlagGet_Flash_Boot(void)
{
	return (uint8_t)SREG_FLASH_BOOT_FLAGE.POR_CODE;
}

extern void DataCacheInvalidAll(void);//core_d1088.c
extern void ICacheInvalidAll(void);//core_d1088.c
void start_up_grate(uint32_t UpdateResource)
{
	int i;
	uint32_t temp = 0;
	typedef void (*fun)();
	fun jump_fun;

	*(uint32_t *)ADR_FLASH_BOOT_FLAGE = 0;
	if(ResourceValue(AppResourceCard) && (UpdateResource == AppResourceCard))
	{
		//挂载检测指定的mva包存在，
		SREG_FLASH_BOOT_FLAGE.updata = 1;//升级使能位
		#if CFG_RES_CARD_GPIO == SDIO_A15_A16_A17
		SREG_FLASH_BOOT_FLAGE.sdcard = 1;
		#else
		SREG_FLASH_BOOT_FLAGE.sdcard = 2;
		#endif
	}
	else if(ResourceValue(AppResourceUDisk) && (UpdateResource == AppResourceUDisk))
	{
		//挂载检测指定的mva包存在，
		SREG_FLASH_BOOT_FLAGE.updata = 1;//升级使能位
		SREG_FLASH_BOOT_FLAGE.UDisk = 1;
	}
	else if(ResourceValue(AppResourceUsbDevice) && (UpdateResource == AppResourceUsbDevice))
	{
		//检测PC升级的连接有效性。
		SREG_FLASH_BOOT_FLAGE.PC = 1;//pc升级
		SREG_FLASH_BOOT_FLAGE.updata = 1;//升级使能位
	}

	if(SREG_FLASH_BOOT_FLAGE.updata)
	{
		temp = *(uint32_t *)ADR_FLASH_BOOT_FLAGE;
		APP_DBG("start_up_grate0...................");
		jump_fun = (fun)0;
		WDG_Enable(WDG_STEP_1S);
		GIE_DISABLE();
		DisableIDCache();	//关闭IDcache
		DataCacheInvalidAll();//清除Dcache
		ICacheInvalidAll();//清除Icache
		SysTickDeInit();
		SysTimerIntFlagClear();
		
		//DMA
		*(uint32_t *)0x4000D100 = 0;
		for(i=0x4000D000;i<0x4000D104;)
		{
			*(uint32_t *)i = 0;
			i=i+4;
		}

		*(uint32_t *)0x40022000 &= ~0x77FFF8;//REG复位 flash aupll not reset
		*(uint32_t *)0x40022000 |= 0x7FFFF8;

		*(uint32_t *)0x40022004 &= ~0x7FFFF7FF;//fun reset  flash not reset
		*(uint32_t *)0x40022004 |= 0x7FFFF7FF;


		*(uint32_t *)ADR_FLASH_BOOT_FLAGE = temp;
		__nds32__mtsr(0, NDS32_SR_INT_MASK2);//中断使能位清零
		__nds32__mtsr(__nds32__mfsr(NDS32_SR_HSP_CTL) & 0, NDS32_SR_HSP_CTL);
		__asm("NOP");
		GPIO_RegisterResetMask();
		jump_fun();
		while(1);
	}
}

#endif
