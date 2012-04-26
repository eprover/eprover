#!/usr/bin/env python2.7
# ----------------------------------
#
# prot_discretizer.py
#
# Usage: prot_discretizer.py
#
# Read test protocols and round run times upwards towards
# discrete values selectable  via command line switches:
#
# --constant: Set all times to 0.
# --equidist[=<val>]: Round all values to the nearest integral
#   multiple of <val>. If <val> is omitted, assumes 10.
# --prop: Round all times to 1,2,5,10,20,50,100,...
#

import sys
import re
import string
from pylib_discretize import *
import pylib_io
import pylib_eprots

ed_opt = "--equidist"
ed_optspc = ed_opt+"="
ed_spclen = len(ed_optspc);


round_fun = equidist_round(10)

options = pylib_io.get_options()
files   = pylib_io.get_args()


for opt in options:
    if opt == "--constant":
        round_fun = const_round
    elif opt == "--prop":
        round_fun = prop_round
    elif opt == "--none":
        round_fun = no_round
    elif opt == ed_opt:
        round_fun = equidist_round(10)
    elif opt[0:ed_spclen] == ed_optspc:
        try:
            round_fun = equidist_round(int(opt[ed_spclen:]))
        except ValueError:
            sys.stderr.write("Wrong argument to option "+equidist+".\n")
            raise
    else:
        sys.stderr.write("Unknown option "+opt+".\n")
        raise "OptionError"
    

for file in files:
    f=open(file, "r")
    lines = f.readlines()
    res = map(pylib_eprots.process_line(round_fun), lines);
    for line in res:
        print line
        
