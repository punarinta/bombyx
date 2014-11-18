#!/bin/sh

gcc \
web.c \
../../core/block.c \
../../core/bytecode.c \
../../core/challoc.c \
../../core/map.c \
../../core/operator.c \
../../core/sys.c \
../../core/var.c \
../common.c \
\
-std=c99 \
-O0 -shared -fPIC -o ../web.ccn -L../../vendor/ -ljansson

# -O3 -DNDEBUG -shared -fPIC -o ../web.ccn
