#!/bin/bash

CC="gcc"
FLAGS="-g -std=c99 -Wall -Werror -L./lib -I./include"
LIBS="-l:libraylib.so -lm" 
EXE_DEBUG="./bin/hotray.debug"
EXE_RELEASE="./bin/hotray.release"
SHARED_TMP="./bin/game.tmp.so"
DEBUG_ENTRY_POINT="./src/debug.c"
RELEASE_ENTRY_POINT="./src/release.c"
SHARED_SRC="./src/game.c"

CMD_FLAG_DEBUG=false

ROOT_HELP='USAGE:\n\t./app.sh [COMMAND]\nCOMMANDS:\n\trun\n\treload'
RUN_HELP='USAGE:\n\t./app.sh run [FLAG]\nFLAGS:\n\t--debug\n\t--release'


if [ "$1" = "run" ]; then
    if [ "$2" = "--debug" ]; then
        $CC $FLAGS $DEBUG_ENTRY_POINT -o $EXE_DEBUG $LIBS
        #$CC $FLAGS -shared $SHARED_SRC -o $SHARED_TMP
        LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH $EXE_DEBUG
    elif [ "$2" = "--release" ];then
        $CC $FLAGS $RELEASE_ENTRY_POINT -o $EXE_RELEASE $LIBS
    else
        echo -e "unknown flag $2"
        echo -e $RUN_HELP
    fi 
elif [ "$1" = "reload" ]; then
        $CC $FLAGS -shared $SHARED_SRC -o $SHARED_TMP
else
    echo -e "unknown command $1"
    echo -e $ROOT_HELP
fi

