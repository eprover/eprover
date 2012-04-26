#!/usr/bin/env python2.7
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

Copyright 2004, 2005 Stephan Schulz, schulz@eprover.org

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
from os import system

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


cvc_ios_pat = re.compile("ios_t1_ios_bia_np_sf_ai.*cvc")
e_ios_pat   = re.compile("ios_t1_ios_np_sf_ai.*tptp")

#cvc_circqueue_pat = re.compile("circular_queue_t1_record_ios_mod_np_sf_ai.*cvc");
#cvc_queue_pat = re.compile("queue_t1_record_ios_np_sf_ai.*cvc");
#e_circqueue_pat = re.compile("circular_queue_t1_record_ios_mod_np_sf_ai.*tptp");
#e_queue_pat = re.compile("queue_t1_record_ios_np_sf_ai.*tptp");

cvc_circqueue_pat = re.compile("circular_queue_t1_native_record_ios_mod_np_sf_ai.*cvc");
cvc_queue_pat = re.compile("queue_t1_native_record_ios_np_sf_ai.*cvc");
e_circqueue_pat = re.compile("circular_queue_t1_native_record_ios_mod_np_sf_ai.*tptp");
e_queue_pat = re.compile("queue_t1_native_record_ios_np_sf_ai.*tptp");




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


sd = {
    "CVC"                            : 1,
    "CVC Lite"                       : 2,
    "CVC (flattened)"                : 3,
    "CVC Lite (flattened)"           : 4,
    "E (sts11), built-in index type" : 5,
    "E (sts11), axiomatized indices" : 6,
    "E (sts11)"                      : 6, # Intentional reuse!
    "E (good-lpo), built-in index type" : 5,
    "E (good-lpo), axiomatized indices" : 6,
    "E (good-lpo)"                      : 6, # Intentional reuse!
    "E (sts28)"                      : 7,
    "E (std-kbo)"                      : 7,
    "E (auto)"                       : 8
    }

pylib_graphs.style_dict = sd

# All systems with system-specific input - T1

# Storecomm

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sc_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_sc_pat),
    ("E (good-lpo), built-in index type" , dir+"protokoll_E_sts11_t1", e_sc_ni_pat),
    ("E (good-lpo), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_sc_pat)
    ]

plot = pylib_graphs.plot("Storecomm, T1, Native", data)
plot_list.append(plot)

# Storecomm_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_scinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_scinv_pat),
    ("E (good-lpo), built-in index type", dir+"protokoll_E_sts11_t1", e_scinv_ni_pat),    
    ("E (good-lpo), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_scinv_pat)
    ]

plot = pylib_graphs.plot("Storecomm_invalid, T1, Native", data)
plot_list.append(plot)

#Storeinv

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_si_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_si_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t1", e_si_pat)
    ]

plot = pylib_graphs.plot("Storeinv, T1, Native", data)
plot_list.append(plot)

#Storeinv_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_siinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_siinv_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t1", e_siinv_pat)
    ]

plot = pylib_graphs.plot("Storeinv_invalid, T1, Native", data)
plot_list.append(plot)

#Swap

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sw_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_sw_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t1", e_sw_pat)
    ]


plot = pylib_graphs.plot("Swap, T1, Native", data)
plot_list.append(plot)

#Swap_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_swinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_swinv_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t1", e_swinv_pat)
    ]

plot = pylib_graphs.plot("Swap_invalid, T1, Native", data)
plot_list.append(plot)





# All systems with flattend input - T1

# Storecomm

data = [
    ("CVC (flattened)" , dir+"protokoll_CVC_Auto", cvc_scf_pat),
    ("CVC Lite (flattened)" , dir+"protokoll_CVCL_Auto", cvc_scf_pat),
    ("E (good-lpo), built-in index type" , dir+"protokoll_E_sts11_t1", e_sc_ni_pat),
    ("E (good-lpo), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_sc_pat),    
    ]

plot = pylib_graphs.plot("Storecomm, T1, Flat", data)
plot_list.append(plot)

# Storecomm_invalid

data = [
    ("CVC (flattened)" , dir+"protokoll_CVC_Auto", cvc_scfinv_pat),
    ("CVC Lite (flattened)" , dir+"protokoll_CVCL_Auto", cvc_scfinv_pat),
    ("E (good-lpo), built-in index type", dir+"protokoll_E_sts11_t1", e_scinv_ni_pat),    
    ("E (good-lpo), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_scinv_pat),    
    ]

plot = pylib_graphs.plot("Storecomm_invalid, T1, Flat",data)
plot_list.append(plot)

#Storeinv

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sif_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_sif_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t1", e_si_pat)  
    ]

plot = pylib_graphs.plot("Storeinv, T1, Flat", data)
plot_list.append(plot)

#Storeinv_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sifinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_sifinv_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t1", e_siinv_pat) 
    ]

