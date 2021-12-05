/* Copyright (c) 2018 SENSORO Co.,Ltd. All Rights Reserved.
 *
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#ifdef __cplusplus
extern "C"
{
#endif

//#include "stdbool.h"

/** The upper 8 bits of a 32 bit value */
//lint -emacro(572,MSB) // Suppress warning 572 "Excessive shift value"
#define MSB_32(a) (((a) & 0xFF000000) >> 24)
/** The lower 8 bits (of a 32 bit value) */
#define LSB_32(a) ((a) & 0x000000FF)

/** The upper 8 bits of a 16 bit value */
//lint -emacro(572,MSB_16) // Suppress warning 572 "Excessive shift value"
#define MSB_16(a) (((a) & 0xFF00) >> 8)
/** The lower 8 bits (of a 16 bit value) */
#define LSB_16(a) ((a) & 0x00FF)

/** Leaves the minimum of the two 32-bit arguments */
/*lint -emacro(506, MIN) */ /* Suppress "Constant value Boolean */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
/** Leaves the maximum of the two 32-bit arguments */
/*lint -emacro(506, MAX) */ /* Suppress "Constant value Boolean */
#define MAX(a, b) ((a) < (b) ? (b) : (a))

/** Concatenates two parameters. Useful as a second level of indirection,
 *  when a parameter can be macro itself. */
#define CONCAT_2(p1, p2)      p1##p2
/** Concatenates three parameters. Useful as a second level of indirection,
 *  when a parameter can be macro itself. */
#define CONCAT_3(p1, p2, p3)  p1##p2##p3

/**@brief Set a bit in the uint32 word.
 *
 * @param[in] W  Word whose bit is being set.
 * @param[in] B  Bit number in the word to be set.
 */
#ifndef SET_BIT
#define SET_BIT(W,B)  ((W) |= (uint32_t)(1U << (B)))
#endif

/**@brief Clears a bit in the uint32 word.
 *
 * @param[in] W   Word whose bit is to be cleared.
 * @param[in] B   Bit number in the word to be cleared.
 */
#define CLR_BIT(W, B) ((W) &= (~((uint32_t)1 << (B))))


/**@brief Checks if a bit is set.
 *
 * @param[in] W   Word whose bit is to be checked.
 * @param[in] B   Bit number in the word to be checked.
 *
 * @retval 1 if bit is set.
 * @retval 0 if bit is not set.
 */
#define IS_SET(W,B) (((W) >> (B)) & 1)

#define BIT_0 0x01 /**< The value of bit 0 */
#define BIT_1 0x02 /**< The value of bit 1 */
#define BIT_2 0x04 /**< The value of bit 2 */
#define BIT_3 0x08 /**< The value of bit 3 */
#define BIT_4 0x10 /**< The value of bit 4 */
#define BIT_5 0x20 /**< The value of bit 5 */
#define BIT_6 0x40 /**< The value of bit 6 */
#define BIT_7 0x80 /**< The value of bit 7 */
#define BIT_8 0x0100 /**< The value of bit 8 */
#define BIT_9 0x0200 /**< The value of bit 9 */
#define BIT_10 0x0400 /**< The value of bit 10 */
#define BIT_11 0x0800 /**< The value of bit 11 */
#define BIT_12 0x1000 /**< The value of bit 12 */
#define BIT_13 0x2000 /**< The value of bit 13 */
#define BIT_14 0x4000 /**< The value of bit 14 */
#define BIT_15 0x8000 /**< The value of bit 15 */
#define BIT_16 0x00010000 /**< The value of bit 16 */
#define BIT_17 0x00020000 /**< The value of bit 17 */
#define BIT_18 0x00040000 /**< The value of bit 18 */
#define BIT_19 0x00080000 /**< The value of bit 19 */
#define BIT_20 0x00100000 /**< The value of bit 20 */
#define BIT_21 0x00200000 /**< The value of bit 21 */
#define BIT_22 0x00400000 /**< The value of bit 22 */
#define BIT_23 0x00800000 /**< The value of bit 23 */
#define BIT_24 0x01000000 /**< The value of bit 24 */
#define BIT_25 0x02000000 /**< The value of bit 25 */
#define BIT_26 0x04000000 /**< The value of bit 26 */
#define BIT_27 0x08000000 /**< The value of bit 27 */
#define BIT_28 0x10000000 /**< The value of bit 28 */
#define BIT_29 0x20000000 /**< The value of bit 29 */
#define BIT_30 0x40000000 /**< The value of bit 30 */
#define BIT_31 0x80000000 /**< The value of bit 31 */

#define UNUSED_VARIABLE(X)  ((void)(X))
#define UNUSED_PARAMETER(X) UNUSED_VARIABLE(X)
#define UNUSED_RETURN_VALUE(X) UNUSED_VARIABLE(X)

#define ARRAY_SIZE(array)   ((sizeof(array) / sizeof((array)[0])))

