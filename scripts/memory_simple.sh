#!/bin/bash

rm -f ../build/memory/simple.*
mkdir -p ../build/memory/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/memory/simple.o ../demo/memory/simple.cpp
g++ ../build/memory/simple.o -lpthread -o ../build/memory/simple.exe 
../build/memory/simple.exe
