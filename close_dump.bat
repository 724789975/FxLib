@echo off
echo 正在关闭Dump...
reg delete "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps" /f
echo Dump已经关闭
pause
@echo on
