#!/bin/bash

rm -f ../build/threading/pool.*
mkdir -p ../build/threading/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/threading/pool.o ../demo/threading/pool.cpp
g++ ../build/threading/pool.o -lpthread -o ../build/threading/pool.exe 
../build/threading/pool.exe
