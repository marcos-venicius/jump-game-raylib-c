#!/usr/bin/env bash

clang -Wall -Wextra -ggdb -pedantic -I./raylib/include/ -Wl,-rpath=./raylib/lib/ -L./raylib/lib/ -lraylib -o game main.c
