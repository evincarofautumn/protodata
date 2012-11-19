CXXFLAGS=-std=c++0x
CPPFLAGS=-I. -Ivendor -MD -MP
SRC=main.cpp tokenizer.cpp

.PHONY : all
all : pd

pd : $(SRC:%.cpp=%.o)
	$(CXX) -o $@ $^

-include $(SRC:%.cpp=%.d)
