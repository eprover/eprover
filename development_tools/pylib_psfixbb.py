#!/usr/bin/env python2.7
# ----------------------------------
#
# pylib_psfixbb.py 1.1
#
# Set tight bounding boxes for PostScript files. The basic idea comes from
# fixbb version 0.31 by Jeff Spirko.
#
# To use as a standalone program, safe as psfixbb.py or just psfixbb.
#
# Version history:
# 1.0 Wed Feb 16 20:24:59 CET 2005
#     First verson released
# 1.1 Wed Feb 16 20:24:59 CET 2005
#     Improved error handling, speedups 

"""
pylib_psfixbb.py 1.1

Usage: psfixbb [Options] <file> ...

Set tight bounding boxes for encapsulated (and some other) PostScript
files. 

Many programs will generate PostScript with bounding boxes that are
much larger than necessary. psfixbb will use the ghostscript
interpreter to determine a tight bounding box for the PostScript files
given on the command line and will change the first bounding box in
each file accordingly. For multi-page files with multiple bounding
boxes, the new bounding box will fit all pages.

psfixbb will not try to insert a bounding box into a file that does
not already have one. In that case it will leave the original file
unmodified.

Restriction: For simplicity, psfixbb will read the whole file into
memory. This should not be a problem with current memory sizes and
typical PostScript files. If your files are larger than your swap
space, you are in trouble.

Options:

-h
 Print this information and exit.

Copyright 2005 Stephan Schulz, schulz@eprover.org

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


# If necessary, change this to point to your gs executable!

gs_command = "gs -dQUIET -dBATCH -dNOPAUSE -sDEVICE=bbox"

ws_pattern = re.compile("\s+")
bb_found_exception = "Found Bounding Box"
bb_error_exception = "Could not find Bounding Box"
bb_string = "%%BoundingBox:"

def get_options(argv=sys.argv[1:]):
    """
    Filter argument list for arguments starting with a -.
    """
    options = filter(lambda x:x[0:1]=="-", argv)
    return options

def get_args(argv=sys.argv[1:]):
    """
    Filter argument list for real arguments.
    """
    files   = filter(lambda x:x[0:1]!="-", argv)
    return files


           
def gs_getbb(filename):
    """
    Try to get a bounding box for a PostScript file from gs (which
    should be in the search path!
    """
    p = os.popen3(gs_command+" "+filename, "r")[2];
    gsres = p.readlines()
    p.close()

    res = None
    new_bb = None

    try:
        for line in gsres:
            if line.startswith(bb_string):
                tmp = ws_pattern.split(line)
                if len(tmp)!=6:
                    raise bb_error_exception
                new_bb = map(lambda x:int(x), tmp[1:-1])            
                if res:
                    res[0]=min(res[0], new_bb[0])
                    res[1]=min(res[1], new_bb[1])
                    res[2]=max(res[2], new_bb[2])
                    res[3]=min(res[3], new_bb[3])
                else:
                    res = new_bb
        if not res:
            raise bb_error_exception

    except bb_error_exception:        
        sys.exit("Could not get bounding box for " +filename+\
                 " from GhostScript")
        
    return "%%BoundingBox: "+string.join(map(lambda x:str(x), res))+"\n"


def replace_bb(file,bb):
    """
    Replace the first BoundingBox in file with bb. Do nothing (but
    print a warning) if no Bounding Box is found.
    """

    fh = open(file, "r")
    ps = fh.readlines()
    fh.close()

    try:
        for i in xrange(len(ps)):
            if ps[i].startswith(bb_string):
                ps[i] = bb
                raise bb_found_exception
        sys.stderr.write("Warning: Could not find bounding box in "\
                         +file+", file unmodified.\n")
    except bb_found_exception:
        fh = open(file, "w")
        for l in ps:
            fh.write(l)
        fh.close()


def fixbb(file):
    """
    Replace the first BB in file with a tighly fitting one.
    """
    bb = gs_getbb(file)
    replace_bb(file,bb)

if __name__ == '__main__':
    for option in get_options():
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    for file in get_args():
        fixbb(file)
