/*-----------------------------------------------------------------------

  File  : eground.c

  Author: Stephan Schulz

  Contents

  Read a problem specification and test wether the problem has a
  finite Herbrand universe. If yes, create at least all ground
  instances of clauses necessary for a ground refutation.

  Copyright 1998-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sun May 27 23:35:28 CEST 2001

  -----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_signals.h>
#include <ccl_splitting.h>
#include <ccl_grounding.h>
#include <che_clausesetfeatures.h>
#include <che_hcb.h>
#include <ccl_formulafunc.h>
#include <e_version.h>



/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME "eground"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_PRINT_STATISTICS,
   OPT_RUSAGE_INFO,
   OPT_SUPRESS_RESULT,
   OPT_LOP_PARSE,
   OPT_TPTP_PARSE,
   OPT_TPTP_PRINT,
   OPT_TPTP_FORMAT,
   OPT_TSTP_PARSE,
   OPT_TSTP_PRINT,
   OPT_TSTP_FORMAT,
   OPT_DEF_CNF,
   OPT_MINISCOPE_LIMIT,
   OPT_DIMACS_PRINT,
   OPT_SPLIT_TRIES,
   OPT_DISABLE_UNIT_SUBSUMPTION,
   OPT_DISABLE_UNIT_RESOLUTION,
   OPT_DISABLE_TAUTOLOGY_DETECTION,
   OPT_MEM_LIMIT,
   OPT_CPU_LIMIT,
   OPT_SOFTCPU_LIMIT,
   OPT_PART_COMPLETE,
   OPT_GIVE_UP,
   OPT_CONSTRAINTS,
   OPT_LOCAL_CONSTRAINTS,
   OPT_FIX_MINISAT
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
    "Verbose comments on the progress of the program by printing "
    "technical information to stderr."},

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
    "nearly no output except for the final clauses, level 1 produces"
    " minimal additional output. Higher levels are without meaning in"
    " " NAME " (I think)."},

   {OPT_PRINT_STATISTICS,
    '\0', "print-statistics",
    NoArg, NULL,
    "Print a short statistical summary of clauses read and "
    "generated."},

   {OPT_RUSAGE_INFO,
    'R', "resources-info",
    NoArg, NULL,
    "Give some information about the resources used by the system. "
    "You will usually get CPU time information. On systems returning "
    "more information with the rusage() system call, you will also "
    "get information about memory consumption."},

   {OPT_SUPRESS_RESULT,
    '\0', "suppress-result",
    NoArg, NULL,
    "Suppress actual printing of the result, just give a short message "
    "about success. Useful mainly for test runs."},

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
    "Parse TPTP-2 format instead of E-LOP (except includes, "
    "which are handles as in TPTP-3, as TPTP-2 include syntax"
    " is considered harmful)."},

   {OPT_TPTP_PRINT,
    '\0', "tptp-out",
    NoArg, NULL,
    "Print TPTP-2 format instead of E-LOP."},

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
    "is still under development, and the version implemented may not be "
    "fully conformant at all times. It works on all TPTP 3.0.1 input "
    "files (including includes)."},

   {OPT_TSTP_PRINT,
    '\0', "tstp-out",
    NoArg, NULL,
    "Print output clauses in TPTP-3 syntax."},

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

   {OPT_DIMACS_PRINT,
    'd', "dimacs",
    NoArg, NULL,
    "Print output in the DIMACS format suitable for many propositional "
    "provers."},

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


   {OPT_SPLIT_TRIES,
    '\0' , "split-tries",
    OptArg, "1",
    "Determine the number of tries for splitting. If 0, no splitting "
    "is performed. If 1, only variable-disjoint splits are "
    "done. Otherwise, up to the desired number of variable "
    "permutations is tried to find a splitting subset."},

   {OPT_DISABLE_UNIT_SUBSUMPTION,
    'U' , "no-unit-subsumption",
    NoArg, NULL,
    "Do not check if clauses are subsumed by previously encountered "
    "unit clauses."},

   {OPT_DISABLE_UNIT_RESOLUTION,
    'r' , "no-unit-resolution",
    NoArg, NULL,
    "Do not perform forward-unit-resolution on new clauses."},

   {OPT_DISABLE_TAUTOLOGY_DETECTION,
    't' , "no-tautology-detection",
    NoArg, NULL,
    "Do not perform tautology deletion on new clauses."},

   {OPT_MEM_LIMIT,
    'm', "memory-limit",
    ReqArg, NULL,
    "Limit the memory the system may use. The argument is "
    "the allowed amount of memory in MB. This option may not work "
    "everywhere, due to broken and/or strange behaviour of setrlimit() "
    "in some UNIX implementations. It does work under all tested "
    "versions of Solaris and GNU/Linux."},

   {OPT_CPU_LIMIT,
    '\0', "cpu-limit",
    OptArg, "300",
    "Limit the cpu time the program should run. The optional argument "
    "is the CPU time in seconds. The program will terminate immediately"
    " after reaching the time limit, regardless of internal state. This"
    " option may not work "
    "everywhere, due to broken and/or strange behaviour of setrlimit() "
    "in some UNIX implementations. It does work under all tested "
    "versions of Solaris, HP-UX and GNU/Linux. As a side effect, this "
    "option will inhibit core file writing."},

   {OPT_SOFTCPU_LIMIT,
    '\0', "soft-cpu-limit",
    OptArg, "310",
    "Limit the cpu time spend in grounding. After the time expires,"
    " the prover will print an partial system."},

   {OPT_PART_COMPLETE,
    'i', "add-one-instance",
    NoArg, NULL,
    "If the grounding procedure runs out of time or memory, try to add"
    " at least one instance of each clause to the set. This might "
    "fail for  really large clause sets, since the reserve memory kept"
    " for this purpose may be insufficient. "},

   {OPT_GIVE_UP,
    'g', "give-up",
    ReqArg, NULL,
    "Give up early if the problem is unlikely to be reasonably"
    " small. If run without constraints, the program will give up if"
    " the clause with the largest number of instances will be expanded"
    " into more than this number of instances. If run with constraints,"
    " the program keeps a running count and will terminate if the"
    " estimated total"
    " number of clauses would exceed this value . A value of 0 will"
    " leave this test disabled."},

   {OPT_CONSTRAINTS,
    'c', "constraints",
    NoArg, NULL,
    "Use global purity constraints to restrict the number of "
    "instantiations done."},

   {OPT_LOCAL_CONSTRAINTS,
    'C', "local-constraints",
    NoArg, NULL,
    "Use local purity constraints to further restrict the number of "
    "instantiations done. Implies the previous option. Not yet "
    "implemented! Note to self: Split clauses need to get fresh "
    "variables if this is to work!"},

   {OPT_FIX_MINISAT,
    'M', "fix-minisat",
    NoArg, NULL,
    "Fix the preamble to include only the maximum variable index, "
    "to compensate for MiniSAT's problematic interpretation of "
    "the DIMAC syntax."},

    {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char   *outname = NULL;
IOFormat parse_format = AutoFormat;
bool   dimacs_format = false;
int    split_tries = 0;
bool   unit_sub = true,
       unit_res = true,
       taut_check = true,
       add_single_instance = false,
       constraints = false,
       local_constraints = false,
       print_statistics = false,
       print_rusage = false,
       print_result = true,
       fix_minisat = false,
       app_encode = false;
long   give_up = 0,
       miniscope_limit  = DEFAULT_MINISCOPE_LIMIT,
       initial_literals = 0,
       initial_clauses = 0;
long FormulaDefLimit = 24;

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
   TB_p            terms;
   VarBank_p       freshvars;
   TypeBank_p      type_bank;
   Sig_p           sig;
   ClauseSet_p     clauses, dummy;
   FormulaSet_p    formulas, f_ax_archive;
   GroundSet_p     groundset;
   Scanner_p       in;
   int             i;
   CLState_p       state;
   SpecFeatureCell features;
   FunCode         selected_symbol = 0;
   DefStore_p      def_store;
   PermVector_p    perm;
   //long            symbols = 100; /* Temporary fix */
   FVCollect_p     cspec;
   StrTree_p       skip_includes = NULL;

   assert(argv[0]);

