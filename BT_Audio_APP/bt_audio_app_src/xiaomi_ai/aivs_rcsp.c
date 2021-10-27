
#include "aivs_rcsp.h"
#include "aivs_ota.h"
#include "xm_auth.h"
#include "aivs_encode.h"

//MV slicon sdk
#include <nds32_intrinsic.h>
#include "bt_manager.h"
#include "main_task.h"
#include "debug.h"
#include "mv_fifo_api.h"
#include "spp_app_proc.h"
#include "xm_xiaoai_api.h"
#include "ai_service.h"

#ifdef CFG_XIAOAI_AI_EN

#define OPUS_5FRAMESIZEADDHEAD_SIZE	(63*5 + 6)	//5 frame data + Xiaoai header = spp send len.
#define OPUS_1FRAMESIZEADDHEAD_SIZE	(63)		//1 frame opus data = ble send data.
												//有些iPhone MTU 185  有些250
uint8 get_communicate_way(void);
uint spp_send(char* param, int param_len);
uint ble_send(char* param, int param_len);
uint8_t pcm_buffer_init();
void pcm_buffer_deinit();

osMutexId AIENMutex = NULL;

uint8_t set_way_flag = CURRENT_COMMUNICATE_BLE;
unsigned char spp_senddata[512];

void Lock_AIEncoding_init()
{
	if(AIENMutex == NULL)
		AIENMutex = osMutexCreate();
}
void Lock_AIEncoding()
{
	osMutexLock(AIENMutex);
}
void Unlock_AIEncoding()
{
	osMutexUnlock(AIENMutex);
}

void set_communicate_way(uint8 set_flag)
{
	set_way_flag = set_flag;
}
uint8 get_communicate_way(void)
{
	return set_way_flag;
}
uint ble_send(char* param, int param_len)
{

	ble_set_data(param,param_len);
	return param_len;
}

uint spp_send(char* param, int param_len)
{
	uint ret_val;
	memcpy(spp_senddata,param,param_len);
	ret_val = (uint)SppSendData(spp_senddata,param_len);
	return ret_val;
}


// aivs rcsp frame header
#define AIVS_RCSP_START           0xFEDCBA
#define AIVS_RCSP_END             0xEF

// aivs rcsp response flag
#define AIVS_RCSP_RESP            1
#define AIVS_RCSP_NO_RESP         0

// aivs 16bit UUID
#define AIVS_RCSP_SERVICE_UUID    0xAF00
#define AIVS_PAIR_RX_UUID         0xAF01
#define AIVS_PAIR_TX_UUID         0xAF02
#define AIVS_AUDIO_RX_UUID        0xAF03
#define AIVS_AUDIO_TX_UUID        0xAF04
//强制设备支持的
#define AIVS_RCSP_RX_UUID         0xAF05
#define AIVS_RCSP_TX_UUID         0xAF06

// aivs 128bit UUID
#define AIVS_128_UUID_ENABLE
// aivs 128bit UUID format: 0000XXXX-0000-1000-8000-00805F9B34FB
#define AIVS_UUID_128(uuid)  0xFB,0x34,0x9B,0x5F,0x80,0x00,0x00,0x80,0x00,0x10,0x00,0x00,(uuid & 0xFF),((uuid >> 8) & 0xFF),0x00,0x00

//static uint8 aivs_rscp_service_128_uuid[16] = {AIVS_UUID_128(AIVS_RCSP_SERVICE_UUID)};
//static uint8 aivs_pair_rx_128_uuid[16]      = {AIVS_UUID_128(AIVS_PAIR_RX_UUID)};
//static uint8 aivs_pair_tx_128_uuid[16]      = {AIVS_UUID_128(AIVS_PAIR_TX_UUID)};
//static uint8 aivs_audio_rx_128_uuid[16]     = {AIVS_UUID_128(AIVS_AUDIO_RX_UUID)};
//static uint8 aivs_audio_tx_128_uuid[16]     = {AIVS_UUID_128(AIVS_AUDIO_TX_UUID)};
//static uint8 aivs_rcsp_rx_128_uuid[16]      = {AIVS_UUID_128(AIVS_RCSP_RX_UUID)};
//static uint8 aivs_rcsp_tx_128_uuid[16]     = {AIVS_UUID_128(AIVS_RCSP_TX_UUID)};
//
//// Place holders for the GATT Server App to be able to lookup handles.
//static uint8 aivs_pair_rx_charval[25];
//static uint8 aivs_pair_tx_charval[25];
//static uint8 aivs_audio_rx_charval[25];
//static uint8 aivs_audio_tx_charval[25];
//static uint8 aivs_rcsp_rx_charval[255];
//static uint8 aivs_rcsp_tx_charval[100];
//
//// aivs_rscp Characteristic user configuration
//static uint8 aivs_pair_tx_config[2] = {0x00, 0x00};
//static uint8 aivs_audio_tx_config[2] = {0x00, 0x00};
//static uint8 aivs_rcsp_tx_config[2] = {0x00, 0x00};

// aivs_rscp opcode_SN, 0~255
static uint8 aivs_rcsp_opcodesn = 0;

// 是否处于解码模式
uint8 aivs_speech_start_flag = 0;//用于进程自锁

// 是否连接成功
uint8 aivs_speech_con_flag = 0;

