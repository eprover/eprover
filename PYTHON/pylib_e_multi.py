#!/usr/bin/env python3

"""
pylib_e_multi.py

Usage: pylib_e_multi.py <config_file>

Run a server process accepting and processing jobs for E. This is a
fairly specific hack allowing us to run starexec scripts locally and
to process the result with the genprot.py script.

Options:

-h Print this help.

Copyright 2019-2024 Stephan Schulz, schulz@eprover.org

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

Stephan Schulz
DHBW Stuttgart
Informatik
Jaegerstrasse 56
70174 Stuttgart
Germany

or (preferably) via email (address above).
"""

import sys
import re
import string
import os
import os.path
import getopt
import subprocess
import zipfile
import shutil
from time import sleep
import selectors


class id(object):
    def __init__(self, prefix):
        self.prefix = prefix
        self.count  = 0

    def newId(self):
        self.count = self.count+1
        return "%s%04d"%(self.prefix,self.count)

homeDir = os.path.expanduser("~")
eRunDir = homeDir+"/EPROVER/RUNDIR"
eResDir = eRunDir
eFlatTPTP = homeDir+"/EPROVER/TPTP_8.2.0_FLAT"

class etask(object):
    ids = id("t")

    def __init__(self, script, problem):
        self.script = script
        self.problem = problem
        self.id = etask.ids.newId()
        self.status = "new"
        self.output = b""
        self.proc = None

    def __str__(self):
        return "<%s, %s, %s, %s>"%(self.id, self.script,
        self.problem, self.status)

    def getId(self):
        return self.id

    def run(self, time = 5):
        if self.status !="new":
            return None

        cmd = "(export STAREXEC_CPU_LIMIT=%d; cd %s; ./%s %s/%s)"% (
            time, eRunDir, self.script, eFlatTPTP, self.problem)

        self.cmd = cmd
        self.proc = subprocess.Popen(cmd, stdin=None,
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT,
                                     shell=True,
                                     close_fds=True)
        self.status = "run"
        return self.fileno()

    def fileno(self):
        """
        Return the pipe's fileno to support select.
        """
        if self.status!="run":
            return None
        return self.proc.stdout.fileno()

    def getRes(self):
        if self.status=="run":
            tmp = self.proc.stdout.read()
            self.output = self.output+tmp
            if tmp != b"":
                return None
            self.status = "done"
        return self.output.decode("ASCII")


class job(object):
    ids = id("j")

    def __init__(self, script, problist):
        self.id = job.ids.newId()

        self.containerdir = "Job"+self.getId()+"_output"
        self.logdir = self.containerdir+"/Problems"
        self.script = script
        self.stratname = "EFake__E_"+script[len("starexec_run_"):]
        self.problems = []
        self.cardinality = 0
        problems = [p.strip() for p in problist.split("\n")]
        self.problems = [p for p in problems if p != "" and p[0]!="#"]
        self.cardinality = len(self.problems)
        self.filecounter = 0
        print("# Created job "+self.getId()+" with ",
              self.cardinality," problems")


    def getId(self):
        return self.id

    def submitTasks(self, scheduler):
        for p in self.problems:
            task = etask(self.script, p)
            scheduler.addTask(task, self)

    def processResult(self, result):
        #print(result)
        res = result.split("\n")
        probname = None
        for l in res:
            if l.startswith("% Problem"):
                tmp = l.split(":")
                probname = tmp[1].strip()
                domname = probname[0:3]
        if probname != None:
            dirpath = eResDir+"/"+self.logdir+"/"+domname+"/"+self.stratname+"/"+probname
            filepath = dirpath+"/"+"%07d.txt"%(self.filecounter,)
            self.filecounter += 1
            os.makedirs(dirpath, exist_ok=True)
            fp = open(filepath, "w")
            fp.write(result)
            fp.close()


    def returnTask(self, task):
        print("# Task "+task.getId()+" complete")
        res = task.getRes().split("\n")
        for l in res:
            if l.startswith("# SZS"):
                print(l)
            if l.startswith("# Total time"):
                print(l)
        self.processResult(task.getRes())
        self.cardinality -= 1
        if self.cardinality == 0:
            print("# Job "+self.getId()+" "+self.script+" complete")
            shutil.make_archive(self.containerdir,
                                "zip",
                                base_dir=self.containerdir)
            shutil.rmtree(self.containerdir)
            print("# Job "+self.getId()+" zipped")



