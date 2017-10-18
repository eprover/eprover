#!/usr/bin/env python2.7

import sys
import re
import string

pick_by_global_performance = True
optimize_large_class_limit = 200

white_space     = re.compile('\s+')
trail_space     = re.compile('\s*$')
arg_term        = re.compile('\s|$')
full_comment    = re.compile('^#')
dash            = re.compile('-')
slash           = re.compile('/')
match_heuristic = re.compile("-H'?\([^#]*\)'?")
match_class     = re.compile('CLASS_[a-zA-Z-0-9]*$')
eval_f_sep      = re.compile('\),')
problem_ext     = re.compile('\.[a-z]*$')


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
                print "Unclassified problem ", prob
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

def find_optimal_heuristic(classes, exclude):
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

def trans_heuristic_name(name):
    tmp = re.sub('p[^_]*_', "", name);
    tmp = re.sub(dash,"_", tmp)
    tmp = re.sub('[.]csv',"", tmp)
    return tmp


def heuristic_define(name):
    mr = match_heuristic.search(stratdesc[name])
    if not mr:
        raise RuntimeError, "No heuristic defined in " + name;
    res= '"' + trans_heuristic_name(name) + ' = \\n"\n"'
    tmp = stratdesc[name][mr.start()+2:mr.end()]
    tmp = re.sub("'", "", tmp);
    res=res+ re.sub(eval_f_sep,'),"\n" ',tmp) +'\\n"'

    return res

def extract_arg(line, mopt):
    l = line[mopt.end():]
    m = arg_term.search(l)
    res = l[0:m.start()]

    if res == "":
        raise RuntimeError, "Argument to option in command line missing: "+line[mopt.start():]

    return res

match_ac_l      = re.compile(" --ac-handling=")
match_acnag_l   = re.compile(" --ac-non-aggressive")
match_litsel_s  = re.compile(" -W *")
match_litsel_l  = re.compile(" --literal-selection-strategy=")
match_op_l      = re.compile(" --select-on-processing-only")
match_ipl_s     = re.compile(" -i")
match_ipl_l     = re.compile(" --inherit-paramod-literals")
match_ipg_s     = re.compile(" -j")
match_ipg_l     = re.compile(" --inherit-goal-pm-literals")
match_ipc_l     = re.compile(" --inherit-conjecture-pm-literals")
match_pic_l     = re.compile(" --prefer-initial-clauses")
match_split_l   = re.compile(" --split-clauses=")
match_splitm_l  = re.compile(" --split-method=")
match_splita_l  = re.compile(" --split-aggressive")
match_splitr_l  = re.compile(" --split-reuse-defs")
match_der_l     = re.compile(" --destructive-er")
match_sder_l    = re.compile(" --strong-destructive-er")
match_dera_l    = re.compile(" --destructive-er-aggressive")
match_demod_s   = re.compile(" -F *")
match_demod_l   = re.compile(" --forward_demod_level=")
match_snm_l     = re.compile(" --selection-neg-min=")
match_g_demod_s = re.compile(" --prefer-general-demodulators")
match_g_demod_l = re.compile(" -g")
match_unproc_s  = re.compile(" --simplify-with-unprocessed-units=")
match_unproc_sd = re.compile(" --simplify-with-unprocessed-units")
match_fcsr      = re.compile(" --forward-context-sr")
match_fcsra     = re.compile(" --forward-context-sr-aggressive")
match_bcsr      = re.compile(" --backward-context-sr")
match_simul_pm  = re.compile("--simul-paramod");
match_osimul_pm = re.compile("--oriented-simul-paramod");
match_presat_ir = re.compile("--presat-simplify");
match_sos_types = re.compile("--sos-uses-input-types");
match_condense  = re.compile("--condense");
match_condag    = re.compile("--condense-aggressive");



