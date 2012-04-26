#!/usr/bin/env python2.7
# ----------------------------------
#
# e_scheduler.py
#
# Determine the problem class of a problem and run E using a suitable
# schedule.  
#
# Version history:
# 1.0 Mon May  8 23:34:30 CEST 2006
#     Draft version

"""
e_scheduler.py 

Usage: e_scheduler.py [Options] <tptpfile>

Read a TPTP input file, determine the problem class, select a suitable
schedule and run E accordingly.

Restriction: The initial version only reads a fixed format and does
not support extra options.

Options:

-h
 Print this information and exit.

Copyright 2006 Stephan Schulz, schulz@eprover.org

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

Dr. Stephan Schulz
Hirschstr. 35
76144 Karlsruhe
Germany

or via email (address above).
"""


import sys
import re
import string
import os
import resource


# If necessary, change this to point to your E executable!

classify_command = "classify_problem -ca-aa-aaaaaa-a --tstp-in --ax-some-limit=46 --ax-many-limit=205 --lit-some-limit=212 --lit-many-limit=620 --term-medium-limit=163 --term-large-limit=2270 --farity-medium-limit=4 --farity-large-limit=29 --gpc-few-limit=2 --gpc-many-limit=5 " 

colon_pattern = re.compile("\s*:\s*")
cl_error_exception ="Problem interpreting classify_problem output"


def get_options(argv=sys.argv[1:]):
    """
    Filter argument list for arguments starting with a -.
    """
    options = filter(lambda x:x[0:1]=="-", argv)
    return options

def get_args(argv=sys.argv[1:]):
    """
    Filter argument list for real arguments.
    """
    files   = filter(lambda x:x[0:1]!="-", argv)
    return files

           
def get_problem_class(filename):
    """
    Run classify_problem (from the E distribution) to find out the
    class of a problem. 
    """
    p = os.popen(classify_command+filename, "r");
    clres = p.readlines()
    p.close()
    res = None

    # There really should be only one line, but let's err on the safe
    # side anyways... 
    try:
        for line in clres:
	    if line.startswith(filename):
		tmp = colon_pattern.split(line)
                if len(tmp)!=3:
                    raise cl_error_exception
                res = "CLASS_"+tmp[2][:-1]
                break;
        if not res:
	    raise cl_error_exception

    except cl_error_exception:        
        sys.exit("Could not get class of " +filename);
        
    return res 
