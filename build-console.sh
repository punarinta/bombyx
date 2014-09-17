#!/bin/sh

gcc \
core/*.c \
larva.c \
console.c \
\
-std=c99 \
-g -o bombyx

# valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./bombyx tests/1.leaf -v
# http://www.unknownroad.com/rtfm/gdbtut/gdbtoc.html
