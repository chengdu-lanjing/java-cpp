#!/bin/bash

rm -f ../build/memory/resurrection.*
mkdir -p ../build/memory/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/memory/resurrection.o ../demo/memory/resurrection.cpp
g++ ../build/memory/resurrection.o -lpthread -o ../build/memory/resurrection.exe 
../build/memory/resurrection.exe
