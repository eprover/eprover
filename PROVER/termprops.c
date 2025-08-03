/*-----------------------------------------------------------------------

File  : termprops.c

Author: Stephan Schulz

Contents

  Read a set of terms and print term, number of symbols and depth for
  each term

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
#include <cte_termbanks.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERBOSE,
   OPT_OUTPUT
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
   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char *outname = NULL;
bool app_encode = false;
ProblemType problemType  = PROBLEM_NOT_INIT;
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
   TypeBank_p  type_bank;
   Sig_p       sig;
   long        count, size, depth, size_sum, depth_sum, size_max,
               depth_max;
   int         i;
   Term_p      term;
   TB_p        bank;
   bool        sym,com;

   assert(argv[0]);
   InitIO(argv[0]);

   state = process_options(argc, argv);

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   OpenGlobalOut(outname);

   type_bank = TypeBankAlloc();
   sig = SigAlloc(type_bank);
   bank = TBAlloc(sig);
   size_sum = depth_sum = size_max = depth_max = count = 0;

   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], true, NULL);
      while(!TestInpTok(in,NoToken))
      {
    term  = TBTermParse(in, bank);
    size  = TermWeight(term,1,1);
    depth = TermDepth(term);
    if(term->arity == 2)
    {
       sym = (term->args[0]==term->args[1]);
    }
    else
    {
       sym = false;
    }
    if((term->arity == 2) && (term->args[0]->arity == 1))
    {
       com = (term->args[0]->args[1]==term->args[1]);
    }
    else
    {
       com = false;
    }
    TermPrint(GlobalOut, term, sig, DEREF_NEVER);
    fprintf(GlobalOut, "  : %ld : %ld : %c : %c\n",
       size,depth, sym?'s':'n',com?'s':'n');
    /* TBDelete(bank,term); */
    count++;
    size_sum+=size;
    depth_sum+=depth;
    size_max = MAX(size_max, size);
    depth_max = MAX(depth_max, depth);
      }
      DestroyScanner(in);
   }

   fprintf(GlobalOut,
           COMCHAR" Terms: %ld  ASize: %f MSize: %ld, ADepth: %f MDepth: %ld\n",
           count, size_sum/(float)count, size_max,
           depth_sum/(float)count, depth_max);
   bank->sig = NULL;
   TBFree(bank);
   SigFree(sig);
   TypeBankFree(type_bank);

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
// Global Variables: opts, Verbose, TermPrologArgs,
//                   TBPrintInternalInfo
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
Usage: termprops [options] [files]\n\
\n\
Read a set of terms and print it with size and depth information.\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
