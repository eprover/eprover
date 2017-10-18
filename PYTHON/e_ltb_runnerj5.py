#!/usr/bin/env python

"""
e_ltb_runnerj5 0.1

Usage: e_ltb_runnerj5.py <jobfile> [<proverpath>]

Try to prove as many problems as possible from the jobs in jobfile,
using the competition setup of CASC-J5. This tries one strategy on
each problem (time permitting).

Options:

-h Print this help.

Copyright 2010 Schulz, schulz@eprover.org

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
import resource

e_template = "%s -s %s -xAuto -tAuto --delete-bad-limit=2147483647 --tptp3-format --print-statistics --resources-info --cpu-limit=%d %s"


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
    global timelimit
    
    res = []
    fp = open(filename, "r")
    joblines = fp.readlines()
    fp.close()
    problem_mode = False

    for i in joblines:
        if i.startswith("limit.time.problem.wc"):
            timelimit = int(i[len("limit.time.problem.wc "):])
        
        if i.startswith("% SZS start BatchProblems"):
            problem_mode = True
        elif i.startswith("% SZS end BatchProblems"):
            problem_mode = False
        elif problem_mode == True:            
            try:
                infile, outfile = i.split()
                res.append(filejob(infile, outfile))
            except:
                print "Warning: Bogus line: ", i
        else:
            pass
    return res

def run_e(prover, filter, timelimit, problem):
    cmd = e_template % (prover, filter, timelimit, problem.infile)
    res = run_shell_command(cmd)

    cpu_time = timelimit
    preproc_estimate = None
    status = "unknown"
    for l in res:
        if l.startswith("# SZS status"):
            status = l[len("# SZS status "):-1]
        elif l.startswith("# Total time               :"):
            tmp = l.split()
            cpu_time = float(tmp[4])
        elif l.startswith("# Preprocessing time       :"):
            tmp = l.split()
            preproc_estimate = float(tmp[4])

    if status in ["Theorem", "Unsatisfiable", \
                  "CounterSatisfiable", "Satisfiable"]:
        fp = open(problem.outfile, "w")
        for i in res:
            fp.write(i)
        fp.close()
        return (status, cpu_time, preproc_estimate)
    return (False, cpu_time, preproc_estimate)

def get_cpu_time():
    self_usage  = resource.getrusage(resource.RUSAGE_SELF)
    child_usage = resource.getrusage(resource.RUSAGE_CHILDREN)
    return self_usage.ru_utime+self_usage.ru_stime+\
           child_usage.ru_utime+child_usage.ru_stime
 

if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    timelimit = None
    prover    = "eprover"

    if len(args) < 1 or len(args) >2:
        print __doc__
        sys.exit()
        
    jobfile = args[0]
    if len(args)>1:
        prover = args[1]

    jobs = filejob_parser(jobfile)
    worklist = list(jobs)
    if not timelimit:
        timelimit = 30

    # We need to go into the first problem, anyways.
    
    while worklist:
        problem = worklist.pop(0)

        print "\n% SZS status Started for", problem.infile
        sys.stdout.flush()
        joblist = [""]
        
        while joblist:        
            status = None

            job = joblist.pop(0)
            status, cpu_time, preproc = run_e(prover, job, timelimit, problem)
            
            if status:
                print "% SZS status", status, "for", problem.infile
                sys.stdout.flush()
                break
                
        print "% SZS status Ended for", problem.infile
        sys.stdout.flush()       
            
            
