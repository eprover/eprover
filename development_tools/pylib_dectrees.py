#!/usr/bin/env python2.7
# ----------------------------------
#
# Module pylib_dectrees
#
# Code for learning decision trees from examples.
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


import pylib_basics
import pylib_probabilities
import pylib_ml_examples


def decision_tree(sample, entropy_compare_fun, tolerance, max_split):
    """
    Return a decision tree for sample.
    """
    root =  decision_tree_node(sample, "")
    tmp = True
    while tmp:
        tmp = root.refine(entropy_compare_fun,tolerance,max_split)
    return root

def global_decision_tree(sample, dummy, tolerance, max_split):
    """
    Return a decision tree for sample.
    """
    root =  decision_tree_node(sample, "")
    tmp = True
    while tmp:
        tmp = root.global_refine_search(max_split)
        if tmp:
            tmp = root.global_refine_apply(tolerance*root.best_relinfogain)
            entros = root.entropies()
            rig = entros[3]
            if rig > root.best_relinfogain:
                root.best_relinfogain = rig
    return root

class decision_tree_node:
    """
    A decision tree (duh!)
    """
    def __init__(self,
                 sample,
                 node_name = "",
                 root    = None) :
        """
        Create an initialized tree node. 
        """
        self.subtrees    = {}
        self.node_name   = node_name
        self.sample_size = len(sample)
        self.classadmin  = sample.classadmin
        self.sample      = sample
        self.root        = root
        self.closed      = False
        (self.tclass, self.tclass_freq) = sample.most_frequent_class()
        if not root:
            self.root = self
            self.root.best_relinfogain = 0
        if len(self.sample)<=1:
            self.closed = True


    def extend_leaf(self, part):
        """
        Extend the current leaf node with a given partition.
        """
        assert(self.isleaf())

        if self.node_name == "":
            name_pfx = ""
        else:
            name_pfx =  self.node_name+" and "
        for i in part.parts.keys():
            self.subtrees[i] = decision_tree_node(part.parts[i],
                                                  name_pfx+repr(i),
                                                  self.root)

    def refine(self,           
               entropy_compare_fun,
               tolerance,
               maxsplit):
        """
        Expand a tree node one level. This only affects
        self.subtrees. Return True if tree could be expanded with
               entropic win, False otherwise.
        """
        if self.closed:
            return False

        if not self.isleaf():
            res = False
            for i in self.subtrees.values():
                if i.refine(entropy_compare_fun, tolerance, maxsplit):
                    res = True
            return res

        old_rig = self.root.best_relinfogain
        
        if pylib_basics.verbose():
            print "# Searching test for node {%s} (%ld examples) (current RIG: %f)"\
                  %(self.node_name,self.sample_size, old_rig)
        (relinfgain, absinfgain, part) = \
                     pylib_ml_examples.find_best_partition(self.sample,
                                                           entropy_compare_fun,
                                                           maxsplit)
        if not part:
            if pylib_basics.verbose():
                print "# No possible split found"
            return False
        if pylib_basics.verbose():
            print "# Best partition: ", part.abstracter

        self.extend_leaf(part)
        entros  =  self.root.entropies()
        new_rig = entros[3]

        if pylib_basics.verbose():
            print "# New RIG: ", new_rig,
        if new_rig < old_rig*tolerance:
            if pylib_basics.verbose():
                print " Node closed"            
            self.subtrees = {}
            self.closed = True
            res = False
        else:
            if pylib_basics.verbose():
                print " Node expanded"            
            res = True
        if new_rig > self.root.best_relinfogain:
            self.root.best_relinfogain = new_rig
        return res
        

    def global_refine_search(self, maxsplit):
        """ 
        For each open leaf node in the tree find the extension that
        leads to the best tree and store it and its evaluation in
        node.best_part, node.eval. Return True if an open node was
        found.
        """        
        res = False
        if self.closed:
            return res
        if not self.isleaf():
            for i in self.subtrees.values():
                if i.global_refine_search(maxsplit):
                    res =  True
            return res

        old_rig = self.root.best_relinfogain        
        if pylib_basics.verbose():
            print "# Searching test for node {%s} (%ld examples) (current RIG: %f)"\
                  %(self.node_name,self.sample_size, old_rig)

        pg = pylib_ml_examples.partition_generator(self.sample, maxsplit)
        self.best_refine = None
        self.best_rig    = 0
        res = True
        while 1:
            try:
                part = pg.next()
                self.extend_leaf(part)
                entros  =  self.root.entropies()
                new_rig = entros[3]
                if new_rig > self.best_rig:
                    self.best_rig = new_rig
                    self.best_refine = self.subtrees
                self.subtrees = {}
            except StopIteration:
                break
        return res

    def global_refine_apply(self, rig_limit):
        """
        Apply all tree refinedments that are good enough. Return True
        if any have been found.
        """
        res = False
        if self.closed:
            return res

        if not self.isleaf():
            for i in self.subtrees.values():                
                if i.global_refine_apply(rig_limit):
                    res = True
            return res

        if self.best_rig >= rig_limit and self.best_refine:
            self.subtrees = self.best_refine
            res = True
            if pylib_basics.verbose():
                print "# Extending ", self," with ", self.best_refine
        else:
            self.best_refine = None
        return res
        
        
    def isleaf(self):
        """
        Return true if node is a leaf node, False otherwise.
        """
        return len(self.subtrees)==0
        
    def size(self):
        """
        Return the number of nodes in the tree, including leaf nodes.
        """
        res = 1
        if not self.isleaf():
            for i in self.subtrees.values():
                res += i.size()
        return res

    def leaves(self):
        """
        Return number of leaves in the tree.
        """
        if self.isleaf():
            res = 1
        else:
            res = 0
            for i in self.subtrees.values():
                res += i.leaves()
        return res
        

    def depth(self):
        """
        Return the depth of the tree (leaves have dept 0, not 1!).
        """
        res = 0
        if not self.isleaf():
            res = 1+max([i.depth() for i in self.subtrees.values()])
        return res

    def printout(self, depth=0):
        """
        Print a simple indented version of the tree.
        """
        prefix = "   " * depth 
        print prefix+"Node {"+self.node_name+"}("+ repr(self.sample_size)+" examples)"
        if self.isleaf():
            print prefix+"Class: %s with %f (%ld examples)"%\
                  (repr(self.classadmin.get_name(self.tclass)),
                   self.tclass_freq,
                   self.sample_size)
        else:
            for i in self.subtrees.values():
                i.printout(depth+1)

    def __repr__(self):
        return self.node_name


    def classify(self,example):
        """
        Return predicted class of example.
        """
        return self.nodify(example).tclass


    def nodify(self,example):
        """
        Return the leaf node example drops into.
        """
        if self.isleaf():
            return self
        else:
            for i in self.subtrees.keys():
                if i(example):
                    return self.subtrees[i].nodify(example)
            raise "IncompleteTreeError"

    
    def classify_set(self, set, verbose=True):
        """
        Classify all examoles in set. If possible, return successes
        and number of examples.
        """
        count = 0
        succ  = 0
        for i in set:
            sclass = self.classify(i)
            tclass = i.tclass_val()
            if tclass!=None:               
                count +=1
                res = "!="
                if sclass == tclass:
                    succ+=1
                    res = "=="
                if verbose:
                    tc = repr(self.classadmin.get_name(tclass))
                    sc = repr(self.classadmin.get_name(sclass))
                    print "%-30s: %-5s %s %-5s" %(i.id, tc, res, sc)
            else:
                sc = repr(self.classadmin.get_name(sclass))
                print "%-30s: %-5s" %(i.id, sc)
        return (succ, count)

    def entropies(self):
        """
        Return a tuple of various entropic measures.
        """
        tp        = tree_partition(self, self.sample)
        apriori   = tp.get_class_entropy()
        cost      = tp.get_entropy()
        remainder = tp.get_remainder_entropy()
        absinfgain = apriori-remainder
        if cost == 0:
            relinfgain = 0
        else:
            relinfgain = pylib_probabilities.rel_info_gain(apriori,
                                                           remainder,
                                                           cost)
        return (apriori, remainder, absinfgain, relinfgain)


    def characteristics(self):
        """
        Return a tuple of characteristic values for the tree. 
        """
        tsize     = self.size()
        tdepth    = self.depth()
        tleaves   = self.leaves()

        return (tsize, tdepth, tleaves)


class tree_partition(pylib_ml_examples.partition):
    """
    Create a partition based on the leaves of a decision tree.
    """
    def __init__(self, tree, examples):
        self.abstracter = lambda x:tree.nodify(x)
        pylib_ml_examples.partition.__init__(self, examples)

