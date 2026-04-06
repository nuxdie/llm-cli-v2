#!/bin/sh
cmake --preset debug
cmake --build --preset debug

./build/debug/llm
