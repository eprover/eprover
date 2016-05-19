#!/usr/bin/env python

"""
appendix_gen.py 0.1

Usage: appendix_gen.py <prot1> ... 

Read E protocol or TPTP files and print a correlation between literal
selection strategy and end of the file name.

Options:

-h Print this help.

Copyright 2014 Stephan Schulz, schulz@eprover.org

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

Stephan Schulz
DHBW Stuttgart
Fakultaet Technik
Informatik
Rotebuehlplatz 41
70178 Stuttgart
Germany

or via email (address above).
"""

import sys
import re
import string
import getopt

name_app_re =     re.compile("_S?..?P?$")
litsel_strat_re = re.compile("-W\w*")


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
        mo = name_app_re.search(i)
        if mo:
            appendix = mo.group()[1:]                                  
            fp = open(i, "r")
            contents = fp.read(4096)
            mo = litsel_strat_re.search(contents)
            if  mo:
                litsel = mo.group()[2:]
            else:
                litsel = "NoSelection"

            print "%-4s %-40s"%(appendix, litsel)

        else:
            print "No valid appendix", i
        
        
