#!/usr/bin/env python2.2
# ----------------------------------
#
# Module pylib_ml_examples.py
#
# Functions and classes for handling sets of examples form machine
# learning, where each example contains an identifier, a set of
# features of different types, and a desired class.
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

from __future__ import generators

from types import *
import string
from UserList import UserList

import pylib_basics
import pylib_io
import pylib_probabilities

UnknownClass = "UnknownClass"

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
    if t1 == StringType:
        return t1
    if t2 == StringType:
        return t2
    if t1 == FloatType:
        return t1
    if t2 == FloatType:
        return t2
    assert(t1==IntType and t2==IntType)
    return t1


def typemax_l(l):
    """
    Reduce a list of types to the most specific one compatible with
    all elements.
    """
    return reduce(typemax, l, int)


class ml_example:
    """
    Representing an example for machine learning, with am identifier
    (id), a list of features values (features), and (optionally) a
    class (tclass).
    """
    def __init__(self, rep):        
        if type(rep) == StringType:
            tmp = rep.split(":");
            tmp = map(string.strip, tmp)
            tmp[1] = tmp[1].split(",")
            tmp[1] = map(string.strip, tmp[1])
        else:
            assert type(rep) == TupleType
            assert len(rep) == 2 or len(rep)==3
            tmp = rep

        assert type(tmp[1]) == ListType

        self.id       = tmp[0]
        self.features = map(atofeatureval,tmp[1])
        if len(tmp)==3:            
            self.tclass = tmp[2]
        else:
            self.tclass = UnknownClass

    def feature_no(self):
        """
        Return number of different features of example.
        """
        return len(self.features)
    
    def __repr__(self):
        features = string.join(map(str, self.features),",")
        res = self.id + " : " +features;
        if self.tclass!=UnknownClass:
            res+= " : "+self.tclass
        return res

    def feature_val(self, f_no):
        """
        Return value of a given feature.
        """
        return self.features[f_no]

    def tclass_val(self):
        """
        Return target class of feature.
        """
        return self.tclass
        

class ml_exampleset(UserList):
    """
    Representing a set of examples for machine learning.
    """
    def __init__(self, data=[]):
        UserList.__init__(self)
        self.feature_no = None
        self.name       = None
        self.init_precomputed()
        for i in data:
            self.append(i)

    def set_name(self, name):
        self.name = name

    def append(self, element):
        if self.feature_no:
            assert self.feature_no == element.feature_no()
        else:
            self.feature_no = element.feature_no()
        UserList.append(self, element)
        self.init_precomputed()

    def init_precomputed(self):
        """
        Initialize/reset precomputed values that potentially change
        with each new inertion.
        """
        self.feature_types   = None
        self.feature_values  = None
        self.feature_dvalues = None
        self.feature_range   = None
        self.class_values    = None
        self.class_dvalues   = None


    def get_feature_type(self, feature):
        """
        Give the type of the requested feature.
        """
        return self.get_feature_types()[feature]
        
        
    def get_feature_types(self):
        """
        Return a list of types of features used in examples,
        recomputing it if necessary.
        """
        if not self.feature_no:
            return None
        if self.feature_types:
            return self.feature_types
        self.feature_types = []
        for i in range(0, self.feature_no):
            self.feature_types.append(typemax_l(map(lambda element, f=i:
                                                    type(element.feature_val(f)),
                                                    self.data)))
        return self.feature_types

    def get_feature_values(self, feature):
        """
        Return (sorted) list of all values for a given feature.
        """
        if not self.feature_no:
            return None
        if not self.feature_values:
            self.feature_values = [None] * self.feature_no
        if not self.feature_values[feature]:
            self.feature_values[feature] =\
                map(lambda x, f=feature:x.feature_val(f), self)
            self.feature_values[feature].sort()
        return self.feature_values[feature]


    def get_feature_range(self, feature):
        """
        Return tuple with largest and smallest feature value (using
        natural order of the type).
        """
        if not self.feature_no:
            return None
        if not self.feature_range:
            self.feature_range = [None] * self.feature_no
        if not self.feature_range[feature]:
            tmp = self.get_feature_values(feature)
            self.feature_range[feature] = (tmp[0], tmp[-1])            
        return self.feature_range[feature]

    def get_distinct_feature_values(self, feature):
        """
        Return (sorted) list of all distinct values for a given
        feature.
        """
        if not self.feature_no:
            return None
        if not self.feature_dvalues:
            self.feature_dvalues = [None] * self.feature_no
        if not self.feature_dvalues[feature]:
            self.feature_dvalues[feature] =\
                pylib_basics.uniq(self.get_feature_values(feature))            
        return self.feature_dvalues[feature]

    def get_class_values(self):
        """
        Return (sorted) list of all classes of examples.
        """
        if self.class_values == None:
            self.class_values = map(lambda x:x.tclass_val(), self)
            self.class_values.sort()
        return self.class_values

    def get_distinct_class_values(self):
        """
        Return (sorted) list of all occuring classes.
        """
        if self.class_dvalues == None:
            self.class_dvalues = pylib_basics.uniq(self.get_class_values())
        return self.class_dvalues     
    
    def parse(self, file):
        f = pylib_io.flexopen(file,'r')
        l = f.readlines()
        pylib_io.flexclose(f)
        for line in l:
            if line[0] == "#":
                continue
            ex = ml_example(line)
            self.append(ex)            

    def __repr__(self):
        if self.name:
            res = "# "+self.name +"\n"
        else:
            res = ""
        for i in self:
            res+=repr(i)+"\n"
        return res

    def abs_freq_vector(self, abstracter):
        tmppart = partition()
        tmppart.abstracter = abstracter
        tmppart.insert_set(self)
        return tmppart.abs_freq_vector()

    def class_freq_vector(self):
        """
        Return a vector of absolute class frequencies.
        """
        classes   = self.get_distinct_class_values()
        class_occ = self.get_class_values()
        res = map(lambda x,l=class_occ:l.count(x), classes)
        # print res
        return res


