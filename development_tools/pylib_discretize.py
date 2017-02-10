#!/usr/bin/env python2.7
# ----------------------------------
#
# Module pylib_discretize
#
# Functions (and classes) for rounding/equalizing floats 
# in various ways.

import math

def const_round(val):
    """
    Always "round" to 0 (not just towards 0!)
    """
    return 0;
    
class equidist_round:
    """
    Round to the nearest multiple of distance.
    """
    def __init__(self, distance=5):
        self.distance = distance
		
    def __call__(self, val):
        return int(math.ceil(float(val)/self.distance)*self.distance)

def prop_round(val):
    """
    Round up to the nearest 1,2,5*10^X
    """
    base = 1;
    while val > 5:
        base = base*10;
	val = val/10;
    if val < 1:
        res = 1
    elif val < 2:
        res = 2
    else:
        res = 5
    return res*base

class log_round:
    """
    Round up to the nearest 2^X (>= start)
    """

    def __init__(self, start=0.5):
        self.start = start

    def __call__(self,val):
        res=self.start
        while res<val:
            res *=2
        return res
    		
def no_round(val):
    """
    Identity function, don't round at all.
    """
    return val



