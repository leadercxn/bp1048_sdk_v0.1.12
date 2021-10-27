
#include "type.h"
#include <string.h>
#include "ble_api.h"
#include "ble_app_func.h"
#include "bt_app_func.h"
#include "bt_manager.h"
#include "app_config.h"

#ifdef 	CFG_XIAOAI_AI_EN
	#include "aivs_rcsp.h"
	#include "xm_auth.h"
	#include "xm_xiaoai_api.h"
#endif

extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;

#ifdef CFG_FUNC_AI_EN
#include "ai.h"
#endif


#include "debug.h"
#if (BLE_SUPPORT == ENABLE)

#ifdef CFG_FUNC_AI_EN
#define DEFAULT_BLENAME   "BP10-BT0000"

const uint8_t advertisement_data[] = {
	0x02, 0x01, 0x02,		//flag:LE General Disconverable
	0x03, 0x03, 0x00, 0xab,	//16bit service UUIDs
	12,   0x09, 'B', 'P', '1', '0', '-','B','T','0','0','0','0',//BP10-BT
};
uint8_t gAdvertisementData[50]; //保存广播数据

const uint8_t profile_data[] =
{
    // 0x0001 PRIMARY_SERVICE-GAP_SERVICE
    0x0a, 0x00, 0x02, 0xf0, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,
    // 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME-READ
    0x0d, 0x00, 0x02, 0xf0, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE-GAP_DEVICE_NAME-READ-'Hifier-mic0201'
    // READ_ANYBODY
    0x16, 0x00, 0x02, 0xf0, 0x03, 0x00, 0x00, 0x2a, 0x48, 0x69, 0x66, 0x69, 0x65, 0x72, 0x2d, 0x6d, 0x69, 0x63, '0', '0', '0', '0',

    // 0x0004 PRIMARY_SERVICE-AB00
    0x0a, 0x00, 0x02, 0xf0, 0x04, 0x00, 0x00, 0x28, 0x00, 0xab,
    // 0x0005 CHARACTERISTIC-AB01-READ | WRITE | DYNAMIC
    0x0d, 0x00, 0x02, 0xf0, 0x05, 0x00, 0x03, 0x28, 0x0a, 0x06, 0x00, 0x01, 0xab,
    // 0x0006 VALUE-AB01-READ | WRITE | DYNAMIC-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x0a, 0xf1, 0x06, 0x00, 0x01, 0xab,
    // 0x0007 CHARACTERISTIC-AB02-NOTIFY | DYNAMIC
    0x0d, 0x00, 0x02, 0xf0, 0x07, 0x00, 0x03, 0x28, 0x10, 0x08, 0x00, 0x02, 0xab,
    // 0x0008 VALUE-AB02-NOTIFY | DYNAMIC-''
    //
    0x08, 0x00, 0x00, 0xf1, 0x08, 0x00, 0x02, 0xab,
    // 0x0009 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0xf1, 0x09, 0x00, 0x02, 0x29, 0x00, 0x00,
    // 0x000a CHARACTERISTIC-AB03-NOTIFY | DYNAMIC
    0x0d, 0x00, 0x02, 0xf0, 0x0a, 0x00, 0x03, 0x28, 0x10, 0x0b, 0x00, 0x03, 0xab,
    // 0x000b VALUE-AB03-NOTIFY | DYNAMIC-''
    //
    0x08, 0x00, 0x00, 0xf1, 0x0b, 0x00, 0x03, 0xab,
    // 0x000c CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0xf1, 0x0c, 0x00, 0x02, 0x29, 0x00, 0x00,

    // END
    0x00, 0x00,
}; // total size 87 bytes
#else

#ifdef CFG_XIAOAI_AI_EN

#define XIAOAI_ADV_LEN	31
const uint8_t advertisement_data[] = {
	0x02, 0x01, 0x1A,		//flag:LE General Disconverable
	0x1B, 0xFF, 0x8F, 0x03, 0x17, 0x01,
};

uint8_t gAdvertisementData[50]; //保存广播数据
uint8_t gResponseData[50]; //保存广播数据

char send_data[512];//发送CMD缓存
int send_size = 0;//发送数据的长度

uint8_t send_ok = 0;

uint8_t uuid_AF06_descritor[2];

