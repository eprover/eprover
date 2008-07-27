#!/usr/bin/env python2.4

"""
eserver 0.1

Usage: eserver.py <config_file>

Run a server process accepting and processing jobs for E. 

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
import select
import os
import getopt
import socket
import pylib_io
import pylib_econf
import pylib_erun

jobend_re = re.compile("(^|\n|\r\n)\.(\n|\r\n)")

class connection:
    def __init__(self, conn):
        self.sock = conn[0]
        self.peeradr = conn[1]
        self.inbuffer  = ""
        self.outbuffer = ""
        self.filenum   = self.sock.fileno()

    def __str__(self):
        return "<conn (%d) to %s>" % (self.filenum, str(self.peeradr))

    def fileno(self):
         return self.filenum

    def send(self):
        if self.sendable() and not pylib_io.write_will_block(self):
            res = self.sock.send(self.outbuffer)
            self.outbuffer = self.outbuffer[res:]
            return res

    def write(self, data):
        self.outbuffer = self.outbuffer+data
        return self.send
    
    def sendable(self):
        return self.outbuffer!=""

    def recv(self):
        res = self.sock.recv(2048)
        if res == "":
            self.sock.close()
            self.finalize()
            return res        
        self.inbuffer = self.inbuffer+res
        print "Buffer: ",  list(self.inbuffer)
        mo = jobend_re.search(self.inbuffer)
        if mo:
            res = self.inbuffer[:mo.start()]+"\n"
            self.inbuffer = self.inbuffer[mo.end():]
        else:
            res = None
        return res

    def finalize(self):
        self.outbuffer = ""
        self.filenum   = -1


      
class tcp_server:
    """
    Class implementing the listening port of a server and
    creating connected sockets.
    """
    def __init__(self, port):
        self.listen_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        inst = None

        while port < 65536:
            try:
                self.port = port
                self.listen_sock.bind(("", port))
                self.listen_sock.listen(5)
                print "Got port", self.port
                return
            except socket.error, inst:
                port = port+1
        raise inst

    def fileno(self):
        """
        Return the servers fileno to support select.
        """
        return self.listen_sock.fileno() 
    
    def accept(self):
        try:
            connect = connection(self.listen_sock.accept())
        except:
            connect = None
        return connect


class running_job:
    def __init__(self, connection, runner):
        self.connection = connection
        self.runner     = runner

    def fileno(self):
        return self.runner.fileno()

    def __str__self(self):
        return "<"+str(self.connection)+":"+str(self.runner)+">"

    def check_and_report(self):
        res = self.runner.get_result()
        if res:
            self.connection.write(str(res)+"\n")
            return True
        else:
            return False

class waiting_job:
    def __init__(self, connection, deflist):
        self.deflist    = deflist
        self.connection = connection

    def __str__(self):
        return "<:"+":".join(self.deflist)+":>"

class eserver:
    def __init__(self, config):
        self.jobs        = []
        self.running     = []
        self.connections = []
        self.server      = tcp_server(config.port)
        self.config      = config

    def process(self):
        while True:
            ractive = [self.server]+self.connections+self.running
            wactive = [i for i in self.connections if i.sendable()]
            ready   = select.select(ractive, wactive, [], 1)

            for i in ready[0]:
                if i == self.server:
                    new_conn =  self.server.accept()
                    print "New connection", new_conn
                    self.connections.append(new_conn)

                if i in self.connections:
                    command = i.recv()
                    if command == "":
                        print "Connection to ", i, "terminated"
                        self.connections.remove(i)
                        i.finalize()
                    elif command:
                        self.dispatch(i, command)

                if i in self.running:
                    if i.check_and_report():
                        self.running.remove(i)
                        
            for i in ready[1]:
                i.send()

            if len(self.running) < self.max_jobs():
                self.run_job()

    def run_job(self):
        if  len(self.jobs) == 0:
            return        
        print "Moving job from jobs to running"

        job = self.jobs[0]
        self.jobs.remove(job)
        try:
            key     = job.deflist[1]
            prover  = job.deflist[2]
            args    = job.deflist[3]
            problem = job.deflist[4]
            time    = job.deflist[5]
            if time[-1] == "r":
                rawtime = True
                time = float(time[:-1])
            else:
                rawtime = False
                time = float(time)
            if len(job.deflist) > 6:
                res_descriptor = job.deflist[6].split(",")
            else:
                res_descriptor = []
        except:
            return

        print "arg = ", args
        runner = pylib_erun.e_runner(key, self.config, prover,
                                     args, problem, time, rawtime,
                                     res_descriptor)
        rjob = running_job(job.connection, runner)
        self.running.append(rjob)        


    def max_jobs(self):
        return 2

    def create_job(self, connection, command_list):
        job = waiting_job(connection, command_list)
        print "Created waiting job", job
        self.jobs.append(job)
        

    def list_state(self, connection):
        connection.write("Running:\n")
        for i in self.running:
            connection.write("  "+str(i)+"\n")
        connection.write("Waiting:\n")
        for i in self.jobs:
            connection.write("  "+str(i)+"\n")
        connection.write("Connections:\n")
        for i in self.connections:
            connection.write("  "+str(i)+"\n")
        connection.write("===\n")

    def dispatch(self, connection, command):
        print "Command: ", command
        cl = command.split("\n")
        cl = pylib_io.clean_list(cl)

        if cl[0] == "run":
            self.create_job(connection, cl)
        elif cl[0] == "ls":
            self.list_state(connection)
        elif cl[0] == "restart":
            print sys.argv[1:]
            os.execv(sys.argv[0], sys.argv)
                
        

if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    if len(args) > 1 or len(args)<1:
        print __doc__
        sys.exit()

    print args
    config = pylib_econf.e_config(args[0])

    server =eserver(config)

    server.process()
