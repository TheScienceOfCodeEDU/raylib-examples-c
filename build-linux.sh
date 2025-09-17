#!/bin/bash

mkdir -p bin/dbg-linux
rm bin/dbg-linux/run > /dev/null 2>&1
gcc src/main.c -g -ggdb -lraylib -lm -o bin/dbg-linux/run -std=c99 -pedantic
