#------------------------------------------------------------------------
#
# File  : Makefile for PROPOSITIONAL
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the PROPOSITIONAL library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) PROPOSITIONAL/PROPOSITIONAL.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