def parse_control_info(line):
    res = ""


    #
    # AC handling and misc. stuff
    #
    m = match_ac_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.ac_handling="+ac_handling[arg]+";\n"

    m = match_acnag_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.ac_res_aggressive=false;\n"

    m = match_pic_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.prefer_initial_clauses=true;\n"

    m = match_unproc_s.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.unproc_simplify="+unproc_simpl[arg]+";\n"
    else:
        m = match_unproc_sd.search(line)
        if m:
            res = res+ "      control->heuristic_parms.unproc_simplify=TopLevelUnitSimplify;\n"

    #
    # Contextual simplify-reflect
    #

    m = match_fcsr.search(line)
    if m:
        res = res+ "      control->heuristic_parms.forward_context_sr = true;\n"

    m = match_fcsra.search(line)
    if m:
        res = res+ "      control->heuristic_parms.forward_context_sr = true;\n"
        res = res+ "      control->heuristic_parms.forward_context_sr_aggressive = true;\n"

    m = match_bcsr.search(line)
    if m:
        res = res+ "      control->heuristic_parms.backward_context_sr = true;\n"

    #
    # Literal selection parameters
    #
    m = match_litsel_s.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.selection_strategy="+selstrat[arg]+";\n"

    m = match_litsel_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.selection_strategy="+selstrat[arg]+";\n"

    m = match_op_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.select_on_proc_only=true;\n"

    m = match_ipl_s.search(line)
    if m:
        res = res+ "      control->heuristic_parms.inherit_paramod_lit=true;\n"

    m = match_ipl_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.inherit_paramod_lit=true;\n"

    m = match_ipg_s.search(line)
    if m:
        res = res+ "      control->heuristic_parms.inherit_goal_pm_lit=true;\n"

    m = match_ipg_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.inherit_goal_pm_lit=true;\n"

    m = match_ipc_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.inherit_conj_pm_lit=true;\n"

    m = match_snm_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.neg_lit_sel_min="+arg+";\n"

    #
    # Splitting parameters
    #
    m = match_split_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.split_clauses="+arg+";\n"

    m = match_splitm_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.split_method="+arg+";\n"

    m = match_splita_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.split_aggressive=true;\n"

    m = match_splitr_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.split_fresh_defs=false;\n"

    #
    # Destructive equality resolution parameters
    #
    m = match_der_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.er_varlit_destructive=true;\n"

    m = match_sder_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.er_strong_destructive=true;\n"
        res = res+ "      control->heuristic_parms.er_varlit_destructive=true;\n"

    m = match_dera_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.er_aggressive=true;\n"

    #
    # Rewriting parameters
    #
    m = match_demod_s.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.forward_demod="+arg+";\n"

    m = match_demod_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.forward_demod="+arg+";\n"

    m = match_g_demod_s.search(line)
    if m:
        res = res+ "      control->heuristic_parms.prefer_general=true;\n"

    m = match_g_demod_l.search(line)
    if m:
        res = res+ "      control->heuristic_parms.prefer_general=true;\n"

    #
    # Paramodulation
    #
    m = match_simul_pm.search(line)
    if m:
        res = res+ "      control->heuristic_parms.pm_type=ParamodAlwaysSim;\n"

    m = match_osimul_pm.search(line)
    if m:
        res = res+ "      control->heuristic_parms.pm_type=ParamodOrientedSim;\n"

    #
    # Presaturation simplification
    #
    m = match_presat_ir.search(line)
    if m:
        res = res+ "      control->heuristic_parms.presat_interreduction=true;\n"

    #
    # Set of Support determination
    #
    m = match_sos_types.search(line)
    if m:
        res = res+ "      control->heuristic_parms.use_tptp_sos=true;\n"

    #
    # Condensation
    #
    m = match_condense.search(line)
    if m:
        res = res+ "      control->heuristic_parms.condensing=true;\n"

    m = match_condag.search(line)
    if m:
        res = res+ "      control->heuristic_parms.condensing_aggressive=true;\n"



    return res


match_sine = re.compile(" --sine=")

def parse_sine(line):
    m = match_sine.search(line)
    if m:
        arg = extract_arg(line, m)
        return arg
    else:
        return None


