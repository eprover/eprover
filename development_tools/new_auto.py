#!/usr/bin/env python2.7
#
# Usage: new_auto.py <feature_desc_file> <protocols>
#
# Read a list of problem names and (integer, real, or symbolic)
# features for problems, as well as a number of protocols containing
# results for different heuristics of E, and try to find a good
# assigment of heuristics to problem classes.
#
# Copyright 2002-2003 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# This program is part of the support structure for the equational
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
# Stephan Schulz
# Technische Universitaet Muenchen
# Institut fuer Informatik
# Boltzmannstrasse 3
# Garching bei Muenchen
# Germany
#
# or via email (address above).


# For older Python versions
from __future__ import nested_scopes
import sys
import re
import string
import math

white_space     = re.compile('\s+')
trail_space     = re.compile('\s*$')
arg_term        = re.compile('\s|$')
full_comment    = re.compile('^#')
dash            = re.compile('-')
slash           = re.compile('/')
match_heuristic = re.compile("-H'\(.*\)'")
match_class     = re.compile('CLASS_[A-Z-0-9]*$')
eval_f_sep      = re.compile('\),')
problem_ext     = re.compile('\.[a-z]*$')
feature_sep     = re.compile('[ :(),\n]+')

type_int    = type(1)
type_float  = type(1.5)
type_string = type("abc")

log_nat_2 =  math.log(2)

SetEmptyException = "SetEmptyException"
NoSuchPartitionException = "NoSuchPartitionException"


def comment_p(line):
    """Is a line a comment line?"""
    return line[0] == "#"

def uniq_list(l):
    """Return a list containing one copy of each different value in
    l. l is assumed to be sorted."""
    i = 0
    res = []
    limit = len(l)
    while i<limit:
        tmp = l[i]
        res.append(tmp)
        while i<limit and l[i] == tmp:
            i=i+1
    return res


def add(x,y):
    """Add two values with a mapable function"""
    return x+y

def eval_cmp(e1,e2):
    if e1[0] > e2[0]:
        return -1
    if e1[0] < e2[0]:
        return 1    
    if e1[1] > e2[1]:
        return 1
    if e1[1] < e2[1]:
        return -1
    return 0


class protocol:
    """Store a single result file"""
    def __init__(self):
        self.name = ""
        self.desc = ""
        self.probs = {}
        self.eval = None

    def parse(self, name):
        f = open(name, "r")
        lines = f.readlines()
        f.close()
        self.name = name
        self.desc = reduce(add, filter(comment_p, lines), "")
        lines = filter(lambda x: not comment_p(x), lines)
        lines = map(lambda x: string.split(x), lines)
        for i in lines:
            self.probs[i[0]] = i[1],string.atof(i[2])
        return self

    # For the future, Consider if it makes sense to allow incomplete
    # protocols and return failure for non-existent names!
    def result(self, prob):
        return self.probs[prob]

    def compute_eval(self):
        return reduce(result_add, map(self.result, self.probs.keys()), (0,0))

    def evaluate(self):
        if(not self.eval):
            self.eval=self.compute_eval()
        return self.eval

    def eval_set(self, set):
        return reduce(result_add, map(self.result, set), (0,0))        

    def __cmp__(self, other):
        e1 = self.evaluate()
        e2 = other.evaluate()
        return eval_cmp(e1,e2)

