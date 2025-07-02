#!/bin/bash

mkdir -p bin/dbg-linux
gcc src/main.c -g -ggdb -lraylib -lm -o bin/dbg-linux/run -std=c99 -pedantic
