#!/bin/bash

filepath=$(dirname "$0")
#echo $filepath

function State()
{
	#strState=""
	bLuaTestRun=0
	for strState in `ps -Af | grep LuaTest | grep -v "LuaTest.sh" | grep -v "grep" | awk '{print $2"--路径--"$8}'`;
	do
		#$bLuaTestRun=1
		echo "pid $strState 正在运行"
	done

	#if [ 0 = $bLuaTestRun ]; then
	#	echo "你想要查找的LuaTest没有运行"
	#fi
}

function Stop()
{
	for proce_id in `ps -A | grep LuaTest | grep -v "LuaTest.sh" | awk '{print $1}'`;
	do
		`kill -9 $proce_id`
		echo "proce_id $proce_id killed"
	done
}

function Start()
{
	strCmd="nohup %filepath/LuaTest > $filepath/LuaTest.log &"
	`nohup "$filepath/LuaTest" > "$filepath/LuaTest.log" &`
	#echo $strCmd
	`$strCm`
	echo "$filepath/LuaTest start success!!!"
}

if [ $# = 0 ]; then
	echo "必须要有一个参数 restart 或者 stop 或者 state"
elif [ $# -gt 1 ]; then
	echo "只能有一个参数 restart 或者 stop 或者 state"
elif [ $1 = "restart" ]; then
	Stop
	Start
elif [ $1 = "stop" ]; then
	Stop
elif [ $1 = "state" ]; then
	State
else
	echo "参数输入错误 必须为 restart 或者 stop"
fi
