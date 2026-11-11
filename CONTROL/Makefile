#------------------------------------------------------------------------
#
# File  : Makefile for CONTROL
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the CONTROL library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) CONTROL/CONTROL.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
