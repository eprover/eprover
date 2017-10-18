#!/usr/bin/env python3
"""
perf_vectors.py 1.0

Usage: perv_vectors.py <protfile1> ... <protfilen>

Read a number of E protocol files and create performance vectors for
each problems.

Options:
 -h
--help      Print this information.

 -f
--failval
  Time value assumed for timouts (for time-based performance
  vectors).

 -m
--maxclusters
  Upper limit of clusters to try when evaluating different clusters.

 -c
--clusters
  Number of clusters to use for final (?) clusterring/class
  assignment.

 -l
--pcalimit
  Upper limit of components for PCA evaluation

 -p
--pca
  Value to use in PCA for dimensionality reduction for clustering. If
  not set, no dimensionality reduction takes place.

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
from sklearn.cluster import KMeans,MiniBatchKMeans, DBSCAN
from sklearn import metrics
from sklearn.metrics import pairwise_distances
from sklearn import datasets
from sklearn.decomposition import PCA



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

def check_none(vec, fail):
    return False

def check_trivial(vec, fail):
    for i in vec:
        if i!=fail:
            return False
    return True

def check_all_succ(vec, fail):
    for i in vec:
        if i==fail:
            return False
    return True

def check_boring(vec, fail):
    return check_trivial(vec, fail) or check_all_succ(vec, fail)


def bin_ind(val, cmp):
    if val==cmp:
        return 1
    return 0

def bin_vec(vec, fail):
    """
    Return an indicator vector for vec.
    """
    return [bin_ind(x, fail) for x in vec]

class ResCollection(object):
    def __init__(self, fail=1000):
        self.problems = {}
        self.fail     = fail
        # print("Self.fail", self.fail)

    def add_result(self, problem, state, time):
        if not problem in self.problems:
            self.problems[problem] = list([])

        results = self.problems[problem]
        if state == "F":
            results.append(float(self.fail))
        else:
            results.append(float(time))

    def add_line(self, line):
        # print("Adding:"+line)
        parts = line.split()
        problem = parts[0]
        state   = parts[1]
        time    = parts[2]
        self.add_result(problem, state, time)

    def ret_vectors(self, filter_trivial=check_boring):
        """
        Return the vectors.
        """
        return np.array([vec for vec in self.problems.values()
                         if not filter_trivial(vec,self.fail)])

    def ret_names(self, filter_trivial=check_boring):
        """
        Return the problem names.
        """
        return [name for name in self.problems.keys()
                if not filter_trivial(self.problems[name],self.fail)]

    def bin_vectors(self, filter_trivial=check_boring):
        """
        Return the vectors as binary (success/fail vectors)
        """
        cands = self.ret_vectors(filter_trivial)
        return np.array([bin_vec(vec, self.fail) for vec in cands])

    def print_vectors(self):
        for res in self.problems:
            print(res)
            print (self.problems[res])


def pca_eval(rescol, limit):
    """
    Perform PCA on the vector sets and print
    """
    bvectors  = rescol.bin_vectors()
    nvectors  = rescol.ret_vectors()
    tbvectors = rescol.bin_vectors(check_trivial)
    tnvectors = rescol.ret_vectors(check_trivial)
    cbvectors = rescol.bin_vectors(check_none)
    cnvectors = rescol.ret_vectors(check_none)

    jobs = [bvectors, nvectors, tbvectors, tnvectors, cbvectors,
            cnvectors]

    print("# $1: components")
    print("# $2: binvectors (non-boring)  ", len(bvectors))
    print("# $3: nvectors   (non-boring)  ", len(nvectors))
    print("# $4: binvectors (non-trivial) ", len(tbvectors))
    print("# $5: nvectors   (non-trivial) ", len(tnvectors))
    print("# $6: binvectors (all)         ", len(cbvectors))
    print("# $7: nvectors   (all)         ", len(cnvectors))

    featureno = len(bvectors[1])
    for k in range(1, min(featureno,limit+1)):
        print("%3d"%(k,),end='  ')

        for job in jobs:
            pca = PCA(copy=True, iterated_power='auto',
                      n_components=k, random_state=None, svd_solver='auto',
                      tol=0.0, whiten=True)
            pca.fit(job)
            print("%1.6f"%(sum(pca.explained_variance_ratio_),),
                  end=' ')
        print(flush=True)


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


def cluster_cov_eval(vectors, labels):
    """
    Return the fraction of problems classified in a labeled class
    where the best covering strategy covers it. Arguments are the
    unmodified binary vectors and the cluster assignment.
    """
    size = max(labels)+1
    classes = [list() for i in range(size)]
    # print(size, len(labels),max(labels), min(labels), labels)    
    for label, vector in zip(labels, vectors):
        # print(classes[label])
        # print(vector)
        classes[label].append(vector)
        # print(classes[label])
        # print()
    # Now classes is a list of lists of vectors. Concretely:
    # classes[label] is the list of all the individuals, represented
    # as binary vectors.
        
    coverage = 0
    for cl in classes:
        dummy, maxcov = find_best_coverage(cl)
        coverage += maxcov
    return coverage/len(labels)

def cluster_eval(bvectors, nvectors, bvec_archive, limit):
    """
    Perform clustering with up to limit clusters and print the
    results.
    """
    print ("# %2s   %-12s   %-12s   %-12s   %-12s   %-12s   %-12s   %-12s   %-12s"%
           ("c", "binertia", "bshil",  "bcalha", "bcovev",
            "ninertia", "nshil",  "ncalha", "ncovev"),
           flush=True);

    for k in range(2, limit+1):
        print ("%4d  "%(k,), end='')
        clusters = KMeans(n_clusters=k, random_state=random_state,
                          init='k-means++').fit(bvectors)
        labels = clusters.labels_
        print("% 8e  "%(clusters.inertia_,), end='')
        print("% 8e  "%(metrics.silhouette_score(bvectors, labels,
                                                metric='euclidean'),),
              end='')
        print("% 8e  "%(metrics.calinski_harabaz_score(bvectors, labels),), end='')
        print("% 8e  "%(cluster_cov_eval(bvec_archive, labels),), end='')


        clusters = KMeans(n_clusters=k, random_state=random_state,
                          init='k-means++').fit(nvectors)
        # print(clusters)
        labels = clusters.labels_
        print("% 8e  "%(clusters.inertia_,), end='')
        print("% 8e  "%(metrics.silhouette_score(bvectors, labels,
                                                metric='euclidean'),),
              end='')
        print("% 8e  "%(metrics.calinski_harabaz_score(bvectors, labels),), end='')
        print("% 8e  "%(cluster_cov_eval(bvec_archive, labels),), end='')

        print(flush=True)

def arr_str(arr):
    return "np.array(["+",".join([str(val) for val in arr])+"])"

def process_options(optlist):
    """
    Process options.
    """
    global clusterlimit, nclusters, pcano, pcalimit, failval

    for opt, arg in optlist:
        # print(opt,arg)
        if opt == "-h" or opt == "--help":
            print(__doc__)
            sys.exit()
        elif opt =="-m" or opt == "--maxclusters":
            clusterlimit = int(arg)
        elif opt =="-c" or opt == "--clusters":
            nclusters = int(arg)
        elif opt =="-p" or opt == "--pca":
            pcano=int(arg)
        elif opt =="-l" or opt == "--pcalimit":
            pcalimit=int(arg)
        elif opt =="-f" or opt == "--failval":
            failval=int(arg)

pcano = 0
pcalimit = 0
clusterlimit = 0
nclusters = 0
failval = 10000

if __name__ == '__main__':
    contr_args = [arg for arg in sys.argv if arg.find("protoc")==-1]
    prot_args  = [arg for arg in sys.argv if arg.find("protoc")!=-1]
    print("#", " ".join(contr_args), "<%d prots>"%(len(prot_args),))

    opts, args = getopt.gnu_getopt(sys.argv[1:], "hm:c:b:p:l:f:", ["help",
    "--maxclusters=", "--clusters=", "--pca=",
    "--pcalimit=", "--failval="])

    random_state = 123666

    process_options(opts)

    if len(args)<1:
        print(__doc__)
        sys.exit()

    rescol = ResCollection(failval)

    sample_size = 0
    for file in args:
        # print("Processing "+file)
        lines = read_real_lines_from_file(file)
        if sample_size == 0:
            sample_size = len(lines)
        if sample_size!=len(lines):
            print("# ", file, " is incomplete, skipping")
        else:
            for i in lines:
                rescol.add_line(i)

    bvectors = rescol.bin_vectors()
    bvec_archive = np.array(bvectors)
    nvectors = rescol.ret_vectors()
    featureno = len(bvectors[1])
    print("# %d,  %d b/n-vectors" %(len(bvectors), len(nvectors)))

    if pcalimit!=0:
        pca_eval(rescol, pcalimit)
        sys.exit()

    if pcano!=0:
        pcano = min(featureno,pcano)
        print("# Performing PCA reduction with %d components"%(pcano,))
        pca = PCA(copy=True, iterated_power='auto',
                  n_components=pcano, random_state=None, svd_solver='auto',
                  tol=0.0, whiten=True)
        pca.fit(nvectors)
        nvectors=pca.transform(nvectors)

        pca = PCA(copy=True, iterated_power='auto',
                  n_components=pcano, random_state=None, svd_solver='auto',
                  tol=0.0, whiten=True)
        pca.fit(bvectors)
        bvectors=pca.transform(bvectors)

    if clusterlimit!=0:
        cluster_eval(bvectors, nvectors, bvec_archive, clusterlimit)
        sys.exit()

    if nclusters:
        clusters = KMeans(n_clusters=nclusters,
                          random_state=random_state,
                          init='k-means++').fit(bvectors)
        labels = clusters.labels_
        names = rescol.ret_names()
        vecs  = rescol.bin_vectors()
        for prob,perf,cl in zip(names, vecs, labels):
            print("%-20s : %s : %4d"%(prob, arr_str(perf), cl))
        # db = DBSCAN(eps=0.6, min_samples=10).fit(bvectors)
        # core_samples_mask = np.zeros_like(db.labels_, dtype=bool)
        # core_samples_mask[db.core_sample_indices_] = True
        # labels = db.labels_

        # # Number of clusters in labels, ignoring noise if present.
        # n_clusters_ = len(set(labels)) - (1 if -1 in labels else 0)
        # print("# Clusters: ", n_clusters_)
        # labels = db.labels_
        # names = rescol.ret_names()
        # for prob,cl in zip(names,labels):
        #      print("%-20s  %4d"%(prob,cl))
