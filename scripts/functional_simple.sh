#!/bin/bash

rm -f ../build/functional/simple.*
mkdir -p ../build/functional/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/functional/simple.o ../demo/functional/simple.cpp
g++ ../build/functional/simple.o -lpthread -o ../build/functional/simple.exe 
../build/functional/simple.exe
