#!/usr/bin/env python2.7

import sys
import re
import string


white_space     = re.compile('\s+')
trail_space     = re.compile('\s*$')
arg_term        = re.compile('\s|$')
full_comment    = re.compile('^#')
dash            = re.compile('-')
csv             = re.compile('.csv');
slash           = re.compile('/')
match_heuristic = re.compile("-H'?\([^#]*\)'?")
match_class     = re.compile('CLASS_[a-zA-Z-0-9]*$')
match_prot      = re.compile('protocol')
eval_f_sep      = re.compile('\),')
problem_ext     = re.compile('\.[a-z]*$')



def extract_arg(line, mopt):
    l = line[mopt.end():]
    m = arg_term.search(l)
    res = l[0:m.start()]

    if res == "":
        raise RuntimeError, "Argument to option in command line missing: "+line[mopt.start():]

    return res

def extract_opt_arg(line, mopt, opt):
    l = line[mopt.end():]
    # print "extract_opt_arg(", line, ") l=", l
    if len(l)==0:
        return opt
    if l[0]!="=":
        return opt

    m = arg_term.search(l[1:])
    res = l[1:m.start()+1]
    #print "extract_opt_arg(",line, opt, ") =>", res
    return res

def trans_heuristic_name(name):
    tmp = re.sub('p[^_]*_', "", name);
    tmp = re.sub(dash,"_", tmp)
    tmp = re.sub('[.]csv',"", tmp)
    if not tmp[0] in string.ascii_letters:
        tmp = "h"+tmp
    return tmp

def clean_heuristic(heuristic_string):
    """
    Remove traling gunk ;-)
    """
    count = 1
    for i in range(1, len(heuristic_string)):
        if heuristic_string[i] == "(":
            count += 1
        if heuristic_string[i] == ")":
            count -= 1
        if count == 0:
            heuristic_string = heuristic_string[0:i+1]
            break
    return heuristic_string


def heuristic_define(name, stratdesc):
    mr = match_heuristic.search(stratdesc[name])
    if not mr:
        raise RuntimeError, "No heuristic defined in " + name;
    res= '"' + trans_heuristic_name(name) + ' = \\n"\n"'
    tmp = stratdesc[name][mr.start()+2:mr.end()]
    tmp = re.sub("'", "", tmp)
    tmp = clean_heuristic(tmp)
    res=res+ re.sub(eval_f_sep,'),"\n" ',tmp) +'\\n"'

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
#match_unproc_s  = re.compile(" --simplify-with-unprocessed-units=")
#match_unproc_sd = re.compile(" --simplify-with-unprocessed-units")
match_fcsr      = re.compile(" --forward-context-sr")
match_fcsra     = re.compile(" --forward-context-sr-aggressive")
match_bcsr      = re.compile(" --backward-context-sr")
match_simul_pm  = re.compile(" --simul-paramod");
match_osimul_pm = re.compile(" --oriented-simul-paramod");
match_ssimul_pm = re.compile(" --supersimul-paramod");
match_ossimul_pm= re.compile(" --oriented-supersimul-paramod");
match_presat_ir = re.compile(" --presat-simplify");
match_sos_types = re.compile(" --sos-uses-input-types");
match_condense  = re.compile(" --condense");
match_condag    = re.compile(" --condense-aggressive");

match_sat_proc_int    = re.compile(" --satcheck-proc-interval")
match_sat_gen_int     = re.compile(" --satcheck-gen-interval")
match_sat_tt_int      = re.compile(" --satcheck-ttinsert-interval")
match_satcheck        = re.compile(" --satcheck(?=[^-])")
match_sat_norm_const  = re.compile(" --satcheck-normalize-const")
match_sat_norm_unproc = re.compile(" --satcheck-normalize-unproc")
match_sat_dec_limit   = re.compile(" --satcheck-decision-limit")

match_unfold_limit      = re.compile(" --eq-unfold-limit=")
match_unfold_maxclauses = re.compile(" --eq-unfold-maxclauses=")
match_no_unfold         = re.compile(" --no-eq-unfolding")



def parse_control_info2(line, res={}):

    #
    # AC handling and misc. stuff
    #
    m = match_ac_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res["ac_handling"] = ac_handling[arg]

    m = match_acnag_l.search(line)
    if m:
        res["ac_res_aggressive"] = "false"

    m = match_pic_l.search(line)
    if m:
        res["prefer_initial_clauses"] ="true"

