@echo off

if not exist .\build (
  mkdir .\build
)

pushd .\build
cl.exe -nologo /Fmwin32_handmade.map -wd5051 -wd4805 -wd4201 -wd4100 -wd4211 -MT -GR- -Oi /WX /W4 /FC /Z7 /DHANDMADE_INTERNAL=1 /DHANDMADE_SLOW=1 /DHANDMADE_WIN32=1  ..\code\win32_handmade.cpp /link -subsystem:windows,6.02 user32.lib Gdi32.lib 
popd
