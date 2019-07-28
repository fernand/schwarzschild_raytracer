@echo off

IF NOT EXIST build mkdir build
pushd build

rem del *.pdb > NUL 2> NUL

set compilerFlags=-Z7 -FC -wd4005 /MP6
cl /nologo %compilerFlags% ..\main.c ..\include\glad\glad.c /link ..\glfw3dll.lib

popd
