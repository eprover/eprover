#!/usr/bin/env python

"""
Module pylib_econf.py

Code for handling and representing the configuration of a distributed
E compute server.

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
import getopt
from socket import gethostbyname
import pylib_generic
import pylib_io


DEFAULT_LISTEN_PORT = 20000
"""
What is the deault port for listening?
"""

DEFAULT_LOAD_LIMIT = 5000.0
"""
Maximum load acceptable for starting new jobs.
"""


filename_hack_re = re.compile("\.\.")


class e_config(object):
    """
    Represent the configuration of a server. This will normally be
    read from a server-local file.
    """
    template=\
"""
Port:               %d
Binaries directory: %s
Problem directory:  %s
E-Mark:             %f
Memory limit:       %d
Auto options:       %s
Master:             %s
Nice:               %d
"""
    
    def __init__(self, config=None):
        homedir = pylib_io.get_homedir()

        self.port         = DEFAULT_LISTEN_PORT
        self.bindir       = homedir+"/EPROVER/bin"
        self.problemdir   = homedir+"/EPROVER/TPTP"
        self.e_mark       = 100.0
        self.memory_limit = 384
        self.max_procs    = 1
        self.auto_opt     = "-s --print-statistics"
        self.nicelevel    = 10
        self.local_blocks = False
        self.load_limit   = DEFAULT_LOAD_LIMIT
        self.masters      = []
        self.mode         = "announce"
        
        if not config:
            return

        try:
            confdict, conflist = pylib_io.parse_config_file(config)

            for key in confdict:
                value = confdict[key]

                if key == "Binaries directory":
                    self.bindir = value
                elif key == "Port":
                    self.port = int(value)
                elif key == "Problem directory":
                    self.problemdir = value
                elif key == "E-Mark":
                    self.e_mark = float(value)
                elif key == "Memory limit":
                    self.memory_limit = int(value)
                elif key == "Max processes":
                    self.max_procs = int(value)
                elif key == "Auto options":
                    self.auto_opt = value
                elif key == "Nice":
                    self.nicelevel = int(value)
                elif key == "Local blocks":
                    self.local_blocks = value=="true"
                elif key == "Load limit":
                    self.load_limit = float(value)
                elif key == "Master":
                    masterlist = value.split(",")
                    for i in masterlist:
                        try:
                            host, port = i.split(":")
                            host = gethostbyname(host.strip())
                            self.masters.append((host, int(port.strip())))
                        except ValueError:
                            raise pylib_io.ECconfigSyntaxError("Malformed IP address",
                                                               i)
                elif key == "Mode":
                    self.mode = value
                else:
                    raise pylib_io.ECconfigSyntaxError("Unknown keyword",
                                                       key)
        except pylib_io.ECconfigSyntaxError, inst:
            sys.stderr.write(str(inst)+"\n")
            sys.exit(1)
        
    def __str__(self):
        addr = []
        for i in self.masters:
            addr.append(i[0]+":"+str(i[1]))
        masters = ",".join(addr)

        return e_config.template%(self.port, self.bindir, self.problemdir,
                                  self.e_mark, self.memory_limit,
                                  self.auto_opt, masters)


    def concrete_time(self, time, rawtime=False):
        """
        Given an abstract time (with respect to 100 E-Mark),
        return the concrete time for this configuration.
        """
        if rawtime:
            return time
        return time*100.0/self.e_mark

    def abstract_time(self, time, rawtime=False):
        """
        Given an concrete time limit, return the corresponding
        abstract time.
        """
        if rawtime:
            return time
        return time*self.e_mark/100

    def command(self, prover, options, prob, timelimit, rawtime=False):
        """
        Return a proper command array for E.
        """

        # Make sure that prover is not pointing somewhere strange
        
        if filename_hack_re.search(prover):
            cmd_array = ["/bin/false"]

        else:
            procname = "%s/%s" % (self.bindir, prover)
            limits   = "--cpu-limit=%d --memory-limit=%d" %\
                       (self.concrete_time(timelimit, rawtime), self.memory_limit)
            probname = "%s/%s" % (self.problemdir, prob)
            cmd_array = [procname] + \
                        pylib_generic.break_shell_str(limits)+\
                        pylib_generic.break_shell_str(self.auto_opt)+\
                        pylib_generic.break_shell_str(options)+\
                        pylib_generic.break_shell_str(probname)
        
        return cmd_array
        

if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    if len(args) > 2:
        print __doc__
        sys.exit()

    # Minimal unit test
    c = e_config("eserver_config.txt")
    print c
