#!/usr/bin/env python2.4

"""
emaster 0.1

Usage: emaster.py <config_file> [jobfile]...

Run a master process waiting for E-server announcements and
distibuting E jobs to the recognized servers. 

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
import string
import os
import getopt
import socket
import select
import pylib_generic
import pylib_io
import pylib_tcp
import pylib_emconf


announce_matcher = re.compile("eserver:[0-9]+")

class eslave(object):
    def __init__(self, connection):
        self.connection = connection
        self.open_jobs = 0



class emaster(object):
    def __init__(self, config):
        self.rec_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.rec_sock.bind(("", config.port))
        self.client = pylib_tcp.tcp_client()
        self.slaves = {}

        
    def process(self):
        while True:
            ractive = [self.rec_sock]
            wactive = []
            ready   = select.select(ractive, wactive, [], 1)
            if self.rec_sock in ready[0]:
                self.handle_announce()

    def handle_announce(self):
        (data,sender) = self.rec_sock.recvfrom(1024)
        slave_addr, port = sender
        if slave_addr in self.slaves:
            return
        if not announce_matcher.match(data):
            return
        slave_port = int(data[8:])
        if port<1000 or port > 65535:
            return
        connection = self.client.connect((slave_addr, slave_port))
        slave = eslave(connection)
        self.slaves[slave_addr] = slave

if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "hv", ["Verbose"])

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        elif option == "-v" or option =="--verbose":
            pylib_io.Verbose = 1
        else:
            sys.exit("Unknown option "+ option)

    if len(args) > 1 or len(args)<1:
        print __doc__
        sys.exit()

    config = pylib_emconf.e_mconfig(args[0])

    master = emaster(config)

    master.process()
