#!/usr/bin/env python

"""
e_ltb_runner 0.1

Usage: e_ltb_runner.py <jobfile> [<timelimit> [<proverpath>]]

Try to prove as many problems as possible from the jobs in jobfile.

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
import os
import string
import getopt

e_template = "%s -s -xAuto -tAuto --memory-limit=768 --tptp3-format --print-statistics --resources-info --cpu-limit=%d %s"


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

    
def filejob_parser(filename):
    res = []
    fp = open(filename, "r")
    joblines = fp.readlines()
    fp.close()

    for i in joblines:
        try:
            infile, outfile = i.split()
            res.append(filejob(infile, outfile))
        except:
            print "Warning: Bogus line: ", i

    return res

def run_e(prover, timelimit, problem):
    cmd = e_template % (prover, timelimit, problem.infile)
    res = run_shell_command(cmd)

    status = "unknown"
    for l in res:
        if l.startswith("# SZS status"):
            status = l[len("# SZS status "):-1]
            break

    if status in ["Theorem", "Unsatisfiable", \
                  "CounterSatisfiable", "Satisfiable"]:
        fp = open(problem.outfile, "w")
        for i in res:
            fp.write(i)
        fp.close()
        return True
    return False
 

if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    timelimit = 300
    prover    = "eprover"

    if len(args) < 1 or len(args) >3:
        print __doc__
        sys.exit()
        
    jobfile = args[0]
    if len(args)> 1:
        timelimit = int(args[1])
    if len(args)>2:
        prover = args[2]

    jobs = filejob_parser(jobfile)
    worklist = list(jobs)

    time = 5
    iter = 0
    while len(worklist):
        print "Pass", iter, "with", time, "seconds per each of",len(worklist),"problems"
        solved = []
        for i in worklist:
            if run_e(prover, time, i):
                print "Solved:", i
                solved.append(i)
            else:
                print "Failed on:", i
        for i in solved:
            worklist.remove(i)
        
        time = time*2
        iter = iter+1