def class_partitioner(example):
    return example.tclass_val()

    
class partition:
    """
    Represent a partition of examples with named parts. Partition
    type is defined by the abstracter() function that should return a
    partition label for each subset.
    """
    def __init__(self, examples=[]):
        self.parts = {}
        self.insert_set(examples)
        
    def empty(self):
        return len(self.parts)==0

    def insert(self, example):
        """
        Insert a single example.
        """
        part = self.abstracter(example)
        try:
            self.parts[part].append(example)
        except KeyError:
            tmp = ml_exampleset()
            tmp.set_name(part)
            tmp.append(example)
            self.parts[part] = tmp     

    def insert_set(self, examples):
        """
        Insert a whole set of examples.
        """
        for i in examples:
            self.insert(i)
                
    def abstracter(self, example):
        """
        Return a label. This is only defined for concrete instances. 
        """
        assert false, "Virtual function only!"

    def __repr__(self):
        res = "# Partition:\n"
        tmp = self.parts.keys()
        tmp.sort()
        for i in tmp:
            res = res+repr(i)+"\n"+repr(self.parts[i])
        return res

    def abs_freq_vector(self):
        return pylib_probabilities.abs_freq_vector(self.parts)

    def entropy(self):
        af = self.abs_freq_vector()
        return pylib_probabilities.compute_entropy(af)

    def remainder_entropy(self, abstracter=class_partitioner):
        """
        Compute the remainder entropy of performaning the
        abstracter-defined split after the partition.
        """
        # Weird, but the general function is faster than this hack...
        #if abstracter==class_partitioner:
        #    return self.remainder_entropy_class()
        distrib = []
        for i in self.parts.keys():
            tmppart = partition()
            tmppart.abstracter = abstracter
            tmppart.insert_set(self.parts[i])
            distrib.append(tmppart.abs_freq_vector())
        res = pylib_probabilities.remainder_entropy(distrib)        
        return res
    
    def remainder_entropy_class(self):
        """
        Compute the remainder entropy of performing the target class
        split after the partition. This is hopefully faster for the
        special case.
        """
        distrib = []
        for i in self.parts.keys():
            distrib.append(self.parts[i].class_freq_vector())
        return pylib_probabilities.remainder_entropy(distrib)


class scalar_feature_test:
    """
    Callable object representing a single interval constraint on a
    scalar variable. It returns True for values with lb <= val < ub.
    """
    def __init__(self, lower_bound, upper_bound):
        assert not lower_bound or not upper_bound or lower_bound <= upper_bound
        self.lower_bound = lower_bound
        self.upper_bound = upper_bound
        self.name = "X"

    def set_name(self, name):
        self.name = name

    def __call__(self, value):
        if self.lower_bound==None:
            lb = True
        else:
            lb = (value >= self.lower_bound)
        if self.upper_bound==None:
            ub = True
        else:
            ub = (value < self.upper_bound)
        return lb and ub

    def __repr__(self):
        if self.lower_bound==None and self.upper_bound==None:
            return "always_true"    
        elif self.lower_bound==None:
            return self.name+"<"+repr(self.upper_bound)
        elif self.upper_bound==None:
            return self.name+">="+repr(self.lower_bound)
        return self.name+">="+repr(self.lower_bound)+\
               " and "+self.name+"<"+repr(self.upper_bound)    


