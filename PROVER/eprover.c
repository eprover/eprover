/*-----------------------------------------------------------------------

File  : eprover.c

Author: Stephan Schulz

Contents
 
  Main program for the E equational theorem prover.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jun  9 01:32:15 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include <sys/time.h>
#include <unistd.h>

#include <cio_commandline.h>
#include <cio_output.h>
#include <cco_proofproc.h>
#include <cio_signals.h>
#include <ccl_unfold_defs.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define VERSION      "0.8dev018"
#define NAME         "eprover"

#ifdef SAFELOGIC
#define NICKNAME     "Steinthal (Proprietary Safelogic build)"
#else
#define NICKNAME     "Steinthal"
#endif

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
   OPT_PRINT_PID,
   OPT_REQUIRE_NONEMPTY,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_PCL_COMPRESSED,
   OPT_PCL_COMPACT,
   OPT_MEM_LIMIT,
   OPT_CPU_LIMIT,
   OPT_SOFTCPU_LIMIT,
   OPT_RUSAGE_INFO,
   OPT_STEP_LIMIT,
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
   OPT_NO_PREPROCESSING,
   OPT_NO_EQ_UNFOLD,
   OPT_NO_NEG_PARAMOD,
   OPT_NO_NONEQ_PARAMOD,
   OPT_AC_HANDLING,
   OPT_AC_ON_PROC,
   OPT_LITERAL_SELECT,
   OPT_NO_GENERATION,
   OPT_SELECT_ON_PROC_ONLY,
   OPT_INHERIT_PM_LIT,
   OPT_INHERIT_GOAL_PM_LIT,
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
   OPT_SPLIT_TYPES,
   OPT_SPLIT_HOW,
   OPT_SPLIT_AGGRESSIVE,
   OPT_ORDERING,
   OPT_TO_WEIGHTGEN,
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
   OPT_STRONGSUBSUMPTION,
   OPT_WATCHLIST,
   OPT_WATCHLIST_NO_SIMPLIFY,
   OPT_NO_INDEXED_SUBSUMPTION,
   OPT_FVINDEX_STYLE,
   OPT_FVINDEX_FEATURETYPES,
   OPT_FVINDEX_MAXFEATURES,
   OPT_FVINDEX_SLACK,
   OPT_UNPROC_UNIT_SIMPL,
   OPT_DEFINE_WFUN,
   OPT_DEFINE_HEURISTIC,
   OPT_HEURISTIC,
#ifdef SAFELOGIC
   OPT_INTERPRETE_NUMBERS,
#endif
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
    "Print the version number of the prover."},

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
    " also imply a certain clause format (LOP with infix equality "
    "predicate) that is expected by the postprocessing tools."},
   
   {OPT_PCL_COMPRESSED,
    '\0', "pcl-terms-compressed",
    NoArg, NULL,
    "Print terms in the PCL output in shared representation."},

   {OPT_PCL_COMPACT,
    '\0', "pcl-compact",
    NoArg, NULL,
    "Print PCL steps without additional spaces for formatting (safes "
    "disk space for larfge protocols)."},

   {OPT_PRINT_STATISTICS,
    '\0', "print-statistics",
    NoArg, NULL,
    "Print the inference statistics (only relevant for output level 0,"
    "\notherwise they are printed automatically."},

   {OPT_EXPENSIVE_DETAILS,
    '\0', "print-detailed-statistics",
    NoArg, NULL,
    "Print data about the proof state that is potentially expensive "
    "to collect. Includes number of term cells, average superterm "
    "number, and number of rewrite steps."},

   {OPT_PRINT_SATURATED,
    'S', "print-saturated",
    OptArg, DEFAULT_OUTPUT_DESCRIPTOR,
    "Print the (semi-) saturated clause sets after terminating the "
    "saturation process. The argument given describes which parts "
    "should be printed in which order. Legal caracters are 'eigEIGaA',"
    " standing for processed positive units, processed negative units, "
    "processed non-units, unprocessed positive units, unprocessed "
    "negative units, unprocessed non-units, and two types of "
    "additional equality axioms, respectively. Equality axioms will "
    "only be printed if the original specification contained real "
    "equality. In this case, 'a' requests axioms in which a separate" 
    " substitutivity axiom is given for each argument position of a"
    " subction or predicate symbol, while 'A' requests a single "
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
   
   {OPT_PRINT_PID,
    '\0', "print-pid",
    NoArg, NULL,
    "Print the process id of the prover as a comment after option "
    "processing."},

   {OPT_REQUIRE_NONEMPTY,
    '\0', "error-on-empty",
    NoArg, NULL,
    "Return with an error code if the input file contains no clauses. "
    "Formally, the empty clause set is satisfiable, and E will treat "
    "it as such. However, in composite systems this is more often a "
    "sign that something went wrong. Use this option to catch such" 
    " bugs."},

   {OPT_MEM_LIMIT,
    'm', "memory-limit",
    ReqArg, NULL,
    "Limit the memory the prover may use. The argument is "
    "the allowed amount of memory in MB. This option may not work "
    "everywhere, due to broken and/or strange behaviour of setrlimit() "
    "in some UNIX implementations. It does work under all tested "
    "versions of Solaris and GNU/Linux."},   

   {OPT_CPU_LIMIT,
    '\0', "cpu-limit",
    OptArg, "300",
    "Limit the cpu time the prover should run. The optional argument "
    "is the CPU time in seconds. The prover will terminate immediately"
    " after reaching the time limit, regardless of internal state. This"
    " option may not work " 
    "everywhere, due to broken and/or strange behaviour of setrlimit() "
    "in some UNIX implementations. It does work under all tested "
    "versions of Solaris, HP-UX and GNU/Linux. As a side effect, this "
    "option will inhibit core file writing. Please note that if you"
    " use both --cpu-limit and --soft-cpu-limit, the soft limit has to"
    " be larger than the hard limit"}, 

   {OPT_SOFTCPU_LIMIT,
    '\0', "soft-cpu-limit",
    OptArg, "310",
    "Limit the cpu time the prover should spend in the main saturation"
    " phase. The prover will then terminate gracefully, i.e. it will "
    "perform post-processing, " 
    "filtering and printing of unprocessed clauses, if these options"
    " are selected. Note"
    " that for some filtering options (in particular those which "
    "perform full subsumption), the postprocessing time may well be"
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
    "Set the maximal number of clauses to process."},
   
   {OPT_PROC_LIMIT,
    'P', "processed-set-limit",
    ReqArg, NULL,
    "Set the maximal size of the set of processed clauses."},

   {OPT_UNPROC_LIMIT,
    'U', "unprocessed-limit",
    ReqArg, NULL,
    "Set the maximal size of the set of unprocessed clauses."},
   
   {OPT_TOTAL_LIMIT,
    'T', "total-clause-set-limit",
    ReqArg, NULL,
    "Set the maximal size of the set of all clauses."},

   {OPT_NO_INFIX,
    'n', "eqn-no-infix",
    NoArg, NULL,
    "Print equations in prefix notation equal(x,y)."},

   {OPT_FULL_EQ_REP,
    'e', "full-equational-rep",
    NoArg, NULL,
    "Print all literals as equations, even non-equational ones."},

   {OPT_TPTP_PARSE,
    '\0', "tptp-in",
    NoArg, NULL,
    "Parse TPTP format instead of E-LOP{ (does not understand includes, "
    "as TPTP includes are a brain-dead design)."},

   {OPT_TPTP_PRINT,
    '\0', "tptp-out",
    NoArg, NULL,
    "Print TPTP format instead of E-LOP. Implies --eqn-no-infix and "
    "will ignore --full-equational-rep."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp-format",
    NoArg, NULL,
    "Equivalent to --tptp-in and --tptp-out."},

   {OPT_TSTP_PARSE,
    '\0', "tstp-in",
    NoArg, NULL,
    "Parse limited TSTP format instead of E-LOP (no optional "
    "extensions) ."},
   
   {OPT_TSTP_PRINT,
    '\0', "tstp-out",
    NoArg, NULL,
    "Print proof protocol in TSTP syntax (default is PCL). Only "
    "effective for output levels greater than 1."},

   {OPT_TSTP_FORMAT,
    '\0', "tstp-format",
    NoArg, NULL,
    "Equivalent to --tstp-in and --tstp-out."},

   {OPT_NO_PREPROCESSING,
    '\0', "no-preprocessing",
    NoArg, NULL,
    "Do not perform preprocessing on the initial clause set. "
    "Preprocessing currently removes tautologies and orders terms, "
    "literals and clauses in a certain (\"canonical\") way before "
    "anything else happens. Unless the next option is set, it will"
    "also unfold equational definitions."},
   {OPT_NO_EQ_UNFOLD,
    '\0', "no-eq-unfolding",
    NoArg, NULL,
    "During preprocessing, abstain from unfolding (and removing) "
    "equational definitions."},

   {OPT_NO_NEG_PARAMOD,
    'N', "no-paramod-into-negatives",
    NoArg, NULL,
    "Refrain from paramodulating into negative literals."},

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

   {OPT_NO_NONEQ_PARAMOD,
    'E', "paramod-equational-literals-only",
    NoArg, NULL,
    "Don't paramodulate from or into non-equational literals."},
    
   {OPT_LITERAL_SELECT,
    'W', "literal-selection-strategy",
    ReqArg, NULL,
    "Choose a strategy for selection of negative literals. There are "
    "two special values for this option: NoSelection will select no "
    "literal (i.e. perform normal superposition) and "
    "NoGeneration will inhibit all generating inferences. For a list "
    "of the other (hopefully self-documenting) values run "
    "'" NAME " -W none'. There are two variants of each strategy. The "
    "one prefixed with 'P' will allow paramodulations into maximal "
    "positive literals in addition to paramodulations into maximal "
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
    " available: Auto, Weight, StandardWeight, RWeight, FIFO, LIFO and"
    " Uniq. Auto is recommended if you only want to find a proof. It "
    "will also set some additional options. To have optimal "
    "performance, you also have to specify -tAuto to select a good "
    "term ordering. LIFO is unfair and will " 
    "make the prover incomplete. Uniq is used internally and is not "
    "very useful in most cases. You can define more heuristics using"
    " the option -H (see below)."},

   {OPT_FILTER_LIMIT,
    '\0', "filter-limit",
    OptArg, "1000000",
    "Set the limit on the number of 'storage units' in the proof "
    "state, after which the set of unprocessed clauses will be "
    "filtered against the processed clauses to eleminate redundant "
    "clauses. As of E 0.7, a 'storage unit' is approximately one byte, "
    "however, storage is estimated in an abstract way, independend of "
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
    " you if a proof attempt failed due to the incompletness"
    " introduced by this option. It is recommended to set this limit"
    " significantly higher than --filter-limit or"
    " --filter-copies-limit. If you select -xAuto and set a"
    " memory limit, the prover will determine a good value"
    " automatically."}, 

   {OPT_SPLIT_TYPES,
    '\0', "split-clauses",
    OptArg, "7",
    "Determine which clauses should be subject to splitting. The "
    "argument is the binary 'OR' of values for the desired classes:\n"
    "     1:  Horn clauses\n"
    "     2:  Non-Horn clauses\n"
    "     4:  Positive clauses\n"
    "     8:  Negative clauses\n"
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

   {OPT_REWEIGHT_LIMIT,
    '\0', "reweight-limit",
    OptArg, "30000",
    "Set the number of new unprocessed clauses after which the set of"
    " unprocessed clauses will be reevaluated."},

   {OPT_ORDERING,
    't', "term-ordering",
    ReqArg, NULL,
    "Select an ordering type (currently Auto, LPO, KBO or "
    "KBO1). -tAuto is suggested, in particular with -xAuto. KBO and"
    " KBO1 are different implementations of the same ordering, KBO is"
    " usually faster and has had more testing."},

   {OPT_TO_WEIGHTGEN,
    'w', "order-weight-generation",
    ReqArg, NULL,
    "Select a method for the generation of weights for use with the "
    "term ordering. Run '" NAME " -w none' for a list of options."},

   {OPT_TO_PRECGEN,
    'G', "order-precedence-generation",
    ReqArg, NULL,
    "Select a method for the generation of a precedence for use with "
    "the term ordering. Run '" NAME " -G none' for a list of "
    "options."}, 

   {OPT_TO_CONSTWEIGHT,
    'c', "order-constant-weight", 
    ReqArg, NULL,
    "Set a special weight > 0 for constants in the term ordering."
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
    "systems. If you run into segmentation faults while using the "
    "LPO, first try to set this limit to a reasonable value. If the "
    "problem persists, send a bug report ;-)"
   },

   {OPT_TO_RESTRICT_LIT_CMPS,
    '\0', "restrict-literal-comparisons",
    NoArg, NULL,
    "Make all literals uncomparable in the term ordering (i.e. do not"
    " use the term ordering to restrict paramodualtion, equality"
    " resolution and factoring to certain literals. This is necessary"
    " to make Set-of-Support-strategies complete for the non-equational"
    " case (It still is incomplete for the equational case, but pretty"
    " useless anyways)."},

   {OPT_TPTP_SOS,
    '\0', "sos-uses-input-types",
    NoArg, NULL,
    "If input is TPTP format, use TPTP conjectures for initializing "
    "the Set of Support. If not in TPTP format, use E-LOP queries "
    "(clauses of the form ?-l(X),...,m(Y)). Normaly, all negative "
    "clauses are used. Please note that " 
    "most E heuristics do not use this information at all, it is currently "
    "only useful for certain parameter settings (including the SimulateSOS"
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
    "literals of the form X!=t, i.e. replace the original clause with the "
    "result of an equality resolution inference on this literal. Unless I "
    "am braindead, this maintains completeness, although the proof is"
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
    "processed clauses. Not yet implemented!"},

   {OPT_RULES_GENERAL,
    'g', "prefer-general-demodulators",
    NoArg, NULL,
    "Prefer general demodulators. By default, use specialized"
    " demodulators."}, 
   
   {OPT_FORWARD_DEMOD,
    'F', "forward_demod_level",
    ReqArg, NULL,
    "Set the desired level for rewriting of unprocessed clauses. A "
    "value of 0 means no rewriting, 1 indicates to use rules "
    "(orientable equations) only, 2 indicates full rewriting with "
    "rules and instances of unorientable equations. Default is 2."},

   {OPT_STRONGSUBSUMPTION,
    'u', "strong-forward-subsumption",
    NoArg, NULL,
    "Try multiple positions and unit-equations to try to subsume a "
    "single new clause. Default is to search for a single position."},
   
   {OPT_WATCHLIST,
    '\0', "watchlist",
    ReqArg, NULL,
    "Give the name for a file containing clauses to be watched "
    "for during the saturation process. If a clause is generated that "
    "subsumes a watchlist clause, the subsumed clause is removed from "
    "the watchlist. The prover will terminate when the watchlist is"
    " empty. If you want to use the watchlist for guiding the proof,"
    " put the empty clause onto the list and use the built-in clause "
    "selection heuristic "
    "'UseWatchlist' (or build a heuristic yourself using the priority"
    " functions 'PreferWatchlist' and 'DeferWatchlist')."},
   
   {OPT_WATCHLIST_NO_SIMPLIFY,
    '\0', "no-watchlist-simplification",
    NoArg, NULL,
    "Normaly, that watchlist is brought into normal form with respect "
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
    "Set the maximum number of symbols for feature computation. Depending"
    " on the feature selection, a value of X here will convert into 2X "
    "features (for set subsumptiom features), 2X+2 features (for "
    "AC-compatible features) or 4X+2 features (if all features are used, "
    "the default). Note that the actually used set of features may be "
    "smaller than this if the signature does not contain enough symbols."
    " If you select a small value here, you should probably not use"
    " \"Direct\" for the --subsumption-indexing option."},

   {OPT_FVINDEX_SLACK,
    '\0', "fvindex-slack",
    OptArg, "0",
    "Set the number of slots reserved in the index for function symbols "
    "that may be introduced into the signature "
    "later, e.g. by splitting. If no new symbols are introduced, this just "
    "wastes time and memory.If PermOpt is choosen, the slackness slots will "
    "be deleted from the index anyways, but will still waste time in "
    "computing feature vectors."},

   {OPT_UNPROC_UNIT_SIMPL,
    '\0', "simplify-with-unprocessed-units",
    OptArg, "TopSimplify",
    "Determine wether to use unprocessed unit clauses for "
    "simplify-reflect (unit-cutting) "
    "and unit subsumption. Possible values are 'NoSimplify' for strict "
    "DISCOUNT loop, 'TopSimplify' to use unprocessed units at the top "
    "level only, or 'FullSimplify' to use positive units even within "
    "equations."},

   {OPT_DEFINE_WFUN,
    'D', "define-weight-function",
    ReqArg, NULL,
    "Define  a weight function (see manual for details). Later"
    " definitions override previous definitions."}, 

   {OPT_DEFINE_HEURISTIC,
    'H', "define-heuristic",
    ReqArg, NULL,
    "Define a clause selecetion heuristic (see manual for"
    " details). Later definitions override previous definitions."}, 

#ifdef SAFELOGIC
    {OPT_INTERPRETE_NUMBERS,
    '\0', "interprete-numbers",
    OptArg, "s,0",
    "Interprete numbers in the input as successor-terms. The argument "
     "is a comma-separated tuples of function symbols for the "
     "successor function and the 0 element."}, 
#endif
   
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
                  outinfo = false,
                  error_on_empty = false,
                  no_preproc = false,
                  no_eq_unfold = false,
                  pcl_full_terms = true,
                  indexed_subsumption = true;
InputFormat       parse_format = LOPFormat;
long              step_limit = LONG_MAX, 
                  proc_limit = LONG_MAX,
                  unproc_limit = LONG_MAX, 
                  total_limit = LONG_MAX;
char              *outdesc = DEFAULT_OUTPUT_DESCRIPTOR,
                  *filterdesc = DEFAULT_FILTER_DESCRIPTOR;
#ifdef SAFELOGIC
char              *null_symbol=NULL, *succ_symbol=NULL;
#endif


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[]);
void print_help(FILE* out);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
   CLState_p        state;
   Scanner_p        in;    
   ProofState_p     proofstate;
   ProofControl_p   proofcontrol;
   int              i;
   Clause_p         success, filter_success;
   bool             out_of_clauses;
   char*            finals_state = "exists";
   long             parsed_clause_no, preproc_removed=0;

   assert(argv[0]);
   
   InitOutput();
   InitError(NAME);
   ESignalSetup(SIGXCPU);

   h_parms = HeuristicParmsAlloc();
   fvi_parms = FVIndexParmsAlloc();

   state = process_options(argc, argv);

   GlobalOut = OutOpen(outname);

   if(print_pid)
   {
      fprintf(GlobalOut, "# Pid: %d\n", (int)getpid());
      fflush(GlobalOut);
   }

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   proofstate = ProofStateAlloc();
#ifdef SAFELOGIC
   if(null_symbol)
   {
      FunCode tmp;
      
      assert(succ_symbol);
      tmp = SigInsertId(proofstate->signature, null_symbol, 0, false);
      proofstate->signature->null_code = tmp;
      tmp = SigInsertId(proofstate->signature, succ_symbol, 1, false);
      proofstate->signature->succ_code = tmp;
   }
#endif
   proofcontrol = ProofControlAlloc();
   
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL);
      ScannerSetFormat(in, parse_format);
      ClauseSetParseList(in, proofstate->axioms,
			 proofstate->original_terms);
      CheckInpTok(in, NoToken);
      DestroyScanner(in); 
   }
   VERBOUT2("Specification read\n");
   if(error_on_empty && ClauseSetEmpty(proofstate->axioms))
   {
#ifdef PRINT_SOMEERRORS_STDOUT
      fprintf(GlobalOut, "# Error: Input file contains no clauses\n");
      TSTPOUT(GlobalOut, "InputError");
#endif
      Error("Input file contains no clauses", OTHER_ERROR);
   }
   ClauseSetDocInital(GlobalOut, OutputLevel, proofstate->axioms);
   if(watchlist_filename)
   {
      proofstate->watchlist = ClauseSetAlloc();
      
      in = CreateScanner(StreamTypeFile, watchlist_filename, true, NULL);
      ScannerSetFormat(in, parse_format);
      ClauseSetParseList(in, proofstate->watchlist,
			 proofstate->original_terms);
      CheckInpTok(in, NoToken);
      DestroyScanner(in);
      ClauseSetSetProp(proofstate->watchlist, CPWatchOnly);
      ClauseSetDocInital(GlobalOut, OutputLevel, proofstate->watchlist);
   }
   
   parsed_clause_no = proofstate->axioms->members;
   if(!no_preproc)
   {
      preproc_removed = ClauseSetPreprocess(proofstate->axioms,
					    proofstate->watchlist,
					    proofstate->tmp_terms,
					    no_eq_unfold);
   }
   ProofControlInit(proofstate, proofcontrol, h_parms);
   PCLFullTerms = pcl_full_terms; /* Preprocessing always uses full
				     terms! */
   ProofStateInit(proofstate, proofcontrol, h_parms, fvi_parms);

   VERBOUT2("Prover state initialized\n");
   
   success = Saturate(proofstate, proofcontrol, step_limit,
		      proc_limit, unproc_limit, total_limit);
   
   out_of_clauses = ClauseSetEmpty(proofstate->unprocessed);
   if(filter_sat)
   {
      filter_success = ProofStateFilterUnprocessed(proofstate,
						   proofcontrol,
						   filterdesc);
      if(filter_success)
      {
	 success = filter_success;
      }
   }
   
   if(success)
   {
      DocClauseQuoteDefault(2, success, "proof");
      fprintf(GlobalOut, "\n# Proof found!\n");
      TSTPOUT(GlobalOut, "Unsatisfiable");
   }
   else if(proofstate->watchlist && ClauseSetEmpty(proofstate->watchlist))
   {      
      ProofStatePropDocQuote(GlobalOut, OutputLevel, 
			     CPSubsumesWatch, proofstate, 
			     "final_subsumes_wl");
      fprintf(GlobalOut, "\n# Watchlist is empty!\n");
      TSTPOUT(GlobalOut, "ResourceOut"); 
   }
   else
   {
      if(out_of_clauses&&
	 proofstate->state_is_complete&&
	 ParamodOverlapIntoNegativeLiterals&&
	 ParamodOverlapNonEqLiterals&&
	 (proofcontrol->selection_strategy!=SelectNoGeneration))
      {
	 finals_state = "final";
      }
      ProofStatePropDocQuote(GlobalOut, OutputLevel, CPIgnoreProps,
			     proofstate, finals_state);
      if(out_of_clauses)
      {
	 if(!ParamodOverlapIntoNegativeLiterals||
	    !ParamodOverlapNonEqLiterals||
	    (proofcontrol->selection_strategy==SelectNoGeneration))
	 {
	    fprintf(GlobalOut, 
		    "\n# Clause set closed under "
		    "restricted calculus!\n");
	    TSTPOUT(GlobalOut, "GaveUp");
	 }
	 else if(proofstate->state_is_complete)
	 {
	    fprintf(GlobalOut, "\n# No proof found!\n");
	    TSTPOUT(GlobalOut, "Satisfiable");	    
	 }
	 else
	 {
	    fprintf(GlobalOut, "\n# Failure: Out of unprocessed clauses!\n");	    
	    TSTPOUT(GlobalOut, "GaveUp");	    
	 }
      }
      else
      {
	 fprintf(GlobalOut, "\n# Failure: User resource limit exceeded!\n");
	 TSTPOUT(GlobalOut, "ResourceOut");
      }
   }
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
      fprintf(GlobalOut, "# Initial clauses:                     : %ld\n",
	      parsed_clause_no);
      fprintf(GlobalOut, "# Removed in preprocessing             : %ld\n",
	      preproc_removed);
      ProofStateStatisticsPrint(GlobalOut, proofstate);
      fprintf(GlobalOut, "# Clause-clause subsumption calls (NU) : %ld\n",
	      ClauseClauseSubsumptionCalls);
      fprintf(GlobalOut, "# Rec. Clause-clause subsumption calls : %ld\n",
	      ClauseClauseSubsumptionCallsRec);
      fprintf(GlobalOut, "# Unit Clause-clause subsumption calls : %ld\n",
	       UnitClauseClauseSubsumptionCalls);
   }
   /* {char c = getc(stdin);} */
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
   ProofStateFree(proofstate);
   CLStateFree(state);
   FVIndexParmsFree(fvi_parms);
   HeuristicParmsFree(h_parms);
