#------------------------------------------------------------------------
#
# File  : Makefile for PROVER
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the libraries and the programs, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) programs

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	cd .. && $(MAKE) clean_programs
