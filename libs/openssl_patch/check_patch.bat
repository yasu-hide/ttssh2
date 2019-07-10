@echo off

set folder=openssl_patch
set cmdopt2=--binary --backup -p0
set cmdopt1=--dry-run %cmdopt2%

rem
echo OpenSSL 1.1.1�Ƀp�b�`���K�p����Ă��邩���m�F���܂�...
echo.
rem

rem �p�b�`�R�}���h�̑��݃`�F�b�N
set patchcmd="patch.exe"
if exist %patchcmd% (goto cmd_true) else goto cmd_false

:cmd_true


rem �p�b�`�̓K�p�L�����`�F�b�N
:patch1
findstr /c:"# undef AI_PASSIVE" ..\openssl\crypto\bio\bio_lcl.h
if ERRORLEVEL 1 goto fail1
goto patch3
:fail1
pushd ..
%folder%\patch %cmdopt1% < %folder%\ws2_32_dll_patch.txt
%folder%\patch %cmdopt2% < %folder%\ws2_32_dll_patch.txt
popd

rem �ȉ��͂��Ƃŏ���
:patch2
findstr /c:"running on Windows95" ..\openssl\crypto\threads_win.c
if ERRORLEVEL 1 goto fail2
goto patch3
:fail2
pushd ..
%folder%\patch %cmdopt1% < %folder%\InitializeCriticalSectionAndSpinCount_patch.txt
%folder%\patch %cmdopt2% < %folder%\InitializeCriticalSectionAndSpinCount_patch.txt
popd

:patch3
findstr /c:"myInitializeCriticalSectionAndSpinCount" ..\openssl\crypto\threads_win.c
if ERRORLEVEL 1 goto fail3
goto patch4
:fail3
pushd ..
%folder%\patch %cmdopt1% < %folder%\thread_win.txt
%folder%\patch %cmdopt2% < %folder%\thread_win.txt
popd




:patch4
echo "�p�b�`�͓K�p����Ă��܂�"
timeout 5
goto end

:patchfail
echo "�p�b�`���K�p����Ă��Ȃ��悤�ł�"
set /P ANS="���s���܂����H(y/n)"
if "%ANS%"=="y" (
  goto end
) else if "%ANS%"=="n" (
  echo "�o�b�`�t�@�C�����I�����܂�"
  exit /b
) else (
  goto fail
)

goto end

:cmd_false
echo �p�b�`�R�}���h %patchcmd% ��������܂���
echo ���L�T�C�g����_�E�����[�h���Ă�������
echo http://geoffair.net/projects/patch.htm
echo.
goto patchfail

:end
@echo on


