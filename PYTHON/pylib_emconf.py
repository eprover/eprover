#!/usr/bin/env python

"""
Module pylib_emconf.py

Code for handling and representing the configuration of a distributed
E compute master.

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


DEFAULT_ANNOUNCE_PORT = 30000
"""
Servers announce their presence here.
"""

DEFAULT_CONTROL_PORT  = 40000
"""
Control applications connect here.
"""

DEFAULT_TESTDIR = "EPROVER/TESTRUNS_SERVER"
"""
Default directory for test specs and results.
"""

DEFAULT_SYNC_COUNT = 30
"""
How often will a protocol be automatically synced to disk?
"""


filename_hack_re = re.compile("\.\.")


class e_mconfig(object):
    """
    Represent the configuration of a master.
    """
    template=\
"""
Port:               %d
Job directory:      %s
Result directory:   %s
"""

    def __init__(self, config=None):
        homedir = pylib_io.get_homedir()

        self.port         = DEFAULT_ANNOUNCE_PORT
        self.ctrl_port    = DEFAULT_CONTROL_PORT
        self.specdir      = homedir+"/"+DEFAULT_TESTDIR
        self.protdir      = homedir+"/"+DEFAULT_TESTDIR
        self.auto_sync    = DEFAULT_SYNC_COUNT
        self.mode         = "active"

        if not config:
            return

        try:
            confdict, conflist = pylib_io.parse_config_file(config)

            for key in confdict:
                value = confdict[key]

                if key == "Job directory":
                    self.specdir = value
                elif key == "Result directory":
                    self.protdir = value
                elif key == "Port":
                    self.port = int(value)
                elif key == "Control port":
                    self.ctrl_port = int(value)
                elif key == "Synchronize":
                    self.auto_sync = int(value)
                elif key == "Mode":
                    self.mode = value
                else:
                    raise pylib_io.ECconfigSyntaxError("Unknown keyword",
                                                       key)
        except pylib_io.ECconfigSyntaxError, inst:
            sys.stderr.write(str(inst)+"\n")
            sys.exit(1)

    def __str__(self):
        return e_mconfig.template%(self.port, self.specdir, self.protdir)


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
    c = e_mconfig("emaster_config.txt")
    print c
