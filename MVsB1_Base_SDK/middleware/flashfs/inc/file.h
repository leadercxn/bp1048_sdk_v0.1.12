
/**
* @addtogroup Flash-FS
* @{
* @defgroup file file.h
* @{
*/

#ifndef    _FILE_H_
#define    _FILE_H_
#include <stdio.h>
#include "EOSDEF.h"
#ifdef CFG_APP_CONFIG
#include "flash_config.h"
#endif

/************************************************************************/
#define PAGE_SIZE          SIZE_64K   //注意64k对齐
#ifdef CFG_APP_CONFIG
//以下两个参数可由用户修改
#define FLASH_BASE        (((FLASHFS_ADDR - 1) / SIZE_64K+ 1) * SIZE_64K) //~0x1D0000 the first has been defined as code area
#define FLASH_FS_SIZE      ((BP_DATA_ADDR / SIZE_64K) * SIZE_64K - FLASH_BASE)  //注意64k对齐、系统flash空间分配。
#else
#define FLASH_BASE        	0x1A0000 //~0x1D0000 the first has been defined as code area
#define FLASH_FS_SIZE      	5 * SIZE_64K //注意64k对齐、系统flash空间分配。
#endif

/************************************************************************/
#define	DATA_PER_CLUSTER_2K	    (SIZE_2K - 4)
#define	DATA_PER_CLUSTER_4K  	(SIZE_4K - 4)
#define	DATA_PER_CLUSTER_8K	    (SIZE_8K - 4)
#define	DATA_PER_CLUSTER_16K	(SIZE_16K - 4)
#define	DATA_PER_CLUSTER_32K	(SIZE_32K - 4)

/************************************************************************/
/************************************************************************/


#define	  NUM_OF_PAGE		 (FLASH_FS_SIZE  /PAGE_SIZE)

#if (NUM_OF_PAGE > 32)
#error "error:NUM_OF_PAGE"//NUM_OF_PAGE不大于32
#endif

#if  (FLASH_FS_SIZE <= SIZE_4M) //小于4M时蔟值设置为2K,因为容量较大时fat表可能会超过CLUSTER_SIZE的大小
#define	  NUM_OF_CLUSTER		((FLASH_FS_SIZE ) / SIZE_2K)
#define   CLUSTER_SIZE          (DATA_PER_CLUSTER_2K)
#endif
#if  ((FLASH_FS_SIZE <= SIZE_8M) && (FLASH_FS_SIZE > SIZE_4M))//4~8M时蔟值设置为4K
#define	  NUM_OF_CLUSTER		((FLASH_FS_SIZE ) / SIZE_4K)
#define   CLUSTER_SIZE          (DATA_PER_CLUSTER_4K)
#endif
#if  ((FLASH_FS_SIZE <= SIZE_16M) && (FLASH_FS_SIZE > SIZE_8M))//8~16M时蔟值设置为8K
#define	  NUM_OF_CLUSTER		((FLASH_FS_SIZE ) / SIZE_8K)
#define   CLUSTER_SIZE          (DATA_PER_CLUSTER_8K)
#endif



#define PAGE_OFFSET  16

#define	MAX_NAME			8
#define	MAX_EXT				3

typedef	struct	tagDIRECTORY	{
	TCHAR		szName[MAX_NAME];
	TCHAR		szExt[MAX_EXT];
	BYTE		bAttrib;
	BYTE		bReserve[10];
	WORD		wTime;
	WORD		wDate;
	WORD		wTopCluster;
	DWORD		dwSize;
}DIRECTORY, *PDIRECTORY;

/************************************************************************/
/************************************************************************/
#ifndef _NFILE
#define _NFILE 8//7//
/*#define _NFILE _nfiles*/
#endif

#define	NUM_OF_FILE			_NFILE


#define	FAT_FREE			0xfff
#define	FAT_FAT				0xffe
#define	FAT_EOF				0x800
#define	FAT_VOID			0


typedef	struct	tagFAT	{
	DIRECTORY	dir[ NUM_OF_FILE ];
	BYTE		bFAT[ NUM_OF_CLUSTER * 3 / 2 ];
}FAT, *PFAT;
/************************************************************************/

#define	CM_FREE				0xffff
#define	CM_BUSY				0x7fff
#define	CM_FAT				0x04
#define	CM_FAT_ID			0x03
#define	CM_DATA				1
#define	CM_VOID				0

typedef	struct	tagCLUSTER	{
	WORD	wClusterMark;
	WORD	wReserve;
	union	{
		BYTE	byte[CLUSTER_SIZE];
		FAT		fat;
	}data;
}CLUSTER, *PCLUSTER;

/************************************************************************/
/************************************************************************/
typedef	struct	tagFCB	{
	BYTE			nBusyFlag;
	TCHAR		szName[MAX_NAME];
	TCHAR		szExt[MAX_EXT];
	BYTE		bAttrib;
	BYTE		nIndexOfDir;
	DWORD		dwSize;
	DWORD		dwOffset;
}FCB, *PFCB;

