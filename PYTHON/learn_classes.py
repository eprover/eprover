#!/usr/bin/env python3
"""
perf_vectors.py 1.0

Usage: learn_classes.py <class_file>

Read a number of E protocol files and create performance vectors for
each problems.

Options:
 -h
--help      Print this information.

 -n
--normalize Normalize features to range [0,1] before learning.

Copyright 2017 Stephan Schulz, schulz@eprover.org

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

The original copyright holder can be contacted via email or as

Stephan Schulz
DHBW Stuttgart
Fakultaet Technik
Informatik
Rotebuehlplatz 41
70178 Stuttgart
Germany
"""


import sys
import re
import getopt
import numpy as np

import matplotlib.pyplot as plt

from sklearn.model_selection import cross_val_score
from sklearn.datasets import make_blobs
from sklearn.ensemble import RandomForestClassifier
from sklearn.ensemble import ExtraTreesClassifier
from sklearn.tree import DecisionTreeClassifier
import sklearn.tree as tree
from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC
from sklearn.model_selection import StratifiedShuffleSplit

def clean_list(l):
    """
    Given a list of strings, return the list stripped, and with empty
    lines and comment lines (starting in #) removed.
    """
    res = map(lambda x:x.strip(), l)
    res = [l for l in res if not (l.startswith("#") or l=="")]
    return res


def read_real_lines(fp):
    """
    As fp.readlines(), but strip newlines, empty lines, comment lines.
    """
    tmp = fp.readlines()
    return clean_list(tmp)


def read_real_lines_from_file(name):
    """
    Read a file and return a list of stripped, non-comment, nonempty
    lines.
    """
    fp = open(name, "r")
    res = read_real_lines(fp)
    fp.close()

    return res


def find_best_coverage(bvectors):
    """
    Given a set of equal lenght binary vectors (0 = Success, 1 =
    Timeout), return the index where most of the vectors indicate
    success and the number of successes for that index.
    """
    maxcov  = 0
    bestidx = 0
    for i in range(len(bvectors[0])):
        cv = [1-pv[i] for pv in bvectors]
        cov = sum(cv)
        if cov > maxcov:
            maxcov = cov
            bestidx = i
    return bestidx, maxcov


class ClassCollection(object):
    def __init__(self):
        self.probnames = list([])
        self.problems  = list([])
        self.labels    = list([])
        self.perf_vecs = list([])
        self.stratmap  = None # Map class label to best strategy index
        self.probmap   = {}   # Map problem to performance vector


    def add_line(self, line):
        self.stratmap = None
        parts = line.split(":")
        #print( parts)
        probname = parts[0]
        self.probnames.append(probname)
        perf_vec = eval(parts[1])
        label    = int(parts[2])
        features = list(map(float, parts[3].split(",")))
        self.problems.append(features)
        self.perf_vecs.append(perf_vec)
        self.labels.append(label)
        self.probmap[probname] = perf_vec

    def solves(self, problem, strat):
        return self.probmap[problem][strat] == 0

    def eval_model(self, model):
        """
        Given a trained model, return fraction of problems that would
        be solved by the best strategy of the predicted class.
        """
        res = model.predict(self.problems)
        succ = 0
        for i in range(len(res)):
            if self.solves(self.probnames[i], self.get_class_strat(res[i])):
                succ += 1
        print(succ, " out of ", len(res))
        return succ/len(res)

    def get_class_list(self):
        res =  list(set(self.labels))
        res.sort()
        return res

    def get_class_strat(self, cl):
        """
        Return best strategy for class cl.
        """
        if not self.stratmap:
            self.stratmap = {}
            classes = [list() for i in self.labels]
            for label, vector in zip(self.labels, self.perf_vecs):
                classes[label].append(vector)
            for cl in self.get_class_list():
                vectors = classes[cl]
                strat, sols = find_best_coverage(vectors)
                self.stratmap[cl] = strat
        return self.stratmap[cl]

    def get_features(self):
        return np.array(self.problems)

    def get_normalized_features(self):
        arr = np.array(self.problems)
        for i in range(len(arr[0])):
            col = [row[i] for row in arr]
            maximum = max(col)
            minimum = min(col)
            for row in arr:
                row[i] = (row[i]-minimum)/(maximum-minimum)
        return arr


    def get_labels(self):
        return np.array(self.labels)


