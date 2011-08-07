#!/usr/bin/env python2.6

"""
fp_eval.py 0.1

Usage: fp_eval.py <file1> ... 

Read a number of E protocols with extended stats, and do statistical
evalualtion of them.

Options:

-h Print this help.

Copyright 2011 Stephan Schulz, schulz@eprover.org

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
import pylib_io
import pylib_maths
import pylib_eprot


def find_common_successes(prots):
    res = prots[0].get_successes()
    for i in prots[1:]:
        res.intersection_update(i.get_successes())    
    return res
    
def filter_common_successes(prots):
    succs = find_common_successes(prots)
    for i in prots:
        i.filter(succs)
    return prots

def res_hash(res):
    """
    Return a hash of the result parameters such that an equal hash is
    good evidence for very similar proof search.
    """
    return res.values[4], res.values[9]

def find_common_hash(prots, hashfun):
    """
    Return a list of those problems with the same hash result in all
    the protocols.
    """
    res = []
    for i in prots[0].results.values():
        hash = hashfun(i)
        name = i.name()
        for prot in prots[1:]:
            entry = prot.result(i.name())
            nhash = hashfun(entry)
            if hash != nhash:
                name = None
                break
        if name:
            res.append(name)
    return res


def filter_common_search(prots):
    common = find_common_hash(prots, res_hash)
    for i in prots:
        i.filter(common)
    return prots
    


times = [
    (2 , "Run time"),
    (12, "Sat time"),
    (13, "PM  time"),
    (14, "PMI time"),
    (11, "MGU time"),
    (15, "BR  time"),
    (16, "BRI time")]   
    

def profile_analysis(prot, profile_selector):
    res =  map(lambda x:pylib_maths.sum([float(i.values[x[0]]) \
                                         for i in prot.results.values()]),\
               profile_selector)
    return prot.name, res

def table_line(prot_data):
    res = "%-12s "%(prot_data[0][10:],)
    for i in prot_data[1]:
        res = res+"& %16.3f "%(i,)
    res = res+" \\\\\n"
    return res

def table_header(times):
    line1 = "\\begin{tabular}{l"
    line2 = "\\textbf{Name}"
    for i in times:
        line1=line1+"|r"
        line2=line2+"& %-16s"%("\\textbf{"+i[1]+"}",)
    res = line1+"}\n"+line2+"\\\\\n"
    return res

def table_lines(protocols, times):
    res = map(lambda x:table_line(profile_analysis(x, times)), protocols)
    return "".join(res)


def do_table(prots, times):
    res = table_header(times)+"\hline\n"+\
          table_lines(prots, times)+\
          "\hline\n\end{tabular}"
    return res
    

if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    if not args: 
        print __doc__
        sys.exit("No argument file given")
       

    prots = []

    for i in args:
        fp = pylib_io.flexopen(i, "r")
        print "Parsing: ",i 
        prot = pylib_eprot.eprot(i)
        prot.parse();        
        pylib_io.flexclose(fp)
        prots.append(prot)
        
    
    filter_common_successes(prots)        
    filter_common_search(prots)
    
    print do_table(prots, times)
    
    
