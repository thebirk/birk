@echo off
if not defined VisualStudioVersion (
	echo Calling vcvarsall.bat
	call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
)
setlocal

REM Configuration

set OUT=test.exe
set SRC=main.c

set USE_FBLOCKS=1

REM Setup

if /I "%USE_FBLOCKS%" EQU "1" (
	set FBLOCKS=-fblocks -L"%programfiles%/LLVM/lib" -lBlocksRuntime.lib
) else (
	set FBLOCKS=""
)

clang -o %OUT% %SRC% %FBLOCKS%

endlocal