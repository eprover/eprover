#!/usr/bin/env python

"""
prunner 0.1

Usage: prunner.py <jobfile>

Run a list of jobs as fast as possible utilizing multiple cores,
collecting their output and writing it to stdout once the job is
complete.

Options:

-h Print this help.

-c <cores>
   Set the number of cores to utilize in parallel.

Copyright 2011 Schulz, schulz@eprover.org

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
import subprocess
import select
import string
import getopt
import pylib_io



class runner(object):
    """
    Class representing a running job.
    """
    def __init__(self, cmd):
        self.proc = subprocess.Popen(cmd,
                                     stdin=None,
                                     stdout=subprocess.PIPE,
                                     close_fds=True,
                                     shell=True)
        self.cmd    = cmd
        self.res    = ""

    def fileno(self):
        """
        Return the pipe's fileno to support select.
        """
        return self.proc.stdout.fileno()

    def poll_res(self):
        """
        """
        tmp = self.proc.stdout.read()
        if tmp =="":
            return self.res

        self.res = self.res+tmp
        return None

    def __repr__(self):
        return "<running:"+self.cmd.strip()+">"


class prunner(object):
    """
    Class handling a set of jobs.
    """
    def __init__(self, jobs, cpu):
        """
        Initialize object. jobs is a list of command strings to run.
        """
        self.jobs    = jobs
        self.running = []
        self.cpu     = cpu

    def run(self, cores):
        processed = 0
        while self.jobs or self.running:
            while len(self.running) < cores and self.jobs:
                newjob = self.jobs.pop(0)
                print "Starting "+newjob
                job = runner(newjob)
                self.running.append(job)
            print "Running:", self.running
            rd,wr,ev = select.select(self.running, [], [], 1)
            print "Output ready for ", rd
            for i in rd:
                print "Checking ", i
                tmp = i.poll_res()
                if tmp!=None:
                    print tmp
                    print self.cpu
                    self.running.remove(i)
                    processed = processed+1
        return processed





if __name__ == '__main__':
    cores  = 2
    getcpu = False
    opts, args = getopt.gnu_getopt(sys.argv[1:], "hCc:")
    cpu = "Generic CPU"

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        elif option == "-c":
            cores = int(optarg)
        elif option == "-C":
            getcpu = True
        else:
            sys.exit("Unknown option "+ option)

    if getcpu:
        cpu = pylib_io.run_shell_command(
            'cat /proc/cpuinfo | grep "cpu MHz\|bogomips"|head -2')
        cpu = "".join(["# "+i for i in cpu])

    jobs = sys.stdin.readlines()

    control = prunner(jobs, cpu)

    control.run(cores)
