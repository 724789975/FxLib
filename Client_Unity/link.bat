echo off & color 0A
setlocal enabledelayedexpansion
::ָ����ʼ�ļ���
set DIR="%cd%"\Test
echo DIR=%DIR%

:: ���� /R ��ʾ��Ҫ�������ļ���,ȥ����ʾ���������ļ���
:: %%f ��һ������,�����ڵ�����,�����������ֻ����һ����ĸ���,ǰ�����%%
:: ��������ͨ���,����ָ����׺��,*.*��ʾ�����ļ�

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
