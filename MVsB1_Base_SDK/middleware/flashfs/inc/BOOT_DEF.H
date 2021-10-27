#include "eosdef.h"

#ifndef _BOOT_DEF_H
#define _BOOT_DEF_H

		
	typedef	struct {
	char	file_name[32];
	unsigned	long file_size;
	}F_INF;	
	typedef	struct {
	F_INF	file_info[16]; 
	int	File_Count; 
	unsigned	long Total_Size; 
	}F_NAME,	*PF_NAME;
	
	
	typedef	struct {
	unsigned	long RewriteCounter;
	}PAGE_INFO,	*PPAGE_INFO;
	
/*	$<	
	**************************************************************************	
	*	BASIC IO SYSTEM
	*	<bios.c>
	**************************************************************************	
	$>	*/
	int	BiosInitialize(VOID) ;
	int	BiosEntry(int R4, int R5, int R6, int R7);
/*	$<	
	**************************************************************************	
	*	FLASH FILE SYSTEM
	*	<flash.c>
	**************************************************************************	
	$>	*/
	int		BiosEntry(int R0, int R1, int R2, int R3);
	int	b_file_initialize(void) ;
	int	b_file_create(const char* name, int mode);
	int	b_file_open(const char* name, int code);
	int	b_file_close(int handle);
	int	b_file_read(int handle, char* buf, unsigned int count);
	int	b_file_write(int handle, const char* buf, unsigned int count);
	int	b_file_remove(const char* path);
	int	b_file_rename(const char* old, const char* new);
	int	b_file_lseek(int handle, long offset, int base);
	int	b_file_getattr(const char* name);
	int	b_file_setattr(const char* name, unsigned char attr);
	int	b_file_getdiskfree(void) ;
	int	b_file_getfilename( F_NAME* pfile);
	int	b_file_getSecterStatus( PAGE_INFO* psect );
	int	b_getProgramVersion( char* ver );

/*	$h		*/
	#ifdef	_DEBUG
	#define	CLUSTERINFO(w) w ? printf("Cluster%04X[%08lX]->wClusterMark[%04X]\n", w, GetClusterAddress(w), GetClusterAddress(w)->wClusterMark) : printf("Cluster0000!!VOID\n")
	#define	FATINFO(p) Dir(p)
	#define	CHAININFO() FatStatus()
	#else	
	#define	CLUSTERINFO(w)
	#define	FATINFO(p)
	#define	CHAININFO()
	#endif	
/*	$each		*/
/*	$<	
	**************************************************************************	
	*	FLASH ROM CONTROL
	*	<flashcmd.asm>
	**************************************************************************	
	$>	*/
	int	FlashReadReset(PVOID pChipBase);
	int	FlashProgrammData(PVOID pChipBase, PVOID pDestination, PCVOID pSource, unsigned int nCount);
	int	FlashChipErase(PVOID pChipBase);
	int	FlashSectorErase(PVOID pChipBase, PVOID pErasePage);


#endif
