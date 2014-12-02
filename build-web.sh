#!/bin/sh

cd "$(dirname "$0")"

gcc \
core/*.c \
web.c \
\
-std=c99 \
-D WEB_BUILD=1 -O0 -o bombyx-web -lpthread -ldl -lfcgi -ldl -ljansson