#define		fcb_busy_free		0
#define		fcb_busy_read		1
#define		fcb_busy_write		2

/************************************************************************/


extern DWORD  sizeof_fat;
extern DWORD  sizeof_cluster;
extern DWORD  flash_base;//FLASH fs的起始地址
extern DWORD  flashFS_size ;
extern DWORD  page_size;
extern DWORD  cluster_size; //CLUSTER_SIZE
extern DWORD  num_of_page;
extern DWORD  num_of_cluster;
//extern  BYTE  bClusterChain[ NUM_OF_CLUSTER ];
/************************************************************************/









struct __FFILE {
	unsigned char *_bufptr;          /* buffer pointer              */
  long  _bufcnt;          /* buffer count ,for read       */
  unsigned char *_bufbase;         /* buffer base pointer         */
  long  _buflen;          /* buffer length               */
  char  _ioflag1;         /* I/O control flag 1          */
  char  _ioflag2;         /* I/O control flag 2          */
  char  _iofd;
};

typedef struct __FFILE   FFILE;




//extern struct __FFILE {
//       unsigned char *_bufptr;          /* buffer pointer              */
//                long  _bufcnt;          /* buffer count ,for read       */
//       unsigned char *_bufbase;         /* buffer base pointer         */
//                long  _buflen;          /* buffer length               */
//                char  _ioflag1;         /* I/O control flag 1          */
//                char  _ioflag2;         /* I/O control flag 2          */
//                char  _iofd;
//}
extern FFILE _iob[_NFILE];

//typedef struct _iobuf FFILE;

#define _IOREAD    1                    /* only read or update read    */
#define _IOWRITE   2                    /* only write or update write  */
#define _IORW      4                    /* update file                 */
#define _IOUNBUF   8                    /* unbuffering I/O              */
#define _IOBGBUF  16                    /* automatic buffer allocate   */
#define _IOEOF    32                    /* end of file                 */
#define _IOERR    64                    /* I/O error                   */
#define _UNGTC     1                    /* ungetc issued (_ioflag2)    */
#define _IOLINBUF  2                    /* line buffering              */
#define _IOBIN     0x04                 /* binary stream */

#define	O_RDONLY	0x01
#define	O_WRONLY	0x02
#define	O_RDWR		0x04
#define	O_CREAT		0x08
#define	O_TRUNC		0x10
#define	O_APPEND	0x20

#define	FILE_CREATE	1
#define	FILE_OPEN	2
#define	FILE_CLOSE	3
#define	FILE_READ	4
#define	FILE_WRITE	5
#define	FILE_REMOVE	6
#define	FILE_RENAME	7
#define	FILE_LSEEK	8
#define	FILE_GETATTR	9
#define	FILE_SETATTR	10
#define	FILE_NAMESET	11
#define	FILE_PAGESET	12
#define	GET_PROG_VER	13
#define	SCI_READ	20
#define	SCI_WRITE	21
#define	__A_RDONLY	0x01
#define	__A_HIDDEN	0x02
#define	__A_SYSTEM	0x04
#define	__A_VOLID	0x08
#define	__A_SUBDIR	0x10
#define	__A_ARCH	0x20
#define	CODE_READ_ONLY	0x0
#define	CODE_WRITE_ONLY	0x1
#define	CODE_READ_WRITE	0x2

#define SEEK_SET 0 /* start of stream (see fseek) */
#define SEEK_CUR 1 /* current position in stream (see fseek) */
#define SEEK_END 2 /* end of stream (see fseek) */
#undef	BUFSIZ	//for warning by pi
#define BUFSIZ       (512)
  
#ifdef _cplusplus
extern "C"{
#endif	
 FFILE* Fopen(const char* name, const char* mode);

 int Open(const char* name, int mode);

 int Write( int handle, const void* buf, size_t len);

 int Close( int handle );

 int Fclose(FFILE* fp);

 int Read(int handle, char* buffer, size_t size);

 int Fread(void* buffer ,size_t size, size_t block, FFILE* fp);

 int Fwrite(void* buffer ,size_t size, size_t block, FFILE* fp);

 int Rename( char* oldname, char* newname );

 int Remove( const char* filename );

 int Fseek(FFILE *stream, long offset, int fromwhere);

 int Fflush( FFILE *stream );

 int FlashFSInit(void);

 void FlashParaInit(DWORD flashAddr,DWORD flashSize,DWORD pageSize,DWORD clusterSize);
 int CleanupCluster(BOOL bBusyCluster);
 int CleanupCluster_sub(BOOL bBusyCluster,BYTE *pClusterChain);
 void EraseVoidPage(void);//

#define Feof(fp)    ( (fp)->_ioflag1 & _IOEOF )
#define Ferror(fp)  ( (fp)->_ioflag1 & _IOERR )
#define Clearerr(fp) ((void)((fp)->_ioflag1&=~(_IOEOF|_IOERR)))

const unsigned char *GetLibVersionFlashFS(void);

#ifdef _cpluscplus
}
#endif


#endif
