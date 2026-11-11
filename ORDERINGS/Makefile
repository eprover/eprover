#------------------------------------------------------------------------
#
# File  : Makefile for ORDERINGS
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the ORDERINGS library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) ORDERINGS/ORDERINGS.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
