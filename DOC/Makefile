#------------------------------------------------------------------------
#
# File  : Makefile for the DOC directory
#
# Builds the E manual (eprover.pdf) with pdflatex.
#
# Author: Stephan Schulz
#
#------------------------------------------------------------------------

include ../Makefile.vars

all: eprover.pdf

clean:
	rm -f *.aux *.log *.dvi *.ps

eprover.pdf: eprover.tex estyle.sty
	$(PDFLATEX) eprover.tex
	$(PDFLATEX) eprover.tex
