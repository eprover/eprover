#!/usr/bin/env python

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
import subprocess
import pylib_io
import pylib_econf


resline_re     = re.compile("#.*?:.*")


class runner(object):
    """
    Class to run a (long-running) command and to return the result.
    """
    def __init__(self, cmd):
        
        self.proc = subprocess.Popen(cmd, stdin=None,
                                     stdout=subprocess.PIPE,
                                     close_fds=True)
        self.cmd = cmd
        
    def fileno(self):
        """
        Return the pipe's fileno to support select.
        """
        return self.proc.stdout.fileno()

    def get_result(self):
        """
        Check if command has completed. If yes, return (True,
        result). Otherwise return (False, undefined (implemented as
        the temporary result)). Please note that this will  
        block if cmd has neither finished nor provided new data. To
        avoid blocking, use select() first.
        """
        ret = self.proc.poll()

        if ret==None:
            return (False, "")
        
        tmp, tmperr = self.proc.communicate()

        if ret and ret != 6:
            # 6 is E result from CPU timeout
            print "# Warning: '", self.cmd, "' returned status "+\
                  repr(ret)
        return (True, tmp)



class e_res_parser(runner):
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
    

    def __init__(self, time, res_descriptor =[]):
        """
        Initialize E-Runner object.
        """
        self.res_descriptor = res_descriptor
        self.time    = time

    def translate_result(self, e_output):
        """
        Decode the E result and return the list of
        result items.
        """

        resl = e_output.splitlines()

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
            reason = e_res_parser.failure_trans[tmp]
        except:
            if status in ["Theorem", "Unsatisfiable", \
                          "CounterSatisfiable", "Satisfiable"]:
                reason = "success"
            else:
                reason = "unknown"

        try:
            tmp   = resdict["# Total time"]
            time  = float(tmp.split()[0])
        except:
            time = "-"
            
        result = [status, reason, time]

        for i in self.res_descriptor:
            try:
                tmp = resdict[i]
            except:
                tmp = "-"
            result.append(tmp)          

        return result
    
    def encode_result(self, e_output, default_time):
        """
        Return a string representing the result status (i.e. a line in
        an E protocol, but without the (unknown) problem name).
        """
        res = self.translate_result(e_output)
        
        if res:
            if res[2] == "-":
                res[2] = default_time
            result = "%s %f %s "%\
                     (e_res_parser.status_trans[res[0]], res[2], res[1])
            extra = " ".join([str(i) for i in res[3:]])
            return result+extra
        else:
            return None


class e_runner(runner):
    """
    Class to run E and return an interpreted result.
    """

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
        print "Command", cmd
        runner.__init__(self, cmd)
        self.res_parser = e_res_parser(time, res_descriptor)


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
        print res

        res = self.res_parser.translate_result(res)

        status, reason, time = res[0:3]
        rest = res[3:]

        if time == "-":
            time = self.time
            atime = self.time
        else:
            atime = self.config.abstract_time(time)

        result = [self.key, self.problem, status, atime, reason, time]

        for i in rest:
            result.append(i)          

        return result
    




if __name__ == '__main__':
    c = pylib_econf.e_config("EXAMPLE_CFG/eserver_config_test.txt")

    r1 = e_runner("teststrat", c, "eprover", "--tptp3-in -xAuto -tAuto",
                  "GRP001+6.p", 10,
                  ["# Processed clauses",
                   "# Generated clauses",
                   "# Clause-clause subsumption calls (NU)"])
    r2 = e_runner("teststrat", c, "eprover", "--tptp3-in -xAuto -tAuto",
                  "RNG004-1.p", 10, 
                  ["# Processed clauses",
                   "# Generated clauses",
                   "# Clause-clause subsumption calls (NU)"]) 
    r3 = e_runner("teststrat", c, "eprover", "--tptp3-in -xAuto -tAuto",
                  "SET542-6.p", 10, 
                  ["# Processed clauses",
                   "# Generated clauses",
                   "# Clause-clause subsumption calls (NU)"])
   

    running = [r1, r2, r3]
    while len(running)!=0:
        readable = select.select(running, [], [], 1)
        print readable
        for r in readable[0]:
            result = r.get_result()
            if result:
                running.remove(r)
                print result
