#------------------------------------------------------------------------
#
# File  : Makefile for test program (compile and link test prog)
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Sun Jul  6 22:55:11 MET DST 1997
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = cl_test
LIB     = $(PROJECT)
all: $(LIB) patterntest

depend: *.c
	$(MAKEDEPEND)

# Remove all automatically generated files

REMOVE = cl_test patterntest 

clean:
	@touch does_exist.a does_exist.o $(REMOVE)
	rm *.o  *.a $(REMOVE)
	@echo Removed compiled files

# Services (provided by the master Makefile)

include ../Makefile.services

# Build the test program

TEST_PROG = cl_test.o ../lib/CONTROL.a ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

$(LIB): $(TEST_PROG)
	$(LD) -o $(PROJECT) $(TEST_PROG) $(LIBS)


PATTERN_TEST = patterntest.o ../lib/CLAUSES.a ../lib/ORDERINGS.a \
	    ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a ../lib/LEARN.a

patterntest: $(PATTERN_TEST)
	$(LD) -o patterntest $(PATTERN_TEST)  $(LIBS)


PCL_TEST = cl_pcltest.o ../lib/PCL2.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
	../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

cl_pcltest: $(PCL_TEST)
	$(LD) -o cl_pcltest $(PCL_TEST) $(LIBS)


include Makefile.dependencies




