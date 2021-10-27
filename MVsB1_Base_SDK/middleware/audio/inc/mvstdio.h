/**
 *************************************************************************************
 * @file     mvstdio.h
 * @brief    File operation api implementation
 *
 * @author   Aissen Li, ZHAO Ying
 * @version  V4.2.0
 *
 * &copy; Shanghai Mountain View Silicon Tenchnology Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef __MV_STDIO_H__
#define __MV_STDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "typedefine.h"

#ifdef __NDS32__
#define USE_FATFS_FS
#endif

/* Memory operation functions remap */
#define MV_MEMCPY           memcpy
#define MV_MEMCMP           memcmp
#define MV_MEMMOVE          memmove
#define MV_MEMSET           memset
#define MV_MREAD	        mv_mread
#define MV_MEOM             mv_meom
#define MV_STRCMP           strcmp
/* File operation functions remap   */
#define MV_FOPEN            mv_fopen
#define MV_FREAD	        mv_fread
#define MV_FWRITE           mv_fwrite
#define MV_FTELL            mv_ftell
#define MV_FSEEK            mv_fseek
#define MV_FEOF             mv_feof
#define MV_FSOF             mv_fsof
#define MV_FCLOSE           mv_fclose
#define MV_FPRINTF	        mv_fprintf
#define MV_FSCANF	        mv_fscanf
#define MV_SEEK_SET         0   /**< Beginning of file        */
#define MV_SEEK_CUR         1   /**< Current position of file */
#define MV_SEEK_END         2   /**< End of file              */

typedef uint32_t (*mv_mread_callback_t)(void *buffer, uint32_t length);

/**
 * @brief  Copy the values of num bytes from the location pointed by source directly to the memory block pointed by destination
 * @param  dst   Pointer to the destination array where the content is to be copied, type-casted to a pointer of type void*.
 * @param  src   Pointer to the source of data to be copied, type-casted to a pointer of type void*.
 * @param  num   Number of bytes to copy.
 * @return Pointer of the dst memory
 */
void* mv_memcpy(void* dst, const void* src, uint32_t num);

/**
 * @brief  Compare two block of memory
 * @param  ptr1    memory to be compared.
 * @param  ptr2    memory to be compared.
 * @return  0£ºptr1==ptr2, >0£ºptr1>ptr2, <0:ptr1<ptr2
 */
int32_t mv_memcmp(const void* ptr1, const void* ptr2, uint32_t num);

/**
 * @brief  Move the values of num bytes from the location pointed by source directly to the memory block pointed by destination
 * @param  dst   Pointer to the destination array where the content is to be moved, type-casted to a pointer of type void*.
 * @param  src   Pointer to the source of data to be moved, type-casted to a pointer of type void*.
 * @param  num   Number of bytes to move.
 * @return Pointer of the dst memory
 */
void* mv_memmove(void* dst, const void* src, uint32_t num);

/**
 * @brief  Set the first num bytes of the block of memory pointed by ptr to the specified value (interpreted as an unsigned int8_t).
 * @param  ptr   Pointer to the block of memory to fill.
 * @param  value Value to be set.
 * @param  num   Number of bytes to be set.
 * @return ptr is returned.
 */
void* mv_memset(void* ptr, int32_t value, uint32_t num);

/**
 * @brief  Memory handle open
 * @param  mp       Pointer to a memory pointer, @see MemHandle.
 * @param  addr     memory base address
 * @param  capacity memory capacity in bytes
 * @param  callback mv_mread callback function
 * @return NONE
 */
void mv_mopen(MemHandle* mp, uint8_t* addr, uint32_t capacity, mv_mread_callback_t callback);

/**
 * @brief  Memory handle close
 * @param  mp       Pointer to a memory pointer, @see MemHandle.
 * @return NONE
 */
void mv_mclose(MemHandle* mp);

/**
 * @brief  Set mv_mread callback function
 * @param  mp       Pointer to a memory pointer, @see MemHandle.
 * @param  callback mv_mread callback function
 * @return NONE
 */
void mv_mread_callback_set(void *mp, mv_mread_callback_t callback);

/**
 * @brief  Unset mv_mread callback function
 * @param  mp       Pointer to a memory pointer, @see MemHandle.
 * @param  NONE
 * @return NONE
 */
void mv_mread_callback_unset(void *mp);

/**
 * @brief  Read data from memory
 * @param  buffer  Pointer to a block of buffer with a minimum size of (size*count) bytes.
 * @param  size    Size in bytes of each element to be read.
 * @param  count   Number of elements, each one with a size of size bytes.
 * @param  mp      Pointer to a memory pointer.
 * @return The total number of elements successfully read.
 */
uint32_t mv_mread(void *buffer, uint32_t size, uint32_t count, void *mp);

/**
 * @brief  Write data into memory
 * @param  buffer  Pointer to a block of buffer with a minimum size of (size*count) bytes.
 * @param  size    Size in bytes of each element to be read.
 * @param  count   Number of elements, each one with a size of size bytes.
 * @param  mp      Pointer to a memory pointer.
 * @return The total number of elements successfully writed.
 */
uint32_t mv_mwrite(void* _buffer, uint32_t size, uint32_t count, void* mp);

/**
 * @brief  Check End-of-Memory indicator.
 * @param  mp      Pointer to a memory pointer.
 * @return returns a nonzero value if a read operation has attempted to read past the end of the memory; it returns 0 otherwise.
 */
int32_t mv_meom(void *mp);

