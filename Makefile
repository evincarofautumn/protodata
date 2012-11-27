CPPFLAGS=-I. -Ivendor -MD -MP -Wall -Werror -Wno-sign-compare
CXXFLAGS=-std=c++0x
SRC= \
    Term.cpp \
    interpret.cpp \
    io.cpp \
    main.cpp \
    parse.cpp \
    usage.cpp

MAKEFLAGS+=--silent

.PHONY : all
all : build test

.PHONY : clean
clean : clean-pd clean-deps clean-test

.PHONY : clean-pd
clean-pd :
	rm pd
	rm *.o

.PHONY : clean-deps
clean-deps :
	rm *.d

.PHONY : clean-test
clean-test :
	rm test/*.actual

.PHONY : Build
build : pd

pd : $(SRC:%.cpp=%.o)
	$(CXX) -o $@ $^

.PHONY : test
test :
	./test/run.sh ./pd

-include $(SRC:%.cpp=%.d)
