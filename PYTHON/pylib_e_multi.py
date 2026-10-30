#!/usr/bin/env python3

"""
pylib_e_multi.py

Usage: pylib_e_multi.py [-h] [<config_file>]

Run a server process accepting and processing jobs for E. This is a
fairly specific hack allowing us to run starexec scripts locally and
to process the result with the genprot.py script.

The optional config file is read once at startup. While running, the
server polls the directory ~/EPROVER/RUNDIR for two files, which are
processed and then moved to ~/EPROVER/RUNDIR/LOG:

new_config: Configuration lines, as in the config file:
  max_running: <n>  Maximal number of tasks running in parallel
                    (default 32)
  time_limit: <n>   CPU time limit (in seconds) for tasks started from
                    now on (default 5)
  tptp: <dir>       Root of the (standard) TPTP installation for tasks
                    started from now on. A problem XXXnnn.p is taken
                    from <dir>/Problems/XXX/XXXnnn.p, and TPTP is set
                    to <dir> for E to find included axioms (default
                    $TPTP, or /opt/tptp/current if that is not set)
  quit              Terminate the server (after the running tasks are
                    finished, queued tasks are dropped)
  # ...             Comment, printed

new_jobs: One job per line, "<problem_list> <script>". <problem_list>
  is a file in ~/EPROVER/RUNDIR with one problem name per line, and
  <script> a starexec_run_* script in ~/EPROVER/RUNDIR. Lines starting
  with # are printed.

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
import os
import os.path
import getopt
import subprocess
import shutil
import concurrent.futures
from time import sleep


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
eTPTP = os.environ.get("TPTP", "/opt/tptp/current")


def tptpProblemPath(tptp, problem):
    """
    Return the path of problem (e.g. GRP001-1.p) in the standard
    TPTP installation at tptp (i.e. tptp/Problems/GRP/GRP001-1.p).
    """
    return "%s/Problems/%s/%s"%(tptp, problem[0:3], problem)

class etask(object):
    ids = id("t")

    def __init__(self, script, problem):
        self.script = script
        self.problem = problem
        self.id = etask.ids.newId()
        self.cmd = None
        self.output = None

    def __str__(self):
        return "<%s, %s, %s>"%(self.id, self.script, self.problem)

    def getId(self):
        return self.id

    def execute(self, time = 5, tptp = eTPTP):
        """
        Run the task in eRunDir and wait for it to finish. Return (and
        store) its output (stdout and stderr together). This is
        called in a worker thread of the scheduler.
        """
        problem = tptpProblemPath(tptp, self.problem)
        # TPTP is needed by E to find included axiom files
        env = dict(os.environ, STAREXEC_CPU_LIMIT=str(time), TPTP=tptp)
        self.cmd = "STAREXEC_CPU_LIMIT=%d TPTP=%s ./%s %s"%(
            time, tptp, self.script, problem)
        try:
            res = subprocess.run([os.path.join(eRunDir, self.script), problem],
                                 cwd=eRunDir, env=env,
                                 stdin=subprocess.DEVNULL,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT)
            self.output = res.stdout.decode("ASCII", errors="replace")
        except Exception as err:
            # Recognized by genprot.py
            self.output = "exec failed: %s\n"%(err,)
        return self.output

    def getRes(self):
        """
        Return the output of the finished task (None before).
        """
        return self.output


class job(object):
    ids = id("j")

    def __init__(self, script, problist):
        # Ids restart with every server start, so skip ids whose
        # results (from earlier runs) still exist
        while True:
            self.id = job.ids.newId()
            self.containerdir = "Job"+self.getId()+"_output"
            containerpath = eResDir+"/"+self.containerdir
            if not (os.path.exists(containerpath) or
                    os.path.exists(containerpath+".zip")):
                break

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
            # Older run scripts print %% (or #) instead of %
            if re.match(r"(%%?|#) Problem", l):
                tmp = l.split(":")
                probname = tmp[1].strip()
                domname = probname[0:3]
        if probname == None:
            print("# Warning: No Problem line in task output, discarded:")
            for l in res[:5]:
                print("#   "+l)
        else:
            dirpath = eResDir+"/"+self.logdir+"/"+domname+"/"+self.stratname+"/"+probname
            filepath = dirpath+"/"+"%07d.txt"%(self.filecounter,)
            self.filecounter += 1
            os.makedirs(dirpath, exist_ok=True)
            fp = open(filepath, "w")
            fp.write(result)
            fp.close()


    def summaryLines(self, result):
        """
        Return the SZS status and total time lines of E's output. E
        uses % as the comment character (# for old versions or
        --unix-comments).
        """
        return [l for l in result.split("\n")
                if re.match(r"[#%] (SZS|Total time)", l)]

    def returnTask(self, task):
        print("# Task "+task.getId()+" complete")
        for l in self.summaryLines(task.getRes()):
            print(l)
        self.processResult(task.getRes())
        self.cardinality -= 1
        if self.cardinality == 0:
            print("# Job "+self.getId()+" "+self.script+" complete")
            # Results are in eResDir, independent of the working directory
            containerpath = eResDir+"/"+self.containerdir
            if os.path.isdir(containerpath):
                shutil.make_archive(containerpath,
                                    "zip",
                                    root_dir=eResDir,
                                    base_dir=self.containerdir)
                shutil.rmtree(containerpath)
                print("# Job "+self.getId()+" zipped")
            else:
                print("# Warning: Job "+self.getId()+
                      " produced no results (no % Problem lines)")



class scheduler(object):
    ids = id("s")
    # Upper bound for max_running. Worker threads are only created
    # when needed, so this costs nothing.
    max_workers = 1024

    def __init__(self, max_running=32, time_limit=5, tptp=eTPTP):
        self.id = scheduler.ids.newId()
        self.queue    = []
        self.max_running = max_running
        self.time_limit = time_limit
        self.tptp = tptp
        # Future of each running task -> (task, job it belongs to)
        self.running  = {}
        self.pool = concurrent.futures.ThreadPoolExecutor(
            max_workers=scheduler.max_workers)
        self.quit_requested = False
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

    def processConfig(self, conf):
        """
        Process configuration lines (from new_config or the config
        file given on the command line).
        """
        for l in conf:
            if l.startswith("#"):
                print(l)
            elif l == "":
                pass
            elif l.startswith("quit"):
                print("# Exit requested, terminating after running tasks")
                self.quit_requested = True
            else:
                # Split at the first colon only (paths may contain one)
                todo = [p.strip() for p in l.split(":", 1)]
                if len(todo):
                    if todo[0] == "max_running":
                        try:
                            print("# Processing: "+l)
                            self.max_running = int(todo[1])
                            if self.max_running > scheduler.max_workers:
                                print("# Warning: max_running limited to %d"%(
                                    scheduler.max_workers,))
                                self.max_running = scheduler.max_workers
                        except:
                            print("# Warning: Failed to process "+l)
                    elif todo[0] == "time_limit":
                        try:
                            print("# Processing: "+l)
                            self.time_limit = int(todo[1])
                        except:
                            print("# Warning: Failed to process "+l)
                    elif todo[0] == "tptp":
                        if len(todo) == 2 and todo[1] != "":
                            print("# Processing: "+l)
                            self.tptp = todo[1]
                        else:
                            print("# Warning: Failed to process "+l)
                    else:
                        print("# Warning: Don't know what to do with "+l)

    def updateConfig(self):
        self.processConfig(self.consumeFile("new_config"))

    def readConfigFile(self, name):
        """
        Read the config file name (which, unlike new_config, is left
        in place).
        """
        fp = open(name, "r")
        conf = [l.strip() for l in fp.readlines()]
        fp.close()
        self.processConfig(conf)

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
            if os.path.isfile(file):
                print("Stopped scheduling due to "+file)
                return True
        return False

    def startTasks(self):
        """
        Hand queued tasks to the thread pool until max_running tasks
        are running.
        """
        while self.queue and len(self.running) < self.max_running:
            task, source = self.queue.pop(0)
            future = self.pool.submit(task.execute,
                                      self.time_limit, self.tptp)
            self.running[future] = (task, source)

    def finishTask(self, future):
        """
        Process the result of a finished task (in the main thread, so
        that results are filed and zipped one at a time).
        """
        task, source = self.running.pop(future)
        if source:
            source.returnTask(task)
        else:
            print("# Orphan task "+task.getId()+" complete")

    def schedule(self):
        while True:
            self.updateConfig()
            if self.quit_requested:
                break
            self.updateJobs()
            if self.checkCPURequest():
                sleeptime = 10
            else:
                sleeptime = 2
                self.startTasks()

            print("# Jobs running: ", len(self.running))
            print("# Queue:        ", len(self.queue), flush=True)

            if not self.running:
                print("# ...ZZZ...")
                sleep(sleeptime)
            else:
                done, _ = concurrent.futures.wait(
                    self.running, timeout=2,
                    return_when=concurrent.futures.FIRST_COMPLETED)
                for future in done:
                    self.finishTask(future)

        # Quit: Drop the queue, but keep the results of running tasks
        self.queue = []
        if self.running:
            print("# Waiting for %d running tasks"%(len(self.running),),
                  flush=True)
            for future in concurrent.futures.as_completed(list(self.running)):
                self.finishTask(future)
        self.pool.shutdown()
        print("# Terminated")


if __name__ == '__main__':
    try:
        opts, args = getopt.gnu_getopt(sys.argv[1:], "h")
    except getopt.GetoptError as err:
        print(err)
        sys.exit(1)

    for option, optarg in opts:
        if option == "-h":
            print(__doc__)
            sys.exit()

    if len(args) > 1:
        print(__doc__)
        sys.exit(1)

    # consumeFile() moves processed files here
    os.makedirs(eRunDir+"/LOG", exist_ok=True)

    worker = scheduler()

    if args:
        try:
            worker.readConfigFile(args[0])
        except OSError as err:
            print("Cannot read config file: "+str(err))
            sys.exit(1)

    worker.schedule()
