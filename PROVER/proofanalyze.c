/*-----------------------------------------------------------------------

File  : proofanalyse.c

Author: Stephan Schulz

Contents
 
  Read the -l3 or -l4 output of E and determine clause dependencies,
  proof distance and evaluation, and write some of these information
  for clauses matching specified criteria.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Feb 12 00:31:51 MET 1999
    New

-----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <ccl_clausesets.h>
#include <can_treeanalyze.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "proofanalyse"
#define VERSION "0.11"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_NEG_NO,
   OPT_NEG_PROP,
   OPT_PRINT_ALL,
   OPT_PRINT_STATUS,
   OPT_PRINT_DEPS,
   OPT_NOPRINT_STATS
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

   {OPT_NEG_NO,
    'n', "negative-example-number",
    OptArg, "200",
    "Set the (maximum) number of negative examples to pick if the "
    "proof listing does not describe a successful proof."},
    
   {OPT_NEG_PROP,
    'p', "negative-example-proportion",
    ReqArg, NULL, 
    "Set the maximum number of negative examples (expressed as a "
    "proportion of the positive examples) to pick if the proof "
    "listing does describe a successful proof"},
   
   {OPT_PRINT_ALL,
    'a', "print-all-clauses",
    NoArg, NULL,
    "Print information about all clauses (otherwise restrict output "
    "to selected training examples)."},

   {OPT_PRINT_STATUS,
    's', "print-status",
    NoArg, NULL,
    "Print status information (selected/belongs to proof) for "
    "clauses."},

   {OPT_PRINT_DEPS,
    'D', "print-dependencies",
    NoArg, NULL,
    "Print parent information (generating/simplifying) for clauses."},

   {OPT_NOPRINT_STATS,
    'i', "inhibit-status-information",
    NoArg, NULL,
    "Do not print statistical information for clauses."},
  
   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char   *outname = NULL;
bool   print_state = false,
       print_deps  = false,
       print_stats = true;
ClauseProperties filter = CPOpFlag;
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
   Scanner_p       in;    
   int             i;
   CLState_p       state;

   assert(argv[0]);

   InitOutput();
   InitError(NAME);

   state = process_options(argc, argv);

   GlobalOut = OutOpen(outname);
   OutputLevel = 0;
   
   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   ClausesHaveLocalVariables = false; /* We need consistent
					 name->variable mappings in
					 this application! */
   /* infstate = InfStateAlloc(); */

   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL);
      
      /*  InfStateInfListParse(in, GlobalOut, infstate); */
      
      DestroyScanner(in);
   }   
   /* ProofSetClauseStatistics(infstate, 0, 0, 0, 0, PROOF_DIST_INFINITY);
   InfStateSelectExamples(infstate, neg_proportion, neg_examples);
   InfStatePrintClauses(GlobalOut, infstate, filter, print_state, 
			print_deps, print_stats); 

			InfStateFree(infstate); */

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
      case OPT_PRINT_ALL:
	    filter = CPIgnoreProps;
	    break;
      case OPT_PRINT_STATUS:
	    print_state = true;
	    break;
      case OPT_PRINT_DEPS:
	    print_deps = true;
	    break;
      case OPT_NOPRINT_STATS:
	    print_stats = false;
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
Usage: THIS IS BROKEN AT THE MOMENT! proofanalyze [options] [files]\n\
\n\
Parse a full E inference listing, determine clause dependencies and\n\
statistics, and print them for selected clauses.\n");
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





/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


