#!/usr/bin/env python2.7

import sys
import os
import re
import string
from e_option_parse import *

pick_by_global_performance = True
optimize_large_class_limit = 200


class StratPerf(object):
    """
    A class holding the performance of a strategy on a set of
    problems.
    """
    def __init__(self, strat_name):
        self.strat_name = strat_name
        self.problems   = {}
        self.solutions  = 0
        self.soln_time  = 0.0

    def addProblem(self, prob, status, time):
        self.problems[prob] = status, time

    def delProblem(self, prob):
        try:
            del self.problems[prob]
        except KeyError:
            pass
            #print "Warning: Tried to remove unknown problem "+prob+" from "+self.strat_name

    def getName(self):
        return self.strat_name

    def isSuccess(self, prob, time_limit, succ_status):
        status, time = self.problems[prob]
        return (status in succ_status and time<=time_limit)

    def getCardinality(self):
        """
        Return number of problems in the object.
        """
        return len(self.problems)

    def getSuccesses(self, time_limit, succ_status):
        return [(i, self.problems[i]) for i in self.problems if self.isSuccess(i,time_limit, succ_status)]

    def getPerf(self):
        """
        Return currently stored performance.
        """
        return (self.solutions, self.soln_time)

    def computePerf(self, time_limit, succ_status):
        """
        Update and return performance data.
        """
        self.solutions  = 0
        self.soln_time  = 0.0
        for prob in self.problems:
            status, time = self.problems[prob]
            if (status in succ_status) and (time<=time_limit):
                self.solutions += 1
                self.soln_time += time
        return self.getPerf()

    def removeOtherSuccesses(self, other, time_limit, succ_status):
        """
        Remove all problems that are solved in other (given the time
        limit) from self.
        """
        for prob in other.problems:
            if other.isSuccess(prob):
                self.delProblem(prob)



class ClassPerf(object):
    """
    A class associating with the name of a class the data of several
    strategies on this class.
    """
    def __init__(self, classname):
        self.name   = classname
        self.strats = {}

    def addProblem(self, strat_name, prob, status, time):
        """
        Add data for one problem from one strategy to the performance
        object.
        """
        if not strat_name in self.strats:
            set = StratPerf(strat_name)
            self.strats[strat_name] = set
        else:
            set = self.strats[strat_name]

        set.addProblem(prob, status, time)

    def delProblem(self, prob):
        """
        Remove data for a problem from all class performance objects.
        """
        for k in self.strats.values():
            k.delProblem(prob)

    def getCardinality(self):
        """
        Return number of strategies in the object.
        """
        return len(self.strats)

    def computePerf(self, time_limit, succ_status):
        for i in self.strats.values():
            i.computePerf(time_limit, succ_status)

    def delStrat(self, strat_name):
        try:
            del self.strats[strat_name]
            return True
        except KeyError:
            return False

    def getStrat(self, strat_name):
        return self.strats[strat_name]

    def getBestStrat(self):
        solutions = 0
        soln_time = 1
        res = None
        for i in self.strats.values():
            newsol, newtime = i.getPerf()
            if (newsol > solutions) or \
               ((newsol == solutions) and (newtime < soln_time)):
                solutions = newsol
                soln_time = newtime
                res = i.getName()
        return res, solutions

    def getSortedStrats(self):
        l = []
        for x in self.strats.values():
            l.append(x)
        return sorted(l, key=lambda x: x.getPerf())


def print_dict_lines(dict):
    for i in dict.keys():
        print repr(i)+" : "+repr(dict[i])

def print_result(fp, result, opt_res):
    for i in result.keys():
        cl  = result[i]
        res = opt_res[i]
        fp.write("/* %(i)-17s : %(cl)-30s %(res)-4d */\n" % vars())


def print_list_lines(l):
    for i in xrange(0, len(l)):
        print l[i]

def compute_problem_stem(key):
    mr = problem_ext.search(key)
    if not mr:
        raise RuntimeError, "Problem name has no extension (?!?) <" + key+">"
    res = key[0:mr.start()]
    return res;

# Parse a class file into the problems dictionary, return class size
#
# Arguments: File name
#            Internal name of the class
#            Dictionary where the problems are stored

def parse_class(c, key, probs):
    sys.stderr.write("Parsing " + c + " as " + key + "\n")
    i=0
    p=open(c,'r')
    for l in p:
        l=l[:-1]
        probs[compute_problem_stem(l)] = key
        i=i+1
    p.close
    return i

def tuple_add2(t1,t2):
    return (t1[0]+t2[0],t1[1]+t2[1])

