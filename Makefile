INCFLAGS=-I. -Ivendor
DEPFLAGS=-MD -MP
WARNFLAGS=$(addprefix -W,all error no-sign-compare)
EXTRAFLAGS=-fno-deduce-init-list
CPPFLAGS=$(INCFLAGS) $(DEPFLAGS) $(WARNFLAGS) $(EXTRAFLAGS)
CXXFLAGS=-std=c++0x
SRC=$(wildcard *.cpp)

MAKEFLAGS+=--silent

.PHONY : all
all : build test

.PHONY : clean
clean : clean-pd clean-deps clean-test

.PHONY : clean-pd
clean-pd :
	rm -f pd
	rm -f *.o

.PHONY : clean-deps
clean-deps :
	rm -f *.d

.PHONY : clean-test
clean-test :
	rm -f test/*.actual

.PHONY : Build
build : pd

pd : $(SRC:%.cpp=%.o)
	$(CXX) -o $@ $^

.PHONY : test
test :
	./test/run.sh ./pd

-include $(SRC:%.cpp=%.d)
