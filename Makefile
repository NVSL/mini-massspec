
CXX=g++
CXXFLAGS=-std=gnu++17

default: main

.PHONY:test
test: main 
	./main  |  ./check.py
