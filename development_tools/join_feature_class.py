#!/usr/bin/env python2.7
# ----------------------------------
#
# Usage: join_feature_class.py <class-file> <feature-file>
#
# Read class assignment file and feature file and return a file of
# problem names with features and class.
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
import string

import pylib_io
import pylib_eprots

pylib_io.check_argc(2)

cl = pylib_eprots.classification()
cl.parse(sys.argv[1])

fl = pylib_eprots.featurelist()
fl.parse(sys.argv[2])

for i in fl:
    try:
        tclass = cl.classify(i[0])
        print i[0],":", string.join(i[1],","),":",tclass
    except KeyError:
        pass
