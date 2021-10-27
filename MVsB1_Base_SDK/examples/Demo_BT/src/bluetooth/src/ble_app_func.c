

#include "type.h"

#include "ble_api.h"
#include "ble_app_func.h"
#include "bt_app_func.h"
#include "bt_manager.h"
//#include "app_config.h"
extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;

#ifdef CFG_FUNC_AI
#include "ai.h"
#endif


#include "debug.h"
#if (BLE_SUPPORT == ENABLE)

const uint8_t advertisement_data[] = {
	0x02, 0x01, 0x02,		//flag:LE General Disconverable
	0x03, 0x03, 0x00, 0xab,	//16bit service UUIDs
	15,   0x09, 'H', 'i', 'f', 'i', 'e','r','-','m','i','c','0','0','0','0',//
};
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

//
// list service handle ranges
//
#define ATT_SERVICE_GAP_SERVICE_START_HANDLE 0x0001
#define ATT_SERVICE_GAP_SERVICE_END_HANDLE 0x0003
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

#ifdef CFG_FUNC_AI
uint8_t * g_profile_data = NULL;
uint8_t *g_advertisement_data  = NULL;
uint8_t g_advertisement_data_len=0;
#endif

int16_t app_att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
int16_t app_att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int16_t att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
int16_t att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int16_t gap_att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);



#ifdef CFG_FUNC_AI
void ai_ble_set(char *name,uint8_t *bt_addr,uint8_t *ble_addr)
{
	char temp[6];
	uint32_t profile_data_len =0;
	if(strlen(name) >= 10)
	{
		g_advertisement_data_len = sizeof(advertisement_data) + (strlen(name) - 10);
		profile_data_len = sizeof(profile_data) + (strlen(name) - 10);
	}
	else
	{
		g_advertisement_data_len = sizeof(advertisement_data) - (10 - strlen(name));
		profile_data_len = sizeof(profile_data) - (10 - strlen(name));
	}
	g_advertisement_data = osPortMalloc(g_advertisement_data_len);
	g_profile_data = osPortMalloc(profile_data_len);

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
	memcpy(g_playcontrol_app_context.ble_device_addr, btStackConfigParams->ble_LocalDeviceAddr, 6);
	g_playcontrol_app_context.ble_device_role = PERIPHERAL_DEVICE;

#ifdef CFG_FUNC_AI
	g_playcontrol_profile.profile_data 	= g_profile_data;
#else
	g_playcontrol_profile.profile_data 	= (uint8_t *)profile_data;//g_profile_data;
#endif
	g_playcontrol_profile.attr_read		= att_read;
	g_playcontrol_profile.attr_write	= att_write;

	// set gap mode
	g_gap_mode.broadcase_mode		= NON_BROADCAST_MODE;
	g_gap_mode.discoverable_mode	= GENERAL_DISCOVERABLE_MODE;
	g_gap_mode.connectable_mode		= UNDIRECTED_CONNECTABLE_MODE;
	g_gap_mode.bondable_mode		= NON_BONDABLE_MODE;
	SetGapMode(g_gap_mode);
#ifdef CFG_FUNC_AI
	SetAdvertisingData((uint8_t *)g_advertisement_data, g_advertisement_data_len);
#else
	SetAdvertisingData((uint8_t *)advertisement_data, sizeof(advertisement_data));
#endif
	return 0;
}


int8_t UninitBlePlaycontrolProfile(void)
{
#ifdef CFG_FUNC_AI
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
//	int att_value_len;
    if( (attribute_handle >= ATT_SERVICE_GAP_SERVICE_START_HANDLE) && (attribute_handle <= ATT_SERVICE_GAP_SERVICE_END_HANDLE))
	{
    	printf("att_read attribute_handle:%u\n",attribute_handle);
    	switch(attribute_handle)
		{
//	        case ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_CLIENT_CONFIGURATION_HANDLE:
//	            att_value_len = strlen((const char*)gap_device_name);
//	            if (buffer)
//	            {
//	                memcpy(buffer, gap_device_name, att_value_len);
//	            }
//	            return att_value_len;

	        default:
	            return 0;
		}
	}
    else if( (attribute_handle >= ATT_SERVICE_AB00_START_HANDLE) && (attribute_handle <= ATT_SERVICE_AB00_END_HANDLE))
	{
    	return app_att_read(con_handle, attribute_handle, offset, buffer, buffer_size);
	}
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
    	printf("att_write attribute_handle:%u\n",attribute_handle);
    	switch(attribute_handle)
    	{
//			case ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_CLIENT_CONFIGURATION_HANDLE:
//				if(buffer_size >= sizeof(gap_device_name))
//				{
//					buffer_size = sizeof(gap_device_name);
//				}
//				memcpy(gap_device_name,buffer+offset,buffer_size);
//				return buffer_size;

			default:
				return 0;
    	}
	}
    else if( (attribute_handle >= ATT_SERVICE_AB00_START_HANDLE) && (attribute_handle <= ATT_SERVICE_AB00_END_HANDLE))
	{
    	return app_att_write(con_handle, attribute_handle, transaction_mode, offset, buffer, buffer_size);
	}
	else
	{
		//未知句柄
	}
    return 0;
}


int16_t app_att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
	BT_DBG("app_att_read for handle %02x\n", attribute_handle);
	switch(attribute_handle)
	{
		case ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:
			BT_DBG("ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:\n");
			if(buffer == 0)//更新传输目标数据的长度
			{

			}
			else
			{

			}
			break;

		case ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:
			BT_DBG("ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:\n");
			if(buffer == 0)//更新传输目标数据的长度
			{

			}
			else
			{

			}
			break;

		case ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:
			BT_DBG("ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:\n");
			if(buffer == 0)//更新传输目标数据的长度
			{

			}
			else
			{

			}
			break;

		default:
			return 0;
	}
	return 0;
}

int16_t app_att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    BT_DBG("app_att_write for handle %02x\n", attribute_handle);
	switch(attribute_handle)
	{
		case ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:
			BT_DBG("ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:\n");
			#ifdef CFG_FUNC_AI
			ble_rcv_data_proess(buffer,buffer_size);
			#endif
			break;

		case ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:
			BT_DBG("ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:\n");
			break;

		case ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:
			BT_DBG("ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:\n");
			break;

		default:
			return 0;
	}
	return 0;
}

#ifdef CFG_FUNC_AI
extern int att_server_can_send(void);
void ai_ble_run_loop(void)
{
	if (att_server_can_send() == 0)
	{
		return ;
	}
	ble_send_data(ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE,ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE);
}
#endif

#endif

