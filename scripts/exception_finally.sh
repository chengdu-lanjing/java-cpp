#!/bin/bash

rm -f ../build/exception/finally.*
mkdir -p ../build/exception/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/exception/finally.o ../demo/exception/finally.cpp
g++ ../build/exception/finally.o -lpthread -o ../build/exception/finally.exe 
../build/exception/finally.exe