#ifdef FULL_MEM_STATS
   MemFreeListPrint(GlobalOut);
#endif
#ifdef SAFELOGIC
   if(null_symbol)
   {
      assert(succ_symbol);
      FREE(null_symbol);
      FREE(succ_symbol);
   }
#endif
#endif
   if(print_rusage)
   {
      PrintRusage(GlobalOut);
   }
   fflush(GlobalOut);
   OutClose(GlobalOut);
   
#ifdef CLB_MEMORY_DEBUG
   MemFlushFreeList();
   MemDebugPrintStats(stdout);
#endif
   
   return 0;
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
   struct rlimit limit = {RLIM_INFINITY, RLIM_INFINITY};
   long   tmp;

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
	    printf("E " VERSION " " NICKNAME "\n");
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
	    if(OutputLevel >= 2)
	    {
	       EqnUseInfix = true;
	       EqnFullEquationalRep = false;
	       TPTPFormatPrint = true;
	    }
	    break;
      case OPT_PCL_COMPRESSED:
	    pcl_full_terms = false;
	    break;
      case OPT_PCL_COMPACT:
	    PCLStepCompact = true;
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
      case OPT_PRINT_PID:
	    print_pid = true;
	    break;
      case OPT_REQUIRE_NONEMPTY:
	    error_on_empty = true;
	    break;
      case OPT_MEM_LIMIT:
	    h_parms->mem_limit = CLStateGetIntArg(handle, arg);
	    if(getrlimit(RLIMIT_DATA, &limit))
	    {
	       TmpErrno = errno;
	       SysError("Unable to get current memory limit", SYS_ERROR);
	    }
	    limit.rlim_cur = ((rlim_t)(MEGA))*h_parms->mem_limit;
	    if(setrlimit(RLIMIT_DATA, &limit))
	    {
	       TmpErrno = errno;
	       SysError("Unable to set memory limit", SYS_ERROR);
	    }
