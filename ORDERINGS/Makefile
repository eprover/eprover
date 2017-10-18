#------------------------------------------------------------------------
#
# File  : Makefile for ORDERINGS sublibrary.
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Thu Apr 30 03:11:31 MET DST 1998
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = ORDERINGS
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.a; rm *.o  *.a

# Services (provided by the master Makefile)

include ../Makefile.services

ORDER_LIB = cto_ocb.o cto_cmpcache.o cto_lpo.o cto_kbo.o cto_kbolin.o cto_orderings.o

$(LIB): $(ORDER_LIB)
	$(AR) $(LIB) $(ORDER_LIB)

include Makefile.dependencies




