@echo off

rem
rem OpenSSL 1.1.1�Ƀp�b�`���K�p����Ă��邩���m�F����
rem

findstr /c:"# undef AI_PASSIVE" ..\openssl\crypto\bio\bio_lcl.h
if ERRORLEVEL 1 goto fail

findstr /c:"running on Windows95" ..\openssl\crypto\threads_win.c
if ERRORLEVEL 1 goto fail

echo "�p�b�`�͓K�p����Ă��܂�"
timeout 5
goto end

:fail
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

:end
@echo on


