#!/bin/bash

rm -f ../build/http/get.*
mkdir -p ../build/http/
g++ -c -I ../src -DDEBUG -std=c++11 -o ../build/http/get.o ../demo/http/get.cpp
g++ ../build/http/get.o -lpthread -lcurl -o ../build/http/get.exe 
../build/http/get.exe
