#------------------------------------------------------------------------
#
# File  : Makefile (top level make file for E and its libraries)
#
# Author: Stephan Schulz
#
# The single make file for E. It only uses POSIX make features, so it
# should work with GNU make, BSD make and other POSIX makes.
#
# - Makefile.vars holds tools and compile options (edit it to taste).
# - config.mk is written by ./configure (installation paths,
#   higher-order support, comment character, debug build).
# - deps.mk holds the header dependencies. It is written by
#   ./configure and "make depend".
#
# Main targets:
#
#   make            Build the libraries and the programs in PROVER/
#   make install    Copy programs and man pages to the configured
#                   directories
#   make clean      Remove objects, libraries and programs
#   make rebuild    clean, depend, make
#   make extras     Build the programs in SIMPLE_APPS/ and EXTERNAL/
#
# Every object depends on Makefile.vars and config.mk, so changing
# the configuration rebuilds everything.
#
# Created: Sun Jul  6 22:55:11 MET DST 1997
# Rewritten as a single POSIX make file: Nov 2026
#
#------------------------------------------------------------------------

.POSIX:

include Makefile.vars
include config.mk

all: programs


#------------------------------------------------------------------------
# Sources
#------------------------------------------------------------------------

BASICS_OBJ = BASICS/clb_error.o BASICS/clb_memory.o \
	BASICS/clb_os_wrapper.o BASICS/clb_dstrings.o BASICS/clb_verbose.o \
	BASICS/clb_stringtrees.o BASICS/clb_numtrees.o \
	BASICS/clb_numxtrees.o BASICS/clb_numarrtrees.o \
	BASICS/clb_floattrees.o BASICS/clb_pstacks.o BASICS/clb_pqueue.o \
	BASICS/clb_dstacks.o BASICS/clb_ptrees.o BASICS/clb_orderedsets.o \
	BASICS/clb_quadtrees.o BASICS/clb_regmem.o \
	BASICS/clb_permastrings.o BASICS/clb_objtrees.o \
	BASICS/clb_objmaps.o BASICS/clb_fixdarrays.o BASICS/clb_plist.o \
	BASICS/clb_pdarrays.o BASICS/clb_pdrangearrays.o \
	BASICS/clb_ddarrays.o BASICS/clb_sysdate.o $(INTMAP_OBJ) \
	BASICS/clb_simple_stuff.o BASICS/clb_partial_orderings.o \
	BASICS/clb_plocalstacks.o BASICS/clb_min_heap.o

INOUT_OBJ = INOUT/cio_output.o INOUT/cio_simplestuff.o \
	INOUT/cio_fileops.o INOUT/cio_streams.o INOUT/cio_initio.o \
	INOUT/cio_scanner.o INOUT/cio_network.o INOUT/cio_multiplexer.o \
	INOUT/cio_basicparser.o INOUT/cio_commandline.o \
	INOUT/cio_filevars.o INOUT/cio_tempfile.o INOUT/cio_signals.o

TERMS_OBJ = TERMS/cte_functypes.o TERMS/cte_signature.o \
	TERMS/cte_termtypes.o TERMS/cte_termvars.o TERMS/cte_acterms.o \
	TERMS/cte_varhash.o TERMS/cte_varsets.o TERMS/cte_termfunc.o \
	TERMS/cte_termtrees.o TERMS/cte_termcellstore.o \
	TERMS/cte_garbage_coll.o TERMS/cte_termbanks.o TERMS/cte_subst.o \
	TERMS/cte_termpos.o TERMS/cte_termcpos.o TERMS/cte_replace.o \
	TERMS/cte_match_mgu_1-1.o TERMS/cte_idx_fp.o TERMS/cte_fp_index.o \
	TERMS/cte_simpletypes.o TERMS/cte_typecheck.o \
	TERMS/cte_typebanks.o TERMS/cte_termweightext.o \
	TERMS/cte_lambda.o TERMS/cte_dbvars.o TERMS/cte_ho_bindings.o \
	TERMS/cte_ho_csu.o TERMS/cte_pattern_match_mgu.o \
	TERMS/cte_fixpoint_unif.o