typedef enum
{
    /***protocol***/
    AIVS_OPCODE_DATA_TRANSFER                 = 0x01, //data transfor
    AIVS_OPCODE_GET_DEV_INFO                  = 0x02, //getdeviceinfor
    AIVS_OPCODE_DEV_REBOOT                    = 0x03, //reboot
    AIVS_OPCODE_NOTIFY_PHONE_INFO             = 0x04, //mobilephone information
    AIVS_OPCODE_SET_PRO_MTU                   = 0x05, //MTU information
    AIVS_OPCODE_A2F_DISCONN_EDR               = 0x06, //APP notify device disconnect BT
    AIVS_OPCODE_F2A_EDR_STAT                  = 0x07, //device notify APP BT connection status
    AIVS_OPCODE_SET_DEV_INFO                  = 0x08, //APP set device information
    AIVS_OPCODE_GET_DEV_PRIVATE_INFO          = 0x09, //APP get device private information
    AIVS_OPCODE_NOTIFY_COMM_WAY               = 0x0A, //APP notify device communication way
    AIVS_OPCODE_WAKEUP_CLASSIC_BT             = 0x0B, //APP wakeup device's BT
    AIVS_OPCODE_NOTIFY_PHONE_VIRTUAL_ADDR     = 0x0C, //APP notify device phone's virtual address
    AIVS_OPCODE_NOTIFY_F2A_BT_OP              = 0x0D, //device notify APP BT operation
	AIVS_OPCODE_NOTIFY_UNBOUND				  = 0x0F, //delete device

    /***speech***/
    AIVS_OPCODE_SPEECH_START                  = 0xD0, //speech start
    AIVS_OPCODE_SPEECH_STOP                   = 0xD1, //speech stop
    AIVS_OPCODE_SPEECH_CANCEL                 = 0xD2, //speech cancel
    AIVS_OPCODE_SPEECH_LONG_HOLD              = 0xD3, //speech long hold

    /***OTA****/
    AIVS_OPCODE_OTA                           = 0xE0, //OTA
    AIVS_OPCODE_OTA_GET_OFFSET                = 0xE1, //get device update file information offset
    AIVS_OPCODE_OTA_IF_UPDATE                 = 0xE2, //inquire device if can update
    AIVS_OPCODE_OTA_ENTER_UPDATE              = 0xE3, //enter update mode
    AIVS_OPCODE_OTA_EXIT_UPDATE               = 0xE4, //exit update mode
    AIVS_OPCODE_OTA_SEND_BLOCK                = 0xE5, //send firmware update block
    AIVS_OPCODE_OTA_GET_STATUS                = 0xE6, //get device refresh firmware status
    AIVS_OPCODE_OTA_NOTIFY_UPDATE             = 0xE7, //notify update mode

    /***reserved***/
    AIVS_OPCODE_VENDOR_SPEC_RESERVED          = 0xF0, //vendor specific cmd reserved
}aivs_rcsp_opcode;

typedef enum
{
    AIVS_ATTR_TYPE_NAME                   = (0x01 << 0),
    AIVS_ATTR_TYPE_VERSION                = (0x01 << 1),
    AIVS_ATTR_TYPE_BATTERY                = (0x01 << 2),
    AIVS_ATTR_TYPE_VID_AND_PID            = (0x01 << 3),
    AIVS_ATTR_TYPE_EDR_CONNECTION_ON_STAT = (0x01 << 4),
    AIVS_ATTR_TYPE_FW_RUN_TYPE            = (0x01 << 5),
    AIVS_ATTR_TYPE_UBOOT_VERSION          = (0x01 << 6),
    AIVS_ATTR_TYPE_MULT_BATTERY           = (0x01 << 7),
    AIVS_ATTR_TYPE_CODEC_TYPE             = (0x01 << 8),
}aivs_attr_type;

typedef enum
{
    AIVS_PRIV_ATTR_BT_MAC                 = (0x01 << 0),
    AIVS_PRIV_ATTR_BLE_MAC                = (0x01 << 1),
    AIVS_PRIV_ATTR_MAX_MTU                = (0x01 << 2),
    AIVS_PRIV_ATTR_BT_STAT                = (0x01 << 3),
    AIVS_PRIV_ATTR_POWER_MODE             = (0x01 << 4),
}aivs_private_attr_type;
/*********************************XM SERVICE END******************************/


/*********************************XM RCSP BEGIN*******************************/
void aivs_rcsp_pdu(u_int16 param_len, u_int8 *param_ptr)
{
    if(CURRENT_COMMUNICATE_SPP == get_communicate_way())
    {
        spp_send((char*)param_ptr, param_len);
    }
    else
    {
        ble_send((char*)param_ptr, param_len);
    }
}

void aivs_speeth_rcsp_cmd(bool_t req_resp, u_int8 opcode, u_int16 param_len, u_int8 *param_ptr)
{
//    u_int8 pdu[100];

    // RCSP start code
	param_ptr[0] = (AIVS_RCSP_START & 0xFF0000) >> 16;
	param_ptr[1] = (AIVS_RCSP_START & 0x00FF00) >> 8;
	param_ptr[2] = (AIVS_RCSP_START & 0x0000FF) >> 0;

    // RCSP format
	param_ptr[3] = (0x01 << 7)|(((u_int8)req_resp) << 6);  //cmd_resp:1, no_need resp:req_resp
	param_ptr[4] = opcode;                                  //opcode
	param_ptr[5] = (uint8)((param_len & 0xFF00) >> 8);      //length
	param_ptr[6] = (uint8)((param_len & 0x00FF) >> 0);      //length

    // Data
//    os_memcpy(&param_ptr[7], param_ptr, param_len);

    // RCSP end code
	param_ptr[7 + param_len] = AIVS_RCSP_END;

    // Send pdu
    aivs_rcsp_pdu(param_len+8, param_ptr);
}
u_int8 pdu_t[100];
void aivs_rcsp_cmd(bool_t req_resp, u_int8 opcode, u_int16 param_len, u_int8 *param_ptr)
{
    u_int8 *pdu = pdu_t;

    // RCSP start code
    pdu[0] = (AIVS_RCSP_START & 0xFF0000) >> 16;
    pdu[1] = (AIVS_RCSP_START & 0x00FF00) >> 8;
    pdu[2] = (AIVS_RCSP_START & 0x0000FF) >> 0;

    // RCSP format
    pdu[3] = (0x01 << 7)|(((u_int8)req_resp) << 6);  //cmd_resp:1, no_need resp:req_resp
    pdu[4] = opcode;                                  //opcode
    pdu[5] = (uint8)((param_len & 0xFF00) >> 8);      //length
    pdu[6] = (uint8)((param_len & 0x00FF) >> 0);      //length

    // Data
    os_memcpy(&pdu[7], param_ptr, param_len);

    // RCSP end code
    pdu[7 + param_len] = AIVS_RCSP_END;

    // Send pdu
    aivs_rcsp_pdu(param_len+8, pdu);
}

