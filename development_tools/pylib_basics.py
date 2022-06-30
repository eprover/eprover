#!/usr/bin/env python2.7
# ----------------------------------
#
# Module pylib_basics
#
# Trivial stuff not easily classified elsewhere. Some of this stuff 
# should probably be in the base language or libraries, but I cannot
# find it.
#
# Copyright 2003 Stephan Schulz, schulz@eprover.org
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


import math

# Types for which sign is well-defined
NumericScalarTypes = [type(1), type(1L), type(1.0)]

# Logarithm of 2 in base e
LogE2              =  math.log(2)


# Names for floating point infinity

def compute_infinity():
    """
    Find a value that does not grow by squaring it (i.e. inf).
    """
    base = 2.0
    cand = base*base
    for i in xrange(1,1000):
        if cand == base:
            return cand
        base = cand
        cand = base*base
    raise ValueError, "Cannot find positive infinity after 1000 iterations"

Infinity = compute_infinity()



# Global variables:

class Globals:
    pass

globals = Globals()
globals.verbose = False

def verbose():
    return globals.verbose

def sign(number):
    """
    Return the sign of a number.
    """
    if type(number) in NumericScalarTypes:
        if number > 0:
            return 1
        elif number < 0:
            return -1
        else:
            return 0
    raise TypeError


def log2(n):
    """
    Return the logarithm dualis of the number given.
    """
    return math.log(n)/LogE2

def is_sorted(l, cmpfun = cmp):
    """
    Return True if l is a sorted list.
    """
    if len(l) <= 1:
        return True
    old = l[0]
    for i in l[1:]:
        if cmpfun(old,i) > 0:
            return False
        old = i
    return True

def uniq(l):
    """
    Create a new list by replacing sublists containing the same element
    in l  with a single copy of the element. Not very efficient, but
    quite Pythonic. I think. This, as UNIX uniq, requires a sorted list.
    """
    nl = list(l)
    i = 0
 
    while 1:
        try:
            if nl[i+1] == nl[i]:
                del(nl[i])
            else:
                i=i+1
        except IndexError:
            break
    return nl

def uniq_unsorted(l):
    """
    Return a list of different elements in order of first occurrence.
    """
    set = {}
    return [set.setdefault(e,e) for e in l if e not in set]


def element_seq_count(l):
    """
    As uniq, but return a list of the length of the subsequences with
    the same element.
    """
    if len(l) == 0:
        return []
    nl = []
    init = l[0]
    count = 1
    for i in l[1:]:
        if i==init:
            count +=1
        else:
            nl.append(count)
            count = 1
            init = i
    nl.append(count)
    return nl


def mean(l):
    """
    Return the mean of a list (of numbers, hopefully).
    """
    if l:
        return float(sum(l))/len(l)
    raise ValueError, "Cannot get the mean of an empty list!"

def variance(l):
    """
    Compute the variance of a list of numbers.
    """
    m = mean(l)
    sqdiffs = [(i-m)**2 for i in l]
    res = sum(sqdiffs)/(len(l)-1)
    return res

def standard_deviation(l):
    """
    Return the standard deviation of a list of numbers.
    """
    return math.sqrt(variance(l))

def rl_lex_compare(t1, t2):
    """
    Compare two tuples of the same length lexicograhically left to right.
    """
    assert len(t1)==len(t2)
    for i in range(len(t1)-1,-1,-1):
        tmp = cmp(t1[i], t2[i])
        if tmp:
            return tmp
    return 0


class name_number_hash:
    """
    Datatype for maintaining an efficient bijection associating a set
    of n abitrary names into the numbers 0..n.
    """
    def __init__(self):
        self.name_index   = {}
        self.number_index = []
        self.count        = 0

    def insert(self, name):
        try:
            res = self.get_code(name)
        except KeyError:
            self.name_index[name] = self.count
            self.number_index.append(name)
            res = self.count
            self.count += 1
        return res
                                                
    def get_name(self, index):
        return self.number_index[index]

    def get_code(self, name):
        return self.name_index[name]

    def get_entry_no(self):
        return self.count