uint8_t profile_data[] =
{
    // 0x0001 PRIMARY_SERVICE-GAP_SERVICE
    0x0a, 0x00, 0x02, 0xf0, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,
    // 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME-READ
    0x0d, 0x00, 0x02, 0xf0, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE-GAP_DEVICE_NAME-READ-'BP10-BLE'
    // READ_ANYBODY
    0x10, 0x00, 0x02, 0xf0, 0x03, 0x00, 0x00, 0x2a, 0x42, 0x50, 0x31, 0x30, 0x2d, 0x42, 0x4c, 0x45,

    // 0x0004 PRIMARY_SERVICE-AF00
    0x0a, 0x00, 0x02, 0xf0, 0x04, 0x00, 0x00, 0x28, 0x00, 0xaf,
    // 0x0005 CHARACTERISTIC-AF01-WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0xf0, 0x05, 0x00, 0x03, 0x28, 0x04, 0x06, 0x00, 0x01, 0xaf,
    // 0x0006 VALUE-AF01-WRITE_WITHOUT_RESPONSE | DYNAMIC-''
    // WRITE_ANYBODY
    0x08, 0x00, 0x04, 0xf1, 0x06, 0x00, 0x01, 0xaf,
    // 0x0007 CHARACTERISTIC-AF02-NOTIFY
    0x0d, 0x00, 0x02, 0xf0, 0x07, 0x00, 0x03, 0x28, 0x10, 0x08, 0x00, 0x02, 0xaf,
    // 0x0008 VALUE-AF02-NOTIFY-''
    //
    0x08, 0x00, 0x00, 0xf0, 0x08, 0x00, 0x02, 0xaf,
    // 0x0009 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0xf1, 0x09, 0x00, 0x02, 0x29, 0x00, 0x00,
    // 0x000a CHARACTERISTIC-AF03-WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0xf0, 0x0a, 0x00, 0x03, 0x28, 0x04, 0x0b, 0x00, 0x03, 0xaf,
    // 0x000b VALUE-AF03-WRITE_WITHOUT_RESPONSE | DYNAMIC-''
    // WRITE_ANYBODY
    0x08, 0x00, 0x04, 0xf1, 0x0b, 0x00, 0x03, 0xaf,
    // 0x000c CHARACTERISTIC-AF04-NOTIFY
    0x0d, 0x00, 0x02, 0xf0, 0x0c, 0x00, 0x03, 0x28, 0x10, 0x0d, 0x00, 0x04, 0xaf,
    // 0x000d VALUE-AF04-NOTIFY-''
    //
    0x08, 0x00, 0x00, 0xf0, 0x0d, 0x00, 0x04, 0xaf,
    // 0x000e CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0xf1, 0x0e, 0x00, 0x02, 0x29, 0x00, 0x00,
    // 0x000f CHARACTERISTIC-AF05-WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0xf0, 0x0f, 0x00, 0x03, 0x28, 0x04, 0x10, 0x00, 0x05, 0xaf,
    // 0x0010 VALUE-AF05-WRITE_WITHOUT_RESPONSE | DYNAMIC-''
    // WRITE_ANYBODY
    0x08, 0x00, 0x04, 0xf1, 0x10, 0x00, 0x05, 0xaf,
    // 0x0011 CHARACTERISTIC-AF06-NOTIFY
    0x0d, 0x00, 0x02, 0xf0, 0x11, 0x00, 0x03, 0x28, 0x10, 0x12, 0x00, 0x06, 0xaf,
    // 0x0012 VALUE-AF06-NOTIFY-''
    //
    0x08, 0x00, 0x00, 0xf0, 0x12, 0x00, 0x06, 0xaf,
    // 0x0013 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0xf1, 0x13, 0x00, 0x02, 0x29, 0x00, 0x00,

    // END
    0x00, 0x00,
}; // total size 113 bytes


#else
const uint8_t advertisement_data[] = {
	0x02, 0x01, 0x02,		//flag:LE General Discoverable
	0x03, 0x03, 0x00, 0xab,	//16bit service UUIDs
//	9,   0x09, 'B', 'P', '1', '0', '-','B','L','E',//
};

uint8_t gAdvertisementData[50]; //保存广播数据
uint8_t gResponseData[50]; //保存广播数据
	