class scheduler(object):
    ids = id("s")

    def __init__(self, max_running=32):
        self.id = scheduler.ids.newId()
        self.queue    = []
        self.max_running = max_running
        self.running  = 0
        self.tasks    = {}
        self.filecount = 0
        self.blockfiles = [
            os.path.expanduser("~sschulz")+"/CPU_REQUEST",
            os.path.expanduser("~schulz")+"/CPU_REQUEST"
            ]

    def addTask(self, job, source = None):
        self.queue.append((job, source))

    def getId(self):
        return self.id

    def consumeFile(self, name):
        fullname=eRunDir+"/"+name
        res = []
        if os.path.isfile(fullname):
            fp = open(fullname, "r")
            res = [l.strip() for l in fp.readlines()]
            fp.close()
            newname = eRunDir+"/LOG/%05d_"%(self.filecount,)+name+"_"+self.getId()
            self.filecount+=1
            os.rename(fullname, newname)
        return res

    def updateConfig(self):
        conf = self.consumeFile("new_config")
        for l in conf:
            if l.startswith("#"):
                print(l)
            elif l.startswith("quit"):
                print("Exit requested, terminating")
                sys.exit(1)
            else:
                todo = [p.strip() for p in l.split(":")]
                if len(todo):
                    if todo[0] == "max_running":
                        try:
                            print("# Processing: "+l)
                            self.max_running = int(todo[1])
                        except:
                            print("# Warning: Failed to process "+l)
                    else:
                        print("# Warning: Don't know what to do with "+l)

    def updateJobs(self):
        jobs = self.consumeFile("new_jobs")
        for l in jobs:
            if l.startswith("#"):
                print(l)
            elif l == "":
                pass
            else:
                todo = l.split()
                if len(todo)!=2:
                    print("# Warning: Cannot understand "+l)
                else:
                    probfile = eRunDir+"/"+todo[0]
                    script   = todo[1]
                    try:
                        fp = open(probfile, "r")
                        problist = fp.read()
                        fp.close()
                        newjob = job(script, problist)
                        newjob.submitTasks(self)
                    except:
                        print("# Warning: Something wrong with "+l)

    def checkCPURequest(self):
        for file in self.blockfiles:
            try:
                res = os.path.isfile(file)
            except:
                pass
            if res:
                print("Stopped scheduling due to "+file)
                return res
        return False

    def schedule(self):
        sel_tasks = selectors.DefaultSelector()
        sleeptime = 2

        while True:
            self.updateConfig()
            self.updateJobs()
            if self.checkCPURequest():
                sleeptime = 10
            else:
                sleeptime = 2
                while (self.running < self.max_running):
                    try:
                        job, source = self.queue.pop(0)
                        self.tasks[job] = source
                        job.run()
                        sel_tasks.register(job, selectors.EVENT_READ)
                        self.running += 1
                    except IndexError:
                        break

            print("# Jobs running: ", self.running)
            print("# Queue:        ", len(self.queue), flush=True)

            if self.running == 0:
                print("# ...ZZZ...")
                sleep(sleeptime)
            else:
                events = sel_tasks.select(timeout=2)
                for task, mask in events:
                    res = task.fileobj.getRes()
                    if res!=None:
                        sel_tasks.unregister(task.fileobj)
                        self.running -= 1
                        source = self.tasks[task.fileobj]
                        del(self.tasks[task.fileobj])
                        if source:
                            source.returnTask(task.fileobj)
                        else:
                            print("# Orphan job" +
                                  task.fileobj.getId()+
                                  " complete")




if __name__ == '__main__':
    #t1 = etask("starexec_run_E---2.0_G-E--_208_C41_F1_AE_CS_SP_PS_S00", "BOO001-1.p")
    #t2 = etask("starexec_run_E---2.0_G-E--_208_C41_F1_AE_CS_SP_PS_S00", "GRP382-1.p")

    #testjob = job(
    #    "starexec_run_E---2.0_G-E--_208_C41_F1_AE_CS_SP_PS_S00",
    #    "BOO001-1.p\nGRP382-1.p\nALG002-1.p\nALG004-1.p\n"+
    #    "ALG011-1.p\nALG024+1.p\nALG007-1.p\n")

    worker = scheduler()
    # testjob.submitTasks(worker)

    worker.schedule()