void aivs_rcsp_cmd_resp(u_int8 opcode, u_int16 param_len, u_int8 *param_ptr)
{
    u_int8 pdu[50];

    // RCSP start code
    pdu[0] = (AIVS_RCSP_START & 0xFF0000) >> 16;
    pdu[1] = (AIVS_RCSP_START & 0x00FF00) >> 8;
    pdu[2] = (AIVS_RCSP_START & 0x0000FF) >> 0;

    // RCSP format
    pdu[3] = 0x00;                                    //cmd_resp:0, no_need resp:0
    pdu[4] = opcode;                                  //opcode
    pdu[5] = (uint8)((param_len & 0xFF00) >> 8);      //length
    pdu[6] = (uint8)((param_len & 0x00FF) >> 0);      //length

    // Data
    os_memcpy(&pdu[7], param_ptr, param_len);

    // RCSP end code
    pdu[7 + param_len] = AIVS_RCSP_END;

    // Send pdu
    aivs_rcsp_pdu(param_len+8, pdu);
}

void aivs_rcsp_data( bool_t req_resp, u_int16 param_len, u_int8 *param_ptr)
{
    u_int8 pdu[50];

    // RCSP start code
    pdu[0] = (AIVS_RCSP_START & 0xFF0000) >> 16;
    pdu[1] = (AIVS_RCSP_START & 0x00FF00) >> 8;
    pdu[2] = (AIVS_RCSP_START & 0x0000FF) >> 0;

    // RCSP format
    pdu[3] = (0x01 << 7)|(((u_int8)req_resp) << 6);   //cmd_resp:1, no_need resp:req_resp
    pdu[4] = AIVS_OPCODE_DATA_TRANSFER;                  //data_transfer,must be set 0x01
    pdu[5] = (uint8)((param_len & 0xFF00) >> 8);      //length
    pdu[6] = (uint8)((param_len & 0x00FF) >> 0);      //length

    // Data
    os_memcpy(&pdu[7], param_ptr, param_len);

    // RCSP end code
    pdu[7 + param_len] = AIVS_RCSP_END;

    // Send pdu
    aivs_rcsp_pdu(param_len+8, pdu);
}

void aivs_rcsp_data_resp(u_int16 param_len, u_int8 *param_ptr)
{
    u_int8 pdu[50];

    // RCSP start code
    pdu[0] = (AIVS_RCSP_START & 0xFF0000) >> 16;
    pdu[1] = (AIVS_RCSP_START & 0x00FF00) >> 8;
    pdu[2] = (AIVS_RCSP_START & 0x0000FF) >> 0;

    // RCSP format
    pdu[3] = 0x00;                                     //cmd_resp:0, no_need resp:0
    pdu[4] = AIVS_OPCODE_DATA_TRANSFER;                  //data_transfer,must be set 0x01
    pdu[5] = (uint8)((param_len & 0xFF00) >> 8);      //length
    pdu[6] = (uint8)((param_len & 0x00FF) >> 0);      //length

    // Data
    os_memcpy(&pdu[7], param_ptr, param_len);

    // RCSP end code
    pdu[7 + param_len] = AIVS_RCSP_END;

    // Send pdu
    aivs_rcsp_pdu(param_len+8, pdu);
}
/**********************************XM RCSP END*******************************/

/**********************************XM AUTH BEGIN*******************************/

void aivs_auth_process_handler(uint8 length,uint8 *pValue)
{
    uint8 auth_type = pValue[0];
    uint8 result[17] = {0};
    static uint8 auth_result[17];
    uint8 pass[5]={0x02,'p','a','s','s'};

    switch (auth_type)
    {
        case 0:  //random auth data
            if(length == (sizeof(result)/sizeof(result[0])))
            {
                if(get_encrypted_auth_data((unsigned char*)pValue, (unsigned char*)result) == AUTH_SUCCESS)
                {
                    aivs_rcsp_pdu(17, result);
                }
            }
            break;

        case 1:  //encrypted auth data
            if(length == (sizeof(auth_result)/sizeof(auth_result[0])))
            {
                uint8 i = 0;

                for(i = 0; i < (sizeof(auth_result)/sizeof(auth_result[0])); i++)
                {
                    if(pValue[i] != auth_result[i])
                       break;
                }

                if(i == (sizeof(auth_result)/sizeof(auth_result[0])))
                {
                    aivs_rcsp_pdu(sizeof(pass)/sizeof(pass[0]), pass);
                    xm_speech_con_set(1);
                    os_printf("aivs_auth_process PASS\r\n");
                }
                else
                {
                    aivs_rcsp_pdu(sizeof(auth_result)/sizeof(auth_result[0]), auth_result);
                    os_printf("aivs_auth_process ERROR!!!\r\n");
                }
            }
            break;

        case 2:  //pass
            if(length == (sizeof(pass)/sizeof(pass[0])))
            {
                if((pValue[1] == pass[1]) && (pValue[2] == pass[2]) && (pValue[3] == pass[3]) && (pValue[4] == pass[4]))
                {
                    if((get_random_auth_data((unsigned char*)result)== AUTH_SUCCESS) && (get_encrypted_auth_data((unsigned char*)result, (unsigned char*)auth_result) == AUTH_SUCCESS))
                    {
                        aivs_rcsp_pdu(sizeof(result)/sizeof(result[0]), result);
                    }
                }
            }
            break;

        default :
            break;
     }
}


