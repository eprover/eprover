#!/usr/bin/env python2.3
# ----------------------------------
"""
Usage: do_dp_plots.py [Options]

Create a number of plots from test data on the theory of arrays. This
is rather specialized code. To change the number of contents of the
plots, edit the code below.

Plots can be displayed on the screen or written as eps files.

Options:

-h
 Print this information and exit.

-f
 Create result files (default is to display graphs via X)

-d<directory>
 Point to the directory where the result files can be found. This
 overrides the hard-coded default.

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
import os.path

import pylib_io
import pylib_graphs

cvc_sc_pat = re.compile("storecomm.*nf")
e_sc_pat   = re.compile("storecomm.*sf")

cvc_si_pat = re.compile("storeinv.*nf")
e_si_pat   = re.compile("storeinv.*sf")

cvc_sw_pat = re.compile("swap.*nf")
e_sw_pat   = re.compile("swap.*sf")


dir = "/Users/schulz/SOURCES/Projects/VERONA/dp/array/TEST_RESULTS/"
files = False

options = pylib_io.get_options()

for o in options:
    if o == "-f":
        files = True
    if o[0:2] == "-d":
        dir = o[2:]        
    if o=="-h":
        print __doc__
        sys.exit()

if dir[-1]!="/":
    dir = dir + "/"

dir = os.path.expanduser(dir)

t1_sc_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sc_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_sc_pat),
    ("E (sts11), built-in index type" ,
     dir+"protokoll_E_sts11_t1ni_default", e_sc_pat),
    ("E (sts09), axiomatized indices" ,
     dir+"protokoll_E_sts09_t1_default",
     e_sc_pat),    
    ]

t1_sc_plot = pylib_graphs.plot(t1_sc_data)

t2_sc_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sc_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_sc_pat),
    ("E (sts11), built-in index type" ,
     dir+"protokoll_E_sts11_t2ni_default", e_sc_pat),
    ("E (sts09), axiomatized indices" ,
     dir+"protokoll_E_sts09_t2_default",
     e_sc_pat),    
    ]

t2_sc_plot =  pylib_graphs.plot(t2_sc_data)

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

t1_si_plot =  pylib_graphs.plot(t1_si_data)


t2_si_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_si_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_si_pat),
    ("E (sts11)" ,
     dir+"protokoll_E_sts11_t2ni_default", e_si_pat),
    ("E (sts09)" ,
     dir+"protokoll_E_sts09_t2_default",
     e_si_pat),    
    ]

t2_si_plot =  pylib_graphs.plot(t2_si_data)

t1_sw_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sw_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_sw_pat),
    ("E (sts11)" ,
     dir+"protokoll_E_sts11_t1ni_default", e_sw_pat),
    ("E (sts09)" ,
     dir+"protokoll_E_sts09_t1_default",
     e_sw_pat),    
    ]

t1_sw_plot =  pylib_graphs.plot(t1_sw_data)

t2_sw_data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sw_pat),
    ("CVCLite" , dir+"protokoll_CVCL_Auto", cvc_sw_pat),
    ("E (sts11)" ,
     dir+"protokoll_E_sts11_t2ni_default", e_sw_pat),
    ("E (sts09)" ,
     dir+"protokoll_E_sts09_t2_default",
     e_sw_pat),    
    ]

t2_sw_plot =  pylib_graphs.plot(t2_sw_data)


if files:
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
else:
    print "Linear Scale Plots:"
    print "  STORECOMM, t1"
    t1_sc_plot.gnuplot(None)
    print "  STOREINV, t1"
    t1_si_plot.gnuplot(None)
    print "  STOREINV, t2"
    t2_si_plot.gnuplot(None)
    print "  SWAP, t1"
    t1_sw_plot.gnuplot(None)
    print "  SWAP, t2"
    t2_sw_plot.gnuplot(None)

    print "Log Scale Plots:"
    print "  STORECOMM, t1"
    t1_sc_plot.gnuplot(None, True)
    print "  STOREINV, t1"
    t1_si_plot.gnuplot(None, True)
    print "  STOREINV, t2"
    t2_si_plot.gnuplot(None, True)
    print "  SWAP, t1"
    t1_sw_plot.gnuplot(None, True)
    print "  SWAP, t2"
    t2_sw_plot.gnuplot(None, True)
