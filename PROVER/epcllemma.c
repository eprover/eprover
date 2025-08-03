/*-----------------------------------------------------------------------

  File  : epcllemma.c

  Author: Stephan Schulz

  Contents

  Read a PCL protocol and suggest certain clauses as lemmas.

  Copyright 2003-2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Jun 13 17:35:26 CEST 2003

  -----------------------------------------------------------------------*/

#include <stdio.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_tempfile.h>
#include <cio_signals.h>
#include <pcl_lemmas.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "epcllemma"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_SILENT,
   OPT_OUTPUTLEVEL,
   OPT_TPTP_PRINT,
   OPT_TPTP_FORMAT,
   OPT_TSTP_PRINT,
   OPT_TSTP_FORMAT,
   OPT_LOP_PRINT,
   OPT_ITERATIVE_LEMMAS,
   OPT_RECURSIVE_LEMMAS,
   OPT_FLAT_LEMMAS,
   OPT_ABS_LEMMA_LIMIT,
   OPT_REL_LEMMA_LIMIT,
   OPT_ABS_LEMQUAL_LIMIT,
   OPT_REL_LEMQUAL_LIMIT,
   OPT_LEMMA_TREE_BASE_W,
   OPT_LEMMA_SIZE_BASE_W,
   OPT_LEMMA_ACT_PM_W,
   OPT_LEMMA_O_GEN_W,
   OPT_LEMMA_ACT_SIMPL_W,
   OPT_LEMMA_PAS_SIMPL_W,
   OPT_NO_REFERENCE_WEIGHTS,
   OPT_LEMMA_HORN_BONUS,
   OPT_INITIAL_WEIGHT,
   OPT_QUOTE_WEIGHT,
   OPT_PARAMOD_WEIGHT,
   OPT_ERESOLUTION_WEIGHT,
   OPT_EFACTORING_WEIGHT,
   OPT_SIMPLIFYREFLECT_WEIGHT,
   OPT_ACRESOLUTION_WEIGHT,
   OPT_REWRITE_WEIGHT,
   OPT_UREWRITE_WEIGHT,
   OPT_CLAUSENORMALIZE_WEIGHT,
   OPT_SPLITCLAUSE_WEIGHT
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

   {OPT_OUTPUTLEVEL,
    'l', "output-level",
    ReqArg, NULL,
    "Select an output level, greater values imply more verbose "
    "output. Level 0 produces nearly no output, level 1 and 2 will"
    " print just lemmas, level 3 and higher will give a full protocol "
    "with lemmas marked as such."},

   {OPT_TPTP_PRINT,
    '\0', "tptp-out",
    NoArg, NULL,
    "Print lemma sets in TPTP-2 format instead of lop."},

   {OPT_TPTP_FORMAT,
    '\0', "tptp-format",
    NoArg, NULL,
    "Equivalent to --tptp-out (supplied for consistency in the E toolchain."},

   {OPT_TSTP_PRINT,
    '\0', "tstp-out",
    NoArg, NULL,
    "Print lemma sets in TPTP-3 (TSTP) format instead of lop."},

   {OPT_TSTP_FORMAT,
    '\0', "tstp-format",
    NoArg, NULL,
    "Equivalent to --tstp-out (supplied for consistency in the E toolchain."
    " Note that this does not enable parsing of TPTP-3 proofs."},

   {OPT_LOP_PRINT,
    '\0', "lop-out",
    NoArg, NULL,
    "Print output in LOP format. This is only useful for output level 1, "
    "as LOP has no way of distinguishing lemmas and other clauses/formulas."
    " It also is problematic for non-CNF first order proofs, as LOP has "
    "no good syntax for full first-order formulae."},

   {OPT_ITERATIVE_LEMMAS,
    'i', "iterative-lemmas",
    NoArg, NULL,
    "Use a simple iterative lemma generation algorithm that will traverse the PCL"
    " listing in a topological ordering (from axioms to leaf nodes),"
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
    'A', "max-lemmas",
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

   {OPT_LEMMA_TREE_BASE_W,
    'b', "lemma-tree-base-weight",
    ReqArg, NULL,
    "Set the base weight for the influence of references in the lemma "
    "quality evaluation. The larger it is in relation to the "
    "inference weights (below), the less important is the actual "
    "number of references. If you want to use only the lemma size, set this to "
    "1 and the individual reference weights to 0 (using e.g. the "
    "--no-reference-weights option)."},
   {OPT_LEMMA_SIZE_BASE_W,
    '\0', "lemma-size-base-weight",
    ReqArg, NULL,
    "Set the base weight for the influence of size in the lemma "
    "quality evaluation. The larger this is, the less important the actual "
    "size of the lemma becomes."},
   {OPT_LEMMA_ACT_PM_W,
    'a', "active-pm-weight",
    ReqArg, NULL,
    "Determine the weight to use for each use of the clause as an active"
    " paramodulation partner (i.e. in a conditional rewrite step (if you "
    "follow a strictly equational paradigm (which I do)))."},

   {OPT_LEMMA_O_GEN_W,
    'g',"generating-inference-weight",
    ReqArg, NULL,
    "Detemine the weight to give to references in generating infences other than "
    "active paramodulation inferences."},

   {OPT_LEMMA_ACT_SIMPL_W,
    'S',"simplifying-weight",
    ReqArg, NULL,
    "Determine the weight to give to a reference to a clause used as a "
    "simplifying clause."},

   {OPT_LEMMA_PAS_SIMPL_W,
    'p',"simplified-weight",
    ReqArg, NULL,
    "Determine the weight of a reference where a clause is "
    "being simplified."},

   {OPT_NO_REFERENCE_WEIGHTS,
    'N',"no-reference-weights",
    NoArg, NULL,
    "Set all the weights given to references to 0. If the base weight "
    "(see above) is not 0, this leads to a pure size/prooftree evaluation."},

   {OPT_LEMMA_HORN_BONUS,
    'H',"horn-bonus",
    ReqArg, NULL,
    "Weight factor to apply to the evaluation of Horn clauses. Use 1 to be"
    " fair, 2.5 if you think Horn clauses are 2.5 times more dandy than "
    "non-Horn clauses. Yes, nice lemmas _are_ amatter of taste ;-)."},

   {OPT_INITIAL_WEIGHT,
    '\0', "pcl-initial-weight",
    ReqArg, NULL,
    "Set the weight of an 'initial' pseudo-inference for computing the weight of "
    "a PLC proof tree. This is probably best left untouched."},

   {OPT_QUOTE_WEIGHT,
    '\0', "pcl-quote-weight",
    ReqArg, NULL,
    "Set the weight of a  quote  pseudo-inference for computing the weight of "
    "a PLC proof tree. This is probably best left untouched."},

   {OPT_PARAMOD_WEIGHT,
    '\0', "pcl-paramod-weight",
    ReqArg, NULL,
    "Set the weight of a paramodulation inference for computing the weight"
    " of a PLC proof tree."},

   {OPT_ERESOLUTION_WEIGHT,
    '\0', "pcl-eres-weight",
    ReqArg, NULL,
    "Set the weight of an equality resolution inference for computing the"
    " weight of a PLC proof tree."},

   {OPT_EFACTORING_WEIGHT,
    '\0', "pcl-efact-weight",
    ReqArg, NULL,
    "Set the weight of an equality factoring inference for computing the"
    " weight of a PLC proof tree."},

   {OPT_SIMPLIFYREFLECT_WEIGHT,
    '\0', "pcl-sr-weight",
    ReqArg, NULL,
    "Set the weight of a simplify-reflect inference for computing the"
    " weight of a PLC proof tree."},

   {OPT_ACRESOLUTION_WEIGHT,
    '\0', "pcl-acres-weight",
    ReqArg, NULL,
    "Set the weight of an AC resolution inference for computing the"
    " weight of a PLC proof tree."},

   {OPT_REWRITE_WEIGHT,
    '\0', "pcl-rw-weight",
    ReqArg, NULL,
    "Set the weight of a rewrite inference for computing the"
    " weight of a PLC proof tree."},

   {OPT_UREWRITE_WEIGHT,
    '\0', "pcl-urw-weight",
    ReqArg, NULL,
    "Set the weight of a underspecified rewrite inference for computing the"
    " weight of a PLC proof tree. Such an inference describes an"
    " unspecified number of rewrite steps using the same unit clause as a"
    " rewrite rule. Normal E PCL listings should no longer contain"
    " such inferences."},

   {OPT_CLAUSENORMALIZE_WEIGHT,
    '\0', "pcl-cn-weight",
    ReqArg, NULL,
    "Set the weight of a clause normalization inference for computing the"
    " weight of a PLC proof tree. This is probably best left alone, since"
    " most clause normalization is implicit anyways."},

   {OPT_SPLITCLAUSE_WEIGHT,
    '\0', "pcl-split-weight",
    ReqArg, NULL,
    "Set the weight of a splitting pseudo-inference for computing the"
    " weight of a PLC proof tree."},

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
bool       app_encode        = false;
float      min_quality       = 100;
float      min_quality_rel   = 0.3;
bool       min_quality_rel_p = false;
OutputFormatType outputformat = pcl_format;


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
   int             i;

   assert(argv[0]);
#ifdef STACK_SIZE
   INCREASE_STACK_SIZE;
#endif
   InitIO(NAME);

   /* TPTPFormatPrint = true; */
   /* We need consistent name->var mappings here because we
      potentially read the compressed input format. */
   ClausesHaveLocalVariables = false;

   iw = InferenceWeightsAlloc();
   lp = LemmaParamAlloc();
   state = process_options(argc, argv);

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

   if(max_lemmas_rel_p)
   {
      max_lemmas = PCLProtStepNo(prot) * max_lemmas_rel +0.99;
   }
   printf(COMCHAR" Selecting at most %ld lemmas\n", max_lemmas);
   if(min_quality_rel_p)
   {
      PCLProtComputeProofSize(prot, iw, false);
      step = PCLProtComputeLemmaWeights(prot, lp);
      min_quality = step?(step->lemma_quality*min_quality_rel):0;
   }
   printf(COMCHAR" Minimum lemma quality: %f\n", min_quality);

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
   VERBOUT("Lemmas selected\n");
   switch(OutputLevel)
   {
   case 0:
         break;
   case 1:
   case 2:
         PCLProtPrintPropClauses(GlobalOut, prot, PCLIsLemma, outputformat);
         break;
   default:
         PCLProtPrint(GlobalOut, prot, outputformat);
         break;
   }

   PCLProtFree(prot);
   InferenceWeightsFree(iw);
   LemmaParamFree(lp);
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
      case OPT_TPTP_PRINT:
      case OPT_TPTP_FORMAT:
            outputformat = tptp_format;
            break;
      case OPT_TSTP_PRINT:
      case OPT_TSTP_FORMAT:
            outputformat = tstp_format;
            break;
      case OPT_LOP_PRINT:
            outputformat = lop_format;
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
      case OPT_LEMMA_TREE_BASE_W:
            lp->tree_base_weight = CLStateGetIntArg(handle, arg);
            break;
      case OPT_LEMMA_SIZE_BASE_W:
            lp->size_base_weight = CLStateGetIntArg(handle, arg);
            break;
      case OPT_LEMMA_ACT_PM_W:
            lp->act_pm_w = CLStateGetFloatArg(handle, arg);
            break;
      case OPT_LEMMA_O_GEN_W:
            lp->o_gen_w = CLStateGetFloatArg(handle, arg);
            break;
      case OPT_LEMMA_ACT_SIMPL_W:
            lp->act_simpl_w = CLStateGetFloatArg(handle, arg);
            break;
      case OPT_LEMMA_PAS_SIMPL_W:
            lp->pas_simpl_w = CLStateGetFloatArg(handle, arg);
            break;
      case OPT_NO_REFERENCE_WEIGHTS:
            lp->act_pm_w    = 0;
            lp->o_gen_w     = 0;
            lp->pas_simpl_w = 0;
            lp->pas_simpl_w = 0;
            break;
      case OPT_LEMMA_HORN_BONUS:
            lp->horn_bonus = CLStateGetFloatArg(handle, arg);
            break;
      case OPT_INITIAL_WEIGHT:
            (*iw)[PCLOpInitial] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_QUOTE_WEIGHT:
            (*iw)[PCLOpQuote] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PARAMOD_WEIGHT:
            (*iw)[PCLOpParamod] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_ERESOLUTION_WEIGHT:
            (*iw)[PCLOpEResolution] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_EFACTORING_WEIGHT:
            (*iw)[PCLOpEFactoring] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_SIMPLIFYREFLECT_WEIGHT:
            (*iw)[PCLOpSimplifyReflect] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_ACRESOLUTION_WEIGHT:
            (*iw)[PCLOpACResolution] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_REWRITE_WEIGHT:
            (*iw)[PCLOpRewrite] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_UREWRITE_WEIGHT:
            (*iw)[PCLOpURewrite] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_CLAUSENORMALIZE_WEIGHT:
            (*iw)[PCLOpClauseNormalize] = CLStateGetIntArg(handle, arg);
            break;
      case OPT_SPLITCLAUSE_WEIGHT:
            (*iw)[PCLOpSplitClause] = CLStateGetIntArg(handle, arg);
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
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\
Copyright (C) 2003-2005 by Stephan Schulz, " STS_MAIL "\n       \
\n                                                                      \
This program is a part of the support structure for the E equational\n  \
theorem prover. You can find the latest version of the E distribution\n \
as well as additional information at\n"
           E_URL
           "\n\n"
           "This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n  \
the Free Software Foundation; either version 2 of the License, or\n     \
(at your option) any later version.\n                                   \
\n                                                                      \
This program is distributed in the hope that it will be useful,\n       \
but WITHOUT ANY WARRANTY; without even the implied warranty of\n        \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n         \
GNU General Public License for more details.\n                          \
\n                                                                      \
You should have received a copy of the GNU General Public License\n     \
along with this program (it should be contained in the top level\n      \
directory of the distribution in the file COPYING); if not, write to\n  \
the Free Software Foundation, Inc., 59 Temple Place, Suite 330,\n       \
Boston, MA  02111-1307 USA\n                                            \
\n                                                                      \
The original copyright holder can be contacted as\n                     \
\n"
           STS_SNAIL
           "\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
