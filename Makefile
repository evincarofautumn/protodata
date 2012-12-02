INCFLAGS=-I. -Ivendor
DEPFLAGS=-MD -MP
WARNFLAGS=$(addprefix -W,all error no-sign-compare)
EXTRAFLAGS=-fno-deduce-init-list
CPPFLAGS+=$(INCFLAGS) $(DEPFLAGS) $(WARNFLAGS) $(EXTRAFLAGS)
CXXFLAGS+=-std=c++0x
SRC=$(wildcard *.cpp)
OBJFILES=$(SRC:%.cpp=%.o)

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

.PHONY : build
build : pd

pd : $(OBJFILES)
	$(CXX) -o $@ $(OBJFILES)

TESTS=$(basename $(notdir $(wildcard test/*.pd)))
define TESTRULE
test-$1 : pd
	@ ./test/run.sh $$(realpath ./pd) $1
test : test-$1
endef
.PHONY : $(foreach TEST,$(TESTS),test-$(TEST))
$(foreach TEST,$(TESTS),$(eval $(call TESTRULE,$(TEST))))

-include $(SRC:%.cpp=%.d)

define DEPENDS_ON_MAKEFILE
$1 : Makefile

endef

# Any changes to this Makefile cause recompilation.
$(call DEPENDS_ON_MAKEFILE,pd)
$(foreach OBJ,$(OBJFILES),$(eval $(call DEPENDS_ON_MAKEFILE,$(OBJ))))
