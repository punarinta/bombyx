#!/bin/sh

cd "$(dirname "$0")"

gcc \
core/*.c \
console.c \
\
-std=c99 \
-O0 -g -o bombyx -ldl -lm -Lvendor/ -ljansson

# -O3 -DNDEBUG -o bombyx -ldl -lm -Lvendor/ -ljansson

# valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite ./bombyx tests/1.leaf -v
# valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./bombyx tests/1.leaf
# http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html
