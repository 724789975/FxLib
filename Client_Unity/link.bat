echo off & color 0A
setlocal enabledelayedexpansion
::指定起始文件夹
set DIR="%cd%"\Test
echo DIR=%DIR%

:: 参数 /R 表示需要遍历子文件夹,去掉表示不遍历子文件夹
:: %%f 是一个变量,类似于迭代器,但是这个变量只能由一个字母组成,前面带上%%
:: 括号中是通配符,可以指定后缀名,*.*表示所有文件

for /R %DIR% %%f in (*.png *.cs *.dll *.cpp *.jslib *.html *.js *.prefab *.unity *.ttf *.mat *.shader *.anim *.asset *.h *.c *.lua *.xml *.pl) do (
	set "var=%%f"
	set "p=%%f"
	echo !p!
	set "var=!var:Test=Test_Link!"
	echo !var!
	set "p1=D:%%~pf"
	set "p1=!p1:Test=Test_Link!"
	echo !p1!
	md !p1!

	del !var!
	mklink /h !var! !p!
)
pause
