/*-----------------------------------------------------------------------

File  : direct_examples.c

Author: Stephan Schulz

Contents
 
  Generate examples directly from a protocol file. 

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Jul 23 17:46:15 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_tempfile.h>
#include <ccl_clausesets.h>
#include <can_treeanalyze.h>
#include <cio_signals.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "direct_examples"
#define VERSION "0.1dev"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_PRINT_EVAL,
   OPT_NEG_NO,
   OPT_NEG_PROP 
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
    "Verbose comments on the progress of the program."},

   {OPT_OUTPUT,
    'o', "output-file",
    ReqArg, NULL,
   "Redirect output into the named file."},

   {OPT_PRINT_EVAL,
    'e', "print-clause-at-evaluation",
    NoArg, NULL,
    "Print the selected clauses in the form they are evaluated"
    " (default is to print the selected clauses whenever they are"
    " modified by an inference)."},

   {OPT_NEG_NO,
    'n', "negative-example-number",
    ReqArg, NULL,
    "Set the (maximum) number of negative examples to pick if the "
    "proof listing does not describe a successful proof."},
    
   {OPT_NEG_PROP,
    'p', "negative-example-proportion",
    ReqArg, NULL, 
    "Set the maximum number of negative examples (expressed as a "
    "proportion of the positive examples) to pick if the proof "
    "listing does describe a successful proof"},
    
   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char   *outname = NULL;
bool   print_eval = false;
double neg_proportion = 1;
long   neg_examples = 200;

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
   /* Scanner_p       in;     */
   CLState_p       state;
   PStack_p        tmpfileinfo;

   assert(argv[0]);

   printf("This does not work and is not supposed to do so at the moment!\n");
   exit(1);

   InitOutput();
   InitError(NAME);
   atexit(TempFileCleanup);

   ESignalSetup(SIGTERM);
   ESignalSetup(SIGINT);

   state = process_options(argc, argv);

   GlobalOut = OutOpen(outname);
   OutputLevel = 0;
   
   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   tmpfileinfo = CLStateCreateTempFiles(state);
   
   ClausesHaveLocalVariables = false; /* We need consistent
					 name->variable mappings in
					 this application! */
   /* infstate1 = InfStateAlloc();   
   infstate1->print_clause_evals = false;
   infstate1->print_clause_mods  = false;
   infstate1->print_clause_stats = false;

   infstate2 = InfStateAlloc();
   infstate2->print_clause_evals = print_eval;
   infstate2->print_clause_mods  = !print_eval;
   infstate2->print_clause_stats = true;

   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL);      
      InfStateInfListParse(in, GlobalOut, infstate1);      
      DestroyScanner(in);
   }
   infstate2->created_count = infstate1->created_count;
   infstate2->processed_count = infstate1->processed_count; 
   infstate2->update_inf_inc = 0;

   ProofSetClauseStatistics(infstate1, 0, 0, 0, 0, PROOF_DIST_INFINITY);
   InfStateSelectExamples(infstate1, neg_proportion, neg_examples);
   InfStateStoreClauseInfo(&(infstate2->watch_clauses), infstate1);
   
   fprintf(GlobalOut, "# Axioms:\n");
   ClauseSetPrint(GlobalOut, infstate1->axioms, true);
   fprintf(GlobalOut, ".\n\n# Examples:\n");
   InfStateFree(infstate1);
     
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL);      
      InfStateInfListParse(in, GlobalOut, infstate2);      
      DestroyScanner(in);
   }   

   InfStateFree(infstate2); */

   CLStateDestroyTempFiles(state, tmpfileinfo);
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
// Global Variables: 
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
	    printf(NAME " " VERSION "\n");
	    exit(NO_ERROR);
	    break;
      case OPT_OUTPUT:
	    outname = arg;
	    break;
      case OPT_PRINT_EVAL:
	    print_eval = true;
	    break;
      case OPT_NEG_NO:
	    neg_examples = CLStateGetIntArg(handle, arg);
	    break;
      case OPT_NEG_PROP:
	    neg_proportion = CLStateGetFloatArg(handle, arg);
	    if(neg_proportion < 0)
	    {
	       Error("Option -p (--negative-example-proportion)"
		     "requires positive argument.}", USAGE_ERROR);	       
	    }
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
\n"
NAME " " VERSION "\n\
\n\
Usage: THIS IS BROKEN AT THE MOMENT " NAME " [options] [files]\n\
\n\
Parse a full E inference listing (possibly\n\
spread over multiple files), and generate training examples\n\
corresponding to the selected clauses.\n"); 
   PrintOptions(stdout, opts);
   fprintf(out, "\n\
Copyright 1998 by Stephan Schulz, schulz@informatik.tu-muenchen.de\n\
\n\
This program is a part of the support structure for the E equational\n\
theorem prover. You can find the latest version of the E distribution\n\
as well as additional information at\n\
http://wwwjessen.informatik.tu-muenchen.de/~schulz/WORK/eprover.html.\n\
\n\
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
\n\
Stephan Schulz\n\
Technische Universitaet Muenchen\n\
Fakultaet fuer Informatik\n\
Arcisstrasse 20\n\
D-80290 Muenchen\n\
Germany\n\
");

}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


