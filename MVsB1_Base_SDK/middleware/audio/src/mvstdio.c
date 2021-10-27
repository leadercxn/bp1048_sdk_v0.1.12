/**
 *************************************************************************************
 * @file     mvstdio.c
 * @brief    File operation api implementation for FatFS or MV File System
 *
 * @author   Aissen Li, ZHAO Ying
 * @version  V4.2.0
 *
 * &copy; Shanghai Mountain View Silicon Tenchnology Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#include "mvstdio.h"

/* Set the standard file system as default file system */
#if !defined(USE_MV_FS) && !defined(USE_FATFS_FS) && !defined(USE_STD_FS)
#define USE_STD_FS
#endif

void* mv_memcpy(void* dst, const void* src, uint32_t num)
{
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;

    #if 1 // WORD ACCESS
    if(!((uint32_t)dst&3) && !((uint32_t)src&3)) // Check word align
    {
        while(num >= sizeof(int32_t))
        {
            *(int32_t*)d = *(int32_t*)s;
            d += sizeof(int32_t); s += sizeof(int32_t);
            num -= sizeof(int32_t);
        }
    }
    #endif

    while(num--)
    {
        *d++ = *s++;
    }

    return dst;
}

int32_t mv_memcmp(const void* ptr1, const void* ptr2, uint32_t num)
{
    int32_t r = 0;

    const uint8_t *m1 = (const uint8_t *)ptr1;
    const uint8_t *m2 = (const uint8_t *)ptr2;
    
    while(num-- && (r = *m1++ - *m2++) == 0);

    return r;
}

void* mv_memmove(void* dst, const void* src, uint32_t num)
{
    void *ret = dst;

    if(dst <= src || (int8_t*)dst >= ((int8_t *)src + num))
    {
        // Non-Overlapping Buffers
        // copy from lower addresses to higher addresses
        while(num--)
        {
            *(int8_t*)dst = *(int8_t*)src;
            dst = (int8_t *)dst + 1;
            src = (int8_t *)src + 1;
        }
    }
    else
    {
        // Overlapping Buffers
        // copy from higher addresses to ower addresses
        dst = (int8_t *)dst + num - 1;
        src = (int8_t *)src + num - 1;

        while(num--)
        {
            *(int8_t*)dst = *(int8_t*)src;
            dst = (int8_t*)dst - 1;
            src = (int8_t*)src - 1;
        }
    }

    return(ret);
}

void* mv_memset(void* ptr, int32_t value, uint32_t num)
{
    uint8_t *p = (uint8_t*)ptr;

    while (num--)
    {
        *p++ = (uint8_t)value;
    }

    return ptr;
}

//static mv_mread_callback_t mv_mread_callback = 0;

typedef struct
{
	uint8_t* address;				/**< memory base address      */
	uint32_t capacity;				/**< memory capacity in bytes */	
	uint16_t wp;					/**< write point in bytes     */
	uint16_t rp;					/**< read point in bytes      */
	mv_mread_callback_t callback;	/**< callback function address  */
}CircleBufferContext;

void mv_mopen(MemHandle* mp, uint8_t* addr, uint32_t capacity, mv_mread_callback_t callback)
{
    //mp->addr              = addr;
    //mp->mem_capacity      = capacity;
    //mp->mem_len           = 0;
    //mp->p                 = 0;
    //mv_mread_callback     = callback;

	CircleBufferContext* cbc = (CircleBufferContext*)mp;
	cbc->address = addr;
	cbc->capacity = capacity;
	cbc->wp = 0;
	cbc->rp = 0;
	cbc->callback = callback;	
}

void mv_mclose(MemHandle* mp)
{
	CircleBufferContext* cbc = (CircleBufferContext*)mp;
	cbc->address = 0;
	cbc->capacity = 0;
	cbc->wp = 0;
	cbc->rp = 0;
	cbc->callback = 0;
}

void mv_mread_callback_set(void *mp, mv_mread_callback_t callback)
{
	CircleBufferContext* cbc = (CircleBufferContext*)mp;
    cbc->callback = callback;
}

void mv_mread_callback_unset(void *mp)
{
	CircleBufferContext* cbc = (CircleBufferContext*)mp;
	cbc->callback = 0;
}

