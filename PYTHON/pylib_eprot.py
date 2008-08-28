#!/usr/bin/env python2.4

"""
pylib_eprot 0.1

Library supporting E test runs and protocol files.

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
import pylib_generic
import pylib_io



class eresult(object):
    """
    Class representing a single result (i.e. line in an E protocol).
    """
    def __init__(self, line):
        assert line[0]!="#"
        self.values = line.split()
        assert len(self.values) >= 4

    def name(self):
        return self.values[0]

    def status(self):
        return self.values[1]

    def cputime(self):
        return self.values[2]

    def reason(self):
        return self.values[3]

    def __str__(self):
        fixed = "%-29s %s %8s %-10s"%(self.values[0],self.values[1],\
                                      self.values[2],self.values[3])
        reslist = [fixed]
        for i in self.values[4:]:
            reslist.append("%10s"%(i,))
        return " ".join(reslist)

class eprot(object):
    """
    Class representing an E protocol.
    """
    def __init__(self, name):
        self.name     = name
        self.comments = ""
        self.results  = {}
        


if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    if len(args) > 2:
        print __doc__
        sys.exit()

    # Minimal unit test
    res = eresult("ALG046+1.p                    T    4.942  success          7926       2540      31837       5480        682")
    print res

