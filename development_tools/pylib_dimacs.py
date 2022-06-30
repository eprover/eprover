#!/usr/bin/env python2.7
# ----------------------------------
#
# Module pylib_dimacs.py
#
# Functions (and classes) for manipulating propositional formulae in
# DIMACS format. Currently this supports conversion from DIMACS to
# MathSAT.

"""
pylib_dimacs.py 1.0

Usage: pylib_dimacs [options] <file> ...

Read a list of DIMACS CNF files and convert them to MathSAT format.

Restrictions: Does not currently use a real DIMACS parser, but relies
on the fact that DIMACS formulas write one clause per line.

Options:

 -h
  Print this information and exit.

 -o<outfile>
  Use the named file as output (default is stdout). Not compatible
  with -b. 

 -b<app>
  Batch operation: For each input file, generate an separate output
  file, named by appending '.<app>' to the original file name.

 -f
--fix-broken-dimacs
  Many DIMACS syntax input files incorrectly use the empty clause (a
  line containing just '0') as an end-of-file marker. Use this option
  to ignore the empty clause in the input.

--tptp2
--oldtptp
  Convert to classic TPTP CNF format.

--mathsat
  Convert to MathSAT-3 format.

--broken-dimacs
  Print output in DIMACS syntax with a trailing empty clause (this is
  required by some braindead solvers).

Copyright 2005 Stephan Schulz, schulz@eprover.org

This code is part of the support structure for the equational
theorem prover E. Visit

 http://www.eprover.org

for more information.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program ; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston,
MA  02111-1307 USA 

The original copyright holder can be contacted as

Stephan Schulz (I4)
Technische Universitaet Muenchen
Institut fuer Informatik
Boltzmannstrasse 3
Garching bei Muenchen
Germany

or via email (address above).
"""


import re
import string
import sys

import pylib_basics
import pylib_io



def dimacs_to_mathsat_lit(lit):
    """
    Map a DIMACS style literal to a MathSAT literal.
    """
    if lit < 0:
        return "!p" + str(abs(lit))
    return "p"+str(lit)

def dimacs_to_tptp_lit(lit):
    """
    Map a DIMACS style literal to a TPTP literal.
    """
    if lit < 0:
        return "--p" + str(abs(lit))
    return "++p"+str(lit)



class prop_clause(object):
    """
    A propositional clause (as a list of propositional literals).
    """

    clause_counter = 0

    def __init__(self, str = None):
        """
        Default is empty clause...
        """

        prop_clause.clause_counter = prop_clause.clause_counter+1
        self.id = prop_clause.clause_counter+1
        self.literals = []
        if str:
           self.parse_from_string(str)

    def literal_count(self):
        return len(self.literals)        

    def set_literals(self, l):
        self.literals = l

    def get_literals(self):
        return self.literals[:]

    def get_atoms(self):
        return pylib_basics.uniq_unsorted(map(abs, self.literals))

    def parse_from_string(self, str):
        l = re.split('\s+', string.strip(str))
        self.set_literals(map(int,l[:-1]))

    def __str__(self):
        l=map(str, self.literals)
        l.append("0\n")
        return " ".join(l)

    def mathsat_str(self):
        l = map(dimacs_to_mathsat_lit, self.literals)
        return "("+"|".join(l)+")"

    def tptp2_str(self):
        l = map(dimacs_to_tptp_lit, self.literals)
        return "input_clause(c"+str(self.id)+",axiom,["+",".join(l)+"])."
 
        

class prop_formula(object):
    def __init__(self):
        self.clauses = []

    def add_clause(self, clause):
        self.clauses.append(clause)

    def clause_count(self):
        return len(self.clauses)
    
    def get_atoms(self):
        res = []
        for i in self.clauses:
            res.extend(i.get_atoms())
        return res

    def get_max_atom(self):
        a = self.get_atoms()
        if len(a)==0:
           return 0
        return max(a)

    def dimacs_parse(self, file):
        fp = pylib_io.flexopen(file,"r")
        l = fp.readlines()
        pylib_io.flexclose(fp)
        l = [i for i in l if not (i.startswith("%") or
                        i.startswith("#") or i.startswith("c") or
                        i == "\n")]
        if l[-1].startswith("0"):
            if fix_broken_dimacs:
                del(l[-1])
            else:
                sys.stderr.write("Warning: Problem ends in empty "+
                                 "clause. Use --fix-broken-dimacs to suppress it")
        for i in l[1:]:
            self.add_clause(prop_clause(i))

    def __str__(self):
        res = []
        res.append("p cnf %d %d\n" % (self.get_max_atom(),
                                    self.clause_count()))
        res.extend(map(str, self.clauses))
        if format == "broken_dimacs":
            res.append("0\n")
        return "".join(res);

    def mathsat_str(self):
        atoms = map(dimacs_to_mathsat_lit, self.get_atoms())
        head = "VAR\n\n"+", ".join(atoms)+" : BOOLEAN\n\nFORMULA\n\n"
        l=[i.mathsat_str() for i in self.clauses]
        form = "&\n".join(l)

        return head + form

    def tptp2_str(self):
        l=[i.tptp2_str() for i in self.clauses]
        return "\n".join(l)

if __name__ == '__main__':
    format = "dimacs"
    outfile = None
    batch_app = None
    fix_broken_dimacs = False
    
    for option in pylib_io.get_options():
        if option == "-h":
            print __doc__
            sys.exit()
        elif option == "-f" or option == "--fix-broken-dimacs":
            fix_broken_dimacs = True;            
        elif option == "--broken-dimacs":
            format = "broken_dimacs"
        elif option == "--tptp2" or option == "--oldtptp":
            format = "tptp2"
        elif option == "--mathsat":
            format = "mathsat"
        elif option.startswith("-o"):
            outfile = option[2:]
            if outfile == "":
                sys.exit("-o needs non-empty argument")
        elif option.startswith("-b"):
            batch_app =  option[2:]
            if batch_app == "":
                sys.exit("-b needs non-empty argument")
        else:
            sys.exit("Unknown option "+ option)

    if outfile and batch_app:
        sys.exit("Options -o and -b are incompatible")
    
    if not outfile:
        outfile = "-"
    if not batch_app:
        out_fp = pylib_io.flexopen(outfile, "w")
    
    files = pylib_io.get_args()
    if len(files)==0:
        files.append("-")

    for file in files:
        formula = prop_formula()
        formula.dimacs_parse(file)

        if batch_app:
            out_fp = pylib_io.flexopen(file+"."+batch_app, "w")

        if format=="mathsat":
            out_fp.write(formula.mathsat_str())
            out_fp.write("\n")
        elif format=="tptp2":
            out_fp.write(formula.tptp2_str())
            out_fp.write("\n")
        else:
            out_fp.write(str(formula))

        if batch_app:
            pylib_io.flexclose(out_fp)

    if not batch_app:
        pylib_io.flexclose(out_fp)
