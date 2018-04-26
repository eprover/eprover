#------------------------------------------------------------------------
#
# File  : Makefile for the DOC directory
#
# Author: Stephan Schulz
#
# Changes
#
# <1> Tue Jan 13 01:16:22 MET 1998
#     New
#
#------------------------------------------------------------------------

include ../Makefile.vars

# Project specific variables

PROJECT = DOC
LIB     = eprover.pdf

all: $(LIB)

clean:
	@rm -f *.aux *.log *.dvi *.ps;

depend:
	echo "No dependencies for LaTeX documentation"

# Services (provided by the master Makefile)

include ../Makefile.services

DOC_LIB = *.tex

$(LIB): $(DOC_LIB)
	$(PDFLATEX) eprover.tex
	#$(BIBTEX) eprover
	$(PDFLATEX) eprover.tex