def parse_prot(filename, stratname, matrix, succ_cases, time_limit):
    """
    Parse the given file with the given strategy name.
    """
    sys.stderr.write("Parsing " + stratname + "\n")
    p=open(filename,'r')
    l=p.readline()[:-1]
    desc = "";
    while l:
        res = full_comment.match(l)
        if(res):
            desc = desc+l
        else:
            clean = re.sub(trail_space,'',l)
            tuple=re.split(white_space,clean)
            prob = compute_problem_stem(tuple[0]);
            if (problems.has_key(prob)):
                time=float(tuple[2])
                if (tuple[1] in succ_cases) and time<=time_limit:
                    cl=problems[prob]
                    old = matrix[cl][stratname]
                    matrix[cl][stratname]=tuple_add2(old,(1,time))
                    old = stratperf[stratname];
                    stratperf[stratname] = tuple_add2(old,(1,time))
        l=p.readline()
    p.close()
    return desc

def parse_prot_new(filename, stratname, prob_assoc, global_class, classdata):
    """
    Parse the given file with the given strategy name.
    """
    sys.stderr.write("NewParsing " + stratname + "\n")
    p=open(filename,'r')
    desc = ""
    for l in p:
        res = full_comment.match(l)
        if(res):
            desc = desc+l
        else:
            clean  = re.sub(trail_space,'',l)
            tuple  = re.split(white_space,clean)
            prob   = compute_problem_stem(tuple[0]);
            try:
                time   = float(tuple[2])
            except ValueError:
                time = 10000
            status = tuple[1]
            try:
                classdata[prob_assoc[prob]].addProblem(stratname, prob, status, time)
                global_class.addProblem(stratname, prob, status, time)
            except KeyError:
                # print "Unclassified problem ", prob
                pass
    p.close()
    return desc




def eval_heuristic(classes,heuristic):
    res=(0.0,0.0)
    for i in classes:
        res=tuple_add2(res,matrix[i][heuristic])
    return res;

def tuple_is_better2(t1,t2):
    tmpres = t1[0]>t2[0] or (t1[0]==t2[0] and t1[1]<t2[1])
    return tmpres

def find_optimal_heuristic(classes, exclude, default=None):
    if default:
        res = default
        eval = eval_heuristic(classes, res)
    else:
        res  = ""
        eval = (-1.0,0.0)
    for i in stratset.keys():
        if not i in exclude:
            tmp = eval_heuristic(classes, i)
            if tuple_is_better2(tmp, eval):
                eval = tmp;
                res  = i
    return res

def find_covered(heuristic,classes):
    res = []
    for i in classes:
        if matrix[i][heuristic][0]==opt_res[i]:
            res.append(i)
    return res

def compare_strat_global(strat1, strat2):
    eval1 = stratperf[strat1]
    eval2 = stratperf[strat2]
    if eval1[0] > eval2[0]:
        return -1
    if eval1[0] < eval2[0]:
        return 1
    if eval1[1] > eval2[1]:
        return 1
    if eval1[1] < eval2[1]:
        return -1
    return 0


def translate_class(cl, opt_res):
    solved = repr(opt_res[cl])
    res = "      ( /* "+cl+" Solved: "+solved+ " of " +repr(classsize[cl]) + " */\n"
    pref = "       ";
    for i in xrange(6,len(cl)):
        if cl[i]!="-":
            res = res+pref+"Spec"+class_int[i][cl[i]]+"(spec)"
            pref = "&&\n       "
    res = res+")"
    return res;

def translate_class_list(cl, opt_res):
    res = "";
    pref = "";
    for i in cl:
        res = res+pref+translate_class(i, opt_res)+"\n"
        pref = "       ||\n"
    return res[0:-1];


def print_raw(fp, result, opt_res):
    fp.write("/* Raw association */\n")

    sum = 0;
    fp.write("char* raw_class[] = \n{\n")
    for i in result.keys():
        res = opt_res[i]
        cl  = result[i]
        sum += res
        fp.write("   \""+i[6:]+"\",  /* %6d %20s */\n"%(res,cl))
    fp.write("   NULL\n};")
    fp.write("/* Predicted solutions: %d */\n"%(sum,))


def print_strat_once(fp, strat, defined_strats):
    """
    If strat is not in defined_strats, print it and add it to
    defined_strats.
    """
    if not strat in defined_strats:
        fp.write(heuristic_define(strat, stratdesc)+"\n")
        defined_strats.add(strat)

