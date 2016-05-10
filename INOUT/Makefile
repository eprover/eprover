#------------------------------------------------------------------------
#
# File  : Makefile for io_lib of the CLIB library
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Sun Jul  6 22:55:11 MET DST 1997
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = INOUT
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.a; rm *.o  *.a

# Services (provided by the master Makefile)

include ../Makefile.services

IO_LIB = cio_output.o cio_simplestuff.o cio_fileops.o \
	 cio_streams.o  cio_initio.o \
         cio_scanner.o cio_network.o cio_multiplexer.o \
         cio_basicparser.o cio_commandline.o \
         cio_filevars.o cio_tempfile.o cio_signals.o

$(LIB): $(IO_LIB)
	$(AR) $(LIB) $(IO_LIB)

include Makefile.dependencies

