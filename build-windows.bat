@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

echo "Starting..."
echo %CD%

set "build_dir_created=false"

:: %~dp0 is relative to current file path
set BUILD_DIR=%~dp0build
IF NOT EXIST %BUILD_DIR% (
    mkdir %BUILD_DIR%
    set "build_dir_created=true"
)

pushd %BUILD_DIR%

rem Get the current time
set hour=%time:~0,2%
rem Remove the leading space for single-digit hours
if "%hour:~0,1%" == " " set hour=0%hour:~1,1%
rem Create the timestamp
set timestamp=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%hour%%time:~3,2%%time:~6,2%
rem Set the filename
set filename=sdl_snake_game%timestamp%.pdb

del %BUILD_DIR%\*.pdb > NUL 2> NUL
set INCLUDE_PATH=%~dp0vendor\SDL2\windows\visual_studio\x64\include
set LIB_PATH=%~dp0vendor\SDL2\windows\visual_studio\x64\lib

REM Compile main.cpp
cl /Zi /W4 /WX /wd4100 /wd4189 /MD /EHsc /D_CRT_SECURE_NO_WARNINGS %~dp0src\main.cpp /I %INCLUDE_PATH% /link -PDB:%filename% /LIBPATH:%LIB_PATH% SDL2.lib SDL2main.lib SDL2_ttf.lib shell32.lib winmm.lib /SUBSYSTEM:CONSOLE
REM TODO: use /SUBSYSTEM:WINDOWS for release to avoid opening a console

REM Check if compilation was successful
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b %errorlevel%
) else (
    echo Compilation succeeded.
)
popd

REM Only copy dlls if the build directory was just created
if "%build_dir_created%"=="true" (
    echo Copying SDL2.dll to the build directory
    copy /Y %~dp0vendor\SDL2\windows\visual_studio\x64\lib\SDL2.dll build\
    echo Copying SDL2_tff.dll to the build directory
    copy /Y %~dp0vendor\SDL2\windows\visual_studio\x64\lib\SDL2_ttf.dll build\
)