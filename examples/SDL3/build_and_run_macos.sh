#!/bin/sh

CC="clang"
FLAGS="-g -std=c99 -Wall -Werror -L./lib -I./include"
LIBS="-lraylib -lm" 
SRCS="main.c"
EXE="app"

$CC $FLAGS $SRCS -o $EXE $LIBS
DYLD_LIBRARY_PATH=$PWD/lib:$DYLD_LIBRARY_PATH ./app
