#------------------------------------------------------------------------
#
# File  : Makefile for BASICS
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the BASICS library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) BASICS/BASICS.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
