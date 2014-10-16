#!/bin/sh

gcc \
web.c \
\
-std=c99 \
-O0 -shared -fPIC -o ../web.ccn

# -O3 -DNDEBUG -shared -fPIC -o ../web.ccn
