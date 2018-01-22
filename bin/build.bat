@echo off

if not exist w:\build mkdir w:\build
pushd w:\build

del *.pdb > NUL 2> NUL

set CompilerFlags=%1 -MT -nologo -Gr -EHa -FC -Z7 -Fe3DGrapher.exe -Iw:\deps\ -wd4201 -wd4913 -wd4267 -wd4996 -bigobj
set LinkerFlags=-LIBPATH:w:\deps\SDL2\lib\x64\ -LIBPATH:w:\deps\glew\lib\ -LIBPATH:w:\deps\symbolicc++\lib\ /SUBSYSTEM:windows opengl32.lib SDL2main.lib SDL2.lib glew32.lib w:/asset/icon.res -NODEFAULTLIB:ucrt -NODEFAULTLIB:MSVCRT
set Files=w:\src\all.cpp

xcopy w:\deps\SDL2\lib\x64\*.dll w:\build\ /q /y
xcopy w:\deps\glew\lib\*.dll w:\build\ /q /y

echo compiling grapher...
cl %CompilerFlags% %Files% /link %LinkerFlags%
echo done!

popd
