
CXX=g++
CXXFLAGS=-std=gnu++17 -g

default: main

setup: data/804.mxs

data/804.mxs: data/804.mxs.gz
	gunzip -k $<

.PHONY:test
test: main  data/804.mxs
	./main data/804.mxs data/Query1.txt 804-Query1.json
	./check.py 804-Query1.json  < data/804-Query1-ref.json

.PHONY: clean
clean:
	rm -rf main
