#!/usr/bin/env python

"""
include_mapper.py 0.1

Usage: include_mapper.py <file1> ... 

Determine and print the include hierarchy for each of the given files.

Options:

-h Print this help.

Copyright 2008-2011 Stephan Schulz, schulz@eprover.org

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


Lower_word = re.compile("[a-z][a-z0-9_]*")
include_re = re.compile("include\(.*\)\.")
sq_re      = re.compile("'.*'")


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
    def __init__(self, str, refdir = None):
        mo = sq_re.search(str)        
        self.name = mo.group()[1:-1]
        self.refdir = refdir

    def __str__(self):
        inc1 = "include('"+self.name+"')."
        return inc1

    def __repr__(self):
        return self.__str__()
        

def find_tptp_file(filename, refdir=None):
    """
    Find a TPTP file and open it. Return filepointer, directory of
    file, or none of opening fails.
    """

    if os.path.isabs(filename) or filename == "-":        
        try:
            fp = pylib_io.flexopen(filename, "r")
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
   


def find_includes(filename, refdir = None, maxread=50000):
    try:
        fp,refdir = find_tptp_file(filename, refdir)
    except Exception,inst:
        print inst
        raise Exception("Cannot open file: "+filename)

    file = fp.read(maxread)
    
    
    pylib_io.flexclose(fp)
    res = list([])
    start = 0
    mo = include_re.search(file, start)
    while mo:
        start = mo.end()
        inc  = mo.group()
        tmpinc = tptp_include(inc, refdir)
        res.append(tmpinc)
        mo = include_re.search(file, start)
        
    return res


def find_all(files):
    res = []
    while files:
        first = files.pop(0)
        res.append(first)
        rest  = files[1:]
        new = find_includes(first[0], first[1])
        tmp = [(i.name, i.refdir) for i in new]
        files.extend(tmp)
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

    files = [(i, None) for i in filenames]
    res = find_all(files)
    for i in res:
        print "%-50s %s"%i