/**@brief Macro for performing rounded integer division (as opposed to truncating the result).
 *
 * @param[in]   A   Numerator.
 * @param[in]   B   Denominator.
 *
 * @return      Rounded (integer) result of dividing A by B.
 */
#define ROUNDED_DIV(A, B) (((A) + ((B) / 2)) / (B))


/**@brief Macro for checking if an integer is a power of two.
 *
 * @param[in]   A   Number to be tested.
 *
 * @return      true if value is power of two.
 * @return      false if value not power of two.
 */
#define IS_POWER_OF_TWO(A) ( ((A) != 0) && ((((A) - 1) & (A)) == 0) )


/**@brief Macro for converting milliseconds to ticks.
 *
 * @param[in] TIME          Number of milliseconds to convert.
 * @param[in] RESOLUTION    Unit to be converted to in [us/ticks].
 */
#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))


/**@brief Macro for performing integer division, making sure the result is rounded up.
 *
 * @details One typical use for this is to compute the number of objects with size B is needed to
 *          hold A number of bytes.
 *
 * @param[in]   A   Numerator.
 * @param[in]   B   Denominator.
 *
 * @return      Integer result of dividing A by B, rounded up.
 */
#define CEIL_DIV(A, B)      \
    (((A) + (B) - 1) / (B))


/**@brief Macro for creating a buffer aligned to 4 bytes.
 *
 * @param[in]   NAME        Name of the buffor.
 * @param[in]   MIN_SIZE    Size of this buffor (it will be rounded up to multiples of 4 bytes).
 */
#define WORD_ALIGNED_MEM_BUFF(NAME, MIN_SIZE) static uint32_t NAME[CEIL_DIV(MIN_SIZE, sizeof(uint32_t))]


/**@brief Macro for calculating the number of words that are needed to hold a number of bytes.
 *
 * @details Adds 3 and divides by 4.
 *
 * @param[in]  n_bytes  The number of bytes.
 *
 * @return The number of words that @p n_bytes take up (rounded up).
 */
#define BYTES_TO_WORDS(n_bytes) (((n_bytes) + 3) >> 2)


/**@brief The number of bytes in a word.
 */
#define BYTES_PER_WORD (4)


/**@brief Macro for increasing a number to the nearest (larger) multiple of another number.
 *
 * @param[in]  alignment  The number to align to.
 * @param[in]  number     The number to align (increase).
 *
 * @return The aligned (increased) @p number.
 */
#define ALIGN_NUM(alignment, number) ((number - 1) + alignment - ((number - 1) % alignment))

#define IS_NULL(p_data)                 \
    if(!p_data)                         \
    {                                   \
        return -EINVAL;                 \
    }                                   \


/**@brief Function for changing the value unit.
 *
 * @param[in]   value               Value to be rescaled.
 * @param[in]   old_unit_reversal   Reversal of the incoming unit.
 * @param[in]   new_unit_reversal   Reversal of the desired unit.
 *
 * @return      Number of bytes written.
 */
static inline uint64_t value_rescale(uint32_t value, uint32_t old_unit_reversal, uint16_t new_unit_reversal)
{
    return (uint64_t)ROUNDED_DIV((uint64_t)value * new_unit_reversal, old_unit_reversal);
}

/**@brief Function for encoding a uint16 value.
 *
 * @param[in]   value            Value to be encoded.
 * @param[out]  p_encoded_data   Buffer where the encoded data is to be written.
 *
 * @return      Number of bytes written.
 */
static inline uint8_t uint16_encode(uint16_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x00FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0xFF00) >> 8);
    return sizeof(uint16_t);
}

/**@brief Function for encoding a three-byte value.
 *
 * @param[in]   value            Value to be encoded.
 * @param[out]  p_encoded_data   Buffer where the encoded data is to be written.
 *
 * @return      Number of bytes written.
 */
static inline uint8_t uint24_encode(uint32_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x000000FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0x0000FF00) >> 8);
    p_encoded_data[2] = (uint8_t) ((value & 0x00FF0000) >> 16);
    return 3;
}

/**@brief Function for encoding a uint32 value.
 *
 * @param[in]   value            Value to be encoded.
 * @param[out]  p_encoded_data   Buffer where the encoded data is to be written.
 *
 * @return      Number of bytes written.
 */
static inline uint8_t uint32_encode(uint32_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x000000FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0x0000FF00) >> 8);
    p_encoded_data[2] = (uint8_t) ((value & 0x00FF0000) >> 16);
    p_encoded_data[3] = (uint8_t) ((value & 0xFF000000) >> 24);
    return sizeof(uint32_t);
}

/**@brief Function for encoding a uint48 value.
 *
 * @param[in]   value            Value to be encoded.
 * @param[out]  p_encoded_data   Buffer where the encoded data is to be written.
 *
 * @return      Number of bytes written.
 */
