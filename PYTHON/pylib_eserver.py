#!/usr/bin/env python2.5

"""
pylib_eserver 0.1

Main library for the E server.

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
import socket
import pylib_generic
import pylib_io
import pylib_econf
import pylib_erun
import pylib_tcp


service = "eserver"
version = "0.1dev"

jobend_re = re.compile("(^|\n|\r\n)\.(\n|\r\n)")

solution_re = re.compile("\[.*?\]")


class running_job(object):
    def __init__(self, connection, runner):
        self.connection = connection
        self.runner     = runner

    def fileno(self):
        return self.runner.fileno()

    def __str__self(self):
        return "<"+str(self.connection)+":"+str(self.runner)+">"

    def check_and_report(self):
        res = self.runner.get_result()
        if res != None:
            self.connection.write(str(res)+"\n")
            pylib_io.verbout("Job"+str(self)+"done: "+str(res))
            return True
        else:
            return False

class waiting_job(object):
    def __init__(self, connection, deflist):
        self.deflist    = deflist
        self.connection = connection

    def __str__(self):
        return "<:"+":".join(self.deflist)+":>"

class eserver(object):
    def __init__(self, config):
        self.jobs        = []
        self.running     = []
        self.connections = []
        self.server      = pylib_tcp.etcp_server(config.port)
        self.config      = config

    def process(self):
        announce = pylib_generic.timer(0)
        
        while True:
            if announce.expired():
                self.announce()
                #announce.set(60.0)
                announce.set(10.0)

            ractive = [self.server]+self.running+\
                      [i for i in self.connections if i.readable()]
            wactive = [i for i in self.connections if i.sendable()]
            ready   = select.select(ractive, wactive, [], 1)

            for i in ready[0]:
                if i == self.server:
                    new_conn =  self.server.accept(jobend_re)
                    pylib_io.verbout("New connection "+str(new_conn))
                    self.connections.append(new_conn)

                if i in self.connections:
                    commands = i.read()
                    for command in commands:
                        if command == "":
                            pylib_io.verbout("Connection "+str(i)+" terminated")
                            self.connections.remove(i)
                        else:
                            self.dispatch(i, command)

                if i in self.running:
                    if i.check_and_report():
                        self.running.remove(i)
                        
            for i in ready[1]:
                i.send()

            if len(self.running) < self.max_jobs():
                self.run_job()

    def announce(self):
        for addr, port in self.config.masters:
            self.server.announce_self((addr, port))

    def run_job(self):
        if  len(self.jobs) == 0:
            return        

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

        runner = pylib_erun.e_runner(key, self.config, prover,
                                     args, problem, time, rawtime,
                                     res_descriptor)
        rjob = running_job(job.connection, runner)
        self.running.append(rjob)        


    def max_jobs(self):
        return self.config.max_procs

    def create_job(self, connection, command_list):
        job = waiting_job(connection, command_list)
        pylib_io.verbout("Creating job"+str(job))
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

    def dispatch(self, connection, command):
        cl = command.split("\n")
        cl = pylib_io.clean_list(cl)
        pylib_io.verbout(cl[0])
        if cl:
            if cl[0] == "run":
                self.create_job(connection, cl)
            elif cl[0] == "ls":
                self.list_state(connection)
            elif cl[0] == "restart":
                pylib_io.closerange(sys.stderr.fileno()+1, 10000)
                os.execv(sys.argv[0], sys.argv)
            elif cl[0] == "version":
                connection.write(service+" "+version+"\n")
            else:
                connection.write("Error: Unkown command "+cl[0]+".\n")
        
                

def eserver_get_reply(address, command):
    """
    Connect to an eserver, issue a command, and return the first
    complete reply (or None) if the connection fails.
    """
    try:
        conn = pylib_tcp.tcp_client().connect(address)
    except socket.error:
        pylib_io.verbout("No running server found.")
        return None

    conn.write(command)
    while conn.sendable():
        conn.send()

    complete, res = None, ""
    while not complete:
        complete, res = conn.read()
    conn.close()
    return res
    