ORDERINGS_OBJ = ORDERINGS/cto_ocb.o ORDERINGS/cto_cmpcache.o \
	ORDERINGS/cto_lpo.o ORDERINGS/cto_kbo.o ORDERINGS/cto_kbolin.o \
	ORDERINGS/cto_orderings.o

CLAUSES_OBJ = CLAUSES/ccl_ext_index.o CLAUSES/ccl_neweval.o \
	CLAUSES/ccl_eqn.o CLAUSES/ccl_eqnlist.o CLAUSES/ccl_clauseinfo.o \
	CLAUSES/ccl_clauses.o CLAUSES/ccl_gd_transformation.o \
	CLAUSES/ccl_tformulae.o CLAUSES/ccl_formula_wrapper.o \
	CLAUSES/ccl_formulasets.o CLAUSES/ccl_f_generality.o \
	CLAUSES/ccl_sine.o CLAUSES/ccl_garbage_coll.o CLAUSES/ccl_tcnf.o \
	CLAUSES/ccl_propclauses.o CLAUSES/ccl_tautologies.o \
	CLAUSES/ccl_clausepos.o CLAUSES/ccl_clausecpos.o \
	CLAUSES/ccl_pdtrees.o CLAUSES/ccl_freqvectors.o \
	CLAUSES/ccl_featuresets.o CLAUSES/ccl_fcvindexing.o \
	CLAUSES/ccl_ftsplit.o CLAUSES/ccl_ftindexing.o CLAUSES/ccl_fsindexing.o \
	CLAUSES/ccl_clausesets.o \
	CLAUSES/ccl_unfold_defs.o CLAUSES/ccl_clausefunc.o \
	CLAUSES/ccl_formulafunc.o CLAUSES/ccl_groundconstr.o \
	CLAUSES/ccl_grounding.o CLAUSES/ccl_g_lithash.o \
	CLAUSES/ccl_axiomsorter.o CLAUSES/ccl_findex.o \
	CLAUSES/ccl_clausepos_tree.o CLAUSES/ccl_subterm_tree.o \
	CLAUSES/ccl_subterm_index.o CLAUSES/ccl_overlap_index.o \
	CLAUSES/ccl_relevance.o CLAUSES/ccl_inferencedoc.o \
	CLAUSES/ccl_derivation.o CLAUSES/ccl_paramod.o \
	CLAUSES/ccl_factor.o CLAUSES/ccl_eqnresolution.o \
	CLAUSES/ccl_rewrite.o CLAUSES/ccl_unit_simplify.o \
	CLAUSES/ccl_subsumption.o CLAUSES/ccl_condensation.o \
	CLAUSES/ccl_context_sr.o CLAUSES/ccl_diseq_decomp.o \
	CLAUSES/ccl_def_handling.o CLAUSES/ccl_splitting.o \
	CLAUSES/ccl_global_indices.o CLAUSES/ccl_satinterface.o \
	CLAUSES/ccl_proofstate.o CLAUSES/ccl_bce.o CLAUSES/ccl_pred_elim.o

PROPOSITIONAL_OBJ = PROPOSITIONAL/cpr_propsig.o \
	PROPOSITIONAL/cpr_varset.o PROPOSITIONAL/cpr_propclauses.o \
	PROPOSITIONAL/cpr_dpllformula.o PROPOSITIONAL/cpr_dpll.o

LEARN_OBJ = LEARN/cle_patterns.o LEARN/cle_clauseenc.o \
	LEARN/cle_annotations.o LEARN/cle_annoterms.o \
	LEARN/cle_flatannoterms.o LEARN/cle_numfeatures.o \
	LEARN/cle_examplerep.o LEARN/cle_kbdesc.o LEARN/cle_kbinsert.o \
	LEARN/cle_termtops.o LEARN/cle_indexfunctions.o LEARN/cle_tsm.o \
	LEARN/cle_classification.o LEARN/cle_tsmio.o

PCL2_OBJ = PCL2/pcl_idents.o PCL2/pcl_positions.o \
	PCL2/pcl_expressions.o PCL2/pcl_steps.o PCL2/pcl_protocol.o \
	PCL2/pcl_miniclauses.o PCL2/pcl_ministeps.o \
	PCL2/pcl_miniprotocol.o PCL2/pcl_lemmas.o PCL2/pcl_analysis.o \
	PCL2/pcl_propanalysis.o PCL2/pcl_proofcheck.o

