Demo_Encoder������Ҫ��ʾ��mic���뵽DAC������ұ����sbc��msbc�ȸ�ʽ���ݱ��浽TF����
ʾ��������������� BP10ϵ�п����塣

Ӳ������Ҫ��:
    1. ʹ�� BP10ϵ�п����壬��Ӵ���С�壬TX/RX/GND ���� A25/A24/GND ���� (������ 256000)��
    2. SDCARD�ӿ�ʹ��GPIOA20��GPIOA21��GPIOA22

���ܣ�
1.��ʾMIC1��MIC2���룬DAC������̡�
2.�ϵ����TF����
3.DEBUG��ӡ������'r'����ʼ����¼�����ٴΰ���'r'�󣬽���¼�������ļ��� 

��ʾ���ṩ�ı����ʽ������sbc������msbc����mp3��mp2��adpcm
����ͨ��
#define ENC_TYPE_SBC 0
#define ENC_TYPE_MSBC 1
#define ENC_TYPE_MP3 2
#define ENC_TYPE_MP2 3
#define ENC_TYPE_ADPCM 4
#define ENC_TYPE ENC_TYPE_ADPCM
�л���ǰ�ı����ʽ��