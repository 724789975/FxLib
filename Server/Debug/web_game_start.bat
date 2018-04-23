@echo off
echo 正在关闭服务器组，请稍等..
taskkill /f /im WebGameCenter.exe
taskkill /f /im WebGameManager.exe
taskkill /f /im WebGameTeam.exe
taskkill /f /im WebGameLogin.exe
ping -n 2 127.0.0.1>nul
echo 服务器组关闭完成..

del /f /s /q *log.txt
echo 正在启动服务器组，请稍等..

start WebGameCenter --server_port 40000 

ping -n 1 127.0.0.1

start WebGameManager --server_id 30001 --server_port 30001 --player_port 30002 --center_ip 127.0.0.1 --center_port 40000
::ping -n 1 127.0.0.1
::start WebGameManager --server_id 30002 --server_port 30011 --player_port 30012 --center_ip 127.0.0.1 --center_port 40000

ping -n 1 127.0.0.1

start WebGameTeam --server_id 20001 --center_ip 127.0.0.1 --center_port 40000 -- game_manager_port 20001
::ping -n 1 127.0.0.1
::start WebGameTeam --server_id 20002 --center_ip 127.0.0.1 --center_port 40000 -- game_manager_port 20011

ping -n 1 127.0.0.1

start WebGameLogin --server_id 10001 --player_port 11001 --center_ip 127.0.0.1 --center_port 40000 --login_port 11002 --team_port 11003 --game_manager_port 11004
::ping -n 1 127.0.0.1
::start WebGameLogin --server_id 10002 --player_port 11011 --center_ip 127.0.0.1 --center_port 40000 --login_port 11012 --team_port 11013 --game_manager_port 11014


