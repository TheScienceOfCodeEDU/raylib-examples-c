#!/bin/bash

mkdir -p build/linux
gcc main.c -g -ggdb -lraylib  -o build/linux/main -std=c99 -pedantic
