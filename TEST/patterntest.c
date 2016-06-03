/*-----------------------------------------------------------------------

File  : patterntest.c

Author: Stephan Schulz

Contents
 
  Main program for the CLIB test programm

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Nov 28 00:27:40 MET 1997

-----------------------------------------------------------------------*/

#include <stdio.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <ccl_paramod.h>
#include <ccl_clausesets.h>
#include <ccl_rewrite.h>
#include <cle_annoterms.h>
#include <cle_termtops.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_DISCOUNT_VARS,
   OPT_NO_INFIX,
   OPT_FULL_EQ_REP
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
   {OPT_VERBOSE, 
    'v', "verbose", 
    OptArg, "1",
    "Verbose comments on the progress of the program."},
   {OPT_OUTPUT,
    'o', "output-file",
    ReqArg, NULL,
   "Redirect output into the named file."},
   {OPT_DISCOUNT_VARS,
    'd', "discount-vars",
    OptArg, "true",
    "Read and write PCL terms (otherwise use PROLOG terms)."},
   {OPT_NO_INFIX,
    'p', "eqn-no-infix",
    OptArg, "true",
    "Print equations in prefix notation equal(x,y)."},
   {OPT_FULL_EQ_REP,
    'e', "full-equational-rep",
    OptArg, "true",
    "Print all literals as equations, even non-equational ones."},
   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char *outname = NULL;

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
   CLState_p   state;
   Scanner_p   in; 
   TB_p        bank;
   Term_p      term, top;
   int         i;

   assert(argv[0]);
   InitIO(argv[0]);

   state = process_options(argc, argv);
   
   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   bank = TBAlloc(SigAlloc());
   
   in = CreateScanner(StreamTypeFile, state->argv[0], true, NULL);
   
   while(!TestInpTok(in, NoToken))
   {
      term = TBTermParse(in, bank);
      for(i=1; i<5; i++)
      {
	 printf("Depth: %i  Original term: ", i);
	 TBPrintTermFull(stdout, bank, term);
	 printf("\n\n");
	 
	 top = TermTop(term, i, bank->vars);
	 printf("  Top:   ");
	 TermPrint(stdout, top, bank->sig, DEREF_NEVER);
	 printf("\n");
	 TermFree(top);

	 top = AltTermTop(term, i, bank->vars);
	 printf("  Top':  ");
	 TermPrint(stdout, top, bank->sig, DEREF_NEVER);
	 printf("\n");
	 TermFree(top);

	 top = CSTermTop(term, i, bank->vars);
	 printf("  CSTop: ");
	 TermPrint(stdout, top, bank->sig, DEREF_NEVER);
	 printf("\n");
	 TermFree(top);

	 top = ESTermTop(term, i, bank->vars);
	 printf("  ESTop: ");
	 TermPrint(stdout, top, bank->sig, DEREF_NEVER);
	 printf("\n");
	 TermFree(top);
	 printf("\n");
      }
   }
   DestroyScanner(in);

   SigFree(bank->sig);
   bank->sig = NULL;
   TBFree(bank);

   CLStateFree(state);
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
      case OPT_OUTPUT:
	    outname = arg;
	    break;
      case OPT_NO_INFIX:
	    EqnUseInfix = !CLStateGetBoolArg(handle, arg);
	    break;
      case OPT_FULL_EQ_REP:
	    EqnFullEquationalRep = CLStateGetBoolArg(handle, arg);
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
cl_test\n\
\n\
Usage: cl_patterntest [options] [files]\n\
\n\
Read a set of clauses and transform them into patterns.\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