#    m = match_unproc_s.search(line)
#    if m:
#        arg = extract_arg(line, m)
#        res["unproc_simplify"] = unproc_simpl[arg]
#    else:
#        m = match_unproc_sd.search(line)
#        if m:
#            res["unproc_simplify"] ="TopLevelUnitSimplify"

    #
    # Contextual simplify-reflect
    #

    m = match_fcsr.search(line)
    if m:
        res["forward_context_sr"] = "true"

    m = match_fcsra.search(line)
    if m:
        res["forward_context_sr"] = "true"
        res["forward_context_sr_aggressive"] = "true"

    m = match_bcsr.search(line)
    if m:
        res["backward_context_sr"] = "true"

    #
    # Literal selection parameters
    #
    m = match_litsel_s.search(line)
    if m:
        arg = extract_arg(line, m)
        res["selection_strategy"] = selstrat[arg]

    m = match_litsel_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res["selection_strategy"] = selstrat[arg]

    m = match_op_l.search(line)
    if m:
        res["select_on_proc_only"] = "true"

    m = match_ipl_s.search(line)
    if m:
        res["inherit_paramod_lit"] = "true"

    m = match_ipl_l.search(line)
    if m:
        res["inherit_paramod_lit"] = "true"

    m = match_ipg_s.search(line)
    if m:
        res["inherit_goal_pm_lit"] = "true"

    m = match_ipg_l.search(line)
    if m:
        res["inherit_goal_pm_lit"] = "true"

    m = match_ipc_l.search(line)
    if m:
        res["inherit_conj_pm_lit"] = "true"

    m = match_snm_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res["neg_lit_sel_min"] = arg

    #
    # Splitting parameters
    #
    m = match_split_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res["split_clauses"] = arg

    m = match_splitm_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res["split_method"] = arg

    m = match_splita_l.search(line)
    if m:
        res["split_aggressive"] = "true"

    m = match_splitr_l.search(line)
    if m:
        res["split_fresh_defs"] = "false"

    #
    # Destructive equality resolution parameters
    #
    m = match_der_l.search(line)
    if m:
        res["er_varlit_destructive"] = "true"

    m = match_sder_l.search(line)
    if m:
        res["er_strong_destructive"] = "true"
        res["er_varlit_destructive"] = "true"

    m = match_dera_l.search(line)
    if m:
        res["er_aggressive"] = "true"

    #
    # Rewriting parameters
    #
    m = match_demod_s.search(line)
    if m:
        arg = extract_arg(line, m)
        res["forward_demod"] = arg

    m = match_demod_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res["forward_demod"] = arg

    m = match_g_demod_s.search(line)
    if m:
        res["prefer_general"] = "true"

    m = match_g_demod_l.search(line)
    if m:
        res["prefer_general"] = "true"

    #
    # Paramodulation
    #
    m = match_simul_pm.search(line)
    if m:
        res["pm_type"] = "ParamodSim"

    m = match_ssimul_pm.search(line)
    if m:
        res["pm_type"] = "ParamodSuperSim"

    m = match_osimul_pm.search(line)
    if m:
        res["pm_type"] = "ParamodOrientedSim"

    m = match_ossimul_pm.search(line)
    if m:
        res["pm_type"] = "ParamodOrientedSuperSim"

    #
    # Presaturation simplification
    #
    m = match_presat_ir.search(line)
    if m:
        res["presat_interreduction"] = "true"

    #
    # Set of Support determination
    #
    m = match_sos_types.search(line)
    if m:
        res["use_tptp_sos"] = "true"

    #
    # Condensation
    #
    m = match_condense.search(line)
    if m:
        res["condensing"] = "true"

    m = match_condag.search(line)
    if m:
        res["condensing_aggressive"] = "true"

    #
    # SAT
    #

    m = match_sat_proc_int.search(line)
    if m:
        arg = extract_opt_arg(line, m, "5000")
        res["sat_check_step_limit"] = arg

    m = match_sat_gen_int.search(line)
    if m:
        arg = extract_opt_arg(line, m, "10000")
        res["sat_check_size_limit"] = arg

    m = match_sat_tt_int.search(line)
    if m:
        arg = extract_opt_arg(line, m, "5000000")
        res["sat_check_ttinsert_limit"] = arg

    m = match_satcheck.search(line)
    if m:
        # print "XXXXX"
        arg = extract_opt_arg(line, m, "FirstConst")
        # print "YYYY", arg
        res["sat_check_grounding"] = sat_trans[arg]

    m = match_sat_norm_const.search(line)
    if m:
        res["sat_check_normconst"] = "true"

    m = match_sat_norm_unproc.search(line)
    if m:
        res["sat_check_normalize"] = "true"

    m = match_sat_dec_limit.search(line)
    if m:
        arg = extract_opt_arg(line, m, "100")
        res["sat_check_decision_limit"] = arg

    # Eq unfolding
    m = match_unfold_limit.search(line)
    if m:
        arg = extract_arg(line, m)
        res["eqdef_incrlimit"] = arg

    m = match_unfold_maxclauses.search(line)
    if m:
        arg = extract_arg(line, m)
        res["eqdef_maxclauses"] = arg

    m = match_no_unfold.search(line)
    if m:
        res["eqdef_incrlimit"] = "-9223372036854775808"

    print(res)
    return res


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

    # m = match_unproc_s.search(line)
    # if m:
    #     arg = extract_arg(line, m)
    #     res = res+ "      control->heuristic_parms.unproc_simplify="+unproc_simpl[arg]+";\n"
    # else:
    #     m = match_unproc_sd.search(line)
    #     if m:
    #         res = res+ "      control->heuristic_parms.unproc_simplify=TopLevelUnitSimplify;\n"

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
        res = res+ "      control->heuristic_parms.pm_type=ParamodSim;\n"

    m = match_ssimul_pm.search(line)
    if m:
        res = res+ "      control->heuristic_parms.pm_type=ParamodSuperSim;\n"

    m = match_osimul_pm.search(line)
    if m:
        res = res+ "      control->heuristic_parms.pm_type=ParamodOrientedSim;\n"

    m = match_ossimul_pm.search(line)
    if m:
        res = res+ "      control->heuristic_parms.pm_type=ParamodOrientedSuperSim;\n"

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

    #
    # SAT
    #

    m = match_sat_proc_int.search(line)
    if m:
        arg = extract_opt_arg(line, m, "5000")
        res = res+ "      control->heuristic_parms.sat_check_step_limit="+arg+";\n"

    m = match_sat_gen_int.search(line)
    if m:
        arg = extract_opt_arg(line, m, "10000")
        res = res+ "      control->heuristic_parms.sat_check_size_limit="+arg+";\n"

    m = match_sat_tt_int.search(line)
    if m:
        arg = extract_opt_arg(line, m, "5000000")
        res = res+ "      control->heuristic_parms.sat_check_ttinsert_limit="+arg+";\n"

    m = match_satcheck.search(line)
    if m:
        # print "XXXXX"
        arg = extract_opt_arg(line, m, "FirstConst")
        # print "YYYY", arg
        res = res+ "      control->heuristic_parms.sat_check_grounding="+sat_trans[arg]+";\n"
    m = match_sat_norm_const.search(line)
    if m:
        res = res+ "      control->heuristic_parms.sat_check_normconst=true;\n"

    m = match_sat_norm_unproc.search(line)
    if m:
        res = res+ "      control->heuristic_parms.sat_check_normalize=true;\n"

    m = match_sat_dec_limit.search(line)
    if m:
        arg = extract_opt_arg(line, m, "100")
        res = res+ "      control->heuristic_parms.sat_check_decision_limit="+arg+";\n"

    # Eq unfolding
    m = match_unfold_limit.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.eqdef_incrlimit="+arg+";\n"

    m = match_unfold_maxclauses.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      control->heuristic_parms.eqdef_maxclauses="+arg+";\n"

    m = match_no_unfold.search(line)
    if m:
        res = res+ "      control->heuristic_parms.eqdef_incrlimit=LONG_MIN;\n"

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

