#!/usr/bin/env python2.2
# ----------------------------------
"""
Usage: dectreelearn.py <examplefile>

Generate a decision tree for classifying the examples in example
file. 

Copyright 2003 Stephan Schulz, schulz@informatik.tu-muenchen.de

his code is part of the support structure for the equational
heorem prover E. Visit

 http://www4.informatik.tu-muenchen.de/~schulz/WORK/eprover.html 

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


import sys
import string

import pylib_io
import pylib_basics
import pylib_ml_examples
import pylib_probabilities
import pylib_dectrees

relgain_limit = 0.5
entropy_compare_fun = cmp

options = pylib_io.get_options()
for o in options:
    if o[0:2] == "-g":
        relgain_limit = float(o[2:])
    if o == "-r":
        entropy_compare_fun = pylib_basics.rl_lex_compare
    
args    =  pylib_io.get_args()

pylib_io.check_argc(1,args)

set = pylib_ml_examples.ml_exampleset()
set.parse(args[0])
print set.plain_rel_inf_gain()

tree = pylib_dectrees.decision_tree(set,
                                    entropy_compare_fun,
                                    relgain_limit)
tree.printout()

if len(args) == 1:
    (succ, count) = tree.classify_set(set)
    print "Successes: %d out of %d, %f%%"% (succ, count, float(succ)/count*100)
else:
    testset = pylib_ml_examples.ml_exampleset()
    testset.parse(args[1])
    (succ, count) = tree.classify_set(testset)
    print "Successes: %d out of %d, %f%%"% (succ, count, float(succ)/count*100)

#tree.printout()

#tmp = pylib_ml_examples.find_best_partition(set, 10)

#print "Best partion: %1.6f, %1.6f "\
#              %(tmp[0], tmp[1]), tmp[2].abstracter
