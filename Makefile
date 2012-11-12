CXXFLAGS=-std=c++0x
CPPFLAGS=-I. -Ivendor -MD -MP
SRC=main.cpp

.PHONY : all
all : pd

pd : $(SRC:%.cpp=%.o)
	$(CXX) -o $@ $^

-include $(SRC:%.cpp=%.d)
