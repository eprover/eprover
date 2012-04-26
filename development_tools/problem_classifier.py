#!/usr/bin/env python2.7
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
import re

import pylib_io
import pylib_eprots
from pylib_discretize import *

para_re  = re.compile("[0-9]+(\.[0-9]+)?")

files   = pylib_io.get_args()
options = pylib_io.get_options()
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
     equidist: Times are rounded to the nearest k seconds, each
               problem is assigned the globally best heuristic that
               solves it in minimal time.
     prop    : Times are rounded to the nearest [1,2,5]*10^X, each
               problem is assigned the globally best heuristic that
               solves it in minimal time.
     log     : Times are rounded to the nearest k*2^X, each
               problem is assigned the globally best heuristic that
               solves it in minimal time.
     none    : Each problem is assigned the globally best heuristic
               that solves it in minimal time.

equidist and log accept an numeric argument for k, e.g. -rlog0.5 or
-requidist10.

Suggestions are preceded by an mnemonic string denoting the algorithm
and a colon, so you can grep and cut suitable results out. Parsing is
expensive, computing assignments is cheap, so it makes sense to
compute all assignments at once. Other output fields are problem name,
status, and suggested class.

If no -r option is given, uses prop algorithm, but ommits mnemonic
strings.
"""
        sys.exit(1)

def get_param(option, default):
    mr = para_re.search(option)
    if mr:
        tmp = mr.group()
        try:
            res = int(tmp)
        except ValueError:
            res = float(tmp)
        return res
    return default
     
if len(formats) == 0:
    res = protlist.make_classification(prop_round)
    res.printout()
else:
    for o in formats:
        if o.startswith("-rconst"):
            res = protlist.make_classification(const_round)
            res.printout("const :")
        elif o.startswith("-rprop"):
            res = protlist.make_classification(prop_round)
            res.printout("prop  : ")
        elif o.startswith("-rlog"):
            res = protlist.make_classification(log_round(get_param(o, 0.5)))
            res.printout("log%02d :" % get_param(o, 5))
        elif o.startswith("-requidist"):
            res = protlist.make_classification(equidist_round(get_param(o, 5)))
            res.printout("equi%02d:" % get_param(o, 5))
        elif o.startswith("-rnone"):
            res = protlist.make_classification(no_round)
            res.printout("none  :")
        else:
            raise pylib_io.UsageErrorException
        
        
        

