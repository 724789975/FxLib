@echo off
echo ���ڹر�Dump...
reg delete "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps" /f
echo Dump�Ѿ��ر�
pause
@echo on
