#------------------------------------------------------------------------
#
# File  : Makefile for CONTROL part of thew CLIB library/E Prover
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Mon Jun  8 11:09:27 MET DST 1998
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = CONTROL
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.a; rm *.o  *.a

# Services (provided by the master Makefile)

include ../Makefile.services

CONTROL_LIB = cco_simplification.o cco_factoring.o\
	      cco_eqnresolving.o cco_paramodulation.o cco_diseq_decomp.o\
	      cco_forward_contraction.o  cco_clausesplitting.o\
              cco_interpreted.o\
              cco_proofproc.o cco_proc_ctrl.o cco_gproc_ctrl.o\
              cco_batch_spec.o cco_einteractive_mode.o\
	      cco_sine.o cco_esession.o cco_eserver.o cco_scheduling.o\
	      cco_ho_inferences.o cco_preprocessing.o

$(LIB): $(CONTROL_LIB)
	$(AR) $(LIB) $(CONTROL_LIB)

include Makefile.dependencies
