/*-----------------------------------------------------------------------

File  : checkproof.c

Author: Stephan Schulz

Contents

  Read a PCL protocol and try to verify it using a selected prover.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Apr  7 16:14:02 MET DST 2000
    New

-----------------------------------------------------------------------*/

#include <stdio.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_tempfile.h>
#include <cio_signals.h>
#include <pcl_proofcheck.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "checkproof"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_PROVERTYPE,
   OPT_EXECUTABLE,
   OPT_TIME_LIMIT
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
    " output. At the moment, level 0 only prints the result, level 1"
    " prints inference steps as they are verified, level 2 prints"
    " prover commands issued, and level 3 prints all prover output"
    " (which may be very little)"},

   {OPT_PROVERTYPE,
    'p', "prover-type",
    ReqArg, NULL,
    "Set the type of the prover to use for proof"
    " verification. Determines problem syntax, options, and check for"
    " success. Supported options at are  'E' (the"
    " default),'Otter' 'SPASS', and 'scheme-setheo' (not yet"
    " implemented). SPASS support is only tested with SPASS"
    " 0.55 and may fail if the problem contains identifiers reserved "
    "by SPASS. There have been some supple syntax changes, so more"
    " recent SPASS versions will probably fail as well."},

   {OPT_EXECUTABLE,
    'x', "executable",
    ReqArg, NULL,
    "Give the name under which the prover can be called. If no"
    " executable is given, checkproof will guess a name based on the"
    " type of the prover. This guess may be way off!"},

   {OPT_TIME_LIMIT,
    't', "prover-cpu-limit",
    ReqArg, NULL,
    "Limit the CPU time prover may spend on a single step. Default is"
    " 10 seconds."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char       *outname    = NULL;
long       time_limit  = 10;
char       *executable = NULL;
ProverType prover      = EProver;
bool       app_encode  = false;


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
   PCLProt_p   prot;
   long        steps,res,unchecked;
   int         i;

   assert(argv[0]);

   InitIO(NAME);
   atexit(TempFileCleanup);

   ESignalSetup(SIGTERM);
   ESignalSetup(SIGINT);

   OutputFormat = TPTPFormat;
   /* We need consistent name->var mappings here because we
      potentially read the compressed input format. */
   ClausesHaveLocalVariables = false;

   state = process_options(argc, argv);

   OpenGlobalOut(outname);
   prot = PCLProtAlloc();

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   steps = 0;
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], true, NULL, true);
      ScannerSetFormat(in, TPTPFormat);
      steps+=PCLProtParse(in, prot);
      CheckInpTok(in, NoToken);
      DestroyScanner(in);
   }
   VERBOUT2("PCL input read\n");

   res = PCLProtCheck(prot,prover,executable,time_limit, &unchecked);

   fprintf(GlobalOut,
         COMCHAR" Successfully checked %ld of %ld steps (%ld unchecked): ",
         res, steps, unchecked);

   if(res==steps)
   {
      fprintf(GlobalOut, " Proof verified!\n");
   }
   else if((res+unchecked) == steps)
   {
      fprintf(GlobalOut, " Proof partially verified!\n");
   }
   else
   {
      fprintf(GlobalOut,
         " Failed to verify proof!\n");
   }

   PCLProtFree(prot);

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
       break;
      case OPT_PROVERTYPE:
       if(strcmp(arg, "E")==0)
       {
          prover = EProver;
       }
       else if(strcmp(arg, "Otter")==0)
       {
          prover = Otter;
          OutputFormat = LOPFormat;
          EqnUseInfix = false;
       }
       else if(strcmp(arg, "SPASS")==0)
       {
          prover = Spass;
          OutputFormat = LOPFormat;
          EqnUseInfix = false;
       }
       else if(strcmp(arg, "scheme-setheo")==0)
       {
          prover = Setheo;
       }
       else
       {
          Error("Option -p (--prover-type) requires E, "
           "Otter, SPASS or scheme-setheo as an argument",
           USAGE_ERROR);
       }
       break;
      case OPT_EXECUTABLE:
       executable = arg;
       break;
      case OPT_TIME_LIMIT:
       time_limit = CLStateGetIntArg(handle, arg);
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
Read an UPCL2 protocol and verify the inferences using one of a\n\
varity of external provers.\n\
\n\
This is a _very_ experimental program. Passing checkproof does\n\
indicate that all inferences in an UPCL2 protocol are correct\n\
(i.e. that the conclusion is logically implied by the premisses) -\n\
that is, if you believe that the transformation process and the used\n\
prover are correct. However, checkproof will e.g. gladly show that the\n\
empty proof protocol does not contain any buggy steps.\n\
\n\
If a proof protocol fails to pass this test, the proof may still be\n\
correct. Due to e.g. incomplete strategies (this applies in particular\n\
to Otter), build-in limits (Otter), and bugs in the prover (potentially\n\
all systems, but observed in SPASS 0.55), a prover might fail to\n\
verify a correct step. Moreover, due to the different strategies,\n\
calculi, and in particular different term orderings chosen by the\n\
systems, a single UPCL2 inference may result in a proof problem that\n\
is very hard to verify for other provers. However, if a proof step is\n\
rejected by more than one system, you should probably look at this\n\
step in detail.\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
