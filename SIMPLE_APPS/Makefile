#------------------------------------------------------------------------
#
# File  : Makefile for SIMPLE_APPS
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the programs in this directory, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) SIMPLE_APPS/ex_commandline SIMPLE_APPS/term2dag

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	cd .. && $(MAKE) clean_extras