const uint8_t profile_data[] =
{
	// 0x0001 PRIMARY_SERVICE-GAP_SERVICE
	0x0a, 0x00, 0x02, 0xf0, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,
	// 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME-READ | WRITE | DYNAMIC
	0x0d, 0x00, 0x02, 0xf0, 0x02, 0x00, 0x03, 0x28, 0x0a, 0x03, 0x00, 0x00, 0x2a,
	// 0x0003 VALUE-GAP_DEVICE_NAME-READ | WRITE | DYNAMIC-'BP10-BLE'
	// READ_ANYBODY, WRITE_ANYBODY
	0x10, 0x00, 0x0a, 0xf1, 0x03, 0x00, 0x00, 0x2a, 0x42, 0x50, 0x31, 0x30, 0x2d, 0x42, 0x4c, 0x45,

	// 0x0004 PRIMARY_SERVICE-AB00
	0x0a, 0x00, 0x02, 0xf0, 0x04, 0x00, 0x00, 0x28, 0x00, 0xab,
	// 0x0005 CHARACTERISTIC-AB01-READ | WRITE | WRITE_WITHOUT_RESPONSE | DYNAMIC
	0x0d, 0x00, 0x02, 0xf0, 0x05, 0x00, 0x03, 0x28, 0x0e, 0x06, 0x00, 0x01, 0xab,
	// 0x0006 VALUE-AB01-READ | WRITE | WRITE_WITHOUT_RESPONSE | DYNAMIC-''
	// READ_ANYBODY, WRITE_ANYBODY
	0x08, 0x00, 0x0e, 0xf1, 0x06, 0x00, 0x01, 0xab,
	// 0x0007 CHARACTERISTIC-AB02-NOTIFY | DYNAMIC
	0x0d, 0x00, 0x02, 0xf0, 0x07, 0x00, 0x03, 0x28, 0x10, 0x08, 0x00, 0x02, 0xab,
	// 0x0008 VALUE-AB02-NOTIFY | DYNAMIC-''
	//
	0x08, 0x00, 0x00, 0xf1, 0x08, 0x00, 0x02, 0xab,
	// 0x0009 CLIENT_CHARACTERISTIC_CONFIGURATION
	// READ_ANYBODY, WRITE_ANYBODY
	0x0a, 0x00, 0x0e, 0xf1, 0x09, 0x00, 0x02, 0x29, 0x00, 0x00,
	// 0x000a CHARACTERISTIC-AB03-NOTIFY | DYNAMIC
	0x0d, 0x00, 0x02, 0xf0, 0x0a, 0x00, 0x03, 0x28, 0x10, 0x0b, 0x00, 0x03, 0xab,
	// 0x000b VALUE-AB03-NOTIFY | DYNAMIC-''
	//
	0x08, 0x00, 0x00, 0xf1, 0x0b, 0x00, 0x03, 0xab,
	// 0x000c CLIENT_CHARACTERISTIC_CONFIGURATION
	// READ_ANYBODY, WRITE_ANYBODY
	0x0a, 0x00, 0x0e, 0xf1, 0x0c, 0x00, 0x02, 0x29, 0x00, 0x00,

	// END
	0x00, 0x00,
}; // total size 74 bytes
#endif

#endif

//
// list service handle ranges
//
#define ATT_SERVICE_GAP_SERVICE_START_HANDLE 0x0001
#define ATT_SERVICE_GAP_SERVICE_END_HANDLE 0x0003
#define ATT_SERVICE_AF00_START_HANDLE 0x0004
#define ATT_SERVICE_AF00_END_HANDLE 0x0013

//
// list mapping between characteristics and handles
//
#define ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_AF01_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_AF02_01_VALUE_HANDLE 0x0008
#define ATT_CHARACTERISTIC_AF02_01_CLIENT_CONFIGURATION_HANDLE 0x0009
#define ATT_CHARACTERISTIC_AF03_01_VALUE_HANDLE 0x000b
#define ATT_CHARACTERISTIC_AF04_01_VALUE_HANDLE 0x000d
#define ATT_CHARACTERISTIC_AF04_01_CLIENT_CONFIGURATION_HANDLE 0x000e
#define ATT_CHARACTERISTIC_AF05_01_VALUE_HANDLE 0x0010
#define ATT_CHARACTERISTIC_AF06_01_VALUE_HANDLE 0x0012
#define ATT_CHARACTERISTIC_AF06_01_CLIENT_CONFIGURATION_HANDLE 0x0013

