#!/usr/bin/env python2.2
# ----------------------------------
#
# Module pylib_eprots
#
# Functions (and classes) for manipulatig E result protocols (this is
# probably fairly specialized and not that useful for others...
#
# Copyright 2003 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# This code is part of the support structure for the equational
# theorem prover E. Visit
#
#  http://www4.informatik.tu-muenchen.de/~schulz/WORK/eprover.html 
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

import re
import string
import sys
from UserList import UserList

import pylib_basics
import pylib_io
import pylib_discretize


InfiniteTime = 1000000000
EmptyProtSetException = "Cannot get required information from empty protocol set"

def break_prot_line(line):
    """
    Take an E protocoll line and convert it into an array of values of
    the appropriate type (string, float or int) that is returned.
    """
    if line[0] == "#":
        return None
    parts = string.split(line, None)
    for i in range(0, len(parts)):
        try:
            p = int(parts [i])
        except ValueError:
            try:
                p = float(parts[i])
            except ValueError:
                p = parts[i]
        parts[i] = p
    return parts            


def get_relevant_parts(line):
    """
    Accept an E protocol line and return a tuple of the three relevant
    elements: Problem name, status, and time.
    """
    parts = break_prot_line(line)
    if parts:
        return (parts[0],parts[1],parts[2])
    return None

class process_line:
    """
    Class of functional objects converting one E protocol line into
    another by rounding the time as specified by a rounding function.
    """
    def __init__(self, round_fun):
        self.round_fun = round_fun;

    def __call__(self,line):
        if line[0] == "#":
           return line
        parts = string.split(line, None, 4)
        return "%-29s %s %8.3f %-10s %85s" % (parts[0], parts[1],
                                            self.round_fun(float(parts[2])),
                                            parts[3],parts[4])


class eprotocol:
    """
    Data type for storing (the relevant parts of) a standard E test
    protocol.
    """
    def __init__(self, name=None, silent=False):
        self.name      = "Unknown"
        self.data      = {}
        self.proofs    = 0
        self.models    = 0
        self.successes = 0
        self.succ_time = 0.0
        self.entries   = 0
        self.comments  = ""
        self.silent    = silent
        if name:
            self.parse(name)

    def __cmp__(self, other):
        """
        Comparison of two protocols. Smaller is better, i.e. more
        successes or same number of successes in less time.
        """
        tmp = other.successes - self.successes
        if tmp!=0:
            return tmp
        tmp = self.succ_time- other.succ_time
        return pylib_basics.sign(tmp)

    def insert_line(self, line):
        tmp = get_relevant_parts(line)
        if tmp:
            entry = tmp[0]
            state = tmp[1]
            time  = tmp[2]
            self.data[entry] = (state, time)
            self.entries += 1
            if state == "T":
                self.proofs    += 1
                self.successes += 1
                self.succ_time += time
            elif state == "N":
                self.models    += 1
                self.successes += 1
                self.succ_time += time
        else:
            self.comments += line

    def parse(self, file):
        if not self.silent:        
            sys.stderr.write("Parsing "+file+".\n")
        f = pylib_io.flexopen(file,'r')
        l = f.readlines()
        pylib_io.flexclose(f)
        self.name = file
        for line in l:
            self.insert_line(line)

    def repr_entry(self, key):
        """
        Return a representation of the entry fot a single key.
        """
        tmp = self.data[key];
        return "%-29s %s %8.3f" % (key, tmp[0], tmp[1])

    def __repr__(self):
        res = self.comments;
        print self.succ_time
        res +="""# Proofs:    %5d
# Models:    %5d
# Successed: %5d
# Time:      %-8.3f
""" % (self.proofs, self.models, self.successes, self.succ_time)
        for i in self.data.keys():
            res += self.repr_entry(i)
            res += "\n"
        return res;
        
    def eval_problem(self, problem, round_fun=pylib_discretize.no_round):
        """
        Return a tuple (time, solutions, succ_time), where time is the
        time for a successful proof attempt at problem (or
        InfiniteTime for failure), solutions is the number of
        solutions in the protocol, and succ_time is the sum of all
        solution times in the protocol. Unknown problems are treated
        as failures.
        """
        try:
            state, time = self.data[problem]
            if state == "F":
                return (InfiniteTime, self.successes, self.succ_time)
            else:
                return (round_fun(time), self.successes, self.succ_time)
        except KeyError:
            return (InfiniteTime, self.successes, self.succ_time)


def eval_is_better(e1, e2):
    """
    Return true if e1 is better than e2
    """
    if e1[0] < e2[0]:
        return True
    elif e1[0] == e2[0]:
        if e1[1] > e2[1]:
            return True
        elif e1[1] == e2[1]:
            if e1[2] > e2[2]:
                return True
    return False



class classification(UserList):
    def __init__(self, data=[]):
        UserList.__init__(self)
        for i in data:
            self.append(i)

    def append(self, new):
        if len(new)!=2:
            raise TypeError
        UserList.append(self,new)        

    def printout(self, prefix = ""):
        self.sort()
        for i in self:
            print "%s%-29s : %s" % (prefix, i[0], i[1])


class eprot_set:
    """
    Class for storing an arbitrary number of E protocols and
    answering interesting questions about them.
    """    
    def __init__(self, names=[]):
        self.protlist = []
        self.sorted = True;
        self.parse(names)        

    def insert(self,prot):
        self.sorted = False;
        self.protlist.append(prot)        

    def parse(self, names, silent=False):
        for name in names:
            prot = eprotocol(name, silent)
            self.insert(prot)

    def __repr__(self):
        res = ""
        sep = "["
        for i in self.protlist:
            res += sep
            res += i.name
            sep = "\n "
        return res+"]"

    def sort(self):
        if not self.sorted:
            self.protlist.sort()

    def find_class(self, problem, round_fun=pylib_discretize.no_round):
        self.sort()
        try:
            res     = self.protlist[0].name+"(Default)"
            reseval = (InfiniteTime, 10000000, 0)
        except IndexError:
            raise EmptyProtSetException
        for i in self.protlist:
            eval = i.eval_problem(problem, round_fun)
            if eval_is_better(eval, reseval):
                res = i.name
                reseval = eval
        return res

    def make_classification(self,round_fun=pylib_discretize.no_round):
        self.sort()
        try:
            source = self.protlist[0]
        except IndexError:
            raise EmptyProtSetException
        res = classification()
        for i in source.data.keys():
            res.append((i, self.find_class(i,round_fun)))
        return res

    



