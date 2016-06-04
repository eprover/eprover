#!/usr/bin/env python

"""
pylib_tcp 0.1

Library supporting TCP servers, clients, and connections.

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


lineend_re = re.compile("\n|\r\n")
jobend_re = re.compile("(^|\n|\r\n)\.(\n|\r\n)")


class connection(object):
    def __init__(self, conn, rec_end = lineend_re):
        self.sock = conn[0]
        self.peeradr = conn[1]
        self.inbuffer  = ""
        self.outbuffer = ""
        self.filenum   = self.sock.fileno()
        self.rec_end = rec_end

    def __str__(self):
        return "<conn (%d) to %s>" % (self.filenum, str(self.peeradr))

    def fileno(self):
         return self.filenum

    def peer_adr(self):
         return self.peeradr

    def send(self):
        if self.sendable() and not pylib_io.write_will_block(self):
            res = 0
            try:
                res = self.sock.send(self.outbuffer)
            except socket.error:
                pass
            if res > 0:
                self.outbuffer = self.outbuffer[res:]
            return res
        return 0

    def write(self, data):
        """
        Add data to the write buffer and try to send it. Returns number
        of bytes actually written. Any remaining characters are
        buffered for future calls to send(). 
        """

        self.outbuffer = self.outbuffer+data
        return self.send()
    
    def sendable(self):
        return self.outbuffer!=""

    def readable(self):
        return self.filenum != -1

    def recv(self):
        try:
            res = self.sock.recv(2048)
        except socket.error:
            res = ""
        if res == "":
            self.close()
            return res        
        self.inbuffer = self.inbuffer+res
        return res

    def read(self):
        """
        Try to read complete expressions separated by self.rec_end off
        the connection. Returns a list [expr*], where each expr is
        terminated by a rec_end instance. If any record is "", the
        connection has been broken. An empty list just indicates that
        no complete expression has been found yet.
        """        
        tmp = self.recv()

        res = list()
        mo = self.rec_end.search(self.inbuffer)
        while mo:
            res.append(self.inbuffer[:mo.start()]+"\n")
            self.inbuffer = self.inbuffer[mo.end():]
            mo = self.rec_end.search(self.inbuffer)
        if tmp == "":
            res.append("")

        return res

    def close(self):
        if self.filenum!=-1:
            self.sock.close()
        self.outbuffer = ""
        self.filenum   = -1


      
class tcp_server(object):
    """
    Class implementing the listening port of a server and
    creating connected sockets.
    """
    def __init__(self, port):
        self.udpout = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        self.listen_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        inst = None

        while port < 65536:
            try:
                self.port = port
                self.listen_sock.bind(("", port))
                self.listen_sock.listen(5)
                pylib_io.verbout("TCP Server listening on port "+str(self.port))
                return
            except socket.error, inst:
                port = port+1
        raise inst

    def fileno(self):
        """
        Return the servers fileno to support select.
        """
        return self.listen_sock.fileno() 
    
    def accept(self,rec_end = lineend_re):
        try:
            connect = connection(self.listen_sock.accept(), rec_end)
        except:
            connect = None
        return connect



class etcp_server(tcp_server):
    """
    Class implementing a specialized E TCP server.
    """

    def __init__(self, port, emark):
        tcp_server.__init__(self, port)
        tmp = pylib_io.run_shell_command("hostname")
        try:
            self.hostname = tmp[0].strip("\n")
        except IndexError:
            self.hostname = "<unknown>"
        self.emark = emark

    def announce_self(self, addr, ):
        msg = "eserver:%d:%s:%f\n"%(self.port,self.hostname, self.emark)
        pylib_io.verbout("Announcing: "+msg+" to "+str(addr))
        self.udpout.sendto(msg ,0, addr)
        

class tcp_client(object):
    """
    Class implementing a TCP client, creating a connection from a given
    address. 
    """

    def __init__(self, rec_end = lineend_re):
        self.rec_end = rec_end

    def connect(self, address):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(address)
        return connection((sock,address), self.rec_end)

class etcp_client(tcp_client):
    """
    Class implementing an E-Server specific TCP client, creating a
    connection from a given address and sending the init message.
    """

    def __init__(self, port, emark):
        tcp_client.__init__(self, jobend_re)
        self.port = port
        self.emark = emark
        tmp = pylib_io.run_shell_command("hostname")
        try:
            self.hostname = tmp[0].strip("\n")
        except IndexError:
            self.hostname = "<unknown>"
        self.emark = emark

    def connect(self, address):
        connection = tcp_client.connect(self, address)
        if connection:
            msg = "eserver:%d:%s:%f\n"%(self.port,self.hostname, self.emark)
            pylib_io.verbout("Initial msg: "+msg+" send to "+str(address))
            connection.write(msg)
        return connection

