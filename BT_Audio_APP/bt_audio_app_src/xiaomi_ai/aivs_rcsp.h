#ifndef XIAOMI_RCSP_H_
#define XIAOMI_RCSP_H_


#include <stdio.h>
#include "type.h"

//rscp handle
#define AIVS_RCSP_RX_HANDLE       0x100C
#define AIVS_RCSP_TX_HANDLE       0x100E

//device information
#define AIVS_VID             0xFFFF
#define AIVS_PID             0xFFFF
#define AIVS_VERSION         0x1001

#define os_printf(format, ...)			printf(format, ##__VA_ARGS__)
#define os_memcpy	memcpy
#define os_memset 	memset

#define SWAP_ENDIAN16(x)	swap_endian16(x)


typedef uint32_t u_int32;
typedef uint32_t uint32;

typedef uint16_t u_int16;
typedef uint16_t uint16;

typedef uint8_t u_int8;
typedef uint8_t uint8;

typedef unsigned int uint;

typedef bool	bool_t;

//aivs rcsp format
typedef struct
{
    u_int8 cmd;
    u_int8 opcode;
    u_int16 param_len;
    u_int8* param_ptr;
}__attribute__((packed)) AIVS_RCSP;

//aivs rcsp response flag
typedef enum
{
    RCSP_NO_RESP,
    RCSP_RESP,
}aivs_rcsp_resp_flag;

//aivs rcsp status
typedef enum
{
    RCSP_STAT_SUCCESS,
    RCSP_STAT_FAIL,
    RCSP_STAT_UNKOWN_CMD,
    RCSP_STAT_BUSY,
    RCSP_STAT_No_RESP,
    RCSP_STAT_CRC_ERROR,
    RCSP_STAT_ALL_DATA_CRC_ERROR,
    RCSP_STAT_PARAM_ERROR,
    RCSP_STAT_RESPONSE_DATA_OVER_LIMIT,
}aivs_rcsp_status;

void aivs_rcsp_cmd_resp(u_int8 opcode, u_int16 param_len, u_int8 *param_ptr);
uint8 aivs_speech_is_ongoing(void);
void aivs_speech_error_check(void);
void aivs_speech_start(void);
void aivs_speech_data(uint8* data, uint16 length);
void aivs_app_decode(uint8 length, uint8 *pValue);
uint8_t xm_speech_isbusy();
void xm_ai_deinit();

#endif /* XIAOMI_RCSP_H_ */
