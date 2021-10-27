////////////////////////////////////////////////
//
//

#ifndef __BLE_APP_FUNC_H__
#define __BLE_APP_FUNC_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"


#define GAP_ADTYPE_FLAGS                        0x01 //!< Discovery Mode: @ref GAP_ADTYPE_FLAGS_MODES
#define GAP_ADTYPE_16BIT_MORE                   0x02 //!< Service: More 16-bit UUIDs available
#define GAP_ADTYPE_16BIT_COMPLETE               0x03 //!< Service: Complete list of 16-bit UUIDs
#define GAP_ADTYPE_32BIT_MORE                   0x04 //!< Service: More 32-bit UUIDs available
#define GAP_ADTYPE_32BIT_COMPLETE               0x05 //!< Service: Complete list of 32-bit UUIDs
#define GAP_ADTYPE_128BIT_MORE                  0x06 //!< Service: More 128-bit UUIDs available
#define GAP_ADTYPE_128BIT_COMPLETE              0x07 //!< Service: Complete list of 128-bit UUIDs
#define GAP_ADTYPE_LOCAL_NAME_SHORT             0x08 //!< Shortened local name
#define GAP_ADTYPE_LOCAL_NAME_COMPLETE          0x09 //!< Complete local name
#define GAP_ADTYPE_POWER_LEVEL                  0x0A //!< TX Power Level: 0xXX: -127 to +127 dBm
#define GAP_ADTYPE_OOB_CLASS_OF_DEVICE          0x0D //!< Simple Pairing OOB Tag: Class of device (3 octets)
#define GAP_ADTYPE_OOB_SIMPLE_PAIRING_HASHC     0x0E //!< Simple Pairing OOB Tag: Simple Pairing Hash C (16 octets)
#define GAP_ADTYPE_OOB_SIMPLE_PAIRING_RANDR     0x0F //!< Simple Pairing OOB Tag: Simple Pairing Randomizer R (16 octets)
#define GAP_ADTYPE_SM_TK                        0x10 //!< Security Manager TK Value
#define GAP_ADTYPE_SM_OOB_FLAG                  0x11 //!< Security Manager OOB Flags
#define GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE    0x12 //!< Min and Max values of the connection interval (2 octets Min, 2 octets Max) (0xFFFF indicates no conn interval min or max)
#define GAP_ADTYPE_SIGNED_DATA                  0x13 //!< Signed Data field
#define GAP_ADTYPE_SERVICES_LIST_16BIT          0x14 //!< Service Solicitation: list of 16-bit Service UUIDs
#define GAP_ADTYPE_SERVICES_LIST_128BIT         0x15 //!< Service Solicitation: list of 128-bit Service UUIDs
#define GAP_ADTYPE_SERVICE_DATA                 0x16 //!< Service Data - 16-bit UUID
#define GAP_ADTYPE_PUBLIC_TARGET_ADDR           0x17 //!< Public Target Address
#define GAP_ADTYPE_RANDOM_TARGET_ADDR           0x18 //!< Random Target Address
#define GAP_ADTYPE_APPEARANCE                   0x19 //!< Appearance
#define GAP_ADTYPE_ADV_INTERVAL                 0x1A //!< Advertising Interval
#define GAP_ADTYPE_LE_BD_ADDR                   0x1B //!< LE Bluetooth Device Address
#define GAP_ADTYPE_LE_ROLE                      0x1C //!< LE Role
#define GAP_ADTYPE_SIMPLE_PAIRING_HASHC_256     0x1D //!< Simple Pairing Hash C-256
#define GAP_ADTYPE_SIMPLE_PAIRING_RANDR_256     0x1E //!< Simple Pairing Randomizer R-256
#define GAP_ADTYPE_SERVICE_DATA_32BIT           0x20 //!< Service Data - 32-bit UUID
#define GAP_ADTYPE_SERVICE_DATA_128BIT          0x21 //!< Service Data - 128-bit UUID
#define GAP_ADTYPE_3D_INFO_DATA                 0x3D //!< 3D Information Data
#define GAP_ADTYPE_MANUFACTURER_SPECIFIC        0xFF //!< Manufacturer Specific Data: first 2 octets contain the Company Identifier Code followed by the additional manufacturer specific data
/** @} End GAP_ADTYPE_DEFINES */

extern BLE_APP_CONTEXT			g_playcontrol_app_context;
extern GATT_SERVER_PROFILE		g_playcontrol_profile;

void ParseCommandData(uint8_t* data, uint8_t data_len);

void ResponseCommand(uint16_t Cmd, uint8_t* Buf, uint8_t Len);

int8_t InitBlePlaycontrolProfile(void);

int8_t UninitBlePlaycontrolProfile(void);


#ifdef __cplusplus
}
#endif//__cplusplus

#endif/*__BLE_APP_FUNC_H__*/