HEURISTICS_OBJ = HEURISTICS/che_wfcb.o HEURISTICS/che_wfcbadmin.o \
	HEURISTICS/che_axiomscan.o HEURISTICS/che_clausefeatures.o \
	HEURISTICS/che_prio_funs.o HEURISTICS/che_clausesetfeatures.o \
	HEURISTICS/che_rawspecfeatures.o HEURISTICS/che_specsigfeatures.o \
	HEURISTICS/che_clauseweight.o HEURISTICS/che_gdweight.o \
	HEURISTICS/che_dagweight.o HEURISTICS/che_refinedweight.o \
	HEURISTICS/che_varweights.o HEURISTICS/che_funweights.o \
	HEURISTICS/che_orientweight.o HEURISTICS/che_fifo.o \
	HEURISTICS/che_lifo.o HEURISTICS/che_learning.o \
	HEURISTICS/che_simweight.o HEURISTICS/che_to_params.o \
	HEURISTICS/che_hcb.o HEURISTICS/che_litselection.o \
	HEURISTICS/che_proofcontrol.o HEURISTICS/che_hcbadmin.o \
	HEURISTICS/che_heuristics.o HEURISTICS/che_fcode_featurearrays.o \
	HEURISTICS/che_to_weightgen.o HEURISTICS/che_to_precgen.o \
	HEURISTICS/che_to_autoselect.o HEURISTICS/che_axfilter.o \
	HEURISTICS/che_strucweight.o HEURISTICS/che_prefixweight.o \
	HEURISTICS/che_treeweight.o HEURISTICS/che_levweight.o \
	HEURISTICS/che_tfidfweight.o HEURISTICS/che_termweights.o \
	HEURISTICS/che_termweight.o HEURISTICS/che_random.o \
	HEURISTICS/che_diversityweight.o HEURISTICS/che_new_autoschedule.o

CONTROL_OBJ = CONTROL/cco_simplification.o CONTROL/cco_factoring.o \
	CONTROL/cco_eqnresolving.o CONTROL/cco_paramodulation.o \
	CONTROL/cco_diseq_decomp.o CONTROL/cco_forward_contraction.o \
	CONTROL/cco_clausesplitting.o CONTROL/cco_interpreted.o \
	CONTROL/cco_proofproc.o CONTROL/cco_proc_ctrl.o \
	CONTROL/cco_gproc_ctrl.o CONTROL/cco_batch_spec.o \
	CONTROL/cco_einteractive_mode.o CONTROL/cco_sine.o \
	CONTROL/cco_esession.o CONTROL/cco_eserver.o \
	CONTROL/cco_scheduling.o CONTROL/cco_ho_inferences.o \
	CONTROL/cco_preprocessing.o

# PicoSAT is compiled with its own flags (see below).
PICOSAT = CONTRIB/picosat-965
CONTRIB_OBJ = $(PICOSAT)/picosat.o

# The libraries, in link order (a library may only depend on the ones
# after it).
ELIBS = CONTROL/CONTROL.a PCL2/PCL2.a HEURISTICS/HEURISTICS.a \
	LEARN/LEARN.a PROPOSITIONAL/PROPOSITIONAL.a CLAUSES/CLAUSES.a \
	ORDERINGS/ORDERINGS.a TERMS/TERMS.a INOUT/INOUT.a BASICS/BASICS.a \
	CONTRIB/CONTRIB.a

PROGRAMS = PROVER/$(EPROVER_BIN) PROVER/e_ltb_runner PROVER/e_stratpar \
	PROVER/e_deduction_server PROVER/e_axfilter \
	PROVER/classify_problem PROVER/epatternize PROVER/e_client \
	PROVER/e_server PROVER/direct_examples PROVER/epclanalyse \
	PROVER/epclextract PROVER/checkproof PROVER/eground \
	PROVER/enormalizer PROVER/edpll PROVER/epcllemma \
	PROVER/ekb_create PROVER/ekb_insert PROVER/ekb_ginsert \
	PROVER/ekb_delete

