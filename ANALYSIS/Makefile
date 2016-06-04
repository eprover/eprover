#------------------------------------------------------------------------
#
# File  : Makefile for the proof analysis part of CLIB
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Fri Jan 29 17:14:59 MET 1999
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = ANALYSIS
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.a; rm *.o  *.a

# Services (provided by the master Makefile)

include ../Makefile.services

ANALYSIS_LIB = can_clausestats.o can_clausestore.o can_infstate.o \
               can_treeanalyze.o  

$(LIB): $(ANALYSIS_LIB)
	$(AR) $(LIB) $(ANALYSIS_LIB)

include Makefile.dependencies







