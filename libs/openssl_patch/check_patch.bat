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

rem freeaddrinfo/getnameinfo/getaddrinfo API�ˑ������̂���
:patch1
findstr /c:"# undef AI_PASSIVE" ..\openssl\crypto\bio\bio_lcl.h
if ERRORLEVEL 1 goto fail1
goto patch4
:fail1
pushd ..
%folder%\patch %cmdopt1% < %folder%\ws2_32_dll_patch.txt
%folder%\patch %cmdopt2% < %folder%\ws2_32_dll_patch.txt
popd


rem CryptAcquireContextW API�ˑ������̂���
:patch4
rem findstr /c:"add_RAND_buffer" ..\openssl\crypto\rand\rand_win.c
rem if ERRORLEVEL 1 goto fail4
rem goto patch5
rem :fail4
rem pushd ..
rem %folder%\patch %cmdopt1% < %folder%\CryptAcquireContextW.txt
rem %folder%\patch %cmdopt2% < %folder%\CryptAcquireContextW.txt
rem popd


rem WindowsMe��RAND_bytes�ŗ����錻�ۉ���̂��߁B
:patch5
findstr /c:"added if meth is NULL pointer" ..\openssl\crypto\rand\rand_lib.c
if ERRORLEVEL 1 goto fail5
goto patch6
:fail5
pushd ..
%folder%\patch %cmdopt1% < %folder%\RAND_bytes.txt
%folder%\patch %cmdopt2% < %folder%\RAND_bytes.txt
popd


rem WindowsMe��InitializeCriticalSectionAndSpinCount���G���[�ƂȂ錻�ۉ���̂��߁B
:patch6
findstr /c:"myInitializeCriticalSectionAndSpinCount" ..\openssl\crypto\threads_win.c
if ERRORLEVEL 1 goto fail6
goto patch7
:fail6
pushd ..
%folder%\patch %cmdopt1% < %folder%\atomic_api.txt
%folder%\patch %cmdopt2% < %folder%\atomic_api.txt
popd


rem WindowsMe/NT4.0�ł�CryptAcquireContextW�ɂ��G���g���s�[�擾��
rem �ł��Ȃ����߁A�V����������ǉ�����BCryptAcquireContextW�̗��p�͎c���B
:patch7
findstr /c:"void add_RAND_buffer" ..\openssl\crypto\rand\rand_win.c
if ERRORLEVEL 1 goto fail7
goto patch8
:fail7
pushd ..
%folder%\patch %cmdopt1% < %folder%\CryptAcquireContextW2.txt
%folder%\patch %cmdopt2% < %folder%\CryptAcquireContextW2.txt
popd


:patch8


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


