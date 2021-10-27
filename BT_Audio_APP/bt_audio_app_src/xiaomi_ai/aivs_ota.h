

#ifndef XIAOMI_OTA_H_
#define XIAOMI_OTA_H_

#include "aivs_rcsp.h"

void aivs_ota_erase_flash(void);
void aivs_ota_write_flash(void);
uint8 aivs_ota_is_ongoing(void);
//void aivs_ota_get_info_offset_handler(XM_RCSP* aivs_rcsp_pdu);
//void aivs_ota_inquiry_if_can_update_handler(XM_RCSP* aivs_rcsp_pdu);
//void aivs_ota_enter_update_mode_handler(XM_RCSP* aivs_rcsp_pdu);
//void aivs_ota_exit_update_mode_handler(XM_RCSP* aivs_rcsp_pdu);
//void aivs_ota_send_update_block_handler(XM_RCSP* aivs_rcsp_pdu);
//void aivs_ota_get_refresh_status_handler(XM_RCSP* aivs_rcsp_pdu);


#endif /* XIAOMI_OTA_H_ */
