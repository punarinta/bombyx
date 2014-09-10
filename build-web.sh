#!/bin/sh

g++ \
core/*.c \
larva.c \
web.cpp \
\
-lfcgi++ -lfcgi -lmysqlclient_r -std=c++11 \
-O2 -o bombyx-web -lpthread
