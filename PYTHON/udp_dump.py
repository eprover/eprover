#!/usr/bin/env python
"""
udp_dump.py 1.0

Usage: udp_dump.py [options] <port>

Receive UDP packets on the designated port and print the contents in
the selected human-friendly format.

Options:
-h       Print this information.
-v       Verbose mode.
-f<file> Log messages into the given file.
-b<file> Write raw UDP messages in binary into the given file.
-d       Print meta-data about received packets.
-s       Print packets as a printable ASCII approximation.
-H       Print packets in hex.

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
import socket
import binascii
import re
import time
import getopt
import pylib_io
import pylib_generic

output_streams = [sys.stdout]

bin_output = None
"""
Stream to write raw messages to.
"""

output_streams = [sys.stdout]
"""
List of streams for normal (text) output.
"""

print_data = False
"""
Print a meta-data line for each message if true.
"""

print_str  = False
"""
Print an ASCII approximation of each message if true.
"""

print_hex  = False
"""
Print a hex representation of each message if true.
"""


def multi_writeln(str):
    """
    Write str, followed by a newline, to all elements of
    output_streams[].
    """
    global output_streams
    for stream in output_streams:
        stream.write(str)
        stream.write("\n")
        

def print_msg_str(data, count):
    """
    Print the printabled bytes of a message as a string.
    """
    multi_writeln("S:%6d: %s"%(count,pylib_generic.asciify_str(data)))


def print_msg_hex(data, count):
    """
    Print the printabled bytes of a message as a string.
    """
    multi_writeln("H:%6d: %s"%(count, binascii.hexlify(data)))


def print_msg_data(data, count, sender):
    """
    Print msg meta-data: Time, length, sender
    """
    timestr = ("%14.6f" % (time.time()))[-14:]
    multi_writeln("D:%6d: %s s.a.e.:%4d bytes:%s" %
                  (count,timestr,len(data), repr(sender)))

    
def print_message(data, count, sender):
    """
    Print message data as described by the print flags.
    """

    if bin_output:
        bin_output.write(data)
    if print_data:
        print_msg_data(data, count, sender)
    if print_str:
        print_msg_str(data, count)
    if print_hex:
        print_msg_hex(data, count)



def process_options(optlist):
    """
    Process options.
    """
    global output_streams
    global bin_output
    global print_data
    global print_str
    global print_hex
    
    for opt, arg in optlist:
        if opt == "-h":
            print __doc__
            sys.exit()
        elif opt == "-v" or opt == "--verbose":
            pylib_io.Verbose = 1
        elif opt=="-f":
            fp = pylib_io.flexopen(arg, "wb")
            output_streams.append(fp)
        elif opt=="-b":
            bin_output = pylin_io.flexopen(arg, "w")
        elif opt == "-d":
            print_data = True
        elif opt == "-s":
            print_str = True
        elif opt == "-H":
            print_hex = True
        else:
            print __doc__
            usage_error("Unknown option")


if __name__ == '__main__':

    opts, args = getopt.gnu_getopt(sys.argv[1:], "hf:b:dsvH", ["Verbose"])

    process_options(opts)    

    if len(args)!=1:
        print __doc__
        sys.exit()

    port = int(args[0])

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0);
    sock.bind( ("", port) )
    count = 0
    
    while True:
        (data, sender) = sock.recvfrom(16384)        
        msgs = print_message(data, count, sender)
        count = count+1
