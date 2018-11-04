#!/bin/bash

rm -f ../build/database/simple.*
mkdir -p ../build/database/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/database/simple.o ../demo/database/simple.cpp
g++ ../build/database/simple.o -lpthread -lsqlite3 -o ../build/database/simple.exe 
../build/database/simple.exe
