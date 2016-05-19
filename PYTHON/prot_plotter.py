#!/usr/bin/env python

"""
prot_plotter 0.1

Usage: prot_plotter.py [-s <step>] <prot_1> ... <prot_n>

Read a list of protocols and convert them into a format for producing
standard performance plots (solutions over time) with gnuplot. The
result is a list with lines of the format
<time> <solns_1> ... <solns_n>
where each <solns_i> value is the number of solutions up to the time
limit according to the data in <prot_i>

Options:

-h Print this help.

-s Time step resolution. Default is 1.

-l Maximum time limit. Default is 300.

Copyright 2014 Schulz, schulz@eprover.org

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

import sys
import re
import os
import string
import getopt
import pylib_io
import pylib_eprot




if __name__ == '__main__':
    step  = 1.0
    limit = 300.0
    opts, args = getopt.gnu_getopt(sys.argv[1:], "hs:l:")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        elif option == "-s":
            step = float(optarg)
        elif option == "-l":
            limit = float(optarg)
        else:
            sys.exit("Unknown option "+ option)

    res = []
    strats = []

    for prot in args:
        p = pylib_eprot.eprot(prot)        
        p.parse(os.path.dirname(prot))
        res.append(p.get_performance_plot(step, limit))

    print "# Column 1: Time step"
    count = 2
    for prot in args:
        print "# Column %d: %s"%(count, prot)
        count = count+1
    for i in xrange(len(res[0])):
        print "%6.2f"%(res[0][i][0],),
        for j in xrange(len(res)):
            print "%6d"%(res[j][i][1]),
        print
        
