#------------------------------------------------------------------------
#
# File  : Makefile for INOUT
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the INOUT library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) INOUT/INOUT.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
