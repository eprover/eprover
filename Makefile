#------------------------------------------------------------------------
#
# File  : Makefile (top level make file for E and its libraries)
#
# Author: Stephan Schulz
#
# Top level make file. Check Makefile.vars for system-dependend tool
# selection and compilation options. 
#
# Changes
#
# <1> Sun Jul  6 22:55:11 MET DST 1997
#     New
# <2> Mon Jan 12 14:05:24 MET 1998
#     Extended for DOC directory
# <3> Fri Mar 13 17:09:13 MET 1998
#     Extended for CLAUSES directory
#
#------------------------------------------------------------------------

.PHONY: all depend remove_links clean cleandist default_config debug_config distrib fulldistrib top links tags tools rebuild install config remake documentation E man

 include Makefile.vars

# Project specific variables

PROJECT = E

LIBS     = BASICS INOUT TERMS ORDERINGS CLAUSES PROPOSITIONAL LEARN \
           ANALYSIS PCL2 HEURISTICS CONTROL
HEADERS  = $(LIBS) EXTERNAL PROVER
CODE     = $(LIBS) PROVER TEST SIMPLE_APPS EXTERNAL
PARTS    = $(CODE) DOC

all: E


# Generate dependencies

depend: 
	for subdir in $(CODE); do\
	   cd $$subdir; touch Makefile.dependencies; $(MAKE) depend; cd ..;\
	done;

# Remove all automatically generated files

remove_links:
	cd include; touch does_exist.h; rm *.h
	cd lib; touch does_exist.a; rm *.a


clean: remove_links
	for subdir in $(PARTS); do\
	   cd $$subdir; touch Makefile.dependencies;$(MAKE) clean; cd ..;\
	done;

cleandist: clean
	@touch dummy~ PROVER/dummy~
	rm *~ */*~


default_config:
	sed -e 's/CC         = kgcc/CC         = gcc/' Makefile.vars| \
	awk '/^NODEBUG/{print "NODEBUG    = -DNDEBUG -DFAST_EXIT";next}/^MEMDEBUG/{print "MEMDEBUG   = # -DCLB_MEMORY_DEBUG # -DCLB_MEMORY_DEBUG2";next}/^DEBUGGER/{print "DEBUGGER   = # -g -ggdb";next}{print}' > __tmpmake__;mv __tmpmake__ Makefile.vars

debug_config:
	cat Makefile.vars| \
	awk '/^NODEBUG/{print "NODEBUG    = # -DNDEBUG -DFAST_EXIT";next}/^MEMDEBUG/{print "MEMDEBUG   = -DCLB_MEMORY_DEBUG # -DCLB_MEMORY_DEBUG2";next}{print}' > __tmpmake__;mv __tmpmake__ Makefile.vars

# Build a distribution

distrib: man cleandist default_config
	@echo "Did you think about: "
	@echo " - Changing the bibliographies to local version"
	@echo "    ??? "
	@cp etc/PROPRIETARY etc/NO_DISTRIB
	@cd ..; find $(PROJECT) -name "CVS" -print >> $(PROJECT)/etc/NO_DISTRIB;\
         $(TAR) cfX - $(PROJECT)/etc/NO_DISTRIB $(PROJECT) |$(GZIP) - -c > $(PROJECT).tgz

# Include proprietary code not part of the GPL'ed version, 
# as well as CVS subdirecctories

fulldistrib: man cleandist default_config
	@echo "Warning: You are building a full archive!"
	@echo "Did you remember to increase the dev version number and commit to CVS?"
	cd ..; $(TAR) cf - $(PROJECT)|$(GZIP) - -c > $(PROJECT)_FULL.tgz

# Make all library parts

top: E

# Create symbolic links

links: remove_links
	cd include;\
	for subdir in $(HEADERS); do\
	   for file in ../$$subdir/*.h; do\
	     $(LN) $$file .;\
	   done;\
	done;
	cd lib;\
	for subdir in $(LIBS); do\
           $(LN) ../$$subdir/$$subdir.a .;\
	done;

tags: 
	etags */*.c */*.h
	cd PYTHON; make tags

tools:
	cd development_tools;$(MAKE) tools
	cd PYTHON; $(MAKE) tools

# Rebuilding from scratch

rebuild:
	echo 'Rebuilding with debug options $(DEBUGFLAGS)'	
	$(MAKE) clean
	$(MAKE) config
	$(MAKE) depend
	$(MAKE)

# Configure the whole package

config: 
	echo 'Configuring build system and tools'
	$(MAKE) links
	$(MAKE) tools
	$(MAKE) depend


# Configure and copy executables to the installation directory

# Old eproof config - I hope it now runs with /bin/sh, which is 
# guaranteed to be where it belongs. Kept as a historical reference.
# 	@echo "#!"`which bash`" -f" > tmpfile
#	@echo "" >> tmpfile
#	@echo "EXECPATH=$(EXECPATH)" >> tmpfile
#	@awk '{count++; if(count >= 4){print}}' PROVER/eproof >> tmpfile
#	@mv tmpfile PROVER/eproof


install: E
	-sh -c 'mkdir -p $(EXECPATH)'
	-sh -c 'cp PROVER/eprover $(EXECPATH)'
	-sh -c 'cp PROVER/epclextract $(EXECPATH)'
	-sh -c 'cp PROVER/eproof $(EXECPATH)'
	-sh -c 'cp  PROVER/eground $(EXECPATH)'	

# Also remake documentation

remake: config rebuild documentation

documentation:
	cd DOC; $(MAKE)

man: top
	help2man -N PROVER/eproof > DOC/man/eproof.1
	help2man -N PROVER/eprover > DOC/man/eprover.1
	help2man -N PROVER/eground > DOC/man/eground.1
	help2man -N PROVER/epclextract > DOC/man/epclextract.1

# Build the single libraries

E:
	for subdir in $(CODE); do\
	   cd $$subdir;touch Makefile.dependencies;$(MAKE);cd ..;\
	done;


