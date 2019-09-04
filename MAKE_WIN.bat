@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

SET INCLUDE=^
C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include;^
C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\include;^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.16299.0\um;^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.16299.0\shared;^
C:\Program Files (x86)\Windows Kits\10\Include\10.0.16299.0\winrt;^
include;^
source;^
meta\win\include;

SET LIB=^
C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\lib;^
C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\lib;^
C:\Program Files (x86)\Windows Kits\10\lib\10.0.16299.0\ucrt\x86;^
C:\Program Files (x86)\Windows Kits\10\lib\10.0.16299.0\um\x86;^
meta\win\lib;

SET PATH=^
%SystemRoot%\system32;^
C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin;^
C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE;^
meta\win\bin;

SET TARGET_NAME=Binj
SET TARGET_FOLDER=builds\win\
SET TARGET_SUBSYSTEM=/SUBSYSTEM:WINDOWS
SET TARGET_SUBSYSTEM_UNUSED=/SUBSYSTEM:CONSOLE

SET SRC_FOLDER=%~dp0source\
SET OBJ_FOLDER=out\win\
SET OBJ_LIST=

IF NOT EXIST %OBJ_FOLDER% MKDIR %OBJ_FOLDER%
IF NOT EXIST %TARGET_FOLDER% MKDIR %TARGET_FOLDER%

FOR /f %%j IN ('dir /s /b %SRC_FOLDER%*.cpp') DO (
   SET SRC_FILE=%%j
   SET OBJ_FILE=!SRC_FILE:%SRC_FOLDER%=%OBJ_FOLDER%!
   SET OBJ_FILE=!OBJ_FILE:.cpp=.obj!
   SET OBJ_LIST=!OBJ_FILE! !OBJ_LIST!
   FOR %%k IN (!OBJ_FILE!) DO (
      IF NOT EXIST %%~dpk MKDIR %%~dpk
   )
   CL "!SRC_FILE!" /nologo /c /DWIN32 /DGLEW_STATIC /DUSE_DYNAMIC_LIBASS /DTARGET_NAME=\"!TARGET_NAME!\" /EHsc /FS /Gm /Gd /MD /Zi /Fo!OBJ_FILE! /Fd%OBJ_FOLDER%%TARGET_NAME%.pdb
   IF NOT %errorlevel% == 0 PAUSE
)
FOR /f %%j IN ('dir /s /b %SRC_FOLDER%*.c') DO (
   SET SRC_FILE=%%j
   SET OBJ_FILE=!SRC_FILE:%SRC_FOLDER%=%OBJ_FOLDER%!
   SET OBJ_FILE=!OBJ_FILE:.c=.obj!
   SET OBJ_LIST=!OBJ_FILE! !OBJ_LIST!
   FOR %%k IN (!OBJ_FILE!) DO (
      IF NOT EXIST %%~dpk MKDIR %%~dpk
   )
   CL "!SRC_FILE!" /nologo /c /DWIN32 /DGLEW_STATIC /DUSE_DYNAMIC_LIBASS /DTARGET_NAME=\"!TARGET_NAME!\" /EHsc /FS /Gm /Gd /MD /Zi /Fo!OBJ_FILE! /Fd%OBJ_FOLDER%%TARGET_NAME%.pdb
   IF NOT %errorlevel% == 0 PAUSE
)
LINK ^
   /OUT:"!TARGET_FOLDER!!TARGET_NAME!.exe" ^
   !TARGET_SUBSYSTEM! ^
   /MACHINE:X86 ^
   /nologo !OBJ_LIST! ^
   zlibstat.lib ^
   glew32s.lib ^
   opengl32.lib ^
   libogg_static.lib ^
   libvorbis_static.lib ^
   libvorbisfile_static.lib ^
   SDL2.lib ^
   SDL2main.lib ^
   SDL2_image.lib ^
   SDL2_mixer.lib ^
   SDL2_ttf.lib ^
   avcodec.lib ^
   avformat.lib ^
   avutil.lib ^
   swscale.lib ^
   swresample.lib ^
   Ws2_32.lib ^
   Wldap32.lib ^
   libcurl_a.lib ^
   Normaliz.lib ^
   crypt32.lib ^
   kernel32.lib ^
   user32.lib ^
   gdi32.lib ^
   winspool.lib ^
   comdlg32.lib ^
   advapi32.lib ^
   shell32.lib ^
   ole32.lib ^
   oleaut32.lib ^
   uuid.lib ^
   odbc32.lib ^
   odbccp32.lib
cd source
"..\!TARGET_FOLDER!!TARGET_NAME!.exe"
pause
