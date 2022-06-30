#!/usr/bin/env python

"""
tptp_analyse.py 0.1

Usage: tptp_analyse.py [options] <prob_file> ...

Parse a list of TPTP file name occurrences and File lines and generate
an overview of which problem is in which version of TPTP.

Options:

-h
   Print this help.

-s <sourcefile>
  Parse sourcefile and obtain problem statistics from the given
  sources.

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
import getopt
import pylib_io
from pylib_maths import *


def tptp_rep(sample, element):
    if element in sample:
        return element
    else:
        return "  -   "

def header_extract_int(line):
    """
    Given a TPTP header syntax line, extract the numerical value.
    """
    line = line[14:]
    tmp = line.split(":")
    res = int(tmp[1].split()[0])
    return res



class problem_desc(object):
    def __init__(self, filename):
        fp = pylib_io.flexopen(filename, "r")
        desc = fp.readlines(4096)
        pylib_io.flexclose(fp)
        
        self.specsize = 0
        self.atomno   = 0
        self.predno   = 0
        self.funcno   = 0

        if "+" in filename:
            self.fof = True            
        else:
            self.fof = False

        self.filename = filename
        self.name = filename.split("/")[-1]
            
        for i in desc:
            pos = i.find("Number of clauses")
            if pos != -1:
                self.specsize = header_extract_int(i)
            pos = i.find("Number of formulae")
            if pos != -1:
                self.specsize = header_extract_int(i)

            pos = i.find("Number of literals")
            if pos != -1:
                self.atomno = header_extract_int(i)
            pos = i.find("Number of atoms")
            if pos != -1:
                self.atomno = header_extract_int(i)

            pos = i.find("Number of predicates")
            if pos != -1:
                self.predno = header_extract_int(i)
            pos = i.find("Number of functors")
            if pos != -1:
                self.funcno = header_extract_int(i)

    def __str__(self):
        return ("(%s:%d, %d, %d, %d)")%(self.name, self.specsize,
                                        self.atomno, self.predno,
                                        self.funcno)
           

            


class tptp_store(object):
    """
    Represent data about the problems in different versions of the
    TPTP. 
    """
    def __init__(self):
        self.problems = {}
        self.tptps = set()
        self.prob_names = None
        self.prob_desc = {}

    def prob_list(self):
        if not self.prob_names:
            self.prob_names = self.problems.keys()
            self.prob_names.sort()
        return self.prob_names
            

    def add(self, line):
        print line
        tmp = line.split("%")
        probdesc = tmp[0]
        tmp = probdesc.split("/")
        print tmp
        tptp = tmp[1][5:]
        prob = tmp[4][:-1].replace(":", ".")

        if not prob in self.problems:
            self.problems[prob] = set()
            self.prob_names = None

        self.problems[prob].add(tptp)            
        self.tptps.add(tptp)


    def prob_rep(self, order, prob):
        tptps = self.problems[prob]
        presence_rep = map(lambda x:tptp_rep(tptps,x), order)
        presence_rep.insert(0, prob)
        return " ".join(presence_rep)

    def __str__(self):
        order = list(self.tptps)
        order.sort()

        replist = [self.prob_rep(order, prob) for prob in self.prob_list()]
        replist.append("")
        return "\n".join(replist)

    def is_present(self, problem, tptp):
        try:
            return tptp in self.problems[problem]
        except KeyError:
            return False

    def extract(self, tptp):
        """
        Return the list of all problems in the given TPTP.
        """
        return filter(lambda x:self.is_present(x, tptp), self.prob_list())

    def tptps_list(self):
        """
        Return an ordered list of TPTPs represented in the store.
        """
        res = list(self.tptps)
        res.sort()
        return res

        
    def parse_prob_data(self, file):
        fp = pylib_io.flexopen(file, "r")
        for line in fp:
            tmp = problem_desc(line[:-1])
            self.prob_desc[tmp.name] = tmp
        pylib_io.flexclose(fp)
        
    def tptp_name(self, tptp):
        return tptp
    
    def prob_no(self, tptp):
        return len(self.extract(tptp))

    def fof_proportion(self, tptp):
        probs = filter(lambda x:self.prob_desc[x].fof, self.extract(tptp))
        return round(float(len(probs))/self.prob_no(tptp), 2)

    def ax_no(self, tptp):
        l = [self.prob_desc[x].specsize for x in self.extract(tptp)]
        return round(mean(l), 2)

    def atom_no(self, tptp):
        l = [self.prob_desc[x].atomno for x in self.extract(tptp)]
        return round(mean(l), 2)

    def pred_no(self, tptp):
        l = [self.prob_desc[x].predno for x in self.extract(tptp)]
        return round(mean(l), 2)

    def func_no(self, tptp):
        l = [self.prob_desc[x].funcno for x in self.extract(tptp)]
        return round(mean(l), 2)


    def stats(self, fname, feature_extractor):
        res = [fname]
        for i in self.tptps_list():
            res.append(feature_extractor(i))
        return res
        
        
        

if __name__ == '__main__':

    parse_data = None
    
    
    opts, args = getopt.gnu_getopt(sys.argv[1:], "hvs:", ["verbose"])    
    
    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        elif option == "-v" or option =="--verbose":
            pylib_io.Verbose = 1
        elif option == "-s":
            parse_data = optarg 
        else:
            sys.exit("Unknown option "+ option)

    if len(args)<1:
        print __doc__
        sys.exit()

    store = tptp_store()
        
    for arg in args:
        fp = pylib_io.flexopen(arg, "r")
        for line in fp:
            store.add(line)
        pylib_io.flexclose(fp)

    if parse_data:
        store.parse_prob_data(parse_data)

    version  = store.stats("<strong>Version</strong>",  store.tptp_name)
    problems = store.stats("<strong>Problems</strong>", store.prob_no)
    fof_prop = store.stats("<strong>FOF prop</strong>", store.fof_proportion)
    ax_size  = store.stats("<strong>Axioms</strong>",   store.ax_no)
    at_size  = store.stats("<strong>Atoms</strong>",    store.ax_no)
    pr_size  = store.stats("<strong>Preds</strong>",    store.pred_no)
    fu_size  = store.stats("<strong>Funcs</strong>",    store.func_no)

    res = zip(version, problems, fof_prop, ax_size, at_size, pr_size, fu_size)

    for line in res:
        print "<tr>\n  "
        for i in line:
            print "<td>", i, "</td>",
        print "\n<tr>"
        