def process_options(optlist):
    """
    Process options.
    """
    global normalize

    for opt, arg in optlist:
        # print(opt,arg)
        if opt == "-h" or opt == "--help":
            print(__doc__)
            sys.exit()
        elif opt == "-n" or opt == "--normalize":
            normalize = True

if __name__ == '__main__':
    print("#", " ".join(sys.argv))

    opts, args = getopt.gnu_getopt(sys.argv[1:], "hn", ["help",
    "--normalize"])

    normalize = False
    random_state = 123666

    process_options(opts)

    if len(args)<1:
        print(__doc__)
        sys.exit()

    classcol = ClassCollection()

    for file in args:
        lines = read_real_lines_from_file(file)
        for i in lines:
            classcol.add_line(i)

    if(normalize):
        X = classcol.get_normalized_features()
    else:
        X = classcol.get_features()

    y = classcol.get_labels()

    # print(classcol.get_class_list())

    # for i in classcol.get_class_list():
    #     print(i, classcol.get_class_strat(i))

    results = []
    results2 = []
    results3 = []

    clf = DecisionTreeClassifier(max_depth=None, min_samples_split=2,
                                 random_state=0)
    cv = StratifiedShuffleSplit ()
    scores = cross_val_score(clf, X, y, cv=cv)
    print("DT:  ", scores.mean(), "+/-", scores.std()*2)
    results.append(scores.mean())
    clf.fit(X,y)
    print("FF:  ", clf.score(X,y))
    results2.append(clf.score(X,y))
    results3.append(classcol.eval_model(clf))

    clf = RandomForestClassifier(n_estimators=10, max_depth=None,
                                 min_samples_split=2, random_state=0)
    scores = cross_val_score(clf, X, y, cv=cv)
    print("RF:  ", scores.mean(), "+/-", scores.std()*2)
    results.append(scores.mean())
    clf.fit(X,y)
    print("FF:  ", clf.score(X,y))
    results2.append(clf.score(X,y))
    results3.append(classcol.eval_model(clf))

    clf = ExtraTreesClassifier(n_estimators=10, max_depth=None,
                               min_samples_split=2, random_state=0)
    scores = cross_val_score(clf, X, y, cv=cv)
    print("ERF: ", scores.mean(), "+/-", scores.std()*2)
    results.append(scores.mean())
    clf.fit(X,y)
    print("FF:  ", clf.score(X,y))
    results2.append(clf.score(X,y))
    results3.append(classcol.eval_model(clf))

    clf = KNeighborsClassifier(n_neighbors=1)
    scores = cross_val_score(clf, X, y, cv=cv)
    print("1-NN:", scores.mean(), "+/-", scores.std()*2)
    results.append(scores.mean())
    clf.fit(X,y)
    print("FF:  ", clf.score(X,y))
    results2.append(clf.score(X,y))
    results3.append(classcol.eval_model(clf))

    clf = KNeighborsClassifier(n_neighbors=2)
    scores = cross_val_score(clf, X, y, cv=cv)
    print("2-NN:", scores.mean(), "+/-", scores.std()*2)
    results.append(scores.mean())
    clf.fit(X,y)
    print("FF:  ", clf.score(X,y))
    results2.append(clf.score(X,y))
    results3.append(classcol.eval_model(clf))

    clf = KNeighborsClassifier(n_neighbors=3)
    scores = cross_val_score(clf, X, y, cv=cv)
    print("3-NN:", scores.mean(), "+/-", scores.std()*2)
    results.append(scores.mean())
    clf.fit(X,y)
    print("FF:  ", clf.score(X,y))
    results2.append(clf.score(X,y))
    results3.append(classcol.eval_model(clf))

    clf = SVC()
    scores = cross_val_score(clf, X, y, cv=cv)
    print("SVM: ", scores.mean(), "+/-", scores.std()*2)
    results.append(scores.mean())
    clf.fit(X,y)
    print("FF:  ", clf.score(X,y))
    results2.append(clf.score(X,y))
    results3.append(classcol.eval_model(clf))

    print("Cross-Validation")
    for res in results:
        print("%2.1f%%"%(res*100,))

    print("Memorization")
    for res in results2:
        print("%2.1f%%"%(res*100,))

    print("Solutions")
    for res in results3:
        print("%2.1f%%"%(res*100,))
