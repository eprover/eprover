#!/usr/bin/env python2.5

"""
Module pylib_erun.py

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
import select
import os
import popen2
import pylib_io
import pylib_econf


resline_re     = re.compile("#.*?:.*")


class runner(object):
    """
    Class to run a (long-running) command and to return the result.
    """
    def __init__(self, cmd):
        
        self.fp = popen2.popen2(cmd)[0]
        self.result = ""
 
    def fileno(self):
        """
        Return the pipe's fileno to support select.
        """
        return self.fp.fileno()

    def get_result(self):
        """
        Check if command has completed. If yes, return (True,
        result). Otherwise return (False, undefined (implemented as
        the temporary result)). Please note that this will  
        block if cmd has neither finished nor provided new data. To
        avoid blocking, use select() first.
        """
        tmp = self.fp.read()
        if tmp != "":
            self.result = self.result+tmp;
            return (False, self.result)
        status = self.fp.close()
        if status:
            print "# Warning: '"+cmd+"' returned status "+\
                  repr(decode_wait_status(status)[0])
        return (True, self.result)


class e_runner(runner):
    """
    Class to run E and return an interpreted result.
    """

    status_trans = \
    {
        "ResourceOut"        :"F",
        "InputError"         :"F",
        "Theorem"            :"T",
        "Unsatisfiable"      :"T",
        "Unknown"            :"F",
        "GaveUp"             :"F",
        "CounterSatisfiable" :"N",
        "Satisfiable"        :"N"
    }

    failure_trans = \
    {
        "Resource limit exceeded (memory)": "maxmem",
        "Resource limit exceeded (time)"  : "maxtime",
        "Out of unprocessed clauses!"     : "incomplete",
        "User resource limit exceeded!"   : "incomplete"
    }
    

    def __init__(self, key, config, prover, args, problem, time,
        rawtime=False, res_descriptor =[]):
        """
        Initialize E-Runner object.
        """
        self.res_descriptor = res_descriptor
        self.config = config
        self.key = key
        self.rawtime = rawtime
        self.time    = time
        self.problem = problem
        cmd = config.command(prover, args, problem, time, rawtime)
        runner.__init__(self, cmd)

    def __str__(self):
        return "<e_run:"+self.key+":"+self.problem+">"

    def get_result(self):
        """
        Check if a result is available. If yes, return the list of
        result items. If no, return None.
        """
        status, res = runner.get_result(self)
        if not status:
            return None

        resl = res.splitlines()

        resdict = {}
        status = "Unknown"

        for l in resl:
            mo = resline_re.match(l)
            if mo:
                key, val = l.split(":",1)
                key = key.strip()
                val = val.strip()
                resdict[key] = val
            elif l.startswith("# SZS status"):
                status = l[len("# SZS status "):]

        try:
            tmp = resdict["# Failure"]
            reason = e_runner.failure_trans[tmp]
        except:
            if status in ["Theorem", "Unsatisfiable", \
                          "CounterSatisfiable", "Satisfiable"]:
                reason = "success"
            else:
                reason = "unknown"

        try:
            tmp   = resdict["# Total time"]
            time  = float(tmp.split()[0])
            atime = self.config.abstract_time(time)
        except:
            time = self.time
            atime = self.time
            
        result = [self.key, self.problem, status, atime, reason, time]

        for i in self.res_descriptor:
            try:
                tmp = resdict[i]
            except:
                tmp = "-"
            result.append(tmp)          

        return result
    




if __name__ == '__main__':
    c = pylib_econf.e_config("eserver_config.txt")

    r1 = e_runner("teststrat", c, "eprover", "LUSK3.lop", "-xAuto -tAuto", 100,
                  ["# Processed clauses",
                   "# Generated clauses",
                   "# Clause-clause subsumption calls (NU)"])
    r2 = e_runner("teststrat", c, "eprover", "LUSK6.lop", "-xAuto -tAuto", 100,
                  ["# Processed clauses",
                   "# Generated clauses",
                   "# Clause-clause subsumption calls (NU)"])
    

    running = [r1, r2]
    while len(running)!=0:
        readable = select.select(running, [], [], 1)
        print readable
        for r in readable[0]:
            result = r.get_result()
            if result:
                running.remove(r)
                
