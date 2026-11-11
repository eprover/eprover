#------------------------------------------------------------------------
#
# File  : Makefile for CONTRIB
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the CONTRIB library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) CONTRIB/CONTRIB.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.a picosat-965/picosat.o
