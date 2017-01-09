@echo off
@echo off
if not defined VisualStudioVersion (
	echo Calling vcvarsall.bat
	call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
)

cl -nologo -Zi -Febirkc main.c