PROVER_OBJ = PROVER/eprover.o PROVER/e_ltb_runner.o PROVER/e_stratpar.o \
	PROVER/e_deduction_server.o PROVER/e_axfilter.o \
	PROVER/classify_problem.o PROVER/epatternize.o PROVER/e_client.o \
	PROVER/e_server.o PROVER/direct_examples.o PROVER/epclanalyse.o \
	PROVER/epclextract.o PROVER/checkproof.o PROVER/eground.o \
	PROVER/enormalizer.o PROVER/edpll.o PROVER/epcllemma.o \
	PROVER/ekb_create.o PROVER/ekb_insert.o PROVER/ekb_ginsert.o \
	PROVER/ekb_delete.o

EXTRAS = SIMPLE_APPS/ex_commandline SIMPLE_APPS/term2dag \
	EXTERNAL/CSSCPA_filter

EXTRAS_OBJ = SIMPLE_APPS/ex_commandline.o SIMPLE_APPS/term2dag.o \
	EXTERNAL/CSSCPA_filter.o EXTERNAL/cex_csscpa.o

LIB_OBJ = $(BASICS_OBJ) $(INOUT_OBJ) $(TERMS_OBJ) $(ORDERINGS_OBJ) \
	$(CLAUSES_OBJ) $(PROPOSITIONAL_OBJ) $(LEARN_OBJ) $(PCL2_OBJ) \
	$(HEURISTICS_OBJ) $(CONTROL_OBJ)

ALL_OBJ = $(LIB_OBJ) $(PROVER_OBJ) $(EXTRAS_OBJ)

# All installed programs and man pages. Programs that have not been
# built (e.g. eprover-ho in a first-order build) are skipped.
INSTALL_PROGS = eprover eprover-ho epclextract e_stratpar eground \
	e_ltb_runner e_deduction_server e_axfilter checkproof ekb_create \
	ekb_delete ekb_ginsert ekb_insert

MAN_PAGES = eprover e_stratpar eground epclextract e_ltb_runner \
	e_deduction_server e_axfilter checkproof ekb_create ekb_delete \
	ekb_ginsert ekb_insert


#------------------------------------------------------------------------
# Compilation
#------------------------------------------------------------------------

INCLUDES = -IBASICS -IINOUT -ITERMS -IORDERINGS -ICLAUSES \
	-IPROPOSITIONAL -ILEARN -IPCL2 -IHEURISTICS -ICONTROL -IPROVER \
	-IEXTERNAL -I$(PICOSAT)

DEBUGFLAGS  = $(PROFFLAGS) $(MEMDEBUG) $(DEBUGGER) $(NODEBUG)
ALL_CFLAGS  = $(OPTFLAGS) $(LTOFLAGS) $(WFLAGS) $(DEBUGFLAGS) \
	$(BUILDFLAGS) $(CONFIGFLAGS) -std=gnu99 $(INCLUDES)
LD          = $(CC) $(OPTFLAGS) $(LTOFLAGS) $(PROFFLAGS) $(DEBUGGER)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(ALL_CFLAGS) -c -o $@ $<

# PicoSAT uses the flags its own configure script uses for E.
PICOSAT_CFLAGS = -DTRACE -Wall -Wextra -Wno-unused-but-set-variable \
	-DNDEBUG -O3

$(PICOSAT)/picosat.o: $(PICOSAT)/picosat.c $(PICOSAT)/picosat.h
	$(CC) $(PICOSAT_CFLAGS) -c -o $@ $(PICOSAT)/picosat.c

$(ALL_OBJ) $(CONTRIB_OBJ): Makefile.vars config.mk


#------------------------------------------------------------------------
# Libraries
#------------------------------------------------------------------------

BASICS/BASICS.a: $(BASICS_OBJ)
	rm -f $@; $(AR) $@ $(BASICS_OBJ)

INOUT/INOUT.a: $(INOUT_OBJ)
	rm -f $@; $(AR) $@ $(INOUT_OBJ)

TERMS/TERMS.a: $(TERMS_OBJ)
	rm -f $@; $(AR) $@ $(TERMS_OBJ)

ORDERINGS/ORDERINGS.a: $(ORDERINGS_OBJ)
	rm -f $@; $(AR) $@ $(ORDERINGS_OBJ)

