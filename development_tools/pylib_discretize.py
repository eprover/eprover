#!/usr/bin/env python2.2
# ----------------------------------
#
# Module pylib_discretize
#
# Functions (and classes) for rounding/equalizing floats 
# in various ways.

import math

def const_round(val):
    return 0;
    
class equidist_round:
    def __init__(self, distance):
        self.distance = distance
		
    def __call__(self, val):
        return int(math.ceil(float(val)/self.distance)*self.distance)

def prop_round(val):
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
		
def no_round(val):
    return val



