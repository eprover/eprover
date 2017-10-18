#!/usr/bin/env python

"""
bsub_eprover 0.1

Usage: bsub_eprover.py <stratfile> ...

Parse the strategy description files provided, generate corresponding
LSF batch files, and submit them to the LSF system for running. This
is, more or less, a one-off hack and not particularly configurable
except by editing the source code.

Options:

-h
   Print this help.

-v
--verbose
   Switch on verbose mode.

-p
--pegasus
   Switch to Pegasus config.

-P
--pegasus6
    Switch to Pegasus config with TPTP 6.0.0.

-f
--force
   Force scheduling even of jobs already running.

-b
--batchsize
   Determine how many jobs to put into one batch.

-j
--jobs
   Determine maximal number of jobs to schedule.

-x
--ext-stats
   Collect extendet statistics (including paramodulations, processed
   clauses, ...).

-X
--ext2-stats
   Collect extended stats and profiling information (assumes the
   prover is compiled with internal profiling support enabled).

-Y
--ext3-stats
  Collect even more extended stats.
   
-Z
--extFV-stats
  Collect FV-Indexing stats.

--ext5-stats
  Collect condensation stats.

Copyright 2010-2011 Stephan Schulz, schulz@eprover.org

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

The original copyright holder can be contacted as

Stephan Schulz (I4)
Technische Universitaet Muenchen
Institut fuer Informatik
Boltzmannstrasse 3
Garching bei Muenchen
Germany

or via email (address above).
"""

import unittest
import sys
import re
import os
import string
import getopt
import pylib_io
import pylib_db
import pylib_eprot
import pylib_erun


def test_cfg():
    global tptp_dir, eprover_dir, testrun_dir, bsub_cmd, bjobs_cmd,\
    bsub_rundir, old_job_dir, db_file, getcpu
    
    tptp_dir = "/Users/schulz/EPROVER/TPTP_4.1.0_FLAT"
    """
    Where are the TPTP problem files?
    """

    eprover_dir = "/Users/schulz/SOURCES/Projects/E/PROVER"
    """
    Where is the eprover binary?
    """

    testrun_dir = "/Users/schulz/EPROVER/TESTRUNS_SERVER"
    """
    Where are spec- and protocol files?
    """

    bsub_cmd="head -2"
    """
    Name of the batch submission program (normally bsub, but we use cat
    for testing).
    """

    bjobs_cmd="cat EXAMPLE_CFG/bjobs.txt"
    """
    Command to list jobs in queue. Will be bjobs -w on the cluster.
    """

    bsub_rundir = "/tmp"
    """
    Where will E run and deposit results?
    """

    old_job_dir = "/tmp/schulz/e_jobs_done"
    """
    Where to move old result files (if any).
    """

    db_file = "EXAMPLE_CFG/bjob_db.db"
    """
    Where to store the job/run associations.
    """

    getcpu = ""
    """
    Try to extract CPU information.
    """

def pegasus_cfg():
    global tptp_dir, eprover_dir, testrun_dir, bsub_cmd, bjobs_cmd,\
    bsub_rundir, old_job_dir, db_file, getcpu
    
    tptp_dir = "/nethome/sschulz/EPROVER/TPTP_5.4.0_FLAT"
    """
    Where are the TPTP problem files?
    """

    eprover_dir = "/nethome/sschulz/bin"
    """
    Where is the eprover binary?
    """

    testrun_dir = "/nethome/sschulz/EPROVER/TESTRUNS_PEGASUS"
    """
    Where are spec- and protocol files?
    """

    bsub_cmd="bsub"
    """
    Name of the batch submission program (normally bsub, but we use cat
    for testing).
    """

    bjobs_cmd="bjobs -w"
    """
    Command to list jobs in queue. Will be bjobs -w on the cluster.
    """

    bsub_rundir = "/nethome/sschulz/RUN"
    """
    Where will E run and deposit results?
    """

    old_job_dir = "/scratch/sschulz/e_jobs_done"
    """
    Where to move old result files (if any).
    """

    db_file = "/nethome/sschulz/RUN/bjob_db.db"
    """
    Where to store the job/run associations.
    """

    getcpu = "-C"
    """
    Try to extract CPU information.
    """


