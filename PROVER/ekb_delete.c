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

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "ekb_delete"
#define VERSION "0.1dev"

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
   SortTable_p     sort_table;
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
            true, NULL);
   ExampleSetParse(in, proof_examples);
   DestroyScanner(in);

   sort_table = DefaultSortTableAlloc();
   sig = SigAlloc(sort_table);
   annoterms = TBAlloc(sig);
   in = CreateScanner(StreamTypeFile,
            KBFileName(name, kb_name, "clausepatterns"),
            true, NULL);
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
   SortTableFree(sort_table);
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
   fprintf(out, "\n\
Copyright (C) 1999 by Stephan Schulz, schulz@informatik.tu-muenchen.de\n\
\n\
This program is a part of the support structure for the E equational\n\
theorem prover. You can find the latest version of the E distribution\n\
as well as additional information at\n\
http://wwwjessen.informatik.tu-muenchen.de/~schulz/WORK/eprover.html.\n\
\n\
This program is free software; you can redistribute it and/or modify\n\
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
\n\
Stephan Schulz\n\
Technische Universitaet Muenchen\n\
Fakultaet fuer Informatik\n\
Arcisstrasse 20\n\
D-80290 Muenchen\n\
Germany\n\
");

}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


