#!/bin/bash

rm -f ../build/memory/BAD_DEMO_memory_leak.*
mkdir -p ../build/memory/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/memory/BAD_DEMO_memory_leak.o ../demo/memory/BAD_DEMO_memory_leak.cpp
g++ ../build/memory/BAD_DEMO_memory_leak.o -lpthread -o ../build/memory/BAD_DEMO_memory_leak.exe 
../build/memory/BAD_DEMO_memory_leak.exe
