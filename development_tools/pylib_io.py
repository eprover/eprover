#!/usr/bin/env python2.7
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
import os

NoStdRWStreamException = BaseException("You cannot open '-' for both reading and writing!")
UsageErrorException = BaseException("Usage Error")

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
    name = os.path.expanduser(name)
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

def clean_list(l):
    """
    Given a list of strings, return the list stripped, and with empty
    lines and comment lines (sarting in #) removed.
    """
    res = map(lambda x:x.strip(), l)
    res = filter(lambda x:not ((x.startswith("#") or x=="")), res);
    return res


def read_real_lines(fp):
    """
    As fp.readlines(), but strip newlines, empty lines, comment lines.
    """
    tmp = fp.readlines()
    return clean_list(tmp)


def parse_lines(name):
    """
    Read a file and return a list of stripped, non-comment, nonempty
    lines.
    """
    fp = flexopen(name, "r")
    res = read_real_lines(fp)
    flexclose(fp)

    return res

def decode_wait_status(status):
    """
    Given a 16 bit status as returned by wait, decode it into a tuple
    (status, signal, core).
    """
    signal = status % 128
    exit_status = status / 256
    if status%256/256 == 1:
        core = True
    else:
        core = False

    return (status, signal, core)


def run_shell_command(cmd):
    """
    Run a local shell command and return the output (or None if an
    error occured).
    """
    fp = os.popen(cmd)
    res = fp.readlines()
    status = fp.close()
    if status:
        print "# Warning: '"+cmd+"' returned status "+\
              repr(decode_wait_status(status)[0])

    return res
