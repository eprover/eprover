#!/usr/bin/env python2.7
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
import random
from UserList import UserList

import pylib_basics
import pylib_io
import pylib_probabilities


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
    def __init__(self,
                 rep,
                 classadmin = pylib_basics.name_number_hash()):        
        self.classadmin = classadmin
        if type(rep) == StringType:
            tmp = rep.split(":");
            tmp = map(string.strip, tmp)
            tmp[1] = tmp[1].split(",")
            tmp[1] = map(string.strip, tmp[1])
        else:
            assert type(rep) == TupleType
            assert len(rep) == 3 or len(rep)==4
            tmp = rep
            
        assert type(tmp[1]) == ListType
        self.id       = tmp[0]
        self.features = map(atofeatureval,tmp[1])
        if len(tmp)==3:
            self.tclass = self.classadmin.insert(tmp[2])
        else:
            self.tclass = None

    def feature_no(self):
        """
        Return number of different features of example.
        """
        return len(self.features)
    
    def __repr__(self):
        features = string.join(map(str, self.features),",")
        res = self.id + " : " +features;
        if self.tclass!=None:
            res+= " : "+self.classadmin.get_name(self.tclass)
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
        self.feature_no  = None
        self.name        = None
        self.class_omega = None
        self.init_precomputed()
        self.classadmin = pylib_basics.name_number_hash()
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

    def extend(self, l):
        for i in l:
            self.append(i)

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

    def get_class_number(self):
        """
        Return the number of distinct classes.
        """
        return self.classadmin.get_entry_no()
    
    def parse(self, file):
        f = pylib_io.flexopen(file,'r')
        l = f.readlines()
        pylib_io.flexclose(f)
        for line in l:
            if line[0] == "#":
                continue
            ex = ml_example(line, self.classadmin)
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

    def most_frequent_class(self):
        """
        Find the most frequent class of examples in the set and return
        it (and its relative frequency). 
        """
        classes = self.get_distinct_class_values()
        class_occ =  self.get_class_values()
        class_freq = [(class_occ.count(cl),cl) for cl in classes]
        class_freq.sort()
        abs_freq, mf_class = class_freq[-1]
        rel_freq = float(abs_freq)/len(class_occ)
        return (mf_class, rel_freq)

    def class_freq_vector(self):
        """
        Return an in-order vector of absolute class frequencies.
        """
        res = [0] * self.get_class_number()
        class_occ = self.get_class_values()
        for i in class_occ:
            res[i]+=1
        # print res, len(res)
        return res
    
    def get_class_entropy(self):
        return pylib_probabilities.compute_entropy_absdistrib(self.class_freq_vector())
    def plain_rel_inf_gain(self):
        apriori_entropy = pylib_probabilities.compute_entropy_absdistrib([1]
                                               * self.get_class_number())
        real_entropy = self.get_class_entropy()
        return pylib_probabilities.rel_info_gain(apriori_entropy,
                                                 real_entropy,
                                                 real_entropy)

    def random_split(self,n):
        """
        Randomly split the example set into n neary equal-sized subsets.
        """
        tmp = list(self)
        random.shuffle(tmp)
        res = []
        for i in range(n):
            res.append([])
        count = 0
        for i in tmp:
            res[count].append(i)
            count+=1
            count = count % n
        return res

    def stratified_split(self,n):
        """
        Randomly split the example set into n neary equal-sized
        subsets stratified for class.
        """
        def weird_cmp(ex1, ex2):
            tmp = cmp(ex1.tclass_val(), ex2.tclass_val)
            if tmp:
                return tmp
            return cmp(ex1.tmp, ex2.tmp)
        
        tmp = list(self)
        random.shuffle(tmp)
        count = 0
        for i in tmp:
            i.tmp = count
            count += 1

        tmp.sort(weird_cmp)        
        res = []
        for i in range(n):
            res.append([])
        count = 0
        for i in tmp:
            res[count].append(i)
            count+=1
            count = count % n
        return res
    

    def crossval_sets(self,n, stratified=True):
        """
        Return a list of n tuples (training_set, test_set), so that
        the union of both is the full set, the 10 test sets are
        disjoint, and the union of the 10 test sets is the full set.
        """
        res = []
        if stratified:
            tmp = self.stratified_split(n)
        else:
            tmp = self.random_split(n)            
        for i in range(len(tmp)):
            train = ml_exampleset()
            train.classadmin = self.classadmin 
            for j in range(len(tmp)):
                if j!=i:
                    train.extend(tmp[j])
            test =  ml_exampleset()
            test.classadmin = self.classadmin
            test.extend(tmp[i])
            res.append((train, test))
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
        self.entropy           = None
        self.class_entropy     = None
        self.remainder_entropy = None
        
    def empty(self):
        return len(self.parts)==0

    def trivial(self):
        return len(self.parts)<=1

    def insert(self, example, class_admin=None):
        """
        Insert a single example.
        """
        self.entropy           = None
        self.class_entropy     = None
        self.remainder_entropy = None
        part = self.abstracter(example)
        try:
            self.parts[part].append(example)
        except KeyError:
            tmp = ml_exampleset()
            tmp.classadmin = class_admin
            tmp.set_name(part)
            tmp.append(example)
            self.parts[part] = tmp     

    def insert_set(self, examples):
        """
        Insert a whole set of examples.
        """        
        if isinstance(examples, ml_exampleset):
            class_admin = examples.classadmin
            for i in examples:
                self.insert(i, class_admin)
        else:
            for i in examples:
                self.insert(i)
            

    def get_size(self):
        return len(self.parts)
                
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

    def get_class_distributions(self):
        res = [i.class_freq_vector() for i in self.parts.values()]
        return res

    def compute_entropies(self):
        if self.entropy != None:
            return
        if len(self.parts)==1: # Avoid rounding errors        
            tmp = self.parts.values()[0].get_class_entropy()
            (self.entropy, self.class_entropy, self.remainder_entropy) =\
                           (0,tmp,tmp)
        else:
            tmp = self.get_class_distributions()
            (self.entropy, self.class_entropy, self.remainder_entropy) =\
                           pylib_probabilities.compute_entropies(tmp)
                
    def get_class_entropy(self):
        """
        Return the a-priory entropy of the class distribution.
        """
        self.compute_entropies()
        return self.class_entropy
                

    def get_entropy(self):
        """
        Return the entropy of the partion.
        """
        self.compute_entropies()
        return self.entropy

    def get_remainder_entropy(self):
        """
        Return the remainder entropy of the class test after
        performing the paritioning.
        """
        self.compute_entropies()        
        return self.remainder_entropy
        

