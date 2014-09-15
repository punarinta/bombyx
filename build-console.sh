#!/bin/sh

gcc \
core/*.c \
larva.c \
console.c \
\
-std=c99 \
-g -o bombyx