def pegasus6_cfg():
    global tptp_dir, eprover_dir, testrun_dir, bsub_cmd, bjobs_cmd,\
    bsub_rundir, old_job_dir, db_file, getcpu
    
    tptp_dir = "/nethome/sschulz/EPROVER/TPTP_6.1.0_FLAT"
    """
    Where are the TPTP problem files?
    """

    eprover_dir = "/nethome/sschulz/bin"
    """
    Where is the eprover binary?
    """

    testrun_dir = "/nethome/sschulz/EPROVER/TESTRUNS_PEGASUS"
    """
    Where are spec- and protocol files?
    """

    bsub_cmd="bsub"
    """
    Name of the batch submission program (normally bsub, but we use cat
    for testing).
    """

    bjobs_cmd="bjobs -w"
    """
    Command to list jobs in queue. Will be bjobs -w on the cluster.
    """

    bsub_rundir = "/nethome/sschulz/RUN"
    """
    Where will E run and deposit results?
    """

    old_job_dir = "/scratch/sschulz/e_jobs_done"
    """
    Where to move old result files (if any).
    """

    db_file = "/nethome/sschulz/RUN/bjob_db.db"
    """
    Where to store the job/run associations.
    """

    getcpu = "-C"
    """
    Try to extract CPU information.
    """


def pegasus_sine_cfg():
    global tptp_dir, eprover_dir, testrun_dir, bsub_cmd, bjobs_cmd,\
    bsub_rundir, old_job_dir, db_file, getcpu

    tptp_dir = "/nethome/sschulz/EPROVER/TPTP_4.1.0_SINE"
    """
    Where are the TPTP problem files?
    """

    eprover_dir = "/nethome/sschulz/bin"
    """
    Where is the eprover binary?
    """

    testrun_dir = "/nethome/sschulz/EPROVER/TESTRUNS_PEGASUS"
    """
    Where are spec- and protocol files?
    """

    bsub_cmd="bsub"
    """
    Name of the batch submission program (normally bsub, but we use cat
    for testing).
    """

    bjobs_cmd="bjobs -w"
    """
    Command to list jobs in queue. Will be bjobs -w on the cluster.
    """

    bsub_rundir = "/nethome/sschulz/RUN"
    """
    Where will E run and deposit results?
    """

    old_job_dir = "/scratch/sschulz/e_jobs_done"
    """
    Where to move old result files (if any).
    """

    db_file = "/nethome/sschulz/RUN/bjob_db.db"
    """
    Where to store the job/run associations.
    """
    
    getcpu = "-C"
    """
    Try to extract CPU information.
    """




max_bsub_jobs = 900
"""
How many jobs are allowed at the same time.
"""

max_bsub_batch = 100
"""
How many prover runs are combined in one batch.
"""

cores = 16
"""
How many jobs to run in parallel
"""

res_complete_marker = re.compile("### Job complete ###")
"""
Result files are considered complete if they contain this string.
"""


bsub_header_tmplt = \
"""
#!/bin/bash
#BSUB -J %s
#BSUB -o %s.out   
#BSUB -e %s.err  
#BSUB -W 15:00
#BSUB -q small 
#BSUB -n %d      
#BSUB -R "span[ptile=%d]"
#
# Run prunner running multiple jobs in parallel
cd %s
%s/prunner.py %s -c %d << EOF
"""
"""
Template for generating bsub job headers.
"""

bsub_job_tmplt = \
"""
echo "### Running: %s";env TPTP=%s %s/eprover --print-version --print-statistics -s --tptp3-format --resources-info %s %s
"""
"""
Template for generating bsub job entries.
"""

ejobname_re=re.compile("erun#[^ ]*.out")
"""
Regular expression matching E job names.
"""

ejob_re=re.compile("erun#[^ ]*")
"""
Regular expression matching E jobs.
"""

ejobhead_str="\n### Running:"


def bsub_submit(job):
    """
    Submit a job to bsub.
    """
    os.chdir(bsub_rundir)
    fp = os.popen(bsub_cmd, "w")
    fp.write(job)
    fp.close()


def encode_res_name(stratname, probname):
    """
    Given a strategy name and the problem name, return the job name.
    """
    return "erun#%s#%s"%(stratname, probname)

