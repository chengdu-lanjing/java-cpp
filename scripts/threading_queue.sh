#!/bin/bash

rm -f ../build/threading/queue.*
mkdir -p ../build/threading/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/threading/queue.o ../demo/threading/queue.cpp
g++ ../build/threading/queue.o -lpthread -o ../build/threading/queue.exe 
../build/threading/queue.exe
