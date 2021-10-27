/**
 *************************************************************************************
 * @file    typedefine.h
 * @brief   Redefine some common data type.
 *
 * @author  Aissen Li, Alfred Zhao
 * @version V5.2.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *
 *************************************************************************************
 */

#ifndef __TYPE_DEFINE_H__
#define __TYPE_DEFINE_H__

#include <stdint.h>
///**
// * Redefine data type
// */
//typedef signed char             int8_t;
//typedef unsigned char           uint8_t;
//typedef signed short            int16_t;
//typedef unsigned short          uint16_t;
//typedef signed int              int32_t;
//typedef unsigned int            uint32_t;
//#ifdef  _MSC_VER
//    typedef __int64             int64_t;
//    typedef unsigned __int64    uint64_t;
//#else
//    typedef signed long long    int64_t;
//    typedef unsigned long long  uint64_t;
//#endif//_MSC_VER

#ifndef NULL
#define NULL                    0
#endif

#define RT_FAILURE              0           /**< return code: failure */
#define RT_SUCCESS              1           /**< return code: success */
#define RT_NO                   0           /**< return code: no      */
#define RT_YES                  1           /**< return code: yes     */

#if   defined (__CC_ARM)        // Keywords for MDK Keil compiler
    #define __ASM               __asm
    #define __INLINE            __inline
    #define INLINE              __inline
    #define __STATIC_INLINE     static __inline
     
#elif defined (__ICCARM__)      // Keywords for IAR compiler
    #define __ASM               __asm
    #define __INLINE            inline
    #define INLINE              inline
    #define __STATIC_INLINE     static inline
        
#elif defined (__GNUC__)        // Keywords for GNU compiler
    #define __ASM               __asm
    #define __INLINE            inline
    #define INLINE              inline
    #define __STATIC_INLINE     static inline
        
#elif defined (_MSC_VER)        // Keywords for MSVC compiler
    #define __ASM               __asm
    #define __INLINE            __inline
    #define INLINE              __inline
    #define __STATIC_INLINE     static __inline

#endif

/**
 * @brief IO Type, can be FILE, MEMORY...
 */
typedef enum _IOType
{
    IO_TYPE_FILE    = 0,        /**< FILE   */
    IO_TYPE_MEMORY              /**< MEMORY */
}IOType;

/**
 * @brief Buffer type. The buffer can be used for input (BUFFER_FOR_INPUT) or output (BUFFER_FOR_OUTPUT)
 */
typedef enum _BufferType
{
    BUFFER_FOR_INPUT  = 0,      /**< buffer for input from external storage */
    BUFFER_FOR_OUTPUT = 1,      /**< buffer for output to external storage  */
}BufferType;

///**
// * @brief PCM data layout mode
// */
//typedef enum _PCM_DATA_MODE
//{
//    DATA_MODE_MONO_S = 0,       /**< Mono-S    : one channel, the pcm data save as "S", both low 16bits and high 16bits have data   */
//    DATA_MODE_MONO_LOW,         /**< Mono-Low  : one channel, only low 16bits have data                                             */
//    DATA_MODE_MONO_HIGH,        /**< Mono-High : one channel, only high 16bits have data                                            */
//    DATA_MODE_STEREO_R_L,       /**< R+L       : two channels, low 16bits are left data, high 16bits are right data                 */
//    DATA_MODE_STEREO_L_R,       /**< L+R       : two channels, low 16bits are right data, high 16bits are left data                 */
//    DATA_MODE_STEREO_HIGH,      /**< Stero-High: two channels, only high 16bits have data                                           */
//    DATA_MODE_STEREO_LOW,       /**< Stero-low : two channels, only low 16bits have data                                            */
//    DATA_MODE_STEREO_S          /**< Stereo-S  : two channels, the pcm data save as "S", both low 16bits and high 16bits have data  */
//}PCM_DATA_MODE;

/**
 * @brief read callback func pointer type
 */
typedef uint32_t (*mv_mread_callback_t)(void *buffer, uint32_t length);

/**
 * @brief Memory Handle Struct
 */
typedef struct _MemHandle
{
    uint8_t    *addr;           /**< memory base address            */
    uint32_t   mem_capacity;    /**< memory capacity in bytes       */
    uint32_t   mem_len;         /**< actual memory length in bytes  */    
    uint32_t   p;               /**< current pointer in bytes       */
    mv_mread_callback_t cb;		/**< read callback func pointer     */
}MemHandle;

/**
 * @brief Buffer context for bit reading (input buffer) / putting (output buffer)
 */
typedef struct _BufferContext
{
    void        *io_handle;         /**< IO handle/pointer, may be FILE *, Memory address. */
    int32_t     io_type;            /**< IO type, value must be in @code IOType            */
    int32_t     buffer_type;        /**< buffer type, value must be in @code BufferType    */
    uint8_t     *buffer;            /**< buffer base address                               */
    int32_t     buffer_capacity;    /**< buffer capacity in bytes                          */    
    int32_t     p;                  /**< current pointer in bits                           */   
    int32_t     buffer_len;         /**< actual buffer length in bytes (input buffer)      */    
    int32_t     flush_pointer;      /**< current flush pointer in bytes                    */
}BufferContext;

#endif//__TYPE_DEFINE_H__
