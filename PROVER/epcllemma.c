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
   OPT_SILENT,
   OPT_TPTP_PRINT,
   OPT_TPTP_FORMAT,
   OPT_ITERATIVE_LEMMAS,
   OPT_RECURSIVE_LEMMAS,
   OPT_FLAT_LEMMAS,
   OPT_ABS_LEMMA_LIMIT,
   OPT_REL_LEMMA_LIMIT,
   OPT_ABS_LEMQUAL_LIMIT,
   OPT_REL_LEMQUAL_LIMIT
}OptionCodes;


typedef enum 
{
   LIterative,
   LRecursive,
   LFlat
}LemmaAlgorithm;


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

   {OPT_ITERATIVE_LEMMAS,
    'l', "iterative-lemmas",
    NoArg, NULL,
    "Use a simple iterative lemma generation algorithm that will traverse the PCL"
    " listing in a topological ordering (from axioms to leave nodes),"
    " picking out lemmas that reach a "
    "certain score. Good for getting a reasonably even distribution of "
    "lemmata for proof presentation. This is the default behaviour (the "
    "option exists just for documentation purposes)."},

   {OPT_RECURSIVE_LEMMAS,
    'r', "recursive-lemmas",
    NoArg, NULL,
    "Use a recursive lemma generation algorithm that will pick out the lemma"
    " with the highest score, recompute scores, and repeat for a given "
    "number of steps. This may lead to very irregular proofs (because"
    " later lemmata may change the score of previous ones), but ensures"
    " that the lemma with the highest score is chosen."},

   {OPT_FLAT_LEMMAS,
    'f', "flat-lemmas",
    NoArg, NULL,
    "Compute lemma scores once and pick the N lemmas with the highest "
    "score. These are bound to be nodes that are close to the "
    "derivation graph boundary, so they are not necessarily good for"
    " strucuring the proof. They may be good for theory exploration, "
    "though. This algorithm is also O(n) in the number of PCL steps "
    "(well, there is a small O(log(n)) component, but its close enough), "
    "while the others may end up O(n^2) in the (unexpected) worst case."},

   {OPT_ABS_LEMMA_LIMIT,
    'a', "max-lemmas",
    ReqArg, NULL,
    "Set the maximal number of lemmas to be selected absolutely."},

   {OPT_REL_LEMMA_LIMIT,
    'R', "max-lemmas-rel",
    ReqArg, NULL,
    "Set the maximal number of lemmas to be selected as a fraction "
    "of the total number of PCL steps in the protocol (always "
    "overwritten if an absolute value is also provided)."},

   {OPT_ABS_LEMQUAL_LIMIT,
    'q', "min-lemma-quality",
    ReqArg, NULL,
    "Set a mimimum lemma score absolutely. Steps with this or a "
    "higher score become lemmata unless another limit prohibits "
    "that.",
   },
   
   {OPT_REL_LEMQUAL_LIMIT,
    'Q', "min-lemma-quality-rel",
    ReqArg, NULL,
    "Set a mimimum lemma score as a fraction of the best possible "
    "lemma score in the proof tree."
   },

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char       *outname          = NULL;
long       time_limit        = 10;
char       *executable       = NULL;
LemmaAlgorithm algo          = LIterative;
InferenceWeight_p iw         = NULL;
LemmaParam_p      lp         = NULL;
long       max_lemmas        = 0;
float      max_lemmas_rel    = 0.001;
bool       max_lemmas_rel_p  = true;
float      min_quality       = 100;
float      min_quality_rel   = 0.3;
bool       min_quality_rel_p = false;

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
   PCLStep_p       step;
   long            steps;
   int             i;

   assert(argv[0]);

   InitOutput();
   InitError(NAME);
   
   /* TPTPFormatPrint = true; */
   /* We need consistent name->var mappings here because we
      potentially read the compressed input format. */
   ClausesHaveLocalVariables = false;

   iw = InferenceWeightsAlloc();
   lp = LemmaParamAlloc();
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
   
   if(max_lemmas_rel_p)
   {     
      max_lemmas = PCLProtStepNo(prot) * max_lemmas_rel +0.99;
   }
   printf("# Selecting at most %ld lemmas\n", max_lemmas);
   if(min_quality_rel_p)
   {
      PCLProtComputeProofSize(prot, iw, false);
      step = PCLProtComputeLemmaWeights(prot, lp);      
      min_quality = step?(step->lemma_quality*min_quality_rel):0;
   }
   printf("# Minimum lemma quality: %f\n", min_quality);

   switch(algo)
   {
   case LRecursive:
	 PCLProtRecFindLemmas(prot, lp, iw, max_lemmas, min_quality);
	 break;
   case LIterative:
	 PCLProtSeqFindLemmas(prot, lp, iw, max_lemmas, min_quality);
	 break;
   case LFlat:
	 PCLProtFlatFindLemmas(prot, lp, iw, max_lemmas, min_quality);
	 break;
   default:
	 assert(false && "Unknown algorithm type ???");
	 break;
   }
   PCLProtPrintPropClauses(GlobalOut, prot, PCLIsLemma, true);
   
   PCLProtFree(prot);
   InferenceWeightsFree(iw);
   LemmaParamFree(lp);
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
   
   assert(iw);
   assert(lp);

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
      case OPT_ITERATIVE_LEMMAS:
	    algo = LIterative;
	    break;
      case OPT_RECURSIVE_LEMMAS:
	    algo = LRecursive;
	    break;
      case OPT_FLAT_LEMMAS:
	    algo = LFlat;
	    break;
      case OPT_ABS_LEMMA_LIMIT:
	    max_lemmas = CLStateGetIntArg(handle, arg);
	    max_lemmas_rel_p = false;
	    break;
      case OPT_REL_LEMMA_LIMIT:
	    max_lemmas_rel = CLStateGetFloatArg(handle, arg);
	    max_lemmas_rel_p = true;
	    break;
      case OPT_ABS_LEMQUAL_LIMIT:
	    min_quality = CLStateGetFloatArg(handle, arg);	    
	    min_quality_rel_p = false;
	    break;
      case OPT_REL_LEMQUAL_LIMIT:
	    min_quality_rel = CLStateGetFloatArg(handle, arg);	    
	    min_quality_rel_p = true;
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


