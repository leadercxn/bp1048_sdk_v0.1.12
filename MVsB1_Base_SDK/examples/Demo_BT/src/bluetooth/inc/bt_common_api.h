/**
 *******************************************************************************
 * @file    bt_common_api.h
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   common related api
 *******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

/**
* @addtogroup Bluetooth
* @{
* @defgroup bt_common_api bt_common_api.h
* @{
*/

#ifndef _BT_COMMON_API_H_
#define _BT_COMMON_API_H_


/*---------------------------------------------------------------------------
 * BtClassOfDevice type
 *
 *     Bit pattern representing the class of device along with the 
 *     supported services. There can be more than one supported service.
 *     Service classes can be ORed together. The Device Class is composed
 *     of a major device class plus a minor device class. ORing together
 *     each service class plus one major device class plus one minor device
 *     class creates the class of device value. The minor device class is
 *     interpreted in the context of the major device class.
 */

typedef uint32_t BtClassOfDevice;

/* Group: Major Service Classes. Can be ORed together */
#define COD_LIMITED_DISCOVERABLE_MODE 0x00002000
#define COD_POSITIONING               0x00010000
#define COD_NETWORKING                0x00020000
#define COD_RENDERING                 0x00040000
#define COD_CAPTURING                 0x00080000
#define COD_OBJECT_TRANSFER           0x00100000
#define COD_AUDIO                     0x00200000
#define COD_TELEPHONY                 0x00400000
#define COD_INFORMATION               0x00800000

/* Group: Major Device Classes (Select one) */
#define COD_MAJOR_MISCELLANEOUS       0x00000000
#define COD_MAJOR_COMPUTER            0x00000100
#define COD_MAJOR_PHONE               0x00000200
#define COD_MAJOR_LAN_ACCESS_POINT    0x00000300
#define COD_MAJOR_AUDIO               0x00000400
#define COD_MAJOR_PERIPHERAL          0x00000500
#define COD_MAJOR_IMAGING             0x00000600
#define COD_MAJOR_UNCLASSIFIED        0x00001F00

/* Group: Minor Device Class - Audio Major class */
#define COD_MINOR_AUDIO_UNCLASSIFIED  0x00000000
#define COD_MINOR_AUDIO_HEADSET       0x00000004
#define COD_MINOR_AUDIO_HANDSFREE     0x00000008
#define COD_MINOR_AUDIO_MICROPHONE    0x00000010
#define COD_MINOR_AUDIO_LOUDSPEAKER   0x00000014
#define COD_MINOR_AUDIO_HEADPHONES    0x00000018
#define COD_MINOR_AUDIO_PORTABLEAUDIO 0x0000001C
#define COD_MINOR_AUDIO_CARAUDIO      0x00000020
#define COD_MINOR_AUDIO_SETTOPBOX     0x00000024
#define COD_MINOR_AUDIO_HIFIAUDIO     0x00000028
#define COD_MINOR_AUDIO_VCR           0x0000002C
#define COD_MINOR_AUDIO_VIDEOCAMERA   0x00000030
#define COD_MINOR_AUDIO_CAMCORDER     0x00000034
#define COD_MINOR_AUDIO_VIDEOMONITOR  0x00000038
#define COD_MINOR_AUDIO_VIDEOSPEAKER  0x0000003C
#define COD_MINOR_AUDIO_CONFERENCING  0x00000040
#define COD_MINOR_AUDIO_GAMING        0x00000048

/* Group: Minor Device Class - Peripheral Major class */
#define COD_MINOR_PERIPH_KEYBOARD     0x00000040
#define COD_MINOR_PERIPH_POINTING     0x00000080
#define COD_MINOR_PERIPH_COMBOKEY     0x000000C0
/* End of BtClassOfDevice */


/*******************************************************************************
*
* Inquiry mode define
*/
typedef uint8_t					BTInquiryMode;

#define INQUIRY_MODE_NORMAL		0 /*!< Normal Inquiry Response format			*/
#define INQUIRY_MODE_RSSI		1 /*!< RSSI Inquiry Response format				*/
#define INQUIRY_MODE_EXTENDED	2 /*!< Extended or RSSI Inquiry Response format	*/


typedef struct _InquriyResultParam
{
	const uint8_t	*addr;
	signed short	rssi;
	const uint8_t	*extResp;
	uint16_t		extRespLen;
}InquriyResultParam;


typedef struct _RequestRemNameParam
{
	const uint8_t	*addr;
	const uint8_t	*name;
	uint16_t		nameLen;
}RequestRemNameParam;


/**
 * BtAccessibleMode type
 */
typedef uint8_t		BtAccessMode;

#define BtAccessModeNotAccessible		0x00 /*!< Non-discoverable or connectable		*/
#define BtAccessModeDiscoverbleOnly		0x01 /*!< Discoverable but not connectable		*/
#define BtAccessModeConnectableOnly		0x02 /*!< Connectable but not discoverable		*/
#define BtAccessModeGeneralAccessible	0x03 /*!< General discoverable and connectable	*/

/**
 * BTLinkMode type
 */
typedef uint8_t		BTLinkMode;

#define BTLinkModeActive		0x00
#define BTLinkModeHold			0x01
#define BTLinkModeSniff			0x02
#define BTLinkModePark			0x03
#define BTLinkModeScatter		0x04

typedef struct _ModeChange
{
	BTLinkMode	mode;
	uint8_t		*addr;
}ModeChange;

/**
 * @brief  User Command: Inquiry command, the bluetooth device will start inquiry with inquiry mode.
 * @param  inquiryMode The inquiry mode, see BTInquiryMode.
 * @return True for the command implement successful 
 * @Note 
 *
 */
bool BTInquiry(BTInquiryMode inquiryMode);

/**
 * @brief  User Command: Cancel inquiry .
 * @param  NONE.
 * @return True for the command implement successful 
 * @Note 
 *
 */
bool BTInquiryCancel(void);

/**
 * @brief  User Command: Get remote device name.
 * @param  addr The bt address of remote device.
 * @return True for the command implement successful 
 * @Note 
 *
 */
bool BTGetRemoteDeviceName(uint8_t * addr);

/**
 * @brief  User Command: Set local bluetooth access mode.
 * @param  accessMode The local bluetooth access mode.
 * @return True for the command implement successful 
 * @Note 
 *
 */
bool BTSetAccessMode(BtAccessMode accessMode);


/**
 * @brief  User Command: Info remote device into sniff mode
 * @param  remAddr The bluetooth address of remote device.
 * @return True for the command implement successful 
 * @Note 
 *
 */

void BTSetRemDevIntoSniffMode(uint8_t * remAddr);

/**
 * @brief  User Command: Info remote device exit from sniff mode
 * @param  remAddr The bluetooth address of remote device.
 * @return True for the command implement successful 
 * @Note 
 *
 */	
void BTSetRemDevExitSniffMode(uint8_t * remAddr);

/**
 * @brief  User Command: Disconnect all connected profile
 * @param  NONE
 * @return True for the command implement successful 
 * @Note 
 *
 */	
bool BTDisconnect(void);

/**
 * @brief  config class of device params
 * @param  bt cod
 * @return NONE 
 * @Note 
 *
 */	
void SetBtClassOfDevice(uint32_t BtCod);



#endif

