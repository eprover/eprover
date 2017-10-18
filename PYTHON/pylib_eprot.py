#!/usr/bin/env python

"""
pylib_eprot 0.1

Library supporting E test runs and protocol files.

Copyright 2008 Stephan Schulz, schulz@eprover.org

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

import sys
import string
import time
import getopt
import os.path
import pylib_generic
import pylib_io

espec_name  = "tptp_"
eprot_name  = "protokoll_"
eprot_name  = "protokoll_"
eprot_name2 = "protocol_"


def parse_espec_string(specstr, sourcename=None, resdict = None, joblist = None):
    """
    Parse a E spec file. 
    \param specstr        is the actual configuration text,
                          interpreted as a sequence of newline-
                          separated lines.
    \param sourcename     is a descriptive name of the source, used
                          for error messages.
    \return               Dictionary of key/value pairs and list of
                          problems. 
    """
    if not sourcename:
        sourcename = "E specification string starting with "+inpstr[:20]
    if resdict == None:
        resdict = {}
    if joblist == None:
        joblist = list([])

    spec_list = specstr.split("\n")
    lineno = 0
    for line in spec_list:
        lineno = lineno+1
        comment = line.split("#")
        line = comment[0]
        line = line.strip()
        if not line:
            continue

        try:
            (key, value) = line.split(":", 1)
            
            key = key.strip()
            value = value.strip()
            if key == "Include":
                try:
                    fp = pylib_io.flexopen(value, "r")
                    newstr = fp.read()
                    pylib_io.flexclose(fp)
                    resdict, joblist =\
                             parse_espec_string(newstr,\
                                                "Included file "+value,\
                                                resdict, joblist)
                except IOError:
                    pass # have to put logging in
            else:
                resdict[key] = value
        except ValueError:
            joblist.append(line)
        
    return (resdict, joblist)


def parse_espec_file(source):
    """
    Parse a E test specifcation file.

    \param source         is either a string (interpreted as a file
                          name) or a file pointer.
    \return               tuple of key/value associations and list of
                          problems.
    """

    if type(source) == type("str"):
        fp = pylib_io.flexopen(source, "r")
        inpstr = fp.read()
        pylib_io.flexclose(fp)
        sourcename = source            
    else:
        assert(type(source)==type(sys.stdin))
        inpstr = fp.read()
        sourcename = source.name

    return parse_espec_string(inpstr, sourcename)




def e_strip_name(name):
    """
    Given an E job name, either in raw, in spec or in prot form,
    return the raw name.
    """
    name = os.path.basename(name)
    if name.startswith(espec_name):
        return name[len(espec_name):]
    if name.startswith(eprot_name):
        return name[len(eprot_name):]
    if name.startswith(eprot_name2):
        return name[len(eprot_name2):]
    return name


class ejob(object):
    """
    Class representing a single job to run.    
    """
    def __init__(self, strat_key, executable, arguments, problem, time_limit):
        self.strat_key  = strat_key
        self.executable = executable
        self.arguments  = arguments
        self.problem    = problem
        self.time_limit = time_limit

    def __repr__(self):
        return ("<%s:%s:%s:%s:%f>" %
                (self.strat_key,
                 self.executable,
                 self.arguments,
                 self.problem,
                 self.time_limit))
        
    def __str__(self):
        return ("run\n%s\n%s\n%s\n%s\n%f\n.\n" %
                (self.strat_key,
                 self.executable,
                 self.arguments,
                 self.problem,
                 self.time_limit))

    def get_args(self):
        """
        Return a string representing the (dynamic) options.
        """
        return "--cpu-limit=%d %s"%(self.time_limit, self.arguments)

    def strat(self):
        return self.strat_key

    def prob(self):
        return self.problem

    def key(self):        
        return self.strat_key+self.problem


class eresult(object):
    """
    Class representing a single result (i.e. line in an E protocol).
    """
    def __init__(self, line):
        assert line[0]!="#"
        self.values = line.split()
        assert len(self.values) >= 4

    def success(self):
        return self.status() in ['T', 'N']

    def name(self):
        return self.values[0]

    def status(self):
        return self.values[1]

    def cputime(self):
        try:
            return float(self.values[2])
        except ValueError:
            return 1000000.0

    def reason(self):
        return self.values[3]

    def __str__(self):
        fixed = "%-29s %s %8s %-10s"%(self.values[0],self.values[1],\
                                      self.values[2],self.values[3])
        reslist = [fixed]
        for i in self.values[4:]:
            reslist.append("%10s"%(i,))
        return " ".join(reslist)

    def __repr__(self):
        return self.__str__()


class eprot(object):
    """
    Class representing an E protocol.
    """
    def __init__(self, name):
        self.name     = e_strip_name(name)
        self.comments = []
        self.results  = {}
        self.filename = None
        self.synced   = True
        self.params   = ""

    def result(self, problem):
        try:
            return self.results[problem]
        except KeyError:
            return None

    def results_no(self):
        return len(self.results)

    def set_params(self, params):
        self.params = params

    def protname(self):
        return eprot_name2+self.name

    def add_result(self, result):
        self.results[result.name()]=result
        self.synced = False

    def del_result(self, res):
        """
        Remove a result, which may be a full result or just a problem
        name (i.e. a string).
        """
        try:
            del(self.results[res.name()])
        except AttributeError:
            del(self.results[res])
        self.synced = False

    def get_result(self, res):
        """
        Return the result for problem res (or None).
        """
        try:
            return self.results[res]
        except KeyError:
            return None

    def get_successes(self):
        """
        Return set of problems that have been solved in this protocol.
        """        
        return set([i.name() for i in self.results.values() if i.success()])

    def get_by_limit(self, selector, limit):
        """
        Return a set of problems p that have a value of at least limit
        in p.values[selector].
        """
        return set([i.name()\
                    for i in self.results.values()\
                    if (float(i.values[selector])>=limit)])
        
    def get_results(self):
        return list(self.results.values())

    def get_timesorted_results(self):
        p = self.get_results()
        p.sort(key=eresult.cputime)
        return p

    def get_performance_plot(self, step=1.0, limit=300):
        """
        Return a list of solutions found for the time limits 0, step,
        2*step, ..., 300.
        """
        tlim = 0.0
        succ = 0
        count = 0
        res = []
        soln = self.get_timesorted_results()
        soln = [r for r in soln if r.success()]

        while tlim <=limit:
            while soln and soln[0].cputime() <= tlim:
                soln.pop(0)
                succ = succ+1
            res.append((tlim, succ))
            count = count+1
            tlim = count*step
        return res

            
    def filter(self, problemlist):
        """
        Filter result list against a problemlist.
        """
        count = 0
        tmp = self.results
        self.results = {}
        for i in problemlist:
            if i in tmp:
                self.results[i] = tmp[i]
                count = count+1
        return count                

    def find_missing(self, problemlist):
        """
        Return the list of problems in problemlist, but without a
        result in self.
        """
        res = []
        for i in problemlist:
            if not i in self.results:
                res.append(i)
        return res

    def parse(self, directory=""):
        filename = os.path.join(directory, self.protname())
        self.filename = filename
        try:
            fp = pylib_io.flexopen(filename, "r")
        except IOError,err:
            return False
        prot = fp.read().split("\n")
        for i in prot:            
            if i:
                if i[0]=="#":
                    pass
                else:
                    res = eresult(i)
                    self.add_result(res)
        self.synced   = True
        return True

    def evaluate(self):
        count      = 0
        success    = 0
        failure    = 0
        incomplete = 0
        nomem      = 0
        unknown    = 0
        time       = 0.0        
        for i in self.results.values():
            count = count+1
            if i.success():
                success = success+1
                time    = time + i.cputime()
            else:
                failure = failure+1
            if i.reason() == "maxmem":
                nomem = nomem+1
            elif i.reason() == "incomplete":
                incomplete = incomplete+1
            elif i.reason() == "unknown":
                unknown = unknown+1
        return (count, success, failure, incomplete, nomem, unknown, time)

    def __str__(self):
        results = [i.__str__() for i in self.results.values()]
        results.sort()
        return "# %s\n"%(self.params,)+"\n".join(self.comments+results)+"\n"

    def sync(self):
        """
        Safe the protocol to the associated disk file (if any and if
        necessary).
        """
        if self.filename and not self.synced:
            pylib_io.verbout("Syncing "+self.filename)
            fp = pylib_io.flexopen(self.filename, "w")
            fp.write(self.__str__())
            pylib_io.flexclose(fp)
            self.synced = True


class espec(object):
    """
    Class representing a specification of a test run (reusing the old
    tptp_ syntax from the original AWK test tools.
    """

    template = """
# Test spec: %s
Executable:  %s
Time limit:  %f
Arguments:   %s
# Test problems:
"""
             
    def __init__(self, name):
        self.name         = e_strip_name(name)
        self.filename     = None
        self.problems     = []
        self.executable   = "eprover"
        self.time_limit   = 300.0
        self.arguments    = ""
        
    def specname(self):
        return "tptp_"+self.name

    def parse(self, directory=""):
        filename = os.path.join(directory, self.specname())
        params, probs = parse_espec_file(filename)
        self.problems = probs
        for key in params:
            value = params[key]
            if key == "Problemdir":
                print "Warning: Problemdir ignored (now E server config item)"
            elif key == "Logfile":
                print "Warning: Logfile ignored (now implied by spec name)"
            elif key == "Time limit":
                self.time_limit = float(value)
            elif key == "Arguments":
                self.arguments = value
            elif key == "Executable":
                execname = os.path.expanduser(value)
                if os.path.isabs(execname):
                    print "Warning: Absolute filenames not supported "+\
                          "anymore, using plain file name"
                    execname = os.path.basename(execname)
                self.executable = execname
            else:
                print "Warning, unknown key "+key+" found"

    def __str__(self):
        params = espec.template % (self.name, self.executable,\
                                   self.time_limit, self.arguments)
        jobs   = "\n".join(self.problems)
        return params+jobs

    def create_job(self, problem):
        return ejob(self.name, self.executable,
                    self.arguments, problem,
                    self.time_limit)
        

class estrat_task(object):
    """
    Class representing a single strategy to test, including
    specification and results (so far).
    """
    def __init__(self, name, auto_sync = None):
        self.name          = e_strip_name(name)
        self.prot          = eprot(name)
        self.spec          = espec(name)
        self.job_complete  = False
        self.auto_sync     = auto_sync
        self.specdir       = None
        self.protdir       = None
        self.retired       = pylib_generic.timer(0)
        self.refresh()


    def __str__(self):
        return "<estrat %15s: %5d of %5d : Stale in: %f>" % (self.name,
                                                             self.results_no(),
                                                             self.total_no(),
                                                             self.retired.remaining())
        
    def parse(self, specdir, protdir):
        """
        Parse spec and protocol.
        """
        self.spec.parse(specdir)
        self.prot.parse(protdir)
        self.prot.set_params(self.spec.arguments)
        self.specdir = specdir
        self.protdir = protdir
        self.prot.filter(self.spec.problems)
        missing = self.prot.find_missing(self.spec.problems)
        self.job_complete = len(missing)==0

    def results_no(self):
        return self.prot.results_no()

    def total_no(self):
        return len(self.spec.problems)

    def find_missing(self):
        return self.prot.find_missing(self.spec.problems)

    def refresh(self):
        """
        Wait for half an hour before considering this protocol stale.
        """
        self.retired.set(1800)

    def stale(self):
        return self.retired.expired()

    def add_result(self, res):
        self.refresh()
        self.prot.add_result(res)
        if self.auto_sync:
            if self.results_no() % self.auto_sync == 0:
                self.sync()
                return True
        return False

    def set_synced(self, synced = True):
        self.prot.synced = synced

    def sync(self):
        self.prot.sync()

    def generate_jobs(self):
        return [self.spec.create_job(i) for i in self.find_missing()]

    def has_job(self, job):
        return job.strat() == self.name

    def complete(self):
        if len(self.prot.results) >= len(self.spec.problems):
            return len(self.find_missing())==0
        else:
            return False


class estrat_set(object):
    """
    A simple set of E strategies indexed by name.
    """
    
    def __init__(self, specdir, protdir=None):
        """
        Constructor. The *dirs are the paths to the directories in
        which to find specs and results, respectively.
        """
        self.strats  = {}
        self.specdir = specdir
        if protdir:
            self.protdir = protdir
        else:
            self.protdir = specdir
            

    def find_strat(self, strat_name):
        """
        Given a name, find and return the corresponding strategy
        object. Create it if necessary.
        """
        name = e_strip_name(strat_name)
        try:
            res = self.strats[name]
        except KeyError:
            res = estrat_task(name)
            res.parse(self.specdir, self.protdir)
            self.strats[name] = res
        return res

    def insert_res(self, strat_name, res):
        """
        Insert res into strat_name.
        """
        strat = self.find_strat(strat_name)
        strat.add_result(res)

    def sync(self):
        """
        Sync all results.
        """
        for i in self.strats.values():
            i.sync()

    def __str__(self):
        return str(self.strats)
        

if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    if len(args) > 2:
        print __doc__
        sys.exit()

    # Minimal unit test
    testprt = eprot("X----_autodev_300")
    testprt.parse("/Users/schulz/EPROVER/TESTRUNS_PEGASUS")
    #print testprt
    print testprt.evaluate()
    #print parse_espec_file("~/EPROVER/TESTRUNS_CASC/tptp_U----_043_B07_F1_PI_AE_CS_SP_S0Y")
    
    print testprt.get_successes()
    testspec = espec("X----_autodev_300")
    testspec.parse("/Users/schulz/EPROVER/TESTRUNS_PEGASUS")
    #print testspec
    
    job = estrat_task("X----_autodev_300")
    job.parse("~/EPROVER/TESTRUNS_PEGASUS/", "~/EPROVER/TESTRUNS_PEGASUS/")
    #print job.prot
    print job.job_complete
    jobs = [job.spec.create_job(i) for i in job.find_missing()]
    print jobs
    
