@echo off

if not exist .\build (
  mkdir .\build
)
pushd .\build

clang-cl.exe -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -Zi ..\code\win32_handmade.cpp user32.lib Gdi32.lib
popd
