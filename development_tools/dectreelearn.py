#!/usr/bin/env python2.7
# ----------------------------------
"""
Usage: dectreelearn.py [options] <trainfile> [testfile]

Generate a decision tree for classifying the examples in example
file and do stuff with it. 

Options:

-x<n>
 Perform n-fold stratified cross-validition. If n is ommited, performs
 10-fold cross-validation. Folds are stratified for class as far as
 possible, but proportions of very rare classes may still be way off
 (if there are only 5 examples with class X, but you request 10-fold
 cross-validation, 5 folds will have _no_ example for that class).

-X<n>
 Same, but performs non-stratified cross-validiation. THIS IS A
 PARTICULARLY BAD IDEA IF THE INPUT SET IS SORTED BY CLASS! I know of
 no reason why you should ever prefer -X to -x, but was reminded about
 the value of stratification the hard way.

-S<n>
 Provide a seed for the pseudo-random number generator used in
 splitting the set for cross-validation. If none is given, current
 system time is used. Use this option to eliminate one random factor
 when comparing different parameters.

-g<limit>
 Sets the relative information gain limit for refining nodes. Smaller
 values make for bigger trees. Default is 0.5, but needs more
 experiments and mybe that a knot in my thinking unknots.

-a
 Use absolute information gain instead of relative information gain to
 select features. If you use -g0.0 -a -s2, this should behave (more or
 less) like a standard ID3 algorithm.

-s<n>
 Set the maximal number of subclasses for each numerical feature to
 n. This only applies at a given node in the tree, the same feature
 can later be picked again and refined.

-v
 Verbose output. The program will print a lot of stuff while doing
 its thing...

-c
 Classify data from a second input file. If classes are given, will
 also print performance, otherwise will just give predicted class for
 each instance.

-e
 Evaluate the generate tree on the training data and print a report.

-n
 Do NOT print the generated tree.

-h
 Print this information and exit.


Copyright 2003 Stephan Schulz, schulz@informatik.tu-muenchen.de

his code is part of the support structure for the equational
heorem prover E. Visit

 http://www4.informatik.tu-muenchen.de/~schulz/WORK/eprover.html 

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


import sys
import string
import random

import pylib_io
import pylib_basics
import pylib_ml_examples
import pylib_probabilities
import pylib_dectrees

relgain_limit = 0.5
entropy_compare_fun = cmp
max_split  = 10
crossval   = 0
eval_tree  = False
classify   = False
printtree  = True
seed       = None
stratified = True
feature_selection = "R"
dectree_constructor = pylib_dectrees.global_decision_tree

options = pylib_io.get_options()
args    = pylib_io.get_args()

for o in options:
    if o[0:2] == "-g":
        relgain_limit = float(o[2:])
    if o == "-a":
        entropy_compare_fun = pylib_basics.rl_lex_compare
        feature_selection = "A"
    if o[0:2] == "-s":
        max_split = int(float(o[2:]))
    if o[0:2] == "-x":
        try:
            crossval = int(float(o[2:]))
        except ValueError:
            crossval = 10
    if o[0:2] == "-X":
        try:
            crossval = int(float(o[2:]))
        except ValueError:
            crossval = 10
        stratified = False
    if o=="-v":
        pylib_basics.globals.verbose = True
    if o=="-c":
        classify = True
        pylib_io.check_argc(2,args)
    if o=="-e":
        eval_tree = True
    if o=="-n":
        printtree = False
    if o=="-h":
        print __doc__
        sys.exit()
    if o[0:2] == "-S":
        seed = long(o[2:])

pylib_io.check_argc(1,None,args)

set = pylib_ml_examples.ml_exampleset()
set.parse(args[0])


if crossval:
    random.seed(seed)
    jobs = set.crossval_sets(crossval, stratified)
    tr_results = []
    te_results = []
    fold = 0
    for i in jobs:
        fold += 1
        tree = dectree_constructor(i[0],
                                   entropy_compare_fun,
                                   relgain_limit,
                                   max_split)
        (tsize, tdepth, tleaves)                     = tree.characteristics()
        (apriori, remainder, absinfgain, relinfgain) = tree.entropies()
        (succ, count) = tree.classify_set(i[0],pylib_basics.verbose())
        succ_percent = float(succ)/count*100
        print "Fold %-2d RIG: %5.3f (%2d,%4d,%4d) Train: %4d out of %4d, %7.3f%% " %\
              (fold, relinfgain,tdepth, tsize, tleaves, succ, count, succ_percent),
        tr_results.append((succ,count,succ_percent,relinfgain, tdepth,
                           tsize, tleaves))
        
        (succ, count) = tree.classify_set(i[1],pylib_basics.verbose())
        succ_percent = float(succ)/count*100
        print "Test: %4d out of %4d, %7.3f%%" % (succ, count, succ_percent)
        te_results.append((succ, count,succ_percent)) 

    tr_percent = (map(lambda x:x[2],tr_results))
    te_percent = (map(lambda x:x[2],te_results))
    rig        = (map(lambda x:x[3],tr_results))
    depths     = (map(lambda x:x[4],tr_results))
    size       = (map(lambda x:x[5],tr_results))
    leaves     = (map(lambda x:x[6],tr_results))
    print "%s Splits %2d RIGL %5.3f RIG %5.3f+/-%5.3f (%5.2f+/-%4.2f, %7.2f+/-%4.2f, %7.2f+/-%4.2f) Train: %7.3f+/-%7.3f%%  Test:  %7.3f+/-%7.3f%%" %\
          (feature_selection,
           max_split,
           relgain_limit,
           pylib_basics.mean(rig),
           pylib_basics.standard_deviation(rig),
           pylib_basics.mean(depths),
           pylib_basics.standard_deviation(depths),
           pylib_basics.mean(size),
           pylib_basics.standard_deviation(size),
           pylib_basics.mean(leaves),
           pylib_basics.standard_deviation(leaves),
           pylib_basics.mean(tr_percent),
           pylib_basics.standard_deviation(tr_percent),
           pylib_basics.mean(te_percent),
           pylib_basics.standard_deviation(te_percent))
           
else:
    tree = dectree_constructor(set,
                               entropy_compare_fun,
                               relgain_limit,
                               max_split)
    if printtree:
        tree.printout()
    if eval_tree:
        (succ, count) = tree.classify_set(set, pylib_basics.verbose())
        print "Successes on training set: %d out of %d, %f%%"% (succ, count, float(succ)/count*100)
    if classify:
        testset = pylib_ml_examples.ml_exampleset()
        testset.parse(args[1])
        (succ, count) = tree.classify_set(testset, True)
        if count!=0:
            print "Successes on test set    : %d out of %d, %f%%"% (succ, count, float(succ)/count*100)

