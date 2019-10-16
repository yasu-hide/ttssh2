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
rem freeaddrinfo/getnameinfo/getaddrinfo API(WindowsXP�ȍ~)�ˑ������̂���
findstr /c:"# undef AI_PASSIVE" ..\openssl\crypto\bio\bio_lcl.h
if ERRORLEVEL 1 goto fail1
goto patch2
:fail1
pushd ..
%folder%\patch %cmdopt1% < %folder%\ws2_32_dll_patch.txt
%folder%\patch %cmdopt2% < %folder%\ws2_32_dll_patch.txt
popd

:patch2
:patch3
:patch4


:patch5
rem WindowsMe��RAND_bytes�ŗ����錻�ۉ���̂��߁B
rem OpenSSL 1.0.2�ł�meth��NULL�`�F�b�N�����������AOpenSSL 1.1.1�łȂ��Ȃ��Ă���B
rem ����NULL�`�F�b�N�͂Ȃ��Ă����͂Ȃ��A�{����InitializeCriticalSectionAndSpinCount�ɂ��邽�߁A
rem �f�t�H���g�ł͓K�p���Ȃ����̂Ƃ���B
rem findstr /c:"added if meth is NULL pointer" ..\openssl\crypto\rand\rand_lib.c
rem if ERRORLEVEL 1 goto fail5
rem goto patch6
rem :fail5
rem pushd ..
rem %folder%\patch %cmdopt1% < %folder%\RAND_bytes.txt
rem %folder%\patch %cmdopt2% < %folder%\RAND_bytes.txt
rem popd


:patch6
rem WindowsMe��InitializeCriticalSectionAndSpinCount���G���[�ƂȂ錻�ۉ���̂��߁B
findstr /c:"myInitializeCriticalSectionAndSpinCount" ..\openssl\crypto\threads_win.c
if ERRORLEVEL 1 goto fail6
goto patch7
:fail6
pushd ..
%folder%\patch %cmdopt1% < %folder%\atomic_api.txt
%folder%\patch %cmdopt2% < %folder%\atomic_api.txt
popd


:patch7
rem Windows98/Me/NT4.0�ł�CryptAcquireContextW�ɂ��G���g���s�[�擾��
rem �ł��Ȃ����߁A�V����������ǉ�����BCryptAcquireContextW�̗��p�͎c���B
findstr /c:"CryptAcquireContextA" ..\openssl\crypto\rand\rand_win.c
if ERRORLEVEL 1 goto fail7
goto patch8
:fail7
pushd ..
%folder%\patch %cmdopt1% < %folder%\CryptAcquireContextW2.txt
%folder%\patch %cmdopt2% < %folder%\CryptAcquireContextW2.txt
popd


:patch8
rem Windows95�ł� InterlockedCompareExchange �� InterlockedCompareExchange ��
rem ���T�|�[�g�̂��߁A�ʂ̏����Œu��������B
rem InitializeCriticalSectionAndSpinCount �����T�|�[�g�����AWindowsMe������
rem ���u�Ɋ܂܂��B
findstr /c:"INTERLOCKEDCOMPAREEXCHANGE" ..\openssl\crypto\threads_win.c
if ERRORLEVEL 1 goto fail8
goto patch9
:fail8
pushd ..
copy /b openssl\crypto\threads_win.c.orig openssl\crypto\threads_win.c.orig2
%folder%\patch %cmdopt1% < %folder%\atomic_api_win95.txt
%folder%\patch %cmdopt2% < %folder%\atomic_api_win95.txt
popd


rem Windows95�ł� CryptAcquireContextW �����T�|�[�g�̂��߁A�G���[�ŕԂ��悤�ɂ���B
rem �G���[��� CryptAcquireContextA ���g���B
:patch9
findstr /c:"myCryptAcquireContextW" ..\openssl\crypto\rand\rand_win.c
if ERRORLEVEL 1 goto fail9
goto patch10
:fail9
pushd ..
copy /b openssl\crypto\rand\rand_win.c.orig openssl\crypto\rand\rand_win.c.orig2
%folder%\patch %cmdopt1% < %folder%\CryptAcquireContextW_win95.txt
%folder%\patch %cmdopt2% < %folder%\CryptAcquireContextW_win95.txt
popd



:patch10


:patch_end
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


