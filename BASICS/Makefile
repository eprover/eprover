#------------------------------------------------------------------------
#
# File  : Makefile for BASICS.a library of generic data types and
# algorithms.
#
# Author: Stephan Schulz
#
# Created: Sun Jul  6 22:55:11 MET DST 1997
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = BASICS
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@rm -f *.o  *.a

# Services (provided by the master Makefile)

include ../Makefile.services

# Build the  library

BASIC_LIB = clb_error.o clb_memory.o clb_os_wrapper.o \
            clb_dstrings.o clb_verbose.o\
            clb_stringtrees.o clb_numtrees.o clb_numxtrees.o \
            clb_floattrees.o clb_pstacks.o\
            clb_pqueue.o clb_dstacks.o clb_ptrees.o clb_quadtrees.o\
            clb_regmem.o clb_permastrings.o \
	    clb_objtrees.o clb_objmaps.o clb_fixdarrays.o\
            clb_plist.o clb_pdarrays.o clb_pdrangearrays.o \
            clb_ddarrays.o clb_sysdate.o \
            clb_intmap.o \
            clb_simple_stuff.o clb_partial_orderings.o \
            clb_plocalstacks.o clb_min_heap.o

MIN_HEAP = clb_min_heap.o

min_heap: $(MIN_HEAP)
	$(LD) -o min_heap  $(BASIC_LIB) -lm

$(LIB): $(BASIC_LIB)
	$(AR) $(LIB) $(BASIC_LIB)

include Makefile.dependencies
