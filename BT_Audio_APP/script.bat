..\..\MVsB1_Base_SDK\tools\remind_script\MergeAudio2BinNew.exe -a 0x0 -i ..\..\..\BT_Audio_APP\remind_file
fc ..\..\MVsB1_Base_SDK\tools\remind_script\sound_remind_item.h .\..\bt_audio_app_src\inc\remind_sound_item.h
if %errorlevel%==1 (copy ..\..\MVsB1_Base_SDK\tools\remind_script\sound_remind_item.h .\..\bt_audio_app_src\inc\remind_sound_item.h)
..\..\MVsB1_Base_SDK\tools\libs_copy_script\copyLibs.exe -s ..\..\..\BT_Audio_APP\script_copy.ini
