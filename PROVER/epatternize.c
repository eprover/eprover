/*-----------------------------------------------------------------------

  File  : epatternize.c

  Author: Stephan Schulz

  Contents

  Read a logic file and convert it to pattern form.

  Copyright 2020 by the author.

  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jun 13 00:41:31 CEST 2020

  -----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <ccl_unfold_defs.h>
#include <ccl_formulafunc.h>
#include <cle_patterns.h>
#include <cle_clauseenc.h>
#include <che_clausesetfeatures.h>
#include <che_hcb.h>
#include <che_specsigfeatures.h>
#include <che_rawspecfeatures.h>
#include <cco_sine.h>
#include <e_version.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME "epatternize"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_PARSE_FEATURES,
   OPT_LOP_PARSE,
   OPT_TPTP_PARSE,
   OPT_TPTP_PRINT,
   OPT_TPTP_FORMAT,
   OPT_TSTP_PARSE,
   OPT_TSTP_PRINT,
   OPT_TSTP_FORMAT,
   OPT_RAW_CLASS,
   OPT_SPECSIG_FTRS,
   OPT_GEN_TPTP_HEADER,
   OPT_NO_PREPROCESSING,
   OPT_EQ_UNFOLD_LIMIT,
   OPT_EQ_UNFOLD_MAXCLAUSES,
   OPT_NO_EQ_UNFOLD,
   OPT_SINE,
   OPT_FREE_NUMBERS,
   OPT_FREE_OBJECTS,
   OPT_DEF_CNF_OLD,
   OPT_DEF_CNF,
   OPT_MINISCOPE_LIMIT,
   OPT_MASK,
   OPT_RAW_MASK,
   OPT_NGU_ABSOLUTE,
   OPT_NGU_FEW_LIMIT,
   OPT_NGU_MANY_LIMIT,
   OPT_NGU_FEW_COUNT,
   OPT_NGU_MANY_COUNT,
   OPT_GPC_ABSOLUTE,
   OPT_GPC_FEW_LIMIT,
   OPT_GPC_MANY_LIMIT,
   OPT_GPC_FEW_COUNT,
   OPT_GPC_MANY_COUNT,
   OPT_AXIOM_MANY_LIMIT,
   OPT_AXIOM_SOME_LIMIT,
   OPT_LIT_MANY_LIMIT,
   OPT_LIT_SOME_LIMIT,
   OPT_TERM_MEDIUM_LIMIT,
   OPT_TERM_LARGE_LIMIT,
   OPT_FAR_SUM_MEDIUM_LIMIT,
   OPT_FAR_SUM_LARGE_LIMIT,
   OPT_MAX_DEPTH_MEDIUM_LIMIT,
   OPT_MAX_DEPTH_DEEP_LIMIT,
   OPT_SIG_MEDIUM_LIMIT,
   OPT_SIG_LARGE_LIMIT,
   OPT_PREDC_MEDIUM_LIMIT,
   OPT_PREDC_LARGE_LIMIT,
   OPT_PRED_MEDIUM_LIMIT,
   OPT_PRED_LARGE_LIMIT,
   OPT_FUNC_MEDIUM_LIMIT,
   OPT_FUNC_LARGE_LIMIT,
   OPT_FUN_MEDIUM_LIMIT,
   OPT_FUN_LARGE_LIMIT
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
    '\0', "version",
    NoArg, NULL,
    "Print the version number of the program."},

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

   {OPT_PARSE_FEATURES,
    'p', "parse-features",
    NoArg, NULL,
    "Parse precomputed feature lines, not real formulae. This "
    "conflicts with the '--generate-tptp-header' option, as not "
    "all information needed for this is stored in feature lines."},

   {OPT_LOP_PARSE,
    '\0', "lop-in",
    NoArg, NULL,
    "Set E-LOP as the input format. If no input format is "
    "selected by this or one of the following options, E will "
    "guess the input format based on the first token. It will "
    "almost always correctly recognize TPTP-3, but it may "
    "misidentify E-LOP files that use TPTP meta-identifiers as "
    "logical symbols."},

   {OPT_TPTP_PARSE,
    '\0', "tptp-in",
    NoArg, NULL,
    "Set TPTP-2 as the input format (but note that includes are "
    "still handled according to TPTP-3 semantics)."},

   {OPT_TPTP_PRINT,
    '\0', "tptp-out",
    NoArg, NULL,
    "No effect (since not clauses/formulas are printed)."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp-format",
    NoArg, NULL,
    "Equivalent to --tptp-in and --tptp-out."},

   {OPT_TSTP_PARSE,
    '\0', "tstp-in",
    NoArg, NULL,
    "Parse TSTP format instead of E-LOP (not all all optional "
    "extensions are currently supported)."},

   {OPT_TSTP_PRINT,
    '\0', "tstp-out",
    NoArg, NULL,
    "No effect (since not clauses/formulas are printed)."},

   {OPT_TSTP_FORMAT,
    '\0', "tstp-format",
    NoArg, NULL,
    "Equivalent to --tstp-in and --tstp-out."},

   {OPT_TSTP_PARSE,
    '\0', "tptp3-in",
    NoArg, NULL,
    "Equivalent to --tstp-in."},

   {OPT_TSTP_PRINT,
    '\0', "tptp3-out",
    NoArg, NULL,
    "Equivalent to --tstp-out."},

   {OPT_TSTP_FORMAT,
    '\0', "tptp3-format",
    NoArg, NULL,
    "Equivalent to --tstp-format."},

   {OPT_RAW_CLASS,
    'r', "raw-class",
    NoArg, NULL,
    "Perform a raw and rough classification on the unclausified and "
    "unpreprocessed problem. This is a largely independent feature "
    "put here to reduce the proliferation of partially redundant "
    "programs. Note that many of the limits do not apply here."},

   {OPT_SPECSIG_FTRS,
    '\0', "specsig",
    NoArg, NULL,
    "Compute and print new-style features based on the distribution "
    "of symbols of differnt arities."},

   {OPT_GEN_TPTP_HEADER,
    'H', "generate-tptp-header",
    NoArg, NULL,
    "Generate the statistics (\"Syntax\") part of a TPTP header for "
    "the problem."},

   {OPT_NO_PREPROCESSING,
    '\0', "no-preprocessing",
    NoArg, NULL,
    "Do not perform preprocessing on the initial clause set. "
    "Preprocessing currently removes tautologies and orders terms, "
    "literals and clauses in a certain (\"canonical\") way before "
    "anything else happens. It also unfolds equational definitons (and "
    "removes them)."},

   {OPT_EQ_UNFOLD_LIMIT,
    '\0', "eq-unfold-limit",
    ReqArg, NULL,
    "During preprocessing, limit unfolding (and removing) of "
    "equational definitions to those where the expanded definiton "
    "is at most the given limit bigger (in terms of standard "
    "weight) than the defined term.."},

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

   {OPT_MINISCOPE_LIMIT,
    '\0', "miniscope-limit",
    OptArg, TFORM_MINISCOPE_LIMIT_STR,
    "Set the limit of variables to miniscope per input formula. The build-in "
    "default is 1000. Only applies to the new (default) clausification "
    "algorithm"},

   {OPT_MASK,
    'c', "class-mask",
    ReqArg, NULL,
    "Provide a mask for the class description. A mask is a 13 letter "
    "string, with positions corresponding to the class "
    "descriptors. Any dash ('-') in the string masks out the "
    "corresponding position in the class descriptor."},

   {OPT_RAW_MASK,
    '\0', "raw-mask",
    ReqArg, NULL,
    "Provide a mask for the rawclass description. A mask is a 7 letter "
    "string, with positions corresponding to the class "
    "descriptors. Any dash ('-') in the string masks out the "
    "corresponding position in the class descriptor."},

   {OPT_NGU_ABSOLUTE,
    'a', "ngu-absolute",
    OptArg, "true",
    "Use absolute values (not percentages) to determine if there are few, "
    "some, or many non-ground unit clauses."},

   {OPT_NGU_FEW_LIMIT,
    'f', "ngu-few-limit",
    ReqArg, NULL,
    "Set the limit (either an absolute integer value or a fraction "
    "between 0 and 1) for the size of the set of non-ground units "
    "to consist of 'few' clauses."},

   {OPT_NGU_MANY_LIMIT,
    'm', "ngu-many-limit",
    ReqArg, NULL,
    "Set the limit (either an absolute integer value or a fraction "
    "between 0 and 1) for the size of the set of non-ground units "
    "to consist of 'many' clauses."},

   {OPT_GPC_ABSOLUTE,
    '\0', "gpc-absolute",
    OptArg, "true",
    "Use absolute values (not percentages) to determine if there are few, "
    "some, or many non-ground unit clauses."},

   {OPT_GPC_FEW_LIMIT,
    '\0', "gpc-few-limit",
    ReqArg, NULL,
    "Set the limit (either an absolute integer value or a fraction "
    "between 0 and 1) for the size of the set of ground positive clauses "
    "to consist of 'few' clauses."},

   {OPT_GPC_MANY_LIMIT,
    '\0', "gpc-many-limit",
    ReqArg, NULL,
    "Set the limit (either an absolute integer value or a fraction "
    "between 0 and 1) for the size of the set of ground positive clauses "
    "to consist of 'many' clauses."},

   {OPT_AXIOM_SOME_LIMIT,
    '\0', "ax-some-limit",
    ReqArg, NULL,
    "Set the minimum number of clauses for a specification to be "
    "considered to be medium size with respect to this measure."},

   {OPT_AXIOM_MANY_LIMIT,
    '\0', "ax-many-limit",
    ReqArg, NULL,
    "Set the minimum number of clauses for a specification to be "
    "considered to be large size with respect to this measure."},

   {OPT_LIT_SOME_LIMIT,
    '\0', "lit-some-limit",
    ReqArg, NULL,
    "Set the mimumum number of literals for a specification to be "
    "considered to be medium size with respect to this measure."},

   {OPT_LIT_MANY_LIMIT,
    '\0', "lit-many-limit",
    ReqArg, NULL,
    "Set the minimum number of literals for a specification to be "
    "considered to be large size with respect to this measure."},

   {OPT_TERM_MEDIUM_LIMIT,
    '\0', "term-medium-limit",
    ReqArg, NULL,
    "Set the minimum number of subterms for a specification to be "
    "considered to be medium size with respect to this measure."},

   {OPT_TERM_LARGE_LIMIT,
    '\0', "term-large-limit",
    ReqArg, NULL,
    "Set the minimum number of subterms for a specification to be "
    "considered to be large size with respect to this measure."},

    {OPT_FAR_SUM_MEDIUM_LIMIT,
    '\0', "farity-medium-limit",
    ReqArg, NULL,
    "Set the minimum sum of function symbol arities for a specification to be "
    "considered to be medium size with respect to this measure."},

   {OPT_FAR_SUM_LARGE_LIMIT,
    '\0', "farity-large-limit",
    ReqArg, NULL,
    "Set the minimum sum of function symbol arities for a specification to be "
    "considered to be large size with respect to this measure."},

   {OPT_MAX_DEPTH_MEDIUM_LIMIT,
    '\0', "max-depth-medium-limit",
    ReqArg, NULL,
    "Set the minimum maximal clause depth for medium depth specifications."},

   {OPT_MAX_DEPTH_DEEP_LIMIT,
    '\0', "max-depth-deep-limit",
    ReqArg, NULL,
    "Set the minimum maximal clause depth for deep depth specifications."},

   {OPT_SIG_MEDIUM_LIMIT,
    '\0', "sig-medium-limit",
    ReqArg, NULL,
    "Set the minimum signature size for medium sized signatures."},

   {OPT_SIG_LARGE_LIMIT,
    '\0', "sig-large-limit",
    ReqArg, NULL,
    "Set the minimum signature size for large signatures."},

   {OPT_PREDC_MEDIUM_LIMIT,
    '\0', "pred-const-medium-limit",
    ReqArg, NULL,
    "Set the minimum number of constant predicate symbols for medium size "
    "by this measure."},

   {OPT_PREDC_LARGE_LIMIT,
    '\0', "pred-const-large-limit",
    ReqArg, NULL,
    "Set the minimum number of constant predicate symbols for large size "
    "by this measure."},

   {OPT_PRED_MEDIUM_LIMIT,
    '\0', "pred-medium-limit",
    ReqArg, NULL,
    "Set the minimum number of non-constant predicate symbols for medium size "
    "by this measure."},

   {OPT_PRED_LARGE_LIMIT,
    '\0', "pred-large-limit",
    ReqArg, NULL,
    "Set the minimum number of non-constant predicate symbols for large size "
    "by this measure."},

   {OPT_FUNC_MEDIUM_LIMIT,
    '\0', "fun-const-medium-limit",
    ReqArg, NULL,
    "Set the minimum number of constant function symbols for medium size "
    "by this measure."},

   {OPT_FUNC_LARGE_LIMIT,
    '\0', "fun-const-large-limit",
    ReqArg, NULL,
    "Set the minimum number of constant function symbols for large size "
    "by this measure."},

   {OPT_FUN_MEDIUM_LIMIT,
    '\0', "fun-medium-limit",
    ReqArg, NULL,
    "Set the minimum number of non-constant function symbols for medium size "
    "by this measure."},

   {OPT_FUN_LARGE_LIMIT,
    '\0', "fun-large-limit",
    ReqArg, NULL,
    "Set the minimum number of non-constant function symbols for large size "
    "by this measure."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char *outname = NULL,
     *mask = "aaaaa----aaaa",
     *raw_mask = "aaaaaaa";

IOFormat parse_format     = AutoFormat;
bool tptp_header      = false,
   raw_classify     = false,
   specsig_classify = false,
   no_preproc       = false,
   parse_features   = false,
   app_encode       = false;
long eqdef_maxclauses = DEFAULT_EQDEF_MAXCLAUSES,
   miniscope_limit  = DEFAULT_MINISCOPE_LIMIT,
   FormulaDefLimit = 24;
long eqdef_incrlimit  = DEFAULT_EQDEF_INCRLIMIT;
FunctionProperties free_symb_prop = FPIgnoreProps;char *sine = NULL;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[], SpecLimits_p limits);
char*     parse_feature_line(Scanner_p in, SpecFeature_p features);
void print_help(FILE* out);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: main()
//
//   The main function and entry point of the program.
//
// Global Variables: Plenty, most simple flags used read-only
//
// Side Effects    : Does everything...
//
/----------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
   ProofState_p    fstate;
   Scanner_p       in;
   int             i;
   CLState_p       state;
   SpecLimits_p    limits;
   StrTree_p       skip_includes = NULL;
   PatternSubst_p  pat_subst;
   Clause_p        clause;
   PStack_p        listrep;
   Term_p          clauserep;


   assert(argv[0]);
#ifdef STACK_SIZE
   INCREASE_STACK_SIZE;
#endif

   InitIO(NAME);

   limits = SpecLimitsAlloc();
   state = process_options(argc, argv, limits);

   OpenGlobalOut(outname);

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   for(i=0; state->argv[i]; i++)
   {
      fstate    = ProofStateAlloc(free_symb_prop);
      pat_subst = PatternDefaultSubstAlloc(fstate->terms->sig);
      in        = CreateScanner(StreamTypeFile, state->argv[i], true, NULL, true);
      ScannerSetFormat(in, parse_format);

      FormulaAndClauseSetParse(in, fstate->f_axioms, fstate->watchlist,
                               fstate->terms, NULL, &skip_includes);
      ProofStateSinE(fstate, sine);

      {
         FormulaSetPreprocConjectures(fstate->f_axioms, fstate->f_ax_archive,
                                      false, false);
         FormulaSetCNF2(fstate->f_axioms,
                        fstate->f_ax_archive,
                        fstate->axioms,
                        fstate->terms,
                        fstate->freshvars,
                        miniscope_limit, FormulaDefLimit,
                        true, true, true, true);

         for(clause = fstate->axioms->anchor->succ;
             clause != fstate->axioms->anchor;
             clause = clause->succ)
         {
            PatternSubstBacktrack(pat_subst, 0);
            listrep = PStackAlloc();
            if(PatternClauseCompute(clause, &pat_subst, &listrep))
            {
               // ClauseTSTPPrint(GlobalOut, clause, true, true);
               // fprintf(GlobalOut, "\n");
               clauserep = FlatEncodeClauseListRep(fstate->terms, listrep);
               PatternTermPrint(GlobalOut, pat_subst, clauserep, fstate->terms->sig);
               fprintf(GlobalOut, "\n");
            }
            PStackFree(listrep);
         }

         DestroyScanner(in);
         PatternSubstFree(pat_subst);
         ProofStateFree(fstate);
      }
   }
   CLStateFree(state);
   SpecLimitsCellFree(limits);

   fflush(GlobalOut);
   OutClose(GlobalOut);
   ExitIO();

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
// Global Variables: Verbose, TermPrologArgs,
//
// Side Effects    : Sets variables in limits, may terminate with
//                   program description if option -h or --help was
//                   present
//
/----------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[], SpecLimits_p limits)
{
   Opt_p handle;
   CLState_p state;
   char*  arg;

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
      case OPT_VERSION:
            printf(NAME" " VERSION "\n");
            exit(NO_ERROR);
      case OPT_PARSE_FEATURES:
            parse_features = true;
            break;
      case OPT_OUTPUT:
            outname = arg;
            break;
      case OPT_LOP_PARSE:
            parse_format = LOPFormat;
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
            break;
      case OPT_TSTP_PARSE:
            parse_format = TSTPFormat;
            break;
      case OPT_TSTP_PRINT:
            OutputFormat = TSTPFormat;
            EqnFullEquationalRep = false;
            EqnUseInfix = false;
            break;
      case OPT_TSTP_FORMAT:
            parse_format = TSTPFormat;
            OutputFormat = TSTPFormat;
            EqnFullEquationalRep = false;
            break;
      case OPT_RAW_CLASS:
            raw_classify = true;
            break;
      case OPT_SPECSIG_FTRS:
            specsig_classify = true;
            break;
      case OPT_GEN_TPTP_HEADER:
            tptp_header = true;
            break;
      case OPT_NO_PREPROCESSING:
            no_preproc = true;
            break;
      case OPT_EQ_UNFOLD_MAXCLAUSES:
            eqdef_maxclauses = CLStateGetIntArg(handle, arg);
            break;
      case OPT_EQ_UNFOLD_LIMIT:
            eqdef_incrlimit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_NO_EQ_UNFOLD:
            eqdef_incrlimit = LONG_MIN;
            break;
      case OPT_SINE:
            sine = arg;
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
      case OPT_MINISCOPE_LIMIT:
            miniscope_limit =  CLStateGetIntArg(handle, arg);
            break;
      case OPT_MASK:
       mask = arg;
       if(strlen(mask)!=13)
       {
          Error("Option -c (--class-mask) requires 13-letter "
                "string as an argument", USAGE_ERROR);
       }
       break;
      case OPT_RAW_MASK:
            raw_mask = arg;
            if(strlen(raw_mask)!=7)
            {
               Error("Option -c (--class-mask) requires 7-letter "
                     "string as an argument", USAGE_ERROR);
            }
            break;
      case OPT_NGU_ABSOLUTE:
            limits->ngu_absolute = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_NGU_FEW_LIMIT:
            limits->ngu_few_limit = CLStateGetFloatArg(handle, arg);
            break;
      case OPT_NGU_MANY_LIMIT:
            limits->ngu_many_limit = CLStateGetFloatArg(handle, arg);
            break;
      case OPT_GPC_ABSOLUTE:
            limits->gpc_absolute = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_GPC_FEW_LIMIT:
            limits->gpc_few_limit = CLStateGetFloatArg(handle, arg);
            break;
      case OPT_GPC_MANY_LIMIT:
            limits->gpc_many_limit = CLStateGetFloatArg(handle, arg);
            break;
      case OPT_AXIOM_SOME_LIMIT:
            limits->ax_some_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_AXIOM_MANY_LIMIT:
            limits->ax_many_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_LIT_SOME_LIMIT:
            limits->lit_some_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_LIT_MANY_LIMIT:
            limits->lit_many_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TERM_MEDIUM_LIMIT:
            limits->term_medium_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TERM_LARGE_LIMIT:
            limits->term_large_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_FAR_SUM_MEDIUM_LIMIT:
            limits->far_sum_medium_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_FAR_SUM_LARGE_LIMIT:
            limits->far_sum_large_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_MAX_DEPTH_MEDIUM_LIMIT:
            limits->depth_medium_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_MAX_DEPTH_DEEP_LIMIT:
            limits->depth_deep_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_SIG_MEDIUM_LIMIT:
            limits->symbols_medium_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_SIG_LARGE_LIMIT:
            limits->symbols_large_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PREDC_MEDIUM_LIMIT:
            limits->predc_medium_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PREDC_LARGE_LIMIT:
            limits->predc_large_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PRED_MEDIUM_LIMIT:
            limits->pred_medium_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PRED_LARGE_LIMIT:
            limits->pred_large_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_FUNC_MEDIUM_LIMIT:
            limits->func_medium_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_FUNC_LARGE_LIMIT:
            limits->func_large_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_FUN_MEDIUM_LIMIT:
            limits->fun_medium_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_FUN_LARGE_LIMIT:
            limits->fun_large_limit = CLStateGetIntArg(handle, arg);
            break;
      default:
            assert(false);
            break;
      }
   }
   return state;
}


/*-----------------------------------------------------------------------
//
// Function: print_help()
//
//   Print the help text.
//
// Global Variables: -
//
// Side Effects    : Output.
//
/----------------------------------------------------------------------*/

void print_help(FILE* out)
{
   fprintf(out, "\n\
\n\
" NAME " " VERSION "\n\
\n\
Usage: classify_problem [options] [files]\n\
\n\
Read sets of clauses/formulas, perform cnfization, then convert \n\
the clauses to patterns and print them.\n\
\n");
   PrintOptions(stdout, opts, "Options:\n\n");
   fprintf(out, "\n\
Copyright (C) 1998-2009 by Stephan Schulz, " STS_MAIL "\n\
\n\
This program is a part of the support structure for the E equational\n\
theorem prover. You can find the latest version of the E distribution\n\
as well as additional information at\n"
E_URL
"\n\
This program is free software; you can redistribute it and/or modify\n\
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
Boston, MA  02111-1307 USA\n\
\n\
The original copyright holder can be contacted as\n\
\n"
STS_SNAIL
"\n");

}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
