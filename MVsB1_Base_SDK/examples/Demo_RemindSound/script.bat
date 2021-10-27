..\..\..\tools\remind_script\MergeAudio2BinNew.exe -a 0x0 -i ..\..\examples\Demo_RemindSound\remind_file
fc ..\..\..\tools\remind_script\sound_remind_item.h .\..\src\remind_sound_item.h
if %errorlevel%==1 (copy ..\..\..\tools\remind_script\sound_remind_item.h .\..\src\remind_sound_item.h)


