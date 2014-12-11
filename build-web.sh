#!/bin/sh

cd "$(dirname "$0")"

gcc \
core/*.c \
web.c \
\
-std=c99 \
-D WEB_BUILD=1 -O0 -o bombyx-web -lpthread -ldl -lm -lfcgi -ldl -Lvendor/ -ljansson

# -D WEB_BUILD=1 -O3 -DNDEBUG -o bombyx-web -lpthread -ldl -lm -lfcgi -ldl -Lvendor/ -ljansson
