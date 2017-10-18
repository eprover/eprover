#!/usr/bin/env python

"""
canonizer.py 0.1

Usage: canonizer.py <file1> ... 

Read a TPTP format file and print it back in canonical form (no
unecessary white space, no comments. This is just a hack!

Options:

-h Print this help.

Copyright 2008 Stephan Schulz, schulz@eprover.org

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
import string
import getopt
import pylib_io
import pylib_generic


ws = re.compile("\s")
br = re.compile("\(\)")


def canonize(fp):
    res = []
    for line in fp:
        if line.startswith("%"):
            pass
        else:
            res.append(line)
    raw = "".join(res)
    clean = ws.sub("", raw)
    clean = br.sub("", clean)
    res = re.sub("\.", ".\n", clean)

    return res

if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    filenames = args

    for i in filenames:
        fp = open(i, "r")
        print canonize(fp)
        
