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
# Created: Sun Jul  6 22:55:11 MET DST 1997
#
#------------------------------------------------------------------------

.PHONY: all depend remove_links clean cleandist default_config debug_config distrib fulldistrib top links tags rebuild install config remake documentation E man starexec starexec-src

include Makefile.vars

# Project specific variables


PROJECT  = $(shell basename `pwd`)

LIBS     = CONTRIB BASICS INOUT TERMS ORDERINGS CLAUSES PROPOSITIONAL LEARN \
           PCL2 HEURISTICS CONTROL
HEADERS  = $(LIBS) EXTERNAL PROVER
CODE     = $(LIBS) SIMPLE_APPS EXTERNAL PROVER
PARTS    = $(CODE) DOC


all: E


# Generate dependencies

depend:
	@for subdir in $(CODE); do\
		cd $$subdir; touch Makefile.dependencies; $(MAKE) depend; cd ..;\
	done;

# Remove all automatically generated files

remove_links:
	@if [ -d include ]; then\
		cd include; rm -f *.h;\
	fi;
	@if [ -d lib ]; then\
		cd lib;     rm -f *.a;\
	fi;
	@rm -f PROVER/picosat

clean: remove_links
	@for subdir in $(PARTS); do\
		cd $$subdir; touch Makefile.dependencies;$(MAKE) clean; cd ..;\
	done;

