/*-----------------------------------------------------------------------

File  : eprover.c

Author: Stephan Schulz

Contents
 
  Main program for the E equational theorem prover.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jun  9 01:32:15 MET DST 1998
    New
<2> See CVS log ;-)
<3> Fri Sep 14 15:29:08 EDT 2007
    Release of 0.999.

-----------------------------------------------------------------------*/


#include <clb_defines.h>
#include <clb_regmem.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <ccl_relevance.h>
#include <cco_proofproc.h>
#include <cco_sine.h>
#include <cio_signals.h>
#include <ccl_unfold_defs.h>
#include <ccl_formulafunc.h>
#include <cte_simplesorts.h>
#include <cco_scheduling.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME         "eprover"

PERF_CTR_DEFINE(SatTimer);

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_PRINT_STATISTICS,
   OPT_EXPENSIVE_DETAILS,   
   OPT_PRINT_SATURATED,
   OPT_PRINT_SAT_INFO,
   OPT_FILTER_SATURATED,
   OPT_PRUNE_ONLY,
   OPT_CNF_ONLY,
   OPT_PRINT_PID,
   OPT_PRINT_VERSION,
   OPT_REQUIRE_NONEMPTY,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_PROOF_OBJECT,
   OPT_PCL_COMPRESSED,
   OPT_PCL_COMPACT,
   OPT_PCL_SHELL_LEVEL,
   OPT_MEM_LIMIT,
   OPT_CPU_LIMIT,
   OPT_SOFTCPU_LIMIT,
   OPT_RUSAGE_INFO,
   OPT_STEP_LIMIT,
   OPT_ANSWER_LIMIT,
   OPT_CONJ_ARE_QUEST,
   OPT_PROC_LIMIT,
   OPT_UNPROC_LIMIT,
   OPT_TOTAL_LIMIT,
   OPT_NO_INFIX,
   OPT_FULL_EQ_REP,
   OPT_TPTP_PARSE,
   OPT_TPTP_PRINT,
   OPT_TPTP_FORMAT,
   OPT_TSTP_PARSE,
   OPT_TSTP_PRINT,
   OPT_TSTP_FORMAT,
   OPT_AUTO,
   OPT_SATAUTO,
   OPT_AUTODEV,
   OPT_SATAUTODEV,
   OPT_AUTO_SCHED,
   OPT_NO_PREPROCESSING,
   OPT_EQ_UNFOLD_LIMIT,
   OPT_EQ_UNFOLD_MAXCLAUSES,
   OPT_NO_EQ_UNFOLD,
   OPT_SINE,
   OPT_REL_PRUNE_LEVEL,
   OPT_PRESAT_SIMPLIY,
   OPT_AC_HANDLING,
   OPT_AC_ON_PROC,
   OPT_LITERAL_SELECT,
   OPT_NO_GENERATION,
   OPT_SELECT_ON_PROC_ONLY,
   OPT_INHERIT_PM_LIT,
   OPT_INHERIT_GOAL_PM_LIT,
   OPT_INHERIT_CONJ_PM_LIT,
   OPT_POS_LITSEL_MIN,
   OPT_POS_LITSEL_MAX,
   OPT_NEG_LITSEL_MIN,
   OPT_NEG_LITSEL_MAX,
   OPT_ALL_LITSEL_MIN,
   OPT_ALL_LITSEL_MAX,
   OPT_WEIGHT_LITSEL_MIN,
   OPT_PREFER_INITIAL_CLAUSES,
   OPT_FILTER_LIMIT,
   OPT_FILTER_COPIES_LIMIT,
   OPT_REWEIGHT_LIMIT,
   OPT_DELETE_BAD_LIMIT,
   OPT_ASSUME_COMPLETENESS,
   OPT_ASSUME_INCOMPLETENESS,
   OPT_DISABLE_EQ_FACTORING,
   OPT_DISABLE_NEGUNIT_PM,
   OPT_CONDENSING,
   OPT_CONDENSING_AGGRESSIVE,
   OPT_NO_GC_FORWARD_SIMPL,
   OPT_USE_SIM_PARAMOD,
   OPT_USE_ORIENTED_SIM_PARAMOD,
   OPT_SPLIT_TYPES,
   OPT_SPLIT_HOW,
   OPT_SPLIT_AGGRESSIVE,
   OPT_SPLIT_REUSE_DEFS,
   OPT_ORDERING,
   OPT_TO_WEIGHTGEN,
   OPT_TO_WEIGHTS,
   OPT_TO_PRECGEN,
   OPT_TO_CONSTWEIGHT,
   OPT_TO_PRECEDENCE,
   OPT_TO_LPO_RECLIMIT,
   OPT_TO_RESTRICT_LIT_CMPS,
   OPT_TPTP_SOS,
   OPT_ER_DESTRUCTIVE,
   OPT_ER_STRONG_DESTRUCTIVE,
   OPT_ER_AGGRESSIVE,
   OPT_FORWARD_CSR,
   OPT_FORWARD_CSR_AGGRESSIVE,
   OPT_BACKWARD_CSR,
   OPT_RULES_GENERAL,
   OPT_FORWARD_DEMOD,
   OPT_STRONG_RHS_INSTANCE,
   OPT_STRONGSUBSUMPTION,
   OPT_WATCHLIST,
   OPT_WATCHLIST_NO_SIMPLIFY,
   OPT_NO_INDEXED_SUBSUMPTION,
   OPT_FVINDEX_STYLE,
   OPT_FVINDEX_FEATURETYPES,
   OPT_FVINDEX_MAXFEATURES,
   OPT_FVINDEX_SLACK,
   OPT_RW_BW_INDEX,
   OPT_PM_FROM_INDEX,
   OPT_PM_INTO_INDEX,
   OPT_FP_INDEX,
   OPT_DETSORT_RW,
   OPT_DETSORT_NEW,
   OPT_DEFINE_WFUN,
   OPT_DEFINE_HEURISTIC,
   OPT_HEURISTIC,
   OPT_FREE_NUMBERS,
   OPT_FREE_OBJECTS,
   OPT_OLD_STYLE_CNF,
   OPT_DEF_CNF,
   OPT_DUMMY
}OptionCodes;



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


