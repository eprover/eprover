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
LIB     = clib.ps eprover.ps eprover.pdf

all: $(LIB)

clean:
	@touch does_exist.aux does_exist.log does_exist.dvi does_exist.ps;
	@rm *.aux *.log *.dvi *.ps;

depend:
	echo "No dependencies for LaTeX documentation"

# Services (provided by the master Makefile)

include ../Makefile.services

DOC_LIB = *.tex

$(LIB): $(DOC_LIB)
	$(LATEX) clib.tex
	$(LATEX) clib.tex
	$(LATEX) clib.tex
	$(DVIPS) -o clib.ps clib.dvi
	$(LATEX) eprover.tex
#        $(BIBTEX) eprover
	$(LATEX) eprover.tex
	$(MAKEINDEX) eprover
	$(LATEX) eprover.tex
	$(LATEX) eprover.tex
	$(PDFLATEX) eprover.tex
	$(DVIPS) -o eprover.ps eprover.dvi
