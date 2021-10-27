/**
 **************************************************************************************
 * @file    mvintrinsics.h
 * @brief   Re-define intrinsics functions in Keil, IAR as the same interface
 * 
 * @author  Aissen Li, Alfred Zhao
 * @version V1.2.1
 * 
 * $Created: 2013-01-09 13:08:01$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
#ifndef __MVINTRINSICS_H__
#define __MVINTRINSICS_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

//#include "typedefine.h"
#include <stdint.h>
    
/*
 *-------------------------------------------------------------------------------------*
 * For MDK KEIL compiler                                                               *
 *-------------------------------------------------------------------------------------*
 */    
#if   defined(__CC_ARM)

#if (__ARMCC_VERSION < 400677)
#error "Please use ARM Compiler Toolchain V4.0.677 or later!"
#endif

#define __CLZ                   __clz                       /**< Count leading zeros                     */
#define __NOP                   __nop                       /**< No Operation does nothing               */
#define __WFI                   __wfi                       /**< Wait For Interrupt                      */
#define __SSAT                  __ssat                      /**< Signed Saturate                         */
#define __USAT                  __usat                      /**< Unsigned Saturate                       */
#define __REV                   __rev                       /**< Reverse byte order (32 bit)             */
#define __REV16                 __REV16                     /**< Reverse byte order (16 bit)             */
#define __REVSH                 __REVSH                     /**< Reverse byte order in signed short value*/
#define __ROR                   __ror                       /**< Rotate Right in unsigned value (32 bit) */
#define __RBIT                  __rbit                      /**< Reverse bit order of value              */

#define __LDREXB(ptr)           ((uint8_t )__ldrex(ptr))    /**< LDR command for  8 bit value            */
#define __LDREXH(ptr)           ((uint16_t)__ldrex(ptr))    /**< LDR command for 16 bit value            */
#define __LDREXW(ptr)           ((uint32_t)__ldrex(ptr))    /**< LDR command for 32 bit value            */
#define __STREXB(value, ptr)    __strex(value, ptr)         /**< STR command for  8 bit value            */
#define __STREXH(value, ptr)    __strex(value, ptr)         /**< STR command for 16 bit value            */
#define __STREXW(value, ptr)    __strex(value, ptr)         /**< STR command for 32 bit value            */
#define __CLREX                 __clrex                     /**< Removes lock created by LDREX           */

/**
 * @brief  Reverse byte order (16 bit)
 *          This function reverses the byte order in two unsigned short values.
 * @param  [in]  value  Value to reverse
 * @return Reversed value
 */
__STATIC_INLINE __ASM uint32_t __REV16(uint32_t value)
{
    rev16 r0, r0
    bx    lr
}

/** 
 * @brief  Reverse byte order in signed short value
 *          This function reverses the byte order in a signed short value with sign extension to integer.
 * @param  [in]  value  Value to reverse
 * @return Reversed value
 */
__STATIC_INLINE __ASM int32_t __REVSH(int32_t value)
{
    revsh r0, r0
    bx    lr
}

/*
 *-------------------------------------------------------------------------------------*
 * For IAR System compiler                                                             *
 *-------------------------------------------------------------------------------------*
 */
#elif defined(__ICCARM__)

#if (__VER__ < 6020000)
#error "Please use IAR Compiler Toolchain V6.20 or later!"
#endif
 
#include <intrinsics.h>

#define __CLZ                   __CLZ                       /**< Count leading zeros                     */
#define __NOP                   __no_operation              /**< No Operation does nothing               */
#define __WFI                   __WFI                       /**< Wait For Interrupt                      */
#define __SSAT                  __SSAT                      /**< Signed Saturate                         */
#define __USAT                  __USAT                      /**< Unsigned Saturate                       */
#define __REV                   __REV                       /**< Reverse byte order (32 bit)             */
#define __REV16                 __REV16                     /**< Reverse byte order (16 bit)             */
#define __REVSH                 __REVSH                     /**< Reverse byte order in signed short value*/
#define __ROR                   __ROR                       /**< Rotate Right in unsigned value (32 bit) */
#define __RBIT                  __RBIT                      /**< Reverse bit order of value              */

