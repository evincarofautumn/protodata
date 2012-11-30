CPPFLAGS=-I. -Ivendor -MD -MP -Wall -Werror -Wno-sign-compare
CXXFLAGS=-std=c++0x
SRC= \
    Term.cpp \
    deleters.cpp \
    interpret.cpp \
    io.cpp \
    main.cpp \
    parse.cpp \
    arguments.cpp

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
