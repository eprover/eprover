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
    if t1 == str:
        return t1
    if t2 == str:
        return t2
    if t1 == float:
        return t1
    if t2 == float:
        return t2
    assert(t1==int and t2==int)
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
        if type(rep) == str:
            tmp = rep.split(":");
            tmp = map(string.strip, tmp)
            tmp[1] = tmp[1].split(",")
            tmp[1] = map(string.strip, tmp[1])
        else:
            assert type(rep) == tuple
            assert len(rep) == 2 or len(rep)==3
            tmp = rep

        assert type(tmp[1]) == list

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
        self.feature_types = None
        self.feature_no = None
        self.name = None
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
        self.feature_types = None

    def get_feature_type(self, feature):
        """
        Give the type of the named feature.
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
        Return list of all values for a given feature.
        """
        return map(lambda x, f=feature:x.feature_val(f), self)

    def get_feature_range(self, feature):
        """
        Return tuple with largest and smallest feature value (using
        natural order of the type).
        """
        tmp = self.get_feature_values(feature)
        tmp.sort()
        return (tmp[0], tmp[-1])

    
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
    

class partition:
    """
    Represent a partition of examples with named partitions. Partition
    type is defined by the abstracter() function that should return a
    partition label for each subset.
    """
    def __init__(self, examples=[]):
        self.partitions = {}
        self.insert_set(examples)


    def insert(self, example):
        """
        Insert a single example.
        """
        part = self.abstracter(example)
        try:
            self.partitions[part].append(example)
        except KeyError:
            tmp = ml_exampleset()
            tmp.set_name(part)
            tmp.append(example)
            self.partitions[part] = tmp     

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
        tmp = self.partitions.keys()
        tmp.sort()
        for i in tmp:
            res = res+repr(i)+"\n"+repr(self.partitions[i])
        return res

    def entropy(self):
        af = pylib_probabilities.abs_freq_vector(self.partitions)
        return pylib_probabilities.compute_entropy(af)
        

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
            lb = value >= self.lower_bound
        if self.upper_bound==None:
            ub = True
        else:
            ub = value < self.upper_bound
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

        
    
class class_partion(partition):
    def __init__(self, examples):
        self.abstracter = lambda x:x.tclass_val()
        partition.__init__(self, examples)


class scalar_feature_partition(partition):
    def __init__(self, examples, feature, limits):
        self.abstracter = scalar_feature_partitioner(feature, limits)
        partition.__init__(self, examples)
    




