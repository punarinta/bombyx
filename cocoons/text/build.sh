#!/bin/sh

gcc \
text.c \
../../core/sys.c \
../common.c \
\
-std=c99 \
-O0 -shared -fPIC -o ../text.ccn -L../../vendor/ -lfcgi -ljansson

# -O3 -DNDEBUG -shared -fPIC -o ../text.ccn -L../../vendor/ -lfcgi -ljansson
