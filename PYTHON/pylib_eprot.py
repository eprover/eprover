#!/usr/bin/env python2.4

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
import getopt
import os.path
import pylib_generic
import pylib_io

espec_name = "tptp_"
eprot_name = "protokoll_"


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
                fp = pylib_io.flexopen(value, "r")
                newstr = fp.read()
                pylib_io.flexclose(fp)
                resdict, joblist =\
                         parse_espec_string(newstr,\
                                            "Included file "+value,\
                                            resdict, joblist)
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
    if name.startswith(espec_name):
        return name[len(espec_name):]
    if name.startswith(eprot_name):
        return name[len(eprot_name):]
    return name
    

class eresult(object):
    """
    Class representing a single result (i.e. line in an E protocol).
    """
    def __init__(self, line):
        assert line[0]!="#"
        self.values = line.split()
        assert len(self.values) >= 4

    def name(self):
        return self.values[0]

    def status(self):
        return self.values[1]

    def cputime(self):
        return float(self.values[2])

    def reason(self):
        return self.values[3]

    def __str__(self):
        fixed = "%-29s %s %8s %-10s"%(self.values[0],self.values[1],\
                                      self.values[2],self.values[3])
        reslist = [fixed]
        for i in self.values[4:]:
            reslist.append("%10s"%(i,))
        return " ".join(reslist)

class eprot(object):
    """
    Class representing an E protocol.
    """
    def __init__(self, name):
        self.name     = e_strip_name(name)
        self.comments = []
        self.results  = {}
        self.filename = None

    def protname(self):
        return eprot_name+self.name

    def parse(self, directory=""):
        filename = os.path.join(directory, self.protname())
        fp = open(filename, "r")
        prot = fp.read().split("\n")
        for i in prot:
            if i:
                if i[0]=="#":
                    self.comments.append(i)
                else:
                    res = eresult(i)
                    self.results[res.name()]=res
        self.filename = filename

    def __str__(self):
        results = [i.__str__() for i in self.results.values()]
        results.sort()
        return "\n".join(self.comments+results)



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
        self.name         = name
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


class ejob(object):
    """
    """
    def __init__(self, name):
        self.name         = name
        self.prot         = eprot(name)
        self.job_complete = False
        
    def jobname(self):
        return "tptp_"+self.name

    def parse(self, directory=""):
        filename = os.path.join(directory, self.protname())


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
    testprt = eprot("X----_auto_300")
    testprt.parse("/Users/schulz/EPROVER/TESTRUNS_CASC")
    #print testprt
    #print parse_espec_file("~/EPROVER/TESTRUNS_CASC/tptp_U----_043_B07_F1_PI_AE_CS_SP_S0Y")
    testspec = espec("X----_auto_300")
    testspec.parse("/Users/schulz/EPROVER/TESTRUNS_CASC")
    print testspec
    
    
