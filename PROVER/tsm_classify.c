/*-----------------------------------------------------------------------

File  : tsm_classify.c

Author: Stephan Schulz

Contents

  Read a set of annotated terms (the training
  set) and a second set of annotated terms (the test set). Build a tsm
  from the first set and evaluated the second set.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Aug 12 14:55:33 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_tempfile.h>
#include <cle_classification.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "tsm_classify"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_OUTPUTLEVEL,
   OPT_OUTPUT,
   OPT_INDEXFUN,
   OPT_INDEXDEPTH,
   OPT_TSMTYPE
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

   {OPT_OUTPUTLEVEL,
    'l', "output-level",
    ReqArg, NULL,
    "Select an output level, greater values imply more verbose "
    "output."},

   {OPT_OUTPUT,
    'o', "output-file",
    ReqArg, NULL,
    "Redirect output into the named file."},

   {OPT_INDEXFUN,
    'i', "index-type",
    ReqArg, NULL,
    "Select an index function type. Run " NAME " -iNone for a list of"
    " possible functions."},

   {OPT_INDEXDEPTH,
    'd', "index-depth",
    ReqArg, NULL,
    "Set the term top depth for the index. A depth of 0 denotes "
    "dynamic depth selection."},

   {OPT_TSMTYPE,
    't', "tsm-type",
    ReqArg, NULL,
    "Select the type of the TSM (Flat, Recursive, Reccurent or RecLocal)."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char      *outname = NULL;
int       index_type = IndexArity;
long      index_depth = 1;
int       tsm_type = TSMTypeRecursive;
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
   CLState_p      state;
   Scanner_p      in;
   char*          infile;
   TypeBank_p     typebank;
   Sig_p          sig;
   TB_p           bank;
   AnnoSet_p      training_set, test_set;
   FlatAnnoSet_p  ftrain_set, ftest_set;
   DDArray_p      weights;
   TSMAdmin_p     admin;
   long           successes, nodes;
   PatternSubst_p subst;

   assert(argv[0]);

   InitIO(NAME);
   OutputLevel = 1;

   state = process_options(argc, argv);

   OpenGlobalOut(outname);

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   infile = TempFileName();
   ConcatFiles(infile, state->argv);

   in = CreateScanner(StreamTypeFile, infile, true, NULL);

   typebank = TypeBankAlloc();
   sig = SigAlloc(typebank);
   bank = TBAlloc(sig);
   AcceptInpId(in, "Training");
   AcceptInpTok(in, Colon);
   training_set = AnnoSetParse(in, bank, 2); /* (Sources, Class) ->2 */
   AcceptInpTok(in, Fullstop);
   AnnoSetFlatten(training_set, ANNOTATIONS_MERGE_ALL);

   AcceptInpId(in, "Test");
   AcceptInpTok(in, Colon);
   test_set = AnnoSetParse(in, bank, 2);
   AcceptInpTok(in, Fullstop);
   AnnoSetFlatten(test_set, ANNOTATIONS_MERGE_ALL);
   DestroyScanner(in);
   TempFileRemove(infile);
   FREE(infile);
   SigSetAllSpecial(sig, true);

   ftrain_set = FlatAnnoSetAlloc();
   ftest_set  = FlatAnnoSetAlloc();
   weights = DDArrayAlloc(2,6);
   DDArrayAssign(weights, 0, 1);
   FlatAnnoSetTranslate(ftrain_set, training_set, weights->array);
   FlatAnnoSetTranslate(ftest_set, test_set, weights->array);
   DDArrayFree(weights);
   admin = TSMAdminAlloc(sig, tsm_type);

   VERBOUT("Parsing and preprocessing done\n");

   subst = PatternDefaultSubstAlloc(sig);
   AnnoSetComputePatternSubst(subst, training_set);
   AnnoSetComputePatternSubst(subst, test_set);
   VERBOUT("PatternSubst generated\n");
   TSMAdminBuildTSM(admin, ftrain_set, index_type,
          index_depth, subst);
   admin->eval_limit = TSMComputeClassificationLimit(admin,
                       ftrain_set);
   admin->unmapped_eval = TSMComputeAverageEval(admin,ftrain_set);
   VERBOUT("TSM build\n");

   successes = TSMClassifySet(admin, ftest_set);
   nodes     = FlatAnnoSetSize(ftest_set);
   fprintf(GlobalOut, "% ld terms, %ld successes, %5.3f percent\n", nodes,
      successes, 100.0*(double)successes/(double)nodes);

   TSMAdminFree(admin);
   VERBOUT("TSM freed\n");
   FlatAnnoSetFree(ftrain_set);
   FlatAnnoSetFree(ftest_set);
   PatternSubstFree(subst);
   AnnoSetFree(training_set);
   AnnoSetFree(test_set);
   bank->sig = 0;
   TBFree(bank);
   SigFree(sig);
   TypeBankFree(typebank);

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
      case OPT_OUTPUTLEVEL:
       OutputLevel = CLStateGetIntArg(handle, arg);
       break;
      case OPT_OUTPUT:
       outname = arg;
       break;
      case OPT_INDEXFUN:
       index_type = GetIndexType(arg);
       printf(COMCHAR" Index type: %d\n", index_type);
       if(index_type == -1)
       {
          DStr_p err = DStrAlloc();
          DStrAppendStr(err,
              "Wrong argument to option -i "
              "(--index-type). Possible "
              "values: ");
          DStrAppendStrArray(err, IndexFunNames, ", ");
          Error(DStrView(err), USAGE_ERROR);
          DStrFree(err);
       }
       else if(index_type == IndexNoIndex)
       {
          Error("Sorry, need to select a real index type!",
           USAGE_ERROR);
       }
       break;
      case OPT_INDEXDEPTH:
       index_depth = CLStateGetIntArg(handle, arg);
       if(index_depth < 0)
       {
          Error("Argument for -d (--index-depth) has to be an "
           "integer number greater than or equal to 0.",
           USAGE_ERROR);
       }
       break;
      case OPT_TSMTYPE:
       tsm_type = GetTSMType(arg);
       if((tsm_type == TSMTypeNoType) || (tsm_type == -1))
       {
          Error("Only Flat, Recursive, Recurrent and RecLocal allowed as"
           "TSM types in option -t (--tsm-type)",
           USAGE_ERROR);
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
Parse a classification problem specification file and return\n\
results. This is an experimental program and does not have all the\n\
usual error checking and hand holding features as E proper!\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
