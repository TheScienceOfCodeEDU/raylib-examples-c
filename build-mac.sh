#!/bin/bash
mkdir -p build/mac
cp -r res build/mac/
gcc main.c -g -ggdb -w -lraylib -o build/mac/main -std=c99 -pedantic