static inline uint8_t uint48_encode(uint64_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x0000000000FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0x00000000FF00) >> 8);
    p_encoded_data[2] = (uint8_t) ((value & 0x000000FF0000) >> 16);
    p_encoded_data[3] = (uint8_t) ((value & 0x0000FF000000) >> 24);
    p_encoded_data[4] = (uint8_t) ((value & 0x00FF00000000) >> 32);
    p_encoded_data[5] = (uint8_t) ((value & 0xFF0000000000) >> 40);
    return 6;
}

/**@brief Function for decoding a uint16 value.
 *
 * @param[in]   p_encoded_data   Buffer where the encoded data is stored.
 *
 * @return      Decoded value.
 */
static inline uint16_t uint16_decode(const uint8_t * p_encoded_data)
{
        return (uint16_t)( (((uint16_t)((uint8_t *)p_encoded_data)[0])) |
                           (((uint16_t)((uint8_t *)p_encoded_data)[1]) << 8 ) );
}

/**@brief Function for decoding a uint16 value in big-endian format.
 *
 * @param[in]   p_encoded_data   Buffer where the encoded data is stored.
 *
 * @return      Decoded value.
 */
static inline uint16_t uint16_big_decode(const uint8_t * p_encoded_data)
{
        return (uint16_t)( (((uint16_t)((uint8_t *)p_encoded_data)[0]) << 8 ) |
                           (((uint16_t)((uint8_t *)p_encoded_data)[1])) );
}

/**@brief Function for decoding a three-byte value.
 *
 * @param[in]   p_encoded_data   Buffer where the encoded data is stored.
 *
 * @return      Decoded value (uint32_t).
 */
static inline uint32_t uint24_decode(const uint8_t * p_encoded_data)
{
    return ( (((uint32_t)((uint8_t *)p_encoded_data)[0]) << 0)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 8)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 16));
}

static inline uint32_t uint24_big_decode(const uint8_t * p_encoded_data)
{
    return ( (((uint32_t)((uint8_t *)p_encoded_data)[0]) << 16)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 8)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 0));
}

/**@brief Function for decoding a uint32 value.
 *
 * @param[in]   p_encoded_data   Buffer where the encoded data is stored.
 *
 * @return      Decoded value.
 */
static inline uint32_t uint32_decode(const uint8_t * p_encoded_data)
{
    return ( (((uint32_t)((uint8_t *)p_encoded_data)[0]) << 0)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 8)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 16) |
             (((uint32_t)((uint8_t *)p_encoded_data)[3]) << 24 ));
}

/**@brief Function for decoding a uint32 value in big-endian format.
 *
 * @param[in]   p_encoded_data   Buffer where the encoded data is stored.
 *
 * @return      Decoded value.
 */
static inline uint32_t uint32_big_decode(const uint8_t * p_encoded_data)
{
    return ( (((uint32_t)((uint8_t *)p_encoded_data)[0]) << 24) |
             (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 16) |
             (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 8)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[3]) << 0) );
}

/**@brief Function for encoding a uint32 value in big-endian format.
 *
 * @param[in]   value            Value to be encoded.
 * @param[out]  p_encoded_data   Buffer where the encoded data will be written.
 *
 * @return      Number of bytes written.
 */
static inline uint8_t uint32_big_encode(uint32_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0xFF000000) >> 24);
    p_encoded_data[1] = (uint8_t) ((value & 0x00FF0000) >> 16);
    p_encoded_data[2] = (uint8_t) ((value & 0x0000FF00) >> 8);
    p_encoded_data[3] = (uint8_t) ((value & 0x000000FF) >> 0);

    return sizeof(uint32_t);
}

/**@brief Function for decoding a uint48 value.
 *
 * @param[in]   p_encoded_data   Buffer where the encoded data is stored.
 *
 * @return      Decoded value. (uint64_t)
 */
static inline uint64_t uint48_decode(const uint8_t * p_encoded_data)
{
    return ( (((uint64_t)((uint8_t *)p_encoded_data)[0]) << 0)  |
             (((uint64_t)((uint8_t *)p_encoded_data)[1]) << 8)  |
             (((uint64_t)((uint8_t *)p_encoded_data)[2]) << 16) |
             (((uint64_t)((uint8_t *)p_encoded_data)[3]) << 24) |
             (((uint64_t)((uint8_t *)p_encoded_data)[4]) << 32) |
             (((uint64_t)((uint8_t *)p_encoded_data)[5]) << 40 ));
}

/**@brief Function for checking if a pointer value is aligned to a 4 byte boundary.
 *
 * @param[in]   p   Pointer value to be checked.
 *
 * @return      TRUE if pointer is aligned to a 4 byte boundary, FALSE otherwise.
 */
static inline bool is_word_aligned(void const* p)
{
    return (((uintptr_t)p & 0x03) == 0);
}

#ifdef __cplusplus
}
#endif

#endif
