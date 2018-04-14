@echo off
echo 正在关闭服务器组，请稍等..
taskkill /f /im WebGameCenter.exe
taskkill /f /im WebGameManager.exe
taskkill /f /im WebGameTeam.exe
taskkill /f /im WebGameLogin.exe
ping -n 2 127.0.0.1>nul
echo 服务器组关闭完成..

