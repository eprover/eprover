/*-----------------------------------------------------------------------

File  : e_axfilter.c

Author: Stephan Schulz

Contents

Parse a problem specification and a filter setup, and produce output
files corresponding to each filter.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 21 13:24:04 CET 2011
    New (but borrowing from LTB runner)

-----------------------------------------------------------------------*/

#include <clb_defines.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <ccl_relevance.h>
#include <cio_signals.h>
#include <ccl_formulafunc.h>
#include <cco_batch_spec.h>
#include <ccl_sine.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME         "e_axfilter"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_FILTER,
   OPT_SEED_SYMBOLS,
   OPT_SEEDS,
   OPT_SEED_SUBSAMPLE,
   OPT_SEED_METHODS,
   OPT_DUMP_FILTER,
   OPT_PRINT_STATISTICS,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_LOP_PARSE,
   OPT_LOP_FORMAT,
   OPT_TPTP_PARSE,
   OPT_TPTP_FORMAT,
   OPT_TSTP_PARSE,
   OPT_TSTP_FORMAT,
   OPT_DUMMY
}OptionCodes;

typedef enum
{
   SubSNone,
   SubSMost,
   SubSLeast,
   SubSRand
}SubSampleMethod;



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
    "Verbose comments on the progress of the program. This technical "
    "information is printed to stderr."},

   {OPT_OUTPUT,
    'o', "output-file",
    ReqArg, NULL,
    "Redirect output into the named file (this affects only some "
    "output, as most is written to automatically generated files "
    "based on the input and filter names."},

   {OPT_SILENT,
    's', "silent",
    NoArg, NULL,
    "Equivalent to --output-level=0."},

   {OPT_OUTPUTLEVEL,
    'l', "output-level",
    ReqArg, NULL,
    "Select an output level, greater values imply more verbose "
    "output."},

   {OPT_FILTER,
    'f', "filter",
    ReqArg, NULL,
    "Specify the filter definition file. If not set, the system "
    "will uses the built-in default."},

   {OPT_SEED_SYMBOLS,
    'S', "seed-symbols",
    OptArg, "p",
    "Enable artificial seeding of the axiom selection process and determine "
    "which symbol classes should be used to generate different sets."
    "The argument is a string of letters, each indicating one class of "
    "symbols to use. 'p' indicates predicate symbols, 'f' non-constant "
    "function symbols, and 'c' constants. Note that this will create "
    "potentially multiple output files for each activated symbols."},

   {OPT_SEEDS,
    '\0', "seeds",
    ReqArg, NULL,
    "Explicitly specify the symbols that should be used as seed symbols for "
    "axiom extraction. This overwrites --seed-subsample and --seed-symbols."},

   {OPT_SEED_SUBSAMPLE,
    '\0', "seed-subsample",
    OptArg, "r1000",
    "Subsample from the set of eligible seed symbols. The argument is a "
    "one-character designator for the method ('m' uses the symbols that "
    "occur in the most input formulas, 'l' uses the symbols that occur in "
    "the least number of formulas, and 'r' samples randomly), followed by "
    "the number of symbols to select."},

   {OPT_SEED_METHODS,
    'm', "seed-method",
    OptArg, "lda",
    "Specify how to select seed axioms when artificially seeding is used."
    "The argument is a string of letters, each indicating one method to "
    "use. The letters are: \n"
    "'l': use the syntactically largest axiom in which the seed symbol occurs.\n"
    "'d': use the most diverse axiom in which the seed symbol occurs, i.e. "
    "the symbol with the largest set of different symbols.\n"
    "'a': use all axioms in which the seed symbol occurs.\n"
    "For 'l' and 'd', if there are multiple candidates, use the first one."
    "If the option is not set, 'a' is assumed."},

   {OPT_DUMP_FILTER,
    'd', "dump-filter",
    NoArg, NULL,
     "Print the filter definition in force."},

   {OPT_LOP_PARSE,
    '\0', "lop-in",
    NoArg, NULL,
    "Set E-LOP as the input format. If no input format is "
    "selected by this or one of the following options, E will "
    "guess the input format based on the first token. It will "
    "almost always correctly recognize TPTP-3, but it may "
    "misidentify E-LOP files that use TPTP meta-identifiers as "
    "logical symbols."},

   {OPT_LOP_FORMAT,
    '\0', "lop-format",
    NoArg, NULL,
    "Equivalent to --lop-in."},

   {OPT_TPTP_PARSE,
    '\0', "tptp-in",
    NoArg, NULL,
    "Parse TPTP-2 format instead of E-LOP (but note that includes are "
    "handled according to TPTP-3 semantics)."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp-format",
    NoArg, NULL,
    "Equivalent to --tptp-in."},

   {OPT_TPTP_PARSE,
    '\0', "tptp2-in",
    NoArg, NULL,
    "Synonymous with --tptp-in."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp2-format",
    NoArg, NULL,
    "Synonymous with --tptp-in."},

   {OPT_TSTP_PARSE,
    '\0', "tstp-in",
    NoArg, NULL,
    "Parse TPTP-3 format instead of E-LOP (Note that TPTP-3 syntax "
    "is still under development, and the version in E may not be "
    "fully conforming at all times. E works on all TPTP 6.3.0 FOF "
    "and CNF input files (including includes)."},

   {OPT_TSTP_FORMAT,
    '\0', "tstp-format",
    NoArg, NULL,
    "Equivalent to --tstp-in."},

   {OPT_TSTP_PARSE,
    '\0', "tptp3-in",
    NoArg, NULL,
    "Synonymous with --tstp-in."},

   {OPT_TSTP_FORMAT,
    '\0', "tptp3-format",
    NoArg, NULL,
    "Synonymous with --tstp-in."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};