strat_options = {
"protokoll_G-E--_001_B31_F1_PI_AE_S4_CS_OS_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureSymbolWeight(ConstPrio,10,10,5,5,5,1.5,1.5,1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_001_K18_F1_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureSymbolWeight(ConstPrio,10,10,5,5,5,1.5,1.5,1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_001_K18_F1_PI_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureSymbolWeight(ConstPrio,10,10,5,5,5,1.5,1.5,1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_001_K18_F1_PI_AE_S4_CS_OS_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureSymbolWeight(ConstPrio,10,10,5,5,5,1.5,1.5,1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_001_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureSymbolWeight(ConstPrio,10,10,5,5,5,1.5,1.5,1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_001_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureSymbolWeight(ConstPrio,10,10,5,5,5,1.5,1.5,1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_002_K18_F1_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_002_K18_F1_PI_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_002_K18_F1_PI_AE_S4_CS_OS_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_003_K18_F1_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.2, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_003_K18_F1_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.2, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_003_K18_F1_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.2, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_003_K18_F1_PI_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --prefer-initial-clauses --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.2, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_003_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --prefer-initial-clauses --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.2, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_003_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --prefer-initial-clauses --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.2, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_004_K18_F1_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.7, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_004_K18_F1_PI_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --prefer-initial-clauses --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.7, 100, 100, 100, 100, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 50, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 50, 1.5, 1.5, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_006_K18_F1_PI_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 50, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_006_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 50, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_006_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 50, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_006_K19_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvmodfreqrankmax0 -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 50, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_007_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.2, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_008_B07_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_008_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_008_K18_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_008_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_008_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_B02_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Garity -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_B07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_B31_F1_PI_AE_S4_CS_SP_S00":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --prefer-initial-clauses --destructive-er -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WNoSelection -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_B31_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --prefer-initial-clauses --destructive-er -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --prefer-initial-clauses --destructive-er -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K02_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Garity -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",
"protokoll_G-E--_010_K04_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -warity -Garity -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K08_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wmodarity -Garity -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",
"protokoll_G-E--_010_K09_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -waritysquared -Garity -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K09_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -waritysquared -Garity -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_S4_AB_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr-aggressive --backward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_S4_AS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr-aggressive --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_S4_CS_SP_S00":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WNoSelection -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --prefer-initial-clauses --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_S4_CS_SP_S3S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHPExceptUniqMaxHorn -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_S4_CS_SP_S3T":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WPSelectNewComplexAHPExceptUniqMaxHorn -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F1_PI_AE_S4_CS_SP_SNG":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --prefer-initial-clauses --destructive-er -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WNoGeneration -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K18_F2_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F2 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K31_F1_PI_AE_S4_CS_SP_S00":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --prefer-initial-clauses --destructive-er -winvfreqrank -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WNoSelection -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K31_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --prefer-initial-clauses --destructive-er -winvfreqrank -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --prefer-initial-clauses --destructive-er -winvfreqrank -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K32_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K32_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_010_K32_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*ConjectureRelativeSymbolWeight(PreferNonGoals,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",
"protokoll_G-E--_011_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_011_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_011_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_B31_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_B31_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_B31_F1_PI_AE_R4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_K18_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_K33_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wprecedence -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_K33_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wprecedence -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_012_K33_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wprecedence -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(7*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 100, 1.5, 1.5, 1),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_020_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 50, 1.5, 0.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_020_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.5, 100, 100, 100, 50, 1.5, 0.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_021_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS, 100,100,100,50,50,0,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,0,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_021_K07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS, 100,100,100,50,50,0,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,0,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_021_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS, 100,100,100,50,50,0,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,0,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_021_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS, 100,100,100,50,50,0,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,0,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_021_K31_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS, 100,100,100,50,50,0,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,0,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_021_K31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS, 100,100,100,50,50,0,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,0,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_021_K32_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS, 100,100,100,50,50,0,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,0,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_021_K32_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS, 100,100,100,50,50,0,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,0,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_022_K07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,5000,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_022_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,5000,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_023_K07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,5000,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,5000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_024_B07_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,50,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_024_B31_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,50,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_024_B31_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,50,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_024_K02_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Garity -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,50,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_024_K07_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,50,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_024_K07_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,50,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_024_K07_F1_PI_AE_R4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,50,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_024_K07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,50,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_024_K18_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,50,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1000,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_K07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,5000,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,100,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_K18_F1_PI_AE_CS_SP_S00":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_K18_F1_PI_AE_R12_CS_SP_S00":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=12 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_K18_F1_PI_AE_S4_CS_SP_S00":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_K31_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_K31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_025_K33_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wprecedence -c1 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_026_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(PreferGoals,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_027_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,20,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_028_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,10,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_B31_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_B31_F1_PI_AE_T4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K02_F1_PI_AE_R4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Garity -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_IO_R4_CS_SP_S0Y":"  --definitional-cnf=24 --sos-uses-input-types --tstp-in --split-aggressive --split-clauses=4 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_Q12_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-clauses=12 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_Q2_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-clauses=2 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-clauses=4 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_Q8_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-clauses=8 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_R12_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=12 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_R4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_R7_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=7 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_R8_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-aggressive --split-clauses=8 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_S12_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=12 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_S4_CS_SP_S0Y_ne":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_S8_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=8 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_029_K18_F1_PI_AE_SU_R4_CS_SP_S0Y":"  --definitional-cnf=24 --simplify-with-unprocessed-units --tstp-in --split-aggressive --split-clauses=4 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,10,50,1.5,1.5,1),3*ConjectureGeneralSymbolWeight(PreferNonGoals,200,100,200,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_030_K16_F1_PI_AE_CS_S4_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*ConjectureGeneralSymbolWeight(PreferGroundGoals,100,100,100,50,50,10,50,1.5,1.5,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_030_K16_F1_PI_AE_CS_S4_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*ConjectureGeneralSymbolWeight(PreferGroundGoals,100,100,100,50,50,10,50,1.5,1.5,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_030_K18_F1_PI_AE_CS_S4_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -winvfreqrank -c1 -Ginvfreq --tstp-in -H'(10*ConjectureGeneralSymbolWeight(PreferGroundGoals,100,100,100,50,50,10,50,1.5,1.5,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_030_K18_F1_PI_AE_CS_S4_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --destructive-er-aggressive --destructive-er --simul-paramod --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -winvfreqrank -c1 -Ginvfreq --tstp-in -H'(10*ConjectureGeneralSymbolWeight(PreferGroundGoals,100,100,100,50,50,10,50,1.5,1.5,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_031_K18_F1_PI_AE_CS_S4_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -winvfreqrank -c1 -Ginvfreq --tstp-in -H'(10*ConjectureGeneralSymbolWeight(PreferGroundGoals,100,100,100,50,50,10,100,1.5,1.5,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_032_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,20,50,1.5,1.5,1),5*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_032_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,20,50,1.5,1.5,1),5*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_032_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*ConjectureGeneralSymbolWeight(SimulateSOS,100,100,100,50,50,20,50,1.5,1.5,1),5*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_033_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Defaultweight(SimulateSOS),5*ConjectureGeneralSymbolWeight(PreferNonGoals,100,100,100,50,50,1,100,1.5,1.5,1),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_040_K18_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.05, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_040_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.05, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_041_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),3*ConjectureRelativeSymbolWeight(ConstPrio,0.3, 100, 100, 100, 100, 2.5, 1, 1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_042_K18_F1_PI_AE_R4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(20*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_042_K18_F1_PI_AE_R7_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=7 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(20*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_042_K18_F1_PI_AE_R8_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=8 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(20*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_042_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(20*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_042_K18_F1_PI_AE_S8_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=8 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(20*ConjectureRelativeSymbolWeight(ConstPrio,0.1, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_044_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --simul-paramod --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP --tstp-in -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),2*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_044_K18_F1_PI_AE_S4_CS_OS_S1S":"  --definitional-cnf=24 --oriented-simul-paramod --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectComplexAHP --tstp-in -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),2*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_044_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --simul-paramod --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP --tstp-in -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),2*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_044_K18_F1_PI_AE_S4_S0Y":"  --definitional-cnf=24 --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred --tstp-in -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),2*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_044_K18_F1_PI_AE_S4_S1S":"  --definitional-cnf=24 --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectComplexAHP --tstp-in -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),2*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_044_K18_F1_PI_AE_S4_S2S":"  --definitional-cnf=24 --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP --tstp-in -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),2*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_044_K18_F1_PI_AE_S4_SP_S2S":"  --definitional-cnf=24 --split-aggressive --split-clauses=4 --simul-paramod --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP --tstp-in -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),2*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_B07_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_B07_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_B07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_B31_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_B31_F2_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -tLPO4 -Ginvfreqconstmin -F2 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -wconstant -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_OS_S00":"  --definitional-cnf=24 --tstp-in --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WNoSelection -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_OS_S0S":"  --definitional-cnf=24 --tstp-in --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectComplexG -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_OS_S1S":"  --definitional-cnf=24 --tstp-in --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectComplexAHP -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_OS_S2S":"  --definitional-cnf=24 --tstp-in --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_S00":"  --definitional-cnf=24 --tstp-in --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WNoSelection -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_S4_SP_S00":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WNoSelection -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S00":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WNoSelection -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S0S":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectComplexG -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S1S":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectComplexAHP -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_Q7_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --simul-paramod --split-clauses=8 --split-reuse-defs --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_S4_CS_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_S4_CS_OS_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_S4_CS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectNewComplexAHP -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_S4_OS_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --oriented-simul-paramod --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_045_K18_F1_PI_AE_S4_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-aggressive --split-clauses=4 --simul-paramod --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(4*Refinedweight(SimulateSOS,1,1,2,1.5,2),3*Refinedweight(PreferNonGoals,1,1,2,1.5,1.5),1*Clauseweight(PreferProcessed,1,1,1),1*FIFOWeight(PreferProcessed))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_050_K18_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-clauses=4 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -x'(12*SymbolTypeweight(ConstPrio,1,200,2,30,1.5,1.5,0.8),1*FIFOWeight(PreferUnits))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_050_K18_F2_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-clauses=4 --split-reuse-defs --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F2 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -x'(12*SymbolTypeweight(ConstPrio,1,200,2,30,1.5,1.5,0.8),1*FIFOWeight(PreferUnits))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_051_K18_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(SimulateSOS,0.05, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-E--_052_K18_F1_PI_AE_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --tstp-in --split-reuse-defs --split-clauses=4 --simul-paramod --forward-context-sr --destructive-er-aggressive --destructive-er --prefer-initial-clauses -winvfreqrank -c1 -Ginvfreq -F1 --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred -H'(10*ConjectureRelativeSymbolWeight(ConstPrio,0.05, 100, 100, 100, 100, 1.5, 1.5, 1.5),1*FIFOWeight(SimulateSOS))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-N--_023_B07_F1_PI_S4_CS_OS_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --oriented-simul-paramod -F1 -tLPO4 -Ginvfreq --prefer-initial-clauses --forward-context-sr -WSelectMaxLComplexAvoidPosPred --split-aggressive --split-clauses=4 --tstp-in -H'(12*Clauseweight(ConstPrio,3,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-N--_023_B07_F1_PI_S4_CS_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 -tLPO4 -Ginvfreq --prefer-initial-clauses --forward-context-sr -WSelectMaxLComplexAvoidPosPred --split-aggressive --split-clauses=4 --tstp-in -H'(12*Clauseweight(ConstPrio,3,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-N--_023_B07_F1_PI_S4_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -tLPO4 -Ginvfreq --prefer-initial-clauses --forward-context-sr -WSelectMaxLComplexAvoidPosPred --split-aggressive --split-clauses=4 --tstp-in -H'(12*Clauseweight(ConstPrio,3,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-N--_023_K07_F1_PI_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -wconstant -Ginvfreq --prefer-initial-clauses --forward-context-sr -WSelectMaxLComplexAvoidPosPred --split-reuse-defs --split-clauses=4 --tstp-in -H'(12*Clauseweight(ConstPrio,3,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-N--_023_K07_F1_PI_R4_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -wconstant -Ginvfreq --prefer-initial-clauses --forward-context-sr -WSelectMaxLComplexAvoidPosPred --split-reuse-defs --split-aggressive --split-clauses=4 --tstp-in -H'(12*Clauseweight(ConstPrio,3,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-N--_023_K07_F1_PI_S4_AB_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -wconstant -Ginvfreq --prefer-initial-clauses --forward-context-sr-aggressive --backward-context-sr -WSelectMaxLComplexAvoidPosPred --split-aggressive --split-clauses=4 --tstp-in -H'(12*Clauseweight(ConstPrio,3,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-N--_023_K07_F1_PI_S4_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -wconstant -Ginvfreq --prefer-initial-clauses --forward-context-sr -WSelectMaxLComplexAvoidPosPred --split-aggressive --split-clauses=4 --tstp-in -H'(12*Clauseweight(ConstPrio,3,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_G-N--_023_K18_F1_PI_Q4_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --forward-context-sr -WSelectMaxLComplexAvoidPosPred --split-reuse-defs --split-clauses=4 --tstp-in -H'(12*Clauseweight(ConstPrio,3,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_B31_F1_PI_AE_SP_S1U":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --destructive-er-aggressive --destructive-er -WSelectComplexAHPExceptRRHorn -tLPO4 -Ginvfreqconstmin --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_B31_F1_PI_AE_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --destructive-er-aggressive --destructive-er -WSelectNewComplexAHP -tLPO4 -Ginvfreqconstmin --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_OS_R8_SP_S1U":"  --definitional-cnf=24 --delete-bad-limit=150000000 --oriented-simul-paramod --split-aggressive --split-clauses=8 --split-reuse-defs -F1 --destructive-er-aggressive --destructive-er -WSelectComplexAHPExceptRRHorn -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_OS_S1U":"  --definitional-cnf=24 --delete-bad-limit=150000000 --oriented-simul-paramod -F1 --destructive-er-aggressive --destructive-er -WSelectComplexAHPExceptRRHorn -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_Q4_SP_SOV":"  --definitional-cnf=24 --split-clauses=4 --split-reuse-defs --delete-bad-limit=150000000 --simul-paramod -F1 --destructive-er-aggressive --destructive-er -WPSelectComplexExceptRRHorn -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -WSelectMaxLComplexAvoidPosPred -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_S2V":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -WPSelectNewComplexAHPExceptRRHorn -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_SOV":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -WPSelectComplexExceptRRHorn -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_SP_S1U":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --destructive-er-aggressive --destructive-er -WSelectComplexAHPExceptRRHorn -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --destructive-er-aggressive --destructive-er -WSelectNewComplexAHP -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_SP_S2U":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --destructive-er-aggressive --destructive-er -WSelectNewComplexAHPExceptRRHorn -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_SP_S2V":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --destructive-er-aggressive --destructive-er -WPSelectNewComplexAHPExceptRRHorn -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K07_F1_PI_AE_SP_SOV":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --destructive-er-aggressive --destructive-er -WPSelectComplexExceptRRHorn -wconstant -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_011_K18_F1_PI_AE_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 --destructive-er-aggressive --destructive-er -WSelectNewComplexAHP -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,1,0.8),8*Refinedweight(PreferNonGoals,2,1,2,3,0.8),1*Clauseweight(ConstPrio,1,1,0.7),1*FIFOWeight(ByNegLitDist))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_014_K18_F1_PI_AE_CS_SP_S2S":"  --definitional-cnf=24 --forward-context-sr --simul-paramod --delete-bad-limit=150000000 --tstp-in -winvfreqrank -c1 -Ginvfreq -F1 --destructive-er-aggressive --destructive-er --prefer-initial-clauses -WSelectNewComplexAHP -H'(12*Refinedweight(PreferGoals,1,2,2,1,0.8),12*Refinedweight(PreferNonGoals,2,1,2,3,0.8),2*ClauseWeightAge(ConstPrio,1,1,0.7,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_014_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --split-aggressive --split-clauses=4 --forward-context-sr --simul-paramod --delete-bad-limit=150000000 --tstp-in -winvfreqrank -c1 -Ginvfreq -F1 --destructive-er-aggressive --destructive-er --prefer-initial-clauses -WSelectNewComplexAHP -H'(12*Refinedweight(PreferGoals,1,2,2,1,0.8),12*Refinedweight(PreferNonGoals,2,1,2,3,0.8),2*ClauseWeightAge(ConstPrio,1,1,0.7,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_024_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --prefer-initial-clauses --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --tstp-in -H'(12*SymbolTypeweight(ConstPrio,7,20,0,0,1.5,5,0.8),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_024_K18_F1_PI_OP_AE_S4_AB_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --prefer-initial-clauses --select-on-processing-only --forward-context-sr-aggressive --backward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --tstp-in -H'(12*SymbolTypeweight(ConstPrio,7,20,0,0,1.5,5,0.8),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_024_K18_F1_PI_OP_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --prefer-initial-clauses --select-on-processing-only --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --tstp-in -H'(12*SymbolTypeweight(ConstPrio,7,20,0,0,1.5,5,0.8),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_031_K09_F1_PI_AE_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectMaxLComplexAvoidPosPred --destructive-er-aggressive --destructive-er -waritysquared -Garity --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),1*Clauseweight(ConstPrio,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_031_K18_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod --forward-context-sr -WSelectMaxLComplexAvoidPosPred -winvfreqrank -c1 -Ginvfreq --destructive-er-aggressive --destructive-er --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),1*Clauseweight(ConstPrio,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_031_K18_F1_PI_AE_CS_SP_S3T":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod --forward-context-sr -WPSelectNewComplexAHPExceptUniqMaxHorn -winvfreqrank -c1 -Ginvfreq --destructive-er-aggressive --destructive-er --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),1*Clauseweight(ConstPrio,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_031_K18_F1_PI_AE_Q4_CS_SP_S3T":"  --definitional-cnf=24 --delete-bad-limit=150000000 --split-clauses=4 --split-reuse-defs -F1 --simul-paramod --forward-context-sr -WPSelectNewComplexAHPExceptUniqMaxHorn -winvfreqrank -c1 -Ginvfreq --destructive-er-aggressive --destructive-er --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),1*Clauseweight(ConstPrio,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S":"  --definitional-cnf=24 --simul-paramod --delete-bad-limit=150000000 -WSelectNewComplexAHP --split-reuse-defs --split-aggressive --split-clauses=12 --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_036_K18_F2_PI_AE_R4_SP_S2S":"  --definitional-cnf=24 --simul-paramod --delete-bad-limit=150000000 -WSelectNewComplexAHP --split-reuse-defs --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",
"protokoll_H----_036_K18_F2_PI_AE_S4_SP_S2S":"  --definitional-cnf=24 --simul-paramod --delete-bad-limit=150000000 -WSelectNewComplexAHP --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_042_B01_F1_PI_AE_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --destructive-er-aggressive --destructive-er -tLPO4 --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_042_B03_F1_PI_AE_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectMaxLComplexAvoidPosPred --destructive-er-aggressive --destructive-er -tLPO4 -Ginvarity --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_042_B03_F1_PI_AE_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --destructive-er-aggressive --destructive-er -tLPO4 -Ginvarity --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_042_B07_F1_PI_AE_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --destructive-er-aggressive --destructive-er -tLPO4 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_042_B30_F1_PI_AE_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --destructive-er-aggressive --destructive-er -tLPO4 -Ginvfreqhack --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_042_B31_F1_PI_AE_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --destructive-er-aggressive --destructive-er -tLPO4 -Ginvfreqconstmin --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_B31_F1_PI_AE_R4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-aggressive --split-clauses=4 --split-reuse-defs --destructive-er-aggressive --destructive-er -tLPO4 -Ginvfreqconstmin --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K02_F1_PI_AE_Q12_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-clauses=12 --split-reuse-defs --destructive-er-aggressive --destructive-er -wconstant -Garity --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K02_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-clauses=4 --split-reuse-defs --destructive-er-aggressive --destructive-er -wconstant -Garity --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K18_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-clauses=4 --split-reuse-defs --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K18_F1_PI_AE_R4_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectMaxLComplexAvoidPosPred --forward-context-sr --split-aggressive --split-clauses=4 --split-reuse-defs --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K18_F1_PI_AE_R4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-aggressive --split-clauses=4 --split-reuse-defs --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K18_F1_PI_AE_R7_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-aggressive --split-clauses=7 --split-reuse-defs --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K18_F1_PI_AE_R8_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-aggressive --split-clauses=8 --split-reuse-defs --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K18_F1_PI_AE_S12_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-aggressive --split-clauses=12 --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_047_K18_F1_PI_AE_S8_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --simul-paramod -F1 -WSelectNewComplexAHP --forward-context-sr --split-aggressive --split-clauses=8 --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*PNRefinedweight(PreferGoals,1,1,1,2,2,2,0.5),10*PNRefinedweight(PreferNonGoals,2,1,1,1,2,2,2),5*OrientLMaxWeight(ConstPrio,2,1,2,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_B31_F1_PI_AE_CS_S4_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -tLPO4 -Ginvfreqconstmin --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K16_F1_PI_AE_CS_R4_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 --split-reuse-defs -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K16_F1_PI_AE_CS_R7_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=7 --split-reuse-defs -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K16_F1_PI_AE_CS_R8_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=8 --split-reuse-defs -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K16_F1_PI_AE_CS_S4_OS_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --oriented-simul-paramod --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K16_F1_PI_AE_CS_S4_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K16_F1_PI_AE_CS_S4_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectNewComplexAHP --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K16_F1_PI_AE_CS_S4_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K16_F1_PI_AE_CS_S4_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -wprecedence -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K18_F1_PI_AE_CS_Q4_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod --destructive-er-aggressive --destructive-er --split-reuse-defs --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -winvfreqrank -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_072_K18_F1_PI_AE_CS_S4_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod --destructive-er-aggressive --destructive-er --split-aggressive --split-clauses=4 -WSelectMaxLComplexAvoidPosPred --prefer-initial-clauses --forward-context-sr -winvfreqrank -c1 -Ginvfreq --tstp-in -H'(10*Refinedweight(PreferGroundGoals,2,1,2,1.0,1),1*Clauseweight(ConstPrio,1,1,1),1*Clauseweight(ByCreationDate,2,1,0.8))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_081_B02_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --prefer-initial-clauses --simul-paramod --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -F1 -tLPO4 -Garity --delete-bad-limit=150000000 -WSelectNewComplex --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,2,2),8*Refinedweight(PreferNonGoals,2,1,2,2,0.5),1*Clauseweight(PreferUnitGroundGoals,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_081_B07_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --prefer-initial-clauses --simul-paramod --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -F1 -tLPO4 -Ginvfreq --delete-bad-limit=150000000 -WSelectNewComplex --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,2,2),8*Refinedweight(PreferNonGoals,2,1,2,2,0.5),1*Clauseweight(PreferUnitGroundGoals,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_081_B31_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --prefer-initial-clauses --simul-paramod --forward-context-sr --split-reuse-defs --split-clauses=4 --destructive-er-aggressive --destructive-er -F1 -tLPO4 -Ginvfreqconstmin --delete-bad-limit=150000000 -WSelectNewComplex --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,2,2),8*Refinedweight(PreferNonGoals,2,1,2,2,0.5),1*Clauseweight(PreferUnitGroundGoals,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_081_B31_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --prefer-initial-clauses --simul-paramod --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -F1 -tLPO4 -Ginvfreqconstmin --delete-bad-limit=150000000 -WSelectMaxLComplexAvoidPosPred --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,2,2),8*Refinedweight(PreferNonGoals,2,1,2,2,0.5),1*Clauseweight(PreferUnitGroundGoals,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_081_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --prefer-initial-clauses --simul-paramod --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -F1 -tLPO4 -Ginvfreqconstmin --delete-bad-limit=150000000 -WSelectNewComplex --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,2,2),8*Refinedweight(PreferNonGoals,2,1,2,2,0.5),1*Clauseweight(PreferUnitGroundGoals,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_081_K18_F1_PI_AE_Q12_CS_SP_S2S":"  --definitional-cnf=24 --prefer-initial-clauses --simul-paramod --forward-context-sr --split-reuse-defs --split-clauses=12 --destructive-er-aggressive --destructive-er -F1 -winvfreqrank -c1 -Ginvfreq --delete-bad-limit=150000000 -WSelectNewComplex --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,2,2),8*Refinedweight(PreferNonGoals,2,1,2,2,0.5),1*Clauseweight(PreferUnitGroundGoals,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_081_K18_F1_PI_AE_Q4_CS_SP_S2S":"  --definitional-cnf=24 --prefer-initial-clauses --simul-paramod --forward-context-sr --split-reuse-defs --split-clauses=4 --destructive-er-aggressive --destructive-er -F1 -winvfreqrank -c1 -Ginvfreq --delete-bad-limit=150000000 -WSelectNewComplex --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,2,2),8*Refinedweight(PreferNonGoals,2,1,2,2,0.5),1*Clauseweight(PreferUnitGroundGoals,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_081_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --prefer-initial-clauses --simul-paramod --forward-context-sr --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -F1 -winvfreqrank -c1 -Ginvfreq --delete-bad-limit=150000000 -WSelectNewComplex --tstp-in -H'(8*Refinedweight(PreferGoals,1,2,2,2,2),8*Refinedweight(PreferNonGoals,2,1,2,2,0.5),1*Clauseweight(PreferUnitGroundGoals,1,1,1),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_B01_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -tLPO4 -Gunary_first --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_B03_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -tLPO4 -Ginvarity --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_B30_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -tLPO4 -Ginvfreqhack --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_B31_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -tLPO4 -Ginvfreqconstmin --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_K18_F1_PI_AE_S4_CS_OS_S1S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --oriented-simul-paramod -WSelectComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_K18_F1_PI_AE_S4_CS_OS_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --oriented-simul-paramod -WSelectNewComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_K18_F1_PI_AE_S4_CS_S1S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 -WSelectComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_K18_F1_PI_AE_S4_CS_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 -WSelectNewComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectMaxLComplexAvoidPosPred --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_K18_F1_PI_AE_S4_CS_SP_S1S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_H----_102_K18_F1_PI_AE_S4_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 -F1 --simul-paramod -WSelectNewComplexAHP --split-aggressive --split-clauses=4 --forward-context-sr --destructive-er-aggressive --destructive-er -winvfreqrank -c1 -Ginvfreq --prefer-initial-clauses --tstp-in -H'(10*Refinedweight(PreferGoals,1,2,2,2,0.5),10*Refinedweight(PreferNonGoals,2,1,2,2,2),3*OrientLMaxWeight(ConstPrio,2,1,2,1,1),2*ClauseWeightAge(ConstPrio,1,1,1,3))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_B07_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --simul-paramod --destructive-er-aggressive --destructive-er -F1 --forward-context-sr -tLPO4 -Ginvfreq --prefer-initial-clauses -WSelectMaxLComplexAvoidPosPred -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_B31_F1_PI_AE_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --simul-paramod --destructive-er-aggressive --destructive-er -F1 -tLPO4 -winvfreqrank --forward-context-sr --prefer-initial-clauses -WSelectNewComplexAHP -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K07_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --simul-paramod --destructive-er-aggressive --destructive-er -F1 --forward-context-sr -wconstant -Ginvfreq --prefer-initial-clauses -WSelectMaxLComplexAvoidPosPred -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K09_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --simul-paramod --destructive-er-aggressive --destructive-er -F1 --forward-context-sr -waritysquared -Garity --prefer-initial-clauses -WSelectMaxLComplexAvoidPosPred -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K18_F1_PI_AE_CS_OS_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --oriented-simul-paramod --destructive-er-aggressive --destructive-er -F1 -winvfreqrank --forward-context-sr -c1 -Ginvfreq --prefer-initial-clauses -WSelectMaxLComplexAvoidPosPred -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K18_F1_PI_AE_CS_OS_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --oriented-simul-paramod --destructive-er-aggressive --destructive-er -F1 -winvfreqrank --forward-context-sr -c1 -Ginvfreq --prefer-initial-clauses -WSelectNewComplexAHP -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K18_F1_PI_AE_CS_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --destructive-er-aggressive --destructive-er -F1 -winvfreqrank --forward-context-sr -c1 -Ginvfreq --prefer-initial-clauses -WSelectMaxLComplexAvoidPosPred -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K18_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --simul-paramod --destructive-er-aggressive --destructive-er -F1 -winvfreqrank --forward-context-sr -c1 -Ginvfreq --prefer-initial-clauses -WSelectMaxLComplexAvoidPosPred -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K18_F1_PI_AE_CS_SP_S2S":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --simul-paramod --destructive-er-aggressive --destructive-er -F1 -winvfreqrank --forward-context-sr -c1 -Ginvfreq --prefer-initial-clauses -WSelectNewComplexAHP -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K18_F1_PI_AE_S4_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --simul-paramod --split-aggressive --split-clauses=4 --destructive-er-aggressive --destructive-er -F1 -winvfreqrank --forward-context-sr -c1 -Ginvfreq --prefer-initial-clauses -WSelectMaxLComplexAvoidPosPred -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K18_F2_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --simul-paramod --destructive-er-aggressive --destructive-er -F2 -winvfreqrank --forward-context-sr -c1 -Ginvfreq --prefer-initial-clauses -WSelectMaxLComplexAvoidPosPred -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",

"protokoll_U----_043_K19_F1_PI_AE_CS_SP_S0Y":"  --definitional-cnf=24 --delete-bad-limit=150000000 --tstp-in --simul-paramod --destructive-er-aggressive --destructive-er -F1 -winvmodfreqrankmax0 -c1 -Ginvfreqhack --forward-context-sr --prefer-initial-clauses -WSelectMaxLComplexAvoidPosPred -H'(4*PNRefinedweight(PreferNonGoals,4,5,5,4,2,1,1),8*PNRefinedweight(PreferGoals,5,2,2,5,2,1,0.5),1*FIFOWeight(ConstPrio))' -s --print-statistics --print-pid --resources-info --memory-limit=192",
}   

def run_prover(schedule, filename):
    """
    Run the prover according to the schedule provided. Return
    success/failure.
    """
    res = "# SZS status: ResourceOut"
    count=1
    count2=1
    total=0
    rest=0
    
    for item in schedule:
    	count=count+1
    
    for strat in schedule:
	count2=count2+1

	total = total+strat[1]
	print strat
	if(count==count2):
		rest = 600-total
		print "Additional Time",rest


	#option = strat_options[strat[0]]
        time = int((strat[1]+rest) * e_mark_scale);
	p=os.popen("eprover "+strat_options[strat[0]]+\
                   " "+"--tstp-in --cpu-limit="+ str(time)+" "+filename, "r");
	#print "pass"
	res=p.readlines()
	p.close()
	for lines in res:
	    #print lines
	    if(lines.rstrip()=="# SZS status: Unsatisfiable" \
               or lines.rstrip()=="# SZS status: Satisfiable"\
               or lines.rstrip()=="# SZS status: Theorem"\
               or lines.rstrip()=="# SZS status: CounterSatisfiable"):
               return lines 
               break;

    return "SZS status: GaveUp"
        
        
        
# This should have one entry per class!

schedules = {
"CLASS_G-SF-FFSS11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFMM31-D":[ ("protokoll_H----_011_B31_F1_PI_AE_SP_S1U", 300.0 ) ],
"CLASS_H-NS-FFSF21-D":[ ("protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_SOV", 300.0 ) , ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S1S", 150.0 ) ],
"CLASS_H-NS-FFSF11-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-MMMS00-M":[ ("protokoll_H----_031_K18_F1_PI_AE_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SF-FFSS21-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NF-FSMF11-M":[ ("protokoll_H----_031_K09_F1_PI_AE_SP_S0Y", 300.0 ) ],
"CLASS_G-SF-SSMF32-M":[ ],
"CLASS_H-SM-FFSF22-M":[ ("protokoll_H----_042_B03_F1_PI_AE_SP_S2S", 300.0 ) ],
"CLASS_G-SF-SSMF31-M":[ ("protokoll_G-E--_029_K18_F1_PI_AE_R12_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NM-SSMS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMF31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SFMM00-M":[ ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S1S", 300.0 ) , ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 150.0 ) , ("protokoll_H----_042_B03_F1_PI_AE_SP_S0Y", 75.0 ) ],
"CLASS_G-NF-FFMF32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SS-MMLM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NS-FFSF21-M":[ ("protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_SOV", 300.0 ) , ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S1S", 150.0 ) ],
"CLASS_G-SF-SMMM31-M":[ ("protokoll_G-E--_029_K18_F1_PI_AE_R12_CS_SP_S0Y", 300.0 ) ],
"CLASS_H-NF-SFMM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SS-SSMM32-M":[ ],
"CLASS_G-SF-SSMS32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMF11-M":[ ("protokoll_H----_042_B07_F1_PI_AE_SP_S2S", 300.0 ) , ("protokoll_G-E--_010_K08_F1_PI_AE_S4_CS_SP_S0Y", 150.0 ) ],
"CLASS_H-SF-SFMF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFMS21-D":[ ("protokoll_G-E--_030_K16_F1_PI_AE_CS_S4_S0Y", 300.0 ) , ("protokoll_H----_011_K07_F1_PI_AE_SP_SOV", 150.0 ) , ("protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S00", 75.0 ) ],
"CLASS_H-SM-FFSS21-D":[ ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S1S", 300.0 ) ],
"CLASS_G-SM-SSMM33-M":[ ("protokoll_G-E--_042_K18_F1_PI_AE_R7_CS_SP_S0Y", 300.0 ) ],
"CLASS_H-SM-FFSF22-D":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_G-NF-FFMS21-M":[ ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S2S", 300.0 ) , ("protokoll_H----_042_B03_F1_PI_AE_SP_S2S", 150.0 ) , ("protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_S0Y", 75.0 ) , ("protokoll_G-E--_020_K18_F1_PI_AE_S4_CS_SP_S0Y", 37.5 ) , ("protokoll_G-E--_045_K18_F1_PI_AE_S4_CS_OS_S2S", 18.75 ) , ("protokoll_G-E--_012_K18_F1_PI_AE_CS_SP_S0Y", 9.375 ) , ("protokoll_H----_042_B30_F1_PI_AE_SP_S2S", 4.6875 ) ],
"CLASS_G-NS-FFMF32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSS21-M":[ ("protokoll_G-E--_010_K18_F1_PI_AE_S4_CS_SP_S3T", 300.0 ) ],
"CLASS_H-PM-FFSF22-M":[ ("protokoll_H----_042_B31_F1_PI_AE_SP_S2S", 300.0 ) ],
"CLASS_G-NF-FFSS00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SF-FFSM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SFMM33-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_S4_SP_S0Y", 300.0 ) ],
"CLASS_H-NM-SSMS22-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SF-FFSF33-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-FFSM00-M":[ ("protokoll_H----_011_B31_F1_PI_AE_SP_S1U", 300.0 ) ],
"CLASS_G-PF-SFMF00-M":[ ],
"CLASS_H-PM-FFSF21-D":[ ("protokoll_H----_042_B07_F1_PI_AE_SP_S2S", 300.0 ) ],
"CLASS_H-PM-FFSF21-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_G-SF-SFMM21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-SSLF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SSLF33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-FFMF32-M":[ ("protokoll_G-E--_010_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-SM-FFSF21-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_H----_102_B30_F1_PI_AE_S4_CS_SP_S2S", 150.0 ) ],
"CLASS_G-NM-SFMF22-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NF-FFMF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-SFMF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFSF21-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) , ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 150.0 ) ],
"CLASS_G-NM-FFMS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-MMLM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFSM33-D":[ ],
"CLASS_G-NS-SSMF32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NS-FFMS21-D":[ ("protokoll_G-E--_030_K16_F1_PI_AE_CS_S4_S0Y", 300.0 ) ],
"CLASS_H-SF-FFSS31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NS-FFMM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMM22-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-FFMS32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SS-FFMF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-PM-FFSF22-M":[ ("protokoll_H----_042_B07_F1_PI_AE_SP_S2S", 300.0 ) ],
"CLASS_G-SF-SSLM33-M":[ ],
"CLASS_G-SM-FFSS11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-FFMS33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMS31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-MMLM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NS-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SS-SSMS21-M":[ ("protokoll_G-E--_023_K07_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SF-FFSM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSF31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-MSMM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMM21-M":[ ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-PM-FFMF21-D":[ ("protokoll_H----_081_K18_F1_PI_AE_Q12_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NM-SFMM21-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-SFMM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NS-FFMF11-M":[ ("protokoll_G-E--_010_K08_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-PM-FFSF21-M":[ ("protokoll_G-E--_032_K18_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) , ("protokoll_H----_081_K18_F1_PI_AE_Q12_CS_SP_S2S", 150.0 ) , ("protokoll_G-E--_001_K18_F1_PI_AE_S4_CS_SP_S0Y", 75.0 ) , ("protokoll_G-E--_029_K07_F1_PI_AE_S4_CS_SP_S0Y", 37.5 ) ],
"CLASS_G-NS-FFMF11-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-PM-FFSF21-D":[ ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NF-SSMF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SMMM21-M":[ ("protokoll_G-E--_044_K18_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) , ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 150.0 ) , ("protokoll_H----_011_K18_F1_PI_AE_SP_S2S", 75.0 ) , ("protokoll_G-E--_045_B07_F1_PI_AE_Q4_CS_SP_S2S", 37.5 ) , ("protokoll_G-E--_021_K07_F1_PI_AE_S4_CS_SP_S0Y", 18.75 ) ],
"CLASS_G-PF-MSLM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SMMM21-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NM-SFMF22-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-FFMS21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFSS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SFMM32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-SFMF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-FFMS21-D":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_H-NF-FFSS21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-PF-MSLM21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-SFMF31-M":[ ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SS-FFMS21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_U-PM-FFSF32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SFMF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SF-SFMM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NS-SSMF11-M":[ ("protokoll_G-E--_010_K09_F1_PI_AE_Q4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SS-FFMF22-M":[ ("protokoll_G-E--_012_K18_F1_PI_AE_CS_SP_S0Y", 300.0 ) , ("protokoll_G-N--_023_K07_F1_PI_Q4_CS_SP_S0Y", 150.0 ) ],
"CLASS_G-NF-MMLM00-M":[ ("protokoll_G-E--_044_K18_F1_PI_AE_S4_S1S", 300.0 ) , ("protokoll_H----_011_K07_F1_PI_AE_Q4_SP_SOV", 150.0 ) , ("protokoll_H----_011_K07_F1_PI_AE_OS_S1U", 75.0 ) ],
"CLASS_H-PM-FFSS21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-PM-FFSS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PS-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFMF32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SFMF32-M":[ ("protokoll_G-E--_030_K16_F1_PI_AE_CS_S4_SP_S0Y", 300.0 ) , ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 150.0 ) , ("protokoll_H----_042_B07_F1_PI_AE_SP_S2S", 75.0 ) ],
"CLASS_G-NF-MMLS32-M":[ ],
"CLASS_H-NM-FFMS21-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_U-NS-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-MMLF21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-SF-FFSF22-M":[ ],
"CLASS_G-SF-SFMM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSF32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PF-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFMF32-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_U-PF-FFSF22-D":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMF21-M":[ ("protokoll_G-E--_024_K07_F1_PI_AE_Q4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 150.0 ) ],
"CLASS_H-SF-FFSS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SSMM33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SFMM33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFSS00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-FFMM21-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_G-PF-MSMM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SFMM31-M":[ ],
"CLASS_G-PF-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFSF31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-PM-FFMF21-D":[ ],
"CLASS_U-PM-FFMM21-D":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_U-PF-FFSF21-D":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_U-PF-FFSF32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-PF-FFSF21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_U-PF-FFSF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PF-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SF-SSMM33-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NM-MMLM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFSF33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-PF-SSMM22-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-MMMM31-M":[ ("protokoll_H----_031_K18_F1_PI_AE_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SM-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-PF-FFSS22-M":[ ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S2S", 300.0 ) ],
"CLASS_H-NF-FFSF21-D":[ ("protokoll_G-E--_021_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-SMMS00-M":[ ("protokoll_G-E--_023_K07_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_U-NF-FFSF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFSF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-MMLF11-M":[ ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S1S", 300.0 ) , ("protokoll_G-E--_023_K07_F1_PI_AE_S4_CS_SP_S0Y", 150.0 ) ],
"CLASS_H-NF-FFMM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMS22-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-PF-SFMM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-MMLM31-D":[ ],
"CLASS_G-SM-SSMM21-M":[ ("protokoll_G-E--_010_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) , ("protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S00", 150.0 ) , ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 75.0 ) ],
"CLASS_G-SF-SFMS32-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 150.0 ) ],
"CLASS_G-SF-MMLM31-M":[ ("protokoll_G-E--_023_K07_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_H-NS-FFMF21-D":[ ("protokoll_G-E--_029_K18_F1_PI_AE_R12_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SM-SSMM21-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NF-SFMS00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-PS-FFSS22-M":[ ],
"CLASS_H-NF-FFMM32-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-SFMS11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFSF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFSF22-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S1S", 300.0 ) , ("protokoll_G-E--_050_K18_F1_PI_AE_Q4_CS_SP_S0Y", 150.0 ) ],
"CLASS_H-NF-FFSF22-D":[ ("protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S00", 300.0 ) , ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 150.0 ) ],
"CLASS_G-SF-MMLM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-FFMF31-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_G-NF-MMLF00-M":[ ("protokoll_G-E--_023_K07_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NM-FFMF31-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SS-FFSM21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-FFSF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SFMS33-M":[ ("protokoll_G-E--_044_K18_F1_PI_AE_S4_S1S", 300.0 ) ],
"CLASS_G-NF-FFSF11-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SSLM31-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_CS_SP_S0Y", 300.0 ) , ("protokoll_H----_081_K18_F1_PI_AE_Q12_CS_SP_S2S", 150.0 ) , ("protokoll_H----_102_B03_F1_PI_AE_S4_CS_SP_S2S", 75.0 ) , ("protokoll_G-E--_051_K18_F1_PI_AE_Q4_CS_SP_S0Y", 37.5 ) , ("protokoll_G-E--_040_K18_F1_PI_AE_S4_CS_SP_S0Y", 18.75 ) , ("protokoll_H----_081_B31_F1_PI_AE_S4_CS_SP_S0Y", 9.375 ) ],
"CLASS_G-NF-FFSF33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMM00-M":[ ("protokoll_H----_011_B31_F1_PI_AE_SP_S1U", 300.0 ) ],
"CLASS_G-SM-MMLM21-D":[ ],
"CLASS_G-SS-SSMF32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-FFMF21-D":[ ],
"CLASS_G-SS-FFSS21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-SM-FFMM21-D":[ ("protokoll_H----_102_B30_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-FFMS00-M":[ ("protokoll_G-E--_044_K18_F1_PI_AE_S4_S0Y", 300.0 ) ],
"CLASS_G-SM-MMMM31-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_U-PM-FFSF22-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NM-SSMM31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMS21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_012_K18_F1_PI_AE_CS_SP_S0Y", 150.0 ) ],
"CLASS_G-NM-SSMM31-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SFSM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFMM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PM-FFSF22-D":[ ("protokoll_H----_042_B07_F1_PI_AE_SP_S2S", 300.0 ) ],
"CLASS_G-SM-FFMS22-M":[ ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S2S", 300.0 ) ],
"CLASS_H-PS-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-PS-FFSF21-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFMM22-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_S4_CS_OS_S2S", 300.0 ) ],
"CLASS_H-PS-FFSF22-D":[ ],
"CLASS_H-PS-FFSF21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NF-SFMM11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SFMM11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SSMM11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SF-FFMM21-M":[ ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S2S", 300.0 ) ],
"CLASS_H-NS-FFMM32-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-SM-FFSS21-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SS-FFSF22-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NS-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SSMM32-M":[ ],
"CLASS_H-NS-FFMS11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SS-SSMS32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFMF31-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_G-NS-SFMS00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SSMS32-M":[ ],
"CLASS_U-NM-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SSMM31-M":[ ("protokoll_G-E--_042_K18_F1_PI_AE_R7_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SF-SSMM31-D":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SF-SSMM32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) , ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 150.0 ) ],
"CLASS_H-NS-FFSF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SFMM32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SFMS00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SSMF32-M":[ ],
"CLASS_G-NS-FFSF21-M":[ ("protokoll_G-N--_023_B07_F1_PI_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NS-FFSF21-D":[ ],
"CLASS_G-PF-MMLM21-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 150.0 ) ],
"CLASS_G-SM-MMLM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PF-FFSM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SS-FFSF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SF-FFSS21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-SF-SSMM11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SMMM21-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-FFSS11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFMS31-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SS-FFSF22-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SM-FFMS21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SMMM21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) , ("protokoll_G-E--_021_B31_F1_PI_AE_S4_CS_SP_S2S", 150.0 ) , ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S1S", 75.0 ) , ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 37.5 ) , ("protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S00", 18.75 ) , ("protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_S0Y", 9.375 ) ],
"CLASS_H-SS-FFSF22-D":[ ("protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_H----_011_B31_F1_PI_AE_SP_S1U", 150.0 ) ],
"CLASS_H-NM-FFMS31-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S00", 300.0 ) ],
"CLASS_G-NM-FFMS31-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_U-PS-FFSF21-D":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_H----_042_B07_F1_PI_AE_SP_S2S", 150.0 ) ],
"CLASS_G-NS-FSMF11-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_S4_SP_S0Y", 300.0 ) , ("protokoll_H----_011_B31_F1_PI_AE_SP_S1U", 150.0 ) ],
"CLASS_U-PS-FFSF21-M":[ ("protokoll_U----_043_K19_F1_PI_AE_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_051_K18_F1_PI_AE_Q4_CS_SP_S0Y", 150.0 ) ],
"CLASS_G-NS-SFMM00-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_S4_SP_S0Y", 300.0 ) ],
"CLASS_G-SS-SSMS31-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NS-FFSF31-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SS-FFSF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-SSMM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFSS22-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_U-NF-FFSM32-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-FFSF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-PF-MMLM22-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SFMS31-M":[ ("protokoll_G-E--_029_K18_F1_PI_AE_R4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 150.0 ) ],
"CLASS_G-SF-SSMF21-M":[ ],
"CLASS_G-SM-MSMS32-M":[ ],
"CLASS_G-PF-MMLM22-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NM-FFMM00-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_U-PF-FFSM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-SFMM21-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-SSMF22-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-PF-FFSF22-M":[ ],
"CLASS_U-NS-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-FFMF22-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PF-FFSS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PS-FFSF22-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_052_K18_F1_PI_AE_Q4_CS_SP_S0Y", 150.0 ) ],
"CLASS_G-SM-FFMF22-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMM31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PS-FFSF22-D":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_024_B07_F1_PI_AE_Q4_CS_SP_S0Y", 150.0 ) ],
"CLASS_U-SM-SFMM21-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SM-MMLM21-M":[ ("protokoll_G-E--_042_K18_F1_PI_AE_R7_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SM-FFMF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-SFMM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NS-FFSS00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SFMF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-MMLM31-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-MMLS31-D":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NS-FFMF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SSMM11-D":[ ],
"CLASS_H-NS-FFSF11-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NS-FFMM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NS-FFMF21-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-FFSS21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-SM-SFMM22-M":[ ("protokoll_G-E--_023_K07_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMF21-M":[ ("protokoll_H----_042_B07_F1_PI_AE_SP_S2S", 300.0 ) ],
"CLASS_G-SM-MSMM32-M":[ ],
"CLASS_G-SM-SSMF31-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NS-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SFMS21-D":[ ("protokoll_G-E--_042_K18_F1_PI_AE_R8_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMM32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PF-FFSF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMF00-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_H-PM-FFMM21-D":[ ],
"CLASS_H-NF-FFMF21-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_U-PS-FFSS21-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFMF21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SM-SSMS33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFSM11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSS32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SSMS31-M":[ ("protokoll_G-E--_042_K18_F1_PI_AE_R7_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMM21-M":[ ("protokoll_H----_102_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) , ("protokoll_G-E--_010_K18_F1_PI_AE_S4_CS_SP_S3S", 150.0 ) , ("protokoll_G-E--_021_B31_F1_PI_AE_S4_CS_SP_S2S", 75.0 ) ],
"CLASS_H-NM-FFMF33-M":[ ],
"CLASS_G-NF-MMLM32-M":[ ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NS-FFSF33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-SFMM31-M":[ ],
"CLASS_H-NM-SFMM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFSS11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-FFMS11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-SFMM31-D":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SS-FFMS33-M":[ ("protokoll_G-E--_024_B31_F1_PI_AE_Q4_CS_SP_S0Y", 300.0 ) , ("protokoll_H----_042_B03_F1_PI_AE_SP_S0Y", 150.0 ) ],
"CLASS_G-SF-FFSF21-M":[ ("protokoll_H----_081_B02_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-FFSF22-D":[ ],
"CLASS_H-NS-FFSS11-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NS-FFMS21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SF-SSMM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-PF-SSLM22-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-MMLS00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PM-FFSS21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFSF32-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NM-FFMF32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SSMM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SF-FFSS32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-MMLS31-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-SF-FFSF21-M":[ ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-SSMM00-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S1S", 300.0 ) ],
"CLASS_U-NF-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-NF-FFSF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-FFSF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SMLM33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-FFSF21-D":[ ],
"CLASS_G-SM-SFMM31-D":[ ("protokoll_G-E--_042_K18_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 150.0 ) , ("protokoll_G-E--_001_B31_F1_PI_AE_S4_CS_OS_S2S", 75.0 ) ],
"CLASS_G-NM-FFMS32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFSS32-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-FFSM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PM-FFSS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFMM21-D":[ ("protokoll_G-E--_024_B31_F1_PI_AE_Q4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NM-FFMM32-M":[ ],
"CLASS_H-NM-FFMM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SMMM00-M":[ ("protokoll_G-E--_022_K18_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_008_K18_F1_PI_AE_CS_SP_S0Y", 150.0 ) , ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 75.0 ) , ("protokoll_U----_043_K19_F1_PI_AE_CS_SP_S0Y", 37.5 ) ],
"CLASS_G-SM-SFMM21-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMM33-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NF-FFSM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SSMF11-M":[ ("protokoll_G-E--_032_K18_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) , ("protokoll_G-E--_010_K08_F1_PI_AE_S4_CS_SP_S0Y", 150.0 ) ],
"CLASS_G-NF-SSMM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SS-FFSS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SSMS32-M":[ ("protokoll_H----_024_K18_F1_PI_OP_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-MMLM11-M":[ ],
"CLASS_H-NF-FFSM21-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_G-SF-MMMM21-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-SFMF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PM-FFMS21-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_H----_042_B07_F1_PI_AE_SP_S2S", 150.0 ) , ("protokoll_H----_081_B07_F1_PI_AE_S4_CS_SP_S2S", 75.0 ) ],
"CLASS_H-SM-FFMS21-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_S4_CS_OS_S2S", 300.0 ) ],
"CLASS_G-SM-SSMS31-D":[ ("protokoll_G-E--_030_K16_F1_PI_AE_CS_S4_S0Y", 300.0 ) , ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 150.0 ) ],
"CLASS_G-SF-MMMM21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) , ("protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S00", 150.0 ) ],
"CLASS_U-NF-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NF-SFMF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SMMM31-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_G-SF-FFMS31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFMF21-M":[ ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NS-FFMS21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-MMLM33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFMF21-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFMF22-M":[ ],
"CLASS_H-SS-FFSS21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-SFMF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NS-FFMS22-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_H-NF-FFMF00-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_CS_SP_S00", 300.0 ) ],
"CLASS_G-PM-FFSS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMS32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SMLF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SSMF33-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_U-PM-FFMF22-D":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PM-FFMF21-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSF21-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PM-FFMF21-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_H----_042_B07_F1_PI_AE_SP_S2S", 150.0 ) ],
"CLASS_H-SF-SFMS11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMF32-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_H-SF-SSMM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFMM31-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_H-SF-SFMM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SS-FFMF33-M":[ ("protokoll_G-E--_020_K18_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) , ("protokoll_G-E--_004_K18_F1_AE_S4_CS_OS_S0Y", 150.0 ) , ("protokoll_G-E--_002_K18_F1_PI_AE_S4_CS_OS_S0Y", 75.0 ) ],
"CLASS_G-NF-FFMM33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SMMF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SF-FFMS32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NM-FFMF21-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NF-SFSS00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-PS-SFMM21-M":[ ("protokoll_H----_102_K18_F1_PI_AE_S4_CS_S2S", 300.0 ) ],
"CLASS_G-NM-FFMF21-D":[ ("protokoll_G-E--_010_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-MMLM21-M":[ ("protokoll_G-E--_003_K18_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-SFMS21-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-MMLM21-D":[ ],
"CLASS_G-SF-MMLM22-M":[ ("protokoll_G-E--_021_K07_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSF22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMF31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-PF-FFMF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-SM-FFMS22-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NF-MMMM00-M":[ ("protokoll_G-E--_033_K18_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_010_K09_F1_PI_AE_Q4_CS_SP_S0Y", 150.0 ) ],
"CLASS_H-NM-SFMS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PM-FFMF32-M":[ ],
"CLASS_G-SM-FFMM31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SSLS33-M":[ ("protokoll_H----_031_K18_F1_PI_AE_CS_SP_S3T", 300.0 ) ],
"CLASS_G-NM-SSMM22-D":[ ("protokoll_G-E--_023_K07_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-PF-FFSF00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFMF33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-FFSM11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFMF22-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SM-FFMF33-M":[ ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NM-FFSF21-M":[ ("protokoll_U----_043_K18_F1_PI_AE_CS_OS_S0Y", 300.0 ) , ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 150.0 ) , ("protokoll_G-E--_003_K18_F1_PI_AE_S4_CS_SP_S2S", 75.0 ) ],
"CLASS_G-NF-SFMS11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SSMM32-M":[ ],
"CLASS_H-NM-FFMF21-D":[ ("protokoll_H----_024_K18_F1_PI_OP_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NF-FFSF11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFMF21-M":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_H-NF-FFSF11-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SM-FFMM32-M":[ ],
"CLASS_G-NS-FFMS00-M":[ ("protokoll_H----_011_B31_F1_PI_AE_SP_S1U", 300.0 ) ],
"CLASS_H-NM-SFMF33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-PM-FFSM22-M":[ ("protokoll_G-E--_012_K31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SM-MMMM21-M":[ ("protokoll_G-E--_052_K18_F1_PI_AE_Q4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NF-SSMS11-M":[ ("protokoll_G-E--_045_K18_F1_PI_AE_S4_SP_S0Y", 300.0 ) ],
"CLASS_G-SM-MMMM21-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SM-SSMM31-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-FFMM31-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SSMM31-D":[ ("protokoll_H----_102_B01_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-SF-SFMM11-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SFMM31-M":[ ("protokoll_G-E--_008_B07_F1_PI_AE_Q4_CS_SP_S2S", 300.0 ) ],
"CLASS_U-PM-FFSF21-D":[ ("protokoll_G-E--_029_K18_F1_PI_AE_R12_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NM-SFMS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFSS22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_U-PM-FFSF21-M":[ ("protokoll_H----_081_K18_F1_PI_AE_Q12_CS_SP_S2S", 300.0 ) , ("protokoll_H----_102_B01_F1_PI_AE_S4_CS_SP_S2S", 150.0 ) ],
"CLASS_G-NM-SFMS22-D":[ ("protokoll_H----_036_K18_F2_PI_AE_R12_SP_S2S", 300.0 ) ],
"CLASS_H-NS-FFSF22-D":[ ("protokoll_G-E--_023_K07_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) , ("protokoll_G-E--_024_B07_F1_PI_AE_Q4_CS_SP_S0Y", 150.0 ) ],
"CLASS_G-NF-SFMM22-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SFMM33-M":[ ("protokoll_H----_081_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-SFMM21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SSMS31-D":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) , ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 150.0 ) ],
"CLASS_H-NM-SFMM31-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-SSMS31-M":[ ("protokoll_G-E--_012_B31_F1_PI_AE_Q4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-NF-SFMS32-M":[ ("protokoll_H----_102_B03_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SS-FFMM33-M":[ ],
"CLASS_H-SS-FFSM22-M":[ ("protokoll_G-E--_021_B31_F1_PI_AE_S4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-SF-MMLF33-M":[ ],
"CLASS_G-PS-FFMM21-M":[ ("protokoll_G-E--_029_K18_F1_PI_AE_R12_CS_SP_S0Y", 300.0 ) ],
"CLASS_H-SF-FFMM32-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_G-NF-FFMM32-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NS-FFMM21-D":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-NF-SSMS00-M":[ ("protokoll_H----_011_K07_F1_PI_AE_OS_S1U", 300.0 ) ],
"CLASS_H-NM-FFSF22-M":[ ("protokoll_G-E--_008_K18_F1_PI_AE_R4_CS_SP_S2S", 300.0 ) ],
"CLASS_H-NM-FFSF21-D":[ ("protokoll_H----_072_K16_F1_PI_AE_CS_S4_S0Y", 300.0 ) , ("protokoll_G-E--_010_K08_F1_PI_AE_S4_CS_SP_S0Y", 150.0 ) , ("protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_S0Y", 75.0 ) , ("protokoll_H----_011_K07_F1_PI_AE_S4_CS_SP_SOV", 37.5 ) , ("protokoll_H----_011_K07_F1_PI_AE_OS_S1U", 18.75 ) ],
"CLASS_U-NF-FFSM00-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_H-NM-FFSF22-D":[ ("protokoll_G-E--_012_B31_F1_PI_AE_S4_CS_SP_S0Y", 300.0 ) ],
"CLASS_G-SS-SSMF21-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SM-SFMF33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"CLASS_G-SF-FFMF33-M":[ ("protokoll_G-E--_005_K18_F1_PI_AE_S4_CS_OS_S0Y", 300.0 ) ],
"DEFAULT":[("protokoll_G-E--_029_K18_F1_PI_AE_Q4_CS_SP_S0Y",300.0)],
}

if __name__ == '__main__':
    for option in get_options():
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    e_mark_scale =  100.0 / 285.646;
    args = get_args()
    if len(args)!=1:
        print "Exactly one file name argument expected"
        sys.exit(1);
        
    cl = get_problem_class(args[0])
    try:
	#cl="vshsjhfsfhsfjhsfshhh"
   	schedule = schedules[cl]
	if not schedule: 
		raise schedule_unavailable
	
    except :
               schedule = schedules["DEFAULT"];

 
    res = run_prover(schedule, args[0])
    print res
    resuse = resource.getrusage(resource.RUSAGE_CHILDREN)
    print """# -------------------------------------------------
# User time                : %f s
# System time              : %f s
# Total time               : %f s
# Maximum resident set size: %d pages
""" % (resuse.ru_utime/e_mark_scale, resuse.ru_stime/e_mark_scale, (resuse.ru_utime+resuse.ru_stime)/e_mark_scale,resuse.ru_maxrss)

    
