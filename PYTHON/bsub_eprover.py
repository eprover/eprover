#!/usr/bin/env python2.6

"""
bsub_eprover 0.1

Usage: bsub_eprover.py <stratfile> ...

Parse the strategy description files provided, generate corresponding
LSF batch files, and submit them to the LSF system for running. This
is, more or less, a one-off hack and not particularly configurable
except by editing the source code.

Options:

-h
   Print this help.

-v
--verbose
   Switch on verbose mode.

Copyright 2010 Stephan Schulz, schulz@eprover.org

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

bsub_name="cat"
"""
Name of the batch submission program (normally bsub, but we use cat
for testing).
"""


res

bsub_tmplt = """
#!/bin/bash
#BSUB -J erun=%s=%s
#BSUB -o %%J.out   
#BSUB -e %%J.err  
#BSUB -W 0:10   
#BSUB -q small 
#BSUB -n 1      
#
# Run serial executable on 1 cpu of one node
cd %s
eprover -s --print-statistics --resources-info %s %s
"""
"""
Template for generating bsub jobs.
"""


def bsub_submit(job):
    """
    Submit a job to bsub.
    """
    fp = os.popen(bsub_name, "w")
    fp.write(job)
    fp.close()


def bsub_gen(stratname, probname, args):
    """
    Generate a bsub specification.
    """
    return bsub_tmplt%\
           (stratname, probname, args, probname)




def process_job(name):
    """
    Determine the problem base name, parse any results (individually
    and collected), determine the missing problems, and submit them
    via bsub.
    """
    # Collect results
    # Determine base name
    # find missing problems
    # generate and submit problems
    



if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        if option == "-v":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    etestset

    for name in args:
        process_job(name)
