@echo off

pushd build

REM 64-bit build
del *.pdb > NUL 2> NUL

REM wd4201 nonstandard extension used : namless struct/union (this one is both in project code, and in dependencies)
REM wd4100 unreferenced formal parameter

cl -Od /FC /Z7 /MDd /W4 /WX /wd4201 /wd4100 /EHsc /I ..\external\include\ -Fmmain.map ..\code\main.cpp  ..\external\src\glad.c /link /DEBUG /incremental:no ..\external\lib\glfw3.lib ..\external\lib\freetyped.lib User32.lib kernel32.lib gdi32.lib Shell32.lib

REM NOTE: just moving font and shader files into the build dir for now. Later, I'll add more robust retrieval logic. (carver - 8-20-20)
cp ../code/*.ttf .
cp ../code/*.frag .
cp ../code/*.vert .

popd

