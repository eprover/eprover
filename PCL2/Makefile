#------------------------------------------------------------------------
#
# File  : Makefile for the PCL2 stuff
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Wed Mar 22 19:35:48 MET 2000
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = PCL2
LIB     = $(PROJECT).a

all: $(LIB)

depend: *.c *.h
	$(MAKEDEPEND)

# Remove all automatically generated files

clean:
	@touch does_exist.o does_exist.a; rm *.o  *.a

# Services (provided by the master Makefile)

include ../Makefile.services

PCL2_LIB = pcl_idents.o pcl_positions.o pcl_expressions.o pcl_steps.o \
	   pcl_protocol.o pcl_miniclauses.o pcl_ministeps.o pcl_miniprotocol.o\
           pcl_lemmas.o pcl_analysis.o pcl_propanalysis.o pcl_proofcheck.o

$(LIB): $(PCL2_LIB)
	$(AR) $(LIB) $(PCL2_LIB)

include Makefile.dependencies