OptCell opts[] =
{
   {OPT_HELP, 
    'h', "help", 
    NoArg, NULL,
    "Print a short description of program usage and options."},

   {OPT_VERSION,
    'V', "version",
    NoArg, NULL,
    "Print the version number of the prover. Please include this"
    " with all bug reports (if any)."},

   {OPT_VERBOSE, 
    'v', "verbose", 
    OptArg, "1",
    "Verbose comments on the progress of the program. This differs "
    "from the output level (below) in that technical information is "
    "printed to stderr, while the output level determines which "
    "logical manipulations of the clauses are printed to stdout."},

   {OPT_OUTPUT,
    'o', "output-file",
    ReqArg, NULL,
   "Redirect output into the named file."},

   {OPT_SILENT,
    's', "silent",
    NoArg, NULL,
    "Equivalent to --output-level=0."},   

   {OPT_OUTPUTLEVEL,
    'l', "output-level",
    ReqArg, NULL,
    "Select an output level, greater values imply more verbose "
    "output. Level 0 produces "
    "nearly no output, level 1 will output each clause as it is "
    "processed, level 2 will output generating inferences, "
    "level 3 will give a full protocol including rewrite steps and "
    "level 4 will include some internal clause renamings. Levels >= 2"
    " also imply PCL2 or TSTP formats (which can be post-processed"
    " with suitable tools)."},
   
   {OPT_PROOF_OBJECT,
    'p', "proof-object",
    OptArg, "1",
    "Generate (and print, in case of success) an internal proof object. "
    "Level 0 will not build a proof object, level 1 will build a "
    "simple, compact proof object that only contains inference rules "
    "and dependencies, level 2 will build a proof object where "
    "inferences are unambiguously described by giving inference "
    "positions, and level 3 will expand this to a proof object where "
    "all intermediate results are explicit. This feature is under "
    "development, so far only level 0 and 1 are operational."}, 

   {OPT_PCL_COMPRESSED,
    '\0', "pcl-terms-compressed",
    NoArg, NULL,
    "Print terms in the PCL output in shared representation."},

   {OPT_PCL_COMPACT,
    '\0', "pcl-compact",
    NoArg, NULL,
    "Print PCL steps without additional spaces for formatting (safes "
    "disk space for large protocols)."},

   {OPT_PCL_SHELL_LEVEL,
    '\0', "pcl-shell-level",
    OptArg, "1",
    "Determines level to which clauses and formulas are suppressed "
    "in the output. Level 0 will print all, level 1 will only print "
    "initial clauses/formulas, level 2 will print no clauses or axioms. "
    "All levels will still print the dependency graph."},

   {OPT_PRINT_STATISTICS,
    '\0', "print-statistics",
    NoArg, NULL,
    "Print the inference statistics (only relevant for output level 0,"
    "otherwise they are printed automatically."},

   {OPT_EXPENSIVE_DETAILS,
    '0', "print-detailed-statistics",
    NoArg, NULL,
    "Print data about the proof state that is potentially expensive "
    "to collect. Includes number of term cells and number of "
    "rewrite steps."}, 

   {OPT_PRINT_SATURATED,
    'S', "print-saturated",
    OptArg, DEFAULT_OUTPUT_DESCRIPTOR,
    "Print the (semi-) saturated clause sets after terminating the "
    "saturation process. The argument given describes which parts "
    "should be printed in which order. Legal characters are 'eigEIGaA',"
    " standing for processed positive units, processed negative units, "
    "processed non-units, unprocessed positive units, unprocessed "
    "negative units, unprocessed non-units, and two types of "
    "additional equality axioms, respectively. Equality axioms will "
    "only be printed if the original specification contained real "
    "equality. In this case, 'a' requests axioms in which a separate" 
    " substitutivity axiom is given for each argument position of a"
    " function or predicate symbol, while 'A' requests a single "
    "substitutivity axiom (covering all positions) for each symbol."},  

   {OPT_PRINT_SAT_INFO,
    '\0', "print-sat-info",
    NoArg, NULL,
    "Print additional information (clause number, weight, etc) as a"
    " comment for clauses from the semi-saturated end system."},

   {OPT_FILTER_SATURATED,
    '\0', "filter-saturated",
    OptArg, DEFAULT_FILTER_DESCRIPTOR,
    "Filter the  (semi-) saturated clause sets after terminating the "
    "saturation process. The argument is a string describing which "
    "operations to take (and in which order). Options are 'u' (remove"
    " all clauses with more than one literal), 'c' (delete all but one"
    " copy of identical clauses, 'n', 'r', 'f' (forward contraction, "
    "unit-subsumption only, no rewriting, rewriting with rules only,"
    " full rewriting, respectively), and 'N', 'R' and 'F' (as their"
    " lower case counterparts, but with non-unit-subsumption enabled"
    " as well)."}, 

   {OPT_PRUNE_ONLY,
    '\0', "prune",
    NoArg, NULL,
    "Stop after relevancy pruning, SInE pruning, and output of the "
    "initial clause- and formula set. This will automatically set "
    "output level to 4 so that the pruned problem specification is "
    "printed. Note that the desired pruning methods must still be "
    "specified (e.g. '--sine=Auto'"},
   
   {OPT_CNF_ONLY,
    '\0', "cnf",
    NoArg, NULL,
    "Convert the input problem into clause normal form and print it."
    " This is (nearly) equivalent to '--print-saturated=eigEIG"
    " --processed-clauses-limit=0' and will by default perform some "
    "usually useful simplifications. You can additionally specify e.g. "
    "'--no-preprocessing' if you want just the result of CNF translation."},

   {OPT_PRINT_PID,
    '\0', "print-pid",
    NoArg, NULL,
    "Print the process id of the prover as a comment after option "
    "processing."},

   {OPT_PRINT_VERSION,
    '\0', "print-version",
    NoArg, NULL,
    "Print the version number of the prover as a comment after option "
    "processing. Note that unlike -version, the prover will not "
    "terminate, but proceed normally."},

   {OPT_REQUIRE_NONEMPTY,
    '\0', "error-on-empty",
    NoArg, NULL,
    "Return with an error code if the input file contains no clauses. "
    "Formally, the empty clause set (as an empty conjunction of "
    "clauses) is trivially satisfiable, and E will treat any empty "
    "input set as satisfiable. However, in composite systems this "
    "is more often a sign that something went wrong. Use this option "
    "to catch such bugs."},

   {OPT_MEM_LIMIT,
    'm', "memory-limit",
    ReqArg, NULL,
    "Limit the memory the prover may use. The argument is "
    "the allowed amount of memory in MB. If you use the argument 'Auto',"
    " the system will try to figure out the amount of physical memory of "
    "your machine and claim most of it. This option may not work "
    "everywhere, due to broken and/or strange behaviour of setrlimit() "
    "in some UNIX implementations, and due to the fact that I know "
    "of no portable way to figure out the physical memory in a machine. "
    "Both the option and the 'Auto' version do work under all tested "
    "versions of Solaris and GNU/Linux. Due to problems with limit "
    "data types, it is currently impossible to set a limit of more than "
    "2 GB (2048 MB)."},

   {OPT_CPU_LIMIT,
    '\0', "cpu-limit",
    OptArg, "300",
    "Limit the cpu time the prover should run. The optional argument "
    "is the CPU time in seconds. The prover will terminate immediately"
    " after reaching the time limit, regardless of internal state. This"
    " option may not work " 
    "everywhere, due to broken and/or strange behaviour of setrlimit() "
    "in some UNIX implementations. It does work under all tested "
    "versions of Solaris, HP-UX, MacOS-X, and GNU/Linux. As a side "
    "effect, this "
    "option will inhibit core file writing. Please note that if you"
    " use both --cpu-limit and --soft-cpu-limit, the soft limit has to"
    " be smaller than the hard limit to have any effect. "}, 

   {OPT_SOFTCPU_LIMIT,
    '\0', "soft-cpu-limit",
    OptArg, "290",
    "Limit the cpu time the prover should spend in the main saturation"
    " phase. The prover will then terminate gracefully, i.e. it will "
    "perform post-processing, " 
    "filtering and printing of unprocessed clauses, if these options"
    " are selected. Note"
    " that for some filtering options (in particular those which "
    "perform full subsumption), the post-processing time may well be"
    " larger than the saturation time. This option is particularly "
    "useful if you want to use E as a preprocessor or lemma generator "
    "in a larger system."},  

   {OPT_RUSAGE_INFO,
    'R', "resources-info",
    NoArg, NULL,
    "Give some information about the resources used by the prover. "
    "You will usually get CPU time information. On systems returning "
    "more information with the rusage() system call, you will also "
    "get information about memory consumption."},

   {OPT_STEP_LIMIT,
    'C', "processed-clauses-limit",
    ReqArg, NULL,
    "Set the maximal number of clauses to process (i.e. the number"
    " of traversals of the main-loop)."},
   
   {OPT_ANSWER_LIMIT,
    '\0', "answers",
    OptArg, "2147483647",
    "Set the maximal number of answers to print for existentially"
    " quantified questions. Without this option, the prover terminates"
    " after the first answer found. If the value is different from 1, "
    "the prover is no longer guaranteed to terminate, even if there is"
    " a finite number of answers."},

   {OPT_CONJ_ARE_QUEST,
    '\0', "conjectures-are-questions",
    NoArg, NULL,
    "Treat all conjectures as questions to be answered. This is a wart "
    "necessary because CASC-J6 has categories requiring answers, but "
    "does not yet support the 'question' type for formulas."},
   
   {OPT_PROC_LIMIT,
    'P', "processed-set-limit",
    ReqArg, NULL,
    "Set the maximal size of the set of processed clauses. This differs "
    "from the previous option in that redundant and back-simplified "
    "processed clauses are not counted."},

   {OPT_UNPROC_LIMIT,
    'U', "unprocessed-limit",
    ReqArg, NULL,
    "Set the maximal size of the set of unprocessed clauses. This is a "
    "termination condition, not something to use to control the deletion"
    " of bad clauses. Compare --delete-bad-limit."},
   
   {OPT_TOTAL_LIMIT,
    'T', "total-clause-set-limit",
    ReqArg, NULL,
    "Set the maximal size of the set of all clauses. See previous option."},

   {OPT_NO_INFIX,
    'n', "eqn-no-infix",
    NoArg, NULL,
    "In LOP, print equations in prefix notation equal(x,y)."},

   {OPT_FULL_EQ_REP,
    'e', "full-equational-rep",
    NoArg, NULL,
    "In LOP. print all literals as equations, even non-equational ones."},

   {OPT_TPTP_PARSE,
    '\0', "tptp-in",
    NoArg, NULL,
    "Parse TPTP-2 format instead of E-LOP (but note that includes are "
    "handled according to TPTP-3 semantics)."},

   {OPT_TPTP_PRINT,
    '\0', "tptp-out",
    NoArg, NULL,
    "Print TPTP format instead of E-LOP. Implies --eqn-no-infix and "
    "will ignore --full-equational-rep."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp-format",
    NoArg, NULL,
    "Equivalent to --tptp-in and --tptp-out."},

   {OPT_TPTP_PARSE,
    '\0', "tptp2-in",
    NoArg, NULL,
    "Synonymous with --tptp-in."},

   {OPT_TPTP_PRINT,
    '\0', "tptp2-out",
    NoArg, NULL,
    "Synonymous with --tptp-out."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp2-format",
    NoArg, NULL,
    "Synonymous with --tptp-format."},

   {OPT_TSTP_PARSE,
    '\0', "tstp-in",
    NoArg, NULL,
    "Parse TPTP-3 format instead of E-LOP (Note that TPTP-3 syntax "
    "is still under development, and the version in E may not be "
    "fully conforming at all times. E works on all TPTP 3.0.1 input "
    "files (including includes)."},
   
   {OPT_TSTP_PRINT,
    '\0', "tstp-out",
    NoArg, NULL,
    "Print output clauses in TPTP-3 syntax. In particular, for output "
    "levels >=2, write derivations as TPTP-3 derivations (default is PCL)."},

   {OPT_TSTP_FORMAT,
    '\0', "tstp-format",
    NoArg, NULL,
    "Equivalent to --tstp-in and --tstp-out."},

   {OPT_TSTP_PARSE,
    '\0', "tptp3-in",
    NoArg, NULL,
    "Synonymous with --tstp-in."},

   {OPT_TSTP_PRINT,
    '\0', "tptp3-out",
    NoArg, NULL,
    "Synonymous with --tstp-out."},

   {OPT_TSTP_FORMAT,
    '\0', "tptp3-format",
    NoArg, NULL,
    "Synonymous with --tstp-format."},

   {OPT_AUTO,
    '\0', "auto", 
    NoArg, NULL,
    "Automatically determine settings for proof search. This is "
    "equivalent to -xAuto -tAuto --sine=Auto."},

   {OPT_SATAUTO,
    '\0', "satauto", 
    NoArg, NULL,
    "Automatically determine settings for proof/saturation search. This is " 
    "equivalent to -xAuto -tAuto."},

   {OPT_AUTODEV,
    '\0', "autodev", 
    NoArg, NULL,
    "Automatically determine settings for proof search (development "
    "version). This is equivalent to -xAutoDev -tAutoDev --sine=Auto."},

   {OPT_SATAUTODEV,
    '\0', "satautodev", 
    NoArg, NULL,
    "Automatically determine settings for proof/saturation search "
    "(development version). This is equivalent to -xAutoDev -tAutoDev."},

   {OPT_AUTO_SCHED,
    '\0', "auto-schedule",
    NoArg, NULL,
    "Use the (experimental) strategy scheduling. This will try several "
    "different fully specified search strategies (aka \"Auto-Modes\"), "
    "one after the other, until a proof or saturation is found, or the "
    "time limit is exceeded."},

   {OPT_NO_PREPROCESSING,
    '\0', "no-preprocessing",
    NoArg, NULL,
    "Do not perform preprocessing on the initial clause set. "
    "Preprocessing currently removes tautologies and orders terms, "
    "literals and clauses in a certain (\"canonical\") way before "
    "anything else happens. Unless limited by one of the following "
    "options, it will also unfold equational definitions."},

   {OPT_EQ_UNFOLD_LIMIT,
    '\0', "eq-unfold-limit",
    ReqArg, NULL,
    "During preprocessing, limit unfolding (and removing) of "
    "equational definitions to those where the expanded definition "
    "is at most the given limit bigger (in terms of standard "
    "weight) than the defined term."},
   
   {OPT_EQ_UNFOLD_MAXCLAUSES,
    '\0', "eq-unfold-maxclauses",
    ReqArg, NULL,
    "During preprocessing, don't try unfolding of equational "
    "definitions if the problem has more than this limit of clauses."},

   {OPT_NO_EQ_UNFOLD,
    '\0', "no-eq-unfolding",
    NoArg, NULL,
    "During preprocessing, abstain from unfolding (and removing) "
    "equational definitions."},

   {OPT_SINE,
    '\0', "sine",
    OptArg, "Auto",
    "Apply SInE to prune the unprocessed axioms with the specified"
    " filter. 'Auto' will automatically pick a filter."},

   {OPT_REL_PRUNE_LEVEL,
    '\0', "rel-pruning-level",
    OptArg, "3",
    "Perform relevancy pruning up to the given level on the"
    " unprocessed axioms."},

   {OPT_PRESAT_SIMPLIY,
    '\0', "presat-simplify",
    NoArg, NULL,
    "Before proper saturation do a complete interreduction of "
    "the proof state."},

   {OPT_AC_HANDLING,
    '\0', "ac-handling",
    OptArg, "KeepUnits",
    "Select AC handling mode. Preselected is 'DiscardAll', other "
    "options are 'None' to disable AC handling, 'KeepUnits', and "
    "'KeepOrientable'."}, 

   {OPT_AC_ON_PROC,
    '\0', "ac-non-aggressive",
    NoArg, NULL,
    "Do AC resolution on negative literals only on processing (by "
    "default, AC resolution is done after clause creation). Only "
    "effective if AC handling is not disabled."},

   {OPT_LITERAL_SELECT,
    'W', "literal-selection-strategy",
    ReqArg, NULL,
    "Choose a strategy for selection of negative literals. There are "
    "two special values for this option: NoSelection will select no "
    "literal (i.e. perform normal superposition) and "
    "NoGeneration will inhibit all generating inferences. For a list "
    "of the other (hopefully self-documenting) values run "
    "'" NAME " -W none'. There are two variants of each strategy. The "
    "one prefixed with 'P' will allow paramodulation into maximal "
    "positive literals in addition to paramodulation into maximal "
    "selected negative literals."},	

   {OPT_NO_GENERATION,
    '\0', "no-generation",
    NoArg, NULL,
    "Don't perform any generating inferences (equivalent to "
    "--literal-selection-strategy=NoGeneration)."},

   {OPT_SELECT_ON_PROC_ONLY,
    '\0', "select-on-processing-only",
    NoArg, NULL,
    "Perform literal selection at processing time only (i.e. select "
    "only in the _given clause_), not before clause evaluation. This"
    " is relevant because many clause selection heuristics give "
    "special consideration to maximal or selected literals."},

   {OPT_INHERIT_PM_LIT,
    'i', "inherit-paramod-literals",
    NoArg, NULL,
    "Always select the negative literals a previous inference "
    "paramodulated into (if possible). If no such literal exists,"
    " select as dictated by the selection strategy."}, 

   {OPT_INHERIT_GOAL_PM_LIT,
    'j', "inherit-goal-pm-literals",
    NoArg, NULL,
    "In a goal (all negative clause), always select the negative "
    "literals a previous inference " 
    "paramodulated into (if possible). If no such literal exists,"
    " select as dictated by the selection strategy."}, 

   {OPT_INHERIT_CONJ_PM_LIT,
    'j', "inherit-conjecture-pm-literals",
    NoArg, NULL,
    "In a conjecture-derived clause), always select the negative "
    "literals a previous inference paramodulated into (if possible). "
    "If no such literal exists, select as dictated by the selection "
    "strategy."},

   {OPT_POS_LITSEL_MIN,
    '\0', "selection-pos-min",
    ReqArg, NULL,
    "Set a lower limit for the number of positive literals a clause must"
    " have to be eligible for literal selection."},

   {OPT_POS_LITSEL_MAX,
    '\0', "selection-pos-max",
    ReqArg, NULL,
    "Set a upper limit for the number of positive literals a clause can"
    " have to be eligible for literal selection."},

   {OPT_NEG_LITSEL_MIN,
    '\0', "selection-neg-min",
    ReqArg, NULL,
    "Set a lower limit for the number of negative literals a clause must"
    " have to be eligible for literal selection."},

   {OPT_NEG_LITSEL_MAX,
    '\0', "selection-neg-max",
    ReqArg, NULL,
    "Set a upper limit for the number of negative literals a clause can"
    " have to be eligible for literal selection."},

   {OPT_ALL_LITSEL_MIN,
    '\0', "selection-all-min",
    ReqArg, NULL,
    "Set a lower limit for the number of literals a clause must"
    " have to be eligible for literal selection."},

   {OPT_ALL_LITSEL_MAX,
    '\0', "selection-all-max",
    ReqArg, NULL,
    "Set an upper limit for the number of literals a clause must"
    " have to be eligible for literal selection."},

   {OPT_WEIGHT_LITSEL_MIN,

    '\0', "selection-weight-min",
    ReqArg, NULL,
    "Set the minimum weight a clause must have to be eligible for"
    " literal selection."},

   {OPT_PREFER_INITIAL_CLAUSES,
    '\0', "prefer-initial-clauses",
    NoArg, NULL,
    "Always process all initial clauses first."},

   {OPT_HEURISTIC,
    'x', "expert-heuristic",
    ReqArg, NULL,
    "Select one of the clause selection heuristics. Currently"
    " at least available: Auto, Weight, StandardWeight, RWeight, FIFO,"
    " LIFO, Uniq, UseWatchlist. For a full list check "
    "HEURISTICS/che_proofcontrol.c. Auto is recommended if you only want"
    " to find a proof. It is special in that it will also set some "
    "additional options. To have optimal "
    "performance, you also should specify -tAuto to select a good "
    "term ordering. LIFO is unfair and will " 
    "make the prover incomplete. Uniq is used internally and is not "
    "very useful in most cases. You can define more heuristics using"
    " the option -H (see below)."},

   {OPT_FILTER_LIMIT,
    '\0', "filter-limit",
    OptArg, "1000000",
    "Set the limit on the number of 'storage units' in the proof "
    "state, after which the set of unprocessed clauses will be "
    "filtered against the processed clauses to eliminate redundant "
    "clauses. As of E 0.7, a 'storage unit' is approximately one byte, "
    "however, storage is estimated in an abstract way, independent of "
    "hardware or memory allocation library, and the storage estimate "
    "is only an approximation."},

   {OPT_FILTER_COPIES_LIMIT,
    '\0', "filter-copies-limit",
    OptArg, "800000",
    "Set the number of storage units in new unprocessed clauses after"
    " which the set of unprocessed clauses will be filtered for"
    " equivalent copies of clauses (see above). As this operation is"
    " cheaper, you may want to set this limit lower than"
    " --filter-limit."},

   {OPT_DELETE_BAD_LIMIT,
    '\0', "delete-bad-limit",
    OptArg, "1500000",
    "Set the number of storage units after which bad clauses are"
    " deleted without further consideration. This causes the prover to"
    " be potentially incomplete, but will allow you to limit the"
    " maximum amount of memory used fairly well. The prover will tell"
    " you if a proof attempt failed due to the incompleteness"
    " introduced by this option. It is recommended to set this limit"
    " significantly higher than --filter-limit or"
    " --filter-copies-limit. If you select -xAuto and set a"
    " memory limit, the prover will determine a good value"
    " automatically."}, 

   {OPT_ASSUME_COMPLETENESS,
    '\0', "assume-completeness",
    NoArg, NULL,
    "There are various way (e.g. the next few options) to configure the "
    "prover to be strongly incomplete in the general case. E will detect"
    " when such an option is selected and return corresponding exit "
    "states (i.e. it will not claim satisfiability just because it ran"
    " out of unprocessed clauses). If you _know_ that for your class "
    "of problems the selected strategy is still complete, use this "
    "option to tell the system that this is the case."},

   {OPT_ASSUME_INCOMPLETENESS,
    '\0', "assume-incompleteness",
    NoArg, NULL,
    "This option instructs the prover to assume incompleteness (typically"
    " because the axiomatization already is incomplete because axioms"
    " have been filtered before they are handed to the system."},

   {OPT_DISABLE_EQ_FACTORING,
    '\0', "disable-eq-factoring",
    NoArg, NULL,
    "Disable equality factoring. This makes the prover incomplete for "
    "general non-Horn problems, but helps for some specialized classes."
    " It is not necessary to disable equality factoring for Horn problems"
    ", as Horn clauses are not factored anyways."}, 
   
   {OPT_DISABLE_NEGUNIT_PM,
    '\0', "disable-paramod-into-neg-units",
    NoArg, NULL,
    "Disable paramodulation into negative unit clause. This makes the"
    " prover incomplete in the general case, but helps for some "
    "specialized classes."}, 

   {OPT_CONDENSING,
    '\0', "condense",
    NoArg, NULL,
    "Enable condensing for the given clause. Condensing replaces a clause "
    "by a more general factor (if such a factor exists)."}, 

   {OPT_CONDENSING_AGGRESSIVE,
    '\0', "condense-aggressive",
    NoArg, NULL,
    "Enable condensing for the given and newly generated clauses."}, 

   {OPT_NO_GC_FORWARD_SIMPL,
    '\0', "disable-given-clause-fw-contraction",
    NoArg, NULL,
    "Disable simplification and subsumption of the newly selected "
    "given clause (clauses are still simplified when they are "
    "generated). In general," 
    " this breaks some basic assumptions of the DISCOUNT loop proof search"
    " procedure. However, there are some problem classes in which "
    " this simplifications empirically never occurs. In such cases, we "
    "can save significant overhead. The option _should_ work in all "
    "cases, but is not expected to improve things in most cases."},

   {OPT_USE_SIM_PARAMOD,
    '\0', "simul-paramod",
    NoArg, NULL,
    "Use simultaneous paramodulation to implement superposition. Default"
    " is to use plain paramodulation. This is an experimental feature."},

   {OPT_USE_ORIENTED_SIM_PARAMOD,
    '\0', "oriented-simul-paramod",
    NoArg, NULL,
    "Use simultaneous paramodulation for oriented from-literals. This "
    "is an experimental feature."}, 

   {OPT_SPLIT_TYPES,
    '\0', "split-clauses",
    OptArg, "7",
    "Determine which clauses should be subject to splitting. The "
    "argument is the binary 'OR' of values for the desired classes:\n"
    "     1:  Horn clauses\n"
    "     2:  Non-Horn clauses\n"
    "     4:  Negative clauses\n"
    "     8:  Positive clauses\n"
    "    16:  Clauses with both positive and negative literals\n"
    "Each set bit adds that class to the set of clauses which will be "
    "split."},

   {OPT_SPLIT_HOW,
    '\0' , "split-method",
    ReqArg, NULL,
    "Determine how to treat ground literals in splitting. The argument"
    " is either '0' to denote no splitting of ground literals (they are"
    " all assigned to the first split clause produced), '1' to denote"
    " that all ground literals should form a single new clause, or '2',"
    " in which case ground literals are treated as usual and are all"
    " split off into individual clauses."},
   
   {OPT_SPLIT_AGGRESSIVE,
    '\0', "split-aggressive",
    NoArg, NULL,
    "Apply splitting to new clauses (after simplification) and before "
    "evaluation. By default, splitting (if activated) is only "
    "performed on selected clauses. "},

   {OPT_SPLIT_REUSE_DEFS,
    '\0', "split-reuse-defs",
    NoArg, NULL,
    "If possible, reuse previous definitions for splitting."},

   {OPT_REWEIGHT_LIMIT,
    '\0', "reweight-limit",
    OptArg, "30000",
    "Set the number of new unprocessed clauses after which the set of"
    " unprocessed clauses will be reevaluated."},

   {OPT_ORDERING,
    't', "term-ordering",
    ReqArg, NULL,
    "Select an ordering type (currently Auto, LPO, LPO4, KBO or "
    "KBO6). -tAuto is suggested, in particular with -xAuto. KBO and"
    " KBO1 are different implementations of the same ordering, KBO is"
    " usually faster and has had more testing. Similarly, LPO4 is an "
    "new, equivalent but superior implementation of LPO."},

   {OPT_TO_WEIGHTGEN,
    'w', "order-weight-generation",
    ReqArg, NULL,
    "Select a method for the generation of weights for use with the "
    "term ordering. Run '" NAME " -w none' for a list of options."},

   {OPT_TO_WEIGHTS,
    '\0', "order-weights",
    ReqArg, NULL,
    "Describe a (partial) assignments of weights to function symbols for "
    "term orderings (in particular, KBO). You can specify a list of weights"
    " of the form 'f1:w1,f2:w2, ...'. Since a total weight assignment is"
    " needed, E will _first_ apply any weight generation scheme specified"
    " (or the default one), and then "
    "modify the weights as specified. Note that E performs only very "
    "basic sanity checks, so you probably can specify weights that break"
    " KBO constraints."
   },

   {OPT_TO_PRECGEN,
    'G', "order-precedence-generation",
    ReqArg, NULL,
    "Select a method for the generation of a precedence for use with "
    "the term ordering. Run '" NAME " -G none' for a list of "
    "options."}, 

   {OPT_TO_CONSTWEIGHT,
    'c', "order-constant-weight", 
    ReqArg, NULL,
    "Set a special weight > 0 for constants in the term ordering. "
    "By default, constants are treated like other function symbols."
   },

   {OPT_TO_PRECEDENCE,
    '\0', "precedence",
    OptArg, "",
    "Describe a (partial) precedence for the term ordering used for "
    "the proof attempt. You can specify a comma-separated list of "
    "precedence chains, where a precedence chain is a list of "
    "function symbols (which all have to appear in the proof problem)"
    ", connected by >, <, or =. If this option is used in connection "
    "with --order-precedence-generation, the partial ordering will be "
    "completed using the selected method, otherwise the prover runs "
    "with a non-ground-total ordering."
   },

   {OPT_TO_LPO_RECLIMIT,
    '\0', "lpo-recursion-limit",
    OptArg, "100",
    "Set a depth limit for LPO comparisons. Most comparisons do not "
    "need more than 10 or 20 levels of recursion. By default, "
    "recursion depth is limited to 1000 to avoid stack overflow "
    "problems. If the limit is reached, the prover assumes that the "
    "terms are uncomparable. Smaller values make the comparison "
    "attempts faster, but less exact. Larger values have the opposite "
    "effect. Values up to 20000 should be save on most operating "
    "systems. If you run into segmentation faults while using "
    "LPO or LPO4, first try to set this limit to a reasonable value. "
    "If the problem persists, send a bug report ;-)"
   },

   {OPT_TO_RESTRICT_LIT_CMPS,
    '\0', "restrict-literal-comparisons",
    NoArg, NULL,
    "Make all literals uncomparable in the term ordering (i.e. do not"
    " use the term ordering to restrict paramodulation, equality"
    " resolution and factoring to certain literals. This is necessary"
    " to make Set-of-Support-strategies complete for the non-equational"
    " case (It still is incomplete for the equational case, but pretty"
    " useless anyways)."},

   {OPT_TPTP_SOS,
    '\0', "sos-uses-input-types",
    NoArg, NULL,
    "If input is TPTP format, use TPTP conjectures for initializing "
    "the Set of Support. If not in TPTP format, use E-LOP queries "
    "(clauses of the form ?-l(X),...,m(Y)). Normally, all negative "
    "clauses are used. Please note that " 
    "most E heuristics do not use this information at all, it is currently "
    "only useful for certain parameter settings (including the SimulateSOS "
    "priority function)."},

   {OPT_ER_DESTRUCTIVE,
     '\0', "destructive-er",
    NoArg, NULL,
    "Allow destructive equality resolution inferences on pure-variable "
    "literals of the form X!=Y, i.e. replace the original clause with the "
    "result of an equality resolution inference on this literal."}, 

   {OPT_ER_STRONG_DESTRUCTIVE,
     '\0', "strong-destructive-er",
    NoArg, NULL,
    "Allow destructive equality resolution inferences on "
    "literals of the form X!=t (where X does not occur in t), i.e. "
    "replace the original clause with the " 
    "result of an equality resolution inference on this literal. Unless I "
    "am brain-dead, this maintains completeness, although the proof is"
    " rather tricky."}, 

   {OPT_ER_AGGRESSIVE,
    '\0', "destructive-er-aggressive",
    NoArg, NULL,
    "Apply destructive equality resolution to all newly generated "
    "clauses, not just to selected clauses. Implies --destructive-er."},

   {OPT_FORWARD_CSR,
    '\0', "forward-context-sr",
    NoArg, NULL,
    "Apply contextual simplify-reflect with processed clauses "
    "to the given clause."},

   {OPT_FORWARD_CSR_AGGRESSIVE,
    '\0', "forward-context-sr-aggressive",
    NoArg, NULL,
    "Apply contextual simplify-reflect with processed clauses "
    "to new clauses. Implies --forward-context-sr."},
   
   {OPT_BACKWARD_CSR,
    '\0', "backward-context-sr",
    NoArg, NULL,
    "Apply contextual simplify-reflect with the given clause to "
    "processed clauses."},

   {OPT_RULES_GENERAL,
    'g', "prefer-general-demodulators",
    NoArg, NULL,
    "Prefer general demodulators. By default, E prefers specialized"
    " demodulators. This affects in which order the rewrite "
    " index is traversed."}, 
   
   {OPT_FORWARD_DEMOD,
    'F', "forward_demod_level",
    ReqArg, NULL,
    "Set the desired level for rewriting of unprocessed clauses. A "
    "value of 0 means no rewriting, 1 indicates to use rules "
    "(orientable equations) only, 2 indicates full rewriting with "
    "rules and instances of unorientable equations. Default behavior is 2."},

   {OPT_STRONG_RHS_INSTANCE,
    '\0', "strong-rw-inst",
    NoArg, NULL,
    "Instantiate unbound variables in matching potential "
    "demodulators with a small constant terms."},

   {OPT_STRONGSUBSUMPTION,
    'u', "strong-forward-subsumption",
    NoArg, NULL,
    "Try multiple positions and unit-equations to try to "
    "equationally subsume a "
    "single new clause. Default is to search for a single position."},
   
   {OPT_WATCHLIST,
    '\0', "watchlist",
    OptArg, WATCHLIST_INLINE_QSTRING,
    "Give the name for a file containing clauses to be watched "
    "for during the saturation process. If a clause is generated that "
    "subsumes a watchlist clause, the subsumed clause is removed from "
    "the watchlist. The prover will terminate when the watchlist is"
    " empty. If you want to use the watchlist for guiding the proof,"
    " put the empty clause onto the list and use the built-in clause "
    "selection heuristic "
    "'UseWatchlist' (or build a heuristic yourself using the priority"
    " functions 'PreferWatchlist' and 'DeferWatchlist')."
    " Use the argument " WATCHLIST_INLINE_QSTRING " (or no argument)"
    " and the special clause type "
    "'watchlist' if you want to put watchlist clauses into the normal input"
    " stream. This is only supported for TPTP input formats."}, 
   
   {OPT_WATCHLIST_NO_SIMPLIFY,
    '\0', "no-watchlist-simplification",
    NoArg, NULL,
    "Normally, that watchlist is brought into normal form with respect "
    "to the current processed clause set and certain simplifications."
    " This option disables this behaviour."},
   
   {OPT_NO_INDEXED_SUBSUMPTION,
    '\0', "conventional-subsumption",
    NoArg, NULL,
    "Equivalent to --subsumption-indexing=None."},

   {OPT_FVINDEX_STYLE,
    '\0', "subsumption-indexing",
    ReqArg, NULL,
    "Determine choice of indexing for (most) subsumption operations. "
    "Choices are 'None' for naive subsumption, 'Direct' for direct mapped"
    " FV-Indexing, 'Perm' for permuted FV-Indexing and 'PermOpt' for "
    "permuted FV-Indexing with deletion of (suspected) non-informative "
    "features. Default behaviour is 'Perm'."},

   {OPT_FVINDEX_FEATURETYPES,
    '\0', "fvindex-featuretypes",
    ReqArg, NULL,
    "Select the feature types used for indexing. Choices are \"None\""
    " to disable FV-indexing, \"AC\" for AC compatible features (the default) "
    "(literal number and symbol counts), \"SS\" for set subsumption "
    "compatible features (symbol depth), and \"All\" for all features."
    "Unless you want to measure the effects of the "
    "different features, I suggest you stick with the default."},

   {OPT_FVINDEX_MAXFEATURES,
    '\0', "fvindex-maxfeatures",
    OptArg, "200",
    "Set the maximum initial number of symbols for feature computation. Depending"
    " on the feature selection, a value of X here will convert into 2X+2 "
    "features (for set subsumption features), 2X+4 features (for "
    "AC-compatible features) or 4X+6 features (if all features are used, "
    "the default). Note that the actually used set of features may be "
    "smaller than this if the signature does not contain enough symbols."
    "For the Perm and PermOpt version, this is _also_ used to set the"
    " maximum depth of the feature vector index. Yes, I should "
    "probably make this into two separate options."
    " If you select a small value here, you should probably not use"
    " \"Direct\" for the --subsumption-indexing option."},

   {OPT_FVINDEX_SLACK,
    '\0', "fvindex-slack",
    OptArg, "0",
    "Set the number of slots reserved in the index for function symbols "
    "that may be introduced into the signature "
    "later, e.g. by splitting. If no new symbols are introduced, this just "
    "wastes time and memory. If PermOpt is chosen, the slackness slots will "
    "be deleted from the index anyways, but will still waste "
    "(a little) time in computing feature vectors."},

   {OPT_RW_BW_INDEX,
    '\0', "rw-bw-index",
    OptArg, "FP7",
    "Select fingerprint function for backwards rewrite index. "
    "\"NoIndex\" will disable paramodulation indexing. For a list "
    "of the other values run '" NAME " --pm-index=none'. FPX functions"
    "will use a fingerprint of X positions, the letters disambiguate "
    "between different fingerprints with the same sample size."},

   {OPT_PM_FROM_INDEX,
    '\0', "pm-from-index",
    OptArg, "FP7",
    "Select fingerprint function for the index for paramodulation from "
    "indexed clauses. \"NoIndex\" "
    "will disable paramodulation indexing. For a list "
    "of the other values run '" NAME " --pm-index=none'. FPX functions"
    "will use a fingerprint of X positions, the letters disambiguate "
    "between different fingerprints with the same sample size."},

   {OPT_PM_INTO_INDEX,
    '\0', "pm-into-index",
    OptArg, "FP7",
    "Select fingerprint function for the index for paramodulation into "
    "the indexed clauses. \"NoIndex\" "
    "will disable paramodulation indexing. For a list "
    "of the other values run '" NAME " --pm-index=none'. FPX functions"
    "will use a fingerprint of X positions, the letters disambiguate "
    "between different fingerprints with the same sample size."},

   {OPT_FP_INDEX,
    '\0', "fp-index",
    OptArg, "FP7",
    "Select fingerprint function for all fingerprint indices. See above."},  

   {OPT_DETSORT_RW,
    '\0', "detsort-rw",
    NoArg, NULL,
    "Sort set of clauses eliminated by backward rewriting using a total "
    "syntactic ordering."},
 
   {OPT_DETSORT_NEW,
    '\0', "detsort-new",
    NoArg, NULL,
    "Sort set of newly generated and backward simplified clauses using "
    "a total syntactic ordering."},

   {OPT_DEFINE_WFUN,
    'D', "define-weight-function",
    ReqArg, NULL,
    "Define  a weight function (see manual for details). Later"
    " definitions override previous definitions."}, 

   {OPT_DEFINE_HEURISTIC,
    'H', "define-heuristic",
    ReqArg, NULL,
    "Define a clause selection heuristic (see manual for"
    " details). Later definitions override previous definitions."}, 

   {OPT_FREE_NUMBERS,
    '\0', "free-numbers",
     NoArg, NULL,
     "Treat numbers (strings of decimal digits) as normal free function "
    "symbols in the input. By default, number now are supposed to denote"
    " domain constants and to be implicitly different from each other."}, 
   
   {OPT_FREE_OBJECTS,
    '\0', "free-objects",
     NoArg, NULL,
     "Treat object identifiers (strings in double quotes) as normal "
    "free function symbols in the input. By default, object identifiers "
    "now represent domain objects and are implicitly different from "
    "each other (and from numbers, unless those are declared to be free)."}, 

   {OPT_DEF_CNF,
    '\0', "definitional-cnf",
    OptArg, TFORM_RENAME_LIMIT_STR,
    "Tune the clausification algorithm to introduces definitions for "
    "subformulae to avoid exponential blow-up. The optional argument "
    "is a fudge factor that determines when definitions are introduced. "
    "0 disables definitions completely. The default works well."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char              *outname = NULL;
char              *watchlist_filename = NULL;
HeuristicParms_p  h_parms;
FVIndexParms_p    fvi_parms;
bool              print_sat = false,
                  print_statistics = false,
                  filter_sat = false,
                  print_rusage = false,
                  print_pid = false,
                  print_version = false,
                  outinfo = false,
                  error_on_empty = false,
                  no_preproc = false,
                  no_eq_unfold = false,
                  pcl_full_terms = true,
                  indexed_subsumption = true,
                  prune_only = false,
                  cnf_only = false,
                  inf_sys_complete = true,
                  assume_inf_sys_complete = false,
                  incomplete = false,
                  conjectures_are_questions = false,
                  strategy_scheduling = false;

IOFormat          parse_format = LOPFormat;
long              step_limit = LONG_MAX, 
                  answer_limit = 1,   
                  proc_limit = LONG_MAX,
                  unproc_limit = LONG_MAX, 
                  total_limit = LONG_MAX,
                  eqdef_maxclauses = DEFAULT_EQDEF_MAXCLAUSES,
                  relevance_prune_level = 0;
int               eqdef_incrlimit = DEFAULT_EQDEF_INCRLIMIT;
char              *outdesc = DEFAULT_OUTPUT_DESCRIPTOR,
                  *filterdesc = DEFAULT_FILTER_DESCRIPTOR;
PStack_p          wfcb_definitions, hcb_definitions;
char              *sine=NULL;

FunctionProperties free_symb_prop = FPIgnoreProps;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[]);
void print_help(FILE* out);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/




/*-----------------------------------------------------------------------
//
// Function: parse_spec()
//
//   Allocate proof state, parse input files into it, and check that
//   requested properties are met. Factored out of main for reasons of
//   readability and length.
//
// Global Variables: -
//
// Side Effects    : Memory, input, may terminate with error.
//
/----------------------------------------------------------------------*/

ProofState_p parse_spec(CLState_p state, 
                        IOFormat parse_format,
                        bool error_on_empty,
                        FunctionProperties free_symb_prop,
                        long* ax_no)
{   
   ProofState_p proofstate;
   Scanner_p in;
   int i;
   StrTree_p skip_includes = NULL;
   long parsed_ax_no;

   proofstate = ProofStateAlloc(free_symb_prop);
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], true, NULL);
      ScannerSetFormat(in, parse_format);
      
      FormulaAndClauseSetParse(in, proofstate->axioms, 
                               proofstate->f_axioms,
                               proofstate->original_terms, 
                               NULL,
                               &skip_includes);
      CheckInpTok(in, NoToken);
      DestroyScanner(in); 
   }
   VERBOUT2("Specification read\n");

   parsed_ax_no = ProofStateAxNo(proofstate);

   if(error_on_empty && (parsed_ax_no == 0))
   {
#ifdef PRINT_SOMEERRORS_STDOUT
      fprintf(GlobalOut, "# Error: Input file contains no clauses or formulas\n");
      TSTPOUT(GlobalOut, "InputError");
#endif
      Error("Input file contains no clauses or formulas", OTHER_ERROR);
   }
   *ax_no = parsed_ax_no;
   return proofstate;
}


