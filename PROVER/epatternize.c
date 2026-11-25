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
#include <ccl_formulafunc.h>
#include <cle_patterns.h>
#include <cle_clauseenc.h>
#include <che_hcb.h>
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
   OPT_LOP_PARSE,
   OPT_TPTP_PARSE,
   OPT_TPTP_PRINT,
   OPT_TPTP_FORMAT,
   OPT_TSTP_PARSE,
   OPT_TSTP_PRINT,
   OPT_TSTP_FORMAT,
   OPT_SINE,
   OPT_FREE_NUMBERS,
   OPT_FREE_OBJECTS,
   OPT_DEF_CNF,
   OPT_MINISCOPE_LIMIT
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
    "Verbose comments on the progress of the program, printed to "
    "stderr."},

   {OPT_OUTPUT,
    'o', "output-file",
    ReqArg, NULL,
   "Redirect output into the named file."},

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
    "No effect (only patterns are printed)."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp-format",
    NoArg, NULL,
    "Equivalent to --tptp-in and --tptp-out."},

   {OPT_TSTP_PARSE,
    '\0', "tstp-in",
    NoArg, NULL,
    "Set TPTP-3 as the input format. TPTP-3 syntax is under ongoing "
    "development, and any given version in E may not be "
    "fully conforming at all times. E works on all TPTP 9.2.1 FOF "
    "and CNF files (including includes)."},

   {OPT_TSTP_PRINT,
    '\0', "tstp-out",
    NoArg, NULL,
    "No effect (only patterns are printed)."},

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

   {OPT_SINE,
    '\0', "sine",
    OptArg, "Auto",
    "Apply SInE to prune the unprocessed axioms with the specified"
    " filter. 'Auto' will automatically pick a filter."},

   {OPT_FREE_NUMBERS,
    '\0', "free-numbers",
     NoArg, NULL,
     "Treat numbers (strings of decimal digits) as normal free function "
    "symbols in the input. By default, numbers are supposed to denote"
    " domain constants and to be implicitly different from each other."},

   {OPT_FREE_OBJECTS,
    '\0', "free-objects",
     NoArg, NULL,
     "Treat object identifiers (strings in double quotes) as normal "
    "free function symbols in the input. By default, object identifiers "
    "represent domain objects and are implicitly different from "
    "each other (and from numbers, unless those are declared to be free)."},

   {OPT_DEF_CNF,
    '\0', "definitional-cnf",
    OptArg, TFORM_RENAME_LIMIT_STR,
    "Tune the clausification algorithm to introduce definitions for "
    "subformulae to avoid exponential blow-up. The optional argument "
    "is a fudge factor that determines when definitions are introduced. "
    "0 disables definitions completely. The default works well."},

   {OPT_MINISCOPE_LIMIT,
    '\0', "miniscope-limit",
    OptArg, TFORM_MINISCOPE_LIMIT_STR,
    "Set the maximal size (in subformulas) of a universally quantified "
    "subformula that is miniscoped to move existential quantifiers out "
    "of its scope. The built-in default is 1048576."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char *outname = NULL,
     *sine    = NULL;

IOFormat parse_format = AutoFormat;
long     miniscope_limit = DEFAULT_MINISCOPE_LIMIT,
         FormulaDefLimit = 24;
FunctionProperties free_symb_prop = FPIgnoreProps;
bool     app_encode = false; /* Required by TypeInferSort() */

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

   state = process_options(argc, argv);

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
                        true, true, true, true, fstate->step_archive);

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
// Global Variables: The option variables above, Verbose,
//                   OutputFormat, EqnFullEquationalRep, EqnUseInfix
//
// Side Effects    : Sets the option variables, may terminate with
//                   program description if option -h or --help was
//                   present
//
/----------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[])
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
Usage: " NAME " [options] [files]\n\
\n\
Read sets of clauses/formulas, perform clausification, then convert\n\
the clauses to patterns and print them.\n\
\n");
   PrintOptions(out, opts, "Options:\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
