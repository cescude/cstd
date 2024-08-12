#!/bin/sh

set -eu

CFLAGS="-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap"
#CFLAGS=-O3

buildif() {
    local SOURCE
    local OBJ
    SOURCE=$1
    OBJ=src/std/$(basename -s.c ${SOURCE}).o

    if [ ! -f ${OBJ} -o ${SOURCE} -nt ${OBJ} ]; then
	echo Building ${SOURCE}
	gcc ${CFLAGS} -c ${SOURCE} -o ${OBJ}
    else
	echo Skipping ${SOURCE}
    fi
}

tags() {
    find src -type f -name '*.[ch]' | etags -
}

libstd.a() {
    find src/std -type f -name '*.c' | while read FILE; do
	buildif ${FILE}
    done
    echo Building libstd.a
    (cd dist; ar rcs libstd.a ../src/std/*.o)
}

opt_demo() {
    buildif src/opt_demo.c
    gcc ${CFLAGS} src/opt_demo.c -Ldist -lstd -o dist/opt_demo
}

mkdir -p dist
tags
libstd.a
opt_demo