void *xm_malloc(size_t __size)
{
	return pvPortMalloc(__size);
}

void  xm_free(void* free_p)
{
	vPortFree(free_p);
}

int xm_log(const char *format, ...)
{
	return 0;
}

int xm_rand(void)
{
	return (int)GetSysTick1MsCnt();
}

void xm_srand()
{

}
/***********************************XM AUTH END********************************/

/**********************************XM PAIR BEGIN*******************************/
void aivs_get_device_info_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 i = 0;
    uint8 pdu[50];
    uint16 length = 2;
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;
    uint32 dev_info_mask;
    os_memcpy(&dev_info_mask,aivs_rcsp_pdu->param_ptr+1,4);

    for(i = 0; i < 32; i++)
    {
        switch(dev_info_mask & (0x01 << i))
        {
            case AIVS_ATTR_TYPE_NAME:
                #if 1
                pdu[length++] = 0x03;   //length

                pdu[length++] = i;    //AIVS_ATTR_TYPE

                pdu[length++] = 0x42;   //device BT name: 'B'
                pdu[length++] = 0x4B;   //device BT name: 'K'
                #endif
                break;

            case AIVS_ATTR_TYPE_VERSION:
                pdu[length++] = 0x03;   //length

                pdu[length++] = i;    //AIVS_ATTR_TYPE

                pdu[length++] = (uint8)(AIVS_VERSION & 0xFF00) >> 8;   //version
                pdu[length++] = (uint8)(AIVS_VERSION & 0x00FF) >> 0;   //version
                break;

            case AIVS_ATTR_TYPE_BATTERY:
                #if 1
                pdu[length++] = 0x02;   //length

                pdu[length++] = i;    //AIVS_ATTR_TYPE

                pdu[length++] = 0x09;   //battery level 100%
                #endif
                break;

            case AIVS_ATTR_TYPE_VID_AND_PID:
                #if 1
                pdu[length++] = 0x05;   //length

                pdu[length++] = i;    //AIVS_ATTR_TYPE

                pdu[length++] = (uint8)(AIVS_VID & 0xFF00) >> 8;   //VID
                pdu[length++] = (uint8)(AIVS_VID & 0x00FF) >> 0;   //VID
                pdu[length++] = (uint8)(AIVS_PID & 0xFF00) >> 8;   //PID
                pdu[length++] = (uint8)(AIVS_PID & 0x00FF) >> 0;   //PID
                #endif
                break;

            case AIVS_ATTR_TYPE_EDR_CONNECTION_ON_STAT:
                #if 1
                pdu[length++] = 0x02;   //length

                pdu[length++] = i;    //AIVS_ATTR_TYPE

                pdu[length++] = 0x0;    //0:disconnect, 1:connected, 2:unpair
                #endif
                break;

            case AIVS_ATTR_TYPE_FW_RUN_TYPE:
                #if 1
                pdu[length++] = 0x02;   //length

                pdu[length++] = i;    //AIVS_ATTR_TYPE

                pdu[length++] = 0x0;    //0:normal system, 1:uboot system
                #endif
                break;

            case AIVS_ATTR_TYPE_UBOOT_VERSION:
                #if 1
                pdu[length++] = 0x05;   //length

                pdu[length++] = i;    //AIVS_ATTR_TYPE

                pdu[length++] = 0x01;    //uboot version
                pdu[length++] = 0x01;    //uboot version
                pdu[length++] = 0x01;    //uboot version
                pdu[length++] = 0x01;    //uboot version
                #endif
                break;

            case AIVS_ATTR_TYPE_MULT_BATTERY:
                #if 1
                pdu[length++] = 0x04;   //length

                pdu[length++] = i;    //AIVS_ATTR_TYPE

                pdu[length++] = 0x09;    //left battery
                pdu[length++] = 0x09;    //right battery
                pdu[length++] = 0xFF;    //box battery(none)
                #endif
                break;

            case AIVS_ATTR_TYPE_CODEC_TYPE:
                pdu[length++] = 0x02;   //length

                pdu[length++] = i;      //AIVS_ATTR_TYPE_CODEC_TYPE

                pdu[length++] = 0x1;   //0:speex(default), 1:opus
                break;

            default:
                break;
        }
    }

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, length, pdu);
    }
}

void aivs_notify_device_reboot_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[2];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;


    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        if(aivs_rcsp_pdu->param_ptr[1] == 0x1)
        {
            os_printf("aivs_Shutdown!\r\n");
            //to do shutdown operation;
        }
        else
        {
            os_printf("aivs_Reboot!\r\n");
            //to do reboot operation;
        }
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
    }
}

void aivs_notify_phone_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[2];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    os_printf("aivs_notify_phone\r\n");

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
    }
}

void aivs_f2a_discon_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
	uint8 pdu[2];
	aivs_rcsp_status status = RCSP_STAT_SUCCESS;

	os_printf("aivs_f2a_discon_handler[%02x]\r\n",aivs_rcsp_pdu->cmd);

	if(GetA2dpState() >= BT_A2DP_STATE_CONNECTED)
	{
		A2dpDisconnect();
	}
	if(GetHfpState() >= BT_HFP_STATE_CONNECTED)
	{
		BtHfpDisconnect();
	}

	pdu[0] = status;
	pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

	if((aivs_rcsp_pdu->cmd & (0x01 << 6)) && (aivs_rcsp_pdu->cmd & (0x01 << 7)))
	{
		aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
	}
}

