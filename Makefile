CPPFLAGS=-I. -Ivendor -MD -MP -Wall -Werror
CXXFLAGS=-std=c++0x
SRC=main.cpp

MAKEFLAGS+=--silent

.PHONY : all
all : pd test

.PHONY : clean
clean :
	rm pd
	rm *.d
	rm *.o
	rm test/*.actual

pd : $(SRC:%.cpp=%.o)
	$(CXX) -o $@ $^

.PHONY : test
test :
	./test/run.sh ./pd

-include $(SRC:%.cpp=%.d)
