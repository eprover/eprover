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

PROJECT = eprover e_ltb_runner e_deduction_server e_axfilter \
	  classify_problem termprops e_client e_server\
          direct_examples epclanalyse epclextract checkproof eground\
          enormalizer edpll epcllemma\
          ekb_create ekb_insert ekb_ginsert ekb_delete tsm_classify
LIB     = $(PROJECT)
all: $(LIB)

depend: *.c
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.dSYM $(PROJECT)
	@rm -r *.o $(PROJECT) *.dSYM

# Services (provided by the master Makefile)

include ../Makefile.services

# Build the programs

EPROVER = eprover.o ../lib/CONTROL.a ../lib/HEURISTICS.a\
            ../lib/LEARN.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

eprover: $(EPROVER)
	$(LD) -o eprover $(EPROVER) $(LIBS)

E_LTB_RUNNER = e_ltb_runner.o ../lib/CONTROL.a ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

e_ltb_runner: $(E_LTB_RUNNER)
	$(LD) -o e_ltb_runner $(E_LTB_RUNNER) $(LIBS)


E_DEDUCTION_SERVER = e_deduction_server.o ../lib/CONTROL.a ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

e_deduction_server: $(E_DEDUCTION_SERVER)
	$(LD) -o e_deduction_server $(E_DEDUCTION_SERVER) $(LIBS)


E_AXFILTER = e_axfilter.o ../lib/CONTROL.a ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

e_axfilter: $(E_AXFILTER)
	$(LD) -o e_axfilter $(E_AXFILTER) $(LIBS)

E_SERVER = e_server.o ../lib/CONTROL.a ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

e_server: $(E_SERVER)
	$(LD) -o e_server $(E_SERVER) $(LIBS)

E_CLIENT = e_client.o ../lib/CONTROL.a ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

e_client: $(E_CLIENT)
	$(LD) -o e_client $(E_CLIENT) $(LIBS)


EGROUND = eground.o ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

eground: $(EGROUND)
	$(LD) -o eground $(EGROUND) $(LIBS)

ENORMALIZER = enormalizer.o ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

enormalizer: $(ENORMALIZER)
	$(LD) -o enormalizer $(ENORMALIZER) $(LIBS)


EDPLL = edpll.o ../lib/PROPOSITIONAL.a ../lib/CLAUSES.a\
        ../lib/ORDERINGS.a ../lib/TERMS.a ../lib/INOUT.a\
	../lib/BASICS.a

edpll: $(EDPLL)
	$(LD) -o edpll $(EDPLL) $(LIBS)

CLASSIFY = classify_problem.o ../lib/HEURISTICS.a\
            ../lib/CLAUSES.a ../lib/ORDERINGS.a ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

classify_problem: $(CLASSIFY)
	$(LD) -o classify_problem $(CLASSIFY) $(LIBS)

TERMPROPS = termprops.o ../lib/TERMS.a\
            ../lib/INOUT.a ../lib/BASICS.a

termprops: $(TERMPROPS)
	$(LD) -o termprops $(TERMPROPS) $(LIBS)

DIRECT_EXAMPLES = direct_examples.o \
            ../lib/PCL2.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

direct_examples: $(DIRECT_EXAMPLES)
	$(LD) -o direct_examples $(DIRECT_EXAMPLES) $(LIBS)


EPCLANALYSE = epclanalyse.o \
        ../lib/PCL2.a ../lib/HEURISTICS.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
        ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

epclanalyse: $(EPCLANALYSE)
	$(LD) -o epclanalyse $(EPCLANALYSE) $(LIBS)


EPCLLEMMA = epcllemma.o \
        ../lib/PCL2.a ../lib/HEURISTICS.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
        ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

epcllemma: $(EPCLLEMMA)
	$(LD) -o epcllemma $(EPCLLEMMA) $(LIBS)


EPCLEXTRACT = epclextract.o \
        ../lib/PCL2.a ../lib/HEURISTICS.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
        ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

epclextract: $(EPCLEXTRACT)
	$(LD) -o epclextract $(EPCLEXTRACT) $(LIBS)

CHECKPROOF = checkproof.o \
        ../lib/PCL2.a ../lib/CLAUSES.a ../lib/ORDERINGS.a \
        ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

checkproof: $(CHECKPROOF)
	$(LD) -o checkproof $(CHECKPROOF) $(LIBS)

EKB_CREATE = ekb_create.o \
            ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

ekb_create: $(EKB_CREATE)
	$(LD) -o ekb_create $(EKB_CREATE) $(LIBS)

EKB_INSERT = ekb_insert.o \
            ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

ekb_insert: $(EKB_INSERT)
	$(LD) -o ekb_insert $(EKB_INSERT) $(LIBS)

EKB_GINSERT = ekb_ginsert.o \
	    ../lib/PCL2.a ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

ekb_ginsert: $(EKB_GINSERT)
	$(LD) -o ekb_ginsert $(EKB_GINSERT) $(LIBS)

EKB_DELETE = ekb_delete.o \
            ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

ekb_delete: $(EKB_DELETE)
	$(LD) -o ekb_delete $(EKB_DELETE) $(LIBS)

TSM_CLASSIFY = tsm_classify.o \
            ../lib/LEARN.a ../lib/ANALYSIS.a ../lib/CLAUSES.a \
            ../lib/ORDERINGS.a \
            ../lib/TERMS.a ../lib/INOUT.a ../lib/BASICS.a

tsm_classify: $(TSM_CLASSIFY)
	$(LD) -o tsm_classify $(TSM_CLASSIFY) $(LIBS)


include Makefile.dependencies