class scalar_feature_partitioner(partition):
    def __init__(self, feature, limits=[]):
        assert(pylib_basics.is_sorted(limits))
        self.feature_no = feature
        tmpname = "feature["+repr(self.feature_no)+"]"
        if len(limits)==0:
            tmp = scalar_feature_test(None, None)
            tmp.set_name(tmpname)
            self.features = [tmp]
        else:
            lb = None
            self.features = []
            for i in limits:
                tmp = scalar_feature_test(lb,i)
                tmp.set_name(tmpname)
                self.features.append(tmp)
                lb = i
            tmp = scalar_feature_test(lb, None)
            tmp.set_name(tmpname)
            self.features.append(tmp)

    def __call__(self, example):
        tmp = example.feature_val(self.feature_no)
        for i in self.features:
            if i(tmp):
                return repr(i)
        assert False, "Not a partition!"

    def __repr__(self):
        res = "["
        sep = ""
        for i in self.features:
            res = res+sep+repr(i)
            sep = " ; "
        res = res + "]"
        return res


class class_partition(partition):
    """
    Generate a partion of a set of examples based on the target class
    if the examples.
    """
    def __init__(self, examples):
        self.abstracter = class_partitioner
        partition.__init__(self, examples)


class scalar_feature_partition(partition):
    """
    Generate a partion of a set of examples based on a scalar
    feature test.
    """
    def __init__(self, examples, feature, limits):
        self.abstracter = scalar_feature_partitioner(feature, limits)
        partition.__init__(self, examples)
    


def weird_filter(values, boundaries):
    """
    Reduce a boundaries= [n0....nn] as follows:
    For all i from 0 to n-1:If there is no element e in values with
    ni<e<=ni+1, drop ni.
    """
    tmpvals = list(values)
    res = []
    for i in boundaries:
        found = False
        while tmpvals!=[] and tmpvals[0]<i:
            found = True
            tmpvals.pop(0)
        if found:
            res.append(i)
    return res

def equisplit_feature_space(feature_values, n):
    """
    Return a set of boundaries that split the space of (distinct)
    feature values into n approximately equal-sized parts. Works for
    both distinct values and real values, as it will discard
    boundaries occuring more than once.
    """    
    if n<=1:
        return []
    tmp = len(feature_values)
    if n>tmp:
        n = tmp
    step = tmp/float(n)
    res = [feature_values[int(i*step)] for i in range(1, n)]
    res = weird_filter(feature_values,res)
    return res


def equisplit_feature_range(dfeature_values, n):
    """
    Return a set of boundaries that split the range of 
    feature values into n equal-sized parts.
    """
    if n<=1:
        return []
    tmp = dfeature_values[-1]-dfeature_values[0]
    step = tmp/float(n)
    res = [(i*step) for i in range(1, n)]
    return weird_filter(dfeature_values,res)


def prop_n_nary_split(feature_values, proportions):
    """
    Return a set of boundaries located at the proporional values
    given.
    """
    tmp = len(feature_values)
    res = [feature_values[int(p*tmp)] for p in proportions]
    return weird_filter(feature_values,res)

def prop_n_nary_rangesplit(feature_values, proportions):
    """
    Return a set of boundaries located at the proporional values
    within the feature value range.
    """
    tmp = feature_values[-1]-feature_values[0]
    res = [((p*tmp)+feature_values[0]) for p in proportions]
    return weird_filter(feature_values,res)

def first_n_and_rest_split(dfeature_values, n):
    """
    Return a set of boundaries so that the first n values have their
    own class, and all the other ones are lumped into one.
    """
    return dfeature_values[1:(n+1)]
 


class discrete_feature_test:
    def __init__(self, set=[]):
        self.set = set
        self.name = "X"
        self.positive = True

    def set_name(self, name):
        self.name = name

    def __call__(self, value):
        return value in self.set

    def __repr__(self):
        res = self.name+" in "+repr(self.set)
        return res

class discrete_feature_else_test(discrete_feature_test):
    def __init__(self, set=[]):
        discrete_feature_test.__init__(self,set)
        self.positive = False

    def __call__(self, value):
        return not(value in self.set)

    def __repr__(self):
        res = self.name+" notin "+repr(self.set)
        return res

            
