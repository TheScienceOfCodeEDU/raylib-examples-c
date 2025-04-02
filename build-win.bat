@echo off
mkdir "build/win" 2>NUL
gcc main.c -IC:\raylib\include -LC:\raylib\lib -g -ggdb -lraylib -lgdi32 -lwinmm -o build/win/main.exe -std=c99 -pedantic