cleandist: clean
	@rm -f *~ */*~

default_config:
	./configure
	@cat Makefile.vars| \
	gawk '/^NODEBUG/{print "NODEBUG    = -DNDEBUG -DFAST_EXIT";next}/^MEMDEBUG/{print "MEMDEBUG   = # -DCLB_MEMORY_DEBUG # -DCLB_MEMORY_DEBUG2";next}/^DEBUGGER/{print "DEBUGGER   = # -g -ggdb";next}/^PROFFLAGS/{print "PROFFLAGS  = # -pg";next}{print}' > __tmpmake__;mv __tmpmake__ Makefile.vars


debug_config:
	@cat Makefile.vars| \
	gawk '/^NODEBUG/{print "NODEBUG    = # -DNDEBUG -DFAST_EXIT";next}/^MEMDEBUG/{print "MEMDEBUG   = -DCLB_MEMORY_DEBUG # -DCLB_MEMORY_DEBUG2";next}{print}' > __tmpmake__;mv __tmpmake__ Makefile.vars

commit_id:
	echo '#define ECOMMITID "'`git rev-parse HEAD`'"' > PROVER/e_gitcommit.h


# Build a distribution
distrib: default_config commit_id man documentation cleandist
	@echo $(MYVAR)
	@echo "Did you think about: "
	@echo " - Changing the bibliographies to local version"
	@echo " - increasing the dev version number and committing to git?"
	@echo "    ??? "
	@echo "" > etc/NO_DISTRIB
	@cd ..; find $(PROJECT) -name ".git" -print >> $(PROJECT)/etc/NO_DISTRIB;\
		$(TAR) cfX - $(PROJECT)/etc/NO_DISTRIB $(PROJECT) |$(GZIP) - -c > $(PROJECT).tgz

# Include the GIT subdirecctories (and non-GPL files, of which there currently are none).
fulldistrib: man documentation cleandist default_config
	@echo "Warning: You are building a full archive!"
	@echo "Did you remember to increase the dev version number and commit to git?"
	@cd ..; $(TAR) cf - $(PROJECT)|$(GZIP) - -c > $(PROJECT)_FULL.tgz

# Build StarExec package. This is not supposed to be super-portable
# StarExec runs all binaries from its local bin/, so we cheat

starexec:
	echo $(STAREXECPATH)
	rm -rf $(STAREXECPATH)
	find . -name ".#*"  -exec rm {} \;
	make clean
	./configure --bindir="."
	make
	./configure --prefix=$(STAREXECPATH)
	make install

	make clean
	./configure --bindir="." --enable-ho
	make
	./configure --prefix=$(STAREXECPATH) --enable-ho
	make install

	cp etc/STAREXEC3.0/starexec_run* $(STAREXECPATH)/bin
	$(eval E_VERSION=`$$(STAREXECPATH)/bin/eprover --version | cut -d' ' -f1-2| sed -e 's/ /-/'`)
	cd $(STAREXECPATH); zip -r $(E_VERSION).zip bin man

starexec-src:
	echo $(STAREXECPATH)
	rm -rf $(STAREXECPATH)
	mkdir $(STAREXECPATH)
	find . -name ".#*"  -exec rm {} \;
	make distrib
	cp ../E.tgz $(STAREXECPATH)
	make clean
	./configure --bindir="."
	make
	./configure --prefix=$(STAREXECPATH)
	make install

	cp etc/STAREXEC3.0/starexec_run* $(STAREXECPATH)/bin
	cp etc/starexec_build $(STAREXECPATH)
	$(eval E_VERSION=`$$(STAREXECPATH)/bin/eprover --version | cut -d' ' -f1-2| sed -e 's/ /-/'`)
	cd $(STAREXECPATH); zip -r $(E_VERSION)_src.zip bin man E.tgz starexec_build



# Make all library parts
top: E

# Create symbolic links
links: remove_links
	@mkdir -p include
	@cd include; find .. -not -path '../include/*' -name "[^.]*.h" -exec $(LN) {} \;
	@mkdir -p lib
#	@cd lib;find .. -not -path '../lib/*' -name "[^.]*.a" -exec $(LN) {} \;
	@cd lib;\
        for subdir in $(LIBS); do\
                $(LN) ../$$subdir/$$subdir.a .;\
        done;
# @cd PROVER; $(LN) $(PICOSAT)/picosat



tags:
	etags-emacs `find . \( -name "*.[ch]" -or -name "*.py" \) -and \( -not -path "*include*" -and -not -name ".#*" \)`
#ctags-exuberant -e -R .
# etags */*.c */*.h
# cd PYTHON; make ptags

# Rebuilding from scratch
rebuild:
	echo 'Rebuilding with debug options $(DEBUGFLAGS)'
	$(MAKE) clean
	$(MAKE) config
	$(MAKE)

# Configure the whole package
config:
	echo 'Configuring build system'
	$(MAKE) links
	$(MAKE) depend
	cd CONTRIB; $(MAKE) config


# Configure and copy executables to the installation directory
install:
	-sh -c 'mkdir -p $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/$(EPROVER_BIN) $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/eprover-ho     $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/epclextract    $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/e_stratpar     $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/eground        $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/e_ltb_runner   $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/e_deduction_server $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/e_axfilter     $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/checkproof     $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/ekb_create     $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/ekb_delete     $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/ekb_ginsert    $(EXECPATH)'
	-sh -c 'development_tools/e_install PROVER/ekb_insert     $(EXECPATH)'
	-sh -c 'development_tools/e_install CONTRIB/picosat-965/picosat $(EXECPATH)'
	-sh -c 'mkdir -p $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/eprover.1      $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/epclextract.1  $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/e_stratpar.1   $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/eground.1      $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/e_ltb_runner.1 $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/e_deduction_server.1 $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/e_axfilter.1   $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/checkproof.1   $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/ekb_create.1   $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/ekb_delete.1   $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/ekb_ginsert.1  $(MANPATH)'
	-sh -c 'development_tools/e_install DOC/man/ekb_insert.1   $(MANPATH)'


# Also remake documentation
remake: config rebuild documentation

README: README.md
	grep -v '```' README.md > README

documentation: README
	cd DOC; $(MAKE)

man: E
	mkdir -p DOC/man
	help2man -N -i DOC/bug_reporting PROVER/eprover      > DOC/man/eprover.1
	help2man -N -i DOC/bug_reporting PROVER/e_stratpar   > DOC/man/e_stratpar.1
	help2man -N -i DOC/bug_reporting PROVER/eground      > DOC/man/eground.1
	help2man -N -i DOC/bug_reporting PROVER/epclextract  > DOC/man/epclextract.1
	help2man -N -i DOC/bug_reporting PROVER/e_ltb_runner > DOC/man/e_ltb_runner.1
	help2man -N -i DOC/bug_reporting PROVER/e_deduction_server > DOC/man/e_deduction_server.1
	help2man -N -i DOC/bug_reporting PROVER/e_axfilter   > DOC/man/e_axfilter.1
	help2man -N -i DOC/bug_reporting PROVER/checkproof   > DOC/man/checkproof.1
	help2man -N -i DOC/bug_reporting PROVER/ekb_create   > DOC/man/ekb_create.1
	help2man -N -i DOC/bug_reporting PROVER/ekb_delete   > DOC/man/ekb_delete.1
	help2man -N -i DOC/bug_reporting PROVER/ekb_ginsert  > DOC/man/ekb_ginsert.1
	help2man -N -i DOC/bug_reporting PROVER/ekb_insert   > DOC/man/ekb_insert.1

# Build the single libraries
E: links
	@for subdir in $(CODE); do\
		cd $$subdir; touch Makefile.dependencies; $(MAKE); cd ..;\
	done;

J ?= 4
benchpress-quick:
	@echo "run provers on example problems..."
	benchpress run -j $(J) -c benchpress.sexp --task eprover-quick-test --progress

.PHONY: benchpress