class discrete_feature_partitioner:
    """
    Generate a functional object that will sort examples into feature
    classes based on the possible values of a discrete feature.
    """
    def __init__(self, feature, values, value_distrib):
        assert len(values)!=0
        self.feature_no = feature
        most_freq = None
        most_freq_count = 0
        for i in values:
            tmp_count = value_distrib.count(i)
            if tmp_count>most_freq_count:
                most_freq_count = tmp_count
                most_freq = i
        assert(most_freq)
        else_set = list(values)
        else_set.remove(most_freq)
        self.ctests = []
        for i in values:
            if i==most_freq:
                tmp = discrete_feature_else_test(else_set)
                tmp.set_name("feature["+repr(feature)+"]")
                self.ctests.append(tmp)
            else:
                tmp = discrete_feature_test([i])
                tmp.set_name("feature["+repr(feature)+"]")
                self.ctests.append(tmp)
            
    def __call__(self, example):
        tmp = example.feature_val(self.feature_no)
        for i in self.ctests:
            if i(tmp):
                return repr(i)
        assert False, "Not a partition!"

    def __repr__(self):
        res = "["
        sep = ""
        for i in self.ctests:
            res = res+sep+repr(i)
            sep = " ; "
        res = res + "]"
        return res    


class discrete_feature_partition(partition):
    """
    Generate a partion of a set of examples based on a discrete
    feature test.
    """
    def __init__(self, examples, feature, subsetlimit=0):
        self.abstracter = \
                 discrete_feature_partitioner(feature,\
                                examples.get_distinct_feature_values(feature),\
                                examples.get_feature_values(feature))
        partition.__init__(self, examples)
    

def partition_generator(examples, feature, max_splits):
    """
    Generate sequence of partitions for the given feature. Tries to
    intelligently guess what to do and where to stop.
    """    
    type = examples.get_feature_type(feature)
    if type == StringType:
        yield discrete_feature_partition(examples, feature)
        return

    values  = examples.get_feature_values(feature)
    dvalues = examples.get_distinct_feature_values(feature)
    max_split = min(max_splits, len(dvalues))

    #print  values
    #print dvalues
    # Generate splits by evenly splitting sequence of all values
    for i in range(2,max_split):
        boundaries = equisplit_feature_space(values,i)
        if len(boundaries)>0:
            part = scalar_feature_partition(examples, feature, boundaries)
            yield part
        
    # Generate splits by evenly splitting sequence of all distinct values
    for i in range(2,max_split):
        boundaries = equisplit_feature_space(dvalues,i)
        if len(boundaries)>0:
            part = scalar_feature_partition(examples, feature, boundaries)
            yield part
        
    # Generate splits by evenly splitting the range of values
    for i in range(2,max_split):
        boundaries = equisplit_feature_range(dvalues,i)
        if len(boundaries)>0:
            part = scalar_feature_partition(examples, feature, boundaries)
            yield part

    # Generate some uneven binary splits
    for i in range(1,9):
        boundaries = prop_n_nary_split(dvalues, [0.1*i])
        if len(boundaries)>0:
            part = scalar_feature_partition(examples, feature, boundaries)
            yield part
        boundaries = prop_n_nary_rangesplit(dvalues, [0.1*i])
        if len(boundaries)>0:
            part = scalar_feature_partition(examples, feature, boundaries)
            yield part
        

    # Generate some weird splits:
    ws = [[0.1,0.9], [0.2,0.8], [0.1,0.5,0.1]]
    for i in ws:
        boundaries = prop_n_nary_split(dvalues, i)
        if len(boundaries)>0:
            part = scalar_feature_partition(examples, feature, boundaries)
            yield part
        boundaries = prop_n_nary_split(values, i)
        if len(boundaries)>0:
            part = scalar_feature_partition(examples, feature, boundaries)
            yield part                
        boundaries = prop_n_nary_rangesplit(dvalues, i)
        if len(boundaries)>0:
            part = scalar_feature_partition(examples, feature, boundaries)
            yield part

    # Generate some even weirder splits ;-)
    limit = min(6,max_splits)
    for i in range(1,limit):
        boundaries = first_n_and_rest_split(dvalues,1)
        if len(boundaries)>0:
            part = scalar_feature_partition(examples, feature, boundaries)
            yield part
        
    return

def find_best_feature_partition(examples, a_priory_entropy, feature, max_splits):
    best_relinfgain = -1
    best_absinfgain = -1
    best_part       = None
    part_gen        = partition_generator(examples, feature, max_splits)
    while 1:
        try:
            part       = part_gen.next()
            cost       = part.entropy()
            remainder  = part.remainder_entropy()
            absinfgain = a_priory_entropy-remainder
            relinfgain = pylib_probabilities.rel_info_gain(a_priory_entropy,remainder,cost)
            if relinfgain> best_relinfgain or \
               relinfgain == best_relinfgain and absinfgain>best_absinfgain:
                best_relinfgain = relinfgain
                best_absinfgain = absinfgain
                best_part       = part
        except StopIteration:
            break
    return (best_relinfgain, best_absinfgain, best_part)