//
// list service handle ranges
//
//#define ATT_SERVICE_GAP_SERVICE_START_HANDLE 0x0001
//#define ATT_SERVICE_GAP_SERVICE_END_HANDLE 0x0003
#define ATT_SERVICE_AB00_START_HANDLE 0x0004
#define ATT_SERVICE_AB00_END_HANDLE 0x000e

//
// list mapping between characteristics and handles
//
#define ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE 0x0008
#define ATT_CHARACTERISTIC_AB02_01_CLIENT_CONFIGURATION_HANDLE 0x0009
#define ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE 0x000b
#define ATT_CHARACTERISTIC_AB03_01_CLIENT_CONFIGURATION_HANDLE 0x000c
#define ATT_CHARACTERISTIC_AB04_01_VALUE_HANDLE 0x000e


BLE_APP_CONTEXT			g_playcontrol_app_context;
GATT_SERVER_PROFILE		g_playcontrol_profile;
GAP_MODE				g_gap_mode;

#ifdef CFG_FUNC_AI_EN
uint8_t * g_profile_data = NULL;
uint8_t *g_advertisement_data  = NULL;
uint8_t g_advertisement_data_len=0;
#endif

int16_t app_att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
int16_t app_att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int16_t att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
int16_t att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int16_t gap_att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);



#ifdef CFG_FUNC_AI_EN
void ai_ble_set(char *name,uint8_t *bt_addr,uint8_t *ble_addr)
{
	char temp[6];
	uint32_t profile_data_len =0;
	uint32_t new_name_len = strlen(name) + 4;

	if(strlen(name) >= 10)
	{
//		g_advertisement_data_len = sizeof(advertisement_data) + (strlen(name) - 10);
		profile_data_len = sizeof(profile_data) + (strlen(name) - 10);
	}
	else
	{
//		g_advertisement_data_len = sizeof(advertisement_data) - (10 - strlen(name));
		profile_data_len = sizeof(profile_data) - (10 - strlen(name));
	}

	g_advertisement_data_len = sizeof(advertisement_data) - strlen(DEFAULT_BLENAME) + new_name_len;//这里赋值注意struct advertisement_data信息信息中的BLE名称。

	g_advertisement_data = (uint8_t*)osPortMalloc(g_advertisement_data_len);
	g_profile_data = (uint8_t*)osPortMalloc(profile_data_len);

	memset(g_advertisement_data,0,g_advertisement_data_len);
	memset(g_profile_data,0,profile_data_len);

	//g_advertisement_data
	uint8_t *p = (uint8_t *)advertisement_data;
	uint8_t len = 0;
	uint8_t len1 = 0;
	uint8_t offset = 0;
	uint8_t offset1 = 0;
	while(1)
	{
		len = p[0]+1;
		if(p[1] == 0x09)
		{
			g_advertisement_data[offset] = 0;
			g_advertisement_data[offset + 1] = 0x09;
			memcpy(g_advertisement_data + offset + 2,name,strlen(name));
			sprintf(temp,"%02X%02X",bt_addr[1],bt_addr[0]);
			memcpy(g_advertisement_data + offset + 2 + strlen(name),temp,strlen(temp));
			g_advertisement_data[offset] = 2 + strlen(name)-1 + 4;
			break;
		}
		else
		{
			memcpy(g_advertisement_data+offset,p,len);
			offset += len;
			p += len;
		}
		if(offset >  sizeof(advertisement_data))
		{
			printf("advertisement_data error    \n");
			while(1);
		}
	}

	p = (uint8_t *)profile_data;
	offset = 0;
	offset1 = 0;
	while(1)
	{
		len = p[0];
		if(len == 0)
		{
			break;
		}
		memcpy(g_profile_data+offset1,p,len);//取出当前的条目 copy到g_profile_data
		if((p[6] == 0x00) && (p[7] == 0x2A))
		{
			memcpy(g_profile_data + offset1 + 8,name,strlen(name));
			sprintf(temp,"%02X%02X",bt_addr[1],bt_addr[0]);
			memcpy(g_profile_data + offset1 + 8 + strlen(name),temp,strlen(temp));
			g_profile_data[offset1] = 8 + strlen(name) + 4;
			len1 = g_profile_data[offset];
		}
		else
		{
			len1 = len;
		}
		//g_profile_data
		offset1 += len1;

		//profile_data  p
		offset += len;
		p += len;
	}
}
#endif


