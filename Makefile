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

include Makefile.vars

# Project specific variables

PROJECT = E

LIBS     = BASICS INOUT TERMS ORDERINGS CLAUSES PROPOSITIONAL LEARN \
           ANALYSIS PCL2 HEURISTICS CONTROL EXTERNAL
CODE     = $(LIBS) PROVER TEST SIMPLE_APPS 
PARTS    = $(CODE) DOC $(SAFELOGIC)

all: warn E

# Warn uninformed first-time installers, e.g. Bernd Loechner ;-)

warn:
	@echo "If you try to install E (instead of just remaking"
	@echo "the system after a change), you need to type"
	@echo ""
	@echo "    make install"
	@echo ""
	@echo "instead of plain make. Installing will rebuild the"
	@echo "system from scratch and adapt all scripts to run in the"
	@echo "current directory and with the tools found on your"
	@echo "system (if any). If you just want to rebuild the system"
	@echo "without  configuration, use"
	@echo ""
	@echo "    make rebuild"
	@echo ""
	@echo "-------------------------------------------------"

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

# Build a distribution

distrib: clean
	@echo "Did you think about: "
	@echo " - Changing the bibliographies to local version"
	@echo "    ??? "
	@cp etc/PROPRIETARY etc/NO_DISTRIB
	@sed -e 's/^SAFELOGIC  = SAFELOGIC/SAFELOGIC  = /' \
             -e 's/^             -DSAFELOGIC \\/             # -DSAFELOGIC \\/' \
            Makefile.vars > tmpfile; mv tmpfile Makefile.vars
	@cd ..; find $(PROJECT) -name "CVS" -print >> $(PROJECT)/etc/NO_DISTRIB;\
         $(TAR) cfX - $(PROJECT)/etc/NO_DISTRIB $(PROJECT) |$(GZIP) - -c > $(PROJECT).tgz

# Include proprietary code not part of the GPL'ed version

fulldistrib: clean
	@echo "Warning: You are building a full archive!"
	cd ..; $(TAR) cf - $(PROJECT)|$(GZIP) - -c > $(PROJECT)_FULL.tgz

# Make all library parts

top: E

# Create symbolic links

links: remove_links
	cd include;\
	for subdir in $(LIBS); do\
	   for file in ../$$subdir/*.h; do\
	     $(LN) $$file .;\
	   done;\
	done;
	cd lib;\
	for subdir in $(LIBS); do\
           $(LN) ../$$subdir/$$subdir.a .;\
	done;

tools:
	cd development_tools;$(MAKE) tools

# Rebuilding from scratch

rebuild:
	echo 'Rebuilding with debug options $(DEBUGFLAGS)'	
	$(MAKE) clean
	$(MAKE) links
	$(MAKE) depend
	$(MAKE)

# Install the complete library

install: 
	echo 'Installing and configuring in place!'
	$(MAKE) tools
	$(MAKE) rebuild

install-exec:	
	@echo "#!"`which bash` > tmpfile
	@echo "" >> tmpfile
	@echo "EXECPATH=$(EXECPATH)" >> tmpfile
	@tail +4 PROVER/eproof >> tmpfile
	@mv tmpfile PROVER/eproof
	@chmod ugo+x PROVER/eproof
	bash -c 'cp PROVER/eprover $(EXECPATH)'
	bash -c 'cp PROVER/epclextract $(EXECPATH)'
	bash -c 'cp PROVER/eproof $(EXECPATH)'
	bash -c 'cp  PROVER/eground $(EXECPATH)'	
#	bash -c 'install -c $(EXECPATH) PROVER/eprover'
#	bash -c 'install -c $(EXECPATH) PROVER/e2pcl'
#	bash -c 'install -c $(EXECPATH) PROVER/eproof'
#	bash -c 'install -c $(EXECPATH) PROVER/eground'

# Also remake documentation

remake: install documentation

documentation:
	cd DOC; $(MAKE)

# Build the single libraries

E:
	for subdir in $(CODE); do\
	   cd $$subdir;touch Makefile.dependencies;$(MAKE);cd ..;\
	done;


