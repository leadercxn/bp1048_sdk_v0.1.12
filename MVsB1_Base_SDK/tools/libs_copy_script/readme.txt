copyLibs.exe�����ű�ʹ��˵��
�˽ű�����ԭcopyLibs�������汾��Ŀ����Ϊ�����㸴�ӵ������꿪���߼���������Ŀ�������ִ��
SDK��Ա���Է����ʹ���߼������﷨���ﵽ������Ӧ��Ĺ������̡�
Ψһ��Ҫ���ľ��Ǳ�дscript_copy.ini�ļ������½�script_copy.ini���ֶεĹ�������ϸ���ͣ�

[main_config].home_dir ��
ָ����׼Ŀ¼������AndesIDE�����ڵĵ�ǰĿ¼�Ƚ����ָ���ļ����·���ǣ���Ҫʹ�ö��../../../���˵���Ŀ��Ŀ¼�����ʹ������ֶ�������������������·�����м򻯡�
��B1X��Ŀ���� main_config.home_dir = ../../../��֮������·��������MVsB1_Base_SDK��ʼ����BT_Audio_APP��ʼ��

[main_config].default_dest_lib ��
����ȱʡ��Ŀ�ĵ�ַ����ĳ�����ʽ�Ŀ���Ŀ��·��û��ָ��ʱ��ʹ�ô�ȱʡ·����

[define_files].fileX ��
��Ҫɨ���ͷ�ļ�������ָ����������file1��file2��file3������Ϊ˳���塣

[macro_declare]
������ҪѰ�ҵĺ꿪�أ�������[macro_express]���ʽ�г��ֵĺ꿪�ر����ڴ˶��壬�������ִ���
�꿪�ض�������ĳ�ʼֵֵ���Զ���Ϊ0��1��
0��Ӧδ#define����DISABLE
1��Ӧ#define����ENABLE

[macro_express]
����꿪�صı��ʽ�Լ���Ӧ�Ŀ���������
"="���Ϊ�Ժ꿪�ؽ����߼����㣬�꿪�ص�ֵ��������ͷ�ļ����塣

���꿪��#define���Լ�#defineΪENABLEʱ���˺꿪�ص�ֵΪ1���磺
#define BT_TWS_SUPPORT
#define BT_TWS_SUPPORT ENABLE

���꿪��δ#define���Լ�#defineΪDISABLEʱ���˺꿪�ص�ֵΪ0���磺
//#define BT_TWS_SUPPORT
#define BT_TWS_SUPPORT DISABLE

"="���Ϊ�꿪�ر��ʽʹ����Щֵ���߼����㣬�����������
not ��
and ��
or ��
() ����

�Ⱥ�"="�ұ�Ϊ�����������壬ʹ��">"�ָ��
">"���Ϊ����Դ��">"�ұ�Ϊ����Ŀ�ġ�������Ŀ��δָ��ʱ��ʹ��ȱʡ��[main_config].default_dest_lib����ʱ">"����д��
source > dest �� copy source to dest

[default_copy].copyX
����ȱʡ������������[macro_express]�еı��ʽ��һ����ʱ������һ�¿������������Զ������Ҳ���Բ����塣