int8_t InitBlePlaycontrolProfile(void)
{
	//register ble callback funciton
	BleAppCallBackRegister(BLEStackCallBackFunc);

	memcpy(g_playcontrol_app_context.ble_device_addr, btStackConfigParams->ble_LocalDeviceAddr, 6);
	g_playcontrol_app_context.ble_device_role = PERIPHERAL_DEVICE;

#ifdef CFG_FUNC_AI_EN
	g_playcontrol_profile.profile_data 	= g_profile_data;
#else
	g_playcontrol_profile.profile_data 	= (uint8_t *)profile_data;//g_profile_data;
#endif
	g_playcontrol_profile.attr_read		= att_read;
	g_playcontrol_profile.attr_write	= att_write;

	// set advertising interval params
	SetAdvIntervalParams(0x0020, 0x0100);

	// set gap mode
	g_gap_mode.broadcase_mode		= NON_BROADCAST_MODE;
	g_gap_mode.discoverable_mode	= GENERAL_DISCOVERABLE_MODE;
	g_gap_mode.connectable_mode		= UNDIRECTED_CONNECTABLE_MODE;
	g_gap_mode.bondable_mode		= NON_BONDABLE_MODE;
	SetGapMode(g_gap_mode);
#ifdef CFG_FUNC_AI_EN
	SetAdvertisingData((uint8_t *)g_advertisement_data, g_advertisement_data_len);
#else

#ifdef CFG_XIAOAI_AI_EN
	{
		uint8_t offset = 0;
		uint8_t adv_len = 0;
		offset = adv_len = sizeof(advertisement_data);
		memcpy(&gAdvertisementData[0], (uint8_t *)advertisement_data, sizeof(advertisement_data));
		{
			//uint8_t* bt_addr_p = btStackConfigParams->bt_LocalDeviceAddr;
			uint8_t* bt_addr_p = btManager.btDevAddr;
			uint8_t adv_bit_set = 0;

			adv_bit_set = 0x20;//
			memset(&gAdvertisementData[offset], adv_bit_set, 1);
			offset++;

			adv_bit_set = 0xf0;//set earphone statue.
			memset(&gAdvertisementData[offset], adv_bit_set, 1);
			offset++;

			adv_bit_set = 0x02;//set BR\EDR statue.
			memset(&gAdvertisementData[offset], adv_bit_set, 1);
			offset++;

			adv_bit_set = 0x01;//set ear phone statue.
			memset(&gAdvertisementData[offset], adv_bit_set, 1);
			offset++;

			adv_bit_set = 0x90;//set ear phone statue.
			memset(&gAdvertisementData[offset], adv_bit_set, 1);
			offset++;

			adv_bit_set = 0x00;//set ear phone statue.
			memset(&gAdvertisementData[offset], adv_bit_set, 1);
			offset++;

			{
				memcpy(&gAdvertisementData[offset], bt_addr_p, 6);
				offset += 6;
			}
			{
				memcpy(&gAdvertisementData[offset], bt_addr_p, 3);
				offset += 3;
			}

			adv_bit_set = 0xFD;//set ear phone statue.
			memset(&gAdvertisementData[offset], adv_bit_set, 1);
			offset++;

			{
				memset(&gAdvertisementData[offset], 0, 6);
				offset += 6;
			}

		}


		adv_len = XIAOAI_ADV_LEN;

		{
			int i = 0;
			printf("data:");
			for(i=0;i<adv_len;i++)
			{
				printf("%02x ",gAdvertisementData[i]);
			}
			printf("\r\n");
		}

		SetAdvertisingData((uint8_t *)gAdvertisementData, adv_len);
	}


	//set adv rsp data
	{
			uint8_t offset = 0;
			uint8_t adv_len = 0;
			uint8_t* ble_addr_p = btStackConfigParams->ble_LocalDeviceAddr;

			memset(&gResponseData[0], 0, sizeof(gResponseData));
			{
				uint8_t rsp_bit_set = 0;
				uint8_t rsp_bits_set[10] ;

#if 1
				adv_len += 0x0D + 1;

				rsp_bit_set = 0x0D;//
				memset(&gResponseData[offset], rsp_bit_set, 1);
				offset++;

				rsp_bit_set = 0xff;//set earphone statue.
				memset(&gResponseData[offset], rsp_bit_set, 1);
				offset++;

				{
					rsp_bits_set[0] = 0x01;
					rsp_bits_set[1] = 0x0B;
					memcpy(&gResponseData[offset], rsp_bits_set, 2);
					offset += 2;
				}

				rsp_bit_set = 0x08;//set BR\EDR statue.
				memset(&gResponseData[offset], rsp_bit_set, 1);
				offset++;

				rsp_bit_set = 0x03;//set ear phone statue.
				memset(&gResponseData[offset], rsp_bit_set, 1);
				offset++;

				rsp_bit_set = 0x01;//set ear phone statue.
				memset(&gResponseData[offset], rsp_bit_set, 1);
				offset++;

				{
					rsp_bits_set[0] = 0xAC;
					rsp_bits_set[1] = 0x01;
//					memcpy(&gResponseData[offset], ble_addr_p, 2);
					offset += 2;
				}

				{
//					memset(&gResponseData[offset], 0xff , 4);
					memcpy(&gResponseData[offset], ble_addr_p + 2 , 4);
					offset += 4;
				}

				rsp_bit_set = 0;//set ear phone statue.
				memset(&gResponseData[offset], rsp_bit_set, 1);
				offset++;
#endif


				{

					rsp_bit_set = strlen(BT_NAME)+1;//set ear phone statue.
					adv_len += rsp_bit_set + 1;
					memset(&gResponseData[offset], rsp_bit_set, 1);
					offset++;

					rsp_bit_set = 0x09;//set ear phone statue.
					memset(&gResponseData[offset], rsp_bit_set, 1);
					offset++;

					memcpy(&gResponseData[offset], BT_NAME, strlen(BT_NAME));
					offset++;
				}

			}

			{
				int i = 0;
				printf("data:");
				for(i=0;i<adv_len;i++)
				{
					printf("%02x ",gResponseData[i]);
				}
				printf("\r\n");
			}

			SetScanResponseData(gResponseData,adv_len);

		}
#else
	//SetAdvertisingData((uint8_t *)advertisement_data, sizeof(advertisement_data));
	{
		uint8_t offset = 0;
		uint8_t adv_len = 0;

#if 1	
		//Advertising Data 长度少于31bytes
		offset = adv_len = sizeof(advertisement_data);
		memcpy(&gAdvertisementData[0], (uint8_t *)advertisement_data, adv_len);

		gAdvertisementData[offset] = (strlen(btStackConfigParams->ble_LocalDeviceName)+1);
		gAdvertisementData[offset+1] = 0x09;
		memcpy(&gAdvertisementData[offset+2], btStackConfigParams->ble_LocalDeviceName, strlen(btStackConfigParams->ble_LocalDeviceName));
		adv_len += (2+strlen(btStackConfigParams->ble_LocalDeviceName));
		
		SetAdvertisingData((uint8_t *)gAdvertisementData, adv_len);
#else	
		//Advertising Data 长度大于31bytes,需要同时使用AdvertisementData和ResponseData
		//参考代码如下:
		//device type
		gAdvertisementData[offset] = 0x02;
		gAdvertisementData[offset+1] = 0x01;
		gAdvertisementData[offset+2] = 0x02;
		offset += 3;

		//Manufacture Specific Data
		gAdvertisementData[offset] = 10;
		gAdvertisementData[offset+1] = 0xff;
		gAdvertisementData[offset+2] = 0xd9;
		gAdvertisementData[offset+3] = 0x06;
		gAdvertisementData[offset+4] = 0x03;
		memcpy(&gAdvertisementData[offset+5], btStackConfigParams->bt_LocalDeviceAddr, 6);
		offset += 11;
		
		adv_len = offset;
		SetAdvertisingData((uint8_t *)gAdvertisementData, adv_len);

		//response data:用于补充广播数据超过31bytes
		offset=0;
		//device name
		gResponseData[offset] = (strlen(btStackConfigParams->ble_LocalDeviceName)+1);
		gResponseData[offset+1] = 0x09;
		memcpy(&gResponseData[offset+2], btStackConfigParams->ble_LocalDeviceName, strlen(btStackConfigParams->ble_LocalDeviceName));
		offset += (2+strlen(btStackConfigParams->ble_LocalDeviceName));

		//16bit service UUIDs
		gResponseData[offset] = 0x03;
		gResponseData[offset+1] = 0x03;
		gResponseData[offset+2] = 0x00;
		gResponseData[offset+3] = 0xab;
		offset += 4;
		
		adv_len = offset;
		SetScanResponseData((uint8_t *)gResponseData, adv_len);
#endif
	}
#endif

#endif
	return 0;
}


