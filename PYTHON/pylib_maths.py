#!/usr/bin/env python

"""
Module pylib_maths.py

Some basic mathematical functions and functional programming tools.

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

import math
import unittest


def fold(f, l, empy, dir):
    """
    Fold/reduce a list with binary function l.
    """
    if l:
        lc = list(l)
        res = lc.pop(dir)
        while lc:
            res = f(res, lc.pop(dir))
    else:
        res = empy
    return res


def fold_l(f, l, empty=None):
    """
    Fold/reduce a list (from the left) with binary function l.
    """
    return fold(f,l,empty,0)


def fold_r(f, l, empty=None):
    """
    Fold/reduce a list (from the left) with binary function l.
    """
    return fold(f,l,empty,-1)


def sum(l):
    """
    Sum over elements of l.
    """
    return fold_l(lambda x,y:x+y, l, 0.0)
    

def mean(l):
    """
    Compute the arithmetic mean of l. Forwards ZeroDivisionError if
    list is empty, TypeError if the list is not numerical.
    """
    return float(sum(l))/len(l)

    

def median(l):
    """
    Compute median of l.
    """
    if l:
        local = l
        local.sort()
        return local[len(l)/2]
    else:
        return None


def sampleStdDeviation(l):
    """
    Return the sample standard deviation of l.
    """
    m         = mean(l)
    dev_vsq   = [(x-m)**2 for x in l]
    sum_diffs = sum(dev_vsq)
    return math.sqrt(sum_diffs/float(len(l)-1))


class TestMath(unittest.TestCase):
    
    def testSum(self):
        self.assertAlmostEqual(sum(range(10)), 45)
        
    def testStats(self):
        l1=[4]*10
        l2=range(10)

        self.assertEqual(median(l1),4)
        self.assertEqual(median(l2),5)
        self.assertAlmostEqual(mean(l1),4)
        self.assertAlmostEqual(mean(l2),4.5)
        
    def testStdDec(self):
        l1 = [1,3,1,3,1,3]

        sigma = sampleStdDeviation(l1)
        print l1, sigma


if __name__ == '__main__':
    unittest.main()