def generate_output(fp, result, stratdesc, class_dir, raw_class, opt_res,
                    used, defined_strats):
    """
    Generate the actual C code. Parameters:
    result associates each class name with a strategy name.
    fp is the filepointer to print to
    stratdesc associates each strategy name with its parameters (in
                    the form a string of command line options).
    class_dir is the name of the directory of classes (only for
                    generating the useful comment)
    raw_class is a boolean value switching to a very reduced output
                    not usable to control E
    opt_res associates each class name with the expected performance
                    of it.
    used is the set of used strategies.
    """
    by_heuristic={}

    for i in result.keys():
        by_heuristic[result[i]]=[]

    for i in result.keys():
        by_heuristic[result[i]].append(i)

    fp.write( """
/* -------------------------------------------------------*/
/* The following code is generated automagically with     */
/* generate_auto.py. Yes, it is fairly ugly ;-)           */
/* -------------------------------------------------------*/

""")
    fp.write( "/* Class dir used: "+class_dir+" */\n\n")
    print_result(fp, result, opt_res)

    if raw_class:
        print_raw(fp, result, opt_res)
    else:
        fp.write( """
#ifdef CHE_PROOFCONTROL_INTERNAL

/* Strategies used:                                       */

""")
        for i in used:
            print_strat_once(fp, i, defined_strats)

        if global_best in used:
            fp.write("/* Global best, "+global_best+", already defined */\n")
        else:
            fp.write( "/* Global best (used as a default): */\n")
            print_strat_once(fp, i, defined_strats)

        fp.write( """#endif

#if defined(CHE_HEURISTICS_INTERNAL) || defined(TO_ORDERING_INTERNAL)

""")

        for i in by_heuristic.keys():
            fp.write( "   else if(\n")
            fp.write( translate_class_list(by_heuristic[i], opt_res)+")\n")
            fp.write( '''   {
#ifdef CHE_HEURISTICS_INTERNAL
            res = "''' + trans_heuristic_name(i) +'";\n')

            fp.write( parse_control_info(stratdesc[i])+"\n")

            fp.write( '''#endif
#ifdef TO_ORDERING_INTERNAL
''')

            fp.write( parse_ordering_info(stratdesc[i])+"\n")
            fp.write( "#endif\n   }\n")



        fp.write( "   else /* Default */\n")
        fp.write( '''   {
#ifdef CHE_HEURISTICS_INTERNAL
  res = "''' + trans_heuristic_name(global_best) +'";\n')

        fp.write( parse_control_info(stratdesc[global_best])+"\n")

        fp.write( '''#endif
#ifdef TO_ORDERING_INTERNAL
''')
        fp.write( parse_ordering_info(stratdesc[global_best])+"\n")
        fp.write( "#endif\n   }\n")

        fp.write( "#endif\n")

        fp.write( "\n/* Total solutions on test set: %d */\n"%(sum(opt_res.values()),))
        fp.write("""/* -------------------------------------------------------*/
/*     End of automatically generated code.               */
/* -------------------------------------------------------*/

""")



# These are from CASC-J10 tuning - 11 strategies that complement each
# other

backup_strats = [
    "protocol_G-E--_208_C18_F1_SE_CS_SP_PS_S5PRR_RG_S04BN.csv",
    "protocol_G-E--_208_C18_F1_SE_CS_SP_PS_S08CI.csv",
    "protocol_G-E--_302_C18_F1_URBAN_S5PRR_RG_S0Y.csv",
    "protocol_H----_047_B31_F1_PI_AE_R4_CS_SP_S2S.csv",
    "protocol_G-E--_300_C18_F1_SE_CS_SP_PS_S0Y.csv",
    "protocol_G-N--_023_B07_F1_SP_PI_Q7_CS_SP_CO_S5PRR_S0Y.csv",
    "protocol_G-E--_208_B07----S_F1_SE_CS_SP_PS_S5PRR_RG_S04AN.csv",
    "protocol_G-E--_107_C41_F1_PI_AE_Q4_CS_SP_PS_S4S.csv",
    "protocol_G-E--_300_C01_S00.csv",
    "protocol_G-E--_107_B42_F1_PI_SE_Q4_CS_SP_PS_S0YI.csv",
    "protocol_G-E--_208_B00_00_F1_SE_CS_SP_PS_S064A.csv",
    "protocol_H----_011_C07_F1_PI_SE_SP_S0V.csv"
]


def findBestBackupStrat(stratset, used_names, global_best):
    #print used_names, global_best
    cand = global_best

    for c in backup_strats:
        if not c in used_names and c in stratset.keys():
            cand = c
            break
    return cand




