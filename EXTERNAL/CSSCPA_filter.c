/*-----------------------------------------------------------------------

File  : CSSCPA_filter.c

Author: Stephan Schulz, Geoff Sutcliffe

Contents

  Do CSSCPA stuff (read clauses, accept them into the state if they
  are necessary or improve it, reject them otherwise).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Apr 10 15:28:48 MET DST 2000
    New

-----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cex_csscpa.h>
#include <stdio.h>
#include <e_version.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "CSSCPA_filter"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_RANT
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

    {OPT_SILENT,
    's', "silent",
    NoArg, NULL,
    "Equivalent to --output-level=0."},

   {OPT_OUTPUTLEVEL,
    'l', "output-level",
    ReqArg, NULL,
    "Select an output level, greater values imply more verbose"
    " output. At the moment, level 0 only prints the result of each"
    " statement, and level 1 also prints what happens to each"
    " clause."},

    {OPT_RANT,
    'r', "rant-about-input-buffering",
     OptArg, "666",
    "Tell the program how much you hate to include the "
     "'Please'-sequence in the input. The optional argument is the "
     " rant-intensity."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char   *outname   = NULL;
bool   app_encode = false;

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
   Scanner_p     in;
   CLState_p     state;
   CSSCPAState_p procstate;
   int           i;

   assert(argv[0]);

   InitIO(NAME);

   state = process_options(argc, argv);

   OpenGlobalOut(outname);

   OutputFormat = TPTPFormat;

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   procstate = CSSCPAStateAlloc();

   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], true, NULL,true);
      ScannerSetFormat(in, TSTPFormat);
      CSSCPALoop(in, procstate);
      DestroyScanner(in);
   }
   fprintf(GlobalOut, "\n"COMCHAR" Resulting clause set:\n");
   ClauseSetTSTPPrint(GlobalOut, procstate->pos_units, true);
   ClauseSetTSTPPrint(GlobalOut, procstate->neg_units, true);
   ClauseSetTSTPPrint(GlobalOut, procstate->non_units, true);

   CSSCPAStateFree(procstate);
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
      case OPT_VERSION:
       printf(NAME " " VERSION "\n");
       exit(NO_ERROR);
      case OPT_OUTPUT:
       outname = arg;
       break;
      case OPT_SILENT:
            OutputLevel = 0;
            break;
      case OPT_OUTPUTLEVEL:
            OutputLevel = CLStateGetIntArg(handle, arg);
       if(OutputLevel>1)
       {
          Error("Option -l (--output-level) accepts only 0 or 1"
           "for CSSCPA_filter",
                     USAGE_ERROR);
       }
            break;
      case OPT_RANT:
       if(CLStateGetIntArg(handle, arg)!=0)
       {
          fprintf(stderr, "Improve it yourself, mate. The code is"
             " free.\n");
       }
       else
       {
          fprintf(stderr, "You call that a rant????\n");
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
Usage: " NAME " [options] [files]\n\
\n\
Read a list of CSSCPA statements, print the resulting clause set on\n\
termination. A CSSCPA statement is either 'accept: <clause>' or\n\
'check: <clause>', where <clause> is a clause in TPTP format. Clauses\n\
prepended by 'accept' are always integrated into the current clause\n\
set unless they are subsumed or tautological. Clauses prepended by\n\
'check' are only integrated if they subsume clauses with a total\n\
weight that is higher than their own weight. Subsumed clauses are\n\
always removed from the clause set.\n\
\n\
After every statement, clause count, literal count and total clause\n\
weight are printed to the selected output channel (stdout by\n\
default). If you need these results immediately, you'll have to beg\n\
the progam by including the sequence\n\
\n\
Please process clauses now, I beg you, great shining CSSCPA,\n\
wonder of the world, most beautiful program ever written.\n\
\n\
to overcome CLIB's input buffering.\n\
\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
