#!/bin/bash

rm -f ../build/exception/rethrow.*
mkdir -p ../build/exception/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/exception/rethrow.o ../demo/exception/rethrow.cpp
g++ ../build/exception/rethrow.o -lpthread -o ../build/exception/rethrow.exe 
../build/exception/rethrow.exe
