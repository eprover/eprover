#------------------------------------------------------------------------
#
# File  : Makefile for clauses part of the clib library
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Fri Mar 13 17:09:13 MET 1998
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = CLAUSES
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.a; rm *.o  *.a

# Services (provided by the master Makefile)

include ../Makefile.services

CLAUSE_LIB = ccl_ext_index.o ccl_neweval.o ccl_eqn.o ccl_eqnlist.o \
             ccl_clauseinfo.o ccl_clauses.o\
	     ccl_tformulae.o ccl_formula_wrapper.o ccl_formulasets.o \
             ccl_f_generality.o ccl_sine.o ccl_garbage_coll.o ccl_tcnf.o \
             ccl_propclauses.o\
             ccl_tautologies.o ccl_clausepos.o ccl_clausecpos.o \
             ccl_pdtrees.o ccl_freqvectors.o \
             ccl_fcvindexing.o ccl_clausesets.o ccl_unfold_defs.o\
             ccl_clausefunc.o ccl_formulafunc.o ccl_groundconstr.o\
             ccl_grounding.o ccl_g_lithash.o ccl_axiomsorter.o \
	     ccl_findex.o ccl_clausepos_tree.o ccl_subterm_tree.o \
             ccl_subterm_index.o ccl_overlap_index.o ccl_relevance.o\
             ccl_inferencedoc.o ccl_derivation.o ccl_paramod.o ccl_factor.o\
	     ccl_eqnresolution.o\
             ccl_rewrite.o ccl_unit_simplify.o ccl_subsumption.o \
             ccl_condensation.o ccl_context_sr.o \
             ccl_diseq_decomp.o ccl_def_handling.o ccl_splitting.o \
             ccl_global_indices.o ccl_satinterface.o\
             ccl_proofstate.o ccl_bce.o ccl_pred_elim.o

$(LIB): $(CLAUSE_LIB)
	$(AR) $(LIB) $(CLAUSE_LIB)

include Makefile.dependencies
