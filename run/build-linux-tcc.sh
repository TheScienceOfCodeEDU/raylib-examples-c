#!/bin/bash

mkdir -p bin/dbg-linux
rm bin/dbg-linux/run > /dev/null 2>&1
tcc -I/usr/local/include -L/usr/local/lib -lraylib -lm src/main.c -o bin/dbg-linux/run -g -std=c99