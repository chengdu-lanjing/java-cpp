#!/bin/bash

rm -f ../build/logging/simple.*
mkdir -p ../build/logging/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/logging/simple.o ../demo/logging/simple.cpp
g++ ../build/logging/simple.o -lpthread -lcurl -o ../build/logging/simple.exe 
../build/logging/simple.exe
