
#include "aivs_rcsp.h"
#include "app_config.h"

#ifdef CFG_XIAOAI_AI_EN

#define AIVS_OTA_INFO_OFFSET_ADDR    0x000200
#define AIVS_OTA_INFO_OFFSET_LEN     18
#define AIVS_OTA_FIRST_BLOCK_ADDR    0x000400
#define AIVS_OTA_BLOCK_LEN           235
#define AIVS_OTA_BLOCK_DELAY         0         //delay time: 20ms

typedef enum
{
    AIVS_OTA_STATUS_SINGLE_BACKUP_OKAY  = 0x00,
    AIVS_OTA_STATUS_LOW_BATTERY         = 0x01,
    AIVS_OTA_STATUS_FIRMWARE_ERROR      = 0x02,
    AIVS_OTA_STATUS_DUAL_BACKUP_OKAY    = 0x03,
}aivs_ota_status;

typedef enum
{
    AIVS_OTA_RESULT_SUCCESS             = 0x00,
    AIVS_OTA_RESULT_CRC_ERROR           = 0x01,
    AIVS_OTA_RESULT_FAIL                = 0x02,
    AIVS_OTA_RESULT_KEY_MISMATCH        = 0x03,
    AIVS_OTA_RESULT_FILE_ERROR          = 0x04,
    AIVS_OTA_RESULT_UBOOT_MISMATCH      = 0x05,
}aivs_ota_result;

static uint32 aivs_ota_block_len = 0;
static uint32 aivs_ota_block_num = 0;
uint8 aivs_ota_start_flag = 0;
uint8 aivs_ota_data[AIVS_OTA_BLOCK_LEN];
uint8 aivs_ota_data_len = 0;

void aivs_ota_erase_flash(void)
{
    uint32 addr;
    os_printf("Erase flash, Please waiting......\r\n");

    //to do erase operation

    os_printf("Erase success!!!\r\n");
}

void aivs_ota_write_flash(void)
{
    u_int32 wr_addr;
    uint8 data[AIVS_OTA_BLOCK_LEN];
    uint8 i;

    extern u_int8 le_mode;
    if(le_mode)
        return;

    if((aivs_ota_data_len))
    {
        //to do write the flash operation

        aivs_ota_data_len = 0;
    }
}

void aivs_ota_save_data(u_int8 *buf, u_int32 len)
{
    memcpy(aivs_ota_data, buf, len);
    aivs_ota_data_len = len;
}

uint8 aivs_ota_is_ongoing(void)
{
    return aivs_ota_start_flag;
}

void aivs_ota_get_info_offset_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[8];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    os_printf("aivs_ota_get_info_offset\r\n");

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];                               //opcode_SN
    pdu[2] = (uint8)((AIVS_OTA_INFO_OFFSET_ADDR & 0xFF000000) >> 24);   // offset high byte
    pdu[3] = (uint8)((AIVS_OTA_INFO_OFFSET_ADDR & 0x00FF0000) >> 16);   // offset high byte
    pdu[4] = (uint8)((AIVS_OTA_INFO_OFFSET_ADDR & 0x0000FF00) >> 8);    // offset low byte
    pdu[5] = (uint8)((AIVS_OTA_INFO_OFFSET_ADDR & 0x000000FF) >> 0);    // offset low byte
    pdu[6] = (uint8)((AIVS_OTA_INFO_OFFSET_LEN & 0xFF00) >> 8);         // length high byte
    pdu[7] = (uint8)((AIVS_OTA_INFO_OFFSET_LEN & 0x00FF) >> 0);         // length low byte

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
    }
}

void aivs_ota_inquiry_if_can_update_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint16 vid = 0;
    uint16 pid = 0;
    uint16 version = 0;
    uint32 length = 0;
    uint32 crc = 0;
    uint8 pdu[3];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    os_printf("aivs_ota_inquiry_dev_if_can_update_resp\r\n");
    //get the information about OTA file from the E2 command(Byte5-xx)
    memcpy((uint8 *)&vid, &aivs_rcsp_pdu->param_ptr[1], 2);
    memcpy((uint8 *)&pid, &aivs_rcsp_pdu->param_ptr[3], 2);
    memcpy((uint8 *)&version, &aivs_rcsp_pdu->param_ptr[5], 2);
    memcpy((uint8 *)&length, &aivs_rcsp_pdu->param_ptr[7], 4);
    memcpy((uint8 *)&crc, &aivs_rcsp_pdu->param_ptr[15], 4);
