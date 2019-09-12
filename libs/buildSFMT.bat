SET filename=SFMT_version_for_teraterm.h

cd SFMT

if exist "Makefile.msc.release" goto end_mk_release
echo CFLAGS = /MT /O2 /nologo> Makefile.msc.release
echo;>> Makefile.msc.release
echo SFMT.lib: SFMT.c>> Makefile.msc.release
echo 	cl $(CFLAGS) /c SFMT.c>> Makefile.msc.release
echo 	lib /out:SFMT.lib SFMT.obj>> Makefile.msc.release
echo;>> Makefile.msc.release
echo clean:>> Makefile.msc.release
echo 	del *.lib *.obj>> Makefile.msc.release
:end_mk_release

if exist "Makefile.msc.debug" goto end_mk_debug
echo CFLAGS = /MTd /Od /nologo> Makefile.msc.debug
echo;>> Makefile.msc.debug
echo SFMTd.lib: SFMT.c>> Makefile.msc.debug
echo 	cl $(CFLAGS) /c SFMT.c>> Makefile.msc.debug
echo 	lib /out:SFMTd.lib SFMT.obj>> Makefile.msc.debug
echo;>> Makefile.msc.debug
echo clean:>> Makefile.msc.debug
echo 	del *.lib *.obj>> Makefile.msc.debug
:end_mk_debug

nmake /f Makefile.msc.debug
nmake /f Makefile.msc.release

rem �o�[�W�����t�@�C�����Ȃ���΍��
IF EXIST %filename% (GOTO FILE_TRUE) ELSE GOTO FILE_FALSE
:FILE_TRUE
ECHO "�o�[�W�����t�@�C����������܂���"
GOTO END

:FILE_FALSE
ECHO "�o�[�W�����t�@�C����������Ȃ��̂ŐV�K�쐬���܂�"
echo #define SFMT_VERSION "Unknown" > %filename%
GOTO END

:END
cd ..
