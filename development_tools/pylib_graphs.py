#!/usr/bin/env python2.3
# ----------------------------------
"""
Module pylib_graphs.py

Code and datatypes for generating gnuplot plots. Much of the data
collection and conversion is somewhat specialized for the DP project,
but most of the code can be resused or extended easily.

Copyright 2004 Stephan Schulz, schulz@eprover.org

This code is part of the support structure for the equational
heorem prover E. Visit

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

import re
import sys
import os

import pylib_io
import pylib_basics
import pylib_eprots
import pylib_psfixbb

split_res = re.compile("_");
filter_re = re.compile(".*");
name_break = re.compile("[, ]+");


class graph:
    """
    Represents a single graph (data+title). This is just a simple
    container. 
    """
    def __init__(self, title="", data=None):
        self.title=title;
        if data:
            self.data=data;
        else:
            self.data = [];

    def add_problem_data(self, median_list):
        for i in median_list:
            xval = int(split_res.split(i[0])[-1]);
            yval = i[1];
            self.data.append( (xval, yval) );
            self.data.sort();

    def parse(self, file, pattern=re.compile(".*")):
              p = pylib_eprots.eprotocol(file, True);
              f = p.filter(pattern);
              tmp = f.collect_medians()
              self.add_problem_data(tmp);

    def __repr__(self):
        res = "# "+self.title + "\n";
        for i in self.data:
            xval = i[0];
            yval = i[1];
            res = res+ ("%4d %7.4f\n" % (xval, yval));
        return res;
 


class plot:
    """
    Represents a plot: Labels+Format+Graphs.
    """
    def __init__(self, title = "Unnamed plot", graph_descs = []):
        self.graphs = []

        for i in graph_descs:
            g = graph(i[0])
            g.parse(i[1], i[2])
            self.add_graph(g)
            self.title = title

        self.generic_options = """
set style data linespoints
set xlabel 'Instance size'
set ylabel 'Run time (s)'
set key left
"""
        self.file_options ="""
set terminal postscript eps 16 color
set size square
"""

    def add_graph(self, graph):
        self.graphs.append(graph)

    def settings(self, file=None, log=False):
        """
        return a sequence of 'set' commands for gnuplot.
        """
        res = self.generic_options
        if file:
            res = res+self.file_options            
            res = res+"set output \""+file+"\"\n";
        if log:
            res = res+"set logscale y\n";       
        return res

    def plot_command(self):
        """
        Return a plot command that will plot all graphs, properly
        labeled.
        """
        sep = "";
        res = "plot "
        for i in self.graphs:
            res = res+ sep+"'-' title \""+i.title+"\"";
            sep = ", "
        res = res+"\n";
        for i in self.graphs:
            res = res+i.__repr__()+"e\n";
        return res;

    def file_name(self, log=False):
        """
        Return a filename suitable for an eps file of this graph.
        Generated from the title.
        """
        tmp = re.sub(name_break, "_", self.title);
        if log:
            tmp = tmp + "_log";
        return tmp+".eps"
    
    def gnuplot(self, file=False, log=False):
        """
        Call gnuplot and do the plotting.
        """
        filename = None;
        if file:
            filename = self.file_name(log)
        
        pipe=os.popen("gnuplot", "w")
        pipe.write(self.settings(filename, log))
        pipe.write(self.plot_command())
        pipe.flush()
        if not file:
            print "  Return to continue!"
            sys.stdin.readline()
        else:
            pylib_psfixbb.fixbb(filename)
        pipe.close()
