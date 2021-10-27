copyLibs.exe拷贝脚本使用说明
此脚本属于原copyLibs的升级版本，目的是为了满足复杂的蓝牙宏开关逻辑对蓝牙库的拷贝动作执行
SDK人员可以方便得使用逻辑运算语法来达到拷贝对应库的工作流程。
唯一需要做的就是编写script_copy.ini文件，以下将script_copy.ini各字段的功能做详细解释：

[main_config].home_dir ：
指定基准目录。由于AndesIDE编译期的当前目录比较深，在指定文件相对路径是，需要使用多个../../../回退到项目根目录。因此使用这个字段来对以下描述的所有路径进行简化。
对B1X项目来讲 main_config.home_dir = ../../../，之后的相对路径可以以MVsB1_Base_SDK起始或者BT_Audio_APP起始。

[main_config].default_dest_lib ：
定义缺省库目的地址。当某条表达式的拷贝目的路径没有指定时，使用此缺省路径。

[define_files].fileX ：
需要扫描的头文件，可以指定多条，以file1，file2，file3。。。为顺序定义。

[macro_declare]
定义需要寻找的宏开关，凡是在[macro_express]表达式中出现的宏开关必须在此定义，否则会出现错误。
宏开关定义后，它的初始值值可以定义为0或1。
0对应未#define或者DISABLE
1对应#define或者ENABLE

[macro_express]
定义宏开关的表达式以及相应的拷贝动作。
"="左边为对宏开关进行逻辑运算，宏开关的值由搜索的头文件定义。

当宏开关#define，以及#define为ENABLE时，此宏开关的值为1，如：
#define BT_TWS_SUPPORT
#define BT_TWS_SUPPORT ENABLE

当宏开关未#define，以及#define为DISABLE时，此宏开关的值为0，如：
//#define BT_TWS_SUPPORT
#define BT_TWS_SUPPORT DISABLE

"="左边为宏开关表达式使用这些值做逻辑运算，运算符包含：
not 非
and 与
or 或
() 括号

等号"="右边为拷贝动作定义，使用">"分割开。
">"左边为拷贝源，">"右边为拷贝目的。当拷贝目的未指定时，使用缺省的[main_config].default_dest_lib（此时">"不用写）
source > dest 即 copy source to dest

[default_copy].copyX
定义缺省拷贝动作：当[macro_express]中的表达式无一满足时，进行一下拷贝动作，可以定义多条也可以不定义。