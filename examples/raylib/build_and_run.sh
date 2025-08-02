#!/bin/sh

CC="gcc"
FLAGS="-g -std=c99 -Wall -Werror -L./lib -I./include"
LIBS="-l:libraylib.so -lm" 
SRCS="main.c"
EXE="app"

$CC $FLAGS $SRCS -o $EXE $LIBS
LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH ./app
