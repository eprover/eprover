#!/usr/bin/env python2.2
# ----------------------------------
#
# Module pylib_basics
#
# Trivial stuff not easily classified elsewhere. Some of this stuff 
# should probably be in the base language or libraries, but I cannot
# find it.
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


import math

# Types for which sign is well-defined
NumericScalarTypes = [type(1), type(1L), type(1.0)]

# Logarithm of 2 in base e
LogE2              =  math.log(2)


# Names for floating point infinity

def compute_infinity():
    base = 2.0
    cand = base*base
    for i in xrange(1,1000):
        if cand == base:
            return cand
        base = cand
        cand = base*base
    raise ValueError, "Cannot find positive infinity after 1000 iterations"

Infinity = compute_infinity()


# Portable booleans (missing in older Python versions):

try:
    True
    False
except NameError:
    True  = 1
    False = 0


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
    Create a new list by replacing sublists containing the same elemen
    in l  with a single copy of the element. Not very efficient, but
    quite Pythonic. I think.
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

def sum(l):
    """
    Return the sum of elements of l.
    """
    return reduce(lambda x,y:x+y, l, 0)
