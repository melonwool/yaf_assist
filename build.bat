@echo off
del /S /Q ..\..\x64\Debug\ext\yaf_assist
del /Q ..\..\x64\Debug\php_yaf_assist.dll*
cd ..\..
call buildconf.bat
call configure.bat --disable-all --disable-zts --enable-debug --with-mysqlnd --with-mysqli=mysqlnd --enable-cli --enable-yaf_assist
nmake
cd ext\yaf_assist
