#------------------------------------------------------------------------
#
# File  : Makefile for the TERMS library
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

PROJECT = TERMS
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

TERM_LIB = cte_simplesorts.o cte_functypes.o cte_signature.o\
           cte_termtypes.o \
           cte_termvars.o cte_acterms.o\
           cte_varhash.o cte_varsets.o cte_termfunc.o cte_termtrees.o\
           cte_termcellstore.o\
           cte_termbanks.o cte_subst.o cte_termpos.o cte_termcpos.o \
           cte_replace.o cte_match_mgu_1-1.o cte_idx_fp.o cte_fp_index.o \
	   cte_simpletypes.o cte_typecheck.o


$(LIB): $(TERM_LIB)
	$(AR) $(LIB) $(TERM_LIB)

include Makefile.dependencies

