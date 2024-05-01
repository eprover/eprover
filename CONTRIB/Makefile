#------------------------------------------------------------------------
#
# File  : Makefile
#
# This is the Makefile for contributed code of the E distribution. This is
# more convoluted than I like because it deals with PicoSat, which follows
# some conventions that are different frome the ones in E.
#
# Author: Stephan Schulz
#
# Created: Mon Nov 27 12:28:52 CET 2017
#
#------------------------------------------------------------------------

include ../Makefile.vars

PICOSAT = picosat-965

all: picosat

# Remove all automatically generated files

clean: $(PICOSAT)/makefile
	cd $(PICOSAT); $(MAKE) clean
	@rm  -f *.a


# Services (provided by the master Makefile)

include ../Makefile.services

config:
	cd $(PICOSAT); ./configure.sh -t

$(PICOSAT)/makefile: $(PICOSAT)/makefile.in
	make config

picosat: $(PICOSAT)/makefile
	cd $(PICOSAT); $(MAKE); $(MAKE)
	@if [ ! -h CONTRIB.a ]; then\
	   $(LN) $(PICOSAT)/libpicosat.a CONTRIB.a;\
	fi;
