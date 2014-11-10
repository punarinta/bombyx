#!/bin/sh

gcc \
web.c \
../../core/*.c \
\
-std=c99 \
-O0 -shared -fPIC -o ../web.ccn -L../../vendor/ -ljansson

# -O3 -DNDEBUG -shared -fPIC -o ../web.ccn
