/*-----------------------------------------------------------------------

File  : direct_examples.c

Author: Stephan Schulz

Contents

  Generate examples directly from a protocol file.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jul 23 17:46:15 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_tempfile.h>
#include <ccl_clausesets.h>
#include <pcl_analysis.h>
#include <cio_signals.h>
#include <e_version.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "direct_examples"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
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
    'V', "version",
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
double neg_proportion = 1;
long   neg_examples = 200;
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
   /* Scanner_p       in;     */
   CLState_p       state;
   Scanner_p       in;
   PCLProt_p       prot;
   long            proof_steps, neg_steps;
   int             i;

   assert(argv[0]);

   InitIO(NAME);
   ESignalSetup(SIGTERM);
   ESignalSetup(SIGINT);

   /* We need consistent name->var mappings here because we
      potentially read the compressed input format. */
   ClausesHaveLocalVariables = false;

   state = process_options(argc, argv);

   GlobalOut = OutOpen(outname);
   OpenGlobalOut(outname);
   prot = PCLProtAlloc();

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], true, NULL, true);
      ScannerSetFormat(in, TPTPFormat);
      PCLProtParse(in, prot);
      CheckInpTok(in, NoToken);
      DestroyScanner(in);
   }
   VERBOUT2("PCL input read\n");

   PCLProtStripFOF(prot);
   PCLProtResetTreeData(prot, false);
   PCLProtMarkProofClauses(prot);
   PCLProtProofDistance(prot);
   PCLProtUpdateGRefs(prot);
   proof_steps = PCLProtCountProp(prot, PCLIsProofStep);
   neg_steps = proof_steps?neg_proportion*proof_steps:neg_examples;
   PCLProtSelectExamples(prot, neg_steps);
   fprintf(GlobalOut, COMCHAR" Axioms:\n");
   PCLProtPrintPropClauses(GlobalOut, prot, PCLIsInitial, lop_format);
   fprintf(GlobalOut, ".\n\n"COMCHAR" Examples:\n");
   PCLProtPrintExamples(GlobalOut, prot);

   PCLProtFree(prot);

   CLStateFree(state);

   fflush(GlobalOut);
   OutClose(GlobalOut);
   ExitIO();

#ifdef CLB_MEMORY_DEBUG
   MemFlushFreeList();
   MemDebugPrintStats(stdout);
#endif

   return EXIT_SUCCESS;
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
Usage: " NAME " [options] [files]\n\
\n\
Parse a full PCL listing (possibly\n\
spread over multiple files), and generate training examples\n\
corresponding to the selected clauses.\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
