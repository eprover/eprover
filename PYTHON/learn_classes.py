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
from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC

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


class ClassCollection(object):
    def __init__(self):
        self.problems = list([])
        self.labels   = list([])

    def add_line(self, line):
        parts = line.split(":")
        problem = parts[0]
        label   = int(parts[2])
        features = list(map(float, parts[1].split(",")))
        self.problems.append(features)
        self.labels.append(label)

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
    contr_args = [arg for arg in sys.argv if arg.find("protoc")==-1]
    prot_args  = [arg for arg in sys.argv if arg.find("protoc")!=-1]
    print("#", " ".join(contr_args), "<%d prots>"%(len(prot_args),))

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

    clf = DecisionTreeClassifier(max_depth=None, min_samples_split=2,
                                 random_state=0)
    scores = cross_val_score(clf, X, y, cv=10)
    print("DT:  ", scores.mean())

    clf = RandomForestClassifier(n_estimators=10, max_depth=None,
                                 min_samples_split=2, random_state=0)
    scores = cross_val_score(clf, X, y, cv=10)
    print("RF:  ", scores.mean())

    clf = ExtraTreesClassifier(n_estimators=10, max_depth=None,
                               min_samples_split=2, random_state=0)
    scores = cross_val_score(clf, X, y, cv=10)
    print("ERF: ", scores.mean())

    clf = KNeighborsClassifier(n_neighbors=1)
    scores = cross_val_score(clf, X, y, cv=10)
    print("1-NN:", scores.mean())

    clf = KNeighborsClassifier(n_neighbors=2)
    scores = cross_val_score(clf, X, y, cv=10)
    print("2-NN:", scores.mean())

    clf = KNeighborsClassifier(n_neighbors=3)
    scores = cross_val_score(clf, X, y, cv=10)
    print("3-NN:", scores.mean())

    clf = SVC()
    scores = cross_val_score(clf, X, y, cv=10)
    print("SVM: ", scores.mean())

    #clf = SVC(kernel='linear')
    #scores = cross_val_score(clf, X, y, cv=10)
    #print("Slin: ", scores.mean())

    #clf = SVC(kernel='rbf')
    #scores = cross_val_score(clf, X, y, cv=10)
    #print("Srbf:", scores.mean())

    #clf = SVC(kernel='sigmoid')
    #scores = cross_val_score(clf, X, y, cv=10)
    #print("Ssig:", scores.mean())
