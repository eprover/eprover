#------------------------------------------------------------------------
#
# File  : Makefile for EXTERNAL
#
# E is built by the top level Makefile. This only forwards to it:
# "make" here builds CSSCPA_filter, any other target is
# passed on unchanged.
#
#------------------------------------------------------------------------

all:
	cd .. && $(MAKE) EXTERNAL/CSSCPA_filter

.DEFAULT:
	cd .. && $(MAKE) $@

clean:
	cd .. && $(MAKE) clean_extras