/*-----------------------------------------------------------------------
//
// Function: print_info()
//
//   Check if pid and version should be printed, if yes, do so.
//
// Global Variables: print_pid, print_version
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_info(void)
{
   if(print_pid)
   {
      fprintf(GlobalOut, "# Pid: %d\n", (int)getpid());
      fflush(GlobalOut);
   }
   if(print_version)
   {
      fprintf(GlobalOut, "# Version: " VERSION "\n");
      fflush(GlobalOut);
   }   
}


/*-----------------------------------------------------------------------
//
// Function: main()
//
//   Main entry point of the prover.
//
// Global Variables: Plenty, mostly flags shared with
//                   process_options. See list above.
//
// Side Effects    : Yes ;-)
//
/----------------------------------------------------------------------*/


int main(int argc, char* argv[])
{
   int              retval = NO_ERROR;
   CLState_p        state;
   ProofState_p     proofstate;
   ProofControl_p   proofcontrol;
   Clause_p         success = NULL, 
                    filter_success;
   bool             out_of_clauses;
   char             *finals_state = "exists",
                    *sat_status = "Derivation";
   long             raw_clause_no,
                    preproc_removed=0, 
                    neg_conjectures,
                    parsed_ax_no,
                    relevancy_pruned = 0;
   double           preproc_time;
   pid_t            pid = 1;


   assert(argv[0]);

   InitIO(NAME);
#ifdef STACK_SIZE
   IncreaseMaxStackSize(argv, STACK_SIZE);
#endif
   ESignalSetup(SIGXCPU);

   h_parms = HeuristicParmsAlloc();
   fvi_parms = FVIndexParmsAlloc();   
   wfcb_definitions = PStackAlloc();
   hcb_definitions = PStackAlloc();

   state = process_options(argc, argv);
   
   OpenGlobalOut(outname);
   print_info();


   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   proofstate = parse_spec(state, parse_format, 
                           error_on_empty, free_symb_prop,
                           &parsed_ax_no);  


   relevancy_pruned += ProofStateSinE(proofstate, sine);
   relevancy_pruned += ProofStatePreprocess(proofstate, relevance_prune_level);

   if(strategy_scheduling)
   {
      ExecuteSchedule(StratSchedule, h_parms, print_rusage);
   }
   
   FormulaSetDocInital(GlobalOut, OutputLevel, proofstate->f_axioms);
   ClauseSetDocInital(GlobalOut, OutputLevel, proofstate->axioms);

   if(prune_only)
   {
      fprintf(GlobalOut, "\n# Pruning successful!\n");	    
      TSTPOUT(GlobalOut, "Unknown");
      goto cleanup1;
   }

   if(relevancy_pruned || incomplete)
   {
      proofstate->state_is_complete = false;
   }
   if(BuildProofObject)
   {
      FormulaSetArchive(proofstate->f_axioms, proofstate->f_ax_archive);
   }   
   if((neg_conjectures =
       FormulaSetPreprocConjectures(proofstate->f_axioms, 
                                    proofstate->f_ax_archive,
                                    answer_limit>0, 
                                    conjectures_are_questions)))
   {
      VERBOUT("Negated conjectures.\n");
   }
   if(FormulaSetCNF(proofstate->f_axioms,
                    proofstate->f_ax_archive,
                    proofstate->axioms, 
                    proofstate->original_terms, 
                    proofstate->freshvars,
                    proofstate->gc_original_terms))
   {
      VERBOUT("CNFization done\n");
   }
   ProofStateInitWatchlist(proofstate, watchlist_filename, parse_format);
   raw_clause_no = proofstate->axioms->members;   
   if(!no_preproc)
   {
      if(BuildProofObject)
      {
         ClauseSetArchive(proofstate->ax_archive, proofstate->axioms);
         if(proofstate->watchlist)
         {
            ClauseSetArchive(proofstate->ax_archive, proofstate->watchlist);
         }
      }
      preproc_removed = ClauseSetPreprocess(proofstate->axioms,
					    proofstate->watchlist,
                                            proofstate->archive,
					    proofstate->tmp_terms,
					    eqdef_incrlimit, 
                                            eqdef_maxclauses);
   }

   proofcontrol = ProofControlAlloc();
   ProofControlInit(proofstate, proofcontrol, h_parms, 
                    fvi_parms, wfcb_definitions, hcb_definitions);
   PCLFullTerms = pcl_full_terms; /* Preprocessing always uses full
				     terms, so we set the flag for
				     the main proof search only now! */
   ProofStateInit(proofstate, proofcontrol);

   VERBOUT2("Prover state initialized\n");   
   preproc_time = GetTotalCPUTime();
   if(print_rusage)
   {
      fprintf(GlobalOut, "# Preprocessing time       : %.3f s\n", preproc_time);
   }
   if(proofcontrol->heuristic_parms.presat_interreduction)
   {
      LiteralSelectionFun sel_strat = 
         proofcontrol->heuristic_parms.selection_strategy;

      proofcontrol->heuristic_parms.selection_strategy = SelectNoGeneration;
      success = Saturate(proofstate, proofcontrol, LONG_MAX,
                         LONG_MAX, LONG_MAX, LONG_MAX, LONG_MAX);
      fprintf(GlobalOut, "# Presaturation interreduction done\n");
      proofcontrol->heuristic_parms.selection_strategy = sel_strat;
      if(!success)
      {
         ProofStateResetProcessed(proofstate, proofcontrol);
      }
   }
   PERF_CTR_ENTRY(SatTimer);
   
   if(!success)
   {      
      success = Saturate(proofstate, proofcontrol, step_limit,
                         proc_limit, unproc_limit, total_limit, answer_limit);
   }
   PERF_CTR_EXIT(SatTimer);
   
   out_of_clauses = ClauseSetEmpty(proofstate->unprocessed);
   if(filter_sat)
   {
      filter_success = ProofStateFilterUnprocessed(proofstate,
						   proofcontrol,
						   filterdesc);
      if(filter_success)
      {
	 success = filter_success;
         PStackPushP(proofstate->extract_roots, success);
      }
   }
   
   if(success||proofstate->answer_count)
   {
      assert(!PStackEmpty(proofstate->extract_roots));
      if(success)
      {
         DocClauseQuoteDefault(2, success, "proof");
      }      
      fprintf(GlobalOut, "\n# Proof found!\n");
      if(!proofstate->status_reported)
      {
         TSTPOUT(GlobalOut, neg_conjectures?"Theorem":"Unsatisfiable");
         proofstate->status_reported = true;
         retval = PROOF_FOUND;
      }      
      if(BuildProofObject)
      {
         Derivation_p derivation = 
            DerivationCompute(proofstate->extract_roots, 
                              proofstate->signature);
         DerivationPrint(GlobalOut, derivation, "CNFRefutation");
         DerivationFree(derivation);
      }
   }
   else if(proofstate->watchlist && ClauseSetEmpty(proofstate->watchlist))
   {      
      ProofStatePropDocQuote(GlobalOut, OutputLevel, 
			     CPSubsumesWatch, proofstate, 
			     "final_subsumes_wl");
      fprintf(GlobalOut, "\n# Watchlist is empty!\n");
      TSTPOUT(GlobalOut, "ResourceOut"); 
      retval = RESOURCE_OUT;
   }
   else
   {
      if(out_of_clauses&&
	 proofstate->state_is_complete&&
	 (inf_sys_complete || assume_inf_sys_complete))
      {
	 finals_state = "final";
      }
      ProofStatePropDocQuote(GlobalOut, OutputLevel, CPIgnoreProps,
			     proofstate, finals_state);

      if(cnf_only)
      {
         fprintf(GlobalOut, "\n# CNFization successful!\n");	    
         TSTPOUT(GlobalOut, "Unknown");
      }
      else if(out_of_clauses)
      {
	 if(!(inf_sys_complete || assume_inf_sys_complete))
	 {
	    fprintf(GlobalOut, 
		    "\n# Clause set closed under "
		    "restricted calculus!\n");
	    TSTPOUT(GlobalOut, "GaveUp");
            retval = INCOMPLETE_PROOFSTATE;
	 }
	 else if(proofstate->state_is_complete && inf_sys_complete)
	 {
	    fprintf(GlobalOut, "\n# No proof found!\n");
	    TSTPOUT(GlobalOut, neg_conjectures?"CounterSatisfiable":"Satisfiable");
            sat_status = "Saturation";
            retval = SATISFIABLE;
	 }
	 else
	 {
	    fprintf(GlobalOut, "\n# Failure: Out of unprocessed clauses!\n");	    
	    TSTPOUT(GlobalOut, "GaveUp");	    
            retval = INCOMPLETE_PROOFSTATE;
	 }
      }
      else 
      {
	 fprintf(GlobalOut, "\n# Failure: User resource limit exceeded!\n");
	 TSTPOUT(GlobalOut, "ResourceOut");
         retval = RESOURCE_OUT;
      }
      if(BuildProofObject)
      {
         Derivation_p derivation;

         ClauseSetPushClauses(proofstate->extract_roots, 
                              proofstate->processed_pos_rules);
         ClauseSetPushClauses(proofstate->extract_roots,
                              proofstate->processed_pos_eqns);
         ClauseSetPushClauses(proofstate->extract_roots,
                              proofstate->processed_neg_units);
         ClauseSetPushClauses(proofstate->extract_roots,
                              proofstate->processed_non_units);
         derivation = DerivationCompute(proofstate->extract_roots, 
                                        proofstate->signature);
         DerivationPrint(GlobalOut, derivation, sat_status);
         DerivationFree(derivation);
      }

   }
   /* ClauseSetDerivationStackStatistics(proofstate->unprocessed); */
   if(print_sat)
   {
      if(proofstate->non_redundant_deleted)
      {
	 fprintf(GlobalOut, "\n# Saturated system is incomplete!\n");
      }
      if(success)
      {
	 fprintf(GlobalOut, "# Saturated system contains the empty clause:\n");
	 ClausePrint(GlobalOut, success, true);
	 fputc('\n',GlobalOut);	 
	 fputc('\n',GlobalOut);	 
      }
      ProofStatePrintSelective(GlobalOut, proofstate, outdesc,
			       outinfo);
      fprintf(GlobalOut, "\n");
   }	 
   
   if(success)
   {
      ClauseFree(success);
   }
   fflush(GlobalOut);
   
   if(OutputLevel||print_statistics)
   {
      fprintf(GlobalOut, "# Parsed axioms                        : %ld\n",
              parsed_ax_no);
      fprintf(GlobalOut, "# Removed by relevancy pruning/SinE    : %ld\n",
              relevancy_pruned);
      fprintf(GlobalOut, "# Initial clauses                      : %ld\n",
	      raw_clause_no);
      fprintf(GlobalOut, "# Removed in clause preprocessing      : %ld\n",
	      preproc_removed);
      ProofStateStatisticsPrint(GlobalOut, proofstate);
      fprintf(GlobalOut, "# Clause-clause subsumption calls (NU) : %ld\n",
	      ClauseClauseSubsumptionCalls);
      fprintf(GlobalOut, "# Rec. Clause-clause subsumption calls : %ld\n",
	      ClauseClauseSubsumptionCallsRec);
      fprintf(GlobalOut, "# Non-unit clause-clause subsumptions  : %ld\n",
	      ClauseClauseSubsumptionSuccesses);
      fprintf(GlobalOut, "# Unit Clause-clause subsumption calls : %ld\n",
              UnitClauseClauseSubsumptionCalls);
      fprintf(GlobalOut, "# Rewrite failures with RHS unbound    : %ld\n",
              RewriteUnboundVarFails);
      fprintf(GlobalOut, "# BW rewrite match attempts            : %ld\n",
              BWRWMatchAttempts);
      fprintf(GlobalOut, "# BW rewrite match successes           : %ld\n",
              BWRWMatchSuccesses);
      fprintf(GlobalOut, "# Condensation attempts                : %ld\n",
              CondensationAttempts);
      fprintf(GlobalOut, "# Condensation successes               : %ld\n",
              CondensationSuccesses);

#ifdef MEASURE_UNIFICATION
      fprintf(GlobalOut, "# Unification attempts                 : %ld\n",
              UnifAttempts);
      fprintf(GlobalOut, "# Unification successes                : %ld\n",
              UnifSuccesses);
#endif
      PERF_CTR_PRINT(GlobalOut, MguTimer);
      PERF_CTR_PRINT(GlobalOut, SatTimer);
      PERF_CTR_PRINT(GlobalOut, ParamodTimer);
      PERF_CTR_PRINT(GlobalOut, PMIndexTimer);
      PERF_CTR_PRINT(GlobalOut, IndexUnifTimer);
      PERF_CTR_PRINT(GlobalOut, BWRWTimer);
      PERF_CTR_PRINT(GlobalOut, BWRWIndexTimer);
      PERF_CTR_PRINT(GlobalOut, IndexMatchTimer);
      PERF_CTR_PRINT(GlobalOut, FreqVecTimer);
      PERF_CTR_PRINT(GlobalOut, FVIndexTimer);
      PERF_CTR_PRINT(GlobalOut, SubsumeTimer);
      PERF_CTR_PRINT(GlobalOut, SetSubsumeTimer);

#ifdef PRINT_INDEX_STATS
      fprintf(GlobalOut, "# Backwards rewriting index : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.bw_rw_index);
      fprintf(GlobalOut, "\n");
      /*FPIndexPrintDot(GlobalOut, "rw_bw_index", 
        proofstate->gindices.bw_rw_index,
        SubtermTreePrintDot,
        proofstate->signature);*/
      fprintf(GlobalOut, "# Paramod-from index        : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.pm_from_index);
      fprintf(GlobalOut, "\n");
      FPIndexPrintDot(GlobalOut, "pm_from_index", 
                      proofstate->gindices.pm_from_index,
                      SubtermTreePrintDot,
                      proofstate->signature);
      fprintf(GlobalOut, "# Paramod-into index        : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.pm_into_index);
      fprintf(GlobalOut, "\n");
      fprintf(GlobalOut, "# Paramod-neg-atom index    : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.pm_negp_index);
      fprintf(GlobalOut, "\n");
#endif
   }   
#ifndef FAST_EXIT
#ifdef FULL_MEM_STATS
   fprintf(GlobalOut,
	   "# sizeof TermCell     : %ld\n"
	   "# sizeof EqnCell      : %ld\n"
	   "# sizeof ClauseCell   : %ld\n"
	   "# sizeof PTreeCell    : %ld\n"
	   "# sizeof PDTNodeCell  : %ld\n"
	   "# sizeof EvalCell     : %ld\n"
	   "# sizeof ClausePosCell: %ld\n"
	   "# sizeof PDArrayCell  : %ld\n",
	   sizeof(TermCell),
	   sizeof(EqnCell),
	   sizeof(ClauseCell),
	   sizeof(PTreeCell),
	   sizeof(PDTNodeCell),
	   sizeof(EvalCell),
	   sizeof(ClausePosCell),
	   sizeof(PDArrayCell));	
   fprintf(GlobalOut, "# Estimated memory usage: %ld\n",
	   ProofStateStorage(proofstate));
   MemFreeListPrint(GlobalOut);
#endif
   ProofControlFree(proofcontrol);
#endif
cleanup1:
#ifndef FAST_EXIT
   ProofStateFree(proofstate);
   CLStateFree(state);
   PStackFree(hcb_definitions);
   PStackFree(wfcb_definitions);
   FVIndexParmsFree(fvi_parms);
   HeuristicParmsFree(h_parms);
#ifdef FULL_MEM_STATS
   MemFreeListPrint(GlobalOut);
#endif
#endif
   if(print_rusage && pid)
   {
      PrintRusage(GlobalOut);
   }
   if(!pid)
   {
      return retval;
   }
   fflush(GlobalOut);
   OutClose(GlobalOut);
   ExitIO();
#ifdef CLB_MEMORY_DEBUG
   RegMemCleanUp();
   MemFlushFreeList();
   MemDebugPrintStats(stdout);
#endif
   return retval;
}


/*-----------------------------------------------------------------------
//
// Function: check_fp_index_arg()
//
//   Check in arg is a valid term describing a FP-index function. If
//   yes, return true. If no, print error (nominally return false).
//
// Global Variables: -
//
// Side Effects    : May terminate program with error.
//
/----------------------------------------------------------------------*/

bool check_fp_index_arg(char* arg, char* opt)
{
   DStr_p err;

   if(GetFPIndexFunction(arg)||(strcmp(arg, "NoIndex")==0))
   {
      return true;
   }
   err = DStrAlloc();
   DStrAppendStr(err, 
                 "Wrong argument to option ");
   DStrAppendStr(err, 
                 opt);
   DStrAppendStr(err, 
                 ". Possible values: "); 
   DStrAppendStrArray(err, FPIndexNames, ", ");
   Error(DStrView(err), USAGE_ERROR);
   DStrFree(err);
   
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: process_options()
//
//   Read and process the command line option, return (the pointer to)
//   a CLState object containing the remaining arguments.
//
// Global Variables: opts, Verbose, TBPrintInternalInfo 
//
// Side Effects    : Sets variables, may terminate with program
//                   description if option -h or --help was present
//
/----------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[])
{
   Opt_p handle;
   CLState_p state;
   char*  arg;
   long   tmp;
   long   mem_limit;

   state = CLStateAlloc(argc,argv);
   
   while((handle = CLStateGetOpt(state, &arg, opts)))
   {
      switch(handle->option_code)
      {
      case OPT_VERBOSE:
	    Verbose = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_HELP: 
	    print_help(stdout);
	    exit(NO_ERROR);
	    break;
      case OPT_VERSION:
	    fprintf(stdout, "E " VERSION " " E_NICKNAME "\n");
	    exit(NO_ERROR);
	    break;
      case OPT_OUTPUT:
	    outname = arg;
	    break;
      case OPT_SILENT:
	    OutputLevel = 0;
	    break;
      case OPT_OUTPUTLEVEL:
	    OutputLevel = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_PROOF_OBJECT:
            BuildProofObject =  CLStateGetIntArg(handle, arg);
	    if((BuildProofObject > 3) || 
	       (BuildProofObject < 0))
	    {
	       Error("Option --proof-object) accepts "
		     "argument from {0..3}", USAGE_ERROR);
	    }            
            break;
      case OPT_PCL_COMPRESSED:
	    pcl_full_terms = false;
	    break;
      case OPT_PCL_COMPACT:
	    PCLStepCompact = true;
	    break;
      case OPT_PCL_SHELL_LEVEL:
            PCLShellLevel =  CLStateGetIntArg(handle, arg);
	    if((PCLShellLevel > 2) || 
	       (PCLShellLevel < 0))
	    {
	       Error("Option --pcl-shell-level) accepts "
		     "argument from {0..2}", USAGE_ERROR);
	    }
            break;
      case OPT_PRINT_STATISTICS:
	    print_statistics = true;
	    break;
      case OPT_EXPENSIVE_DETAILS:
	    TBPrintDetails = true;
	    break;
      case OPT_PRINT_SATURATED:
	    outdesc = arg;
	    if(!CheckLetterString(outdesc, "eigEIGaA"))
	    {
	       Error("Illegal argument to option -S"
		     " (--print-saturated)", 
		     USAGE_ERROR);
	    }
	    print_sat = true;
	    break;	    
      case OPT_PRINT_SAT_INFO:
	    outinfo = true;
	    break;
      case OPT_FILTER_SATURATED:
	    filterdesc = arg;
	    if(!CheckLetterString(filterdesc, "ucnrfNRF"))
	    {
	       Error("Illegal argument to option --filter-saturated",
		     USAGE_ERROR);
	    }
	    filter_sat = true;	    
	    break;
      case OPT_PRUNE_ONLY:
            OutputLevel = 4;
            prune_only   = true;
            break;
      case OPT_CNF_ONLY:
            outdesc    = "eigEIG";
            print_sat  = true;
            proc_limit = 0;
            cnf_only   = true;
            break;
      case OPT_PRINT_PID:
	    print_pid = true;
	    break;
      case OPT_PRINT_VERSION:
	    print_version = true;
	    break;
      case OPT_REQUIRE_NONEMPTY:
	    error_on_empty = true;
	    break;
      case OPT_MEM_LIMIT:
            if(strcmp(arg, "Auto")==0)
            {              
               long tmpmem =  GetSystemPhysMemory();
               
               if(tmpmem==-1)
               {
                  Error("Cannot find physical memory automatically. "
                        "Give explicit value to --memory-limit", OTHER_ERROR);
               }               
               VERBOSE(fprintf(stderr, 
                               "Physical memory determined as %ld MB\n",
                               tmpmem););

               mem_limit = 0.8*tmpmem;  
               if(mem_limit > 2048)
               {  /* Many OSes cannot handle more than 2GB per process */
                  mem_limit = 2048; 
               }
               h_parms->delete_bad_limit =
                  (float)(mem_limit-2)*0.7*MEGA;
            }
            else
            {
               /* We expect the user to know what he is doing */
               mem_limit = CLStateGetIntArg(handle, arg);
            }
            VERBOSE(fprintf(stderr, 
                            "Memory limit set to %ld MB\n", 
                            mem_limit););
            h_parms->mem_limit = MEGA*mem_limit;
            break;
      case OPT_CPU_LIMIT:
	    HardTimeLimit = CLStateGetIntArg(handle, arg);
            ScheduleTimeLimit = HardTimeLimit;
	    if((SoftTimeLimit != RLIM_INFINITY) &&
               (HardTimeLimit<=SoftTimeLimit))
            {
               Error("Hard time limit has to be larger than soft"
                     "time limit", USAGE_ERROR);
            }
            break;
      case OPT_SOFTCPU_LIMIT:
	    SoftTimeLimit = CLStateGetIntArg(handle, arg);
            ScheduleTimeLimit = SoftTimeLimit;

	    if((HardTimeLimit != RLIM_INFINITY) &&
	       (HardTimeLimit<=SoftTimeLimit))
            {
               Error("Soft time limit has to be smaller than hard"
			"time limit", USAGE_ERROR);
	    }	    
	    break;
      case OPT_RUSAGE_INFO:
	    print_rusage = true;
	    break;
      case OPT_STEP_LIMIT:
	    step_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_ANSWER_LIMIT:
	    answer_limit = CLStateGetIntArg(handle, arg);            
	    break;
      case OPT_CONJ_ARE_QUEST:
            conjectures_are_questions = true;
            break;
      case OPT_PROC_LIMIT:
	    proc_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_UNPROC_LIMIT:
	    unproc_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_TOTAL_LIMIT:
	    total_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_NO_INFIX:
	    EqnUseInfix = false;
	    break;
      case OPT_FULL_EQ_REP:
	    EqnFullEquationalRep = true;
	    break;
      case OPT_TPTP_PARSE:
	    parse_format = TPTPFormat;
	    break;
      case OPT_TPTP_PRINT:
	    OutputFormat = TPTPFormat;
	    EqnFullEquationalRep = false;
	    EqnUseInfix = false;
	    break;
      case OPT_TPTP_FORMAT:
	    parse_format = TPTPFormat;	    
	    OutputFormat = TPTPFormat;
	    EqnFullEquationalRep = false;
	    EqnUseInfix = false;
	    break;
      case OPT_TSTP_PARSE:
	    parse_format = TSTPFormat;
	    break;
      case OPT_TSTP_PRINT:
	    DocOutputFormat = tstp_format;
	    OutputFormat = TSTPFormat;
	    EqnUseInfix = true;
	    break;
      case OPT_TSTP_FORMAT:
	    parse_format = TSTPFormat;
	    DocOutputFormat = tstp_format;
	    OutputFormat = TSTPFormat;
	    EqnUseInfix = true;
	    break;
      case OPT_AUTO:
	    h_parms->heuristic_name = "Auto";
            h_parms->ordertype = AUTO;
            sine = "Auto";
            break;
      case OPT_SATAUTO:
	    h_parms->heuristic_name = "Auto";
            h_parms->ordertype = AUTO;
            break;
      case OPT_AUTODEV:
	    h_parms->heuristic_name = "AutoDev";
            h_parms->ordertype = AUTODEV;
            sine = "Auto";
            break;
      case OPT_SATAUTODEV:
	    h_parms->heuristic_name = "AutoDev";
            h_parms->ordertype = AUTODEV;
            break;
      case OPT_AUTO_SCHED:
            strategy_scheduling = true;
            sine = "Auto";                        
            break;
      case OPT_NO_PREPROCESSING:
	    no_preproc = true;
	    break;
      case OPT_EQ_UNFOLD_LIMIT:
            eqdef_incrlimit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_EQ_UNFOLD_MAXCLAUSES:
            eqdef_maxclauses = CLStateGetIntArg(handle, arg);
            break;
      case OPT_NO_EQ_UNFOLD:
	    eqdef_incrlimit = INT_MIN;
	    break;
      case OPT_SINE:
            sine = arg;
            break;
      case OPT_REL_PRUNE_LEVEL:
            relevance_prune_level = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PRESAT_SIMPLIY:
            h_parms->presat_interreduction = true;
            break;
      case OPT_AC_HANDLING:
	    if(strcmp(arg, "None")==0)
	    {
	       h_parms->ac_handling = NoACHandling;
	    }
	    else if(strcmp(arg, "DiscardAll")==0)
	    {
	       h_parms->ac_handling = ACDiscardAll;
	    }
	    else if(strcmp(arg, "KeepUnits")==0)
	    {
	       h_parms->ac_handling = ACKeepUnits;
	    }
	    else if(strcmp(arg, "KeepOrientable")==0)
	    {
	       h_parms->ac_handling = ACKeepOrientable;
	    }
	    else
	    {
	       Error("Option --ac_handling requires None, DiscardAll, "
		     "KeepUnits, or KeepOrientable as an argument",
		     USAGE_ERROR);
	    }
	    break;
      case OPT_AC_ON_PROC:
	    h_parms->ac_res_aggressive = false;
	    break;
      case OPT_NO_GENERATION:
	    h_parms->selection_strategy=SelectNoGeneration;
	    break;
      case OPT_SELECT_ON_PROC_ONLY:
	    h_parms->select_on_proc_only = true;
	    break;
      case OPT_INHERIT_PM_LIT:
	    h_parms->inherit_paramod_lit = true;
	    break;
      case OPT_INHERIT_GOAL_PM_LIT:
	    h_parms->inherit_goal_pm_lit = true;
	    break;
      case OPT_INHERIT_CONJ_PM_LIT:
	    h_parms->inherit_conj_pm_lit = true;
	    break;

      case OPT_LITERAL_SELECT:
	    h_parms->selection_strategy = GetLitSelFun(arg);
	    if(!h_parms->selection_strategy)
	    {
	       DStr_p err = DStrAlloc();
	       DStrAppendStr(err, 
			     "Wrong argument to option -W "
			     "(--literal-selection-strategy). Possible "
			     "values: "); 
	       LitSelAppendNames(err);
	       Error(DStrView(err), USAGE_ERROR);
	       DStrFree(err);
	    }
            if(h_parms->selection_strategy == SelectNoGeneration)
            {
               inf_sys_complete = false;
            }
	    break;
      case OPT_POS_LITSEL_MIN:
	    h_parms->pos_lit_sel_min = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_POS_LITSEL_MAX:
	    h_parms->pos_lit_sel_max = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_NEG_LITSEL_MIN:
	    h_parms->neg_lit_sel_min = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_NEG_LITSEL_MAX:
	    h_parms->neg_lit_sel_max = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_ALL_LITSEL_MIN:
	    h_parms->all_lit_sel_min = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_ALL_LITSEL_MAX:
	    h_parms->all_lit_sel_max = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_WEIGHT_LITSEL_MIN:
	    h_parms->weight_sel_min = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_PREFER_INITIAL_CLAUSES:
	    h_parms->prefer_initial_clauses = true;
	    break;
      case OPT_HEURISTIC:
	    h_parms->heuristic_name = arg;
	    break;
      case OPT_FILTER_LIMIT:
	    h_parms->filter_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_FILTER_COPIES_LIMIT:
	    h_parms->filter_copies_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_DELETE_BAD_LIMIT:
	    h_parms->delete_bad_limit = CLStateGetIntArg(handle, arg);
	    break; 
      case OPT_ASSUME_COMPLETENESS:
            assume_inf_sys_complete = true;
	    break;	    
      case OPT_ASSUME_INCOMPLETENESS:
            incomplete = true;
	    break;	    
      case OPT_NO_GC_FORWARD_SIMPL:
            h_parms->enable_given_forward_simpl = false;
            break;
      case OPT_DISABLE_EQ_FACTORING:
            h_parms->enable_eq_factoring = false;
            inf_sys_complete = false;
            break;
      case OPT_DISABLE_NEGUNIT_PM:
            h_parms->enable_neg_unit_paramod = false;
            inf_sys_complete = false;
            break;
      case OPT_CONDENSING:
            h_parms->condensing = true;
            break;
      case OPT_CONDENSING_AGGRESSIVE:
            h_parms->condensing = true;
            h_parms->condensing_aggressive = true;
            break;
      case OPT_USE_SIM_PARAMOD:
            h_parms->pm_type = ParamodAlwaysSim;
            break;
       case OPT_USE_ORIENTED_SIM_PARAMOD:
            h_parms->pm_type = ParamodOrientedSim;
            break;
      case OPT_SPLIT_TYPES:
	    h_parms->split_clauses = CLStateGetIntArg(handle, arg);
	    break;	    
      case OPT_SPLIT_HOW:
	    tmp = CLStateGetIntArg(handle, arg);
	    if((tmp < 0) ||(tmp > 2))
	    {
	       Error("Argument to option --split-method "
		     "has to be value between 0 and 2", USAGE_ERROR);
	    }
            h_parms->split_method = tmp;
	    break;
      case OPT_SPLIT_AGGRESSIVE:
	    h_parms->split_aggressive = true;
	    break;
      case OPT_SPLIT_REUSE_DEFS:
	    h_parms->split_fresh_defs = false;
	    break;
      case OPT_REWEIGHT_LIMIT:
	    h_parms->reweight_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_ORDERING:	    
	    if(strcmp(arg, "Auto")==0)
	    {
	       h_parms->ordertype = AUTO;
	    }
	    else if(strcmp(arg, "AutoCASC")==0)
	    {
	       h_parms->ordertype = AUTOCASC;
	    }
	    else if(strcmp(arg, "AutoDev")==0)
	    {
	       h_parms->ordertype = AUTODEV;
	    }
	    else if(strcmp(arg, "AutoSched0")==0)
	    {
	       h_parms->ordertype = AUTOSCHED0;
	    }
	    else if(strcmp(arg, "AutoSched1")==0)
	    {
	       h_parms->ordertype = AUTOSCHED1;
	    }
	    else if(strcmp(arg, "AutoSched2")==0)
	    {
	       h_parms->ordertype = AUTOSCHED2;
	    }
	    else if(strcmp(arg, "AutoSched3")==0)
	    {
	       h_parms->ordertype = AUTOSCHED3;
	    }
	    else if(strcmp(arg, "AutoSched4")==0)
	    {
	       h_parms->ordertype = AUTOSCHED4;
	    }
	    else if(strcmp(arg, "Optimize")==0)
	    {
	       h_parms->ordertype = OPTIMIZE_AX;
	    }
	    else if(strcmp(arg, "LPO")==0)
	    {
	       h_parms->ordertype = LPO;
	    }
            else if(strcmp(arg, "LPOCopy")==0)
	    {
	       h_parms->ordertype = LPOCopy;
	    }	       
            else if(strcmp(arg, "LPO4")==0)
	    {
	       h_parms->ordertype = LPO4;
	    }
            else if(strcmp(arg, "LPO4Copy")==0)
	    {
	       h_parms->ordertype = LPO4Copy;
	    }
	    else if(strcmp(arg, "KBO")==0)
	    {
	       h_parms->ordertype = KBO;
            }
	    else if(strcmp(arg, "KBO6")==0)
	    {
	       h_parms->ordertype = KBO6;
	    }
	    else
	    {
	       Error("Option -t (--term-ordering) requires Auto, "
		     "AutoCASC, AutoDev, AutoSched0, AutoSched1, "
                     "AutoSched2, AutoSched3, AutoSched4, Optimize, "
                     "LPO, LPO4, or KBO as an argument", 
		     USAGE_ERROR);
	    }
	    break;
      case OPT_TO_WEIGHTGEN:
	    h_parms->to_weight_gen = TOTranslateWeightGenMethod(arg);
	    if(!h_parms->to_weight_gen)
	    {
	       DStr_p err = DStrAlloc();
	       DStrAppendStr(err, 
			     "Wrong argument to option -w "
			     "(--order-weight-generation). Possible "
			     "values: "); 
	       DStrAppendStrArray(err, TOWeightGenNames, ", ");
	       Error(DStrView(err), USAGE_ERROR);
	       DStrFree(err);
	    }		      
	    break;
      case OPT_TO_WEIGHTS:
	    h_parms->to_pre_weights = arg;
            break;
      case OPT_TO_PRECGEN:
	    h_parms->to_prec_gen = TOTranslatePrecGenMethod(arg);
	    if(!h_parms->to_prec_gen)
	    {
	       DStr_p err = DStrAlloc();
	       DStrAppendStr(err, 
			     "Wrong argument to option -G "
			     "(--order-precedence-generation). Possible "
			     "values: "); 
	       DStrAppendStrArray(err, TOPrecGenNames, ", ");
	       Error(DStrView(err), USAGE_ERROR);
	       DStrFree(err);
	    }		      
	    break;
      case OPT_TO_CONSTWEIGHT:
	    h_parms->to_const_weight = CLStateGetIntArg(handle, arg); 
	    if(h_parms->to_const_weight<=0)
	    {
	       Error("Argument to option -c (--order-constant-weight) "
		     "has to be > 0", USAGE_ERROR);
	    }
	    break;
      case OPT_TO_PRECEDENCE:
	    h_parms->to_pre_prec = arg;
	    break;
      case OPT_TO_LPO_RECLIMIT:
	    LPORecursionDepthLimit = CLStateGetIntArg(handle, arg); 
	    if(LPORecursionDepthLimit<=0)
	    {
	       Error("Argument to option --lpo-recursion-limit "
		     "has to be > 0", USAGE_ERROR);
	    }
	    if(LPORecursionDepthLimit>20000)
	    {
	       Warning("Using very large values for "
		       "--lpo-recursion-limit may lead to stack "
		       "overflows and segmentation faults.");
	    }
      case OPT_TO_RESTRICT_LIT_CMPS:
	    h_parms->no_lit_cmp = true;
	    break;
      case OPT_TPTP_SOS:
	    h_parms->use_tptp_sos = true;
	    break;
      case OPT_ER_DESTRUCTIVE:
	    h_parms->er_varlit_destructive = true;
	    break;
      case OPT_ER_STRONG_DESTRUCTIVE:
	    h_parms->er_varlit_destructive = true; /* Implied */
	    h_parms->er_strong_destructive = true;
	    break;
      case OPT_ER_AGGRESSIVE:
	    h_parms->er_aggressive = true;
	    break;
      case OPT_FORWARD_CSR:
	    h_parms->forward_context_sr = true;
	    break;
      case OPT_FORWARD_CSR_AGGRESSIVE:
	    h_parms->forward_context_sr = true;
	    h_parms->forward_context_sr_aggressive = true;
	    break;
      case OPT_BACKWARD_CSR:
	    h_parms->backward_context_sr = true;
	    break;
      case OPT_RULES_GENERAL:
	    h_parms->prefer_general = true;
	    break;
      case OPT_FORWARD_DEMOD:
	    tmp =  CLStateGetIntArg(handle, arg);
	    if((tmp > 2) || 
	       (tmp < 0))
	    {
	       Error("Option -F (--forward_demod_level) requires "
		     "argument from {0..2}", USAGE_ERROR);
	    }
	    h_parms->forward_demod = tmp;
	    break;
      case OPT_STRONG_RHS_INSTANCE:
            RewriteStrongRHSInst = true;
            break;
      case OPT_STRONGSUBSUMPTION:
	    StrongUnitForwardSubsumption = true;
	    break;  
      case OPT_WATCHLIST:
            if(strcmp(WATCHLIST_INLINE_STRING, arg)==0 ||
               strcmp(WATCHLIST_INLINE_QSTRING, arg)==0  )
            {
               watchlist_filename = UseInlinedWatchList;
            }
            else
            {
               watchlist_filename = arg;
            }
	    break;
      case OPT_WATCHLIST_NO_SIMPLIFY:
	    h_parms->watchlist_simplify = false;
	    break;
      case OPT_NO_INDEXED_SUBSUMPTION:
	    fvi_parms->cspec.features = FVINoFeatures;
	    break;
      case OPT_FVINDEX_STYLE:
	    if(strcmp(arg, "None")==0)
	    {
	       fvi_parms->cspec.features = FVINoFeatures;
	    }
	    else if(strcmp(arg, "Direct")==0)
	    {
	       fvi_parms->use_perm_vectors = false;
	    }
	    else if(strcmp(arg, "Perm")==0)
	    {
	       fvi_parms->use_perm_vectors = true;
	       fvi_parms->eliminate_uninformative = false;       
	    }
	    else if(strcmp(arg, "PermOpt")==0)
	    {
	       fvi_parms->use_perm_vectors = true;
	       fvi_parms->eliminate_uninformative = true;
	    }
	    else
	    {
	       Error("Option --subsumption-indexing requires "
		     "'None', 'Direct', 'Perm', or 'PermOpt'.", USAGE_ERROR);
	    }
	    break;
      case OPT_FVINDEX_FEATURETYPES:
	    if(strcmp(arg, "None")==0)
	    {
	       fvi_parms->cspec.features = FVINoFeatures;
	    }
	    else if(strcmp(arg, "AC")==0)
	    {
	       fvi_parms->cspec.features = FVIACFeatures;
	    }
	    else if(strcmp(arg, "SS")==0)
	    {
	       fvi_parms->cspec.features = FVISSFeatures;
	    }
	    else if(strcmp(arg, "All")==0)
	    {
	       fvi_parms->cspec.features = FVIAllFeatures;
	    }
	    else if(strcmp(arg, "Bill")==0)
	    {
	       fvi_parms->cspec.features = FVIBillFeatures;
	    }
	    else if(strcmp(arg, "BillPlus")==0)
	    {
	       fvi_parms->cspec.features = FVIBillPlusFeatures;
	    }
	    else if(strcmp(arg, "ACFold")==0)
	    {
	       fvi_parms->cspec.features = FVIACFold;
	    }
	    else if(strcmp(arg, "ACStagger")==0)
	    {
	       fvi_parms->cspec.features = FVIACStagger;
	    }
	    else
	    {
	       Error("Option --fvindex-featuretypes requires "
		     "'None', 'AC', 'SS', or 'All'.", USAGE_ERROR);
	    }
	    break;
      case OPT_FVINDEX_MAXFEATURES:
	    tmp = CLStateGetIntArg(handle, arg);
	    if(tmp<=0)
	    {
	       Error("Argument to option --fvindex-maxfeatures "
		     "has to be > 0", USAGE_ERROR);
	    }
	    fvi_parms->max_symbols = tmp;
	    break;
      case OPT_FVINDEX_SLACK:
	    tmp = CLStateGetIntArg(handle, arg);
	    if(tmp<0)
	    {
	       Error("Argument to option --fvindex-slack "
		     "has to be >= 0", USAGE_ERROR);
	    }
	    fvi_parms->symbol_slack = tmp;
	    break;
      case OPT_RW_BW_INDEX:
            check_fp_index_arg(arg, "--rw-bw-index");
            strcpy(h_parms->rw_bw_index_type, arg);
            break;
      case OPT_PM_FROM_INDEX:
            check_fp_index_arg(arg, "--pm-from-index");
            strcpy(h_parms->pm_from_index_type, arg);
            break;
      case OPT_PM_INTO_INDEX:
            check_fp_index_arg(arg, "--pm-into-index");
            strcpy(h_parms->pm_into_index_type, arg);
            break;
      case OPT_FP_INDEX:
            check_fp_index_arg(arg, "--fp-index");
            strcpy(h_parms->rw_bw_index_type, arg);
            strcpy(h_parms->pm_from_index_type, arg);
            strcpy(h_parms->pm_into_index_type, arg);
            break;
      case OPT_DETSORT_RW:
            h_parms->detsort_bw_rw = true;
            break;
      case OPT_DETSORT_NEW:
            h_parms->detsort_tmpset = true;
            break;
      case OPT_DEFINE_WFUN:
	    PStackPushP(wfcb_definitions, arg);
	    break;
      case OPT_DEFINE_HEURISTIC:
	    PStackPushP(hcb_definitions, arg);
	    break;
      case OPT_FREE_NUMBERS:
            free_symb_prop = free_symb_prop|FPIsInteger|FPIsRational|FPIsFloat;
            break;
      case OPT_FREE_OBJECTS:
            free_symb_prop = free_symb_prop|FPIsObject;
            break;
      case OPT_DEF_CNF:
            FormulaDefLimit     = CLStateGetIntArg(handle, arg);
            break;
      default:
	    assert(false && "Unknown option");
	    break;
      }
   }
   if((HardTimeLimit!=RLIM_INFINITY)||(SoftTimeLimit!=RLIM_INFINITY))
   {
      if(SoftTimeLimit!=RLIM_INFINITY)
      {
         SetSoftRlimit(RLIMIT_CPU, SoftTimeLimit);
	 TimeLimitIsSoft = true;
      }
      else
      {
         SetSoftRlimit(RLIMIT_CPU, HardTimeLimit);
	 TimeLimitIsSoft = false;
      }

      if(SetSoftRlimit(RLIMIT_CORE, 0)!=RLimSuccess)
      {
         perror("eprover");
         Warning("Cannot prevent core dumps!");
      }
   }
   SetMemoryLimit(h_parms->mem_limit);

   return state;
}

void print_help(FILE* out)
{
   fprintf(out, "\n\
E " VERSION " \"" E_NICKNAME "\"\n\
\n\
Usage: " NAME " [options] [files]\n\
\n\
Read a set of first-order clauses and formulae and try to refute it.\n\
\n");
   PrintOptions(stdout, opts, "Options:\n\n");
   fprintf(out, "\n\
"STS_COPYRIGHT", " STS_MAIL "\n\
\n\
You can find the latest version of E and additional information at\n"
E_URL
"\n\n"
"This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program (it should be contained in the top level\n\
directory of the distribution in the file COPYING); if not, write to\n\
the Free Software Foundation, Inc., 59 Temple Place, Suite 330,\n\
Boston, MA  02111-1307 USA\n"
"\n\
The original copyright holder can be contacted as\n\
\n"
STS_SNAIL
"\n");

}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


