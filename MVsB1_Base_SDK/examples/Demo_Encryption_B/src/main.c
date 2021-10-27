/* * @brief   Encryption Demo B
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2019-12-31 11:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "math.h"
#include "type.h"
#include "debug.h"
#include "delay.h"
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "timeout.h"
#include "timer.h"
#include "clk.h"
#include "watchdog.h"
#include "remap.h"
#include "spi_flash.h"
#include "chip_info.h"


unsigned short Test[256];

extern unsigned short CRC16(unsigned char* buf,unsigned int buflen,unsigned short CRC);
extern void __c_init_rom(void);
extern void MemTest(void);
int main(void)
{
	uint32_t i;
	unsigned short CRCTest1=0, CRCTest2;

	Chip_Init(1);
	WDG_Disable();

	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

	Clock_Config(1, 24000000);
	Clock_PllLock(288000);
	Clock_APllLock(288000);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_UARTClkSelect(PLL_CLK_MODE);

	GPIO_PortAModeSet(GPIOA24, 1);//Rx, A24:uart1_rxd_0
	GPIO_PortAModeSet(GPIOA25, 6);//Tx, A25:uart1_txd_0
	DbgUartInit(1, 256000, 8, 0, 1);

	//Ӧ�ô��빤����ʼ��ַ����Ҫ��sag�ļ���һ��
	//flash����������Ҫcopy��RAM�У������ַ��һ��ϵͳ���г���
	//Remap_InitTcm(0, 12);
	Remap_InitTcm(0x70000, 12);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

	//�㷨���̵�ȫ�ֱ�����ʼ��
	//����㷨����û��ȫ�ֱ�������ע�͵����д���
	__c_init_rom();

	DBG("\n");
	DBG("/-----------------------------------------------------\\\n");
	DBG("|                     Encryption Example              |\n");
	DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
	DBG("\\-----------------------------------------------------/\n");
	DBG("\n");

	//����Ϊ�㷨���̵�API���ã���ʾ����
	for(i=0; i<256; i++)
	{
		Test[i] = i;
	}
	CRCTest2 = CRC16((uint8_t *)Test, sizeof(Test), CRCTest1);

	DBG("CRCTest2 = %x\n", CRCTest2);

	MemTest();
	{
		extern unsigned int SamTest;
		extern unsigned int SamTestbuf[12];
		uint32_t i;
		DBG("SamTest = %d\n", SamTest);
		for(i=0; i<12; i++)
		{
			DBG("SamTestbuf[%u] = %u\n", (unsigned int)i, (unsigned int)SamTestbuf[i]);
		}
	}
	//������ʾ����

	while(1);

}
