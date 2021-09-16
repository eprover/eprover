#------------------------------------------------------------------------
#
# File  : Makefile for library of learning algorithms
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Mon Feb 16 01:04:12 MET 1998
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = HEURISTICS
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.a; rm *.o  *.a

# Services (provided by the master Makefile)

include ../Makefile.services

HEURISTICS_LIB = che_wfcb.o che_wfcbadmin.o \
                 che_axiomscan.o \
		 che_clausefeatures.o \
	         che_prio_funs.o \
                 che_clausesetfeatures.o \
                 che_rawspecfeatures.o \
                 che_specsigfeatures.o \
	         che_clauseweight.o \
	         che_dagweight.o \
                 che_refinedweight.o \
		 che_varweights.o \
		 che_funweights.o \
		 che_orientweight.o \
		 che_fifo.o che_lifo.o \
                 che_learning.o \
                 che_simweight.o \
		 che_to_params.o \
	         che_hcb.o \
                 che_litselection.o \
	         che_proofcontrol.o \
		 che_hcbadmin.o \
		 che_heuristics.o \
                 che_fcode_featurearrays.o\
		 che_to_weightgen.o \
		 che_to_precgen.o \
		 che_to_autoselect.o \
                 che_axfilter.o \
		 che_strucweight.o \
	         che_prefixweight.o \
	         che_treeweight.o \
		 che_levweight.o \
	         che_tfidfweight.o \
	         che_termweights.o \
		 che_termweight.o \
	         che_random.o \
                 che_diversityweight.o \
				 che_new_autoschedule.o


$(LIB): $(HEURISTICS_LIB)
	$(AR) $(LIB) $(HEURISTICS_LIB)

include Makefile.dependencies