class protocolset:
    """Store a set of protocols (i.e. heuristics with results)"""
    def __init__(self):
        self.set = {}
        self.sorted = None

    def insert(self, prot):
        self.set[prot.name] = prot
        self.sorted = None

    def __repr__(self):
        return repr(self.set)
        
    def result(self,prot, prob):
        return self.set[prot].probs[prob]

    def success(self,prot,prob):
        return self.result(prot,prob)[0]!="F"

    def time(self,prot,prob):
        if self.success(prot,prob):
            return self.result(prot,prob)[1]
        return None

    def sort(self):
        if not self.sorted:
            prots = self.set.values()
            prots.sort()
            self.sorted = prots
        return self.sorted
        #return map(lambda x:x.name, prots)

    def optimal_prob(self, prob):
        """Find the optimal heuristic for a problem"""
        res = None
        t = 0
        for i in self.sort():
            tmp = i.result(prob)
            # print  tmp
            if tmp[0]!="F" and (not res or tmp[1]<t):
                res = i
                t = tmp[1]                          
        return res

    def canonical_prob(self, prob):
        """Find the most general heuristic that solves a problem"""
        for i in self.sort():
            tmp = i.result(prob)
            if tmp[0]!="F":
                return i
        return 0

    def optimal_set(self, set):
        """Find the best heuristic for a set of problems."""
        res = self.sort()[0]
        eval = res.eval_set(set)
        for i in self.sort()[1:]:
            tmp = i.eval_set(set)
            if eval_cmp(tmp, eval) < 0:
                res = i
                eval = tmp
        return res, eval[0], eval[1]

    def canonical_set(self, set):
        """Find the most  general heuristic that solves the same
           number of problems as the best one for a set of problems."""
        opt = self.optimal_set(set)
        solutions = opt[1]
        for i in self.sort():
            tmp = i.eval_set(set)
            if tmp[0] == solutions:
                return i, tmp[0], tmp[1]
        raise "Impossible state"
        
    def compute_total_solved(self, set):
        return filter(self.optimal_prob, set)
  
def atofeatureval(str):
    """Try to convert a string to an integer or a float,
       and return the converted value"""
    try:
        res=string.atoi(str)
        return res
    except ValueError:
        pass
    try:
        res=string.atof(str)
        return res
    except ValueError:
        pass
    return str


def typemax(t1,t2):
    """Given two types (string, int, float), return
       the more general one"""
    if t1 == type_string:
        return t1
    if t2 == type_string:
        return t2
    if t1 == type_float:
        return t1
    if t2 == type_float:
        return t2
    assert(t1==type_int and t2==type_int)
    return t1

def result_add(r1, r2):
    #print "R1|", r1, "|R1"
    #print "R2|", r2, "|R2"
    if r2[0]=='F':
        return r1
    else:
        return r1[0]+1, r1[1]+r2[1]

class featurelist:
    """Store problem feature list"""
    def __init__(self, name):
        self.features = {}
        self.featuretypes = []
        self.max_index = -1
        self.parse(name)
        self.featurerange = map(self.collect_feature_values,
                                range(0,self.max_index))
        
    def insert_line(self,l):
        # print l
        l = re.split(feature_sep,l);
        # print l
        features = map(atofeatureval, l[2:-1])
        self.features[l[0]] = features;
        if self.max_index == -1:
            self.max_index = len(features)-1;
            self.featuretypes = map(type,features)  
        else:
            if len(features)-1 != self.max_index:                
                raise "Inconsistent feature number"
            self.featuretypes = map(typemax,map(type,features),
                                    self.featuretypes)            
        
    def parse(self, name):
        f = open(name, "r")        
        lines = filter(lambda x: not comment_p(x), f.readlines())
        f.close()
        map(self.insert_line, lines)

    def collect_feature_values(self, feature):
        tmp = {}
        for i in self.features:
            tmp[self.value(i,feature)] = 1
        return tmp.keys()

    def collect_feature_list(self, feature, set):
        """Return the list of feature values for examples from set"""
        res = []
        for i in set:
            res.append(self.value(i,feature))
        return res

    def value(self, prob, feature):
        #print "Value:", prob, feature
        return self.features[prob][feature]

    def type(self, feature):
        return self.featuretypes[feature]

    def get_repr(self, prob):
        return prob+repr(self.features[prob])

    

class featuretest:
    """Define a test on a feature"""

    def __init__(self,data,feature):
        self.features = data
        self.feature = feature
        self.type = None

    def ftest(self, prob):
        raise "Base class does not implement feature test"

    def filter_set(self, set):
        return filter(self.ftest, set)

    def __repr__(self):
        return "Generic feature test "+repr(id(self))

class rangefeaturetest(featuretest):
    def __init__(self,data,feature):
        featuretest.__init__(self,data,feature)
        self.upper = None
        self.lower = None
        self.type = "Range"

    def set_limits(self, lower, upper):
        if self.features.type(self.feature)==type_int:
            try:
                upper = int(upper)
            except TypeError:
                pass
            try:
                lower = int(lower)
            except TypeError:
                pass            
        self.upper = upper
        self.lower = lower

    def get_limits(self):
        return self.lower, self.upper

    def ftest(self, prob):
        tmp = self.features.value(prob, self.feature)
        if(self.lower == None):
            return tmp < self.upper
        if(self.upper == None):
            return self.lower<=tmp        
        return (self.lower<=tmp) and (tmp < self.upper)

    def __repr__(self):
        return "Range["+repr(self.lower)+"<=X<"+repr(self.upper)+"]"

