REM @echo off


REM IF NOT EXIST /build mkdir /build
REM pushd build


REM 64-bit build
del *.pdb > NUL 2> NUL
cl -Od /FC /Z7 /I ..\external\include /I ..\external\src /MDd main.cpp -Fmmain.map ..\external\src\glad.c /link /DEBUG /incremental:no ..\external\lib\glfw3.lib ../external\lib\freetyped.lib User32.lib kernel32.lib gdi32.lib Shell32.lib
