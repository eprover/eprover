#!/usr/bin/env python2.7
# ----------------------------------
"""
Usage: eprot_proc.py [options] [protocols]

Parse E format test run protocols and do stuff with them. Currently
mostly for testing  the libraries. This might replace many of the
older AWK tools in the future.

Options:

-h
 Print this information and exit.


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

split_res = re.compile("_");
filter_re = re.compile(".*");

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
    def __init__(self, graph_descs = []):
        self.graphs = []

        for i in graph_descs:
            g = graph(i[0])
            g.parse(i[1], i[2])
            self.add_graph(g)

    def add_graph(self, graph):
        self.graphs.append(graph)

    def settings(self, file=None, log=False):
        """
        return a sequence of 'set' commands for gnuplot.
        """
        res = "unset logscale\n"+\
              "set style data linespoints\n"+\
              "set xlabel 'Instance size'\n"+\
              "set ylabel 'Run time (s)'\n"+\
              "set key left\n";
        if file:
            res = res+"set terminal postscript color\n";
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

    def gnuplot(self, file=None, log=False):
        """
        Call gnuplot and do the plotting.
        """
        pipe=os.popen("gnuplot", "w")
        pipe.write(self.settings(file, log))
        pipe.write(self.plot_command())
        pipe.flush()
        if not file:
            print "Retun to continue!"
            sys.stdin.readline()
        pipe.close()


dir = "/Users/schulz/SOURCES/Projects/VERONA/dp/array/TEST_RESULTS/"
cvc_sc_pat = re.compile("storecomm.*nf")
e_sc_pat   = re.compile("storecomm.*sf")

cvc_si_pat = re.compile("storeinv.*nf")
e_si_pat   = re.compile("storeinv.*sf")

cvc_sw_pat = re.compile("swap.*nf")
e_sw_pat   = re.compile("swap.*sf")


t1_sc_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sc_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_sc_pat),
    ("E (sts11), built-in index type" ,
     dir+"protokoll_E_sts11_t1ni_default", e_sc_pat),
    ("E (sts09), axiomatized indices" ,
     dir+"protokoll_E_sts09_t1_default",
     e_sc_pat),    
    ]

t1_sc_plot = plot(t1_sc_data)

t2_sc_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sc_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_sc_pat),
    ("E (sts11), built-in index type" ,
     dir+"protokoll_E_sts11_t2ni_default", e_sc_pat),
    ("E (sts09), axiomatized indices" ,
     dir+"protokoll_E_sts09_t2_default",
     e_sc_pat),    
    ]

t2_sc_plot = plot(t2_sc_data)

# Broken, as E solves no single problem
# t2_sc_plot.gnuplot(None)

t1_si_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_si_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_si_pat),
    ("E (sts11)" ,
     dir+"protokoll_E_sts11_t1ni_default", e_si_pat),
    ("E (sts09)" ,
     dir+"protokoll_E_sts09_t1_default",
     e_si_pat),    
    ]

t1_si_plot = plot(t1_si_data)


t2_si_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_si_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_si_pat),
    ("E (sts11)" ,
     dir+"protokoll_E_sts11_t2ni_default", e_si_pat),
    ("E (sts09)" ,
     dir+"protokoll_E_sts09_t2_default",
     e_si_pat),    
    ]

t2_si_plot = plot(t2_si_data)

t1_sw_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sw_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_sw_pat),
    ("E (sts11)" ,
     dir+"protokoll_E_sts11_t1ni_default", e_sw_pat),
    ("E (sts09)" ,
     dir+"protokoll_E_sts09_t1_default",
     e_sw_pat),    
    ]

t1_sw_plot = plot(t1_sw_data)

t2_sw_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sw_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_sw_pat),
    ("E (sts11)" ,
     dir+"protokoll_E_sts11_t2ni_default", e_sw_pat),
    ("E (sts09)" ,
     dir+"protokoll_E_sts09_t2_default",
     e_sw_pat),    
    ]

t2_sw_plot = plot(t2_sw_data)


#t1_sc_plot.gnuplot(None)
#t1_si_plot.gnuplot(None)
#t2_si_plot.gnuplot(None)
#t1_sw_plot.gnuplot(None)
#t2_sw_plot.gnuplot(None)

t1_sc_plot.gnuplot("t1_storecomm_plot.eps")
t1_si_plot.gnuplot("t1_storeinv_plot.eps")
t2_si_plot.gnuplot("t2_storeinv_plot.eps")
t1_sw_plot.gnuplot("t1_swap_plot.eps")
t2_sw_plot.gnuplot("t2_swap_plot.eps")

t1_sc_plot.gnuplot("t1_storecomm_log_plot.eps", True)
t1_si_plot.gnuplot("t1_storeinv_log_plot.eps", True)
t2_si_plot.gnuplot("t2_storeinv_log_plot.eps", True)
t1_sw_plot.gnuplot("t1_swap_log_plot.eps", True)
t2_sw_plot.gnuplot("t2_swap_log_plot.eps", True)