#ifdef STACK_SIZE
   INCREASE_STACK_SIZE;
#endif
   InitIO(NAME);
   ESignalSetup(SIGXCPU);

   state = process_options(argc, argv);

   OpenGlobalOut(outname);

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   type_bank    = TypeBankAlloc();
   sig          = SigAlloc(type_bank);
   SigInsertInternalCodes(sig);
   terms        = TBAlloc(sig);
   def_store    = DefStoreAlloc(terms);
   clauses      = ClauseSetAlloc();
   dummy        = ClauseSetAlloc();
   formulas     = FormulaSetAlloc();
   f_ax_archive = FormulaSetAlloc();

   TBGCRegisterClauseSet(terms, clauses);
   TBGCRegisterFormulaSet(terms, formulas);
   TBGCRegisterFormulaSet(terms, f_ax_archive);

   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], true, NULL, true);
      ScannerSetFormat(in, parse_format);
      if(parse_format == AutoFormat && in->format == TSTPFormat)
      {
         OutputFormat = TSTPFormat;
      }

      /* ClauseSetParseList(in, clauses, terms); */
      FormulaAndClauseSetParse(in, formulas, dummy, terms,
                               NULL, &skip_includes);
      CheckInpTok(in, NoToken);
      DestroyScanner(in);
   }
   CLStateFree(state);
   ClauseSetFree(dummy);

   if(FormulaSetPreprocConjectures(formulas, f_ax_archive, false, false))
   {
      VERBOUT("Negated conjectures.\n");
   }
   freshvars = VarBankAlloc(type_bank);
   if(FormulaSetCNF2(formulas, f_ax_archive, clauses, terms, freshvars,
                     1048576, FormulaDefLimit,
                     true, true, true, true))
   {
      VERBOUT("CNFization done\n");
   }
   VarBankFree(freshvars);

   TBGCDeregisterFormulaSet(terms, formulas);
   FormulaSetFree(formulas);
   TBGCDeregisterFormulaSet(terms, f_ax_archive);
   FormulaSetFree(f_ax_archive);

   ClauseSetRemoveSuperfluousLiterals(clauses);

   cspec = FVCollectAlloc(FVIACFeatures,
                          0,
                          0,
                          0,
                          0, 0, 0,
                          0, 0, 0,
                          0, 0, 0,
                          0, 0, 0);
   cspec->max_symbols = FVINDEX_MAX_FEATURES_DEFAULT;

   perm = PermVectorCompute(clauses,
                            cspec,
                            false);
   def_store->def_clauses->fvindex = FVIAnchorAlloc(cspec, perm);

   SpecFeaturesCompute(&features, clauses, NULL, NULL, terms);

   if(!SpecNoEq(&features))
   {
      Warning("Recoding equational literals. Be sure to include"
              " equality axioms!\n");
      ClauseSetEqlitRecode(clauses);
   }
   if(SigFindMaxFunctionArity(sig)&&!ClauseSetIsGround(clauses))
   {
      Error("Grounding not possible: Specification is "
            "not near-propositional. There is an infinite Herbrand "
            "universe and there are non-ground clauses in the "
            "specification!", INPUT_SEMANTIC_ERROR);
   }
   if(add_single_instance)
   {
      selected_symbol = ClauseSetFindFreqSymbol(clauses, terms->sig,
                                                0, false);
   }

   initial_clauses  = clauses->members;
   initial_literals = clauses->literals;

   if(split_tries)
   {
      ClauseSet_p tmpset = ClauseSetAlloc();

      ClauseSetSplitClausesGeneral(def_store, true, clauses, tmpset, split_tries-1);

      ClauseSetFree(clauses);
      clauses = tmpset;
   }
   ClauseSetSort(clauses, ClauseCmpByLen);

   groundset = GroundSetAlloc(terms);

   if(constraints)
   {
      ClauseSetCreateConstrGroundInstances(terms, clauses,
                                           groundset,
                                           unit_sub,
                                           unit_res,
                                           taut_check,
                                           give_up,
                                           0);
   }
   else
   {
      ClauseSetCreateGroundInstances(terms, clauses,
                                     groundset,
                                     unit_sub,
                                     unit_res,
                                     taut_check,
                                     give_up);
   }
   if((groundset->complete!=cpl_complete) && add_single_instance)
   {
      GroundSetState gss_cache = groundset->complete;
      MemIsLow = false; /* Kind of optimistic, but otherwise
                           ClauseSetCreateConstrGroundInstances() will
                           fail immediately */
      ClauseSetCreateConstrGroundInstances(terms, clauses,
                                           groundset,
                                           unit_sub,
                                           unit_res,
                                           taut_check,
                                           give_up,
                                           selected_symbol);
      groundset->complete = gss_cache;
   }
   if(OutputLevel == 1)
   {
      fputc('\n', GlobalOut);
   }
   if(print_result)
   {
      if(dimacs_format)
      {
         long max_lit =  groundset->max_literal;

         if(fix_minisat)
         {
            max_lit = GroundSetMaxVar(groundset);
         }

         PrintDimacsHeader(GlobalOut, max_lit,
                           GroundSetDimacsPrintMembers(groundset));
         GroundSetPrintDimacs(GlobalOut, groundset);
      }
      else
      {
         GroundSetPrint(GlobalOut, groundset);
      }
      switch(groundset->complete)
      {
      case cpl_complete:
            fprintf(GlobalOut,
                    COMCHAR" Full and complete proof state written!\n");
            break;
      case cpl_lowmem:
            fprintf(GlobalOut,
                    COMCHAR" Out of memory: Proof state incomplete!\n");
            break;
      case cpl_timeout:
            fprintf(GlobalOut,
                    COMCHAR" Timeout: Proof state incomplete!\n");
            break;
      default:
            assert(false && "Unknown incompleteness?!?");
      }
   }
   else
   {
      fprintf(GlobalOut, COMCHAR" Success!\n");
   }
   if(print_statistics)
   {
      fprintf(GlobalOut,
              "\n"
              COMCHAR" Initial clauses                      : %ld\n"
              COMCHAR" Initial literals                     : %ld\n"
              COMCHAR" Generated clauses                    : %ld\n"
              COMCHAR" Generated literals                   : %ld\n",
              initial_clauses, initial_literals,
              GroundSetMembers(groundset),
              GroundSetLiterals(groundset));
   }
