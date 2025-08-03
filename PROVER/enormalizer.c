/*-----------------------------------------------------------------------

  File  : enormalizer.c

  Author: Stephan Schulz

  Contents

  Read a set of unit clauses (and/or formulas) and a set of
  terms/clauses/formulas. The unit clauses/formulas are interpreted as
  rewrite rules. The terms are normalized using these rewrite
  rules. If the rule system is not confluent, the results are
  deterministic but unspecified. If the rule system is not
  terminating, rewriting might get stuck into an infinite loop.

  Copyright 2013-2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Feb  4 23:21:45 CET 2013

-----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_signals.h>
#include <ccl_rewrite.h>
#include <ccl_formulafunc.h>
#include <e_version.h>



/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME "enormalizer"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_TERMSOURCE,
   OPT_CLAUSESOURCE,
   OPT_FORMULASOURCE,
   OPT_OUTPUT,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_PRINT_STATISTICS,
   OPT_RUSAGE_INFO,
   OPT_LOP_PARSE,
   OPT_TPTP_PARSE,
   OPT_TPTP_PRINT,
   OPT_TPTP_FORMAT,
   OPT_TSTP_PARSE,
   OPT_TSTP_PRINT,
   OPT_TSTP_FORMAT,
   OPT_MEM_LIMIT,
   OPT_CPU_LIMIT,
   OPT_SOFTCPU_LIMIT
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

   {OPT_TERMSOURCE,
    't', "terms",
    ReqArg, NULL,
    "Name of the files containing terms to be normalized. If '-' "
    "is used as the argument, terms are read from standard input."},

   {OPT_CLAUSESOURCE,
    'c', "clauses",
    ReqArg, NULL,
    "Name of the files containing clauses to be normalized. If '-' "
    "is used as the argument, clauses are read from standard input."},

   {OPT_FORMULASOURCE,
    'f', "formulas",
    ReqArg, NULL,
    "Name of the files containing fomulas to be normalized. If '-' "
    "is used as the argument, formulas are read from standard input. "
    "Note that formula-syntax is not supported in LOP syntax, but "
    "requires --tptp2-format or --tptp3-format"},

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

    {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char *outname = NULL,
     *termname = NULL,
     *clausename = NULL,
     *formulaname = NULL;
IOFormat parse_format = AutoFormat;
bool   print_statistics = false,
       print_rusage = false,
       print_result = true,
       app_encode   = false;
long   give_up = 0,
       initial_literals = 0,
       initial_clauses = 0;

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
// Function: build_rw_system()
//
//   Extract all positive unit clauses from spec, mark them as
//   oriented in the natural direction (left to right), and insert
//   them into demods. Free all other clauses and print a warning.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long build_rw_system(ClauseSet_p demods, ClauseSet_p spec)
{
   long count=0;
   Clause_p handle;

   while((handle = ClauseSetExtractFirst(spec)))
   {
      if(ClauseIsDemodulator(handle))
      {
         SysDateInc(&(demods->date));
         handle->date = demods->date;
         EqnSetProp(handle->literals, EPIsOriented);
         ClauseSetPDTIndexedInsert(demods, handle);
         count++;
      }
      else
      {
         fprintf(stderr, "%s: Clause is not a rewrite rule: ", NAME);
         ClausePrint(stderr, handle, true);
         fprintf(stderr, " -- ignoring\n");
         ClauseFree(handle);
      }
   }
   return count;
}

/*-----------------------------------------------------------------------
//
// Function: process_terms()
//
//   Open infile, read terms, compute and print their normal forms.
//
// Global Variables: parse_format, GlobalOut
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void process_terms(char* infile, TB_p terms, OCB_p ocb, ClauseSet_p *demodulators)
{
   Term_p t, tp;
   Scanner_p in;

   if(infile)
   {
      in = CreateScanner(StreamTypeFile, infile, true, NULL, true);
      ScannerSetFormat(in, parse_format);
      while(!TestInpTok(in, NoToken))
      {
         t  = TBTermParse(in, terms);
         tp = TermComputeLINormalform(ocb, terms, t,
                                      demodulators,
                                      1, false, false, false);
         TBPrintTermFull(GlobalOut, terms, t);
         fprintf(GlobalOut, " ==> ");
         TBPrintTermFull(GlobalOut, terms, tp);
         fprintf(GlobalOut, "\n");
      }
      DestroyScanner(in);
   }
}

/*-----------------------------------------------------------------------
//
// Function: process_clauses()
//
//   Open infile, read clauses, and compute and print their normal
//   forms.
//
// Global Variables: parse_format, GlobalOut
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void process_clauses(char* infile, TB_p terms, OCB_p ocb, ClauseSet_p *demodulators)
{
   Clause_p clause;
   Scanner_p in;

   if(infile)
   {
      in = CreateScanner(StreamTypeFile, infile, true, NULL, true);
      ScannerSetFormat(in, parse_format);
      while(!TestInpTok(in, NoToken))
      {
         clause  = ClauseParse(in, terms);
         ClausePrint(GlobalOut, clause, true);
         ClauseComputeLINormalform(ocb, terms, clause,
                                      demodulators,
                                      1, false, false);
         fprintf(GlobalOut, " ==> ");
         ClausePrint(GlobalOut, clause, true);
         fprintf(GlobalOut, "\n");
         ClauseFree(clause);
      }
      DestroyScanner(in);
   }
}

/*-----------------------------------------------------------------------
//
// Function: process_formulas()
//
//   Open infile, read formulas, and compute and print their normal
//   forms.
//
// Global Variables: parse_format, GlobalOut
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void process_formulas(char* infile, TB_p terms, OCB_p ocb, ClauseSet_p *demodulators)
{
   WFormula_p form;
   Scanner_p in;

   if(infile)
   {
      in = CreateScanner(StreamTypeFile, infile, true, NULL, true);
      ScannerSetFormat(in, parse_format);
      while(!TestInpTok(in, NoToken))
      {
         form = WFormulaParse(in, terms);
         WFormulaPrint(GlobalOut, form, true);

         form->tformula = TermComputeLINormalform(ocb, terms, form->tformula,
                                                  demodulators,
                                                  1, false, false, false);
         fprintf(GlobalOut, " ==> ");
         WFormulaPrint(GlobalOut, form, true);
         fprintf(GlobalOut, "\n");
         WFormulaFree(form);
      }
      DestroyScanner(in);
   }
}


/*-----------------------------------------------------------------------
//
// Function: main()
//
//   Entry point of the program and driver of the processing.
//
// Global Variables: All declared in this file
//
// Side Effects    : Yes ;-)
//
/----------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
   TB_p            terms;
   VarBank_p       freshvars;
   TypeBank_p      typebank;
   Sig_p           sig;
   ClauseSet_p     clauses, dummy;
   FormulaSet_p    formulas, f_ax_archive;
   Scanner_p       in;
   int             i;
   CLState_p       state;
   StrTree_p       skip_includes = NULL;
   ClauseSet_p     demodulators[1];
   OCB_p           ocb;

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

   typebank     = TypeBankAlloc();
   sig          = SigAlloc(typebank);
   SigInsertInternalCodes(sig);
   terms        = TBAlloc(sig);
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
      /* ClauseSetParseList(in, clauses, terms); */
      FormulaAndClauseSetParse(in, formulas, dummy, terms,
                               NULL, &skip_includes);
      CheckInpTok(in, NoToken);
      DestroyScanner(in);
   }
   ClauseSetFree(dummy);
   CLStateFree(state);

   if(FormulaSetPreprocConjectures(formulas, f_ax_archive, false, false))
   {
      VERBOUT("Negated conjectures.\n");
   }
   freshvars = VarBankAlloc(typebank);
   if(FormulaSetCNF2(formulas, f_ax_archive,
                     clauses, terms, freshvars,
                     1000, 24, true, true, true, true))
   {
      VERBOUT("CNFization done\n");
   }
   VarBankFree(freshvars);

   GCDeregisterFormulaSet(terms->gc, formulas);
   FormulaSetFree(formulas);
   GCDeregisterFormulaSet(terms->gc, f_ax_archive);
   FormulaSetFree(f_ax_archive);

   demodulators[0] = ClauseSetAlloc();
   demodulators[0]->demod_index = PDTreeAlloc(terms);
   TBGCRegisterClauseSet(terms, demodulators[0]);

   build_rw_system(demodulators[0], clauses);


   GCDeregisterClauseSet(terms->gc, clauses);
   ClauseSetFree(clauses);

   VERBOUT(COMCHAR" Demodulators\n");
   VERBOSE(ClauseSetPrint(stderr, demodulators[0], true););

   ocb = OCBAlloc(EMPTY, false, terms->sig, LFHO_ORDER);

   process_terms(termname, terms, ocb, demodulators);
   process_clauses(clausename, terms, ocb, demodulators);
   process_formulas(formulaname, terms, ocb, demodulators);

   OCBFree(ocb);

