
/*
* bt_obex_api.h
*/

/**
* @addtogroup Bluetooth
* @{
* @{
*/

#ifndef __BT_OBEX_API_H__
#define __BT_OBEX_API_H__

/**
 * OBEX releate values
 */
enum
{
    BT_OBEX_NAME         = 1, /**< File Name */
    BT_OBEX_LENGTH       = 3, /**< File Length */
    BT_OBEX_BODY         = 8, /**< File Data */
    BT_OBEX_BODY_END     = 9, /**< File Data (Last Part) */
};

/**
 * OBEX releate event
 */
typedef enum{
	BT_STACK_EVENT_OBEX_NONE = 0,
	BT_STACK_EVENT_OBEX_CONNECTED,
	BT_STACK_EVENT_OBEX_DISCONNECTED,
	BT_STACK_EVENT_OBEX_DATA_RECEIVED,
}BT_OBEX_CALLBACK_EVENT;

typedef struct _BT_OBEX_CALLBACK_PARAMS
{
/*
 *  Following fields are valid when event is DATA_IND
 *      A file is splitted into multiple parts
 *      A part is further splitted into multiple segments
 *      Each time a segment is received
 *
 *      type & total describes the part
 *      segoff/segdata/seglen describes the current segment
 */
    uint8_t		type;       // Name, Length, Body ...
    uint16_t	total;      // total length of this part of data
    uint16_t	segoff;     // offset of this segment
    uint8_t		*segdata;   // pointer to data
    uint16_t	seglen;     // Segment Length
	
}BT_OBEX_CALLBACK_PARAMS;

typedef void (*BTObexCallbackFunc)(BT_OBEX_CALLBACK_EVENT event, BT_OBEX_CALLBACK_PARAMS * param);

void BtObexCallback(BT_OBEX_CALLBACK_EVENT event, BT_OBEX_CALLBACK_PARAMS * param);

bool ObexAppInit(BTObexCallbackFunc callback);

#endif