#
# Regular expressions for ordering related stuff.
#

match_rlc_l = re.compile(" --restrict-literal-comparisons")
match_to_s  = re.compile(" -t *")
match_to_l  = re.compile(" --term-ordering=")
match_tow_s = re.compile(" -w *")
match_tow_l = re.compile(" --order-weight-generation=")
match_top_s = re.compile(" -G *")
match_top_l = re.compile(" --order-precedence-generation=")
match_ocw_l = re.compile(" --order-constant-weight=")
match_ocw_s = re.compile(" -c *")
match_prc_l = re.compile(" --precedence=")

def parse_ordering_info(line):
    res = ""

    m = match_rlc_l.search(line)
    if m:
        res = res+ "      oparms.no_lit_cmp=true;\n"

    m = match_to_s.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.ordertype="+ordering[arg]+";\n"

    m = match_to_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.ordertype="+ordering[arg]+";\n"

    m = match_tow_s.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.to_weight_gen="+weight_gen[arg]+";\n"

    m = match_tow_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.to_weight_gen="+weight_gen[arg]+";\n"

    m = match_top_s.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.to_prec_gen="+prec_gen[arg]+";\n"

    m = match_top_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.to_prec_gen="+prec_gen[arg]+";\n"
    m = match_ocw_s.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.to_const_weight="+arg+";\n"
    m = match_top_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.to_const_weight"+arg+";\n"
#    m = match_prc_l.search(line)
#    if m:
#        arg = extract_arg(line, m)
#        if arg != "":
#            raise RuntimeError, "Can only handle empty precedence "+arg
#       res = res+ "      oparms.to_prec_gen=";\n"

    return res


#------------------------------------------------------------------
# Begin data section
#------------------------------------------------------------------

class_int={
    6: {"U": "AxiomsAreUnit", "H": "AxiomsAreNonUnitHorn", "G": "AxiomsAreGeneral"},
    7: {"U": "GoalsAreUnit", "H": "GoalsAreHorn", "G": "GoalsAreGeneral"},
    8: {"N": "NoEq", "S": "SomeEq", "P": "PureEq"},
    9: {"F": "FewNGPosUnits", "S": "SomeNGPosUnits", "M": "ManyNGPosUnits"},
    10:{"N": "GoalsHaveVars", "G": "GoalsAreGround"},
    11:{"F": "FewAxioms", "S": "SomeAxioms", "M": "ManyAxioms"},
    12:{"F": "FewLiterals", "S": "SomeLiterals", "M": "ManyLiterals"},
    13:{"S": "SmallTerms", "M": "MediumTerms", "L": "LargeTerms"},
    14:{"F": "FewGroundPos", "S": "SomeGroundPos", "M": "ManyGroundPos"},
    15:{"0": "MaxFArity0", "1": "MaxFArity1", "2": "MaxFArity2","3": "MaxFArity3Plus"},
    16:{"0": "AvgFArity0", "1": "AvgFArity1", "2": "AvgFArity2","3": "AvgFArity3Plus"},
    17:{"S": "SmallFArSum", "M": "MediumFArSum", "L": "LargeFArSum"},
    18:{"S": "ShallowMaxDepth", "M": "MediumMaxDepth", "D": "DeepMaxDepth"}
}

