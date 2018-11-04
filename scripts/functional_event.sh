#!/bin/bash

rm -f ../build/functional/event.*
mkdir -p ../build/functional/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/functional/event.o ../demo/functional/event.cpp
g++ ../build/functional/event.o -lpthread -o ../build/functional/event.exe 
../build/functional/event.exe
