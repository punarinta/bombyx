#!/bin/sh

gcc \
core/*.c \
console.c \
\
-std=c99 \
-O0 -g -o bombyx -ldl

# -O3 -o bombyx -ldl

# valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite ./bombyx tests/1.leaf -v
# valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./bombyx tests/1.leaf
# http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html