match_rlc_l  = re.compile(" --restrict-literal-comparisons")
match_lc_l   = re.compile(" --literal-comparison=")
match_to_s   = re.compile(" -t *")
match_to_l   = re.compile(" --term-ordering=")
match_tow_s  = re.compile(" -w *")
match_tow_l  = re.compile(" --order-weight-generation=")
match_top_s  = re.compile(" -G *")
match_top_l  = re.compile(" --order-precedence-generation=")
match_ocw_l  = re.compile(" --order-constant-weight=")
match_ocw_s  = re.compile(" -c *")
match_prc_l  = re.compile(" --precedence=")
match_srhs_l = re.compile(" --strong-rw-inst")



match_ppc_l = re.compile("--prec-pure-conj=")
match_pca_l = re.compile("--prec-conj-axiom=")
match_ppa_l = re.compile("--prec-pure-axiom=")
match_psk_l = re.compile("--prec-skolem=")
match_pdp_l = re.compile("--prec-defpred=")


def parse_ordering_info(line):
    res = ""

    m = match_rlc_l.search(line)
    if m:
        res = res+ "      oparms.lit_cmp=LCNoCmp;\n"

    m = match_lc_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.lit_cmp="+litcmp[arg]+";\n"

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
        res = res+ "      oparms.to_const_weight="+arg+";\n"
#    m = match_prc_l.search(line)
#    if m:
#        arg = extract_arg(line, m)
#        if arg != "":
#            raise RuntimeError, "Can only handle empty precedence "+arg
#       res = res+ "      oparms.to_prec_gen=";\n"

    m = match_srhs_l.search(line)
    if m:
        res = res+ "      oparms.rewrite_strong_rhs_inst=true;\n"



    m = match_ppc_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.conj_only_mod="+arg+";\n"

    m = match_pca_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.conj_axiom_mod="+arg+";\n"

    m = match_ppa_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.axiom_only_mod="+arg+";\n"

    m = match_psk_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.skolem_mod="+arg+";\n"

    m = match_pdp_l.search(line)
    if m:
        arg = extract_arg(line, m)
        res = res+ "      oparms.defpred_mod="+arg+";\n"

    return res