#define __LDREXB(ptr)           ((uint8_t )__LDREXB(ptr))   /**< LDR command for  8 bit value            */
#define __LDREXH(ptr)           ((uint16_t)__LDREXH(ptr))   /**< LDR command for 16 bit value            */
#define __LDREXW(ptr)           ((uint32_t)__LDREX(ptr))    /**< LDR command for 32 bit value            */
#define __STREXB(value, ptr)    __STREXB(value, ptr)        /**< STR command for  8 bit value            */
#define __STREXH(value, ptr)    __STREXH(value, ptr)        /**< STR command for 16 bit value            */
#define __STREXW(value, ptr)    __STREX(value, ptr)         /**< STR command for 32 bit value            */
#define __CLREX                 __CLREX                     /**< Removes lock created by LDREX           */

///*
// *-------------------------------------------------------------------------------------*
// * For GNU compiler                                                                    *
// *-------------------------------------------------------------------------------------*
// */
//#elif defined(__GNUC__)
////TODO: Add codes if to support GNU compiler
/*
 *-------------------------------------------------------------------------------------*
 * For Andes compiler                                                                  *
 *-------------------------------------------------------------------------------------*
 */
#elif defined(__NDS32__)

#include <nds32_intrinsic.h>

#define __CLZ                   __nds32__clz                		/**< Count leading zeros                     */
#define __NOP                   __nds32__nop              			/**< No Operation does nothing               */
#define __WFI                                          				/**< Wait For Interrupt                      */
#define __SSAT(v,s)             __nds32__clips(v, (s)-1)    		/**< Signed Saturate                         */
#define __USAT                  __nds32__clip               		/**< Unsigned Saturate                       */
#define __REV(v)                __nds32__rotr(__nds32__wsbh(v),16)  /**< Reverse byte order (32 bit)             */
#define __REV16                 __nds32__wsbh               		/**< Reverse byte order (16 bit)             */
#define __REVSH                                      				/**< Reverse byte order in signed short value*/
#define __ROR                   __nds32__rotr               		/**< Rotate Right in unsigned value (32 bit) */
//#define __RBIT                  __nds32__bitrev             		/**< Reverse bit order of value              */

/*
 *-------------------------------------------------------------------------------------*
 * For MSVC compiler                                                                   *
 *-------------------------------------------------------------------------------------*
 */
#elif defined(_MSC_VER) || defined(__GNUC__) // __GNUC__ for Qt's compiler

#define __CLZ                   __CLZ   /**< Count leading zeros                     */
#define __NOP()                         /**< No Operation does nothing               */
#define __WFI()                         /**< Wait For Interrupt                      */
#define __SSAT                  __SSAT  /**< Signed Saturate                         */
#define __USAT                  __USAT  /**< Unsigned Saturate                       */
#define __REV                   __REV   /**< Reverse byte order (32 bit)             */
#define __REV16                 __REV16 /**< Reverse byte order (16 bit)             */
#define __REVSH                 __REVSH /**< Reverse byte order in signed short value*/
#define __ROR                   __ROR   /**< Rotate Right in unsigned value (32 bit) */
#define __RBIT                  __RBIT  /**< Reverse bit order of value              */

#include <math.h>

#define LOG2(x)                 (log(x)/log(2))

__STATIC_INLINE uint8_t __CLZ(uint32_t value)
{
    if(value)
    {
        return 31-(uint8_t)LOG2(value);
    }
    else
    {
        return 32;
    }
}

__STATIC_INLINE int32_t __SSAT(int32_t value, uint32_t sat)
{
    //int32_t min = -(1<<sat)/2;
    //int32_t max = (1<<sat)/2-1;
	int32_t min = -(1 << (sat-1));
	int32_t max = (1<<(sat-1))-1;
    
    return value>max ? max : (value<min ? min : value); 
}

__STATIC_INLINE uint32_t __USAT(uint32_t value, uint32_t sat)
{
    uint32_t max = (1<<sat)-1;
    
    return value>max ? max : value; 
}

__STATIC_INLINE uint32_t __REV(uint32_t x)
{
    return (uint32_t)(((x) >> 24) | (((uint32_t)(x) & 0xff0000) >> 8) | (((uint32_t)(x) & 0xff00) << 8) | ((uint32_t)(x) << 24));
}

__STATIC_INLINE uint32_t __REV16(uint32_t x)
{
    return (uint16_t)(((x) >> 8)  | (((uint16_t)(x) & 0x00ff) << 8));
}

__STATIC_INLINE int32_t __REVSH(uint16_t x)
{
    //TODO:    
    return x;
}

#endif

// Common definitions
#define CLIP_16BIT(a) __SSAT(a,16)
#define CLIP_31BIT(a) __SSAT(a,31)
#define ABS(a) (((a) < 0) ? -(a) : (a))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define PI 3.14159265358979

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__MVINTRINSICS_H__
