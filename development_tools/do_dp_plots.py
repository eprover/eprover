#!/usr/bin/env python2.3
# ----------------------------------
"""
Usage: do_dp_plots.py [Options]

Create a number of plots from test data on the theory of arrays. This
is rather specialized code. To change the number or contents of the
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

cvc_sc_pat    = re.compile("storecomm_[^i].*nf_ai")
cvc_scf_pat   = re.compile("storecomm_[^i].*sf_ai")
e_sc_pat      = re.compile("storecomm_[^i].*sf_ai")
e_sc_ni_pat   = re.compile("storecomm_[^i].*sf_ni")

cvc_si_pat  = re.compile("storeinv_[^i].*nf_ai")
cvc_sif_pat = re.compile("storeinv_[^i].*sf_ai")
e_si_pat    = re.compile("storeinv_[^i].*sf_ai")

cvc_sw_pat  = re.compile("swap_[^i].*nf_ai")
cvc_swf_pat = re.compile("swap_[^i].*sf_ai")
e_sw_pat    = re.compile("swap_[^i].*sf_ai")

cvc_scinv_pat    = re.compile("storecomm_invalid.*nf_ai")
cvc_scfinv_pat   = re.compile("storecomm_invalid.*sf_ai")
e_scinv_pat      = re.compile("storecomm_invalid.*sf_ai")
e_scinv_ni_pat   = re.compile("storecomm_invalid.*sf_ni")

cvc_siinv_pat  = re.compile("storeinv_invalid.*nf_ai")
cvc_sifinv_pat = re.compile("storeinv_invalid.*sf_ai")
e_siinv_pat    = re.compile("storeinv_invalid.*sf_ai")

cvc_swinv_pat  = re.compile("swap_invalid.*nf_ai")
cvc_swfinv_pat = re.compile("swap_invalid.*sf_ai")
e_swinv_pat    = re.compile("swap_invalid.*sf_ai")


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

plot_list = []

# All systems with system-specific input - T1

# Storecomm

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sc_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_sc_pat),
    ("E (sts11), built-in index type" , dir+"protokoll_E_sts11_t1", e_sc_ni_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_sc_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_sc_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_sc_pat), 
    ]

plot = pylib_graphs.plot("Storecomm, T1, Native", data)
plot_list.append(plot)

# Storecomm_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_scinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_scinv_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_scinv_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_scinv_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_scinv_pat), 
    ]

plot = pylib_graphs.plot("Storecomm_invalid, T1, Native", data)
plot_list.append(plot)

#Storeinv

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_si_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_si_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_si_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_si_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_si_pat), 
    ]

plot = pylib_graphs.plot("Storeinv, T1, Native", data)
plot_list.append(plot)

#Storeinv_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_siinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_siinv_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_siinv_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_siinv_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_siinv_pat), 
    ]

plot = pylib_graphs.plot("Storeinv_invalid, T1, Native", data)
plot_list.append(plot)

#Swap

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sw_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_sw_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_sw_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_sw_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_sw_pat), 
    ]


plot = pylib_graphs.plot("Swap, T1, Native", data)
plot_list.append(plot)

#Swap_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_swinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_swinv_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_swinv_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_swinv_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_swinv_pat), 
    ]

plot = pylib_graphs.plot("Swap_invalid, T1, Native", data)
plot_list.append(plot)

# All systems with flattend input - T1

# Storecomm

data = [
    ("CVC (flattened)" , dir+"protokoll_CVC_Auto", cvc_scf_pat),
    ("CVC Lite (flattened)" , dir+"protokoll_CVCL_Auto", cvc_scf_pat),
    ("E (sts11), built-in index type" , dir+"protokoll_E_sts11_t1", e_sc_ni_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_sc_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_sc_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_sc_pat), 
    ]

plot = pylib_graphs.plot("Storecomm, T1, Flat", data)
plot_list.append(plot)

# Storecomm_invalid

data = [
    ("CVC (flattened)" , dir+"protokoll_CVC_Auto", cvc_scfinv_pat),
    ("CVC Lite (flattened)" , dir+"protokoll_CVCL_Auto", cvc_scfinv_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_scinv_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_scinv_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_scinv_pat), 
    ]

plot = pylib_graphs.plot("Storecomm_invalid, T1, Native",data)
plot_list.append(plot)

#Storeinv

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_si_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_si_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_si_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_si_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_si_pat), 
    ]

plot = pylib_graphs.plot("Storeinv, T1, Native", data)
plot_list.append(plot)

#Storeinv_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_siinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_siinv_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_siinv_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_siinv_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_siinv_pat), 
    ]

plot = pylib_graphs.plot("Storeinv_invalid, T1, Native", data)
plot_list.append(plot)

#Swap

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sw_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_sw_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_sw_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_sw_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_sw_pat), 
    ]

plot = pylib_graphs.plot("Swap, T1, Native", data)
plot_list.append(plot)

#Swap_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_swinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_swinv_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_swinv_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t1", e_swinv_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t1", e_swinv_pat), 
    ]

plot = pylib_graphs.plot("Swap_invalid, T1, Native", data)
plot_list.append(plot)


# All systems with system-specific input - T3 on swap


#Swap

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sw_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_sw_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t3", e_sw_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t3", e_sw_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t3", e_sw_pat), 
    ]


plot = pylib_graphs.plot("Swap, T3, Native", data)
plot_list.append(plot)


data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_swf_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_swf_pat),
    ("E (sts11), axiomatized indices" , dir+"protokoll_E_sts11_t3", e_sw_pat),    
    ("E (sts13), axiomatized indices" , dir+"protokoll_E_sts13_t3", e_sw_pat),    
    ("E (sts14), axiomatized indices", dir+"protokoll_E_sts14_t3", e_sw_pat), 
    ]


plot = pylib_graphs.plot("Swap, T3, Flat", data)
plot_list.append(plot)



# Create files


for i in plot_list:
    print "Title: "+i.title+ " (linear)";
    i.gnuplot(files, None);
    print "Title: "+i.title+ " (log)";
    i.gnuplot(files, True);
        
