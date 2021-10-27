#ifndef __FLASH_CONFIG_H__
#define __FLASH_CONFIG_H__
#include "app_config.h"

//用于定义Flash 区域使用情况(v0.4)
//默认flash为2M Byte，如果大小有变化注意调整边界
#define	CODE_ADDR				0x0
#define CONST_DATA_ADDR    		0x198000	//开启蓝牙时code边界

#ifdef CFG_CHIP_BP1048P4
#define AUDIO_EFFECT_ADDR  		0x1C8000 + 0x200000	//根据实际需求定义(暂时未用)
#define FLASHFS_ADDR			0x1D0000 + 0x200000	//根据实际需求定义(暂时未用)

//常读写区域
#define USER_DATA_ADDR       0x1F0000 + 0x200000 //size:12KB(0x1f0000~0x1f2fff)
#define BP_DATA_ADDR         (0x1F3000 + 0x200000) //size:32KB(0x1f3000~0x1fafff)
#define BT_DATA_ADDR         (0x1FB000 + 0x200000) //size:12KB(0x1fb000~0x1fdfff)

//参量化区域
#define USER_CONFIG_ADDR     (0x1FE000 + 0x200000) //size:4KB(0x1fe000~0x1fefff)
#define BT_CONFIG_ADDR       (0x1FF000 + 0x200000) //size:4KB(0x1ff000~0x1fffff)
#else
#define AUDIO_EFFECT_ADDR  		0x1C8000	//根据实际需求定义(暂时未用)
#define FLASHFS_ADDR			0x1D0000	//根据实际需求定义(暂时未用)

//常读写区域
#define USER_DATA_ADDR     		0x1F0000	//size:12KB(0x1f0000~0x1f2fff)
#define BP_DATA_ADDR     		0x1F3000	//size:32KB(0x1f3000~0x1fafff)
#define BT_DATA_ADDR     		0x1FB000	//size:12KB(0x1fb000~0x1fdfff)

//参量化区域
#define USER_CONFIG_ADDR		0x1FE000	//size:4KB(0x1fe000~0x1fefff)
#define BT_CONFIG_ADDR			0x1FF000	//size:4KB(0x1ff000~0x1fffff)
#endif

/**音频SDK版本号：V1.0.0**/
/*0xB1：芯片B1X，01：大版本号， 00：小版本号， 00：用户修订号（由用户设定，可结合补丁号）；实际存储字节序：1A 01 00 00 ，略区别于sdk版本*/
/*开启flash_boot时，用flashboot升级usercode后，boot明码区和code明码(如0xB8和0xB8+0x10000)两个值会不同，前者是burner烧录时版本，后者是mva版本需关注*/
#define	 CFG_SDK_VER_CHIPID			(0xB1)
#define  CFG_SDK_MAJOR_VERSION		(0)
#define  CFG_SDK_MINOR_VERSION		(1)
#define  CFG_SDK_PATCH_VERSION	    (12)

#endif

