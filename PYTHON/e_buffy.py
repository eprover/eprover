#!/usr/bin/env python

"""
e_buffy.py 0.1

Usage: e_buffy.py <problem> 

Classify a problem, select a schedule, and run E on the problem using
that schedule.

Options:

 -h
  Print this help.
  
 -f<factor>
--scale=<factor>
  Speed difference beween training machines and current machines.
  
 -t
--cpu-limit=<time>

  Total CPU time limit for the task.
   
 
    
Copyright 2009 Stephan Schulz, schulz@eprover.org

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

e_template = "%s %s --memory-limit=Auto --tptp3-format --print-statistics --resources-info --cpu-limit=%d %s"
e_cl_template = "%s --tptp3-format --cpu-limit=


def run_shell_command(cmd):
    """
    Run a local shell command and return the output (or None if an
    error occured).
    """
    fp = os.popen(cmd)
    res = fp.readlines()
    status = fp.close()

    return res


class filejob:
    def __init__(self, infile, outfile):
        self.infile  = infile
        self.outfile = outfile

    def __str__(self):
        return "<filejob: %s -> %s>"%(self.infile, self.outfile)

  
def run_e_classify(prover, options, timelimit, problem):
    """
    Run E as a classifier and return the result of the
    classification.
    """
    cmd = e_template % (prover, options, timelimit, problem.infile)
    res = run_shell_command(cmd)

    status = "unknown"
    for l in res:
        if l.startswith("# SZS status"):
            status = l[len("# SZS status "):-1]
            break

    if status in ["Theorem", "Unsatisfiable", \
                  "CounterSatisfiable", "Satisfiable"]:
        return True, status
    return False, status 
 

  
def run_e(prover, options, timelimit, problem):
    """
    Run E with the given options and time limit on the problem and
    report the result.
    """
    cmd = e_template % (prover, options, timelimit, problem.infile)
    res = run_shell_command(cmd)

    status = "unknown"
    for l in res:
        if l.startswith("# SZS status"):
            status = l[len("# SZS status "):-1]
            break

    if status in ["Theorem", "Unsatisfiable", \
                  "CounterSatisfiable", "Satisfiable"]:
        return True, status
    return False, status 
 

if __name__ == '__main__':

    scale     = 1.0
    cpu_limit = 300.0
    prover    = "/Users/schulz/SOURCES/Projects/E/PROVER/eprover"
    
    opts, args = getopt.gnu_getopt(sys.argv[1:], "hf:t:",
                                   ["scale=", "cpu-limit="])

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        elif option=="-f" or option=="--scale":
            scale=float(opt)
        elif option=="-t" or option=="--cpu-limit":
            cpu_limit=float(opt)
        else:
            sys.exit("Unknown option "+ option)

    if len(args) != 1:
        print __doc__
        sys.exit()

    