def decode_res_name(filename):
    """
    Results for a given test run are stored in a file whose name
    includes the name of the tested strategy and the name of the
    problem. This function extracts these components and returns them
    as a tuple (stratname, probname).
    """
    if filename.endswith(".err") or filename.endswith(".out"):
        filename = filename[:-4]
    
    tmp = filename.split("#")
    if len(tmp) != 3:
        print "decode_res_name: Unexpected file name ", filename
        return None
    return tmp[1], tmp[2]
    

def find_batch_jobs(job_db):
    """
    Find all jobs in the job queue and return them in the form of a
    set. 
    """
    res = set()
    count = 0;
    
    txt = pylib_io.run_shell_command(bjobs_cmd)
    for i in txt:
        mo = ejob_re.search(i)
        if mo:
            count = count+1
            job = mo.group()
            assoc = job_db.find_entry(job)
            if assoc:
                res.update(assoc)
            else:
                res.add(job)

    return count,res


def read_res_file(filename):
    """
    Try to open and read the named file. Return the contents if
    successful and the file contains the string
    '### Job complete ###', None otherwise.
    """
    try:
        fp = pylib_io.flexopen(filename, "r")
    except IOError:
        return None
    res = fp.read()

    mo = res_complete_marker.search(res)
    if mo:
        return res
    return None


def decode_result(decoder, stratset, filename):
    """
    Parse a result file and insert it into stratset. Return true on
    success, false otherwise.
    """
    resstr = read_res_file(filename)
    if resstr:
        try:
            sname, pname = decode_res_name(filename)
        except TypeError:
            return False
        strat = stratset.find_strat(sname)
        maxtime = strat.spec.time_limit
        res = pylib_eprot.eresult(pname+" "+decoder.encode_result(resstr,maxtime))
        if res:
            strat.add_result(res)
            return True
        return False
    else:
        return False
        
def decode_results(decoder, stratset, filename):
    """
    Parse a batch result file and insert the results into
    stratset. 
    """
    resstr = read_res_file(filename)
    if resstr:
        reslist = resstr.split(ejobhead_str)
        for i in reslist[1:]:
            try:
                filename, rest = i.split("\n", 1)            
                filename = filename.strip()
                sname, pname = decode_res_name(filename)
                strat = stratset.find_strat(sname)
                maxtime = strat.spec.time_limit
                res = pylib_eprot.eresult(pname+" "+decoder.encode_result(rest,maxtime))
                if res:
                    strat.add_result(res)
            except ValueError:
                pass
        return True
    else:
        return False


   
def process_complete_jobs(decoder, stratset, job_db, resdir = ""):
    """
    Process all jobs in resdir. Store the results in stratset, move
    processed complete files to donedir (if this argument is given).
    """
    os.chdir(resdir)
    names = os.listdir(".")

    for job in names:
        if ejobname_re.match(job):
            pylib_io.verbout("Parsing "+job);
            res = decode_results(decoder, stratset, job)
            if res:
                pylib_io.verbout("Found "+job+" complete");
                job_db.del_entry(job)
                joberr = job[:-4]+".err"
                try:
                    os.unlink(job)
                    os.unlink(joberr)
                except OSError:
                    pass

def encode_job_name(job):
    """
    Given a job, return the unique name of the running job.
    """
    stratname = job.strat()
    probname  = job.prob()
    jname = encode_res_name(stratname, probname)
    return jname

def bsub_gen_header(job):
    """
    Generate a bsub specification header.
    """
    jname = encode_job_name(job)
    return bsub_header_tmplt%\
           (jname, jname, jname, cores, cores, bsub_rundir, eprover_dir, getcpu, cores)


def bsub_gen_job(job):
    """
    Generate a bsub single job command.
    """
    probname  = job.prob()
    args      = job.get_args()
    jname     = encode_job_name(job)
    return bsub_job_tmplt%\
           (jname,  tptp_dir, eprover_dir, args, probname)

             
def bsub_gen_batch(batch):
    """
    Generate a batch description for bsub to run the jobs in batch.
    """
    header = bsub_gen_header(batch[0])

    joblist = [bsub_gen_job(i) for i in batch]
    jobs = "\n".join(joblist)
    
    footer = '\nEOF\necho "### Job complete ###"\n\n'

    return header+jobs+footer


