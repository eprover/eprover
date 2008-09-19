#!/usr/bin/env python2.5

"""
include_mapper.py 0.1

Usage: include_mapper.py <file1> ... 

Determine and print the include hierarchy for each of the given files.

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
import getopt
import os
import os.path
import pylib_io
import pylib_generic


lower_word = re.compile("[a-z][a-z0-9_]*")

class tptp_atomic_word(object):
    """
    Represent a fucking TPTP "lower_word" that has reached the syntax
    via unholy inheritence from Prolog and needs unreasonable amounts
    of coddling whereever it occurs. Grrr!
    """
    def __init__(self, value):
        mo = lower_word.match(value)
        if mo:
            self.value = value
            self.quote = False
        else:
            pass
            
        


class tptp_include(object):
    """
    Represents a TPTP include statement, with filename and optional
    list of included axioms.
    """
    def __init__(self, file, selection = None):
        self.file = file
        self.selection = selection

    def __str__(self):
        inc1 = "include('"+self.file+"'"
        if self.selection:
            inc2 = "["+",".join(self.selection)+"]"
        else:
            inc2 = ""
        return inc1+inc2+")."
        

def find_tptp_file(filename, refdir=None):
    """
    Find a TPTP file and open it. Return filepointer, directory of
    file, or none of opening fails.
    """
    print "Tring to open", filename
    print "os.path.isabs():", os.path.isabs(filename)

    if os.path.isabs(filename) or filename == "-":        
        print "Here we are"
        try:
            fp = pylib_io.flexopen(filename, "r")
            print "Open ok"
            return (fp, pylib_io.get_directory(filename))
        except Exception, inst:
            print inst
            return None
    # We don't yet know where to search
    if not refdir:
        # Consider filename relative to local dir
        try:
            fp = pylib_io.flexopen(filename, "r")
            return (fp, pylib_io.get_directory(filename))
        except:
            pass
    else:
        # We have a reference directory and cannot use local dir
        name = os.path.join(refdir, filename)
        try:
            fp = pylib_io.flexopen(name, "r")
            return (fp, pylib_io.get_directory(name))
        except:
            pass
    # Everything failed, try looking at $TPTP
    refdir = os.getenv("TPTP")
    name = os.path.join(refdir, filename)
    try:
        fp = pylib_io.flexopen(name, "r")
        return (fp, pylib_io.get_directory(name))
    except:
        return None

    
   


def find_includes(filename, refdir = None, maxread=20000):
    print "Tring to open", filename
    try:
        fp,refdir = find_tptp_file(filename, refdir)
    except Exception,inst:
        print inst
        raise Exception("Cannot open file: "+filename)

    file = fp.read(maxread)
    print file
    pylib_io.flexclose(fp)
    res = []
    return res



if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    filenames = args

    for i in filenames:
        print find_includes(i)