#ifndef FAST_EXIT
   GroundSetFree(groundset);
   ClauseSetFree(clauses);

   terms->sig = NULL;
   DefStoreFree(def_store);
   FVCollectFree(cspec);
   TBFree(terms);
   SigFree(sig);
   TypeBankFree(type_bank);
#endif
   if(print_rusage)
   {
      PrintRusage(GlobalOut);
   }

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
// Global Variables: opts, Verbose, TermPrologArgs,
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
   rlim_t mem_limit = 0;

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
      case OPT_SILENT:
            OutputLevel = 0;
            break;
      case OPT_OUTPUTLEVEL:
            OutputLevel = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PRINT_STATISTICS:
            print_statistics = true;
            break;
      case OPT_RUSAGE_INFO:
            print_rusage = true;
            break;
      case OPT_SUPRESS_RESULT:
            print_result = false;
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
            EqnUseInfix = false;
            break;
      case OPT_TSTP_PARSE:
            parse_format = TSTPFormat;
            break;
      case OPT_TSTP_PRINT:
            OutputFormat = TSTPFormat;
            EqnUseInfix = true;
            break;
      case OPT_TSTP_FORMAT:
            parse_format = TSTPFormat;
            OutputFormat = TSTPFormat;
            EqnUseInfix = true;
            break;
      case OPT_DIMACS_PRINT:
            dimacs_format = true;
            break;
      case OPT_DEF_CNF:
            FormulaDefLimit     = CLStateGetIntArg(handle, arg);
            break;
      case OPT_MINISCOPE_LIMIT:
            miniscope_limit =  CLStateGetIntArg(handle, arg);
            break;
      case OPT_SPLIT_TRIES:
            split_tries = CLStateGetIntArg(handle, arg);
            if((split_tries < 0))
            {
               Error("Argument to option --split-tries "
                     "has to be value greater than or equal to 0 ",
                     USAGE_ERROR);
            }
            break;
      case OPT_DISABLE_UNIT_SUBSUMPTION:
            unit_sub = false;
            break;
      case OPT_DISABLE_UNIT_RESOLUTION:
            unit_res = false;
            break;
      case OPT_DISABLE_TAUTOLOGY_DETECTION:
            taut_check = false;
            break;
      case OPT_MEM_LIMIT:
            if(strcmp(arg, "Auto")==0)
            {
               long tmpmem =  GetSystemPhysMemory();
               mem_limit = 0.8*tmpmem;

               if(tmpmem==-1)
               {
                  Error("Cannot find physical memory automatically. "
                        "Give explicit value to --memory-limit", OTHER_ERROR);
               }
               mem_limit = MEGA*mem_limit;
               VERBOSE(fprintf(stderr,
                               "Physical memory determined as %ld MB\n"
                               "Memory limit set to %lld MB\n",
                               tmpmem,
                               (long long)mem_limit););
            }
            else
            {
               mem_limit = MEGA*CLStateGetIntArg(handle, arg);
            }
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
      case OPT_PART_COMPLETE:
            add_single_instance = true;
            break;
      case OPT_GIVE_UP:
            give_up = CLStateGetIntArg(handle, arg);
            break;
      case OPT_CONSTRAINTS:
            constraints = true;
            break;
      case OPT_LOCAL_CONSTRAINTS:
            constraints = true;
            local_constraints = true;
            ClausesHaveDisjointVariables = true;
            ClausesHaveLocalVariables = false;
            break;
      case OPT_FIX_MINISAT:
            fix_minisat = true;
            break;
      default:
            assert(false);
            break;
      }
   }
   if((HardTimeLimit!=RLIM_INFINITY)||(SoftTimeLimit!=RLIM_INFINITY))
   {
      if(getrlimit(RLIMIT_CPU, &limit))
      {
         TmpErrno = errno;
         SysError("Unable to get system cpu time limit", SYS_ERROR);
      }
      SystemTimeLimit = limit.rlim_max;
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
   SetMemoryLimit(mem_limit);
   return state;
}


void print_help(FILE* out)
{
   fprintf(out, "\n\
\n\
"NAME " " VERSION "\n\
\n\
Usage: eground [options] [files]\n\
\n\
Read a set of clauses and determine if it can be grounded (i.e. is\n\
either already ground or has no non-constant function symbols). If\n\
this is the case, print sufficiently many ground instances of the\n\
clauses to guarantee that a ground refutation can be found for\n\
unsatisfiable clause sets.\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        Endsudo of File                                  */
/*---------------------------------------------------------------------*/
