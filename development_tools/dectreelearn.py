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
import pylib_ml_examples
import pylib_probabilities

pylib_io.check_argc(1)

set = pylib_ml_examples.ml_exampleset()
set.parse(sys.argv[1])

part = pylib_ml_examples.class_partion(set)
print part.entropy()

print pylib_probabilities.compute_entropy([1,1,1,1,4])
