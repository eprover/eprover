/*-----------------------------------------------------------------------

File  : epcllemma.c

Author: Stephan Schulz

Contents
 
  Read a PCL protocol and suggest certain clauses as lemmas.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Jun 13 17:35:26 CEST 2003
    New (from epclanalyse.c)

-----------------------------------------------------------------------*/

#include <stdio.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_tempfile.h>
#include <cio_signals.h>
#include <pcl_lemmas.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "epcllemma"
#define VERSION "0.1"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_SILENT
,
   OPT_TPTP_PRINT,
   OPT_TPTP_FORMAT
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
   
   {OPT_TPTP_PRINT,
    '\0', "tptp-out",
    NoArg, NULL,
    "Print lemma sets in TPTP format instead of lop."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp-format",
    NoArg, NULL,
    "Equivalent to --tptp-out (supplied for consistency in the E toolchain."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char       *outname    = NULL;
long       time_limit  = 10;
char       *executable = NULL;


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
   CLState_p       state;
   Scanner_p       in; 
   PCLProt_p       prot;
   PCLStep_p       best_lemma;
   long            steps;
   int             i;
   InferenceWeight_p iw;
   LemmaParam_p      lp;

   assert(argv[0]);

   InitOutput();
   InitError(NAME);
   atexit(TempFileCleanup);

   ESignalSetup(SIGTERM);
   ESignalSetup(SIGINT);
   
   /* TPTPFormatPrint = true; */
   /* We need consistent name->var mappings here because we
      potentially read the compressed input format. */
   ClausesHaveLocalVariables = false;

   state = process_options(argc, argv);

   GlobalOut = OutOpen(outname);
   prot = PCLProtAlloc();

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   steps = 0;
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i] , true, NULL);
      ScannerSetFormat(in, TPTPFormat);
      steps+=PCLProtParse(in, prot);
      CheckInpTok(in, NoToken);
      DestroyScanner(in); 
   }
   VERBOUT2("PCL input read\n");
   iw = InferenceWeightsAlloc();
   lp = LemmaParamAlloc();
   PCLProtResetTreeData(prot, false);
   PCLProtUpdateRefs(prot);
   PCLProtComputeProofSize(prot, iw, false);
   best_lemma = PCLProtComputeLemmaWeights(prot, lp);   
   /*  PCLProtPrintExtra(GlobalOut, prot, true); */
   if(best_lemma)
   {
      PCLProtSeqFindLemmas(prot, lp, iw, best_lemma->lemma_quality);
      PCLProtPrintPropClauses(GlobalOut, prot, PCLIsLemma, true);
   }
   
   InferenceWeightsFree(iw);
   LemmaParamFree(lp);
   PCLProtFree(prot);
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
      case OPT_SILENT:
	    OutputLevel = 0;
	    break;
      case OPT_TPTP_PRINT:
	    TPTPFormatPrint = true;
	    EqnFullEquationalRep = false;
	    EqnUseInfix = false;
	    break;
      case OPT_TPTP_FORMAT:
	    TPTPFormatPrint = true;
	    EqnFullEquationalRep = false;
	    EqnUseInfix = false;
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
   fprintf(out, 
	   "\n"
	   "\n"
NAME " " VERSION "\n"
	   "\n"
"Usage: " NAME " [options] [files]\n"
"\n"
"Read an UPCL2 protocol and suggest certain steps as lemmas."
"\n");
   PrintOptions(stdout, opts);
   fprintf(out, "\n\
Copyright 2003 by Stephan Schulz, " STS_MAIL "\n\
\n\
This program is a part of the support structure for the E equational\n\
theorem prover. You can find the latest version of the E distribution\n\
as well as additional information at\n"
E_URL
"\n\n"
"This program is free software; you can redistribute it and/or modify\n\
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


