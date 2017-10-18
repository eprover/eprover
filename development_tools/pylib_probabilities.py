#!/usr/bin/env python2.7
# ----------------------------------
#
# Module pylib_probabilities.py
#
# Functions and classes for computing probability estimates and
# entropy for various datatypes.
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

from types import *
import pylib_basics



def rel_frequency(subset_size, total_size, omega_size=None):
    """
    Estimate probability of a subset, given the the total 
    number of draws and the subset size, and the size of omega (the
    probability space).
    """  
    return subset_size/float(total_size)


def rule_of_succession(subset_size, total_size, omega_size):
    """
    Same thing, but use rule of succession instead of relative
    frequency.
    """
    return (subset_size+1)/float((total_size+omega_size))


def prob_vector(abs_freq_vector, sample_size, omega_size):
    """
    Compute vector of probability estimates for a vector of absolute
    frequencies. 
    """
    assert type(abs_freq_vector) == ListType
    
    pv = map(lambda x:rule_of_succession(x,sample_size,omega_size), abs_freq_vector)
    #print "prob_vector:", abs_freq_vector
    #print "prob_vector:", pv
    #print  "prob_vector:", sum(pv)
    return pv


def info_content(p):
    """
    Return the information content of an event with probability p.
    """
    if p==0:
        return 0 # Hack for bad estimators
    return -pylib_basics.log2(p)


def compute_entropy(prob_distrib):
    """
    Compute the entropy for a list of relative frequencies.
    """
    assert type(prob_distrib) == ListType

    info = map(info_content, prob_distrib)
    weighted_info = map(lambda x,y:x*y,prob_distrib,info)
    return reduce(lambda x,y:x+y, weighted_info, 0)

def compute_entropy_absdistrib(distrib):
    """
    As above, but use a list of absolute frequencies, one entry per
    elementary event!
    """
    omega_size = len(distrib)
    sample_size = sum(distrib)
    prob_distrib = prob_vector(distrib,sample_size, omega_size)
    return compute_entropy(prob_distrib)


def remainder_entropy(sample, estimator=rel_frequency):
    """
    Return the remainder entropy H(A|B) of a sample, where a sample
    is a list of lists, outer lists represent the result of B, inner
    list the results of A|B
    """
    assert type(sample)  == ListType
    
    sample_size = reduce(lambda x,y:x+y, map(sum, sample),0)
    outer_omega_size = len(sample)
    #print "remainder_entropy:", sample    
    outer_abs_freq = map(sum, sample)
    outer_probs = prob_vector(outer_abs_freq,estimator)
    #print  "remainder_entropy:", outer_abs_freq
    #print  "remainder_entropy:", outer_probs, sum(outer_probs)
    weighted_entropies= map(lambda x, y: (y,
                                          compute_entropy(x, estimator)),
                            sample,
                            outer_probs)
    #print weighted_entropies
    c_ent = reduce(lambda x,y:x+y, map(lambda x:x[0]*x[1], weighted_entropies),0)
    return c_ent



def compute_entropies(distrib):
    """
    Distrib is a list of lists.
    Assume two experiments, A with outcomes A1...An and B with
    outcomes B1...Bn. We give the distribution of results in a sample
    as follows: The elements with Ai are in the ith list of
    distrib. The elements with Bj are represented by the jth entry in
    all the Ai. Return the entropy of A, the entropy of B, and the
    remainder entropy of B|A.
    """
    omega_size = len(distrib*len(distrib[0]))
    sample_size = sum(map(sum, distrib))
    # print sample_size, omega_size
    prob_distrib = map(lambda x:prob_vector(x,sample_size, omega_size),distrib)

    part_distrib = map(sum, prob_distrib)
    part_entropy = compute_entropy(part_distrib)
    # print "part_distrib:", part_distrib, sum(part_distrib)

    class_distrib = reduce(lambda l1, l2:map(lambda x,y:x+y, l1, l2),
                           prob_distrib)
    class_entropy = compute_entropy(class_distrib)
    # print "Class_distrib:", class_distrib, sum(class_distrib)
    
    remainder_entropy = 0;
    for i in range(0, len(distrib)):
        remainder_entropy += (part_distrib[i]*
                              compute_entropy_absdistrib(distrib[i]))    

    return part_entropy,class_entropy,remainder_entropy

    

def info_gain(a_priori_entropy, remainder_entropy):
    """
    Compute the expected information gain given a-priory entropy of a
    distribution and remainder entropy.
    """
    #assert a_priori_entropy>=remainder_entropy
    if a_priori_entropy < remainder_entropy:
        # Allow for rounding errors        
        if pylib_basics.verbose():
            print "Warning: Remainder enropy > a-priori-entropy",
            print remainder_entropy, ">", a_priori_entropy
        remainder_entropy = a_priori_entropy
    
    return a_priori_entropy - remainder_entropy


def rel_info_gain(a_priori_entropy, remainder_entropy, test_cost_entropy):
    """
    Compute the relative information gain given a-priory entropy,
    remainder entropy, and the entropy of the test used to parition
    the sample.
    """
    g = info_gain(a_priori_entropy, remainder_entropy)
    if g == test_cost_entropy:
        return pylib_basics.Infinity
    else:
        return g/(test_cost_entropy - g)


    




