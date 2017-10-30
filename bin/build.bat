@echo off

if not exist w:\build mkdir w:\build
pushd w:\build

del *.pdb > NUL 2> NUL

set Game_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Fe3DGrapher.exe -Iw:\deps\ -wd4201 -wd4913 -wd4267 -wd4996 /bigobj
set Game_ReleaseCompilerFlags=-O2 -MT -nologo -Gr -EHa -FC -Z7 -Fe3DGrapher.exe -Iw:\deps\ -wd4201 -wd4913 -wd4267 -wd4996 /bigobj
set Game_LinkerFlags=-LIBPATH:w:\deps\SDL2\lib\x64\ -LIBPATH:w:\deps\glew\lib\ -LIBPATH:w:\deps\symbolicc++\lib\ /SUBSYSTEM:windows opengl32.lib SDL2main.lib SDL2.lib glew32.lib w:/asset/icon.res -NODEFAULTLIB:ucrt -NODEFAULTLIB:MSVCRT

set Files=w:\src\all.cpp

xcopy w:\deps\SDL2\lib\x64\*.dll w:\build\ /q /y
xcopy w:\deps\glew\lib\*.dll w:\build\ /q /y

echo compiling grapher...
if "%1"=="release" (
	cl %Game_ReleaseCompilerFlags% %Files% /link %Game_LinkerFlags%
) else (
	cl %Game_DebugCompilerFlags% %Files% /link %Game_LinkerFlags%
)
echo done!

popd
