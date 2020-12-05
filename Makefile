
CXX=g++
CXXFLAGS=-std=gnu++17 -g

default: main

.PHONY:test
test: main 
	./main data/804.mxs data/Query1.txt 804-Query1.json
	./check.py 804-Query1.json  < data/804-Query1-ref.json

.PHONY: clean
clean:
	rm -rf main
