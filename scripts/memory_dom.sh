#!/bin/bash

rm -f ../build/memory/dom.*
mkdir -p ../build/memory/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/memory/dom.o ../demo/memory/dom.cpp
g++ ../build/memory/dom.o -lpthread -o ../build/memory/dom.exe 
../build/memory/dom.exe
