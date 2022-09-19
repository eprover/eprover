/*-----------------------------------------------------------------------

  File  : term2dag.c

  Author: Stephan Schulz

  Contents

  Main program for a simple CLIB application: Read term set, write
  equivalent DAG.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Nov 28 00:27:40 MET 1997

-----------------------------------------------------------------------*/

#include <stdio.h>
#include <cio_commandline.h>
#include <cio_basicparser.h>
#include <cio_output.h>
#include <cte_termbanks.h>

#define VERSION "0.1 - Sat Nov 29 16:39:20 MET 1997"

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_PRINT_REFS
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
   {OPT_PRINT_REFS,
    'r', "print-reference-number",
    OptArg, "true",
    "Print number of references for each DAG node as a comment."},
   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char *outname = NULL;
bool app_encode = false;

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
   CLState_p state;
   Scanner_p in;
   Term_p term;
   TB_p bank;
   FILE* out;
   int i;

   assert(argv[0]);
   InitError(argv[0]);

   state = process_options(argc, argv);

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   TBPrintInternalInfo = true;
   out = OutOpen(outname);
   bank = TBAlloc(SigAlloc(TypeBankAlloc()));

   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL, true);
      while(!TestInpTok(in, NoToken))
      {
         term  = TBTermParse(in, bank);
         TermCellSetProp(term, TPTopPos);
      }
      DestroyScanner(in);
   }
   SigPrint(out, bank->sig);
   TBPrintBankInOrder(out, bank);
   TypeBankFree(bank->sig->type_bank);
   SigFree(bank->sig);
   bank->sig = NULL;
   TBFree(bank);
   OutClose(out);
   CLStateFree(state);
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
..                   description if option -h or --help was present
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
      case OPT_PRINT_REFS:
            TBPrintInternalInfo = CLStateGetBoolArg(handle, arg);
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
\n                                              \
term2dag "VERSION"\n                            \
\n                                              \
Usage: term2dag [options] [files]\n             \
\n                                                      \
Read a set of terms and print a DAG representing it.\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
