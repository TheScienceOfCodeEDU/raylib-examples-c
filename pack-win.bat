@echo off

REM Clear
setlocal
:clear
set /p answer=[SCRIPT]: Do you want to clear the release folder? (y/n): 

if /i "%answer%"=="y" goto yes-clear
if /i "%answer%"=="N" goto end-clear

echo Please enter Y or N.
goto clearFolder

:yes-clear
rmdir /s /q "bin/rel-win"
goto end-clear

:end-clear
endlocal


REM Compile
:compile
echo [SCRIPT]: Compiling release binaries
mkdir "bin/rel-win" 2>NUL
copy "C:\raylib\lib\raylib.dll" "bin/rel-win" >NUL
gcc src/main.c -O3 -Wall -IC:\raylib\include -Lbin/rel-win -lraylib -lgdi32 -lwinmm -lm -o bin/rel-win/run.exe -std=c99 -pedantic -mwindows


REM Root folder
echo [SCRIPT]: Copying root folder
robocopy "root" "bin/rel-win/" /e
