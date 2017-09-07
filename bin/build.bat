@echo off

if not exist w:\build mkdir w:\build
pushd w:\build

del *.pdb > NUL 2> NUL

set Game_DebugCompilerFlags=-Od -MTd -nologo -Gr -EHa -Oi -W4 -Z7 -FC -Fe3DGrapher.exe -Iw:\deps\glm\include -Iw:\deps\ -wd4201 -wd4244 -wd4457 -wd4189 -wd4267
set Game_ReleaseCompilerFlags=-O2 -MT -nologo -Gr -EHa -W4 -FC -Z7 -Fe3DGrapher.exe -Iw:\deps\glm\include -Iw:\deps\ -wd4201 -wd4244 -wd4457 -wd4189 -wd4267
set Game_LinkerFlags=-LIBPATH:w:\deps\SDL2\lib\x64\ /SUBSYSTEM:windows opengl32.lib SDL2_ttf.lib SDL2main.lib SDL2.lib

set Files=w:\src\*.cpp

xcopy w:\deps\SDL2\lib\x64\*.dll w:\build\ /q /y

echo compiling grapher...
if "%1"=="release" (
	cl %Game_ReleaseCompilerFlags% %Files% /link %Game_LinkerFlags%
) else (
	cl %Game_DebugCompilerFlags% %Files% /link %Game_LinkerFlags%
)
echo done!

popd