def process_strat(name, results, running, max_jobs, job_db):
    """
    Determine the strategy base name, parse any results (individually
    and collected), determine the missing problems, and submit them
    via bsub.
    """
    strat = results.find_strat(name)

    open_jobs = []
    for job in strat.generate_jobs():
        jname = encode_res_name(job.strat(), job.prob())
        if not jname in running:
            open_jobs.append(job)

    print "Found ", len(open_jobs), " open jobs in ", name

    res = 0
    while len(open_jobs) and res < max_jobs:
        batch     = open_jobs[:max_bsub_batch]
        open_jobs = open_jobs[max_bsub_batch:]
        res = res+1
        
        job_str = bsub_gen_batch(batch)
        jname = encode_job_name(batch[0])
        print "Submitting ", jname, " with ", len(batch), " problems"
        bsub_submit(job_str)
        namelist = [encode_job_name(j) for j in batch]
        job_db.add_entry(jname, namelist)

    job_db.sync()
    return res


stats = [
    "# Version",
    "# Processed clauses"
    ]

x_stats = [
    "# Unification attempts",
    "# Unification successes",
    "# Generated clauses",
    "# Paramodulations",
    "# Current number of processed clauses",
    "# Current number of unprocessed clauses"
    ]

x2_stats = [
    "# PC(MguTimer)",
    "# PC(SatTimer)",
    "# PC(ParamodTimer)",
    "# PC(PMIndexTimer)",
    "# PC(BWRWTimer)",
    "# PC(BWRWIndexTimer)",
    "# Backwards rewriting index",
    "# Paramod-from index",
    "# Paramod-into index"
    ]

x3_stats = [
    "# BW rewrite match attempts",
    "# BW rewrite match successes"    
    ]

x4_stats = [
    "# Clause-clause subsumption calls (NU)",
    "# Rec. Clause-clause subsumption calls",
    "# Unit Clause-clause subsumption calls",
    "# ...subsumed",
    "# Backward-subsumed",
    "# PC(MguTimer)",
    "# PC(SatTimer)",
    "# PC(ParamodTimer)",
    "# PC(PMIndexTimer)",
    "# PC(BWRWTimer)",
    "# PC(BWRWIndexTimer)",
    "# PC(FreqVecTimer)",
    "# PC(FVIndexTimer)",
    "# PC(SubsumeTimer)",
    "# PC(SetSubsumeTimer)"
    ]

x5_stats = [
    "# Condensation attempts",
    "# Condensation successes"    
    ]

x6_stats = [
    "# cpu MHz",
    "# bogomips"
    ]


class TestDecoding(unittest.TestCase):
    """
    Test result decoding.
    """
    def setUp(self):
        """        
        """
        self.teststr = """
# Proof found!
# SZS status Unsatisfiable
# Parsed axioms                        : 11
# Removed by relevancy pruning         : 0
# Initial clauses                      : 11
# Removed in clause preprocessing      : 0
# Initial clauses in saturation        : 11
# Processed clauses                    : 198
# ...of these trivial                  : 44
# ...subsumed                          : 104
# ...remaining for further processing  : 50
# Other redundant clauses eliminated   : 0
# Clauses deleted for lack of memory   : 0
# Backward-subsumed                    : 1
# Backward-rewritten                   : 25
# Generated clauses                    : 891
# ...of the previous two non-trivial   : 566
# Contextual simplify-reflections      : 0
# Paramodulations                      : 891
# Factorizations                       : 0
# Equation resolutions                 : 0
# Current number of processed clauses  : 24
#    Positive orientable unit clauses  : 19
#    Positive unorientable unit clauses: 5
#    Negative unit clauses             : 0
#    Non-unit-clauses                  : 0
# Current number of unprocessed clauses: 164
# ...number of literals in the above   : 164
# Clause-clause subsumption calls (NU) : 0
# Rec. Clause-clause subsumption calls : 0
# Unit Clause-clause subsumption calls : 20
# Rewrite failures with RHS unbound    : 0
# BW rewrite match attempts            : 144
# BW rewrite match successes           : 95
# Unification attempts                 : 875
# Unification successes                : 805
# PC(MguTimer)                         : 0.000910
# PC(SatTimer)                         : 0.011210
# PC(ParamodTimer)                     : 0.004687
# PC(PMIndexTimer)                     : 0.000372
# PC(IndexUnifTimer)                   : 0.000420
# PC(BWRWTimer)                        : 0.000749
# PC(BWRWIndexTimer)                   : 0.000275
# PC(IndexMatchTimer)                  : 0.000108
# PC(FreqVecTimer)                     : 0.000263
# PC(FVIndexTimer)                     : 0.000139
# PC(SubsumeTimer)                     : 0.000014
# PC(SetSubsumeTimer)                  : 0.000122
# Backwards rewriting index :    97 nodes,    18 leaves,   2.28+/-2.076 terms/leaf
# Paramod-from index        :    79 nodes,    15 leaves,   1.80+/-1.327 terms/leaf
# Paramod-into index        :    86 nodes,    16 leaves,   2.19+/-1.911 terms/leaf
# Paramod-neg-atom index    :     1 nodes,     0 leaves,   0.00+/-0.000 terms/leaf

# -------------------------------------------------
# User time                : 0.013 s
# System time              : 0.005 s
# Total time               : 0.019 s
# Maximum resident set size: 2383872 pages
"""
        
    def testDecode(self):
        """
        """
        decoder = pylib_erun.e_res_parser(300, stats+x_stats+x4_stats)
        tmp = decoder.translate_result(self.teststr)
        self.assertEqual(3+len(stats+x_stats+x4_stats), len(tmp))
        

