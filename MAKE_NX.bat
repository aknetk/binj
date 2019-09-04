ECHO OFF
make -f Makefile.nx
if NOT %errorlevel% == 0 goto :FINISHED

copy Z:\switch\Binj\binj.nro builds/nx/Binj.nro
C:\devkitPro\tools\bin\nxlink.exe builds/nx/Binj.nro -s

:FINISHED
pause
