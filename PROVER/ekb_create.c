/*-----------------------------------------------------------------------

  File  : ekb_create.c

  Author: Stephan Schulz

  Contents

  Create a new, empty knowledge base for E.

  Copyright 1998, 1999, 2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Jul 23 17:46:15 MET DST 1999

  -----------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cle_kbdesc.h>
#include <e_version.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME    "ekb_create"

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERSION,
   OPT_VERBOSE,
   OPT_NEG_NO,
   OPT_NEG_PROP,
   OPT_SELECT_EVAL
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
    "Print the version number of the program."},

   {OPT_VERBOSE,
    'v', "verbose",
    OptArg, "1",
    "Verbose comments on the progress of the program."},

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

double neg_proportion = 1.0;
long   neg_examples   =   0;

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
   KBDesc_p        handle;
   char            *basename = "E_KNOWLEDGE";
   DStr_p          name;
   FILE            *out;

   assert(argv[0]);

   InitIO(NAME);

   state = process_options(argc, argv);

   if(state->argc > 1)
   {
      Error("Only one non-option argument (name of the knowledge base)"
       " expected", USAGE_ERROR);
   }
   if(state->argc == 1)
   {
      basename = state->argv[0];
   }
   else
   {
      VERBOUT("Using default name\n");
   }

   VERBOUT("Creating base directory...\n");

   if(mkdir(basename,S_IRWXU|S_IRWXG))
   {
      TmpErrno = errno;
      SysError("Cannot create base directory '%s'",
               SYNTAX_ERROR,
               basename);
   }
   VERBOUT("...successful.\nCreating files...\n");

   name = DStrAlloc();

   handle = KBDescAlloc(KB_VERSION, neg_proportion,neg_examples);

   out = OutOpen(KBFileName(name, basename, "description"));
   KBDescPrint(out, handle);
   OutClose(out);
   KBDescFree(handle);

   out = OutOpen(KBFileName(name, basename, "signature"));
   fprintf(out,
           COMCHAR" Special function symbols that are not generalized.\n"
           COMCHAR" You need to hand-hack this at the moment.\n");
   OutClose(out);

   out = OutOpen(KBFileName(name, basename, "problems"));
   fprintf(out,
           COMCHAR" Example names and features. \n");
   OutClose(out);

   out = OutOpen(KBFileName(name, basename, "clausepatterns"));
   fprintf(out,
           COMCHAR" Individual annotated patterns. \n");
   OutClose(out);

   VERBOUT("...done.\nCreating subdirectory FILES...\n");

   if(mkdir(KBFileName(name, basename, "FILES"), S_IRWXU|S_IRWXG))
   {
      SysError("Cannot create base directory '%s'",
               FILE_ERROR,
               basename);
      TmpErrno = errno;
   }
   VERBOUT("...done.\nNew knowledge base complete.\n");

   DStrFree(name);
   CLStateFree(state);
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
Usage: " NAME " [options] [<name>]\n\
\n\
Create an empty knowledge base with name <name> for E.\n\n");
   PrintOptions(stdout, opts, "Options\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
