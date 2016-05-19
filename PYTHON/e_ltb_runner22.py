#!/usr/bin/env python

"""
e_ltb_runner22 0.1

Usage: e_ltb_runner22.py <jobfile> [<timelimit> [<proverpath>]]

Try to prove as many problems as possible from the jobs in jobfile,
using the competition setup of CASC-22. This tries three strategies on
each problem (time permitting) - Auto-Mode with relevance pruning
levels 2, 3, and unlimited.

Options:

-h Print this help.

Copyright 2008,2009 Stephan Schulz, schulz@eprover.org

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
    res = []
    fp = open(filename, "r")
    joblines = fp.readlines()
    fp.close()
    problem_mode = False

    for i in joblines:
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
    if not timelimit:
        timelimit = 240*len(worklist)

    # We need to go into the first problem, anyways.
    estimated_preproc_time = -10.0
    limit = timelimit/(len(worklist)*2)
    
    while worklist:
        problem = worklist.pop(0)

        print "\n% SZS status Started for", problem.infile
        sys.stdout.flush()
        joblist = [ "--rel-pruning-level=2", "--rel-pruning-level=3", ""]
        
        while joblist:        
            remaining_time = timelimit - get_cpu_time()
            jobno = (3.0 * len(worklist))+len(joblist)
            time_per_problem = remaining_time/jobno
            status = None

            job = joblist.pop(0)
            if time_per_problem > estimated_preproc_time+5.0:
                if estimated_preproc_time < 0:
                    limit = limit*2
                else:
                    limit = time_per_problem
                status, cpu_time, preproc = run_e(prover, job, limit, problem)
                
                if preproc and preproc > estimated_preproc_time:
                    estimated_preproc_time = preproc
                if status:
                    print "% SZS status", status, "for", problem.infile
                    sys.stdout.flush()
                    break
                
        print "% SZS status Ended for", problem.infile
        sys.stdout.flush()       
            
            
