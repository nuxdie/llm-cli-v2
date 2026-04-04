#!/bin/sh
gcc -c linenoise.c -o linenoise.o
g++ -std=c++23 llm.cpp linenoise.o -o llm