uint8_t get_isbtplayinit()
{
	if(GetA2dpState() >= BT_A2DP_STATE_CONNECTED)
		return 1;

	return 0;
}



void aivs_get_dev_private_info_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 i = 0, j = 0;
    uint8 pdu[50];
    uint16 length = 2;
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;
    uint32 private_info_mask;
    extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;

    os_memcpy(&private_info_mask, aivs_rcsp_pdu->param_ptr+1, 4);

    for(i = 0; i < 32; i++)
    {
        switch(private_info_mask & (0x01 << i))
        {
            case AIVS_PRIV_ATTR_BT_MAC:
                pdu[length++] = 0x07;   //length

                pdu[length++] = i;    //AIVS_PRIVATE_ATTR_TYPE
#if 0
                for(j = 0 ; j < 6; j++)
                {
                    //to do get the BT MAC addr
                    pdu[length++] = btStackConfigParams->bt_LocalDeviceAddr[5-j];//0x66 + j;   //BT MAC addr

                }
#else
                os_memcpy(pdu + length,btStackConfigParams->bt_LocalDeviceAddr,6);
                length += 6;
#endif
                break;

            case AIVS_PRIV_ATTR_BLE_MAC:
                #if 0
                pdu[length++] = 0x07;   //length

                pdu[length++] = i;    //AIVS_PRIVATE_ATTR_TYPE

                pdu[length++] = 0x55;   //BLE MAC addr
                pdu[length++] = 0x23;   //BLE MAC addr
                pdu[length++] = 0xaB;   //BLE MAC addr
                pdu[length++] = 0x76;   //BLE MAC addr
                pdu[length++] = 0x32;   //BLE MAC addr
                pdu[length++] = 0x68;   //BLE MAC addr
                #endif
                pdu[length++] = 0x07;   //length

                pdu[length++] = i;

                for(j = 0 ; j < 6; j++)
				{
					//to do get the BT MAC addr
					pdu[length++] = btStackConfigParams->ble_LocalDeviceAddr[j];//0x66 + j;   //BT MAC addr

				}

//                os_memcpy(pdu + length,btStackConfigParams->ble_LocalDeviceAddr,6);
//                length += 6;
                break;

            case AIVS_PRIV_ATTR_MAX_MTU:
                break;

            case AIVS_PRIV_ATTR_BT_STAT:
                pdu[length++] = 0x02;   //length

                pdu[length++] = i;    //AIVS_PRIVATE_ATTR_TYPE

                pdu[length++] = get_isbtplayinit();   //0:BT disconnected, 1:BT connected
                break;

            case AIVS_PRIV_ATTR_POWER_MODE:
                pdu[length++] = 0x02;   //length

                pdu[length++] = i;    //AIVS_PRIVATE_ATTR_TYPE

                pdu[length++] = 0x0;   //0:normal mode, 1:low power mode
                break;

            default:
                break;
        }
    }

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, length, pdu);
    }
}

uint16 aivs_crc16(uint8 *puchMsg, uint16 usDataLen)
{
	uint16 wCRCin = 0x0000;
	uint16 wCPoly = 0x1021;
	uint8 wChar = 0;
	uint16 i;
	while (usDataLen--)
	{
		wChar = *(puchMsg++);
		wCRCin ^= (wChar << 8);
		for(i = 0;i < 8;i++)
		{
			if(wCRCin & 0x8000)
			{
				wCRCin = (wCRCin << 1) ^ wCPoly;
			}
			else
			{
				wCRCin = wCRCin << 1;
			}
		}
	}
	return (wCRCin) ;
}

void aivs_gen_ble_virtual_addr(uint8* bt_addr,uint8 index)
{
	uint8 ble_tmp_addr[6];
	uint8 add_val,next_val=0;
	uint8 byte_3_val,i,cur_bit;
	uint16 byte4_5;
	uint8 cur_bit_val[8];
	//const uint8 bt_addr_test[]={0xE4,0xDB,0x6D,0xB2,0xa7,0x94};
	const uint8 bit_index[]={0,2,4,5,6};
	const uint8 bit_index_1[]={1,3,7};
	os_memcpy(ble_tmp_addr,bt_addr,3);
	byte_3_val=*(bt_addr+3);
	byte4_5=aivs_crc16(bt_addr,6);
	ble_tmp_addr[5]=byte4_5&0xff;
	ble_tmp_addr[4]=(byte4_5>>8)&0xff;
	for(i=0;i<3;i++)
	{
		cur_bit=bit_index_1[i];
		if((byte_3_val&(1<<cur_bit))==0x0)
		{
			byte_3_val|=1<<cur_bit;
		}
		else
		{
			byte_3_val&=~(1<<cur_bit);
		}
	}
	if(index>0x1F)
	{
		index=0;
	}
	for(i=0;i<sizeof(bit_index);i++)
	{
		cur_bit=bit_index[i];
		cur_bit_val[i]=(index&(0x01<<i))>>i;
		add_val=((byte_3_val&(1<<cur_bit))>>cur_bit)+cur_bit_val[i]+next_val;
		if(add_val==3)
		{
			byte_3_val|=1<<cur_bit;
			next_val=1;
		}
		else if(add_val==2)
		{
			byte_3_val&=~(1<<cur_bit);
			next_val=1;
		}
		else
		{
			byte_3_val|=add_val<<cur_bit;
			next_val=0;
		}
	}
	os_printf("byte_3_val:0x%x\n",byte_3_val);
	ble_tmp_addr[3]=byte_3_val;
}

void aivs_notify_phone_virtual_addr_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[2];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    os_printf("aivs_notify_phone_virtual_addr\r\n");

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
    }
}

void aivs_wakeup_classic_bt_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
	uint8 pdu[2];
	aivs_rcsp_status status = RCSP_STAT_SUCCESS;

	os_printf("aivs_wakeup_classic_bt\r\n");

	pdu[0] = status;
	pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

	if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
	{
		aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
	}
}

