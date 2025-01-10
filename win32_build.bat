@ECHO OFF

SET CompilerOptions=-std=c23 -Wall -Wextra -Werror -pedantic -Wno-c23-extensions -Wno-unused-parameter -g 

IF NOT EXIST .\build MKDIR build
PUSHD .\build

DEL *.pdb > NUL 2> NUL
DEL *.ilk > NUL 2> NUL

clang ..\src\win32_main.c %CompilerOptions% -o srend.exe -l User32.lib -l Gdi32.lib

POPD
