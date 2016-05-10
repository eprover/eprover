#------------------------------------------------------------------------
#
# File  : Makefile for the PROPOSITIONAL library
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Thu Apr 24 17:09:00 CEST 2003
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = PROPOSITIONAL
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.a; rm *.o  *.a

# Services (provided by the master Makefile)

include ../Makefile.services

# Build the  library

PROP_LIB = cpr_propsig.o cpr_varset.o cpr_propclauses.o cpr_dpllformula.o cpr_dpll.o


$(LIB): $(PROP_LIB)
	$(AR) $(LIB) $(PROP_LIB)

include Makefile.dependencies

