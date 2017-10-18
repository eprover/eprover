#!/usr/bin/env python

"""
pylib_emaster.py 0.1

Library containing the master code for the distributed E test system. 

Options:

-h Print this help.

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
import re
import string
import os
import getopt
import socket
import time
import select
import pylib_generic
import pylib_io
import pylib_tcp
import pylib_emconf
import pylib_eprot
import pylib_etestset


announce_matcher = re.compile("eserver:[0-9]+:[^:]*:[^:]*")
SLAVE_OPEN_JOB_LIMIT = 10




class xresult(object):
    def __init__(self, desc_str):
        tmp = map(lambda x:x.strip(),desc_str[1:-2].split(","))
        self.strat      = tmp[0].strip("'")
        self.prob       = tmp[1].strip("'")
        self.state      = tmp[2].strip("'")
        self.raw_time   = float(tmp[5])
        self.terminated = tmp[4].strip("'")
        self.norm_time  = float(tmp[3])
        self.rest       = tmp[6:]

    def key(self):
        return self.strat+self.prob

    def res_str(self):
        if self.state in ["Theorem", "Unsatisfiable"]:
            status = "T"
        elif self.state in ["CounterSatisfiable", "Satisfiable"]:
            status = "N"
        else:
            status = "F"
        fixed = "%-29s %s %8s %-10s "%(self.prob, status,
                                       self.norm_time, self.terminated)
        res = fixed+" ".join([str(i) for i in self.rest])
        return res
            
           

class eslave(object):
    def __init__(self, connection, addr, name=None, emark=None):
        self.connection = connection
        self.addr  = addr
        self.name  = name
        self.emark = emark
        self.open_jobs = {}
        self.last_activity = time.time()

    def __str__(self):
        if self.initialized():
            gap = time.time() - self.last_activity
            return "<eslave:%s:open=%d:%s:gap=%.2f:emark=%f>"%\
                   (str(self.addr),self.jobs_no(),self.name,gap, self.emark)
        else:
            return "<eslave:%s:uninitialized>"%(str(self.addr),)

    def __cmp__(self, other):
        try:
            res = cmp(self.name, other.name)
        except AttributeError:
            res = 0
        if not res:
            res = cmp(hash(self), hash(other))
        return res

    def address(self):
        """
        Return the IP addess of the partner.
        """
        return self.addr

    def fileno(self):
        return self.connection.fileno()

    def sendable(self):
        return self.connection.sendable()

    def jobs(self):
        return self.open_jobs.values()

    def jobs_no(self):
        return len(self.jobs())

    def add_job(self, job):
        self.open_jobs[job.key()] = job
        pylib_io.verbout("Adding:"+str(job))
        self.connection.write(str(job))

    def initialized(self):
        return self.name != None

    def proc_read(self):
        """
        Try to read a result from the connection. Return a list of
        (results or empty string), with empty string indicating a
        broken connection.
        """
        self.last_activity = time.time()
        tmp = self.connection.read()
        ret = list()
        for i in tmp:
            if i== "":
                ret.append("")
            else:
                if not self.initialized():
                    tmp = i.split(":")
                    try:
                        slave_port = int(tmp[1])
                        slave_name = tmp[2]
                        slave_emark = float(tmp[3])
                        self.name = slave_name
                        self.emark = slave_emark
                        pylib_io.verbout("Slave "+str(self.connection)+" initialized")
                        
                    except (IndexError, ValueError):
                        pass
                else:
                    try:
                        res = xresult(i)
                        ret.append(res)
                        del self.open_jobs[res.key()]
                    except (IndexError, KeyError):
                        pass
                    
        return ret

    def proc_write(self):
        """
        Try to write to the connection. Return the number of bytes
        written.
        """
        res = self.connection.send()



class emaster(object):
    def __init__(self, config, auto_sync=5):
        self.rec_sock     = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.rec_sock.bind(("", config.port))
        self.config       = config
        self.client       = pylib_tcp.tcp_client()
        self.slaves       = {}
        self.strats       = pylib_etestset.etestset([], auto_sync)
        self.ctrl_server  = pylib_tcp.tcp_server(config.ctrl_port)
        self.slave_server = pylib_tcp.tcp_server(config.port)
        self.ctrls        = []


    def add_strat(self, strat):
        self.strats.add_strat(strat, self.config.specdir, self.config.protdir)
        pylib_io.verbout("New job (%d open):"%(len(self.strats.strats),)+str(strat))

    def sneak_strat(self, strat):
        self.strats.sneak_strat(strat, self.config.specdir, self.config.protdir)
        pylib_io.verbout("New job (%d open):"%(len(self.strats.strats),)+str(strat))

    def add_slave_jobs(self, slave):
        res = 0
        while slave.initialized() and (slave.jobs_no() < SLAVE_OPEN_JOB_LIMIT):
            job = self.strats.next_job()
            if job:
                slave.add_job(job)
                res = res+1
            else:
                res = None
                break
        return res

    def prune_stale_strats(self):
        for i in  self.strats.processing.values():
            if i.stale():
                self.strats.deactivate_strat(i)
                
        
    def process(self):
        while True:
            self.prune_stale_strats()

            ractive = [self.rec_sock, self.ctrl_server,
                       self.slave_server]+\
                       self.slaves.values()+self.ctrls
            wactive = ([i for i in self.slaves.values() if i.sendable()] +
                       [i for i in self.ctrls if i.sendable()])
            
            ready   = select.select(ractive, wactive, [], 1)
            
            for reader in ready[0]:
                if reader == self.rec_sock:
                    self.handle_announce()
                elif reader == self.ctrl_server:
                    new_ctrl = reader.accept()
                    if new_ctrl:
                        self.ctrls.append(new_ctrl)
                        new_ctrl.write("> ")
                elif isinstance(reader, eslave):
                    results = reader.proc_read()
                    if not self.add_results(results):
                        del(self.slaves[reader.address()])
                elif reader == self.slave_server:
                    self.handle_connect(reader)
                else:
                    self.process_ctrl(reader)
            for writer in ready[1]:
                if isinstance(writer, eslave):
                    writer.proc_write()
                else:
                    writer.send()

            for i in self.slaves.values():
                if self.add_slave_jobs(i) == None:
                    break


    def process_ctrl(self, ctrl):
        """
        Read and process control commands.
        """
        commands = ctrl.read()
        for i in commands:
            if i == "":
                self.ctrls.remove(ctrl)
                return
            else:
                self.ctrl_command(ctrl, i[:-1])

    def ctrl_command(self, ctrl, command):
        """
        Execute control commands.
        """
        if command == "ls":
            self.exec_ls(ctrl)
        elif command.startswith("add"):
            self.exec_add(self, command)
        elif command.startswith("sneak"):
            self.exec_sneak(self, command)
        elif command.startswith("help"):
            self.exec_help(ctrl)
        elif command == "restart slaves":
            self.exec_restart()
        elif command == "terminate master":
            self.exec_terminate()
        elif command == "quit":
            self.exec_quit(ctrl)
        elif command == "purge":
            self.exec_purge()
        elif command =="":
            pass
        else:
            ctrl.write("Unknown command\n")
        ctrl.write("> ")
            
    def exec_ls(self, ctrl):        
        ctrl.write("Slaves (%d):\n"%(len(self.slaves,)))
        slaves = self.slaves.values()
        slaves.sort()
        for i in slaves:
            ctrl.write(str(i)+"\n")
        ctrl.write("Running tasks:\n")
        ctrl.write(self.strats.proc_str())
        ctrl.write("Scheduled tasks:\n")               
        ctrl.write(self.strats.strats_str())

    def exec_help(self, ctrl):
        ctrl.write("""