CLAUSES/CLAUSES.a: $(CLAUSES_OBJ)
	rm -f $@; $(AR) $@ $(CLAUSES_OBJ)

PROPOSITIONAL/PROPOSITIONAL.a: $(PROPOSITIONAL_OBJ)
	rm -f $@; $(AR) $@ $(PROPOSITIONAL_OBJ)

LEARN/LEARN.a: $(LEARN_OBJ)
	rm -f $@; $(AR) $@ $(LEARN_OBJ)

PCL2/PCL2.a: $(PCL2_OBJ)
	rm -f $@; $(AR) $@ $(PCL2_OBJ)

HEURISTICS/HEURISTICS.a: $(HEURISTICS_OBJ)
	rm -f $@; $(AR) $@ $(HEURISTICS_OBJ)

CONTROL/CONTROL.a: $(CONTROL_OBJ)
	rm -f $@; $(AR) $@ $(CONTROL_OBJ)

CONTRIB/CONTRIB.a: $(CONTRIB_OBJ)
	rm -f $@; $(AR) $@ $(CONTRIB_OBJ)

libs: $(ELIBS)


#------------------------------------------------------------------------
# Programs (every program is linked against all libraries; the linker
# only takes what is needed)
#------------------------------------------------------------------------

programs: $(PROGRAMS)

extras: $(EXTRAS)

PROVER/$(EPROVER_BIN): PROVER/eprover.o $(ELIBS)
	$(LD) -o $@ PROVER/eprover.o $(ELIBS) $(LIBS)

PROVER/e_ltb_runner: PROVER/e_ltb_runner.o $(ELIBS)
	$(LD) -o $@ PROVER/e_ltb_runner.o $(ELIBS) $(LIBS)

PROVER/e_stratpar: PROVER/e_stratpar.o $(ELIBS)
	$(LD) -o $@ PROVER/e_stratpar.o $(ELIBS) $(LIBS)

PROVER/e_deduction_server: PROVER/e_deduction_server.o $(ELIBS)
	$(LD) -o $@ PROVER/e_deduction_server.o $(ELIBS) $(LIBS)

PROVER/e_axfilter: PROVER/e_axfilter.o $(ELIBS)
	$(LD) -o $@ PROVER/e_axfilter.o $(ELIBS) $(LIBS)

PROVER/classify_problem: PROVER/classify_problem.o $(ELIBS)
	$(LD) -o $@ PROVER/classify_problem.o $(ELIBS) $(LIBS)

PROVER/epatternize: PROVER/epatternize.o $(ELIBS)
	$(LD) -o $@ PROVER/epatternize.o $(ELIBS) $(LIBS)

PROVER/e_client: PROVER/e_client.o $(ELIBS)
	$(LD) -o $@ PROVER/e_client.o $(ELIBS) $(LIBS)

PROVER/e_server: PROVER/e_server.o $(ELIBS)
	$(LD) -o $@ PROVER/e_server.o $(ELIBS) $(LIBS)

PROVER/direct_examples: PROVER/direct_examples.o $(ELIBS)
	$(LD) -o $@ PROVER/direct_examples.o $(ELIBS) $(LIBS)

PROVER/epclanalyse: PROVER/epclanalyse.o $(ELIBS)
	$(LD) -o $@ PROVER/epclanalyse.o $(ELIBS) $(LIBS)

PROVER/epclextract: PROVER/epclextract.o $(ELIBS)
	$(LD) -o $@ PROVER/epclextract.o $(ELIBS) $(LIBS)

PROVER/checkproof: PROVER/checkproof.o $(ELIBS)
	$(LD) -o $@ PROVER/checkproof.o $(ELIBS) $(LIBS)

PROVER/eground: PROVER/eground.o $(ELIBS)
	$(LD) -o $@ PROVER/eground.o $(ELIBS) $(LIBS)

PROVER/enormalizer: PROVER/enormalizer.o $(ELIBS)
	$(LD) -o $@ PROVER/enormalizer.o $(ELIBS) $(LIBS)

PROVER/edpll: PROVER/edpll.o $(ELIBS)
	$(LD) -o $@ PROVER/edpll.o $(ELIBS) $(LIBS)

