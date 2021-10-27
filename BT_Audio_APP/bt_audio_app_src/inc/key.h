/**
 **************************************************************************************
 * @file    Key.h
 * @brief   key 
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-1-11 17:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include "app_config.h"
#include "code_key.h"
#include "type.h"

#ifndef __KEY_H__
#define __KEY_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

//按键消息表中index总数
//其中power key adc index为5个；adc key index为22个；ir key index为21个
//用户可根据需要在消息表中增加，并对应修改此总数值；


#define KEY_MSG_DEFAULT_NUM        5


//#define BIT(n)  1 << n

enum
{
	ADC_GPIOA20     = BIT(1),
	ADC_GPIOA21     = BIT(2),
	ADC_GPIOA22     = BIT(3),
	ADC_GPIOA23     = BIT(4),
	ADC_GPIOA24     = BIT(5),
	ADC_GPIOA25     = BIT(6),
	ADC_GPIOA26     = BIT(7),
	ADC_GPIOA27     = BIT(8),
	ADC_GPIOA28     = BIT(9),
	ADC_GPIOA29     = BIT(10),
	ADC_GPIOA30     = BIT(11),
	ADC_GPIOA31     = BIT(12),
	ADC_GPIOB0      = BIT(13),
	ADC_GPIOB1      = BIT(14),
};


/*******键值名宏定义，只为可读性，如有数值重复不影响组合事件查表************/
//ADC Keys 按键键值与命名映射，如果顺序颠倒，或跳过某键值，请改此处。
#define ADC_POWER_KEY01              (0)
#define ADC_POWER_KEY02              (1)
#define ADC_POWER_KEY03              (2)
#define ADC_POWER_KEY04              (3)
#define ADC_POWER_KEY05              (4)
#define ADC_POWER_KEY06              (5)
#define ADC_POWER_KEY07              (6)
#define ADC_POWER_KEY08              (7)
#define ADC_POWER_KEY09              (8)
#define ADC_POWER_KEY10              (9)
#define ADC_POWER_KEY11              (10)

#define ADC_CHANNEL0_KEY01              (0)
#define ADC_CHANNEL0_KEY02              (1)
#define ADC_CHANNEL0_KEY03              (2)
#define ADC_CHANNEL0_KEY04              (3)
#define ADC_CHANNEL0_KEY05              (4)
#define ADC_CHANNEL0_KEY06              (5)
#define ADC_CHANNEL0_KEY07              (6)
#define ADC_CHANNEL0_KEY08              (7)
#define ADC_CHANNEL0_KEY09              (8)
#define ADC_CHANNEL0_KEY10              (9)
#define ADC_CHANNEL0_KEY11              (10)

#define ADC_CHANNEL1_KEY01              (0)
#define ADC_CHANNEL1_KEY02              (1)
#define ADC_CHANNEL1_KEY03              (2)
#define ADC_CHANNEL1_KEY04              (3)
#define ADC_CHANNEL1_KEY05              (4)
#define ADC_CHANNEL1_KEY06              (5)
#define ADC_CHANNEL1_KEY07              (6)
#define ADC_CHANNEL1_KEY08              (7)
#define ADC_CHANNEL1_KEY09              (8)
#define ADC_CHANNEL1_KEY10              (9)
#define ADC_CHANNEL1_KEY11              (10)


#define IO_KEY01              (0)
#define IO_KEY02              (1)
#define IO_KEY03              (2)
#define IO_KEY04              (3)
#define IO_KEY05              (4)
#define IO_KEY06              (5)
#define IO_KEY07              (6)
#define IO_KEY08              (7)
#define IO_KEY09              (8)
#define IO_KEY10              (9)
#define IO_KEY11              (10)
/*******键值名宏定义，组合事件查表************/
#define IR_KEY00						(0)//power
#define IR_KEY01						(1)//Mode
#define IR_KEY02						(2)//Mute
#define IR_KEY03						(3)//Pause
#define IR_KEY04						(4)//Pre
#define IR_KEY05						(5)//Next
#define IR_KEY06						(6)//EQ
#define IR_KEY07						(7)//V-
#define IR_KEY08						(8)//V+
#define IR_KEY09						(9)//'0'
#define IR_KEY10						(10)
#define IR_KEY11						(11)
#define IR_KEY12						(12)
#define IR_KEY13						(13)
#define IR_KEY14						(14)
#define IR_KEY15						(15)
#define IR_KEY16						(16)
#define IR_KEY17						(17)
#define IR_KEY18						(18)
#define IR_KEY19						(19)
#define IR_KEY20						(20)


#define KEY_CODE_KEY1					0xf0	//旋钮1

//串口ASCII值本身具有可读性，不必重定义



