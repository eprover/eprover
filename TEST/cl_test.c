/*-----------------------------------------------------------------------

File  : cl_test.c

Author: Stephan Schulz

Contents
 
  Main program for the CLIB test programm

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Nov 28 00:27:40 MET 1997

-----------------------------------------------------------------------*/

#include <clb_ddarrays.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cio_basicparser.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERBOSE,
   OPT_OUTPUT,
   OPT_PRINT_REFS,
   OPT_DISCOUNT_VARS,
   OPT_NO_INFIX,
   OPT_FULL_EQ_REP
}OptionCodes;



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

OptCell opts[] =
{   
   {OPT_NOOPT,
    '\0', NULL,
    NoArg, NULL,
    NULL}
};

char *outname = NULL;

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
   Scanner_p in; 
   long      i = 0;
   DDArray_p array = DDArrayAlloc(10000,10000);
   double    part;
   CLState_p state;
   
   assert(argv[0]);
   InitOutput();
   InitError(argv[0]);

   state = process_options(argc, argv);
   
   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   in = CreateScanner(StreamTypeFile, state->argv[0] , true, NULL);
   
   part = ParseFloat(in);;
   i=0;
   while(!TestInpTok(in, NoToken))
   {
      DDArrayAssign(array, i, ParseFloat(in));
      i++;
   }
   printf("Values parsed:    %ld\n"
	  "Part requested:   %f\n"
	  "Separating value: %f\n", i, part, DDArraySelectPart(array,
							       part,
							       i));   
   DDArrayFree(array);   
   DestroyScanner(in);
   CLStateFree(state);
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
// Global Variables: opts, Verbose, TermPrologArgs,
//                   TBPrintInternalInfo 
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
cl_test\n\
\n\
Usage: cl_test [options] [files]\n\
\n\
Do test stuff ;-).\n\
\n");
   PrintOptions(stdout, opts);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


