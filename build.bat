@echo off

set CommonCompilerFlags=-nologo /Fmwin32_handmade.map -wd4456 -wd4505 -wd4189 -wd5051 -wd4805 -wd4201 -wd4100 -wd4211 -wd4101 -wd4459 -MT -GR- -Oi /WX /W4 /FC /Z7 /DHANDMADE_INTERNAL=1 /DHANDMADE_SLOW=1 /DHANDMADE_WIN32=1
set CommonLinkerFlags=user32.lib Gdi32.lib winmm.lib

if not exist .\build (
  mkdir .\build
)

pushd .\build
@REM 32bit build
@REM cl.exe %CommonCompilerFlags% ..\code\win32_handmade.cpp /link -subsystem:windows,6.02 %CommonLinkerFlags%

@REM 64bit build
cl.exe %CommonCompilerFlags% ..\code\win32_handmade.cpp %CommonLinkerFlags%
popd
