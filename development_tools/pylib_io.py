#!/usr/bin/env python2.3
# ----------------------------------
#
# Module pylib_io
#
# Functions for handling io, in particular to transparently support
# the use of - for stdin/stdout.
# Copyright 2003,2004 Stephan Schulz, schulz@eprover.org
#
# This code is part of the support structure for the equational
# theorem prover E. Visit
#
#  http://www.eprover.org
#
# for more information.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program ; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA  02111-1307 USA 
#
# The original copyright holder can be contacted as
#
# Stephan Schulz (I4)
# Technische Universitaet Muenchen
# Institut fuer Informatik
# Boltzmannstrasse 3
# Garching bei Muenchen
# Germany
#
# or via email (address above).

import sys

NoStdRWStreamException = "You cannot open '-' for both reading and writing!"
UsageErrorException = "Usage Error"

def flexopen(name, mode):
    """
    Open a file or stdin/out for reading. Uses the convention that a
    dash represents stdin/out.
    """
    if(name == "-"):
        if(mode == "r"):
            return sys.stdin;
        elif(mode == "w"):
            return sys.stdout
        else:
            raise NoStdRWStreamException
    return open(name, mode)

def flexclose(file):
    """
    "Closes" a file object. If stdin/out, flushes it, otherwise  calls
    close on it.
    """
    if((file == sys.stdout) or (file == sys.stderr)):
        file.flush()
    else:
        file.close()

def check_argc(argmin,argmax=None,argv=sys.argv[1:]):
    """
    Print if the required number of arguments has been given, print an
    error message, if not.
    """
    if (argmin and (len(argv)<argmin)) or (argmax and (len(argv)>argmax)):
        if(argmin==1):
            sys.stderr.write("Usage: "+sys.argv[0]+" <arg>\n")
        elif(argmin==2):
            print "Usage: "+sys.argv[0]+" <arg1> <arg2>"
        elif argmax==argmin:
            sys.stderr.write("Usage: " + sys.argv[0]+
                             " <arg1> ... <arg" + repr(argmin) + ">\n")
        elif argmax:
            sys.stderr.write("Usage: " + sys.argv[0]+
                             " <arg1> ... <arg" + repr(argmin) +
                             "> [... <arg" + repr[argmax] + ">]\n")
        else:
            sys.stderr.write("Usage: " + sys.argv[0]+
                             " <arg1> ... <arg" + repr(argmin) + ">...\n")    
        raise UsageErrorException;


        
def get_options(argv=sys.argv[1:]):
    """
    Filter argument list for arguments starting with a -.
    """
    options = filter(lambda x:x[0:1]=="-" and x!="-", argv)
    return options

def get_args(argv=sys.argv[1:]):
    """
    Filter argument list for real arguments.
    """
    files   = filter(lambda x:x[0:1]!="-" or x=="-", argv)
    return files


           
        
