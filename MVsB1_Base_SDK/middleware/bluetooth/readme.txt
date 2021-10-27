针对各种应用需求，定制了几个版本的库，说明如下：
libBtStack.a (由脚本自动生成)
当前应用使用的蓝牙库

libBtStack_Std.a
通用功能的蓝牙库，包含BLE，BT的A2DP，AVRCP，HFP，SPP，PBAP，HID等, 

libBtStack_Std_AvrcpBrws.a
在通用蓝牙库基础上，增加了avrcp browse功能；ram消耗增加20K+；

libBtStack_Std_NoBle.a
在通用蓝牙库基础上，去掉了BLE功能

libBtStack_Std_NoBle_NoHfp.a
在通用蓝牙库基础上，去掉了BLE和HFP功能；

libBtStack_Tws_Peer.a
TWS Peer 对箱蓝牙库

libBtStack_Tws_Peer_NoBle.a
TWS Peer 对箱蓝牙库基础上，去掉了BLE功能

libBtStack_Tws_Peer_NoBle_NoHfp.a
TWS Peer 对箱蓝牙库基础上，去掉了BLE和HFP功能

libBtStack_Tws_Soundbar.a
TWS Soundbar 蓝牙库

自动化脚本功能：
启用脚本Script后，编译前，IDE会根据appconfig自动选择 通用/TWS对箱/TWSsoundbar匹配的蓝牙库复制为libBtStack.a