void aivs_notify_unbound_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
	uint8 pdu[2];
	aivs_rcsp_status status = RCSP_STAT_SUCCESS;

	os_printf("aivs_notify_unbound\r\n");

	pdu[0] = status;
	pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

	if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
	{
		aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
	}
}

void aivs_notify_comm_way_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[2];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    os_printf("aivs_notify_comm_way[%02x]\r\n",aivs_rcsp_pdu->param_ptr[1]);
    os_printf("%02x %02x %02x %02x %02x\r\n",
    		aivs_rcsp_pdu->cmd,aivs_rcsp_pdu->opcode,aivs_rcsp_pdu->param_len,
			aivs_rcsp_pdu->param_ptr[0],aivs_rcsp_pdu->param_ptr[1]);

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
    }

    if(aivs_rcsp_pdu->param_ptr[1] == 0)
    {
    	set_communicate_way(CURRENT_COMMUNICATE_BLE);
    }
    else
    {
        set_communicate_way(CURRENT_COMMUNICATE_SPP);
    }
}
/**********************************XM PAIR END********************************/


/********************************XM SPEECH BEGIN******************************/
uint8 aivs_speech_is_ongoing(void)
{
    return aivs_speech_start_flag;
}

uint8 xm_ai_init()
{
	static uint8_t resampler_init_f = 0;

	if(!resampler_init_f)
	{
		xmai_resampler_init();
		XM_AI_Mutex_init();
		resampler_init_f = 1;
	}
	if(!pcm_buffer_init())
		APP_DBG("pcm_buffer_init ERR!!!\r\n");

	aivs_opus_encode_init();
	mv_opus_fifo_init();
	SoftFlagRegister(SoftFlagAiProcess);
	XiaoAiTaskMsgSendStart();

	return 1;
}

void xm_ai_deinit()
{
	GIE_DISABLE();
	SoftFlagDeregister(SoftFlagAiProcess);
	aivs_opus_encode_destroy();
	mv_opus_fifo_deinit();
	pcm_buffer_deinit();
//	XmAiPlayKill();
	GIE_ENABLE();

	vTaskDelay(2);
	XiaoAiStopMsgSendtoMain();
}

uint32_t data_len_encode = OPUS_5FRAMESIZEADDHEAD_SIZE;
void xm_ai_encode_data_run_loop(void)
{
	static uint8_t frame_cnt = 0;
	if(get_communicate_way() == CURRENT_COMMUNICATE_SPP)
	{
		if(mv_opus_get_len() > OPUS_5FRAMESIZEADDHEAD_SIZE)
		{
			mv_opus_get_data((uint8_t*)spp_senddata+9,data_len_encode);
			aivs_speech_data(spp_senddata,(uint16_t)data_len_encode);
		}
	}
	else if(get_ble_send_size() == 0)
	{
		if(frame_cnt == 0)
		{
			if(mv_opus_get_len() >= OPUS_1FRAMESIZEADDHEAD_SIZE)
			{
				frame_cnt++;
				mv_opus_get_data((uint8_t*)(get_send_data_p()+9),OPUS_1FRAMESIZEADDHEAD_SIZE+6);
				aivs_speech_data((uint8_t*)get_send_data_p(),(uint16_t)(OPUS_1FRAMESIZEADDHEAD_SIZE+6));

			}
		}
		else
		{
			if(mv_opus_get_len() >= OPUS_1FRAMESIZEADDHEAD_SIZE)
			{
				frame_cnt++;
				mv_opus_get_data((uint8_t*)(get_send_data_p() + 9),OPUS_1FRAMESIZEADDHEAD_SIZE);
				aivs_speech_data((uint8_t*)get_send_data_p(),(uint16_t)OPUS_1FRAMESIZEADDHEAD_SIZE);
				if(frame_cnt >= 5)
				{
					frame_cnt = 0;
				}
			}
		}

	}
}

int16_t* pcm_16k_buf = NULL;
#define PCM_16K_BUF_LEN 640*4

int16_t* pcm_16k_fifo = NULL;
#define PCM_16K_FIFO_LEN 640*4

int16_t* speex_data = NULL;
#define SPEEX_DATA_LEN 640*4

int16_t* ai_pcm_data = NULL;
#define AI_PCM_DATA_LEN 640*4

static uint32_t ai_frame_size = 640;

uint8_t pcm_buffer_init()
{

	Lock_AIEncoding_init();

	if(pcm_16k_buf == NULL)
		pcm_16k_buf = (int16_t*)pvPortMalloc(PCM_16K_BUF_LEN);
	if(pcm_16k_buf == NULL)
		return 0;

	if(pcm_16k_fifo == NULL)
		pcm_16k_fifo = (int16_t*)pvPortMalloc(PCM_16K_FIFO_LEN);
	if(pcm_16k_fifo == NULL)
		return 0;

	if(speex_data == NULL)
		speex_data = (int16_t*)pvPortMalloc(SPEEX_DATA_LEN);
	if(speex_data == NULL)
		return 0;

	if(ai_pcm_data == NULL)
		ai_pcm_data = (int16_t*)pvPortMalloc(AI_PCM_DATA_LEN);
	if(ai_pcm_data == NULL)
		return 0;

	return 1;
}

void pcm_buffer_deinit()
{
	if(pcm_16k_buf)
		vPortFree(pcm_16k_buf);
	pcm_16k_buf = NULL;

	if(pcm_16k_fifo)
		vPortFree(pcm_16k_fifo);
	pcm_16k_fifo = NULL;

	if(speex_data)
		vPortFree(speex_data);
	speex_data = NULL;

	if(ai_pcm_data)
		vPortFree(ai_pcm_data);
	ai_pcm_data = NULL;
}

