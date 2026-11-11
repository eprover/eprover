#------------------------------------------------------------------------
#
# File  : Makefile for PCL2
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds the PCL2 library, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) PCL2/PCL2.a

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	rm -f *.o *.a