class scalar_feature_test:
    """
    Callable object representing a single interval constraint on a
    scalar variable. It returns True for values with lb <= val < ub.
    """
    def __init__(self, feature, lower_bound, upper_bound):
        assert not lower_bound or not upper_bound or lower_bound <= upper_bound
        self.lower_bound = lower_bound
        self.upper_bound = upper_bound
        self.feature = feature
        self.name = "feature["+repr(feature)+"]"

    def __call__(self, example):
        value = example.feature_val(self.feature)
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
            self.features = [tmp]
        else:
            lb = None
            self.features = []
            for i in limits:
                tmp = scalar_feature_test(feature,lb,i)
                self.features.append(tmp)
                lb = i
            tmp = scalar_feature_test(feature,lb, None)
            self.features.append(tmp)

    def __call__(self, example):
        for i in self.features:
            if i(example):
                return i
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
    def __init__(self, feature, set=[]):
        self.set = set
        self.feature = feature
        self.name = "feature["+repr(feature)+"]"
        self.positive = True

    def __call__(self, example):
        value = example.feature_val(self.feature)
        return value in self.set

    def __repr__(self):
        res = self.name+" in "+repr(self.set)
        return res

class discrete_feature_else_test(discrete_feature_test):
    def __init__(self, feature, set=[]):
        discrete_feature_test.__init__(self,feature,set)
        self.positive = False

    def __call__(self, example):
        value = example.feature_val(self.feature)
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
                tmp = discrete_feature_else_test(feature,else_set)
                self.ctests.append(tmp)
            else:
                tmp = discrete_feature_test(feature,[i])
                self.ctests.append(tmp)
            
    def __call__(self, example):
        for i in self.ctests:
            if i(example):
                return i
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


