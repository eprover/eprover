#------------------------------------------------------------------------
#
# File  : Makefile for code outside the core E hierarchy.
#
# Author: Stephan Schulz
#
# Changes
#
# Created: Mon Apr 10 00:33:40 GMT 2000
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = CSSCPA_filter
LIB     = $(PROJECT)

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.dSYM $(PROJECT)
	@rm -r *.o $(PROJECT) *.dSYM

# Services (provided by the master Makefile)

include ../Makefile.services

CSSCPA = CSSCPA_filter.o cex_csscpa.o \
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a ../lib/CONTRIB.a

$(LIB): $(CSSCPA)
	$(LD) -o CSSCPA_filter $(CSSCPA) $(LIBS)

include Makefile.dependencies