#------------------------------------------------------------------
# Begin main
#------------------------------------------------------------------

argc = len(sys.argv)

if argc <= 1:
    raise RuntimeError, "Usage: generate_auto.py <classes> <protocols>"

i=1

problems   = {} # Keys are problems, values are problem classes
classlist  = [] # List of all class names
classsize  = {} # Class name with associated size
stratset   = {} # Names of strategies with associated file names
stratdesc  = {} # Names of strategies with associated command line
stratperf  = {} # Global performance of strategy (strategy -> sol, time)
matrix     = {} # Keys: Class names. Objects: Dictionaries associating
                 # strategy with performance in this class

classdata  = {} # Associates class name with ClassPerf instance
                # describing the performance of (all) strategies on
                # this class.

class_dir  = ""

succ_cases = ["T", "N"]
add_local_prots = False
raw_class  = False
time_limit = 864000.0


for i in sys.argv[1:]:
    if i=="--proofs":
        succ_cases = ["T"]
    elif i=="--models":
        succ_cases = ["N"]
    elif i=="--raw":
        raw_class = True
    elif i=="--local":
        pick_by_global_performace = False
    elif i=="--lprots":
        add_local_prots = True
    elif i.startswith("--time="):
        time_limit = float(i[7:])
    elif i[0:2] == "--":
        raise RuntimeError, "Unknown option (probably typo)"

sys.argv = filter(lambda x:x[0]!="-", sys.argv) # Take out options

for i in sys.argv[1:]:
    res = match_class.search(i)
    if(res):
        key = i[res.start():res.end()]
        class_dir = i[0:res.start()]
        classlist.append(key)
        classsize[key] = parse_class(i, key, problems)
        classdata[key] = ClassPerf(key)
    else:
        key = re.split(slash,i)[-1]
        stratset[key] = i

if add_local_prots:
    for i in os.listdir("."):
        res = match_prot.search(i)
        if res:
            key = i
            stratset[key] = i


global_class = ClassPerf("ALL")

# Now problems is a dictionary associating every class with the
# associated problems and stratset is a dictionary associating the
# "local" name of the protocol with the full (relative or absolute)
# file name.

for i in stratset.keys():
    stratdesc[i] = parse_prot_new(stratset[i], i, problems, global_class, classdata)


# Start determining the strategies

sys.stderr.write("Parsing done, running optimizer\n")

time_limits = [150, 69, 22, 18, 13, 9, 5, 5, 5, 4]

# Temporary version to get a ranking of the "best others"
# time_limits = [5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5]


itercount = 0;

defined_strats = set()

global_class.computePerf(time_limit, succ_cases)
best_strats = global_class.getSortedStrats()

# print "// best strats: ", best_strats[:5]
#for i in best_strats[:5]:
#    print i.getPerf()

used_strats = {}
for i in classlist:
    used_strats[i] = set()



for time_limit in time_limits:
    print "# Auto-Schedule iteration %d with time limit %d"\
          %(itercount, time_limit)

    result     = {}
    global_class.computePerf(time_limit, succ_cases)
    global_best, global_perf = global_class.getBestStrat()

    print "# Global best: %s (with %d solutions) out of %d strategies "%\
          (global_best, global_perf, global_class.getCardinality())

    # print global_class.strats[global_best].getSuccesses(time_limit, succ_cases)

    opt_res = {}
    used    = set()

    used.add(global_best)


    for i in classlist:
        perf = classdata[i]
        perf.computePerf(time_limit, succ_cases)
        best, solns = perf.getBestStrat()
        if solns == 0:
            best = findBestBackupStrat(stratset, used_strats[i], global_best)
        result[i] = best
        opt_res[i] = solns
        used.add(best)
        used_strats[i].add(best)

    # And now we print the results

    fp = open("che_X_auto_sched%d.c"%(itercount,), "w")
    generate_output(fp, result, stratdesc, class_dir, raw_class,
                    opt_res, used, defined_strats)
    fp.close()

    # Now for the cleanup: For each class, we remove the best
    # strategy, and all problems solved by it.

    for i in classlist:
        perf = classdata[i]
        best, solns = perf.getBestStrat()
        if solns == 0:
            best = global_best
        try:
            # print "Best: ", best
            beststrat = perf.getStrat(best)
            # print "found: ", best
            solved = beststrat.getSuccesses(time_limit, succ_cases)
            for prob in solved:
                probname = prob[0]
                perf.delProblem(probname)
                global_class.delProblem(probname)
            perf.delStrat(best)
        except KeyError:
            pass
    itercount += 1
