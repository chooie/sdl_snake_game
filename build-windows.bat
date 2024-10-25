@echo off

set "build_dir_created=false"

REM Check if the build directory exists; create it if not
if not exist build (
    mkdir build
    set "build_dir_created=true"
)

REM Compile main.cpp
g++ -Wall -Werror -Wconversion -I vendor/SDL2/windows/include -L vendor/SDL2/windows/lib -o build\main.exe main.cpp -lmingw32 -lSDL2main -lSDL2 -lwinmm -mwindows

REM Check if compilation was successful
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b %errorlevel%
)

REM Only copy SDL2.dll if the build directory was just created
if "%build_dir_created%"=="true" (
    echo Copying SDL2.dll to the build directory
    copy /Y vendor\SDL2\windows\lib\SDL2.dll build\
)