PROVER/epcllemma: PROVER/epcllemma.o $(ELIBS)
	$(LD) -o $@ PROVER/epcllemma.o $(ELIBS) $(LIBS)

PROVER/ekb_create: PROVER/ekb_create.o $(ELIBS)
	$(LD) -o $@ PROVER/ekb_create.o $(ELIBS) $(LIBS)

PROVER/ekb_insert: PROVER/ekb_insert.o $(ELIBS)
	$(LD) -o $@ PROVER/ekb_insert.o $(ELIBS) $(LIBS)

PROVER/ekb_ginsert: PROVER/ekb_ginsert.o $(ELIBS)
	$(LD) -o $@ PROVER/ekb_ginsert.o $(ELIBS) $(LIBS)

PROVER/ekb_delete: PROVER/ekb_delete.o $(ELIBS)
	$(LD) -o $@ PROVER/ekb_delete.o $(ELIBS) $(LIBS)

SIMPLE_APPS/ex_commandline: SIMPLE_APPS/ex_commandline.o $(ELIBS)
	$(LD) -o $@ SIMPLE_APPS/ex_commandline.o $(ELIBS) $(LIBS)

SIMPLE_APPS/term2dag: SIMPLE_APPS/term2dag.o $(ELIBS)
	$(LD) -o $@ SIMPLE_APPS/term2dag.o $(ELIBS) $(LIBS)

EXTERNAL/CSSCPA_filter: EXTERNAL/CSSCPA_filter.o EXTERNAL/cex_csscpa.o \
	$(ELIBS)
	$(LD) -o $@ EXTERNAL/CSSCPA_filter.o EXTERNAL/cex_csscpa.o \
	$(ELIBS) $(LIBS)


#------------------------------------------------------------------------
# Generated files
#------------------------------------------------------------------------

# The commit id for --version. The script only rewrites the file if
# the id changes, so this does not cause needless recompilation.
PROVER/e_gitcommit.h: FORCE
	@sh development_tools/e_gitcommit PROVER/e_gitcommit.h

commit_id: PROVER/e_gitcommit.h

FORCE:

# Header dependencies of all objects. The compiler has to support -MM
# and -MT (gcc and clang do).
depend: PROVER/e_gitcommit.h
	@echo "Generating deps.mk"
	@echo "# Header dependencies, generated by make depend" > deps.mk.tmp
	@for f in $(ALL_OBJ:.o=.c); do \
	   $(CC) $(ALL_CFLAGS) -MM -MT `echo $$f | sed 's/\.c$$/.o/'` \
	      $$f >> deps.mk.tmp || exit 1; \
	done
	@mv deps.mk.tmp deps.mk

README: README.md
	grep -v '```' README.md > README


#------------------------------------------------------------------------
# Cleaning and rebuilding
#------------------------------------------------------------------------

clean: clean_programs clean_extras
	rm -f $(LIB_OBJ) $(CONTRIB_OBJ) $(ELIBS)
	cd DOC && $(MAKE) clean