class setfeaturetest(featuretest):
    def __init__(self,data,feature):
        featuretest.__init__(self,data,feature)
        self.set = []
        self.type = "Set"
        
    def set_set(self, set):
        self.set = set

    def get_set(self):
        return self.set

    def ftest(self, prob):
        tmp = self.features.value(prob, self.feature)
        return tmp in self.set

    def __repr__(self):
        return "Set--X in "+repr(self.set)

class notsetfeaturetest(setfeaturetest):
    def __init__(self,data,feature):
        setfeaturetest.__init__(self,data,feature)
        self.type = "NotSet"
        
    def ftest(self, prob):
        tmp = self.features.value(prob, self.feature)
        return not (tmp in self.set)

    def __repr__(self):
        return "Set--X not in "+repr(self.test)

class partition:
     """Represent a partition as a dictionary from feature tests to
        lists of problem names"""

     def __init__(self, data):
         self.part = {}
         self.data = data

     def __repr__(self):
         return repr(self.data)

     def add_test(self, test, set):
         """Add a normal feature test (and problem set) to the partial
         partition."""
         tmp = test.filter_set(set)
         if len(tmp) == 0:
             raise SetEmptyException
         self.part[test] = tmp
         # print  test, self.part[test]

     def give_sets(self):
         return self.part.values()
         

     def add_elsetest(self, test, set):
         """Add an "else" feature test with empty problem set to the
         partial partition."""
         tmp = test.filter_set(set)
         assert(len(tmp)==0)
         self.part[test] = tmp
         # print  test, self.part[test]

     def make_equidist(self, feature, set, n):
         """Create a partition of cardinality n by splitting the
         feature space into n equally sized parts. Works only for
         numeric features."""

         assert(self.data.type(feature)==type_int or
                self.data.type(feature)==type_float)
         values = self.data.collect_feature_list(feature, set)
         values.sort()
         diff =  values[-1]-values[1]
         print values[1],values[-1]
         step = float(diff)/float(n)
         old = None
         i = values[1]+step
         while(i<values[-1]):
             try:
                 t = rangefeaturetest(self.data, feature)
                 t.set_limits(old, i)
                 self.add_test(t,set)
                 old = i
             except SetEmptyException:
                 pass
             i = i+step
         try:
             t1 = rangefeaturetest(self.data, feature)
             t1.set_limits(old, None)
             self.add_test(t1,set)
         except SetEmptyException:
             t.upper = None
             self.add_test(t,set)
                       

     def make_equicard(self, feature, set, n):
         """Create a partition of cardinality n by splitting the
         example space into n equally sized parts. Works only for
         numeric features."""

         assert(self.data.type(feature)==type_int or
                self.data.type(feature)==type_float)
         values = self.data.collect_feature_list(feature, set)
         values.sort()
         total = len(values)
         step = total/n
         old = None
         for i in range(step, step*n, step):
             try:
                 t = rangefeaturetest(self.data, feature)
                 t.set_limits(old, values[i])
                 self.add_test(t,set)
                 old = values[i]
             except SetEmptyException:
                 pass
         try:
             t1 = rangefeaturetest(self.data, feature)
             t1.set_limits(old, None)
             self.add_test(t1,set)
         except SetEmptyException:
             t.upper = None
             self.add_test(t,set)

     def make_individual_limited_num(self, feature, set, n):
         """Create a partition with one subset for each value of the
         feature, but create at most n partitions (starting with the
         smallest feature values)."""

         assert(self.data.type(feature)==type_int or
                self.data.type(feature)==type_float)
         values = self.data.collect_feature_list(feature, set)
         values.sort()
         values = uniq_list(values)
         old = None
         for i in range(1,min(n,len(values))):
             t = rangefeaturetest(self.data, feature)
             t.set_limits(old, values[i])
             self.add_test(t,set)
             old = values[i]
         t = rangefeaturetest(self.data, feature)
         t.set_limits(values[i], None)
         self.add_test(t,set)
       
     def make_individual_limited_symb(self, feature, set, n):
         """Create a partition with one subset for each value of the
         feature, but create at most n partitions. Also add an
         additional else subset."""

         values = self.data.collect_feature_list(feature, set)
         values.sort()
         values = uniq_list(values)
         limit = min(n-1,len(values)-1)
         for i in range(0,limit):
             t = setfeaturetest(self.data, feature)
             t.set_set([values[i]])
             self.add_test(t,set)
         t = setfeaturetest(self.data, feature)
         t.set_set(values[limit:])
         self.add_test(t,set)
         t = notsetfeaturetest(self.data, feature)
         t.set_set(values)
         self.add_elsetest(t,set)

     def make_binary_num(self, feature, set, n):
         """Create a partition so that the first n feature values are
         mapped into set one, subset, the rest into the other"""
         assert(self.data.type(feature)==type_int or
                self.data.type(feature)==type_float)
         values = self.data.collect_feature_list(feature, set)
         values.sort()
         try:
             t = rangefeaturetest(self.data, feature)             
             t.set_limits(None, values[n])
             self.add_test(t,set)
             t = rangefeaturetest(self.data, feature)             
             t.set_limits(values[n], None)
             self.add_test(t,set)
         except (SetEmptyException,KeyError):
             raise NoSuchPartitionException


