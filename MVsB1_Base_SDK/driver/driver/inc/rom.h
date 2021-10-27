/**
 *****************************************************************************
 * @file     rom.h
 * @author   Ben
 * @version  V1.1.0
 * @date     20-06-2019
 * @brief    ROM function application interface for B1
 *
 * @attention:
 * 1.Download MVA package to the bank b start address in flash(e.g. 0x100000)
 * 2.Execute the following code:
 *   ROM_BOOT_DBUpgrade_Apply(1 , 0x100000);//or ROM_BOOT_DBUpgrade_Apply(0,0);
 *   ROM_BOOT_DBUpgrade_Reset();
 * 3.The MCU will auto boot to dual bank upgrade function after reset.Please wait for some time before boot to a new application code.
 * 4.The upgrade executing time is related to the code size and CRC check size.Please refer to the relevant documentation.
 *
 *
 * <h2><center>&copy; COPYRIGHT 2019 MVSilicon </center></h2>
 */

#ifndef DRIVER_INC_ROM_H_
#define DRIVER_INC_ROM_H_

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/*
 * error code definition in dualbank upgrade progress
 * */
typedef enum _UPGRADE_ERRNO{
	ERRNO_NONE = 0,				//0  û�д���
	ERRNO_NO_MVA_ERR, 			//1  û���ҵ�MVA��
	ERRNO_MVA_CRC_ERR, 			//2 MVA CRC����
	ERRNO_FLASH_DRV_ERR, 		//3 FLASH DRV ��������FlashDrv��size����У�����
	ERRNO_FLASH_DRV_RUN_ERR, 	//4 FLASH DRV���д�������Flash��������
    ERRNO_FLASH_WRITE_ERR,      //5 FLASH д����
	ERRNO_FLASH_ERASE_ERR, 		//6 FLASH ��������
	ERRNO_CODE_SIZE_ERR,        //7    �����������Ӱ��Ĵ�С����
	ERRNO_UPGRADED_CRC_ERR, 	//8    ����CODE��CRCУ�����
	ERRNO_KEY_MISMATCH_ERR,		//9 key ��һ���Դ���
	ERRNO_DATA_SIZE_ERR,	    //10  ���������Ӱ���С����
	ERRNO_BANKA_CRC_ERR, 	    //11 BANK A crcУ�����
	STATE_NEED_UPGRADE,         //12  ��Ҫ����
	STATE_ONLY_DATA_UPGRADED,   //13  ����DATA�ɹ�������Ҫ����code
	STATE_NO_NEED_UPGRADE,      //14 ����Ҫ����
	STATE_UPGRADE_SUCCESS       //15  �����ɹ�
}ENUM_UPGRADE_ERRNO;//���16��״̬

/*****************************************************************************************************************
*		User Defined Register Operating functions:
*		    ROM_UserRegisterSet()
*		    ROM_UserRegisterClear()
*		    ROM_UserRegisterGet()
*		This 16 bits Register will lost data if power off,and will hold the data after system reset or MCU reset.
*		�û��Զ���Ĵ�����16bit���������� ��
*		�����������ᶪʧ  ���綪ʧ
******************************************************************************************************************/
void ROM_UserRegisterSet(uint16_t setData);

void ROM_UserRegisterClear(void);

uint16_t ROM_UserRegisterGet(void);

/**
 * @brief     Apply for dual bank upgrade
 *
 * @param[in] upgradeMode ��[0 1]
 *                1: mode 1,apply upgrade by assigning the bank b start address
 *                0: mode 0,apply upgrade by auto searching MAV package header that no need address
 * @param[in] bBankAddr  =0x1000*n n��(0 0x1000)  start address of bank b who store the MVA package header
 * @return    apply result ��[-1 0 1]
 *                1  apply mode 1 success.
 *                -1 apply mode 1 failure.   Error address error
 *                0  apply mode 0 success.

 * ���ܣ�����˫bank����������������
 * ������upgradeMode��     upgradeMode = 1 : ģʽ1,����ָ��bank b��ַ����������.
 *                     upgradeMode = 0 : ģʽ0,����ǿ����������,�Զ���ѰMVA���׵�ַ.��ʱ������bBankAddr.
 *       bBankAddr:    MVA�����������׵�ַ���õ�ַҪ0x1000=4096���� ��
 * ����ֵ��
 *        1   ģʽ1���óɹ�
 *        -1  ģʽ1����ʧ��
 *        0   ģʽ0���óɹ�
 * ��ע�����������������ҪSYS RESET���ٽ���
 */
int8_t ROM_BankBUpgradeApply(int8_t upgradeMode,uint32_t bBankAddr);

/*
 * @brief   Get return code for dual bank upgrade
 *
 * ���ܣ���ȡ˫bank�����ķ�����
 */
ENUM_UPGRADE_ERRNO ROM_BankBUpgradeReturnCodeGet(void);

/*
 * @brief   SysReset after Applying for dual bank upgrade from ROM
 *
 * ���ܣ�����˫bank���������ϵͳ��λ,�˺����̻���ROM��
 */
void ROM_SysReset(void);

/*
 * @brief   Crc16 function from ROM
 *
 * ���ܣ�ʹ��ROM�й̻��˵�CRC16�ĺ���
 */
unsigned short ROM_CRC16(const char* Buf,unsigned long BufLen,unsigned short CRC);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif /* DRIVER_INC_ROM_H_ */