/**
 * @brief  Get the size of valid memory.
 * @param  mp      Pointer to a memory pointer.
 * @return returns the size of valid memory.
 */
int32_t mv_msize(void *mp);

/**
 * @brief  Get the free space size of memory.
 * @param  mp      Pointer to a memory pointer.
 * @return returns the free space size of memory.
 */
int32_t mv_mremain(void* mp);

/**
 * @brief  Compare two strings
 * @param  str1    C string to be compared.
 * @param  str2    C string to be compared.
 * @return  0£ºstr1==str2, >0£ºstr1>str2, <0:str1<str2
 */
int32_t mv_strcmp(const int8_t *str1, const int8_t *str2);

/**
 * @brief  Open a file
 * @param  file_name    file name
 * @param  mode         open mode
 * @return File Handle
 */
#if defined(USE_MV_FS)
#if defined(USE_MV_FOPEN)
void* mv_fopen(const int8_t *file_name, const int8_t *mode, void* fp);
#endif
#else
void* mv_fopen(const int8_t *file_name, const int8_t *mode);
#endif

/**
 * @brief  Read data from stream
 * @param  buffer  Pointer to a block of buffer with a minimum size of (size*count) bytes.
 * @param  size    Size in bytes of each element to be read.
 * @param  count   Number of elements, each one with a size of size bytes.
 * @param  fp      Pointer to a FILE object that specifies an input stream.
 * @return The total number of elements successfully read.
 */
uint32_t mv_fread(void *buffer, uint32_t size, uint32_t count, void *fp);

/**
 * @brief  Write data to stream
 * @param  buffer  Pointer to a block of buffer with a minimum size of (size*count) bytes.
 * @param  size    Size in bytes of each element to be write.
 * @param  count   Number of elements, each one with a size of size bytes.
 * @param  fp      Pointer to a FILE object that specifies an output stream.
 * @return The total number of elements successfully write.
 */
uint32_t mv_fwrite(const void *buffer, uint32_t size, uint32_t count, void *fp);

/**
 * @brief  Gets the current position of a file pointer.
 * @param  fp      Pointer to a FILE object that specifies stream.
 * @return The current position of a file pointer.
 */
int32_t mv_ftell(void *fp);

/**
 * @brief  Moves the file pointer to a specified location.
 * @param  fp      Pointer to a FILE object that specifies stream.
 * @param  offset  Number of bytes from origin.
 * @param  origin  Initial position.
 * @return If successful, the function returns a zero value. Otherwise, it returns nonzero value.
 */
int32_t mv_fseek(void *fp, int32_t offset, int32_t origin);

/**
 * @brief  Check End-of-File indicator.
 * @param  fp      Pointer to a FILE object that specifies stream.
 * @return The feof function returns a nonzero value if a read operation has attempted to read past the end of the file; it returns 0 otherwise.
 */
int32_t mv_feof(void *fp);

/**
 * @brief  Get size of the opened file.
 * @param  fp      Pointer to a FILE object that specifies an output stream.
 * @return size of the opened file.
 */
int32_t mv_fsof(void *fp);

/**
 * @brief  Closes a stream.
 * @param  fp      Pointer to a FILE object that specifies an output stream.
 * @return fclose returns 0 if the stream is successfully closed.
 */
int32_t mv_fclose(void *fp);

/**
 * @brief Writes to the specified stream a sequence of data formatted as the format argument specifies. 
 *        After the format parameter, the function expects at least as many additional arguments as 
 *        specified in format.
 * @param stream  Pointer to a FILE object that identifies the stream. 
 * @param format  C string that contains the text to be written to the stream. It can optionally contain 
 *                embedded format tags that are replaced by the values specified in subsequent additional 
 *                arguments and formatted as requested. The number of arguments following the format 
 *                parameters should at least be as much as the number of format tags.
 * @param additional arguments Depending on the format string, the function may expect a sequence of 
 *                additional arguments, each containing one value to be inserted instead of each %-tag 
 *                specified in the format parameter, if any. There should be the same number of these 
 *                arguments as the number of %-tags that expect a value.
 * @return On success, the total number of characters written is returned.
 *          On failure, a negative number is returned.
 */ 
int32_t mv_fprintf(void *stream, const int8_t *format, ...);

/**
 * @brief Reads data from the stream and stores them according to the parameter format into the 
 *        locations pointed by the additional arguments. The additional arguments should point to 
 *        already allocated objects of the type specified by their corresponding format tag within 
 *        the format string.
 * @param stream Pointer to a FILE object that identifies the stream to read data from. 
 * @param format C string that contains one or more of the following items
 * @param additional arguments The function expects a sequence of references as additional arguments, 
 *        each one pointing to an object of the type specified by their corresponding %-tag within the 
 *        format string, in the same order. For each format specifier in the format string that 
 *        retrieves data, an additional argument should be specified. These arguments are expected to 
 *        be references (pointers): if you want to store the result of a fscanf operation on a regular 
 *        variable you should precede its identifier with the reference operator, i.e. an ampersand 
 *        sign (&), like in: 
 *                          int32_t n;
 *                          fscanf (stream,"%d",&n);
 * @return On success, the function returns the number of items successfully read. This count can 
 *          match the expected number of readings or be less -even zero- in the case of a matching 
 *          failure. In the case of an input failure before any data could be successfully read, EOF 
 *          is returned.
 */
int32_t mv_fscanf(void *stream, const int8_t * format, ...);

#ifdef __cplusplus
}
#endif

#endif//__MV_STDIO_H__