#ifndef FAST_EXIT
   ClauseSetFree(demodulators[0]);

   terms->sig = NULL;
   TBFree(terms);
   SigFree(sig);
   TypeBankFree(typebank);
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
      case OPT_TERMSOURCE:
            termname = arg;
            break;
      case OPT_CLAUSESOURCE:
            clausename = arg;
            break;
      case OPT_FORMULASOURCE:
            formulaname = arg;
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
"NAME " " VERSION "\n\
\n\
Usage: enormalizer [options] [rulefiles]\n\
\n\
Read a set of rewrite rules (in the form of unit clauses and/or\n\
formulas) with a single positive literal) and sets of terms, clauses,\n\
and/or formulas (the \"normalization targets\", from files specified\n\
with the proper options - see below) to rewrite. Rewrite rules are read\n\
from the left to right as specified in the input, without regard to any\n\
term ordering.\n\
\n\
The normalization targets are rewritten using these rewrite rules until\n\
a normal form is reached. If the rule system is not confluent, the\n\
results are deterministic but unspecified. If the rule system is not\n\
terminating, rewriting might get stuck into an infinite loop. \n\
\n\
The rewrite strategy is leftmost-innermost. The order of rewrite rules\n\
tried at each subterm is deterministic, but unspecified and\n\
independent of input order (it depends on the order in which rules are\n\
returned from the perfect discrimination tree index).\n\
\n\
The normalized terms/clauses/formulas are printed.\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