#ifdef RLIMIT_AS
	    if(setrlimit(RLIMIT_AS, &limit))
	    {
	       TmpErrno = errno;
	       SysError("Unable to set memory limit", SYS_ERROR);
	    }
#endif
	    break;
      case OPT_CPU_LIMIT:
	    HardTimeLimit = CLStateGetIntArg(handle, arg);
	    if(SoftTimeLimit != RLIM_INFINITY)
	    {
	       if(HardTimeLimit<=SoftTimeLimit)
	       {
		  Error("Hard time limit has to be larger than soft"
			"time limit", USAGE_ERROR);
	       }
	    }
	    break;
      case OPT_SOFTCPU_LIMIT:
	    SoftTimeLimit = CLStateGetIntArg(handle, arg);
	    if(HardTimeLimit != RLIM_INFINITY)
	    {
	       if(HardTimeLimit<=SoftTimeLimit)
	       {
		  Error("Soft time limit has to be smaller than hard"
			"time limit", USAGE_ERROR);
	       }
	    }	    
	    break;
      case OPT_RUSAGE_INFO:
	    print_rusage = true;
	    break;
      case OPT_STEP_LIMIT:
	    step_limit = CLStateGetIntArg(handle, arg);
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
	    TPTPFormatPrint = true;
	    EqnFullEquationalRep = false;
	    EqnUseInfix = false;
	    break;
      case OPT_TPTP_FORMAT:
	    parse_format = TPTPFormat;	    
	    TPTPFormatPrint = true;
	    EqnFullEquationalRep = false;
	    EqnUseInfix = false;
	    break;
      case OPT_TSTP_PARSE:
	    parse_format = TSTPFormat;
	    break;
      case OPT_TSTP_PRINT:
	    OutputFormat = tstp_format;
	    break;
      case OPT_TSTP_FORMAT:
	    parse_format = TSTPFormat;
	    OutputFormat = tstp_format;
	    break;
      case OPT_NO_PREPROCESSING:
	    no_preproc = true;
	    break;
      case OPT_NO_EQ_UNFOLD:
	    no_eq_unfold = true;
	    break;
      case OPT_NO_NEG_PARAMOD:
	    ParamodOverlapIntoNegativeLiterals = false;
	    break;	    
      case OPT_NO_NONEQ_PARAMOD:
	    ParamodOverlapNonEqLiterals = false;
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
      case OPT_LITERAL_SELECT:
	    h_parms->selection_strategy = GetLitSelFun(arg);
	    if(!h_parms->selection_strategy)
	    {
	       DStr_p err = DStrAlloc();
	       DStrAppendStr(err, 
			     "Wrong argument to option -W "
			     "(--literal-selection-strategy). Possible "
			     "values: "); 
	       DStrAppendStrArray(err, LiteralSelectionFunNames, ", ");
	       Error(DStrView(err), USAGE_ERROR);
	       DStrFree(err);
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
      case OPT_SPLIT_TYPES:
	    h_parms->split_clauses = CLStateGetIntArg(handle, arg);
	    break;	    
      case OPT_SPLIT_HOW:
	    h_parms->split_method = CLStateGetIntArg(handle, arg);
	    if((h_parms->split_method < 0) ||(h_parms->split_method>2))
	    {
	       Error("Argument to option --split-method "
		     "has to be value between 0 and 2", USAGE_ERROR);
	    }
	    break;
      case OPT_SPLIT_AGGRESSIVE:
	    h_parms->split_aggressive = true;
	    break;
      case OPT_REWEIGHT_LIMIT:
	    h_parms->reweight_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_ORDERING:	    
	    if(strcmp(arg, "Auto")==0)
	    {
	       h_parms->ordertype = AUTO;
	    }
	    else if(strcmp(arg, "Auto071")==0)
	    {
	       h_parms->ordertype = AUTO071;
	    }
	    else if(strcmp(arg, "AutoDev")==0)
	    {
	       h_parms->ordertype = AUTODEV;
	    }
#ifdef SAFELOGIC
	    else if(strcmp(arg, "Auto")==0)
	    {
	       h_parms->ordertype = SL_AUTO;
	    }
#endif
	    else if(strcmp(arg, "Optimize")==0)
	    {
	       h_parms->ordertype = OPTIMIZE_AX;
	    }
	    else if(strcmp(arg, "LPO")==0)
	    {
	       h_parms->ordertype = LPO;
	    }
	    else if(strcmp(arg, "KBO")==0)
	    {
	       h_parms->ordertype = KBO;
	    }
	    else
	    {
	       Error("Option -t (--term-ordering) requires Auto, "
		     "Auto071, AutoDev, Optimize, LPO, or KBO as an "
		     "argument", 
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
      case OPT_STRONGSUBSUMPTION:
	    StrongUnitForwardSubsumption = true;
	    break;  
      case OPT_WATCHLIST:
	    watchlist_filename = arg;
	    break;
      case OPT_WATCHLIST_NO_SIMPLIFY:
	    h_parms->watchlist_simplify = false;
	    break;
      case OPT_NO_INDEXED_SUBSUMPTION:
	    fvi_parms->features = FVINoFeatures;
	    break;
      case OPT_FVINDEX_STYLE:
	    if(strcmp(arg, "None")==0)
	    {
	       fvi_parms->features = FVINoFeatures;
	    }
	    else if(strcmp(arg, "Direct")==0)
	    {
	       fvi_parms->use_perm_vectors = false;
	    }
	    else if(strcmp(arg, "Perm")==0)
	    {
	       fvi_parms->use_perm_vectors = true;
	       fvi_parms->eleminate_uninformative = false;       
	    }
	    else if(strcmp(arg, "PermOpt")==0)
	    {
	       fvi_parms->use_perm_vectors = true;
	       fvi_parms->eleminate_uninformative = true;
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
	       fvi_parms->features = FVINoFeatures;
	    }
	    else if(strcmp(arg, "AC")==0)
	    {
	       fvi_parms->features = FVIACFeatures;
	    }
	    else if(strcmp(arg, "SS")==0)
	    {
	       fvi_parms->features = FVISSFeatures;
	    }
	    else if(strcmp(arg, "All")==0)
	    {
	       fvi_parms->features = FVIAllFeatures;
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
	       Error("Argument to option --fvindex-maxdepth "
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
      case OPT_UNPROC_UNIT_SIMPL:
	    h_parms->unproc_simplify = TransUnitSimplifyString(arg);
	    if(h_parms->unproc_simplify==-1)
	    {
	       DStr_p err = DStrAlloc();
	       DStrAppendStr(err, 
			     "Wrong argument to option"
			     " --simplify-with-unprocessed-units" 
			     "Possible values: "); 
	       DStrAppendStrArray(err, UnitSimplifyNames, ", ");
	       Error(DStrView(err), USAGE_ERROR);
	       DStrFree(err);
	    }
	    break;
      case OPT_DEFINE_WFUN:
	    PStackPushP(h_parms->wfcb_definitions, arg);
	    break;
      case OPT_DEFINE_HEURISTIC:
	    PStackPushP(h_parms->hcb_definitions, arg);
	    break;
#ifdef SAFELOGIC
      case OPT_INTERPRETE_NUMBERS:
      {
	 Scanner_p in = CreateScanner(StreamTypeOptionString, arg,
				      false,NULL);
	 succ_symbol = SecureStrdup(DStrView(AktToken(in)->literal));
	 AcceptInpTok(in, SigIdentToken);
	 AcceptInpTok(in, Comma);
	 null_symbol = SecureStrdup(DStrView(AktToken(in)->literal));
	 AcceptInpTok(in, SigIdentToken);
	 CheckInpTok(in, NoToken);
	 DestroyScanner(in);
      }
      break;
#endif
      default:
	    assert(false && "Unknown option");
	    break;
      }
   }
   if((HardTimeLimit!=RLIM_INFINITY)||(SoftTimeLimit!=RLIM_INFINITY))
   {
      if(getrlimit(RLIMIT_CPU, &limit))
      {
	 TmpErrno = errno;
	 SysError("Unable to get sytem cpu time limit", SYS_ERROR);
      }
      SystemTimeLimit = limit.rlim_max;
      HardTimeLimit = MIN(HardTimeLimit, SystemTimeLimit);
      SoftTimeLimit = MIN(SoftTimeLimit, SystemTimeLimit);

      if(SoftTimeLimit!=RLIM_INFINITY)
      {
	 limit.rlim_max = SystemTimeLimit; /* Redundant, but clearer */
	 limit.rlim_cur = SoftTimeLimit;
	 TimeLimitIsSoft = true;
      }
      else
      {
	 limit.rlim_max = SystemTimeLimit;
	 limit.rlim_cur = HardTimeLimit;
	 TimeLimitIsSoft = false;
      }
      if(setrlimit(RLIMIT_CPU, &limit))
      {
	 TmpErrno = errno;
	 SysError("Unable to set cpu time limit", SYS_ERROR);
      }
      limit.rlim_max = RLIM_INFINITY;
      limit.rlim_cur = 0;

      if(setrlimit(RLIMIT_CORE, &limit))
      {
	 TmpErrno = errno;
	 SysError("Unable to prevent core dumps", SYS_ERROR);
      }
   }
   return state;
}

void print_help(FILE* out)
{
   fprintf(out, "\n\
E " VERSION " \"" NICKNAME "\"\n\
\n\
Usage: " NAME " [options] [files]\n\
\n\
Read a set of clauses and try to refute it.\n\
\n");
   PrintOptions(stdout, opts);
   fprintf(out, "\n\
Copyright 1998-2003 by Stephan Schulz, " STS_MAIL "\n\
\n\
You can find the latest version of E and additional information at\n"
E_URL
"\n\n"
#ifdef SAFELOGIC
"This version of E includes proprietary code of Safelogic A.B.,\n\
Gothenburg, Sweden, and you are bound by the licensing conditions of\n\
your contract. You may not use, redistribute or modify this version\n\
without an explicit license. You can get the free version at the URL\n\
given above.\n"
#else
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
#endif
"\n\
The original copyright holder can be contacted as\n\
\n"
STS_SNAIL
"\n");

}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


