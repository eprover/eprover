/*-----------------------------------------------------------------------

File  : ekb_delete.c

Author: Stephan Schulz

Contents

  Delete a training example from the knowledge base.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 28 16:21:33 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include <cio_commandline.h>
#include <cio_output.h>
#include <cle_kbinsert.h>
#include <e_version.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "ekb_delete"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_KB,
   OPT_NAME
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

   {OPT_KB,
    'k',"knowledge-base",
    ReqArg, NULL,
    "Select the knowledge base. If not given, select E_KNOWLEDGE."},

   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char* kb_name = "E_KNOWLEDGE";
char* ex_name;
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
   CLState_p       state;
   DStr_p          name, store_file;
   FILE            *out;
   ExampleSet_p    proof_examples;
   ExampleRep_p    to_delete;
   AnnoSet_p       clause_examples;
   TB_p            annoterms;
   Scanner_p       in;
   TypeBank_p      typetable;
   Sig_p           sig;

   assert(argv[0]);

   InitIO(NAME);

   state = process_options(argc, argv);

   if(state->argc !=  1)
   {
      Error("One argument (name of the problem to remove) required",
       USAGE_ERROR);
   }
   assert(state->argv[0]);
   ex_name = state->argv[0];

   name = DStrAlloc();

   /* Step 1: Read existing files: problems, clausepatterns, signature
    */
   proof_examples = ExampleSetAlloc();
   in = CreateScanner(StreamTypeFile,
            KBFileName(name, kb_name, "problems"),
                      true, NULL, true);
   ExampleSetParse(in, proof_examples);
   DestroyScanner(in);

   typetable = TypeBankAlloc();
   sig = SigAlloc(typetable);
   annoterms = TBAlloc(sig);
   in = CreateScanner(StreamTypeFile,
            KBFileName(name, kb_name, "clausepatterns"),
                      true, NULL, true);
   clause_examples = AnnoSetParse(in, annoterms, KB_ANNOTATION_NO);
   DestroyScanner(in);

   VERBOUT("Old knowledge base files parsed successfully\n");

   /* Step 2: Check validity of remove request */

   to_delete = ExampleSetFindName(proof_examples, ex_name);
   if(!to_delete)
   {
      DStr_p error = DStrAlloc();

      DStrAppendStr(error, "Example name '");
      DStrAppendStr(error, ex_name);
      DStrAppendStr(error, "' does not exist in knowledge base");
      Error(DStrView(error), USAGE_ERROR);

      DStrFree(error);
   }
   /* Step 3: Remove examples and delete example file */

   AnnoSetRemoveByIdent(clause_examples, to_delete->ident);
   ExampleSetDeleteId(proof_examples, to_delete->ident);

   store_file = DStrAlloc();
   DStrAppendStr(store_file, KBFileName(name, kb_name, "FILES/"));
   DStrAppendStr(store_file, ex_name);

   FileRemove(DStrView(store_file));
   DStrFree(store_file);

   /* Step 4: Write everything back: problems, clausepatterns */

   out = OutOpen(KBFileName(name, kb_name, "clausepatterns"));
   AnnoSetPrint(out, clause_examples);
   OutClose(out);

   out = OutOpen(KBFileName(name, kb_name, "problems"));
   ExampleSetPrint(out, proof_examples);
   OutClose(out);

   /* Finally clean up */

   DStrFree(name);
   AnnoSetFree(clause_examples);
   annoterms->sig = NULL;
   TBFree(annoterms);
   SigFree(sig);
   ExampleSetFree(proof_examples);
   CLStateFree(state);
   TypeBankFree(typetable);
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
      case OPT_KB:
       kb_name = arg;
       break;
      case OPT_NAME:
       ex_name = arg;
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
Usage: " NAME " [options] <name>\n\
\n\
Remove the example <name> from an E knowledge base.\n\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
