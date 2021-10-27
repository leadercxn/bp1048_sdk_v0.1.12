#include <string.h>
#include "type.h"
#include "backup.h"
#include "backup_interface.h"
#include "clk.h"
#include "debug.h"
#include "chip_info.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

static bool BAKEUP_FirstFlag = TRUE;

void Backup_Clock(BACKUP_CLOCK Clock_Sel)
{
	if (Clock_Sel == BACKUP_CLK_32K_OSC)
	{
		BACKUP_32KEnable(OSC32K_SOURCE);
		BACKUP_32KSourceSelect(OSC32K_SOURCE);
		BACKUP_32KModeSelect(OSC32K_MODE_LOW_FREQ_OSC);
	}
	else if(Clock_Sel == BACKUP_CLK_32K_RC32)
	{
		BACKUP_32KEnable(RC32K_SOURCE);
		BACKUP_32KSourceSelect(RC32K_SOURCE);
		BACKUP_32KModeSelect(OSC32K_MODE_LOW_FREQ_OSC);
	}
	else if(Clock_Sel == BACKUP_CLK_32K_XM)
	{
		BACKUP_32KEnable(OSC32K_SOURCE);
		BACKUP_32KSourceSelect(OSC32K_SOURCE);
		BACKUP_32KModeSelect(xMHZ_MODE_HIGH_FREQ_OSC);
	}
}

uint32_t BACKUP_GetConfig(void)
{
	if(BACKUP_IsPowerKeyEnable())
	{
		//DBG("powerkey enable\n");
		if(BACKUP_PowerKeyModeGet() == SOFT_MODE)
		{
			//DBG("soft mode\n");
			return POWERKEY_MODE_PUSH_BUTTON;
		}
		else
		{
			//DBG("hard mode\n");
			if(BACKUP_PowerKeyHardModeGet() == LEVEL_TRIGGER)
			{
				//DBG("level:");
				if(BACKUP_PowerKeyActiveLevelGet() == LOW_INDICATE_POWERON)
				{
					//DBG("low run \n");
					return POWERKEY_MODE_SLIDE_SWITCH_HPD;
				}
				else
				{
					//DBG("high run \n");
					return POWERKEY_MODE_SLIDE_SWITCH_LPD;
				}
			}
		}
	}

	return POWERKEY_MODE_BYPASS;
}

void GetPowerupEvent()
{
	POWERUP_EVENT_SEL Powerup_src;

	Powerup_src = BACKUP_PowerupEventGet();

	printf("flag:%d\r\n",Powerup_src);

	switch(Powerup_src)
	{
	case SEL_VIN:
		BACKUP_PowerupEventClr();
		DBG("Powerup by VIN\n");
		break;
	case GPIO_C0:
		BACKUP_PowerupEventClr();
		DBG("Powerup by GPIOC0\n");
		break;
	case VALID_POWERKEY_EVENT:
		BACKUP_PowerupEventClr();
		DBG("Powerup by POWERKEY\n");
		break;
	case RTC_EVNET:
		BACKUP_PowerupEventClr();
		DBG("Powerup by RTC\n");
		break;
	default:
		BACKUP_PowerupEventClr();
		DBG("unknow mode powerup\n");
		break;
	}
}

//CountTime: 0(30us) -- 2000ms. step: 8ms
void SystemPowerKeyInit(uint32_t Mode, uint16_t CountTime)
{
	uint32_t PowerUpEvent = BACKUP_PowerupEventGet();
	BAKEUP_FirstFlag = BACKUP_FristPowerOnFlagGet();
	//bool IsPowerKerTrg = FALSE;
#ifdef CFG_FUNC_POWERKEY_EN
	int32_t Cnt = 3 * 1000 * (POWERKEY_CNT / 2);
#else
	int32_t Cnt = 3 * 1000 * 1000;
#endif
	if(BAKEUP_FirstFlag)
	{
		printf("first use Powerkey!!\r\n");
	}
	else
	{
		GetPowerupEvent();
	}

	switch(Mode)
	{
	case POWERKEY_MODE_BYPASS:
		//DBG("\npowerkey bypass mode!\n");
		BACKUP_PowerKeyDisable();
		break;
	case POWERKEY_MODE_PUSH_BUTTON:
		//if(TRUE == BAKEUP_FirstFlag)
		{
			CountTime /= 8;
			BACKUP_PowerKeyDisable();
			BACKUP_PowerKeyModeSet(SOFT_MODE);
			BACKUP_PowerKeyActiveLevelSet(LOW_INDICATE_POWERON);
			BACKUP_PowerKeyTrigMaxCntSet((uint8_t)CountTime);
			BACKUP_PowerKeyStateClear();
			BACKUP_FastPowerDownEnable();
			BACKUP_PowerKeyEnable();
			if((PowerUpEvent & (1 << 2)) == VALID_POWERKEY_EVENT
				|| (TRUE == BAKEUP_FirstFlag))//第一次上电需要按下PowerKey
			{
				DBG("first power on\n");
				while(1)
				{
					//如果系统上电同时按住PowerKey，则系统上电
					if(TRUE == BACKUP_PowerKeyTrigStateGet())
					{
						DBG("1212");
						//IsPowerKerTrg = TRUE;
						//break;
					}
					if(TRUE == BACKUP_PowerKeyPinStateGet())
					{
						DBG("Powen Down\n");
						//Backup_PowerDown();
						BACKUP_SystemPowerDown();
					}
					else
					{
						Cnt--;
						if(Cnt == 0)
						{
							break;
						}
					}
				}
			}
		}
		DBG("power on\n");
		//DBG("1:%d\n",BACKUP_PowerKeyTrigStateGet());
		BACKUP_PowerKeyStateClear();
		//DBG("2:%d\n",BACKUP_PowerKeyTrigStateGet());
		break;

	case POWERKEY_MODE_SLIDE_SWITCH_HPD:
	case POWERKEY_MODE_SLIDE_SWITCH_LPD:
		//if(TRUE == BAKEUP_FirstFlag)
		{
			CountTime /= 8;
			BACKUP_PowerKeyDisable();
			BACKUP_PowerKeyModeSet(HARD_MODE);
			BACKUP_PowerKeyHardModeSet(LEVEL_TRIGGER);//LEVEL_TRIGGER = 0

			if(Mode == POWERKEY_MODE_SLIDE_SWITCH_LPD)
			{
				BACKUP_PowerKeyActiveLevelSet(HIGH_INDICATE_POWERON);
			}
			else if(Mode == POWERKEY_MODE_SLIDE_SWITCH_HPD)
			{
				BACKUP_PowerKeyActiveLevelSet(LOW_INDICATE_POWERON);
			}
			BACKUP_PowerKeyTrigMaxCntSet((uint8_t)CountTime);
			//BACKUP_FastPowerDownEnable();
			BACKUP_PowerKeyEnable();
			BACKUP_PowerKeyStateClear();
			//DBG("1:%d\n",BACKUP_PowerKeyTrigStateGet());

			if(TRUE == BAKEUP_FirstFlag)
			{
				//DBG("first power on\n");
				if(Mode == POWERKEY_MODE_SLIDE_SWITCH_LPD)
				{
					if(FALSE == BACKUP_PowerKeyPinStateGet())
					{
						//DBG("Powen Down\n");
						//Backup_PowerDown();
						BACKUP_SystemPowerDown();
					}
				}
				else if(Mode == POWERKEY_MODE_SLIDE_SWITCH_HPD)
				{
					if(TRUE == BACKUP_PowerKeyPinStateGet())
					{
						//DBG("Powen Down\n");
						//Backup_PowerDown();
						BACKUP_SystemPowerDown();
					}
				}
			}
		}
		BACKUP_PowerKeyStateClear();
		//DBG("2:%d\n",BACKUP_PowerKeyTrigStateGet());
		//DBG("power on\n");
		break;
	default:
		break;
	}
}

