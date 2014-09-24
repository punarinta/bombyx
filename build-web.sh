#!/bin/sh

g++ \
core/*.c \
larva.c \
web.cpp \
\
-std=c++11 \
-O0 -o bombyx-web -lpthread -ldl -lfcgi++ -lfcgi -lmysqlclient_r
