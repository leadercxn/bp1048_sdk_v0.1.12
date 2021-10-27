#include "string.h"
#include "file.h"
#include "BOOT_DEF.H"

/****************Lib中使用,静态内存，与宏相关****************/
FAT			ramFAT;

int FlashFSInit(void)
{
	/*********************************************************************
	 * 注意：用户仅可修改FLASH_BASE、FLASH_FS_SIZE、PAGE_SIZE、CLUSTER_SIZE 四个宏，
	 * 由于宏的依赖关系，不要直接在FlashParaInit()函数入口上填写参数。
	 *********************************************************************/
	FlashParaInit(FLASH_BASE,FLASH_FS_SIZE,PAGE_SIZE,CLUSTER_SIZE);//参数设置

	if(b_file_initialize() != 0)//flash FS初始化
	{
		return -1;
	}
	memset(&_iob, 0, sizeof(_iob));

	return 0;
}

int	CleanupCluster(BOOL bBusyCluster)
{
    BYTE		bClusterChain[ NUM_OF_CLUSTER ];

	return(CleanupCluster_sub(bBusyCluster,bClusterChain));
}
