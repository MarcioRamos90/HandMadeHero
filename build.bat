@echo off

if not exist .\build (
  mkdir .\build
)
pushd .\build

clang-cl.exe -Zi ..\code\win32_handmade.cpp user32.lib Gdi32.lib
popd