if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "uhvpPsfb:j:xXYZCc:",
                                   ["unit-test",
                                    "pegasus",
                                    "pegasus6",
                                    "peg-sine",
                                    "force",
                                    "batchsize=",
                                    "jobs="
                                    "ext-stats",
                                    "ext2-stats",
                                    "ext3-stats",
                                    "extFV-stats",
                                    "ext5-stats",
                                    "cpu-stats",
                                    "cores="])
    force_scheduling = False

    test_cfg()
    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        elif option == "-v":
            pylib_io.Verbose = 1
        elif option == "-u" or option == "--unit-test":
            sys.argv = sys.argv[:1]
            unittest.main()
        elif option == "-p" or option == "--pegasus":
            pegasus_cfg()
        elif option == "-P" or option == "--pegasus6":
            pegasus6_cfg()
        elif option == "-s" or option == "--peg-sine":
            pegasus_sine_cfg()
        elif option == "-f" or option == "--force":
            force_scheduling = True
        elif option =="-b" or option =="--batchsize":
            max_bsub_batch = int(optarg)
        elif option =="-j" or option =="--jobs":
            max_bsub_jobs = int(optarg)
        elif option == "-x" or option == "--ext-stats":
            stats.extend(x_stats)
        elif option == "-X" or option == "--ext2-stats":
            stats.extend(x_stats)
            stats.extend(x2_stats)
        elif option == "-Y" or option == "--ext3-stats":
            stats.extend(x_stats)
            stats.extend(x2_stats)
            stats.extend(x3_stats)
        elif option == "-Z" or option == "--extFV-stats":
            stats.extend(x_stats)
            stats.extend(x4_stats)
        elif  option == "--ext5-stats":
            stats.extend(x_stats)
            stats.extend(x5_stats)
        elif  option == "-C" or option == "--cpu-stats":
            stats.extend(x6_stats)
        elif option == "-c" or option == "--cores":
            cores = int(optarg)
        else:
            sys.exit("Unknown option "+ option)

    try:
        os.makedirs(bsub_rundir)
        os.makedirs(old_job_dir)
    except OSError:
        pass

    job_db = pylib_db.key_db(db_file)
    results = pylib_eprot.estrat_set(testrun_dir)  
    parser = pylib_erun.e_res_parser(300, stats)

    jobcount, scheduled = find_batch_jobs(job_db)

    print "Currently running:", jobcount

    if force_scheduling:
        scheduled = set()
        
    process_complete_jobs(parser, results, job_db, bsub_rundir)
    results.sync()

    max_bsub_jobs = max_bsub_jobs - jobcount
    for name in args:
        newscheduled = process_strat(name, results, scheduled,
                                     max_bsub_jobs, job_db)
        max_bsub_jobs = max_bsub_jobs - newscheduled
        if max_bsub_jobs <= 0:
            break
