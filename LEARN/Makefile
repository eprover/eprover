#------------------------------------------------------------------------
#
# File  : Makefile for the LEARN library
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Wed Apr 14 23:10:36 MET DST 1999
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = LEARN
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

LEARN_LIB = cle_patterns.o cle_clauseenc.o cle_annotations.o \
	cle_annoterms.o cle_flatannoterms.o cle_numfeatures.o \
        cle_examplerep.o \
	cle_kbdesc.o cle_kbinsert.o cle_termtops.o \
        cle_indexfunctions.o cle_tsm.o cle_classification.o \
        cle_tsmio.o


$(LIB): $(LEARN_LIB)
	$(AR) $(LIB) $(LEARN_LIB)

include Makefile.dependencies

