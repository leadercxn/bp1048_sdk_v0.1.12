/*
 **************************************************************************************
 * @file    hw_interface.h
 * @brief  
 * 
 * @author   
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "debug.h"
#include "ctrlvars.h"
//#include "config_opus.h"
#include "app_config.h"
#include "gpio.h"
#include "sys_gpio.h"
#ifndef __HW_INTERFACE__
#define __HW_INTERFACE__
/*
**************************************************************************************
*  ����GPIOӦ�ÿ������ýӿ�����
*  
**************************************************************************************
*/

//------ϵͳ��Դ���ؿ���IO��ѡ��--------------------------------------------//
//GPIOB6��POWER ON,OFF����
#define POWER_CTL_PORT             B
#define POWER_CTL_PIN              0//GPIO_INDEX31
//------------------------------------------------------------------------//

//------�ⲿ���ؼ��IOѡ��----------------------------------------------//
#ifdef	CFG_SOFT_POWER_KEY_EN
//SOFT POWER KEY, GPIOA27��POWER KEY���
#define POWER_KEY_PORT             A
#define POWER_KEY_PIN              GPIO_INDEX27
#endif
//------------------------------------------------------------------------//

//------GPIO KEY���IOѡ��------------------------------------------------//
#ifdef CFG_GPIO_KEY1_EN
#define GPIO_KEY1_PORT             A
#define GPIO_KEY1                  GPIO_INDEX23
#endif

#ifdef CFG_GPIO_KEY2_EN
#define GPIO_KEY2_PORT             A
#define GPIO_KEY2                  GPIO_INDEX26
#endif
//------------------------------------------------------------------------//

//------�������IOѡ��----------------------------------------------------//
#ifdef CFG_FUNC_DETECT_PHONE_EN
#define PHONE_DETECT_PORT          B
#define PHONE_DETECT_PIN           GPIO_INDEX27
#endif
//------------------------------------------------------------------------//

//------MIC ���IOѡ��----------------------------------------------------//
#ifdef CFG_FUNC_DETECT_MIC_EN
#define MIC1_DETECT_PORT           B
#define MIC1_DETECT_PIN            GPIO_INDEX26

#define MIC2_DETECT_PORT           B
#define MIC2_DETECT_PIN            GPIO_INDEX25
#endif
//------------------------------------------------------------------------//

//------3��/4�߶������ͼ��IOѡ��----------------------------- -----------//
#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
#define MIC_SEGMENT_PORT           B
#define MIC_SEGMENT_PIN            GPIO_INDEX24
#endif
//------------------------------------------------------------------------//

//------mic mute ����IOѡ��-----------------------------------------------//
#define MIC_MUTE_CTL_PORT          B
#define MIC_MUTE_CTL_PIN           0//GPIO_INDEX23
//------------------------------------------------------------------------//

//------�����IOѡ��----------------------------------------------------//
#if CFG_CHARGER_DETECT_EN
#define CHARGE_DETECT_PORT         B
#define CHARGE_DETECT_PIN          GPIO_INDEX22
#endif
//------------------------------------------------------------------------//

//------MUTE��·�����MUTE����IOѡ��--------------------------------------//
#define MUTE_CTL_PORT              B
#define MUTE_CTL_PIN               0//GPIO_INDEX21

//------------------------------------------------------------------------//

//------LED����IOѡ��-----------------------------------------------------//
#if CFG_LED_EN
#define POWER_LED_PORT             B
#define POWER_LED_PIN              GPIO_INDEX20

#define EAR_LED_PORT               B
#define EAR_LED_PIN                GPIO_INDEX19

#define SONG_LED_PORT              B
#define SONG_LED_PIN               GPIO_INDEX18

#define TALK_LED_PORT              B
#define TALK_LED_PIN               GPIO_INDEX17

#define KTV_LED_PORT               B
#define KTV_LED_PIN                GPIO_INDEX16

#define AUTO_TUNE_LED_PORT         B 
#define AUTO_TUNE_LED_PIN          GPIO_INDEX15

#define SHUNNING_LED_PORT          B
#define SHUNNING_LED_PIN           GPIO_INDEX14
#endif
//------------------------------------------------------------------------//



/*
**************************************************************************************
*  ���²���Ҫ�޸ģ���
*  
**************************************************************************************
*/