IOFormat parse_format = AutoFormat;
char     *outname     = NULL;
char     *filtername  = NULL;
bool     dumpfilter   = false;
bool     seed_preds   = false,
         seed_funs    = false,
         seed_consts  = false,
         seed_large   = false,
         seed_diverse = false,
         seed_all     = true,
         app_encode   = false;
char*    seedstr      = NULL;

SubSampleMethod subsample   = SubSNone;
long            sample_size = LONG_MAX;


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
// Function: filter_problem()
//
//   Given a structured problem data structure, an axfilter, and the
//   core name of the output, apply the filter to the problem and
//   write the result into a properly named file (which is determined
//   from the core name and the filter name).
//
// Global Variables: -
//
// Side Effects    : I/O (writes result file), Memory operations
//
/----------------------------------------------------------------------*/

void filter_problem(StructFOFSpec_p ctrl,
                    AxFilter_p filter,
                    char* corename, char* desc)
{
   DStr_p   filename = DStrAlloc();
   PStack_p formulas, clauses;
   FILE     *fp;

   DStrAppendStr(filename, corename);
   DStrAppendChar(filename, '_');
   DStrAppendStr(filename, filter->name);
   DStrAppendStr(filename, ".p");

   formulas = PStackAlloc();
   clauses  = PStackAlloc();

   StructFOFSpecGetProblem(ctrl,
                           filter,
                           clauses,
                           formulas);

   fprintf(GlobalOut, COMCHAR" Filter: %s goes into file %s\n",
           filter->name,
           DStrView(filename));

   fp = fopen(DStrView(filename), "w");
   fprintf(fp, COMCHAR" Filter %s on file %s\n",
           filter->name, corename);
   if(desc)
   {
      fprintf(fp, "%s", desc);
   }
   SigPrintTypeDeclsTSTP(fp, ctrl->terms->sig);
   PStackClausePrintTSTP(fp, clauses);
   PStackFormulaPrintTSTP(fp, formulas);
   fclose(fp);

   PStackFree(clauses);
   PStackFree(formulas);
   DStrFree(filename);
}

