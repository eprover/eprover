#!/usr/bin/env python2.2
# ----------------------------------
#
# Usage: problem_classifyer.py <prot1> ... <protn>
#
# Read E protocols and return a suggested class for each proof problem
# found.
#
# Copyright 2003 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# This code is part of the support structure for the equational
# theorem prover E. Visit
#
#  http://www4.informatik.tu-muenchen.de/~schulz/WORK/eprover.html 
#
# for more information.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program ; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA  02111-1307 USA 
#
# The original copyright holder can be contacted as
#
# Stephan Schulz (I4)
# Technische Universitaet Muenchen
# Institut fuer Informatik
# Boltzmannstrasse 3
# Garching bei Muenchen
# Germany
#
# or via email (address above).

import sys
import pylib_io
import pylib_eprots
from pylib_discretize import *

files   = pylib_io.get_args(sys.argv)
options = pylib_io.get_options(sys.argv)
formats = filter(lambda x:x[0:2]=="-r", options)
stuff   = filter(lambda x:x[0:2]!="-r", options)

protlist = pylib_eprots.eprot_set(files);

for i in stuff:
    if i=="-h"or i=="--help":
        print """
problem_classifier 0.1

Usage: problem_classifier.py [-r<format1> ...-r<formatn>]  <protocols>

Read a set of E protocols and suggest various assignments of protocols
(i.e. heuristics) to proof problems. Only problems occurring in the
best overal protocol are handled.

Options:

-r<format>: Select an algorithms for computing the
            assignment.
     Available:

     const   : Each problem is assigned the globally best heuristic
               that solves it.
     equidist: Times are rounded to the nearest 10 second, each
               problem is assigned the globally best heuristic that
               solves it in minimal time.
     prop    : Times are rounded to the nearest [1,2,5]*10^X, each
               problem is assigned the globally best heuristic that
               solves it in minimal time.
     none    : Each problem is assigned the globally best heuristic
               that solves it in minimal time.

Suggestions are preceded by an mnemonic string denoting the algorithm
and a colon, so you can grep and cut suitable results out. Parsing is
expensive, computing assignments is cheap, so it makes sense to
compute all assignments at once.

If no -r option is given, uses prop algorithm, but ommits mnemonic
strings.
"""
        sys.exit(1)
        
if len(formats) == 0:
    res = protlist.make_classification(prop_round)
    res.printout()
else:
    for o in formats:
        if o == "-rconst":
            res = protlist.make_classification(const_round)
            res.printout("const: ")
        elif  o == "-rprop":
            res = protlist.make_classification(prop_round)
            res.printout("prop : ")
        elif  o == "-requidist":
            res = protlist.make_classification(equidist_round(10))
            res.printout("equi : ")
        elif o == "-rnone":
            res = protlist.make_classification(no_round)
            res.printout("none : ")
        else:
            raise pylib_io.UsageErrorException
        
        
        