bool SystemPowerKeyDetect(void)
{
	bool IsPowerKeyTrig = FALSE;
	switch(BACKUP_GetConfig())
	{
	case POWERKEY_MODE_BYPASS:
		break;
	case POWERKEY_MODE_PUSH_BUTTON:
		if((FALSE == BACKUP_PowerKeyPinStateGet()) && (BACKUP_PowerKeyActiveLevelGet() == LOW_INDICATE_POWERON))
		{
			if(TRUE == BACKUP_PowerKeyTrigStateGet())
			{
				BACKUP_PowerKeyStateClear();
				//return TRUE;
				IsPowerKeyTrig = TRUE;
			}
		}
		break;
	case POWERKEY_MODE_SLIDE_SWITCH_LPD:
	case POWERKEY_MODE_SLIDE_SWITCH_HPD:
		//if(BACKUP_PowerKeyTrigStateGet() == TRUE)
		{
			if(((BACKUP_PowerKeyPinStateGet() == FALSE) && (BACKUP_PowerKeyActiveLevelGet() == HIGH_INDICATE_POWERON))
			|| ((BACKUP_PowerKeyPinStateGet() == TRUE) && (BACKUP_PowerKeyActiveLevelGet() == LOW_INDICATE_POWERON)))
			{
				//DBG("\npowerdown:\n");
				//DBG("powerkey trig state:%d\n",BACKUP_PowerKeyTrigStateGet());
				BACKUP_PowerKeyStateClear();
				//DBG("powerkey trig state:%d\n",BACKUP_PowerKeyTrigStateGet());
				//return TRUE;
				IsPowerKeyTrig = TRUE;
			}
			//BACKUP_PowerKeyStateClear();
		}
		break;
	default:
		break;
	}
	//DBG("IsPowerKeyTrig = %d\n", IsPowerKeyTrig);
	return IsPowerKeyTrig;
}

void SystemPowerDown(void)
{
	switch(BACKUP_GetConfig())
	{
	case POWERKEY_MODE_BYPASS:
		break;

	case POWERKEY_MODE_PUSH_BUTTON:
		DBG("Push Button\n");
		DBG("do something ..., breakpoint\n");
		DBG("System goto PowerDown\n");

		//Backup_PowerDown();
		BACKUP_SystemPowerDown();
		break;
	case POWERKEY_MODE_SLIDE_SWITCH_LPD:
	case POWERKEY_MODE_SLIDE_SWITCH_HPD:
		//do something
		//DBG("System goto PowerDown\n");
		//DBG("*");
		//Backup_PowerDown();
		BACKUP_SystemPowerDown();
		break;
	default:
		break;
	}
}

void PowerKeyModeGet(void)
{
	switch (BACKUP_GetConfig())
	{
	case POWERKEY_MODE_BYPASS:
		DBG("powerkey bypass\n");
		break;
	case POWERKEY_MODE_PUSH_BUTTON:
		DBG("powerkey soft mode, powerkey default floating\n");
		break;
	
	case POWERKEY_MODE_SLIDE_SWITCH_LPD:
		DBG("powerkey level high active\n");
		break;
	case POWERKEY_MODE_SLIDE_SWITCH_HPD:
		DBG("powerkey level low active\n");
		break;
	default:
		DBG("unknow mode\n");
		break;
	}
}

