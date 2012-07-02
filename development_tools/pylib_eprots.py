#!/usr/bin/env python2.7
# ----------------------------------
#
# Module pylib_eprots
#
# Functions (and classes) for manipulating E result protocols (this is
# probably fairly specialized and not that useful for others...
#
# Copyright 2003-2005 Stephan Schulz, schulz@eprover.org
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

import re
import string
import sys
from UserList import UserList

import pylib_basics
import pylib_io
import pylib_discretize


InfiniteTime = 1000000000
EmptyProtSetException = "Cannot get required information from empty protocol set"
train_number_re = re.compile("_[0-9]*\.")


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
        return (parts[0],parts[1],parts[2], parts[3])
    return None

class process_line:
    """
    Class of functional objects converting one E protocol line into
    another by rounding the time as specified by a rounding function.
    """
    def __init__(self, round_fun):
        self.round_fun = round_fun;

    def __call__(self,line):
        print line
        if line[0] == "#":
           return line
        parts = string.split(line, None, 4)
        return "%-29s %s %8.3f %-10s %85s" % (parts[0], parts[1],
                                            self.round_fun(float(parts[2])),
                                            parts[3],parts[4])


def compare_entries(entry1, entry2):
    """
    Compare two tuples of the form (status, time, reason) so that
    failures are smaller than successes and better times are smaller
    than larger ones.
    """
    res = cmp(entry1[0], entry2[0])
    if res!=0:
        return -1*res;
    res = cmp(entry1[1], entry2[1])
    if res!=0:
        return res;
    return cmp(entry1[2], entry2[2])
    

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

    def insert_entry(self, entry, state, time, reason):
        self.data[entry] = (state, time, reason)
        self.entries += 1
        if state == "T":
            self.proofs    += 1
            self.successes += 1
            self.succ_time += time
        elif state == "N":
            self.models    += 1
            self.successes += 1
            self.succ_time += time

    def insert_line(self, line):
        tmp = get_relevant_parts(line)
        if tmp:
            self.insert_entry(tmp[0], tmp[1], tmp[2], tmp[3]);
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


    def filter(self, filter_re):
        """
        Return a new protocol with the same name containing just
        problems where the name matches the filtering regexp.
        """
        res = eprotocol(None, self.silent);
        iter_list = self.data.keys()
        iter_list.sort()
        for i in iter_list:
            if filter_re.search(i):
                l_entry = self.data[i]
                res.insert_entry(i, l_entry[0], l_entry[1], l_entry[2]);
        return res


    def collect_sample(self):
        """
        Consider names to consist of a class part and a running number
        of the form '_XXX' directly before the suffix. Return a
        dictionary associating every class with a list of all
        results.
        """
        res = {}

        for i in self.data.keys():
            entry = self.data[i]
            mo = train_number_re.search(i);
            key = i[:mo.start()];
            try:
                res[key].append(entry)
            except KeyError:
                res[key] = [];
                res[key].append(entry)
        for i in res.keys():
            res[i].sort(compare_entries);
        return res

    def collect_medians(self):
        """
        For each class, compute the median value. Return  a sorted
        list of class/value tuples. If the median is no sucess,
        generate no pair!
        """
        tmp = self.collect_sample()
        res = [];
        for i in tmp.keys():
            entry = tmp[i];
            value = entry[int((len(entry)-1)/2)];
            if value[0]!='F':
                res.append( (i, value[1]));
        res.sort();
        return res;

    def collect_times(self, success_only=False):
        """
        Return a list of the run times for all problems (or all
        successfully solved problems) in the protocol.
        """
        if success_only:
            return [i[1] for i in self.data.values()  if i[0]!='F']
        else:
            return [i[1] for i in self.data.values()]

    
    def repr_entry(self, key):
        """
        Return a representation of the entry for a single key.
        """
        tmp = self.data[key];
        return "%-38s %s %8.3f  %s" % (key, tmp[0], tmp[1], tmp[2])

    def __repr__(self):
        res = self.comments;
        res +="""# Proofs:    %5d
# Models:    %5d
# Successed: %5d
# Time:      %-8.3f
""" % (self.proofs, self.models, self.successes, self.succ_time)
        tmpkeys = self.data.keys()
        tmpkeys.sort()
        for i in tmpkeys:
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
            state, time = self.data[problem];
            if state == "F":
                return (InfiniteTime, self.successes, self.succ_time)
            else:
                return (round_fun(time), self.successes, self.succ_time)
        except KeyError:
            return (InfiniteTime, self.successes, self.succ_time)

    def get_status(self, problem):
        (state, time) = self.data[problem]
        return state

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
        self.hash = {}
        
    def append(self, new):
        if len(new)!=3:
            raise TypeError
        UserList.append(self,new)
        self.hash[new[0]] = new[2]

    def printout(self, prefix = ""):
        self.sort()
        for i in self:
            print "%s%-29s : %s : %s" % (prefix, i[0], i[1], i[2])

    def parse(self, file):
        f = pylib_io.flexopen(file,"r")
        l = f.readlines()
        pylib_io.flexclose(f)
        for i in l:
            if i.startswith("#"):
                continue
            tmp = string.split(i, ":");
            if len(tmp) == 4:
                del(tmp[0])
            tmp = map(string.strip, tmp)
            self.append((tmp[0], tmp[1], tmp[2]))
    
    def classify(self, prob):
        return self.hash[prob]
    

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
            res     = self.protlist[0]
            reseval = (InfiniteTime, 10000000, 0)
        except IndexError:
            raise EmptyProtSetException
        for i in self.protlist:
            eval = i.eval_problem(problem, round_fun)
            if eval_is_better(eval, reseval):
                res = i
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
            prot = self.find_class(i,round_fun)
            status = prot.get_status(i)
            res.append((i, status, prot.name))
        return res


class featurelist(UserList):
    def __init__(self, data=[]):
        UserList.__init__(self)
        for i in data:
            self.append(i)

    def parse(self, file):
        f = pylib_io.flexopen(file,"r")
        l = f.readlines()
        pylib_io.flexclose(f)
        for i in l:
            if i.startswith("#"):
                continue
            tmp = string.split(i, ":");
            name = tmp[0].strip()
            featurestring = (tmp[1].strip())[1:-1]
            features = map(string.strip,string.split(featurestring, ","))
            if len(tmp) == 3: # Old style features
                add_features = tmp[2].strip()
                features.append(add_features[0])
                features.append(add_features[1])
                features.append(add_features[2])
                features.append(add_features[4])
                features.append(add_features[9])
            self.append((name, features))
    def printout(self):
        self.sort()
        for i in self:
            print i[0],":", string.join(i[1],",")




