#!/usr/bin/env python2.7

import sys
import os
import re
import string
from e_option_parse import *


pick_by_global_performance = True
optimize_large_class_limit = 200


def print_dict_lines(dict):
    for i in dict.keys():
        print repr(i)+" : "+repr(dict[i])

def print_result():
    for i in result.keys():
        cl  = result[i]
        res = opt_res[i]
        print "/* %(i)-19s : %(cl)-45s %(res)-4d */" % vars()


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
    l=p.readline()[:-1]
    while l:
        probs[compute_problem_stem(l)] = key
        l=p.readline()[:-1]
        i=i+1
    p.close
    return i

def tuple_add2(t1,t2):
    return (t1[0]+t2[0],t1[1]+t2[1])

def parse_prot(filename, stratname, matrix, succ_cases):
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
            if (problems.has_key(prob)) and (tuple[1] in succ_cases):
                cl=problems[prob]
                try:
                    time   = float(tuple[2])
                except ValueError:
                    time = 10000
                old = matrix[cl][stratname]
                matrix[cl][stratname]=tuple_add2(old,(1,time))
                old = stratperf[stratname];
                stratperf[stratname] = tuple_add2(old,(1,time))
        l=p.readline()
    p.close
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


def translate_class(cl):
    solved = repr(matrix[cl][result[cl]][0])
    res = "      ( /* "+cl+" Solved: "+solved+ " of " +repr(classsize[cl]) + " */\n"
    pref = "       ";
    for i in xrange(6,len(cl)):
        # print "# YYY", cl
        if cl[i]!="-":
            # print "# ZZZ", i, cl[i]
            res = res+pref+"Spec"+class_int[i][cl[i]]+"(spec)"
            pref = "&&\n       "
    res = res+")"
    return res;

def translate_class_list(cl):
    res = "";
    pref = "";
    for i in cl:
        res = res+pref+translate_class(i)+"\n"
        pref = "       ||\n"
    return res[0:-1];




def print_raw():
    print "/* Raw association */"

    sum = 0;
    print "char* raw_class[] = \n{"
    for i in result.keys():
        res = opt_res[i]
        cl  = result[i]
        sum += res
        print "   \""+i[6:]+"\",  /* %6d %20s */"%(res,cl)
    print "   NULL\n};"

    print "char* raw_sine[] = \n{"
    for i in result.keys():
        cl  = result[i]
        arg = parse_sine(stratdesc[cl])
        if arg:
            print "   \""+arg+"\","
        else:
            print "   NULL,"
    print "   NULL\n};"
    print "/* Predicted solutions: %d */"%(sum,)


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
class_dir  = "";

succ_cases = ["T", "N"]
raw_class  = False
add_local_prots = False


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
    else:
        key = re.split(slash,i)[-1]
        stratset[key] = i

if add_local_prots:
    for i in os.listdir("."):
        res = match_prot.search(i)
        if res:
            key = i
            stratset[key] = i

for i in classlist:
    matrix[i] = {}
    for j in stratset.keys():
        matrix[i][j] = (0,0.0)

for i in stratset.keys():
    stratperf[i]=(0,0)
    stratdesc[i]=parse_prot(stratset[i],i,matrix,succ_cases)


# Start determining the strategies

sys.stderr.write("Parsing done, running optimizer\n")

sum=0
opt_res = {};
for i in classlist:
    solved = 0
    for j in matrix[i].keys():
        solved = max(solved, matrix[i][j][0])
    opt_res[i] = solved
    sum = sum+solved

class_list_iter = list(classlist)
result = {}
used   = []
myused = {}

if pick_by_global_performance:
    ordered_strats = stratperf.keys()
    ordered_strats.sort(compare_strat_global)
    # print_dict_lines(stratperf)
    # print_list_lines(ordered_strats)

while class_list_iter:
    if pick_by_global_performance:
        h = ordered_strats.pop(0)
    else:
        h = find_optimal_heuristic(class_list_iter, used, None)

    covered =  find_covered(h, class_list_iter)
    for i in covered:
        result[i] = h
        class_list_iter.remove(i)
    used.append(h)
    myused[h] = 1


# Now we might want to do some post-optimization....or not!

if optimize_large_class_limit > 0:
    class_list_iter = list(classlist)

    while class_list_iter:
        cand = class_list_iter.pop()
        if classsize[cand] > optimize_large_class_limit:
            tmp = result[cand]
            h = find_optimal_heuristic([cand], [], tmp)
            result[cand] = h
            # print "Heuristic for "+cand+" changed from "+tmp+" to "+h
            if not h in myused:
                used.append(h)
                myused[h] = 1

# And now we print the results

by_heuristic={}

for i in result.keys():
    by_heuristic[result[i]]=[]

for i in result.keys():
    by_heuristic[result[i]].append(i)

print """
/* -------------------------------------------------------*/
/* The following code is generated automagically with     */
/* generate_auto.py. Yes, it is fairly ugly ;-)           */
/* -------------------------------------------------------*/
"""
print "/* Class dir used: "+class_dir+" */\n\n"
print_result()

if raw_class:
    print_raw()
else:

    print """
#ifdef CHE_PROOFCONTROL_INTERNAL

/* Strategies used:                                       */

"""

    for i in by_heuristic.keys():
        print heuristic_define(i, stratdesc)

    if used[0] in by_heuristic.keys():
        print "/* Global best, "+used[0]+", already defined */"
    else:
        print "/* Global best (used as a default): */"
        print heuristic_define(used[0],stratdesc)

    print """#endif

#if defined(CHE_HEURISTICS_INTERNAL) || defined(TO_ORDERING_INTERNAL)
"""

    for i in by_heuristic.keys():
        print "   else if("
        print translate_class_list(by_heuristic[i])+")"
        print '''   {
#ifdef CHE_HEURISTICS_INTERNAL
      res = "''' + trans_heuristic_name(i) +'";'

        print parse_control_info(stratdesc[i])

        print '''#endif
#ifdef TO_ORDERING_INTERNAL'''

        print parse_ordering_info(stratdesc[i])
        print "#endif\n   }"



    print "   else /* Default */"
    print '''   {
#ifdef CHE_HEURISTICS_INTERNAL
  res = "''' + trans_heuristic_name(used[0]) +'";'

    print parse_control_info(stratdesc[used[0]])

    print '''#endif
#ifdef TO_ORDERING_INTERNAL'''
    print parse_ordering_info(stratdesc[used[0]])
    print "#endif\n   }"


    print "#endif"

    print "\n/* Total solutions on test set:", sum, "*/"
    print """/* -------------------------------------------------------*/
/*     End of automatically generated code.               */
/* -------------------------------------------------------*/
"""