selstrat={
   "NoSelection"                        : "SelectNoLiterals",
   "NoGeneration"                       : "SelectNoGeneration",
   "SelectNegativeLiterals"             : "SelectNegativeLiterals",
   "PSelectNegativeLiterals"            : "PSelectNegativeLiterals",
   "SelectPureVarNegLiterals"           : "SelectFirstVariableLiteral",
   "PSelectPureVarNegLiterals"          : "PSelectFirstVariableLiteral",
   "SelectLargestNegLit"                : "SelectLargestNegativeLiteral",
   "PSelectLargestNegLit"               : "PSelectLargestNegativeLiteral",
   "SelectSmallestNegLit"               : "SelectSmallestNegativeLiteral",
   "PSelectSmallestNegLit"              : "PSelectSmallestNegativeLiteral",
   "SelectLargestOrientable"            : "SelectLargestOrientableLiteral",
   "PSelectLargestOrientable"           : "PSelectLargestOrientableLiteral",
   "MSelectLargestOrientable"           : "MSelectLargestOrientableLiteral",
   "SelectSmallestOrientable"           : "SelectSmallestOrientableLiteral",
   "PSelectSmallestOrientable"          : "PSelectSmallestOrientableLiteral",
   "MSelectSmallestOrientable"          : "MSelectSmallestOrientableLiteral",
   "SelectDiffNegLit"                   : "SelectDiffNegativeLiteral",
   "PSelectDiffNegLit"                  : "PSelectDiffNegativeLiteral",
   "SelectGroundNegLit"                 : "SelectGroundNegativeLiteral",
   "PSelectGroundNegLit"                : "PSelectGroundNegativeLiteral",
   "SelectOptimalLit"                   : "SelectOptimalLiteral",
   "PSelectOptimalLit"                  : "PSelectOptimalLiteral",
   "SelectMinOptimalLit"                : "SelectMinOptimalLiteral",
   "PSelectMinOptimalLit"               : "PSelectMinOptimalLiteral",
   "SelectMinOptimalNoTypePred"         : "SelectMinOptimalNoTypePred",
   "PSelectMinOptimalNoTypePred"        : "PSelectMinOptimalNoTypePred",
   "SelectMinOptimalNoXTypePred"        : "SelectMinOptimalNoXTypePred",
   "PSelectMinOptimalNoXTypePred"       : "PSelectMinOptimalNoXTypePred",
   "SelectMinOptimalNoRXTypePred"       : "SelectMinOptimalNoRXTypePred",
   "PSelectMinOptimalNoRXTypePred"      : "PSelectMinOptimalNoRXTypePred",
   "SelectCondOptimalLit"               : "SelectCondOptimalLiteral",
   "PSelectCondOptimalLit"              : "PSelectCondOptimaslLiteral",
   "SelectAllCondOptimalLit"            : "SelectAllCondOptimalLiteral",
   "PSelectAllCondOptimalLit"           : "PSelectAllCondOptimalLiteral",
   "SelectOptimalRestrDepth2"           : "SelectDepth2OptimalLiteral",
   "PSelectOptimalRestrDepth2"          : "PSelectDepth2OptimalLiteral",
   "SelectOptimalRestrPDepth2"          : "SelectPDepth2OptimalLiteral",
   "PSelectOptimalRestrPDepth2"         : "PSelectPDepth2OptimalLiteral",
   "SelectOptimalRestrNDepth2"          : "SelectNDepth2OptimalLiteral",
   "PSelectOptimalRestrNDepth2"         : "PSelectNDepth2OptimalLiteral",
   "SelectNonRROptimalLit"              : "SelectNonRROptimalLiteral",
   "PSelectNonRROptimalLit"             : "PSelectNonRROptimalLiteral",
   "SelectNonStrongRROptimalLit"        : "SelectNonStrongRROptimalLiteral",
   "PSelectNonStrongRROptimalLit"       : "PSelectNonStrongRROptimalLiteral",
   "SelectAntiRROptimalLit"             : "SelectAntiRROptimalLiteral",
   "PSelectAntiRROptimalLit"            : "PSelectAntiRROptimalLiteral",
   "SelectNonAntiRROptimalLit"          : "SelectNonAntiRROptimalLiteral",
   "PSelectNonAntiRROptimalLit"         : "PSelectNonAntiRROptimalLiteral",
   "SelectStrongRRNonRROptimalLit"      : "SelectStrongRRNonRROptimalLiteral",
   "PSelectStrongRRNonRROptimalLit"     : "PSelectStrongRRNonRROptimalLiteral",
   "SelectUnlessUniqMax"                : "SelectUnlessUniqMaxOptimalLiteral",
   "PSelectUnlessUniqMax"               : "PSelectUnlessUniqMaxOptimalLiteral",
   "SelectUnlessPosMax"                 : "SelectUnlessPosMaxOptimalLiteral",
   "PSelectUnlessPosMax"                : "PSelectUnlessPosMaxOptimalLiteral",
   "SelectUnlessUniqPosMax"             : "SelectUnlessUniqPosMaxOptimalLiteral",
   "PSelectUnlessUniqPosMax"            : "PSelectUnlessUniqPosMaxOptimalLiteral",
   "SelectUnlessUniqMaxPos"             : "SelectUnlessUniqMaxPosOptimalLiteral",
   "PSelectUnlessUniqMaxPos"            : "PSelectUnlessUniqMaxPosOptimalLiteral",
   "SelectComplex"                      : "SelectComplex",
   "PSelectComplex"                     : "PSelectComplex",
   "SelectComplexExceptRRHorn"          : "SelectComplexExceptRRHorn",
   "PSelectComplexExceptRRHorn"         : "PSelectComplexExceptRRHorn",
   "SelectLComplex"                     : "SelectLComplex",
   "PSelectLComplex"                    : "PSelectLComplex",
   "SelectMaxLComplex"                  : "SelectMaxLComplex",
   "PSelectMaxLComplex"                 : "PSelectMaxLComplex",
   "SelectMaxLComplexNoTypePred"        : "SelectMaxLComplexNoTypePred",
   "PSelectMaxLComplexNoTypePred"       : "PSelectMaxLComplexNoTypePred",
   "SelectMaxLComplexNoXTypePred"       : "SelectMaxLComplexNoXTypePred",
   "PSelectMaxLComplexNoXTypePred"      : "PSelectMaxLComplexNoXTypePred",
   "SelectComplexPreferNEQ"             : "SelectComplexPreferNEQ",
   "PSelectComplexPreferNEQ"            : "PSelectComplexPreferNEQ",
   "SelectComplexPreferEQ"              : "SelectComplexPreferEQ",
   "PSelectComplexPreferEQ"             : "PSelectComplexPreferEQ",
   "SelectComplexExceptUniqMaxHorn"     : "SelectComplexExceptUniqMaxHorn",
   "PSelectComplexExceptUniqMaxHorn"    : "PSelectComplexExceptUniqMaxHorn",
   "MSelectComplexExceptUniqMaxHorn"    : "MSelectComplexExceptUniqMaxHorn",
   "SelectNewComplex"                   : "SelectNewComplex",
   "PSelectNewComplex"                  : "PSelectNewComplex",
   "SelectNewComplexExceptUniqMaxHorn"  : "SelectNewComplexExceptUniqMaxHorn",
   "PSelectNewComplexExceptUniqMaxHorn" : "PSelectNewComplexExceptUniqMaxHorn",
   "SelectMinInfpos"                    : "SelectMinInfpos",
   "PSelectMinInfpos"                   : "PSelectMinInfpos",
   "HSelectMinInfpos"                   : "HSelectMinInfpos",
   "GSelectMinInfpos"                   : "GSelectMinInfpos",
   "SelectMinInfposNoTypePred"          : "SelectMinInfposNoTypePred",
   "PSelectMinInfposNoTypePred"         : "PSelectMinInfposNoTypePred",
   "SelectMin2Infpos"                   : "SelectMin2Infpos",
   "PSelectMin2Infpos"                  : "PSelectMin2Infpos",
   "SelectComplexExceptUniqMaxPosHorn"  : "SelectComplexExceptUniqMaxPosHorn",
   "PSelectComplexExceptUniqMaxPosHorn" : "PSelectComplexExceptUniqMaxPosHorn",
   "SelectUnlessUniqMaxSmallestOrientable" : "SelectUnlessUniqMaxSmallestOrientable",
   "PSelectUnlessUniqMaxSmallestOrientable": "PSelectUnlessUniqMaxSmallestOrientable",
   "SelectDivLits"                      : "SelectDiversificationLiterals",
   "SelectDivPreferIntoLits"            : "SelectDiversificationPreferIntoLiterals",
   "SelectMaxLComplexG"                 : "SelectMaxLComplexG",

   "SelectMaxLComplexAvoidPosPred"      : "SelectMaxLComplexAvoidPosPred",
   "SelectMaxLComplexAPPNTNp"           : "SelectMaxLComplexAPPNTNp",
   "SelectMaxLComplexAvoidPosUPred"     : "SelectMaxLComplexAvoidPosUPred",
   "SelectComplexG"                     : "SelectComplexG",

   "SelectComplexAHP"                   : "SelectComplexAHP",
   "PSelectComplexAHP"                  : "PSelectComplexAHP",

   "SelectNewComplexAHP"                : "SelectNewComplexAHP",
   "PSelectNewComplexAHP"               : "PSelectNewComplexAHP",

   "SelectComplexAHPExceptRRHorn"       : "SelectComplexAHPExceptRRHorn",
   "PSelectComplexAHPExceptRRHorn"      : "PSelectComplexAHPExceptRRHorn",

   "SelectNewComplexAHPExceptRRHorn"       : "SelectNewComplexAHPExceptRRHorn",
   "PSelectNewComplexAHPExceptRRHorn"      : "PSelectNewComplexAHPExceptRRHorn",

   "SelectNewComplexAHPExceptUniqMaxHorn" : "SelectNewComplexAHPExceptUniqMaxHorn",
   "PSelectNewComplexAHPExceptUniqMaxHorn": "PSelectNewComplexAHPExceptUniqMaxHorn",

   "SelectNewComplexAHPNS"              : "SelectNewComplexAHPNS",
   "SelectVGNonCR"                      : "SelectVGNonCR",

   "SelectCQArEqLast"                   : "SelectCQArEqLast",
   "SelectCQArEqFirst"                  : "SelectCQArEqFirst",
   "SelectCQIArEqLast"                  : "SelectCQIArEqLast",
   "SelectCQIArEqFirst"                 : "SelectCQIArEqFirst",
   "SelectCQAr"                         : "SelectCQAr",
   "SelectCQIAr"                        : "SelectCQIAr",
   "SelectCQArNpEqFirst"                : "SelectCQArNpEqFirst",
   "SelectCQIArNpEqFirst"               : "SelectCQIArNpEqFirst",
   "SelectGrCQArEqFirst"                : "SelectGrCQArEqFirst",
   "SelectCQGrArEqFirst"                : "SelectCQGrArEqFirst",
   "SelectCQArNTEqFirst"                : "SelectCQArNTEqFirst",
   "SelectCQIArNTEqFirst"               : "SelectCQIArNTEqFirst",
   "SelectCQArNTNpEqFirst"              : "SelectCQArNTNpEqFirst",
   "SelectCQIArNTNpEqFirst"             : "SelectCQIArNTNpEqFirst",
   "SelectCQArNXTEqFirst"               : "SelectCQArNXTEqFirst",
   "SelectCQIArNXTEqFirst"              : "SelectCQIArNXTEqFirst",

   "SelectCQArNTNp"                     :  "SelectCQArNTNp",
   "SelectCQIArNTNp"                    :  "SelectCQIArNTNp",
   "SelectCQArNT"                       :  "SelectCQArNT",
   "SelectCQIArNT"                      :  "SelectCQIArNT",
   "SelectCQArNp"                       :  "SelectCQArNp",
   "SelectCQIArNp"                      :  "SelectCQIArNp",

   "SelectCQArNpEqFirstUnlessPDom"      : "SelectCQArNpEqFirstUnlessPDom",
   "SelectCQArNTEqFirstUnlessPDom"      : "SelectCQArNTEqFirstUnlessPDom",

   "SelectCQPrecW"                      : "SelectCQPrecW",
   "SelectCQIPrecW"                     : "SelectCQIPrecW",
   "SelectCQPrecWNTNp"                  : "SelectCQPrecWNTNp",
   "SelectCQIPrecWNTNp"                 : "SelectCQIPrecWNTNp"
}