/**************************事件宏，不允许数值重复************************/
//Offset分组映射，便于组合查表
#define KEY_PWR_KEY_EVENT_TYPE		  0x10

#define KEY_ADC0_KEY_EVENT_TYPE		  0x20

#define KEY_ADC1_KEY_EVENT_TYPE		  0x30

#define KEY_ADC_KEY_UNKOWN_TYPE	      (ADC_KEY_UNKOWN_TYPE)
#define KEY_ADC_KEY_PRESSED 		  (ADC_KEY_PRESSED)
#define KEY_ADC_KEY_RELEASED		  (ADC_KEY_RELEASED)
#define KEY_ADC_KEY_LONG_PRESSED	  (ADC_KEY_LONG_PRESSED)
#define KEY_ADC_KEY_LONG_PRESS_HOLD   (ADC_KEY_LONG_PRESS_HOLD)
#define KEY_ADC_KEY_LONG_RELEASED	  (ADC_KEY_LONG_RELEASED)

#define KEY_IO_KEY_EVENT_TYPE		  0x40
#define KEY_IO_KEY_UNKOWN_TYPE		  (KEY_IO_KEY_EVENT_TYPE + ADC_KEY_UNKOWN_TYPE)
#define KEY_IO_KEY_PRESSED 		      (KEY_IO_KEY_EVENT_TYPE + ADC_KEY_PRESSED)
#define KEY_IO_KEY_RELEASED		      (KEY_IO_KEY_EVENT_TYPE + ADC_KEY_RELEASED)
#define KEY_IO_KEY_LONG_PRESSED	      (KEY_IO_KEY_EVENT_TYPE + ADC_KEY_LONG_PRESSED)
#define KEY_IO_KEY_LONG_PRESS_HOLD	  (KEY_IO_KEY_EVENT_TYPE + ADC_KEY_LONG_PRESS_HOLD)
#define KEY_IO_KEY_LONG_RELEASED	  (KEY_IO_KEY_EVENT_TYPE + ADC_KEY_LONG_RELEASED)


#define	KEY_CODE_KEY_EVENT_TYPE		  0x50
#define KEY_CODE_KEY_NONE			  (KEY_CODE_KEY_EVENT_TYPE + CODE_KEY_NONE)
#define KEY_CODE_KEY_FORWARD		  (KEY_CODE_KEY_EVENT_TYPE + CODE_KEY_FORWARD)
#define KEY_CODE_KEY_BACKWARD		  (KEY_CODE_KEY_EVENT_TYPE + CODE_KEY_BACKWARD)

#define	KEY_UART_KEY_EVENT_TYPE		  0x60

#define KEY_IR_KEY_EVENT_TYPE		  0x70
#define KEY_IR_KEY_UNKOWN_TYPE		  (KEY_IR_KEY_EVENT_TYPE + IR_KEY_UNKOWN_TYPE)
#define	KEY_IR_KEY_PRESSED			  (KEY_IR_KEY_EVENT_TYPE + IR_KEY_PRESSED)
#define	KEY_IR_KEY_RELEASED			  (KEY_IR_KEY_EVENT_TYPE + IR_KEY_RELEASED)
#define	KEY_IR_KEY_LONG_PRESSED		  (KEY_IR_KEY_EVENT_TYPE + IR_KEY_LONG_PRESSED)
#define	KEY_IR_KEY_LONG_PRESS_HOLD	  (KEY_IR_KEY_EVENT_TYPE + IR_KEY_LONG_PRESS_HOLD)
#define	KEY_IR_KEY_LONG_RELEASED	  (KEY_IR_KEY_EVENT_TYPE + IR_KEY_LONG_RELEASED)

#define KEY_POWER_KEY_EVENT_TYPE	  0x80

//----------------------------------------------------------------------//
extern const uint16_t KEY_DEFAULT_MAP[][KEY_MSG_DEFAULT_NUM];

typedef struct _DBKeyMsg_1_
{
	uint16_t dbclick_en;
	uint16_t dbclick_counter;
	uint16_t dbclick_timeout;

	uint32_t KeyMsg;
	uint32_t dbclick_msg;
}KEYBOARD_MSG;

void DbclickInit(void);
void DbclickProcess(void);
uint8_t DbclickGetMsg(uint16_t Msg);
void KeyInit(void);
MessageId KeyScan(void);
void BeepEnable(void);
void SetIrKeyValue(uint8_t KeyType, uint16_t KeyValue);
void SetAdcKeyValue(uint8_t KeyType, uint16_t KeyValue);
void SetGlobalKeyValue(uint8_t KeyType, uint16_t KeyValue);
uint32_t GetGlobalKeyValue(void);
void ClrGlobalKeyValue(void);



#ifdef __cplusplus
}
#endif//__cplusplus


#endif