//------ϵͳ��Դ���ؿ���IO�Ĵ�������--------------------------------------------//
#define POWER_CTL_IE               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPIE)         
#define POWER_CTL_OE               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPOE) 
#define POWER_CTL_PU               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPPU) 
#define POWER_CTL_PD               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPPD)         
#define POWER_CTL_IN               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPIN)         
#define POWER_CTL_OUT              STRING_CONNECT(GPIO,POWER_CTL_PORT,GPOUT) 
//------------------------------------------------------------------------------//

//------�ⲿ���ؼ��IO�Ĵ�������----------------------------------------------//
#ifdef	CFG_SOFT_POWER_KEY_EN
#define POWER_KEY_IE               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPIE)         
#define POWER_KEY_OE               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPOE) 
#define POWER_KEY_PU               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPPU) 
#define POWER_KEY_PD               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPPD)         
#define POWER_KEY_IN               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPIN)         
#define POWER_KEY_OUT              STRING_CONNECT(GPIO,POWER_KEY_PORT,GPOUT) 
#endif
//------------------------------------------------------------------------------//

//------GPIO KEY���IO�Ĵ�������------------------------------------------------// 
#ifdef CFG_GPIO_KEY1_EN
#define GPIO_KEY1_IE               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPIE)         
#define GPIO_KEY1_OE               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPOE) 
#define GPIO_KEY1_PU               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPPU) 
#define GPIO_KEY1_PD               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPPD)         
#define GPIO_KEY1_IN               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPIN)         
#define GPIO_KEY1_OUT              STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPOUT) 
#endif

#ifdef CFG_GPIO_KEY1_EN
#define GPIO_KEY2_IE               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPIE)         
#define GPIO_KEY2_OE               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPOE) 
#define GPIO_KEY2_PU               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPPU) 
#define GPIO_KEY2_PD               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPPD)         
#define GPIO_KEY2_IN               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPIN)         
#define GPIO_KEY2_OUT              STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPOUT) 
#endif
//-------------------------------------------------------------------------------//

//------�������IO�Ĵ�������-----------------------------------------------------//
#ifdef CFG_FUNC_DETECT_PHONE_EN
#define PHONE_DETECT_IE            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPIE)         
#define PHONE_DETECT_OE            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPOE) 
#define PHONE_DETECT_PU            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPPU) 
#define PHONE_DETECT_PD            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPPD)         
#define PHONE_DETECT_IN            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPIN)         
#define PHONE_DETECT_OUT           STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPOUT) 
#endif
//-------------------------------------------------------------------------------//

//------MIC���IO�Ĵ�������------------------------------------------------------//
#ifdef CFG_FUNC_DETECT_MIC_EN
#define MIC1_DETECT_IE              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPIE)         
#define MIC1_DETECT_OE              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPOE) 
#define MIC1_DETECT_PU              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPPU) 
#define MIC1_DETECT_PD              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPPD)         
#define MIC1_DETECT_IN              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPIN)         
#define MIC1_DETECT_OUT             STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPOUT) 

#define MIC2_DETECT_IE              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPIE)         
#define MIC2_DETECT_OE              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPOE) 
#define MIC2_DETECT_PU              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPPU) 
#define MIC2_DETECT_PD              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPPD)         
#define MIC2_DETECT_IN              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPIN)         
#define MIC2_DETECT_OUT             STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPOUT) 
#endif
//-------------------------------------------------------------------------------//

//------3��/4�߶������ͼ��IO�Ĵ�������------------------------------ -----------//
#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
#define MIC_SEGMENT_IE             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPIE)         
#define MIC_SEGMENT_OE             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPOE) 
#define MIC_SEGMENT_PU             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPPU) 
#define MIC_SEGMENT_PD             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPPD)         
#define MIC_SEGMENT_IN             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPIN)         
#define MIC_SEGMENT_OUT            STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPOUT) 
#endif
//-------------------------------------------------------------------------------//

//------mic mute ����IO�Ĵ�������------------------------------------------------//
#define MIC_MUTE_CTL_IE            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPIE)         
#define MIC_MUTE_CTL_OE            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPOE) 
#define MIC_MUTE_CTL_PU            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPPU) 
#define MIC_MUTE_CTL_PD            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPPD)         
#define MIC_MUTE_CTL_IN            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPIN)         
#define MIC_MUTE_CTL_OUT           STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPOUT) 
//-------------------------------------------------------------------------------//

//------�������IO�Ĵ�������-----------------------------------------------------//
#if CFG_CHARGER_DETECT_EN
#define CHARGE_DETECT_IE           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPIE)         
#define CHARGE_DETECT_OE           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPOE) 
#define CHARGE_DETECT_PU           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPPU) 
#define CHARGE_DETECT_PD           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPPD)         
#define CHARGE_DETECT_IN           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPIN)         
#define CHARGE_DETECT_OUT          STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPOUT) 
#endif
//-------------------------------------------------------------------------------//

