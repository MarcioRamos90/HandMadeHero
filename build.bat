@echo off

if not exist .\build (
  mkdir .\build
)

pushd .\build
clang-cl.exe /WX /W4 -Wimplicit-int-conversion -Wsign-conversion /DHANDMADE_INTERNAL=1 /DHANDMADE_SLOW=1 /DHANDMADE_WIN32=1 /FC /Zi ..\code\win32_handmade.cpp user32.lib Gdi32.lib
popd
