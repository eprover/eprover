#------------------------------------------------------------------------
#
# File  : Makefile for LEARN
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the LEARN library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) LEARN/LEARN.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
