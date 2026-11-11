#------------------------------------------------------------------------
#
# File  : Makefile for CLAUSES
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the CLAUSES library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) CLAUSES/CLAUSES.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
