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
        return len(self.features)
    
    def __repr__(self):
        features = string.join(map(str, self.features),",")
        res = self.id + " : " +features;
        if self.tclass!=UnknownClass:
            res+= " : "+self.tclass
        return res

    def feature_val(self, f_no):
        return self.features[f_no]


class ml_exampleset(UserList):
    """
    Representing a set of examples for machine learning.
    """
    def __init__(self, data=[]):
        UserList.__init__(self)
        self.feature_types = None
        self.feature_no = None
        for i in data:
            self.append(i)

    def append(self, element):
        if self.feature_no:
            assert self.feature_no == element.feature_no()
        else:
            self.feature_no = element.feature_no()
        UserList.append(self, element)
        self.feature_types = None

    def compute_feature_types(self):
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
        res = ""
        for i in self:
            res+=repr(i)+"\n"
        return res
    