class one_and_rest_partitioner(discrete_feature_partitioner):
    """
    Generate a functional object that will sort examples into two
    classes based on wether a certain feature has a given value or
    not.
    """
    def __init__(self, feature, value):
        self.feature_no = feature
        self.ctests = []
        tmp = discrete_feature_else_test(feature, [value])
        self.ctests.append(tmp)
        tmp = discrete_feature_test(feature, [value])
        self.ctests.append(tmp)

class one_and_rest_partition(partition):
    """
    Generate a partion of a set of examples based on a binary outcome
    feature test (feature = value or feature != value)
    """
    def __init__(self, examples, feature, value):
        self.abstracter = one_and_rest_partitioner(feature, value)
        partition.__init__(self, examples)

    
    
def partition_generator_feature(examples, feature, max_splits):
    """
    Generate sequence of partitions for the given feature. Tries to
    intelligently guess what to do and where to stop.
    """    
    type = examples.get_feature_type(feature)
    if type == StringType:
        tmp = discrete_feature_partition(examples, feature)
        if not tmp.trivial():
            yield tmp        
        dvalues = examples.get_distinct_feature_values(feature)
        if len(dvalues) > max_splits:
            return
        for i in dvalues:
            yield one_and_rest_partition(examples, feature, i)        

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
    if max_splits >= 3:
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

def partition_generator(examples, max_split):
    """
    Enumerate all possible feature partions for examples.
    """
    for i in range(0,examples.feature_no):
        pg = partition_generator_feature(examples, i, max_split)
        while 1:
            try:
                part = pg.next()
                yield part
            except StopIteration:
                break
    return

def find_best_feature_partition(examples,
                                compare_fun,
                                feature,
                                max_splits):
    assert isinstance(examples, ml_exampleset)
    assert type(feature) == IntType
    assert type(max_splits) == IntType

    best_relinfgain = -1
    best_absinfgain = -1
    best_part       = None
    part_gen        = partition_generator_feature(examples, feature, max_splits)
    while 1:
        try:
            part       = part_gen.next()
            apriori    = part.get_class_entropy()
            cost       = part.get_entropy()
            remainder  = part.get_remainder_entropy()
            # print "# A-priori, cost, remainder = (%2.6f, %2.6f, %2.6f)" %\
            # (apriori,cost,remainder)
            absinfgain = apriori-remainder
            relinfgain = pylib_probabilities.rel_info_gain(apriori,
                                                           remainder,
                                                           cost)
            
            #print relinfgain, absinfgain
            if compare_fun((relinfgain, absinfgain),
                           (best_relinfgain,best_absinfgain)) > 0:
                best_relinfgain = relinfgain
                best_absinfgain = absinfgain
                best_part       = part
        except StopIteration:
            break
    return (best_relinfgain, best_absinfgain, best_part)




def find_best_partition(examples,
                        compare_fun,
                        max_splits):
    assert type(max_splits == IntType)
    best_relinfgain = -1
    best_absinfgain = -1
    best_part       = None    

    if len(examples) > 1:
        for i in range(0,examples.feature_no):        
            (relinfgain, absinfgain, part) = \
                         find_best_feature_partition(examples,
                                                     compare_fun,
                                                     i,
                                                     max_splits)
            if pylib_basics.verbose():
                print "# Evaluating feature %d: %1.6f, %1.6f "\
                      %(i,relinfgain, absinfgain),
            if part:
                if pylib_basics.verbose():
                    print part.abstracter
            else:
                if pylib_basics.verbose():
                    print "# No split possible, feature is homogenous:",
                    print examples.get_distinct_feature_values(i)
            if compare_fun((relinfgain, absinfgain),
                           (best_relinfgain,best_absinfgain)) > 0:
                best_relinfgain = relinfgain
                best_absinfgain = absinfgain
                best_part = part                
                
    return  (best_relinfgain, best_absinfgain, best_part)