int8_t UninitBlePlaycontrolProfile(void)
{
#ifdef CFG_FUNC_AI_EN
	if(g_profile_data)
	{
		osPortFree(g_profile_data);
	}
	g_profile_data = NULL;
	if(g_advertisement_data)
	{
		osPortFree(g_advertisement_data);
	}
	g_advertisement_data = NULL;
#endif
	return 0;
}

int16_t att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    if( (attribute_handle >= ATT_SERVICE_GAP_SERVICE_START_HANDLE) && (attribute_handle <= ATT_SERVICE_GAP_SERVICE_END_HANDLE))
	{
    	//APP_DBG("att_read attribute_handle:%u\n",attribute_handle);
    	switch(attribute_handle)
		{
			case ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE:
				if(buffer)
				{
					memcpy(buffer, btStackConfigParams->ble_LocalDeviceName, strlen((char*)btStackConfigParams->ble_LocalDeviceName));
					return (int16_t)strlen((char*)btStackConfigParams->ble_LocalDeviceName);
				}
				return 0;
				
	        default:
	            return 0;
		}
	}
    else if( (attribute_handle >= ATT_SERVICE_AB00_START_HANDLE) && (attribute_handle <= ATT_SERVICE_AB00_END_HANDLE))
	{
    	return app_att_read(con_handle, attribute_handle, offset, buffer, buffer_size);
	}
