////////////////////////////////////////////////
//
//
#include "debug.h"

#include "ble_api.h"
#include "ble_app_func.h"

#if (BLE_SUPPORT == ENABLE)

uint8_t BleConnectFlag=0;
void BLEStackCallBackFunc(uint8_t event)
{
	switch(event)
	{
		case BLE_STACK_INIT_OK:
			BT_DBG("BLE_STACK_INIT_OK\n");
			BleConnectFlag = 0;
			break;
		case BLE_STACK_CONNECTED:
			BT_DBG("BLE_STACK_CONNECTED\n");
			BleConnectFlag = 1;
			break;
			
		case BLE_STACK_DISCONNECTED:
			BT_DBG("BLE_STACK_DISCONNECTED\n");
			BleConnectFlag = 0;
			break;

		case GATT_SERVER_INDICATION_TIMEOUT:
			BT_DBG("GATT_SERVER_INDICATION_TIMEOUT\n");
			break;

		case GATT_SERVER_INDICATION_COMPLETE:
			BT_DBG("GATT_SERVER_INDICATION_COMPLETE\n");
			break;

		default:
			break;

	}
}

#endif

