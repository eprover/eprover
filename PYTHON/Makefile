#------------------------------------------------------------------------
#
# File  : Makefile for the PYTHON directory
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Tue Jan 13 01:16:22 MET 1998
#     New
#
#------------------------------------------------------------------------

.PHONY: tags all tools

include ../Makefile.vars

# Project specific variables

all: $(LIB)

# Services (provided by the master Makefile)

include ../Makefile.services

ptags:
	etags *.py

tools:
	@echo "Configuring tools"
	../development_tools/configure_tools
