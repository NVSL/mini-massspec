
CXX=g++
CXXFLAGS=-std=gnu++17 -g

default: main

.PHONY:test
test: main 
#	./main aoeu aoeu aoeu |  ./check.py test-ref.json

.PHONY: clean
clean:
	rm -rf main