//------MUTE��·�����MUTE����IO�Ĵ�������---------------------------------------//
#define MUTE_CTL_IE               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPIE)         
#define MUTE_CTL_OE               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPOE) 
#define MUTE_CTL_PU               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPPU) 
#define MUTE_CTL_PD               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPPD)         
#define MUTE_CTL_IN               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPIN)         
#define MUTE_CTL_OUT              STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPOUT) 
//-------------------------------------------------------------------------------//

//------LED����IO�Ĵ�������------------------------------------------------------//
#if CFG_LED_EN
#define POWER_LED_IE              STRING_CONNECT(GPIO,POWER_LED_PORT,GPIE)         
#define POWER_LED_OE              STRING_CONNECT(GPIO,POWER_LED_PORT,GPOE) 
#define POWER_LED_PU              STRING_CONNECT(GPIO,POWER_LED_PORT,GPPU) 
#define POWER_LED_PD              STRING_CONNECT(GPIO,POWER_LED_PORT,GPPD)         
#define POWER_LED_IN              STRING_CONNECT(GPIO,POWER_LED_PORT,GPIN)         
#define POWER_LED_OUT             STRING_CONNECT(GPIO,POWER_LED_PORT,GPOUT) 

#define EAR_LED_IE                STRING_CONNECT(GPIO,EAR_LED_PORT,GPIE)         
#define EAR_LED_OE                STRING_CONNECT(GPIO,EAR_LED_PORT,GPOE) 
#define EAR_LED_PU                STRING_CONNECT(GPIO,EAR_LED_PORT,GPPU) 
#define EAR_LED_PD                STRING_CONNECT(GPIO,EAR_LED_PORT,GPPD)         
#define EAR_LED_IN                STRING_CONNECT(GPIO,EAR_LED_PORT,GPIN)         
#define EAR_LED_OUT               STRING_CONNECT(GPIO,EAR_LED_PORT,GPOUT) 

#define SONG_LED_IE               STRING_CONNECT(GPIO,SONG_LED_PORT,GPIE)         
#define SONG_LED_OE               STRING_CONNECT(GPIO,SONG_LED_PORT,GPOE) 
#define SONG_LED_PU               STRING_CONNECT(GPIO,SONG_LED_PORT,GPPU) 
#define SONG_LED_PD               STRING_CONNECT(GPIO,SONG_LED_PORT,GPPD)         
#define SONG_LED_IN               STRING_CONNECT(GPIO,SONG_LED_PORT,GPIN)         
#define SONG_LED_OUT              STRING_CONNECT(GPIO,SONG_LED_PORT,GPOUT) 

#define TALK_LED_IE               STRING_CONNECT(GPIO,TALK_LED_PORT,GPIE)         
#define TALK_LED_OE               STRING_CONNECT(GPIO,TALK_LED_PORT,GPOE) 
#define TALK_LED_PU               STRING_CONNECT(GPIO,TALK_LED_PORT,GPPU) 
#define TALK_LED_PD               STRING_CONNECT(GPIO,TALK_LED_PORT,GPPD)         
#define TALK_LED_IN               STRING_CONNECT(GPIO,TALK_LED_PORT,GPIN)         
#define TALK_LED_OUT              STRING_CONNECT(GPIO,TALK_LED_PORT,GPOUT) 

#define KTV_LED_IE               STRING_CONNECT(GPIO,KTV_LED_PORT,GPIE)         
#define KTV_LED_OE               STRING_CONNECT(GPIO,KTV_LED_PORT,GPOE) 
#define KTV_LED_PU               STRING_CONNECT(GPIO,KTV_LED_PORT,GPPU) 
#define KTV_LED_PD               STRING_CONNECT(GPIO,KTV_LED_PORT,GPPD)         
#define KTV_LED_IN               STRING_CONNECT(GPIO,KTV_LED_PORT,GPIN)         
#define KTV_LED_OUT              STRING_CONNECT(GPIO,KTV_LED_PORT,GPOUT) 

#define AUTO_TUNE_LED_IE         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPIE)         
#define AUTO_TUNE_LED_OE         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPOE) 
#define AUTO_TUNE_LED_PU         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPPU) 
#define AUTO_TUNE_LED_PD         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPPD)         
#define AUTO_TUNE_LED_IN         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPIN)         
#define AUTO_TUNE_LED_OUT        STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPOUT) 

