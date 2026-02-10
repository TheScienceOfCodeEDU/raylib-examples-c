@echo off
mkdir "bin/dbg-win" 2>NUL
copy "C:\raylib\lib\raylib.dll" "bin/dbg-win" >NUL
gcc src/main.c -IC:\raylib\include -Lbin/dbg-win -g -ggdb -lraylib -lgdi32 -lwinmm -lm -o bin/dbg-win/run.exe -std=c99 -pedantic