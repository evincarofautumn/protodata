CXX ?= g++

.PHONY : all
all : pd

pd : $(wildcard ./*.cpp)
	$(CXX) -std=c++0x -I. -Ivendor $+ -o $@