#define SHUNNING_LED_IE         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPIE)         
#define SHUNNING_LED_OE         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPOE) 
#define SHUNNING_LED_PU         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPPU) 
#define SHUNNING_LED_PD         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPPD)         
#define SHUNNING_LED_IN         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPIN)         
#define SHUNNING_LED_OUT        STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPOUT) 
#endif
//-------------------------------------------------------------------------------//


/*
**************************************************************************************
*  GPIOӦ�õ��õĺ궨��
*  
   ע��1.����������Ҫʱ�ɶ���յĺ�;
       2.������Ҫ����ʵ�ʵİ��ӣ�������ȷ�Ŀ��Ƹߵ��߼���ƽ
**************************************************************************************
*/

//------LED������غ궨��--------------------------------------------------------//

#if CFG_LED_EN
#define EAR_LED_OFF()	  do{\
							 GPIO_RegOneBitClear(EAR_LED_IE, EAR_LED_PIN);\
							 GPIO_RegOneBitSet(EAR_LED_OE, EAR_LED_PIN);\
							 GPIO_RegOneBitClear(EAR_LED_OUT, EAR_LED_PIN);\
							 }while(0)
							 
#define EAR_LED_ON()	   do{\
							 GPIO_RegOneBitClear(EAR_LED_IE, EAR_LED_PIN);\
							 GPIO_RegOneBitSet(EAR_LED_OE, EAR_LED_PIN);\
							 GPIO_RegOneBitSet(EAR_LED_OUT, EAR_LED_PIN);\
							 }while(0)
							 
#define SONG_LED_OFF()	  do{\
						    GPIO_RegOneBitClear(SONG_LED_IE, SONG_LED_PIN);\
						    GPIO_RegOneBitSet(SONG_LED_OE, SONG_LED_PIN);\
						    GPIO_RegOneBitSet(SONG_LED_OUT, SONG_LED_PIN);\
							}while(0)
								 
#define SONG_LED_ON()	   do{\
						     GPIO_RegOneBitClear(SONG_LED_IE, SONG_LED_PIN);\
						     GPIO_RegOneBitSet(SONG_LED_OE, SONG_LED_PIN);\
						     GPIO_RegOneBitClear(SONG_LED_OUT, SONG_LED_PIN);\
							 }while(0)
							 
#define TALK_LED_OFF()	  do{\
							 GPIO_RegOneBitClear(TALK_LED_IE, TALK_LED_PIN);\
							 GPIO_RegOneBitSet(TALK_LED_OE, TALK_LED_PIN);\
							 GPIO_RegOneBitSet(TALK_LED_OUT, TALK_LED_PIN);\
							 }while(0)
							 
#define TALK_LED_ON()	   do{\
							 GPIO_RegOneBitClear(TALK_LED_IE, TALK_LED_PIN);\
							 GPIO_RegOneBitSet(TALK_LED_OE, TALK_LED_PIN);\
							 GPIO_RegOneBitClear(TALK_LED_OUT, TALK_LED_PIN);\
							 }while(0) 
							 
#define KTV_LED_OFF()	  do{\
							GPIO_RegOneBitClear(KTV_LED_IE, KTV_LED_PIN);\
							GPIO_RegOneBitSet(KTV_LED_OE, KTV_LED_PIN);\
							GPIO_RegOneBitSet(KTV_LED_OUT, KTV_LED_PIN);\
							}while(0)
								 
#define KTV_LED_ON()	   do{\
							GPIO_RegOneBitClear(KTV_LED_IE, KTV_LED_PIN);\
							GPIO_RegOneBitSet(KTV_LED_OE, KTV_LED_PIN);\
							GPIO_RegOneBitClear(KTV_LED_OUT, KTV_LED_PIN);\
							}while(0)
                          
#define AUTO_LED_OFF()	  do{\
							 GPIO_RegOneBitClear(AUTO_TUNE_LED_IE, AUTO_TUNE_LED_PIN);\
							 GPIO_RegOneBitSet(AUTO_TUNE_LED_OE, AUTO_TUNE_LED_PIN);\
							 GPIO_RegOneBitSet(AUTO_TUNE_LED_OUT, AUTO_TUNE_LED_PIN);\
							 }while(0)
							 
#define AUTO_LED_ON()	   do{\
                        	 GPIO_RegOneBitClear(AUTO_TUNE_LED_IE, AUTO_TUNE_LED_PIN);\
                        	 GPIO_RegOneBitSet(AUTO_TUNE_LED_OE, AUTO_TUNE_LED_PIN);\
                        	 GPIO_RegOneBitClear(AUTO_TUNE_LED_OUT, AUTO_TUNE_LED_PIN);\
							 }while(0)    
							 
