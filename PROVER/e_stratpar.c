/*-----------------------------------------------------------------------

  File  : e_stratpar.c

  Author: Stephan Schulz

  Contents

  Hack for the SLB category of CASC-2017 - run 8 E's in parallel
  on a given problem

  Copyright 2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jul 18 22:54:25 CEST 2017

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

#define NAME         "e_stratpar"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_CPU_LIMIT,
   OPT_DUMMY
}OptionCodes;

ProblemType problemType  = PROBLEM_NOT_INIT;

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

   {OPT_CPU_LIMIT,
    '\0', "cpu-limit",
    OptArg, "300",
    "Limit the cpu time the prover should run. The optional argument "
    "is the CPU time in seconds."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};


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
   CLState_p        state;
   char             *prover    = "eprover";
   EPCtrl_p tmp;
   int i;
   char optbuffer[256];
   char namebuffer[256];

   assert(argv[0]);

   InitIO(NAME);
   HardTimeLimit=3600;

   state = process_options(argc, argv);

   if((state->argc < 1) || (state->argc > 2))
   {
      Error("Usage: e_ltb_runner <spec> [<path-to-eprover>] \n",
            USAGE_ERROR);
   }

   EPCtrlSet_p ctrl = EPCtrlSetAlloc();

   for(i=0; i<8; i++)
   {
      sprintf(optbuffer, "-xAutoSched%d -tAutoSched%d --sine", i, i);
      sprintf(namebuffer, "AutoSched%d", i);
      tmp = ECtrlCreateGeneric(prover, namebuffer, optbuffer, "",
                               HardTimeLimit/2, SecureStrdup(state->argv[0]));
      EPCtrlSetAddProc(ctrl, tmp);
   }

   while(!EPCtrlSetEmpty(ctrl))
   {
      tmp = EPCtrlSetGetResult(ctrl, false);
      if(tmp)
      {
         fprintf(GlobalOut, "%s", DStrView(tmp->output));
         break;
      }
   }
   if(!tmp)
   {
      TSTPOUT(GlobalOut, "GaveUp");
   }
   EPCtrlSetFree(ctrl, false);

   CLStateFree(state);

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
   char*  arg;

   state = CLStateAlloc(argc,argv);

   while((handle = CLStateGetOpt(state, &arg, opts)))
   {
      switch(handle->option_code)
      {
      case OPT_HELP:
            print_help(stdout);
            exit(NO_ERROR);
      case OPT_VERSION:
            fprintf(stdout, NAME " " VERSION " " E_NICKNAME "\n");
            exit(NO_ERROR);
      case OPT_CPU_LIMIT:
            HardTimeLimit = CLStateGetIntArg(handle, arg);
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
Usage: " NAME " [options] [file]\n\
\n\
Run 8 instances of E with different strategies in parallel.\n\
\n");
   PrintOptions(stdout, opts, "Options:\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
