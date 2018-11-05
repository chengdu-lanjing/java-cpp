#!/bin/bash

rm -f ../build/exception/chain.*
mkdir -p ../build/exception/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/exception/chain.o ../demo/exception/chain.cpp
g++ ../build/exception/chain.o -lpthread -o ../build/exception/chain.exe 
../build/exception/chain.exe
