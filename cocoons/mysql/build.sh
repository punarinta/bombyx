#!/bin/sh

cd "$(dirname "$0")"

gcc \
mysql.c \
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
-O0 -shared -fPIC -o ../mysql.ccn -L../../vendor/ -lfcgi -ljansson -lmysqlclient_r

# -O3 -DNDEBUG -shared -fPIC -o ../mysql.ccn -L../../vendor/ -lfcgi -ljansson