uint32_t mv_mread(void* _buffer, uint32_t size, uint32_t count, void *mp)
{
    CircleBufferContext* cbc = (CircleBufferContext*)mp;

    uint8_t* buffer = (uint8_t*)_buffer;
    uint32_t required_bytes = size * count;
    uint32_t read_bytes;
    uint32_t remain_bytes;
    uint32_t wp = cbc->wp;

    if(required_bytes == 0) return 0;

    if(wp >= cbc->rp)
    {
        remain_bytes = wp - cbc->rp;

        if(required_bytes > remain_bytes)
        {
            read_bytes = remain_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);
            cbc->rp += read_bytes;

            if(cbc->callback)
            {
                read_bytes += cbc->callback(((uint8_t*)buffer) + read_bytes, required_bytes - read_bytes);
            }
        }
        else
        {
            read_bytes = required_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);
            cbc->rp += read_bytes;
        }
    }
    else
    {
        remain_bytes = cbc->capacity - cbc->rp;

        if(required_bytes > remain_bytes)
        {
            read_bytes = remain_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);

            if(required_bytes - read_bytes > wp)
            {
                memcpy(buffer + read_bytes, &cbc->address[0], wp);
                cbc->rp = wp;
                read_bytes += wp;

                if(cbc->callback)
                {
                    read_bytes += cbc->callback(((uint8_t*)buffer) + read_bytes, required_bytes - read_bytes);
                }
            }
            else
            {
                memcpy(buffer + read_bytes, &cbc->address[0], required_bytes - read_bytes);
                cbc->rp = required_bytes - read_bytes;
                read_bytes = required_bytes;
            }
        }
        else
        {
            read_bytes = required_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);
            cbc->rp += read_bytes;
        }
    }

    return read_bytes;
}

#define RWP_SAFE_INTERVAL   (1)

