/*-----------------------------------------------------------------------

File  : classify_problem.c

Author: Stephan Schulz

Contents
 
  Read a specification and print classification and feature vector.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat Dec 12 22:39:18 MET 1998
    New

-----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <ccl_unfold_defs.h>
#include <ccl_formulafunc.h>
#include <che_clausesetfeatures.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define VERSION "0.41"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_TPTP_PARSE,
   OPT_TPTP_PRINT,
   OPT_TPTP_FORMAT,
   OPT_GEN_TPTP_HEADER,
   OPT_NO_PREPROCESSING,
   OPT_MASK,
   OPT_ABSOLUTE,
   OPT_FEW_LIMIT,
   OPT_MANY_LIMIT,
   OPT_FEW_COUNT,
   OPT_MANY_COUNT,
   OPT_AXIOM_MANY_LIMIT,
   OPT_AXIOM_SOME_LIMIT,
   OPT_LIT_MANY_LIMIT,
   OPT_LIT_SOME_LIMIT,
   OPT_TERM_MANY_LIMIT,
   OPT_TERM_SOME_LIMIT
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

   {OPT_TPTP_PARSE,
    '\0', "tptp-in",
    NoArg, NULL,
    "Parse TPTP format instead of lop (does not understand includes, "
    "as TPTP includes are a brain-dead design)."},

   {OPT_TPTP_PRINT,
    '\0', "tptp-out",
    NoArg, NULL,
    "Print TPTP format instead of lop. Implies --eqn-no-infix and "
    "will ignore --full-equational-rep."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp-format",
    NoArg, NULL,
    "Equivalent to --tptp-in and --tptp-out."},

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

   {OPT_MASK,
    'c', "class-mask",
    ReqArg, NULL,
    "Provide a mask for the class description. A mask is a 10 letter "
    "string, with positions corresponding to the class "
    "descriptors. Any dash ('-') in the string masks out the "
    "corresponding position in the class descriptor."},

   {OPT_ABSOLUTE,
    'a', "absolute",
    OptArg, "true",
    "Use absolute values (not percentages) to determine if few, some "
    "or many clauses have a certain property."},

   {OPT_FEW_LIMIT,
    'f', "few-limit",
    ReqArg, NULL,
    "Set the limit (either an absolute integer value or a part "
    "between 0 and 1) up to which only a few clauses "
    "from a given set are considered to have a certain property."},
   
   {OPT_MANY_LIMIT,
    'm', "many-limit",
    ReqArg, NULL,
    "Set the limit (either an absolute integer value or a part "
    "between 0 and 1) from which on many clauses from "
    "a given set are considered to have a certain property."},

   {OPT_AXIOM_SOME_LIMIT,
    '\0', "ax-some-limit",
    ReqArg, NULL,
    "Set the mimumum number of clauses for a specification to be "
    "considered to be medium size with respect to this measure."},

   {OPT_AXIOM_MANY_LIMIT,
    '\0', "ax-many-limit",
    ReqArg, NULL,
    "Set the mimumum number of clauses for a specification to be "
    "considered to be large size with respect to this measure."},

   {OPT_LIT_SOME_LIMIT,
    '\0', "lit-some-limit",
    ReqArg, NULL,
    "Set the mimumum number of literals for a specification to be "
    "considered to be medium size with respect to this measure."},

   {OPT_LIT_MANY_LIMIT,
    '\0', "lit-many-limit",
    ReqArg, NULL,
    "Set the mimumum number of literals for a specification to be "
    "considered to be large size with respect to this measure."},

   {OPT_TERM_SOME_LIMIT,
    '\0', "term-some-limit",
    ReqArg, NULL,
    "Set the mimumum number of subterms for a specification to be "
    "considered to be medium size with respect to this measure."},

   {OPT_TERM_MANY_LIMIT,
    '\0', "term-many-limit",
    ReqArg, NULL,
    "Set the mimumum number of subterms for a specification to be "
    "considered to be large size with respect to this measure."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char   *outname = NULL, 
       *mask = "aaaaa----a";
double few_limit  = FEW_DEFAULT,
       many_limit = MANY_DEFAULT;
bool   absolute = false;
long   ax_some_limit     = AX_SOME_DEFAULT,
       ax_many_limit     = AX_MANY_DEFAULT, 
       lit_some_limit    = LIT_SOME_DEFAULT,
       lit_many_limit    = LIT_MANY_DEFAULT, 
       term_some_limit   = TERM_MED_DEFAULT,
       term_many_limit   = TERM_LARGE_DEFAULT;
IOFormat parse_format    = LOPFormat;
bool     tptp_header     = false,
         no_preproc      = false;

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
   /* TB_p            terms;
      Sig_p           sig; */
   ProofState_p    fstate;
   /* ClauseSet_p     clauses; */
   Scanner_p       in;    
   int             i, min_arity, max_arity, symbol_count;
   long            depthmax, depthsum, count;
   CLState_p       state;
   SpecFeatureCell features;

   assert(argv[0]);
   
   InitOutput();
   InitError("classify_problem");

   state = process_options(argc, argv);

   if(absolute)
   {
      if(few_limit < 0)
      {
	 Error("Option -f (--few-limit) requires value >=0.",
	       USAGE_ERROR);
      }
      if(many_limit < 0)
      {
	 Error("Option -m (--many-limit) requires value >=0.",
	       USAGE_ERROR);
      }
      if(few_limit == FEW_DEFAULT)
      {
	 few_limit = FEW_ABSDEFAULT;
      }
      if(many_limit == MANY_DEFAULT)
      {
	 many_limit = MANY_ABSDEFAULT;
      }
   }
   else
   {
      if((few_limit < 0.0)||(few_limit > 1.0))
      {
	 Error("Option -f (--few-limit) requires value between "
	       "0 and 1.", USAGE_ERROR);
      }
      if((many_limit < 0.0)||(many_limit > 1.0))
      {
	 Error("Option -M (--many-limit) requires value between "
	       "0 and 1.", USAGE_ERROR);
      }
   }
   if(few_limit>many_limit)
   {
      Warning("Limit for 'many' was smaller than limit for "
	      "'few'. Was set to 'few'-limit.");
   }
      
   GlobalOut = OutOpen(outname);

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   
   for(i=0; state->argv[i]; i++)
   {
      /* sig   = SigAlloc();
         terms = TBAlloc(TPIgnoreProps, sig); */
      fstate = ProofStateAlloc();
      in    = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL);
      ScannerSetFormat(in, parse_format);
      
      FormulaAndClauseSetParse(in, fstate->axioms, 
                               fstate->f_axioms,
                               fstate->original_terms);
      FormulaSetPreprocConjectures(fstate->f_axioms);
      FormulaSetCNF(fstate->f_axioms, fstate->axioms, 
                    fstate->original_terms, fstate->freshvars);

      if(!no_preproc)
      {
         ClauseSetPreprocess(fstate->axioms,
                             fstate->watchlist,
                             fstate->tmp_terms,
                             false);
      }      /* SigPrint(stdout,sig);*/
      SpecFeaturesCompute(&features, fstate->axioms, fstate->signature);
      SpecFeaturesAddEval(&features, few_limit, many_limit, absolute,
			  ax_some_limit, ax_many_limit,
			  lit_some_limit, lit_many_limit,
			  term_some_limit, term_many_limit);
      

      if(!tptp_header)
      {
	 fprintf(GlobalOut, "%s : ", state->argv[i]);
	 SpecFeaturesPrint(GlobalOut, &features);
	 fprintf(GlobalOut, " : ");
	 SpecTypePrint(GlobalOut, &features, mask);
	 fprintf(GlobalOut, "\n");
      }
      else
      {
	 fprintf(GlobalOut, 
		 "%% Syntax   : Number of clauses    : %4ld "
		 "(%4ld non-Horn; %3ld unit; %3ld RR)\n",
		 features.clauses, 
		 features.clauses-features.horn,
		 features.unit,
		 ClauseSetCountTPTPRangeRestricted(fstate->axioms));
	 fprintf(GlobalOut, 
		 "%%            Number of literals   : %4ld "
		 "(%4ld equality)\n",
		 features.literals, 
		 ClauseSetCountEqnLiterals(fstate->axioms));	 
	 fprintf(GlobalOut, 
		 "%%            Maximal clause size  : %4ld ",
		 ClauseSetMaxLiteralNumber(fstate->axioms));
	 if(features.clauses)
	 {
	    fprintf(GlobalOut, "(%4ld average)\n",
		    features.literals/features.clauses);
	 }
	 else
	 {
	    fprintf(GlobalOut, "(   - average)\n");
	 }

	 symbol_count = SigCountSymbols(fstate->signature, true);
	 min_arity = SigFindMinPredicateArity(fstate->signature);
	 max_arity = SigFindMaxPredicateArity(fstate->signature);	
	 
	 if(features.eq_content!=SpecNoEq)
	 {/* Correct for the fact that TPTP treats equal as a normal
	     predicate symbol */
	    symbol_count++;
	    max_arity = MAX(max_arity,2);
	    min_arity = MIN(min_arity,2);
	 }
	 fprintf(GlobalOut, 
		 "%%            Number of predicates : %4d "
		 "(%4d propositional; ",
		 symbol_count,
		 SigCountAritySymbols(fstate->signature, 0, true));
	 if(symbol_count)
	 {	    
	    fprintf(GlobalOut, "%d-%d arity)\n",
		    min_arity, max_arity);
	 }
	 else
	 {
	    fprintf(GlobalOut, "--- arity)\n");
	 }

	 symbol_count = SigCountSymbols(fstate->signature, false);
	 min_arity = SigFindMinFunctionArity(fstate->signature);
	 max_arity = SigFindMaxFunctionArity(fstate->signature);	

	 fprintf(GlobalOut, 
		 "%%            Number of functors   : %4d "
		 "(%4d constant; ",
		 symbol_count,
		 SigCountAritySymbols(fstate->signature, 0, false));
	 if(symbol_count)
	 {	    
	    fprintf(GlobalOut, "%d-%d arity)\n",
		    min_arity, max_arity);
	 }
	 else
	 {
	    fprintf(GlobalOut, "--- arity)\n");
	 }
	 fprintf(GlobalOut, 
		 "%%            Number of variables  : %4ld (%4ld singleton)\n",
		 ClauseSetCountVariables(fstate->axioms),
		 ClauseSetCountSingletons(fstate->axioms));
	 depthmax=0;
	 depthsum=0;
	 count=0;
	 ClauseSetTPTPDepthInfoAdd(fstate->axioms, &depthmax, &depthsum,
				   &count);
	 if(count)
	 {
	    fprintf(GlobalOut, 
		    "%%            Maximal term depth   : %4ld (%4ld average)\n",
		    depthmax, depthsum/count);
	 }
	 else
	 {
	    fprintf(GlobalOut, 
		    "%%            Maximal term depth   :    - (   - average)\n");
	 }
      }
      DestroyScanner(in);
      ProofStateFree(fstate);
   }
   
   CLStateFree(state);
   
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
	    printf("classify_problem " VERSION "\n");
	    exit(NO_ERROR);
	    break;
      case OPT_OUTPUT:
	    outname = arg;
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
      case OPT_GEN_TPTP_HEADER:
	    tptp_header = true;
	    break;
      case OPT_NO_PREPROCESSING:
	    no_preproc = true;
	    break;
      case OPT_MASK:
	    mask = arg;
	    if(strlen(mask)!=10)
	    {
	       Error("Option -c (--class-mask) requires 10-letter "
		     "string as an argument", USAGE_ERROR);
	    }
	    break;
      case OPT_ABSOLUTE:
	    absolute = CLStateGetBoolArg(handle, arg);
	    break;
      case OPT_FEW_LIMIT:
	    few_limit = CLStateGetFloatArg(handle, arg);
	    break;
      case OPT_MANY_LIMIT:
	    many_limit = CLStateGetFloatArg(handle, arg);
	    break;
      case OPT_AXIOM_SOME_LIMIT:
	    ax_some_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_AXIOM_MANY_LIMIT:
	    ax_many_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_LIT_SOME_LIMIT:
	    lit_some_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_LIT_MANY_LIMIT:
	    lit_many_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_TERM_SOME_LIMIT:
	    term_some_limit = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_TERM_MANY_LIMIT:
	    term_many_limit = CLStateGetIntArg(handle, arg);
	    break;
      default:
	 assert(false);
	 break;
      }
   }
   return state;
}

void print_help(FILE* out)
{
   fprintf(out, "\n\
\n\
classify_problem " VERSION "\n\
\n\
Usage: classify_problem [options] [files]\n\
\n\
Read sets of clauses and classify them according to predefined criteria.\n\
\n");
   PrintOptions(stdout, opts);
   fprintf(out, "\n\
Copyright 1998-2003 by Stephan Schulz, " STS_MAIL "\n\
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