#define SHUNNING_LED_OFF()	do{\
							GPIO_RegOneBitClear(SHUNNING_LED_IE, SHUNNING_LED_PIN);\
							GPIO_RegOneBitSet(SHUNNING_LED_OE, SHUNNING_LED_PIN);\
							GPIO_RegOneBitSet(SHUNNING_LED_OUT, SHUNNING_LED_PIN);\
							}while(0)
								 
#define SHUNNING_LED_ON()  do{\
							GPIO_RegOneBitClear(SHUNNING_LED_IE, SHUNNING_LED_PIN);\
							GPIO_RegOneBitSet(SHUNNING_LED_OE, SHUNNING_LED_PIN);\
							GPIO_RegOneBitClear(SHUNNING_LED_OUT, SHUNNING_LED_PIN);\
							}while(0)

#define POWER_LED_ON()     do{\
						 GPIO_RegOneBitClear(POWER_LED_IE, POWER_LED_PIN);\
						 GPIO_RegOneBitSet(POWER_LED_OE, POWER_LED_PIN);\
						 GPIO_RegOneBitClear(POWER_LED_OUT, POWER_LED_PIN);\
                         }while(0)
#define POWER_LED_OFF()     do{\
						 GPIO_RegOneBitClear(POWER_LED_IE, POWER_LED_PIN);\
						 GPIO_RegOneBitSet(POWER_LED_OE, POWER_LED_PIN);\
						 GPIO_RegOneBitSet(POWER_LED_OUT, POWER_LED_PIN);\
                         }while(0)

#endif
//-------------------------------------------------------------------------------//

//------�ⲿMUTE��·����ſ�����غ궨��-----------------------------------------//
#define MUTE_OFF()	  do{\
						 GPIO_RegOneBitClear(MUTE_CTL_IE, MUTE_CTL_PIN);\
						 GPIO_RegOneBitSet(MUTE_CTL_OE, MUTE_CTL_PIN);\
						 GPIO_RegOneBitSet(MUTE_CTL_OUT, MUTE_CTL_PIN);\
						 }while(0)
						 
#define MUTE_ON()	   do{\
						 GPIO_RegOneBitClear(MUTE_CTL_IE, MUTE_CTL_PIN);\
						 GPIO_RegOneBitSet(MUTE_CTL_OE, MUTE_CTL_PIN);\
						 GPIO_RegOneBitClear(MUTE_CTL_OUT, MUTE_CTL_PIN);\
						 }while(0)
//-------------------------------------------------------------------------------//


//------MIC MUTE������غ궨��---------------------------------------------------//
#define MIC_MUTE_ON()	  do{\
						 GPIO_RegOneBitClear(MIC_MUTE_CTL_IE, MIC_MUTE_CTL_PIN);\
						 GPIO_RegOneBitSet(MIC_MUTE_CTL_OE, MIC_MUTE_CTL_PIN);\
						 GPIO_RegOneBitSet(MIC_MUTE_CTL_OUT, MIC_MUTE_CTL_PIN);\
						 }while(0)
						 
#define MIC_MUTE_OFF()	   do{\
						 GPIO_RegOneBitClear(MIC_MUTE_CTL_IE, MIC_MUTE_CTL_PIN);\
						 GPIO_RegOneBitSet(MIC_MUTE_CTL_OE, MIC_MUTE_CTL_PIN);\
						 GPIO_RegOneBitClear(MIC_MUTE_CTL_OUT, MIC_MUTE_CTL_PIN);\
						 }while(0)					 
//-------------------------------------------------------------------------------//


//------�ⲿ�ܵ�Դ������غ궨��-------------------------------------------------//
#define POWER_ON()     do{\
						 GPIO_RegOneBitClear(POWER_CTL_IE, POWER_CTL_PIN);\
						 GPIO_RegOneBitSet(POWER_CTL_OE, POWER_CTL_PIN);\
						 GPIO_RegOneBitSet(POWER_CTL_OUT, POWER_CTL_PIN);\
                         }while(0);
#define POWER_OFF()     do{\
						 GPIO_RegOneBitClear(POWER_CTL_IE, POWER_CTL_PIN);\
						 GPIO_RegOneBitSet(POWER_CTL_OE, POWER_CTL_PIN);\
						 GPIO_RegOneBitClear(POWER_CTL_OUT, POWER_CTL_PIN);\
                         }while(0);
//-------------------------------------------------------------------------------//

#endif //end of __USER_HW_INTERFACE__
