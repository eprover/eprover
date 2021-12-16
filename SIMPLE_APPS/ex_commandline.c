/*-----------------------------------------------------------------------

File  : ex_commandline.c

Author: Stephan Schulz

Contents

  Example program for demonstrating the use of the cio_commandline
  module of CLIB.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jan 20 00:34:12 MET 1998
    created

-----------------------------------------------------------------------*/

#include <cio_commandline.h>

#define VERSION "1.0 Tue Jan 20 00:35:40 MET 1998"

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_INT_EXAMPLE,
   OPT_FLOAT_EXAMPLE
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
   {OPT_INT_EXAMPLE,
    'i', "int_example",
    ReqArg, "1",
    "Print the value given with the option.."},
   {OPT_FLOAT_EXAMPLE,
    'f', "float_example",
    OptArg, "3.1415",
    "Print the given argument or a default value."},
   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

ProblemType problemType  = PROBLEM_NOT_INIT;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

void      print_help(FILE* out);
CLState_p process_options(int argc, char* argv[]);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
   CLState_p state;
   int i;

   assert(argv[0]);
   InitError(argv[0]);

   /* Process options */
   state = process_options(argc, argv);

   /* Use - (=stdin) as the default argument if no non-option argument
      remains: */
   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   for(i=0; state->argv[i]; i++)
   {
      printf("File to process: %s\n", state->argv[i]);
   }
   CLStateFree(state);

   return NO_ERROR;
}


/*-----------------------------------------------------------------------
//
// Function: process_options()
//
//   Read and process the command line option, return (the pointer to)
//   a CLState object containing the remaining arguments.
//
// Global Variables: opts
//
// Side Effects    : Sets variables, may terminate with program
//                   description if option -h or --help was present,
//                   output arguments to example options.
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
      case OPT_INT_EXAMPLE:
    printf("Integer option has value %ld\n",
      CLStateGetIntArg(handle, arg));
    break;
      case OPT_FLOAT_EXAMPLE:
    printf("Float option has value %f\n",
      CLStateGetFloatArg(handle, arg));
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
\n\
ex_commandline.c "VERSION"\n\
\n\
Usage: ex_commandline [options] [files]\n\
\n\
Shows the usage of options, print non-option commandline arguments.\n\
\n");
   PrintOptions(stdout, opts, "Options\n\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