plot = pylib_graphs.plot("Storeinv_invalid, T1, Flat", data)
plot_list.append(plot)

#Swap

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_swf_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_swf_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t1", e_sw_pat)
    ]

plot = pylib_graphs.plot("Swap, T1, Flat", data)
plot_list.append(plot)

#Swap_invalid

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_swfinv_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_swfinv_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t1", e_swinv_pat)
    ]

plot = pylib_graphs.plot("Swap_invalid, T1, Flat", data)
plot_list.append(plot)


# All systems with system-specific input - T3 on swap


#Swap

data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_sw_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_sw_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t3", e_sw_pat)
    ]


plot = pylib_graphs.plot("Swap, T3, Native", data)
plot_list.append(plot)


data = [
    ("CVC" , dir+"protokoll_CVC_Auto", cvc_swf_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto", cvc_swf_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_t3", e_sw_pat)
    ]


plot = pylib_graphs.plot("Swap, T3, Flat", data)
plot_list.append(plot)



# IOS

data = [
    ("CVC" , dir+"protokoll_CVC_Auto_ios", cvc_ios_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto_ios", cvc_ios_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_ios", e_ios_pat),
    ("E (std-kbo)" , dir+"protokoll_E_sts28_ios", e_ios_pat),
    ]


plot = pylib_graphs.plot("IOS", data)
plot_list.append(plot)

data = [
    ("CVC" , dir+"protokoll_CVC_Auto_ios", cvc_ios_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto_ios", cvc_ios_pat),
    ("E (std-kbo)" , dir+"protokoll_E_sts28_ios", e_ios_pat)
    ]


plot = pylib_graphs.plot("IOS_opt", data)
plot_list.append(plot)

# Queues

data = [
    ("CVC" , dir+"protokoll_CVC_Auto_ios", cvc_queue_pat),
    ("CVC Lite" , dir+"protokoll_CVCL_Auto_ios", cvc_queue_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_ios", e_queue_pat),
    ]


plot = pylib_graphs.plot("Queues", data)
plot_list.append(plot)


# Circular queues

data = [
    ("CVC Lite" , dir+"protokoll_CVCL_Auto_ios", cvc_circqueue_pat),
    ("E (good-lpo)" , dir+"protokoll_E_sts11_ios", e_circqueue_pat),
    ]


plot = pylib_graphs.plot("Circular queues", data)
plot_list.append(plot)


# E for IWIL


data = [
    ("CVC (flattened)" , dir+"protokoll_CVC_Auto", cvc_scf_pat),
    ("CVC Lite (flattened)" , dir+"protokoll_CVCL_Auto", cvc_scf_pat),
    ("E (good-lpo), built-in index type" , dir+"protokoll_E_sts11_t1", e_sc_ni_pat),
    ("E (good-lpo), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_sc_pat)
    ]

plot = pylib_graphs.plot("Storecomm, Flat, IWIL", data)
plot_list.append(plot)


data = [
    ("CVC (flattened)" , dir+"protokoll_CVC_Auto", cvc_scfinv_pat),
    ("CVC Lite (flattened)" , dir+"protokoll_CVCL_Auto", cvc_scfinv_pat),
    ("E (good-lpo), built-in index type", dir+"protokoll_E_sts11_t1", e_scinv_ni_pat),    
    ("E (good-lpo), axiomatized indices" , dir+"protokoll_E_sts11_t1", e_scinv_pat)
    ]

plot = pylib_graphs.plot("Storecomm_Invalid, Flat, IWIL", data)
plot_list.append(plot)


# Create files


for i in plot_list:
    print "Title: "+i.title+ " (linear)";
    i.gnuplot(files, None);
    print "Title: "+i.title+ " (log)";
    i.gnuplot(files, True);

