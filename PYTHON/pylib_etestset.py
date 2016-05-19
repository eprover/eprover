#!/usr/bin/env python

"""
pylib_etestset 0.1

Library supporting E test runs.

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
import string
import getopt
import os.path
import pylib_generic
import pylib_io
import pylib_eprot


def strat_str(strat):
    if isinstance(strat, pylib_eprot.estrat_task):
        return str(strat)
    else:
        return pylib_eprot.e_strip_name(strat[0])

class etestset(object):
    """
    Class representing the current state of a set of test runs.
    """
    def __init__(self, strats=[], auto_sync = None):
        self.strats = strats
        self.jobqueue   = []
        self.processing = {}
        self.auto_sync = auto_sync


    def proc_str(self):
        """
        Return a string decribing current tasks in processing.
        """
        return "\n".join([str(i) for i in self.processing.values()])+"\n"


    def strats_str(self):
        """
        Return a string decribing not yet activated tasks.
        """
        return "\n".join([strat_str(i) for i in self.strats])+"\n"


    def activate_strat(self):
        """
        Activate a strategy. This will fail with IndexError if no
        open strategy is available either in full or in string form.
        """
        name, specdir, protdir = self.strats.pop(0)
        strat = pylib_eprot.estrat_task(name, self.auto_sync)
        try:
            strat.parse(specdir, protdir)
            self.jobqueue.extend(strat.generate_jobs())
            self.processing[strat.name] = strat
        except IOError, problem:
            pylib_io.verbout("Cannot parse "+name+": "+str(problem)+"\n");


    def purge_jobs(self):
        """
        Remove all jobs from the current job queue.
        """
        self.jobqueue = []

    def deactivate_strat(self, strat, back=True):
        """
        Remove a strategy from processing and re-add it to the
        strats-queue (unless it is complete)
        """
        if strat.name in self.processing:
            del(self.processing[strat.name])
            tmp = filter(lambda x:not strat.has_job(x)  ,self.jobqueue)
            self.jobqueue = tmp

        strat.sync()
        if not strat.complete():
            if back:
                self.strats.append((strat.name, strat.specdir, strat.protdir))
            else:
                self.strats.insert((strat.name, strat.specdir, strat.protdir))

    def add_strat(self, name, specdir, protdir):
        """
        Add a strategy to the strats queue.
        """
        self.strats.append((name, specdir, protdir))

    def sneak_strat(self, name, specdir, protdir):
        """
        Add a strategy to the top of strats queue.
        """
        self.strats.insert(0, (name, specdir, protdir))


    def next_job(self):
        """
        Return the next open job (if any)
        """
        try:
            while len(self.jobqueue) == 0:
                self.activate_strat()
            return self.jobqueue.pop(0)
        except IndexError:
            return None


    def add_result(self, stratname, result):
        """
        Add a result (instance of eresult) to the test set.
        """
        if stratname in self.processing:
            strat = self.processing[stratname]
            strat.add_result(result)
            if strat.complete():
                strat.sync()
                del self.processing[stratname]            
            return True
        return False
               
    
if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "hd:")

    dir = ""

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        elif option == "-d":
            dir = optarg
        else:
            sys.exit("Unknown option "+ option)

    if len(args) < 1:
        print __doc__
        sys.exit()

    testset = etestset([(i,dir, dir) for i in args])

    job = testset.next_job()
    while job:
        print repr(job)
        job = testset.next_job()

    print testset.processing