void opus_encode_data(int16_t* buf,uint32_t sample,uint8_t ch)
{
	int outputLen = 0;
	uint32_t len = 0;
	static uint32_t last_length = 0;
	int i;
	int ii;

	if((ai_pcm_data == NULL) || (pcm_16k_fifo == NULL) || (pcm_16k_buf == NULL))
		return;
	if(!aivs_speech_is_ongoing())//语音传输已经结束。
		return;

	Lock_AIEncoding();//防止audio core解码时被释放资源。

	for(ii=0;ii<sample/128;ii++)
	{
		if(ch == 2)
		{
			for(i=0;i<128;i++)
			{
				ai_pcm_data[i] = buf[2*i + 256*ii];
			}
		}
		else
		{
			memcpy(ai_pcm_data,&buf[128*ii],128*2);
		}
		len = xmai_resampler_apply(ai_pcm_data,pcm_16k_buf,128);
		for(i=0;i<len;i++)
		{
			pcm_16k_buf[i] = __nds32__clips((pcm_16k_buf[i]*10),16-1);//放大10倍
		}
		len = len*2;
		uint8_t *p_fifo = (uint8_t *)pcm_16k_fifo;
		uint8_t *p_buffer = (uint8_t *)pcm_16k_buf;
		memcpy(p_fifo+last_length,p_buffer,len);
		last_length = last_length + len;
		if(last_length >= ai_frame_size)
		{
			XiaoAiTaskMsgSendRun();

			while(OpusEncodedLenGet() == 0)
			{
				vTaskDelay(1);
				if(!SoftFlagGet(SoftFlagAiProcess))
					OpusEncodedLenSet(1);//任务停止就退出，防止死锁。
			}
			//aivs_opus_encode_stream((char*)pcm_16k_fifo,(int)ai_frame_size,(uint8_t*)speex_data,&outputLen);

			outputLen = OpusEncodedLenGet();
			memcpy(p_fifo,p_fifo+ai_frame_size,last_length - ai_frame_size);
			last_length = last_length - ai_frame_size;
			mv_opus_get_send((uint8_t*)speex_data,(uint32_t)outputLen);
		}
	}
	Unlock_AIEncoding();


}

//返回编码后的长度*************************
int32_t encoded_len = 0;

void OpusEncodedLenSet(int32_t set)
{
	encoded_len = set;
}

int32_t OpusEncodedLenGet()
{
	return encoded_len;
}

//************************************

void OpusEncoderTaskPro()
{
	aivs_opus_encode_stream((char*)pcm_16k_fifo,(int)ai_frame_size,(uint8_t*)speex_data,(int*)&encoded_len);
	if(!encoded_len)
		OpusEncodedLenSet(1);//编码失败发送1防止audiocore死锁。
}

void aivs_speech_start(void)
{
    uint8 pdu[2];

    os_printf("aivs_speech_start\r\n");

    pdu[0] = aivs_rcsp_opcodesn++;
    pdu[1] = 0x00;                //short button only
    //pdu[2] = 0x01;                //fastmode only. no need

    aivs_rcsp_cmd(RCSP_RESP, AIVS_OPCODE_SPEECH_START, 2, pdu);
}

void aivs_speech_start_resp_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 status = aivs_rcsp_pdu->param_ptr[0];
    uint8 opcode_sn = aivs_rcsp_pdu->param_ptr[1];
    uint8 pdu[2];

    os_printf("aivs_speech_start_resp_handler[%02x]\r\n",aivs_rcsp_pdu->cmd);

    if(!(aivs_rcsp_pdu->cmd & (0x01 << 7)) && !(aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {
        if((status == RCSP_STAT_SUCCESS) && (opcode_sn == (aivs_rcsp_opcodesn - 1)))
        {
            if(!aivs_speech_start_flag)
            {
                if(!xm_ai_init())
                	return;
                aivs_speech_start_flag = 1;
            }
        }
    }
    else if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
	{
    	pdu[0] = RCSP_STAT_SUCCESS;
    	pdu[1] = aivs_rcsp_pdu->param_ptr[0];
		aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
//		if(!aivs_speech_start_flag)//app暂时没有手机唤醒的功能。
//		{
//			if(!xm_ai_init())
//				 return;
//
//			aivs_speech_start_flag = 1;
//		}
	}

}

void aivs_speech_stop(void)
{
	uint8 pdu[2];

	os_printf("aivs_speech_stop\r\n");

	pdu[0] = aivs_rcsp_opcodesn++;
	pdu[1] = 0x00;                //short button only
	//pdu[2] = 0x01;                //fastmode only. no need

	aivs_rcsp_cmd(RCSP_RESP, AIVS_OPCODE_SPEECH_STOP, 2, pdu);
}

void aivs_speech_stop_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
    uint8 pdu[2];
    aivs_rcsp_status status = RCSP_STAT_SUCCESS;

    os_printf("aivs_speech_stop\r\n");

    pdu[0] = status;
    pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

    if((aivs_rcsp_pdu->cmd & (0x01 << 7)) && (aivs_rcsp_pdu->cmd & (0x01 << 6)))
    {

        aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
        if(aivs_speech_start_flag)
        {
            aivs_speech_start_flag = 0;
            Lock_AIEncoding();
            xm_ai_deinit();
            Unlock_AIEncoding();
        }
    }
}
void aivs_f2a_edr_stat_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
	uint8 pdu[2];
	aivs_rcsp_status status = RCSP_STAT_SUCCESS;

	os_printf("aivs_f2a_edr_stat_handler[%02x]\r\n",aivs_rcsp_pdu->cmd);

	pdu[0] = status;
	pdu[1] = aivs_rcsp_pdu->param_ptr[0];    //opcode_SN

	if((aivs_rcsp_pdu->cmd & (0x01 << 6)) && (aivs_rcsp_pdu->cmd & (0x01 << 7)))
	{
		aivs_rcsp_cmd_resp(aivs_rcsp_pdu->opcode, sizeof(pdu)/sizeof(pdu[0]), pdu);
	}
}


