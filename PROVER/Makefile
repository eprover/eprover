#------------------------------------------------------------------------
#
# File  : Makefile for E prover
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Tue Jun  9 01:29:39 MET DST 1998
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = eprover classify_problem termprops proofanalyze\
          direct_examples epclanalyse epclextract checkproof eground\
          edpll epcllemma\
          ekb_create ekb_insert ekb_ginsert ekb_delete tsm_classify
LIB     = $(PROJECT)
all: $(LIB)

depend: *.c
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o $(PROJECT)
	@rm *.o $(PROJECT)
	@echo Removed compiled files

# Services (provided by the master Makefile)

include ../Makefile.services

# Build the programs

EPROVER = eprover.o ../lib/CONTROL.a ../lib/HEURISTICS.a\
            ../lib/LEARN.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

eprover: $(EPROVER)
	$(LD) -o eprover $(EPROVER)

EGROUND = eground.o ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

eground: $(EGROUND)
	$(LD) -o eground $(EGROUND)

EDPLL = edpll.o ../lib/PROPOSITIONAL.a ../lib/CLAUSES.a\
        ../lib/ORDERINGS.a ../lib/TERMS.a ../lib/INOUT.a\
	../lib/BASICS.a 

edpll: $(EDPLL)
	$(LD) -o edpll $(EDPLL)

CLASSIFY = classify_problem.o ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

classify_problem: $(CLASSIFY)
	$(LD) -o classify_problem $(CLASSIFY)

TERMPROPS = termprops.o ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

termprops: $(TERMPROPS)
	$(LD) -o termprops $(TERMPROPS)

PROOFANALYZE = proofanalyze.o \
            ../lib/ANALYSIS.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

proofanalyze: $(PROOFANALYZE)
	$(LD) -o proofanalyze $(PROOFANALYZE)


DIRECT_EXAMPLES = direct_examples.o \
            ../lib/ANALYSIS.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

direct_examples: $(DIRECT_EXAMPLES)
	$(LD) -o direct_examples $(DIRECT_EXAMPLES)


EPCLANALYSE = epclanalyse.o \
        ../lib/PCL2.a ../lib/HEURISTICS.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
        ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

epclanalyse: $(EPCLANALYSE)
	$(LD) -o epclanalyse $(EPCLANALYSE)


EPCLLEMMA = epcllemma.o \
        ../lib/PCL2.a ../lib/HEURISTICS.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
        ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

epcllemma: $(EPCLLEMMA)
	$(LD) -o epcllemma $(EPCLLEMMA)


EPCLEXTRACT = epclextract.o \
        ../lib/PCL2.a ../lib/HEURISTICS.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
        ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

epclextract: $(EPCLEXTRACT)
	$(LD) -o epclextract $(EPCLEXTRACT)

CHECKPROOF = checkproof.o \
        ../lib/PCL2.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
        ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

checkproof: $(CHECKPROOF)
	$(LD) -o checkproof $(CHECKPROOF)

EKB_CREATE = ekb_create.o \
            ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

ekb_create: $(EKB_CREATE)
	$(LD) -o ekb_create $(EKB_CREATE)

EKB_INSERT = ekb_insert.o \
            ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

ekb_insert: $(EKB_INSERT)
	$(LD) -o ekb_insert $(EKB_INSERT)

EKB_GINSERT = ekb_ginsert.o \
            ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

ekb_ginsert: $(EKB_GINSERT)
	$(LD) -o ekb_ginsert $(EKB_GINSERT)

EKB_DELETE = ekb_delete.o \
            ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

ekb_delete: $(EKB_DELETE)
	$(LD) -o ekb_delete $(EKB_DELETE)

TSM_CLASSIFY = tsm_classify.o \
            ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

tsm_classify: $(TSM_CLASSIFY)
	$(LD) -o tsm_classify $(TSM_CLASSIFY)


include Makefile.dependencies




