@echo off
setlocal
cd /d %~dp0

set CMAKE="cmake.exe2"
set opt=

:retry_vs
echo 1. Visual Studio 15 2017
echo 2. Visual Studio 14 2015
echo 3. Visual Studio 12 2013
echo 4. Visual Studio 11 2012
echo 5. Visual Studio 10 2010
echo 6. Visual Studio 9 2008
echo 7. Visual Studio 8 2005
rem echo 9. Visual Studio NMake (experimental)
rem echo z. Cygwin MinGW Release + Unix Makefiles (experimental)
set /p no="select no "

echo %no%
if "%no%" == "1" set GENERATOR="Visual Studio 15 2017" & goto build_all
if "%no%" == "2" set GENERATOR="Visual Studio 14 2015" & goto build_all
if "%no%" == "3" set GENERATOR="Visual Studio 12 2013" & goto build_all
if "%no%" == "4" set GENERATOR="Visual Studio 11 2012" & goto build_all
if "%no%" == "5" set GENERATOR="Visual Studio 10 2010" & goto build_all
if "%no%" == "6" set GENERATOR="Visual Studio 9 2008" & goto build_all
if "%no%" == "7" set GENERATOR="Visual Studio 8 2005" & call :cmake_3_11_4 & goto build_all_2
if "%no%" == "9" set GENERATOR="NMake Makefiles" & set OPT=-DCMAKE_BUILD_TYPE=Release & goto build_all
if "%no%" == "z" set GENERATOR="Unix Makefiles" & set OPT=-DCMAKE_TOOLCHAIN_FILE=../mingw.toolchain.cmake & goto build_all
echo ? retry
goto retry_vs

:build_all
where %CMAKE%
if %errorlevel% == 0 goto build_all_2
set CMAKE="C:\Program Files\CMake\bin\cmake.exe"
if exist %CMAKE% echo %CMAKE% ���g�p & goto build_all_2
echo cmake��������܂���
pause
exit

:build_all_2
set C=%CMAKE% -DCMAKE_GENERATOR=%GENERATOR% %OPT% -P buildall.cmake
echo %C%
title %C%
pause
%C%

:finish
echo build complete
endlocal
pause
exit

:cmake_3_11_4
set CMAKE=%~dp0cmake-3.11.4-win32-x86\bin\cmake.exe
echo 1. ������cmake.exe���g�p����(PATH��ʂ��Ă���)
echo 2. VS2005�ł��g�p�ł���cmake�g�p����
echo    (�K�v�Ȃ玩���Ń_�E�����[�h���āA���̃t�H���_�ɃC���X�g�[������)
if exist %CMAKE% echo    �C���X�g�[���ς�(%CMAKE%)

set /p no="select no "
echo %no%
if "%no%" == "2" goto download

set CMAKE="cmake.exe"
where %CMAKE%
goto finish_cmake

:download
if exist %CMAKE% goto finish_cmake
call getcmake.bat

:finish_cmake
exit /b