Commands:

ls
add <strategies>
sneak <strategies> 
help
restart slaves
terminate master
quit
purge
""")

    def exec_add(self, ctrl, command):
        for i in command.split()[1:]:
            self.add_strat(i)

    def exec_sneak(self, ctrl, command):
        for i in command.split()[1:]:
            self.sneak_strat(i)

    def exec_restart(self):
        for i in self.slaves.values():
            i.connection.write("\nrestart\n.\n")

    def exec_terminate(self):
        sys.exit()
        
    def exec_quit(self, ctrl):
        ctrl.close()
        self.ctrls.remove(ctrl)

    def exec_purge(self):
        self.strats.purge_jobs()
        

    def add_results(self, results):
        """
        Take a list of xresults and add them to the testset.
        """
        for i in results:
            if i == "":
                return False
            else:
                res = pylib_eprot.eresult(i.res_str())
                self.strats.add_result(i.strat, res)

        return True
        

    def handle_announce(self):
        (data,sender) = self.rec_sock.recvfrom(1024)
        slave_addr, port = sender
        if slave_addr in self.slaves:
            return
        if not announce_matcher.match(data):
            return
        slave_name  = "<unknown>"
        slave_emark = -1.0
        tmp = data.split(":")
        slave_port = int(tmp[1])      
        if port<1000 or port > 65535:
            return
        try:
            slave_name = tmp[2]
            slave_emark = float(tmp[3])
        except (IndexError, ValueError):
            pass

        try:
            connection = self.client.connect((slave_addr, slave_port))
            pylib_io.verbout("New slave: "+str(connection))
            slave = eslave(connection, slave_addr, slave_name, slave_emark)
            self.slaves[slave_addr] = slave
        except (socket.timeout, socket.error):
            pass

    def handle_connect(self, listener):
        """
        Handle a slave trying to connect.
        """
        connection = listener.accept()
        if not connection:
            return
        slave_addr = connection.peer_adr()

        pylib_io.verbout("New pre-slave: "+str(connection))
        slave = eslave(connection, slave_addr)
        self.slaves[slave_addr] = slave

if __name__ == '__main__':
    # Add unit tests here ;-)
    pass

