#------------------------------------------------------------------------
#
# File  : Makefile for the SAFELOGIC files
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Thu Sep 18 16:31:59 MET DST 1997
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = SAFELOGIC
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.a; rm *.o  *.a
	@echo Removed compiled files

# Services (provided by the master Makefile)

include ../Makefile.services

# Build the  library



$(LIB): 
	echo "Nothing to do at the moment"

include Makefile.dependencies