/*-----------------------------------------------------------------------
//
// Function: all_filters_problem()
//
//   Apply all filters to problems.
//
// Global Variables:
//
// Side Effects    : Writes result files
//
/----------------------------------------------------------------------*/

void all_filters_problem(StructFOFSpec_p ctrl,
                         AxFilterSet_p filters,
                         char* corename,
                         bool hypo_filter_only,
                         char *desc)
{
   int i;

   for(i=0; i<AxFilterSetElements(filters); i++)
   {
      /* SigPrint(stdout,ctrl->sig); */
      if(!hypo_filter_only || AxFilterSetGetFilter(filters,i)->use_hypotheses)
      {
         filter_problem(ctrl,
                        AxFilterSetGetFilter(filters,i),
                        corename, desc);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: find_seed_symbols()
//
//   Push all symbols in sig that correspond to the symbol types used
//   for seeding onto result.
//
// Global Variables: seed_preds, seed_funs, seed_consts, seed_large,
//                   seed_diverse, seed_all
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void find_seed_symbols(Sig_p sig, PStack_p result)
{
   FunCode i;

   for(i=sig->internal_symbols+1;
       i<=sig->f_count;
       i++)
   {
      if(seed_preds && SigIsPredicate(sig, i))
      {
         PStackPushInt(result, i);
      }
      else if(seed_funs && !SigIsPredicate(sig, i)
              && SigFindArity(sig, i) > 0)
      {
         PStackPushInt(result, i);
      }
      else if(seed_consts && !SigIsPredicate(sig, i)
              && SigFindArity(sig, i) == 0)
      {
         PStackPushInt(result, i);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: seeded_filter_all()
//
//    Generate seeded axiom selections based on all formulas with
//    symbol seed_symbols (which are already delivered in
//    symb_formulas).
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void seeded_filter_all(StructFOFSpec_p ctrl,
                       AxFilterSet_p filters,
                       char* corename,
                       FunCode seed_symbol,
                       PStack_p symb_formulas)
{
   DStr_p desc = DStrAlloc();
   DStr_p name = DStrAlloc();

   FormulaStackCondSetType(symb_formulas, CPTypeHypothesis);

   DStrAppendStr(desc, "% Seed symbol: ");
   DStrAppendStr(desc, SigFindName(ctrl->terms->sig, seed_symbol));
   DStrAppendStr(desc, " = ");
   DStrAppendInt(desc, seed_symbol);
   DStrAppendStr(desc, "\n% Seeds      : All\n");
   DStrAppendStr(desc, "% Arity      : ");
   DStrAppendInt(desc, SigFindArity(ctrl->terms->sig, seed_symbol));
   DStrAppendStr(desc, "\n% Type       : ");
   DStrAppendStr(desc, SigIsPredicate(ctrl->terms->sig, seed_symbol)?
                 "Predicate\n":"Function\n");

   DStrAppendStr(name, corename);
   DStrAppendStr(name, "_SA_");
   DStrAppendStr(name, SigIsPredicate(ctrl->terms->sig, seed_symbol)?
                 "P":"F");
   DStrAppendInt(name, SigFindArity(ctrl->terms->sig, seed_symbol));
   DStrAppendStr(name, "_");
   DStrAppendInt(name, seed_symbol);

   printf("Name: %s\n", DStrView(name));

   all_filters_problem(ctrl,
                       filters,
                       DStrView(name),
                       true,
                       DStrView(desc));
   FormulaStackCondSetType(symb_formulas, CPTypeAxiom);
   DStrFree(name);
   DStrFree(desc);
}



/*-----------------------------------------------------------------------
//
// Function: seeded_filter_largest()
//
//    Generate seeded axiom selections based on the largest formula
//    with symbol seed_symbols (candidates are already delivered in
//    symb_formulas).
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void seeded_filter_largest(StructFOFSpec_p ctrl,
                           AxFilterSet_p filters,
                           char* corename,
                           FunCode seed_symbol,
                           PStack_p symb_formulas)
{
   DStr_p desc = DStrAlloc();
   DStr_p name = DStrAlloc();
   long max_size = 0, size;
   WFormula_p largest=NULL, handle=NULL;
   PStackPointer i;

   for(i=0; i<PStackGetSP(symb_formulas); i++)
   {
      handle = PStackElementP(symb_formulas, i);
      size = TermWeight(handle->tformula, 1, 1);
      if(size > max_size)
      {
         largest = handle;
         max_size = size;
      }
   }
   if(largest && FormulaQueryType(largest)==CPTypeAxiom)
   {
      FormulaSetType(largest, CPTypeHypothesis);
   }

   DStrAppendStr(desc, "% Seed symbol: ");
   DStrAppendStr(desc, SigFindName(ctrl->terms->sig, seed_symbol));
   DStrAppendStr(desc, " = ");
   DStrAppendInt(desc, seed_symbol);
   DStrAppendStr(desc, "\n% Seeds      : Largest\n");
   DStrAppendStr(desc, "% Arity      : ");
   DStrAppendInt(desc, SigFindArity(ctrl->terms->sig, seed_symbol));
   DStrAppendStr(desc, "\n% Type       : ");
   DStrAppendStr(desc, SigIsPredicate(ctrl->terms->sig, seed_symbol)?
                 "Predicate\n":"Function\n");

   DStrAppendStr(name, corename);
   DStrAppendStr(name, "_SL_");
   DStrAppendStr(name, SigIsPredicate(ctrl->terms->sig, seed_symbol)?
                 "P":"F");
   DStrAppendInt(name, SigFindArity(ctrl->terms->sig, seed_symbol));
   DStrAppendStr(name, "_");
   DStrAppendInt(name, seed_symbol);

   printf("Name: %s\n", DStrView(name));

   all_filters_problem(ctrl,
                       filters,
                       DStrView(name),
                       true,
                       DStrView(desc));
   if(largest && FormulaQueryType(largest)==CPTypeHypothesis)
   {
      FormulaSetType(handle, CPTypeAxiom);
   }
   DStrFree(name);
   DStrFree(desc);
}


/*-----------------------------------------------------------------------
//
// Function: seeded_filter_diverse()
//
//    Generate seeded axiom selections based on the most diverse
//    formula with symbol seed_symbols (candidates are already
//    delivered in symb_formulas).
//
// Global Variables: -
//
// Side Effects    : Output, memory operations
//
/----------------------------------------------------------------------*/

void seeded_filter_diverse(StructFOFSpec_p ctrl,
                           AxFilterSet_p filters,
                           char* corename,
                           FunCode seed_symbol,
                           PStack_p symb_formulas)
{
   DStr_p desc = DStrAlloc();
   DStr_p name = DStrAlloc();
   long max_size = 0, size;
   WFormula_p largest=NULL, handle = NULL;
   PStackPointer i;

   for(i=0; i<PStackGetSP(symb_formulas); i++)
   {
      handle = PStackElementP(symb_formulas, i);
      size = WFormulaSymbolDiversity(handle);
      if(size > max_size)
      {
         largest = handle;
         max_size = size;
      }
   }
   if(largest && FormulaQueryType(largest)==CPTypeAxiom)
   {
      FormulaSetType(handle, CPTypeHypothesis);
   }

   DStrAppendStr(desc, "% Seed symbol: ");
   DStrAppendStr(desc, SigFindName(ctrl->terms->sig, seed_symbol));
   DStrAppendStr(desc, " = ");
   DStrAppendInt(desc, seed_symbol);
   DStrAppendStr(desc, "\n% Seeds      : Diverse\n");
   DStrAppendStr(desc, "% Arity      : ");
   DStrAppendInt(desc, SigFindArity(ctrl->terms->sig, seed_symbol));
   DStrAppendStr(desc, "\n% Type       : ");
   DStrAppendStr(desc, SigIsPredicate(ctrl->terms->sig, seed_symbol)?
                 "Predicate\n":"Function\n");

   DStrAppendStr(name, corename);
   DStrAppendStr(name, "_SD_");
   DStrAppendStr(name, SigIsPredicate(ctrl->terms->sig, seed_symbol)?
                 "P":"F");
   DStrAppendInt(name, SigFindArity(ctrl->terms->sig, seed_symbol));
   DStrAppendStr(name, "_");
   DStrAppendInt(name, seed_symbol);

   printf("Name: %s\n", DStrView(name));

   all_filters_problem(ctrl,
                       filters,
                       DStrView(name),
                       true,
                       DStrView(desc));
   if(largest && FormulaQueryType(largest)==CPTypeHypothesis)
   {
      FormulaSetType(largest, CPTypeAxiom);
   }
   DStrFree(name);
   DStrFree(desc);
}

/*-----------------------------------------------------------------------
//
// Function: subsample_seed_symbols()
//
//    Optionally reduce the set of seed symbols, based on the value of
//    the variables below.
//
// Global Variables: subsample, sample_size
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void subsample_seed_symbols(StructFOFSpec_p ctrl, PStack_p seed_symbols)
{
   WeightedObject_p weight_array;
   long array_size, i, limit;
   FunCode symbol;

   if(subsample == SubSNone)
   {
      return;
   }

   array_size = PStackGetSP(seed_symbols);

   weight_array = WeightedObjectArrayAlloc(array_size);

   switch(subsample)
   {
   case SubSRand:
         for(i=0; i<array_size; i++)
         {
            weight_array[i].weight = JKISSRandDouble(NULL);
            weight_array[i].object.i_val = PStackPopInt(seed_symbols);
         }
         assert(PStackEmpty(seed_symbols));
         break;
   case SubSMost:
         for(i=0; i<array_size; i++)
         {
            symbol = PStackPopInt(seed_symbols);
            weight_array[i].weight = ctrl->f_distrib->dist_array[symbol].fc_freq;
            weight_array[i].object.i_val = symbol;
         }
         break;
   case SubSLeast:
         for(i=0; i<array_size; i++)
         {
            symbol = PStackPopInt(seed_symbols);
            weight_array[i].weight = -ctrl->f_distrib->dist_array[symbol].fc_freq;
            weight_array[i].object.i_val = symbol;
         }
         assert(PStackEmpty(seed_symbols));
         break;
   default:
         assert(false && "Subsampling method not known or not yet implemented");
         break;
   }
   WeightedObjectArraySort(weight_array, array_size);
   limit = MIN(array_size, sample_size);
   for(i=0; i<limit; i++)
   {
      // printf("Symbol: %ld=%s\n", weight_array[i].object.i_val,
      // SigFindName(ctrl->sig, weight_array[i].object.i_val));
      PStackPushInt(seed_symbols, weight_array[i].object.i_val);
   }
   WeightedObjectArrayFree(weight_array);
}



/*-----------------------------------------------------------------------
//
// Function: decode_seed_symbols()
//
//   Parse the symbols from seedstr, find their encoding, and put them
//   onto the provided stack. Terminate with error if there is an
//   unknown symbol.
//
// Global Variables: -
//
// Side Effects    : May terminate program, memory operations
//
/----------------------------------------------------------------------*/

void decode_seed_symbols(Sig_p sig, char* seedstr, PStack_p seed_symbols)
{
   DStr_p id = DStrAlloc();
   FunCode f_code;

   Scanner_p in = CreateScanner(StreamTypeOptionString, seedstr,
                                true, NULL, true);
   CheckInpTok(in, FuncSymbStartToken);
   FuncSymbParse(in, id);
   f_code = SigFindFCode(sig, DStrView(id));
   if(!f_code)
   {

      Error("User-requested symbol %s unknown while parsing option --seeds",
            USAGE_ERROR, DStrView(id) );
   }
   PStackPushInt(seed_symbols, f_code);

   while(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      DStrReset(id);
      FuncSymbParse(in, id);
      f_code = SigFindFCode(sig, DStrView(id));
      if(!f_code)
      {
         Error("User-requested symbol %s unknown while parsing option --seeds",
               USAGE_ERROR, DStrView(id));
      }
      PStackPushInt(seed_symbols, f_code);
   }
   DestroyScanner(in);
   DStrFree(id);
}


/*-----------------------------------------------------------------------
//
// Function: seeded_filters()
//
//   Run through all seeds, all seeding methods and generate all
//   filtered files.
//
// Global Variables: seed_preds, seed_funs, seed_consts, seed_large,
//                   seed_diverse, seed_all
//
// Side Effects    : Writes result files.
//
/----------------------------------------------------------------------*/

void seeded_filters(StructFOFSpec_p ctrl,
                    AxFilterSet_p filters,
                    char* corename)
{
   PStack_p seed_symbols = PStackAlloc();
   FunCode seed;

   if(seedstr)
   {
      decode_seed_symbols(ctrl->terms->sig, seedstr, seed_symbols);
   }
   else
   {
      find_seed_symbols(ctrl->terms->sig, seed_symbols);
      subsample_seed_symbols(ctrl, seed_symbols);
   }

   while(!PStackEmpty(seed_symbols))
   {
      PStack_p symb_formulas = PStackAlloc();
      seed = PStackPopInt(seed_symbols);
      StructFOFSpecCollectFCode(ctrl, seed, symb_formulas);

      if(seed_all)
      {
         seeded_filter_all(ctrl,
                           filters,
                           corename,
                           seed,
                           symb_formulas);
      }
      if(seed_large)
      {
         seeded_filter_largest(ctrl,
                               filters,
                               corename,
                               seed,
                               symb_formulas);
      }
      if(seed_diverse)
      {
         seeded_filter_diverse(ctrl,
                               filters,
                               corename,
                               seed,
                               symb_formulas);
      }
      PStackFree(symb_formulas);
   }

   PStackFree(seed_symbols);
}




/*-----------------------------------------------------------------------
//
// Function: main()
//
//   Main function of the program.
//
// Global Variables: Yes
//
// Side Effects    : All
//
/----------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
   CLState_p        state;
   StructFOFSpec_p  ctrl;
   PStack_p         prob_names = PStackAlloc();
   int              i;
   AxFilterSet_p    filters;
   Scanner_p        in;
   DStr_p           corename;
   char             *tname;

   assert(argv[0]);

   InitIO(NAME);
   DocOutputFormat = tstp_format;
   OutputFormat = TSTPFormat;

   state = process_options(argc, argv);


   OpenGlobalOut(outname);

   if(filtername)
   {
      filters = AxFilterSetAlloc();
      in = CreateScanner(StreamTypeFile, filtername, true, NULL, true);
      AxFilterSetParse(in, filters);
      DestroyScanner(in);
   }
   else
   {
      filters = AxFilterSetCreateInternal(AxFilterDefaultSet);
   }
   if(dumpfilter)
   {
      AxFilterSetPrint(GlobalOut, filters);
   }

   if(state->argc < 1)
   {
      Error("Usage: e_axfilter <problem> [<options>]\n", USAGE_ERROR);
   }

   for(i=0; state->argv[i]; i++)
   {
      PStackPushP(prob_names,  state->argv[i]);
   }
   /* Base name is the stripped base of the first argument */
   tname = FileNameStrip(state->argv[0]);
   corename = DStrAlloc();
   DStrAppendStr(corename, tname);
   FREE(tname);

   ctrl = StructFOFSpecAlloc();
   StructFOFSpecParseAxioms(ctrl, prob_names, parse_format, NULL);
   StructFOFSpecInitDistrib(ctrl, false);
   //GenDistribPrint(stdout, ctrl->f_distrib, 10);
   StructFOFSpecResetShared(ctrl);

   if(seed_preds || seed_funs || seed_consts || seedstr)
   {
      seeded_filters(ctrl, filters, DStrView(corename));
   }
   else
   {
      all_filters_problem(ctrl,
                          filters,
                          DStrView(corename),
                          false,
                          NULL);
   }

   StructFOFSpecFree(ctrl);
   DStrFree(corename);
   AxFilterSetFree(filters);
   CLStateFree(state);
   PStackFree(prob_names);

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
   char  *arg, *ci;

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
            fprintf(stdout, NAME " " VERSION " " E_NICKNAME "\n");
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
      case OPT_FILTER:
            filtername = arg;
            break;
      case OPT_SEED_METHODS:
            seed_all = false;
            CheckOptionLetterString(arg, "lda", "-m (--seed-methods)");
            for(ci=arg; *ci; ci++)
            {
               switch(*ci)
               {
               case 'l':
                     seed_large = true;
                     break;
               case 'd':
                     seed_diverse = true;
                     break;
               case 'a':
                     seed_all = true;
                     break;
               default:
                     assert(false && "Impossible option in string");
                     break;
               }
            }
            break;
      case OPT_SEEDS:
            seedstr = arg;
            break;
      case OPT_SEED_SUBSAMPLE:
            if(strlen(arg)<2 ||
               ((arg[0] != 'm') && (arg[0] != 'l') && (arg[0] != 'r' )) ||
               ((arg[1] < '0') || (arg[1] > '9')))
            {
               Error("Option --seed-subsample) expects "
                     "argument of the form [mlr][0-9]+", USAGE_ERROR);
            }
            switch(arg[0])
            {
            case 'm':
                  subsample = SubSMost;
                  break;
            case 'l':
                  subsample = SubSLeast;
                  break;
            case 'r':
                  subsample = SubSRand;
                  break;
            default:
                  assert(false && "Impossible argument in string");
                  break;
            }
            sample_size = atol(arg+1);
            break;
      case OPT_SEED_SYMBOLS:
            CheckOptionLetterString(arg, "pfc", "-S (--seed-symbols)");
            for(ci=arg; *ci; ci++)
            {
               switch(*ci)
               {
               case 'p':
                     seed_preds = true;
                     break;
               case 'f':
                     seed_funs = true;
                     break;
               case 'c':
                     seed_consts = true;
                     break;
               default:
                     assert(false && "Impossible option in string");
                     break;
               }
            }
            break;
      case OPT_DUMP_FILTER:
            dumpfilter = true;
            break;
      case OPT_LOP_PARSE:
      case OPT_LOP_FORMAT:
            parse_format = LOPFormat;
            break;
      case OPT_TPTP_PARSE:
      case OPT_TPTP_FORMAT:
            parse_format = TPTPFormat;
            break;
      case OPT_TSTP_PARSE:
      case OPT_TSTP_FORMAT:
            parse_format = TSTPFormat;
            break;
      default:
            assert(false && "Unknown option");
            break;
      }
   }
   return state;
}

void print_help(FILE* out)
{
   fprintf(out, "\n"
NAME " " VERSION " \"" E_NICKNAME "\"\n\
\n\
Usage: " NAME " [options] [files]\n\
\n\
This program applies SinE-like goal-directed filters to a problem\n\
specification (a set of clauses and/or formulas) to generate reduced\n\
problem specifications that are easier to handle for a theorem prover,\n\
but still are likely to contain all the axioms necessary for a proof\n\
(if one exists).\n\
\n\
In default mode, the program reads a problem specification and an\n\
(optional) filter specification, and produces one reduced output file \n\
for each filter given. Note that while all standard input formats (LOP,\n\
TPTP-2 and TPTP-3 are supported, output is only and automatically in\n\
TPTP-3. Also note that unlike most of the other tools in the E\n\
distribution, this program does not support pipe-based input and output,\n\
since it uses file names generated from the input file name and filter\n\
names to store the different result files\n\
\n");
   PrintOptions(stdout, opts, "Options:\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
