@echo off

IF NOT EXIST build mkdir build
pushd build

set compilerFlags=-nologo -Oi -WX -W4 -wd4005 -wd4189 -wd4201 -wd4996 -Z7 -FC -MP6
cl %compilerFlags% ..\main.c ..\include\glad\glad.c /link ..\glfw3dll.lib

popd