#ifdef CFG_XIAOAI_AI_EN
    else if( (attribute_handle >= ATT_SERVICE_AF00_START_HANDLE) && (attribute_handle <= ATT_SERVICE_AF00_END_HANDLE))
	{
		return app_att_read(con_handle, attribute_handle, offset, buffer, buffer_size);
	}
#endif
	else
	{
		//未知句柄
	}

    return 0;
}

int16_t att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    if( (attribute_handle >= ATT_SERVICE_GAP_SERVICE_START_HANDLE) && (attribute_handle <= ATT_SERVICE_GAP_SERVICE_END_HANDLE))
	{
    	//APP_DBG("att_write attribute_handle:%u\n",attribute_handle);
    	switch(attribute_handle)
    	{
			case ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE:
				if((buffer)&&(buffer_size))
				{
					extern int32_t BtDeviceBleNameSet(uint8_t* deviceName, uint8_t deviceLen);
					//APP_DBG("name: %s\n", buffer);

					if(buffer_size > BT_NAME_SIZE)
						buffer_size = BT_NAME_SIZE;

					BtDeviceBleNameSet(buffer, buffer_size);

					{
						uint8_t offset = 0;
						uint8_t adv_len = 0;
						offset = adv_len = sizeof(advertisement_data);

						gAdvertisementData[offset] = (strlen((char*)btStackConfigParams->ble_LocalDeviceName)+1);
						gAdvertisementData[offset+1] = 0x09;
						memcpy(&gAdvertisementData[offset+2], btStackConfigParams->ble_LocalDeviceName, strlen((char*)btStackConfigParams->ble_LocalDeviceName));
						adv_len += (2+strlen((char*)btStackConfigParams->ble_LocalDeviceName));
						
						SetAdvertisingData((uint8_t *)gAdvertisementData, adv_len);
					}
				}
				return 0;
				
			default:
				return 0;
    	}
	}
#ifdef CFG_XIAOAI_AI_EN
    else if( (attribute_handle >= ATT_SERVICE_AF00_START_HANDLE) && (attribute_handle <= ATT_SERVICE_AF00_END_HANDLE))
	{
		return app_att_write(con_handle, attribute_handle, transaction_mode, offset, buffer, buffer_size);
	}