ac_handling ={
    "None"          : "NoACHandling",
    "DiscardAll"    : "ACDiscardAll",
    "KeepUnits"     : "ACKeepUnits",
    "KeepOrientable": "ACKeepOrientable"
}

ordering={
    "LPO" :"LPO",
    "LPO4":"LPO4",
    "KBO" :"KBO",
    "KBO6":"KBO6",
    "KBO1":"KBO"
}

weight_gen={
   "none"               : "WNoMethod",
   "firstmaximal0"      : "WSelectMaximal",
   "arity"              : "WArityWeight ",
   "aritymax0"          : "WArityMax0",
   "modarity"           : "WModArityWeight",
   "modaritymax0"       : "WModArityMax0",
   "aritysquared"       : "WAritySqWeight",
   "aritysquaredmax0"   : "WAritySqMax0",
   "invarity"           : "WInvArityWeight",
   "invaritymax0"       : "WInvArityMax0",
   "invaritysquared"    : "WInvSqArityWeight",
   "invaritysquaredmax0": "WInvAritySqMax0",
   "precedence"         : "WPrecedence",
   "invprecedence"      : "WPrecedenceInv",
   "precrank5"          : "WPrecRank5",
   "precrank10"         : "WPrecRank10",
   "precrank20"         : "WPrecRank20",
   "freqcount"          : "WFrequency",
   "invfreqcount"       : "WInvFrequency",
   "freqrank"           : "WFrequencyRank",
   "invfreqrank"        : "WInvFrequencyRank",
   "invconjfreqrank"    : "WInvConjFrequencyRank",
   "freqranksquare"     : "WFrequencyRankSq",
   "invfreqranksquare"  : "WInvFrequencyRankSq",
   "invmodfreqrank"     : "WInvModFreqRank",
   "invmodfreqrankmax0" : "WInvModFreqRankMax0",
   "constant"           : "WConstantWeight"
}

