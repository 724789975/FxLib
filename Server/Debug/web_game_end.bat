@echo off
echo ���ڹرշ������飬���Ե�..
taskkill /f /im WebGameCenter.exe
taskkill /f /im WebGameManager.exe
taskkill /f /im WebGameTeam.exe
taskkill /f /im WebGameLogin.exe
ping -n 2 127.0.0.1>nul
echo ��������ر����..

