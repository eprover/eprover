#------------------------------------------------------------------------
#
# File  : Makefile for TERMS
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the TERMS library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) TERMS/TERMS.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
