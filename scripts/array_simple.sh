#!/bin/bash

rm -f ../build/array/simple.*
mkdir -p ../build/array/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/array/simple.o ../demo/array/simple.cpp
g++ ../build/array/simple.o -lpthread -o ../build/array/simple.exe 
../build/array/simple.exe
