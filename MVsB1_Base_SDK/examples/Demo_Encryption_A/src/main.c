/* * @brief   Encryption Demo A
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2019-12-31 11:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef TEST_PROJECT
void _start(void)//for remove warning
{
}
//ע��˶λ����룬���Դ���϶̣������������
//�㷨�ͻ����븴��֮�󣬴˶δ������ɾ��
//�˻���һ����תָ��Ϊ�����ã�����ʱͨ��оƬ��¼��ַ��ת
__attribute__ ((section(".stub_section"))) __attribute__((naked))
void stub(void)
{
	__asm__ __volatile__(
//						"movi55 $r0,#0x0\n"
//						"sethi $r0,#0x80\n"
//						"jral5 $r0\n"
						".long 0x00800348 \n\n"//48����תָ�0x038000��ʾ448/2=224K������һ��ʼ���о���ת��448KB�ĵط�
		    			".rept (0x4000-0x4)/4 \n\n"
						//".rept (0x80000)/4 \n\n"
    					".long 0xA5A5A5A5 \n\n"
		    			".endr \n\n"
						 ".short 0xFFFF \n\n");
}
#else
#define _M(A)   ".byte "#A" \n\n"
#define M(A)   _M(A)
#define ALG_AREA 0  //3bits  ��λ8 �㷨�ռ��ַ��ΧN��[0kB/64kB/128kB,,,,/448kB]
#define ALG_ICODE_SIZE 0  //5bits ��λ29  �㷨�ռ���Icode ��ΧM��[0kB/16kB/32kB/,,/n*16kB/448kB]

/******************��������������**************************/
//#define ALG_SIZE 0xFFFFFFFF //3���ֽ�codesize  1�ֽ�(��)checksum
//#define ALG_CRC_LEN 0xFFFFFFFF //��ҪУ��ĳ��ȣ�ȥ���������� ����
//#define ALG_CRC_VALUE 0xFFFFFFFF  //У��ֵ �ɼ��ܹ�������
#define ALG_AREA_SET ((ALG_AREA<<5) + ALG_ICODE_SIZE)
#define ALG_ENCYPTION_FLAG 0x00  //0x00:������    0x55:����

__attribute__ ((section(".stub_section"))) __attribute__((naked))
void stub(void)
{
	__asm__ __volatile__(
		//".long 0x00800348 \n\n"//48����תָ�0x038000��ʾ448/2=224K������һ��ʼ���о���ת��448KB�ĵط�
    	".long 0xFFFFFFFF \n\n" //0x00  ALG_SIZE
		".long 0xFFFFFFFF \n\n" //0x04  ALG_CRC_LEN
		".long 0xFFFFFFFF \n\n" //0x08  ALG_CRC_VALUE
		M(ALG_ENCYPTION_FLAG)  //0x0c  ���ܱ�־
		".byte 0xFF \n\n"//0x0d
		".short 0xFFFF \n\n"//0x0e~0x0f     0x00~0x0f������CRCУ��
		M(ALG_AREA_SET)  //0x10  �㷨�ռ���ICODE�ռ�����
		".byte 0xFF \n\n"//0x11
		".short 0xFFFF \n\n"//0x12~0x13     0x10~0x13����CRCУ�飬��ֹ�ֶ��޸�
	);
}
#endif



const unsigned short crc_ccitt_table[256] =
{   0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


unsigned short CRC16(unsigned char* buf,unsigned int buflen,unsigned short CRC)
{
    unsigned  int i;
    for (i = 0 ; i < buflen ; i++) {
        CRC = (CRC << 8) ^ crc_ccitt_table[((CRC >> 8) ^ *(unsigned char*)buf ++) & 0x00FF];
    }
    return CRC;
}

////////////////////////////////////////
//����������ʾ����
//ע��:
//Demo_Encryption_A���̱��������С��Ӱ��Demo_Encryption_B���̵�sag�ļ�����
//2������һ��Ҫһ����ȷ������ϵͳ�������쳣
//����Demo_Encryption_A��ȫ�ֱ�������ע�͵����´��룬�ͷ�code�ռ�
////////////////////////////////
unsigned int SamTest = 1;
unsigned int SamTestbuf[12];
void MemTest(void)
{
	unsigned int i;
	SamTest += 1;
	for(i=0; i<12; i++)
	{
		SamTestbuf[i] = i + 1;
	}
}

//ȫ�ֱ�����ZI���������ʼ������
//Demo_Encryption_B�����ڴ���Demo_Encryption_Aǰ��Ҫ���ã��������δ��ʼ��
void __c_init_rom()
{
/* Use compiler builtin memcpy and memset */
#define MEMCPY(des, src, n) __builtin_memcpy ((des), (src), (n))
#define MEMSET(s, c, n) __builtin_memset ((s), (c), (n))

	extern char _end;
	extern char __bss_start;
	int size;

	/* data section will be copied before we remap.
	 * We don't need to copy data section here. */
	extern char __data_lmastart_rom;
	extern char __data_start_rom;
	extern char _edata;

	/* Copy data section to RAM */
	size = &_edata - &__data_start_rom;
	MEMCPY(&__data_start_rom, &__data_lmastart_rom, size);

	/* Clear bss section */
	size = &_end - &__bss_start;
	MEMSET(&__bss_start, 0, size);
	return;
}