void aivs_speech_stop_resp_handler(AIVS_RCSP* aivs_rcsp_pdu)
{
}

void aivs_speech_cancel(void)
{
}

void aivs_speech_cancel_resp_handler(void)
{
}

void aivs_speech_data(uint8* data, uint16 length)
{

	data[7] = aivs_rcsp_opcodesn++;
	data[8] = AIVS_OPCODE_SPEECH_START;

	aivs_speeth_rcsp_cmd(RCSP_NO_RESP, AIVS_OPCODE_DATA_TRANSFER, length + 2, data);
}

void aivs_speech_error_check(void)
{
    //if spp and ble is disconnection when it is speeching.
    {
        os_printf("\r\n!!!aivs_speech_error!!!\r\n");
//        aivs_opus_encode_destroy();
        aivs_speech_start_flag = 0;
    }
}

uint16_t swap_endian16(uint16_t input)
{
	uint16_t output = 0;

	output =  (((input >> 8) & 0xff) | ((input) & 0xff) << 8);
	return output;
}

uint32_t swap_endian32(uint32_t input)
{
	uint32_t output = 0;

	output =  (((input >> 24) & 0xff) | (((input >> 16) & 0xff) << 8) | \
			(((input >> 8) & 0xff) << 16) | (((input) & 0xff) << 24));
	return output;
}

void xm_speech_con_set(uint8_t set)
{
	aivs_speech_con_flag = set;
}


uint8_t xm_speech_iscon()
{
	return aivs_speech_con_flag;
}

void aivs_conn_edr_status(void)
{
    uint8 pdu[2];

    os_printf("aivs_conn_edr_status\r\n");

    pdu[0] = aivs_rcsp_opcodesn++;
    pdu[1] = 0x01;                //connect ok
    //pdu[2] = 0x01;                //fastmode only. no need

    aivs_rcsp_cmd(RCSP_RESP, AIVS_OPCODE_F2A_EDR_STAT, 2, pdu);
}


/********************************XM SPEECH END******************************/
AIVS_RCSP aivs_rcsp_pdu_data;

void aivs_app_decode(uint8 length, uint8 *pValue)
{
    if((((pValue[0]<<16)|(pValue[1]<<8)|(pValue[2])) == AIVS_RCSP_START) && (pValue[length-1] == AIVS_RCSP_END))
    {
    	AIVS_RCSP *aivs_rcsp_pdu;

    	aivs_rcsp_pdu = &aivs_rcsp_pdu_data;
        memset(aivs_rcsp_pdu, 0, sizeof(AIVS_RCSP));

        os_memcpy(aivs_rcsp_pdu, pValue + 3, sizeof(AIVS_RCSP) - sizeof(u_int8*));

        //转字节序
        aivs_rcsp_pdu->param_len = SWAP_ENDIAN16(aivs_rcsp_pdu->param_len);
        aivs_rcsp_pdu->param_ptr = pValue + 7;

        os_printf("cm_op:0x%02x\r\n",aivs_rcsp_pdu->opcode);

        switch(aivs_rcsp_pdu->opcode)
        {
            case AIVS_OPCODE_DATA_TRANSFER:
                break;

            case AIVS_OPCODE_GET_DEV_INFO:
                aivs_get_device_info_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_DEV_REBOOT:
                aivs_notify_device_reboot_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_NOTIFY_PHONE_INFO:
                aivs_notify_phone_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_SET_PRO_MTU:
                break;

            case AIVS_OPCODE_A2F_DISCONN_EDR:
            	aivs_f2a_discon_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_F2A_EDR_STAT:
            	aivs_f2a_edr_stat_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_SET_DEV_INFO:
                break;


            case AIVS_OPCODE_GET_DEV_PRIVATE_INFO:
                aivs_get_dev_private_info_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_NOTIFY_COMM_WAY:
                aivs_notify_comm_way_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_WAKEUP_CLASSIC_BT:
            	aivs_wakeup_classic_bt_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_NOTIFY_PHONE_VIRTUAL_ADDR:
                aivs_notify_phone_virtual_addr_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_NOTIFY_F2A_BT_OP:
                break;

            case AIVS_OPCODE_NOTIFY_UNBOUND:
            	aivs_notify_phone_virtual_addr_handler(aivs_rcsp_pdu);
            	break;

            case AIVS_OPCODE_SPEECH_START:
                aivs_speech_start_resp_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_SPEECH_STOP:
                aivs_speech_stop_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_SPEECH_CANCEL:
                break;

            case AIVS_OPCODE_SPEECH_LONG_HOLD:
                break;
            case AIVS_OPCODE_OTA_GET_OFFSET:
                aivs_ota_get_info_offset_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_OTA_IF_UPDATE:
                aivs_ota_inquiry_if_can_update_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_OTA_ENTER_UPDATE:
                aivs_ota_enter_update_mode_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_OTA_EXIT_UPDATE:
                aivs_ota_exit_update_mode_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_OTA_SEND_BLOCK:
                aivs_ota_send_update_block_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_OTA_GET_STATUS:
                aivs_ota_get_refresh_status_handler(aivs_rcsp_pdu);
                break;

            case AIVS_OPCODE_OTA_NOTIFY_UPDATE:
                break;
            case AIVS_OPCODE_VENDOR_SPEC_RESERVED:
                break;

            default:
                break;
        }


    }
    else
    {
        aivs_auth_process_handler(length, pValue);
    }
}

#endif	//CFG_XIAOAI_AI_EN