#else
    else if( (attribute_handle >= ATT_SERVICE_AB00_START_HANDLE) && (attribute_handle <= ATT_SERVICE_AB00_END_HANDLE))
	{
    	return app_att_write(con_handle, attribute_handle, transaction_mode, offset, buffer, buffer_size);
	}
#endif
	else
	{
		//未知句柄
	}
    return 0;
}


int16_t app_att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
//	APP_DBG("app_att_read for handle %02x,%d,%d,0x%08lx\n", attribute_handle,offset,buffer_size,buffer);
	switch(attribute_handle)
	{
		case ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:
			//APP_DBG("ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:\n");
			if(buffer == 0)//更新传输目标数据的长度
			{

			}
			else
			{

			}
			break;

		case ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:
			//APP_DBG("ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:\n");
			if(buffer == 0)//更新传输目标数据的长度
			{

			}
			else
			{

			}
			break;

		case ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:
			//APP_DBG("ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:\n");
			if(buffer == 0)//更新传输目标数据的长度
			{

			}
			else
			{

			}
			break;

		case ATT_CHARACTERISTIC_AF06_01_CLIENT_CONFIGURATION_HANDLE:
			if(buffer == 0)//更新传输目标数据的长度
			{

			}
			else
			{
#ifdef		CFG_XIAOAI_AI_EN
				buffer[0] = uuid_AF06_descritor[0];// listen notify
				buffer[1] = uuid_AF06_descritor[1];
#endif
				return 2;
			};
			break;

		default:
			return 0;
	}
	return 0;
}

int16_t app_att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
//    APP_DBG("app_att_write for handle %02x\n", attribute_handle);
	switch(attribute_handle)
	{
		case ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:
			//APP_DBG("ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:\n");
			#ifdef CFG_FUNC_AI_EN
			ble_rcv_data_proess(buffer,buffer_size);
			#endif
			break;

		case ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:
			//APP_DBG("ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:\n");
			break;

		case ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:
			//APP_DBG("ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:\n");
			break;
		case ATT_CHARACTERISTIC_AF05_01_VALUE_HANDLE:
#ifdef CFG_XIAOAI_AI_EN
			xiaoai_app_decode((uint8_t)buffer_size,buffer);
#endif

			break;

		case ATT_CHARACTERISTIC_AF06_01_CLIENT_CONFIGURATION_HANDLE:

			if(buffer_size == 2)
			{
#ifdef		CFG_XIAOAI_AI_EN
				uuid_AF06_descritor[0] = buffer[0];// listen notify
				uuid_AF06_descritor[1] = buffer[1];
#endif
			}

			break;

		default:
			return 0;
	}
	return 0;
}

#ifdef CFG_XIAOAI_AI_EN

void ble_set_data(char* set_send_data,int send_size_d)
{
	memcpy(send_data,set_send_data,send_size_d);
	send_size = send_size_d;
}

void set_send_size(int set_send_size)
{
	send_size = set_send_size;
}

int get_ble_send_size()
{
	return send_size;
}

char* get_send_data_p()
{
	return send_data;
}
int att_server_notify(uint16_t handle, uint8_t *value, uint16_t value_len);
void ble_send_data(uint8_t res_handle,uint8_t data_handle)
{
	if(send_size == 0)
	{
		return;
	}
	{
		if(att_server_notify((uint16_t)ATT_CHARACTERISTIC_AF06_01_VALUE_HANDLE,(uint8_t*)send_data,(uint16_t)send_size) == 0)
		{
//			{
//				int ii = 0;
//
//				T_DBG("ble[%d]:",send_size);
//				for(ii = 0;ii<send_size;ii++)
//				{
//					T_DBG("%02x ",send_data[ii]);
//				}
//				T_DBG("\r\n");
//			}
			send_size = 0;
		}
		return;
	}
}

int att_server_can_send(void);
void ai_ble_run_loop(void)
{
	if ((att_server_can_send() == 0))//||(att_server_resv_data_get()))
	{
		return ;
	}
	ble_send_data((uint8_t)ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE,(uint8_t)ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE);
}
#endif


#ifdef CFG_FUNC_AI_EN
extern int att_server_can_send(void);
void ai_ble_run_loop(void)
{
	if (att_server_can_send() == 0)
	{
		return ;
	}
	ble_send_data((uint8_t)ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE,(uint8_t)ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE);
}
#endif

#endif