def log2(n):
    """Return the logarithm dualis of the number given."""
    return math.log(n)/log_nat_2

def rel_frequency(total_size, subset_size, classes):
    """Estimate probability of a subset, given the the total
    number of draws, the subset size, and the number of classes"""
    return subset_size/float(total_size)

def rule_of_succession(total_size, subset_size, classes):
    """Same thing, but use rule of succession instead of relative
    frequency. """
    return (subset_size+1)/float((total_size+classes))

def info_content(p):
    """Return the information content of an event with probability
    p."""
    return -log2(p)

def compute_entropy(sets, prob_estimate):
    """Compute the entropy for a list of subsets, given a funtion for
    estimating the probability of each subset"""

    distrib = map(len, sets)
    partition_size = len(distrib)
    total = reduce(add,distrib,0)

    probs = map(lambda x, y=total, z=partition_size:
                prob_estimate(y,x,z), distrib)
    info = map(info_content, probs)
    weighted_info = map(lambda x,y:x*y,probs,info)
    # print probs, info, weighted_info
    return reduce(add, weighted_info, 0)

# class assignment:
#     """Store a mapping from problem names to protocols (i.e. heuristics)"""
#     def __init__(self, protocolset):
#         self.mapping = {}
#         self.protocols = protocolset

#     def insert_prot(self, prot):
#         for i in prot.probs.keys():
#             self.mapping[i] = prot.name;

#     def make_empty(self, set):
#         for i in set:
#             self.mapping[i] = None

#     def __repr__(self):
#         res = map(lambda x, m=self.mapping:x+"<-"+m[x]+"\n", self.mapping.keys())
#         return reduce(add,res,"")

#     def ass_heuristic(self, prob):
#         return self.mapping[prob]
#     def result(self, prob):
#         return self.protocols.result(self.ass_heuristic(prob),prob)

#     def evaluate(self, probs=None):
#         if not probs:
#             probs = self.mapping.keys()
#         return reduce(result_add, map(self.result, probs), (0,0))

#     def find_optimal(self, set):
#         """Insert set into the assignment (if it is not already in set) and
#            find the optimal single heuristic for set"""
#         pass

#     def find_canonical(self, set):
#         pass

#     def complete_optimal(self):
#         pass

#     def complete_canonical(self):
#         pass


argc = len(sys.argv)

if argc <= 1:
    raise RuntimeError, "Usage: new_auto.py <prob_desc> <protocols>"

features = featurelist(sys.argv[1])
set = features.features.keys()
part = partition(features)
#part.make_equicard(1,set,2)
part.make_equidist(1,set,100)
print part
print part.give_sets()
print compute_entropy(part.give_sets(), rel_frequency)
print compute_entropy(part.give_sets(), rule_of_succession)
# print features.featuretypes

protocols = protocolset();
for i in sys.argv[2:]:
    print i;
    tmp=protocol().parse(i)
    protocols.insert(tmp)


