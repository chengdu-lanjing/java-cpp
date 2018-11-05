#!/bin/bash

rm -f ../build/threading/scheduler.*
mkdir -p ../build/threading/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/threading/scheduler.o ../demo/threading/scheduler.cpp
g++ ../build/threading/scheduler.o -lpthread -o ../build/threading/scheduler.exe 
../build/threading/scheduler.exe
