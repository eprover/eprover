#!/usr/bin/env python2.7
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


class titled_object(object):
    """
    An object with a title that can be converted into a file name
    for writing a version of it. 
    Yes, I'm going wild on inheritance ;-)
    """

    def __init__(self, title = None):
        self.set_title(title)

    def get_title(self):
        return self.title;

    def set_title(self, title):
        self.title = title

    def filename(self, appendix):
        if self.title:
            tmp = re.sub(name_break, "_", self.title);
        else:
            tmp = "Unnamed"
        return tmp+appendix
            

class graph:
    """
    Represents a single graph (data+title). This is just a simple
    container. 
    """
    def __init__(self, title="", data=None):
        self.title=title;
        self.style = None
        if data:
            self.data=data;
        else:
            self.data = [];

    def get_title(self):
        return self.title

    def set_style(self, style):
        self.style = style

    def get_style(self):
        if self.style:
            return str(self.style)
        return ""

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
 


style_dict = {}

class plot(titled_object):
    """
    Represents a plot: Labels+Format+Graphs.
    """
    def __init__(self, title = "Unnamed plot", graph_descs = []):
        self.graphs = []

        self.set_title(title)

        for i in graph_descs:
            g = graph(i[0])
            g.parse(i[1], i[2])
            if g.get_title() in style_dict:
                g.set_style(style_dict[g.get_title()])
            else:
                print "Warning, unknown graph title "+g.get_title()
            self.add_graph(g)

        self.title = title
        self.xlabel = "'Instance size'";
        self.ylabel = "'Run time (s)'";

        self.generic_options = """
set style data linespoints
set key left
set pointsize 2
set autoscale fix
"""
        self.file_options ="""
set terminal postscript eps 16
set size square
"""


    def add_graph(self, graph):
        self.graphs.append(graph)

    def set_labels(xlabel = None, ylabel = None):
        self.xlabel = xlabel
        self.ylabel = ylabel

    def gnuplot_labels(self):
        """
        Return a string containing options to pass to gnuplot.
        """

        xl=""
        yl=""

        if self.xlabel:
            xl = "set xlabel "+self.xlabel+"\n"
        if self.ylabel:
            yl = "set ylabel "+self.ylabel+"\n"

        return xl+yl
            
    
    def settings(self, file=None, log=False):
        """
        return a sequence of 'set' commands for gnuplot.
        """
        res = self.generic_options+self.gnuplot_labels()
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
            res = res+ sep+"'-' title \""+i.title+\
                  "\" with linespoints "+i.get_style();
            sep = ", "
        res = res+"\n";
        for i in self.graphs:
            res = res+i.__repr__()+"e\n";
        #print res
        return res;

    def full_filename(self, log=False):
        """
        Return a filename suitable for an eps file of this graph.
        Generated from the title.
        """

        tmp = ".eps"
        if log:
            tmp = "_log.eps"
        return self.filename(tmp);
    
    def gnuplot(self, file=False, log=False):
        """
        Call gnuplot and do the plotting.
        """
        filename = None;
        if file:
            filename = self.full_filename(log)
        
        pipe=os.popen("gnuplot", "w")
        pipe.write(self.settings(filename, log))
        pipe.write(self.plot_command())
        pipe.flush()
        if not file:
            print "  Return to continue!"
            sys.stdin.readline()
        
        pipe.close() # Has to come first to make gnuplot actually
                     # and reliably write the file!
        if file:
            pylib_psfixbb.fixbb(filename)




class statistic_sample(titled_object):
    """
    A sample of (numeric) data and methods of displaying and analyzing it.
    """

    def __init__(self, title=None, data=[]):
        self.set_title(title)
        self.data = data[:]
        self.statistics = False
        self.file_options ="""
set terminal postscript eps 16 color
set size square
"""

    def parse_from_eprot(self, file, pattern=re.compile(".*")):
        """
        Parse an E protocol and collect the times to extend the
        sample.
        """

        p = pylib_eprots.eprotocol(file, True);
        f = p.filter(pattern);              
        self.data.extend(f.collect_times())
        self.statistics = False

    
    def compute_statistics(self):
        """
        Compute max, min, average, median, standard deviation and
        number of elements.
        """
        if not self.statistics:
            self.data_sorted = self.data[:]
            self.data_sorted.sort()
            self._min      = self.data_sorted[0]
            self._max      = self.data_sorted[-1]
            self._median   = self.data_sorted[int(len(self.data)/2)]
            self._mean     = pylib_basics.mean(self.data)
            self._sd       = pylib_basics.standard_deviation(self.data)
            self.statistics = True

    def minimum(self):
        self.compute_statistics()
        return self._min

    def maximum(self):
        self.compute_statistics()
        return self._max

    def median(self):
        self.compute_statistics()
        return self._median

    def mean(self):
        self.compute_statistics()
        return self._mean

    def standard_deviation(self):
        self.compute_statistics()
        return self._sd

    def compute_distribution(self, delta):
        delta = float(delta)
        entries = int(self.maximum()/delta)+2
        indices = [int(i/delta) for i in self.data]
        indices.sort()
        print "Values:", entries, indices[-1]
        dist = [0] * entries
        for i in indices:
            dist[i] = dist[i]+1
            
        self._dist =  [(i*delta+delta/2, dist[i]) for i in xrange(len(dist))]
        return self._dist

    def gnuplot_settings(self, file):
        res = """
set xlabel 'Run time (s)'
set ylabel 'Number of instances'
set style fill
set key left
set size square
set xrange [0:3.4]
"""
        if file:
            res =  res+self.file_options
            res = res+"set output \""+file+"\"\n";
        return res;

    def plot_command(self):
        res = """
plot '-' title \"Distribution of run times\" with boxes
"""
        data = [str(i[0])+" "+str(i[1])+"\n" for i in self._dist]
        res = res+"".join(data)+"e\n"
        return res

    def gnuplot(self, file=False):
        """
        Call gnuplot and do the plotting.
        """
        filename = None;
        if file:
            filename = self.filename(".eps")
        
        pipe=os.popen("gnuplot", "w")
        #pipe=sys.stdout
        pipe.write(self.gnuplot_settings(filename))
        pipe.write(self.plot_command())
        pipe.flush()
        if not file:
            print "  Return to continue!"
            sys.stdin.readline()
        
        pipe.close() # Has to come first to make gnuplot actually
                     # and reliably write the file!
        if file:
            pylib_psfixbb.fixbb(filename)
  

            
