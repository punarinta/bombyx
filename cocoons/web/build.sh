#!/bin/sh

cd "$(dirname "$0")"

gcc \
web.c \
bcrypt.c \
bcrypt/crypt_gensalt.c \
bcrypt/crypt_blowfish.c \
bcrypt/wrapper.c \
../../core/block.c \
../../core/challoc.c \
../../core/map.c \
../../core/array.c \
../../core/operator.c \
../../core/sys.c \
../../core/var.c \
../common.c \
\
-std=c99 \
-O0 -shared -fPIC -o ../web.ccn -L../../vendor/ -lfcgi -ljansson

# -O3 -DNDEBUG -shared -fPIC -o ../web.ccn -L../../vendor/ -lfcgi -ljansson
