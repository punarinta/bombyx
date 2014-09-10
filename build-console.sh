#!/bin/sh

gcc \
core/*.c \
larva.c \
console.c \
\
-std=c99 \
-o bombyx