clean_programs:
	rm -f $(PROVER_OBJ) $(PROGRAMS) PROVER/eprover PROVER/eprover-ho
	rm -rf PROVER/*.dSYM

clean_extras:
	rm -f $(EXTRAS_OBJ) $(EXTRAS)

# Also remove editor backups and the generated files of the old
# (recursive) build system. The configuration (config.mk, deps.mk) is
# kept, so make still works afterwards. distrib leaves it out of the
# archive.
cleandist: clean
	rm -f deps.mk.tmp *~ */*~
	rm -rf include lib
	rm -f */Makefile.dependencies
	rm -f $(PICOSAT)/makefile $(PICOSAT)/config.h $(PICOSAT)/*.o \
	   $(PICOSAT)/libpicosat.a $(PICOSAT)/picosat $(PICOSAT)/picomcs \
	   $(PICOSAT)/picomus $(PICOSAT)/picogcnf

rebuild:
	$(MAKE) clean
	$(MAKE) depend
	$(MAKE)

# Configure with default options (release build, programs used in
# place), or reconfigure with the last ./configure options plus
# debugging support.
default_config:
	./configure

debug_config:
	./configure $(CONFIG_ARGS) --debug


#------------------------------------------------------------------------
# Installation
#------------------------------------------------------------------------

install:
	mkdir -p $(EXECPATH)
	for p in $(INSTALL_PROGS); do \
	   sh development_tools/e_install PROVER/$$p $(EXECPATH); \
	done
	mkdir -p $(MANPATH)
	for p in $(MAN_PAGES); do \
	   sh development_tools/e_install DOC/man/$$p.1 $(MANPATH); \
	done


#------------------------------------------------------------------------
# Documentation
#------------------------------------------------------------------------

documentation: README
	cd DOC && $(MAKE)

# A sub-make, as the configuration may just have been changed (distrib).
man:
	$(MAKE) programs
	mkdir -p DOC/man
	for p in $(MAN_PAGES); do \
	   help2man -N -i DOC/bug_reporting PROVER/$$p > DOC/man/$$p.1 \
	      || exit 1; \
	done

tags:
	etags-emacs `find . \( -name "*.[ch]" -or -name "*.py" \) -and \
	   \( -not -path "*include*" -and -not -name ".#*" \)`


#------------------------------------------------------------------------
# Distributions
#------------------------------------------------------------------------

# Build a distribution
distrib: default_config commit_id man documentation cleandist
	@echo "Did you think about: "
	@echo " - Changing the bibliographies to local version"
	@echo " - increasing the dev version number and committing to git?"
	@echo "    ??? "
	@p=`basename "$$PWD"`; echo "" > etc/NO_DISTRIB; \
	cd ..; find $$p -name ".git" -print >> $$p/etc/NO_DISTRIB; \
	echo "$$p/config.mk" >> $$p/etc/NO_DISTRIB; \
	echo "$$p/deps.mk" >> $$p/etc/NO_DISTRIB; \
	$(TAR) cfX - $$p/etc/NO_DISTRIB $$p | $(GZIP) - -c > $$p.tgz

# Include the GIT subdirectories (and non-GPL files, of which there
# currently are none).
fulldistrib: man documentation cleandist default_config
	@echo "Warning: You are building a full archive!"
	@echo "Did you remember to increase the dev version number and commit to git?"
	@p=`basename "$$PWD"`; cd ..; \
	$(TAR) cf - $$p | $(GZIP) - -c > $${p}_FULL.tgz

# Build StarExec package. This is not supposed to be super-portable
# StarExec runs all binaries from its local bin/, so we cheat

starexec:
	echo $(STAREXECPATH)
	rm -rf $(STAREXECPATH)
	find . -name ".#*"  -exec rm {} \;
	$(MAKE) clean
	./configure --bindir="."
	$(MAKE)
	./configure --prefix=$(STAREXECPATH)
	$(MAKE) install
	$(MAKE) clean
	./configure --bindir="." --enable-ho
	$(MAKE)
	./configure --prefix=$(STAREXECPATH) --enable-ho
	$(MAKE) install
	cp etc/STAREXEC3.0/starexec_run* $(STAREXECPATH)/bin
	v=`$(STAREXECPATH)/bin/eprover --version | cut -d' ' -f1-2 | \
	   sed -e 's/ /-/'`; cd $(STAREXECPATH); zip -r $$v.zip bin man

starexec-src:
	echo $(STAREXECPATH)
	rm -rf $(STAREXECPATH)
	mkdir $(STAREXECPATH)
	find . -name ".#*"  -exec rm {} \;
	$(MAKE) distrib
	cp ../E.tgz $(STAREXECPATH)
	$(MAKE) clean
	./configure --bindir="."
	$(MAKE)
	./configure --prefix=$(STAREXECPATH)
	$(MAKE) install
	cp etc/STAREXEC3.0/starexec_run* $(STAREXECPATH)/bin
	cp etc/starexec_build $(STAREXECPATH)
	v=`$(STAREXECPATH)/bin/eprover --version | cut -d' ' -f1-2 | \
	   sed -e 's/ /-/'`; cd $(STAREXECPATH); \
	zip -r $${v}_src.zip bin man E.tgz starexec_build

J = 4
benchpress-quick:
	@echo "run provers on example problems..."
	benchpress run -j $(J) -c etc/benchpress.sexp \
	   --task eprover-quick-test --progress

include deps.mk
