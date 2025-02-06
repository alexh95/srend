@ECHO OFF

FOR %%a IN (%*) DO SET "%%a=1"
IF NOT "%MSVC%"=="1" IF NOT "%CLANG%"=="1" SET MSVC=1

SET CompilerOptionsMsvc=/nologo /std:clatest /FC /Z7 /Od /Ob1
SET CompilerOptionsClang=-std=c23 -g -fdiagnostics-absolute-paths -O0 -fno-inline -Wall -Wextra -Werror -pedantic -Wno-c23-extensions -Wno-unused-parameter

SET LinkerOptionsMsvc=/INCREMENTAL:NO /OPT:REF /MANIFEST:EMBED
SET LinkerOptionsClang=-fuse-ld=lld -Xlinker /MAP -Xlinker /MANIFEST:EMBED

IF NOT EXIST .\build MKDIR build
PUSHD .\build

DEL *.pdb > NUL 2> NUL

if "%MSVC%"=="1" (
    cl %CompilerOptionsMsvc% ..\src\srend.c /D SREND_EXPORTS /Fm"srend.map" /link %LinkerOptionsMsvc% /DLL /OUT:srend.dll /EXPORT:RendererInitialize /EXPORT:RendererUpdateAndDraw
    cl %CompilerOptionsMsvc% ..\src\win32_main.c /Fe"sr.exe" /Fm"sr.map" /link %LinkerOptionsMsvc% user32.lib gdi32.lib
)

if "%CLANG%"=="1" (
    clang %CompilerOptionsClang% ..\src\srend.c -DSREND_EXPORTS -shared -o "srend.dll" %LinkerOptionsClang% -Xlinker /EXPORT:RendererInitialize /EXPORT:RendererUpdateAndDraw
    clang %CompilerOptionsClang% ..\src\win32_main.c -o "sr.exe" %LinkerOptionsClang% -l user32.lib -l gdi32.lib
)

POPD