//    vid     = SWAP_ENDIAN16(vid);
//    pid     = SWAP_ENDIAN16(pid);
//    version = SWAP_ENDIAN16(version);
//    length  = SWAP_ENDIAN32(length);
//    crc     = SWAP_ENDIAN32(crc);

    os_printf("aivs_ota vid:%x, pid:%x, ver:%x, length:%x, crc:%x\r\n", vid, pid, version, length, crc);

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];
    pdu[2] = AIVS_OTA_STATUS_DUAL_BACKUP_OKAY; // it is means that the device is dual backup.

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        if((aivs_rcsp_pdu->param_len == (AIVS_OTA_INFO_OFFSET_LEN + 1)) && (vid == AIVS_VID) && (pid == AIVS_PID)
           && (version != AIVS_VERSION) && (length != 0) && (crc != 0)) // check the information from E2 cmd
        {
            aivs_ota_block_num = length;
            aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
        }
    }
}

void aivs_ota_enter_update_mode_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[10];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    os_printf("aivs_ota_enter_update_mode_resp\r\n");

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];                               //opcode_SN
    pdu[2] = 0x00;                                                    //0: success, 1: fail
    pdu[3] = (uint8)((AIVS_OTA_FIRST_BLOCK_ADDR & 0xFF000000) >> 24);   // first block high byte
    pdu[4] = (uint8)((AIVS_OTA_FIRST_BLOCK_ADDR & 0x00FF0000) >> 16);   // first block high byte
    pdu[5] = (uint8)((AIVS_OTA_FIRST_BLOCK_ADDR & 0x0000FF00) >> 8);    // first block low byte
    pdu[6] = (uint8)((AIVS_OTA_FIRST_BLOCK_ADDR & 0x000000FF) >> 0);    // first block low byte
    pdu[7] = (uint8)((AIVS_OTA_BLOCK_LEN & 0xFF00) >> 8);               // block length high byte
    pdu[8] = (uint8)((AIVS_OTA_BLOCK_LEN & 0x00FF) >> 0);               // block length low byte
    pdu[9] = 0x00;                                                    //0: without CRC32, 1: with CRC32

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        aivs_ota_block_len = 0;
        aivs_ota_start_flag = 1;
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
    }
}

void aivs_ota_exit_update_mode_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[3];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    os_printf("aivs_ota_exit_update_mode_resp\r\n");

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];                               //opcode_SN
    pdu[3] = 0x01;                                                    //0: success, 1: fail

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
    }
}

void aivs_ota_send_update_block_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[11];
    uint32 request_len = 0;
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    //os_printf("aivs_ota_send_update_block_resp\r\n");

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];                               //opcode_SN

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        if(aivs_ota_block_len < aivs_ota_block_num)
        {
            if((aivs_ota_block_len + AIVS_OTA_BLOCK_LEN) <= aivs_ota_block_num)
            {
                request_len = AIVS_OTA_BLOCK_LEN;
                aivs_ota_block_len += AIVS_OTA_BLOCK_LEN;
            }
            else
            {
                request_len = aivs_ota_block_num - aivs_ota_block_len;
                aivs_ota_block_len = aivs_ota_block_num;
            }
        }

        if(request_len)
        {
            pdu[2] = 0x00;                                                    // 0: success, 1: fail
            pdu[3] = (uint8)(((AIVS_OTA_FIRST_BLOCK_ADDR + aivs_ota_block_len) & 0xFF000000) >> 24);  // block offset addr
            pdu[4] = (uint8)(((AIVS_OTA_FIRST_BLOCK_ADDR + aivs_ota_block_len) & 0x00FF0000) >> 16);  // block offset addr
            pdu[5] = (uint8)(((AIVS_OTA_FIRST_BLOCK_ADDR + aivs_ota_block_len) & 0x0000FF00) >> 8);   // block offset addr
            pdu[6] = (uint8)(((AIVS_OTA_FIRST_BLOCK_ADDR + aivs_ota_block_len) & 0x000000FF) >> 0);   // block offset addr
            pdu[7] = (uint8)((request_len & 0xFF00) >> 8);               // block length high byte
            pdu[8] = (uint8)((request_len & 0x00FF) >> 0);               // block length low byte
            pdu[9] = (uint8)((AIVS_OTA_BLOCK_DELAY & 0xFF00) >> 8);             // block delay time
            pdu[10] = (uint8)((AIVS_OTA_BLOCK_DELAY & 0x00FF) >> 0);            // block delay time
        }
        else
        {
            pdu[2] = 0x00;                                                    // 0: success, 1: fail
            memset((uint8*)(&pdu[3]), 0, 8);                                   // block offset addr, length, delay time set 0
        }

        aivs_ota_save_data(&aivs_rcsp_pdu->param_ptr[1], aivs_rcsp_pdu->param_len - 1);

        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);

    }
}

void aivs_ota_get_refresh_status_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[3];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    os_printf("aivs_ota_get_dev_refresh_status_resp\r\n");

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];                               //opcode_SN

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        pdu[2] = AIVS_OTA_RESULT_SUCCESS;
        aivs_ota_start_flag = 0;
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
    }
}

#endif //CFG_XIAOAI_AI_EN
