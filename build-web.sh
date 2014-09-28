#!/bin/sh

g++ \
core/*.c \
web.cpp \
\
-std=c++11 \
-O0 -o bombyx-web -lpthread -ldl -lfcgi++ -lfcgi -lmysqlclient_r