#------------------------------------------------------------------
# Begin data section
#------------------------------------------------------------------

class_int={
    6: {"F": "IsFO", "H":"IsHO"},
    7: {"U": "AxiomsAreUnit", "H": "AxiomsAreNonUnitHorn", "G": "AxiomsAreGeneral"},
    8: {"U": "GoalsAreUnit", "H": "GoalsAreHorn", "G": "GoalsAreGeneral"},
    9: {"N": "NoEq", "S": "SomeEq", "P": "PureEq"},
    10: {"F": "FewNGPosUnits", "S": "SomeNGPosUnits", "M": "ManyNGPosUnits"},
    11:{"N": "GoalsHaveVars", "G": "GoalsAreGround"},
    12:{"F": "FewAxioms", "S": "SomeAxioms", "M": "ManyAxioms"},
    13:{"F": "FewLiterals", "S": "SomeLiterals", "M": "ManyLiterals"},
    14:{"S": "SmallTerms", "M": "MediumTerms", "L": "LargeTerms"},
    15:{"F": "FewGroundPos", "S": "SomeGroundPos", "M": "ManyGroundPos"},
    16:{"0": "MaxFArity0", "1": "MaxFArity1", "2": "MaxFArity2","3": "MaxFArity3Plus"},
    17:{"0": "AvgFArity0", "1": "AvgFArity1", "2": "AvgFArity2","3": "AvgFArity3Plus"},
    18:{"S": "SmallFArSum", "M": "MediumFArSum", "L": "LargeFArSum"},
    19:{"S": "ShallowMaxDepth", "M": "MediumMaxDepth", "D": "DeepMaxDepth"}
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
   "PSelectCondOptimalLit"              : "PSelectCondOptimalLiteral",
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
   "SelectMaxLComplexAPPNoType"         : "SelectMaxLComplexAPPNoType",
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

litcmp={
    "None"     : "LCNoCmp",
    "Normal"   : "LCNormal",
    "TFOEqMax" : "LCTFOEqMax",
    "TFOEqMin" : "LCTFOEqMin"
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
   "typefreqrank"       : "WTypeFrequencyRank",
   "typefreqcount"      : "WTypeFrequencyCount",
   "invtypefreqrank"    : "WInvTypeFrequencyRank",
   "invtypefreqcount"   : "WInvTypeFrequencyCount",
   "combfreqrank"       : "WCombFrequencyRank",
   "combfreqcount"      : "WCombFrequencyCount",
   "invcombfreqrank"    : "WInvCombFrequencyRank",
   "invcombfreqcount"   : "WInvCombFrequencyCount",
   "constant"           : "WConstantWeight"
}

prec_gen={
   "none"            : "PNoMethod",
   "unary_first"     : "PUnaryFirst",
   "unary_freq"      : "PUnaryFirstFreq",
   "arity"           : "PArity",
   "invarity"        : "PInvArity",
   "const_max"       : "PConstMax ",
   "const_min"       : "PInvArConstMin",
   "freq"            : "PByFrequency",
   "invfreq"         : "PByInvFrequency",
   "invconjfreq"     : "PByInvConjFrequency",
   "invfreqconjmax"  : "PByInvFreqConjMax",
   "invfreqconjmin"  : "PByInvFreqConjMin",
   "invfreqconstmin" : "PByInvFreqConstMin",
   "invfreqhack"     : "PByInvFreqHack",
   "typefreq"        : "PByTypeFreq",
   "invtypefreq"     : "PByInvTypeFreq",
   "combfreq"        : "PByCombFreq",
   "invcombfreq"     : "PByInvCombFreq",
   "arrayopt"        : "PArrayOpt",
   "orient_axioms"   : "POrientAxioms"
}

unproc_simpl={
    "NoSimplify"     : "NoUnitSimplify",
    "TopSimplify"    : "TopLevelUnitSimplify",
    "FullSimplify"   : "FullUnitSimplify"
}

sat_trans = {
   "NoGrounding":      "GMNoGrounding",
   "PseudoVar":        "GMPseudoVar",
   "FirstConst":       "GMFirstConst",
   "ConjMinMinFreq":   "GMConjMinMinFreq",
   "ConjMaxMinFreq":   "GMConjMaxMinFreq",
   "ConjMinMaxFreq":   "GMConjMinMaxFreq",
   "ConjMaxMaxFreq":   "GMConjMaxMaxFreq",
   "GlobalMax":        "GMGlobalMax",
   "GlobalMin":        "GMGlobalMin"
   }