uint32_t mv_mwrite(void* _buffer, uint32_t size, uint32_t count, void* mp)
{
    CircleBufferContext* cbc = (CircleBufferContext*)mp;

    uint8_t* buffer = (uint8_t*)_buffer;
    uint32_t remain_bytes;
    uint32_t write_bytes = size * count;
    uint32_t rp = cbc->rp;

    if(cbc->wp >= rp)
    {
        remain_bytes = cbc->capacity - cbc->wp + rp;

        if(remain_bytes >= write_bytes + RWP_SAFE_INTERVAL)
        {
            remain_bytes = cbc->capacity - cbc->wp;

            if(remain_bytes >= write_bytes)
            {
                memcpy(&cbc->address[cbc->wp], buffer, write_bytes);
                cbc->wp += write_bytes;
            }
            else
            {
                memcpy(&cbc->address[cbc->wp], buffer, remain_bytes);
                cbc->wp = write_bytes - remain_bytes;
                memcpy(&cbc->address[0], &buffer[remain_bytes], cbc->wp);
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        remain_bytes = rp - cbc->wp;

        if(remain_bytes >= write_bytes + RWP_SAFE_INTERVAL)
        {
            memcpy(&cbc->address[cbc->wp], buffer, write_bytes);
            cbc->wp += write_bytes;
        }
        else
        {
            return 0;
        }
    }

    if(cbc->wp >= cbc->capacity && cbc->rp)
    {
        cbc->wp = 0;
    }

    return write_bytes;
}

int32_t mv_meom(void *mp)
{
    return 0;//((MemHandle*)mp)->p>=((MemHandle*)mp)->mem_len ? 1 : 0;
}

int32_t mv_msize(void *mp)
{
    CircleBufferContext* cbc = (CircleBufferContext*)mp;

    return cbc->wp >= cbc->rp ? cbc->wp - cbc->rp : cbc->capacity - cbc->rp + cbc->wp;
}

int32_t mv_mremain(void* mp)
{
    int32_t remain = ((CircleBufferContext*)mp)->capacity - mv_msize(mp);

    return remain > RWP_SAFE_INTERVAL ? remain - RWP_SAFE_INTERVAL : 0;
}

int32_t mv_strcmp(const int8_t *str1, const int8_t *str2)
{
   //assert((NULL != str1) && (NULL != str2));

   while(*str1 && *str2 && (*str1 == *str2))
   {
       str1 ++;
       str2 ++;
   }

   return *str1 - *str2;
}

/*---------------------------------------------------------------------------------------------*
 * File I/O operation api implementation for MV File System                                    *
 *---------------------------------------------------------------------------------------------*/
#if   defined(USE_MV_FS)

extern uint32_t FileRead(void *buffer, uint32_t size, uint32_t count, void *fp);
extern uint32_t FileWrite(const void* buffer, uint32_t size, uint32_t count, void *fp);
extern int32_t  FileTell(void *fp);
extern int32_t  FileSeek(void *fp, int32_t offset, int32_t origin);
extern int32_t  FileEOF(void *fp);
extern int32_t  FileSof(void *fp);
extern int32_t  FileClose(void *fp);

#if defined(USE_MV_FOPEN)

typedef unsigned char               bool;

//open mode
#define	MV_FOPEN_MODE_READ          (0x01)
#define	MV_FOPEN_MODE_WRITE         (0x02)
#define	MV_FOPEN_MODE_CREATE_NEW    (0x04)
#define	MV_FOPEN_MODE_CREATE_ALWAYS (0x08)

extern bool FileFlush(void* File);
extern bool FileOpen(void* fp, const uint8_t* file_name, const uint8_t mode);

void* mv_fopen(const int8_t *file_name, const int8_t *mode, void* fp)
{
    uint8_t m;
    int32_t add_flag = 0;

    if(!MV_STRCMP((const char*)mode,"r"))        
    {
        m = MV_FOPEN_MODE_READ;
    }
    else if(!MV_STRCMP((const char*)mode,"rb"))
    {
        m = MV_FOPEN_MODE_READ;
    }
    else if(!MV_STRCMP((const char*)mode,"w"))
    {
        m = MV_FOPEN_MODE_WRITE | MV_FOPEN_MODE_CREATE_ALWAYS;
    }
    else if(!MV_STRCMP((const char*)mode,"wb"))
    {
        m = MV_FOPEN_MODE_WRITE | MV_FOPEN_MODE_CREATE_ALWAYS;
    }
    else if(!MV_STRCMP((const char*)mode,"a"))
    {
        m = MV_FOPEN_MODE_WRITE | MV_FOPEN_MODE_CREATE_ALWAYS;
        add_flag = 1;
    }
    else
    {
        return NULL;
    }

    if(FileOpen(fp, (const uint8_t*)file_name, m))
    {
        if(m & MV_FOPEN_MODE_WRITE)
        {
            if(add_flag)
            {
                FileSeek(fp, 0, MV_SEEK_END);
            }
            else
            {
                FileFlush(fp);
            }
        }

        return fp;
    }
    else
    {
        return NULL;
    }
}

#endif

uint32_t mv_fread(void *buffer, uint32_t size, uint32_t count, void *fp)
{
    return FileRead(buffer, size, count, fp);
}

uint32_t mv_fwrite(const void *buffer, uint32_t size, uint32_t count, void *fp)
{
	return FileWrite(buffer, size, count, fp);
}

int32_t mv_ftell(void *fp)
{
    return FileTell(fp);
}

int32_t mv_fseek(void *fp, int32_t offset, int32_t origin)
{
    return FileSeek(fp, offset, origin);
}

int32_t mv_feof(void *fp)
{
    return FileEOF(fp);
}

int32_t mv_fsof(void *fp)
{
    return FileSof(fp);
}

int32_t mv_fclose(void *fp)
{
    return FileClose(fp);
}

//TODO: to be implemented in furture
//int32_t mv_fprintf(void *stream, const int8_t *format, ...);
//int32_t mv_fscanf(void *stream, const int8_t * format, ...);

/*---------------------------------------------------------------------------------------------*
 * File I/O operation api implementation for Fat File System (Open Source FatFs)               *
 *---------------------------------------------------------------------------------------------*/
#elif defined(USE_FATFS_FS)

#include "ff.h"

#define MAX_FILE_HANDLE_NUM     2  /**< Max file handle number */

typedef FIL MvFile;               /**< Type define FIL        */

/**
 * @brief FILE array struct, for fopen using
 */
typedef struct _MvFileArray
{
    MvFile   file[MAX_FILE_HANDLE_NUM];       /**< file array             */
    int8_t   use_flag[MAX_FILE_HANDLE_NUM];   /**< file handle using flag */
}MvFileArray;

/**
 * @brief File Array , for fopen using
 */
static MvFileArray files;

/**
 * @brief  Get file handle ID
 * @return  -1 failure, >=0 file id
 */
static int32_t get_file_handle_id()
{
    int32_t i;
    
    for(i=0; i<MAX_FILE_HANDLE_NUM; i++)
    {
        if(!files.use_flag[i])
        {
            return i;
        }
    }

    return -1;
}

void* mv_fopen(const int8_t *file_name, const int8_t *mode)
{
    BYTE    m;
    int32_t add_flag = 0;
    int32_t id;
    MvFile  *fp;

    if(!MV_STRCMP((const char*)mode,"r"))        
    {
        m = FA_READ;
    }
    else if(!MV_STRCMP((const char*)mode,"rb"))
    {
        m = FA_READ;
    }
    else if(!MV_STRCMP((const char*)mode,"w"))
    {
        m = FA_WRITE | FA_OPEN_ALWAYS;
    }
    else if(!MV_STRCMP((const char*)mode,"wb"))
    {
        m = FA_WRITE | FA_OPEN_ALWAYS;
    }
    else if(!MV_STRCMP((const char*)mode,"a"))
    {
        m = FA_WRITE | FA_OPEN_ALWAYS;
        add_flag = 1;
    }
    else
    {
        return 0;
    }

    if((id = get_file_handle_id())<0)
    {
        return 0;
    }
    else
    {
        fp = &files.file[id];
        files.use_flag[id] = 1;
    }

    // Delete the writing file if it already existed when open mode not contain 'a'(add)
    if((m&FA_WRITE) && (!add_flag))
    {
        f_unlink((const char*)file_name);
    }

    if(FR_OK == f_open(fp,(const char*)file_name,m))
    {
        if(add_flag) f_lseek(fp,fp->obj.objsize);
        return fp;
    }
    else
    {
        return 0;
    }
}

uint32_t mv_fread(void *buffer, uint32_t size, uint32_t count, void *fp)
{
    uint32_t rt_value;
    
    if(FR_OK == f_read((MvFile*)fp,buffer,size*count,&rt_value))
    {
        return rt_value/size;
    }

    return 0;
}

uint32_t mv_fwrite(const void *buffer, uint32_t size, uint32_t count, void *fp)
{
    uint32_t rt_value;
    if(FR_OK == f_write((MvFile*)fp,buffer,size*count,&rt_value))
    {
        return rt_value/size;
    }

    return 0;
}

int32_t mv_ftell(void *fp)
{
    return ((MvFile*)fp)->fptr;
}

int32_t mv_fseek(void *fp, int32_t offset, int32_t origin)
{
    int32_t ofs;

    switch(origin)
    {
    case MV_SEEK_SET:
        ofs = offset;
        break;
    case MV_SEEK_CUR:
        ofs = offset + ((MvFile*)fp)->fptr;
        break;
    case MV_SEEK_END:
        ofs = offset + ((MvFile*)fp)->obj.objsize;
        break;
    default:
        ofs = offset + ((MvFile*)fp)->fptr;
        break;
    }

    return f_lseek((MvFile*)fp,ofs);
}

int32_t mv_feof(void *fp)
{
    if(((MvFile*)fp)->fptr<((MvFile*)fp)->obj.objsize)
    {
        return 0;
    }
    else
    {
        return ((MvFile*)fp)->fptr-((MvFile*)fp)->obj.objsize+1;
    }
}

int32_t mv_fsof(void *fp)
{
    return ((MvFile*)fp)->obj.objsize;
}

int32_t mv_fclose(void *fp)
{
    files.use_flag[(MvFile*)fp-&files.file[0]] = 0;
    return f_close((MvFile*)fp);
}

//TODO: to be implemented in furture
//int32_t mv_fprintf(void *stream, const int8_t *format, ...);
//int32_t mv_fscanf(void *stream, const int8_t * format, ...);

/*---------------------------------------------------------------------------------------------*
 * File I/O operation api implementation for Standard File System                              *
 *---------------------------------------------------------------------------------------------*/
#else

#include <stdio.h>
#include <stdarg.h>

void* mv_fopen(const int8_t *file_name, const int8_t *mode)
{
    return (void*)fopen((const char*)file_name, (const char*)mode);
}

uint32_t mv_fread(void *buffer, uint32_t size, uint32_t count, void *fp)
{
    return fread(buffer, size, count, (FILE*)fp);
}

uint32_t mv_fwrite(const void *buffer, uint32_t size, uint32_t count, void *fp)
{
	return fwrite(buffer, size, count, (FILE*)fp);
}

int32_t mv_ftell(void *fp)
{
    return ftell((FILE*)fp);
}

int32_t mv_fseek(void *fp, int32_t offset, int32_t origin)
{
    return fseek((FILE*)fp, offset, origin);
}

int32_t mv_feof(void *fp)
{
    return feof((FILE*)fp);
}

int32_t mv_fsof(void *fp)
{
    FILE *f = fp;
    
    int32_t file_pos;
    int32_t file_size;

    file_pos  = ftell(f);
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, file_pos, SEEK_SET);

    return file_size; //filelength(fileno((FILE*)fp));
}

int32_t mv_fclose(void *fp)
{
    return fclose((FILE*)fp);
}

int32_t mv_fprintf(void *stream, const int8_t *format, ...)
{
    int32_t res;
    
    va_list args;
    va_start(args, format);
    res = fprintf(stream, format, args);
    va_end(args);
    
    return res;
}

int32_t mv_fscanf(void *stream, const int8_t * format, ...)
{
    int32_t res;
    
    va_list args;
    va_start(args, format);
    res = fscanf(stream, format, args);
    va_end(args);
    
    return res;
}

#endif