prec_gen={
   "none"            : "PNoMethod",
   "unary_first"     : "PUnaryFirst",
   "unary_freq"      : "PUnaryFirstFreq",
   "arity"           : "PArity",
   "invarity"        : "PInvArity",
   "const_max"       : "PConstFirst ",
   "const_min"       : "PInvArConstMin",
   "freq"            : "PByFrequency",
   "invfreq"         : "PByInvFrequency",
   "invconjfreq"     : "PByInvConjFrequency",
   "invfreqconjmax"  : "PByInvFreqConjMax",
   "invfreqconjmin"  : "PByInvFreqConjMin",
   "invfreqconstmin" : "PByInvFreqConstMin",
   "invfreqhack"     : "PByInvFreqHack",
   "orient_axioms"   : "POrientAxioms"
}

unproc_simpl={
    "NoSimplify"     : "NoUnitSimplify",
    "TopSimplify"    : "TopLevelUnitSimplify",
    "FullSimplify"   : "FullUnitSimplify"
}

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
        fp.write(heuristic_define(strat)+"\n")
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
        for i in by_heuristic.keys():
            print_strat_once(fp, i, defined_strats)

        if global_best in by_heuristic.keys():
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

global_class = ClassPerf("ALL")

# Now problems is a dictionary associating every class with the
# associated problems and stratset is a dictionary associating the
# "local" name of the protocol with the full (relative or absolute)
# file name.

for i in stratset.keys():
    stratdesc[i] = parse_prot_new(stratset[i], i, problems, global_class, classdata)


# Start determining the strategies

sys.stderr.write("Parsing done, running optimizer\n")

time_limits = [152, 74, 24, 18, 13, 9, 5, 5]

itercount = 0;

defined_strats = set()

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
            best = global_best
        result[i] = best
        opt_res[i] = solns
        used.add(best)

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
