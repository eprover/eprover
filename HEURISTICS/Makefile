#------------------------------------------------------------------------
#
# File  : Makefile for HEURISTICS
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the HEURISTICS library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) HEURISTICS/HEURISTICS.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
