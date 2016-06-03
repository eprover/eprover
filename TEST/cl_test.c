/*-----------------------------------------------------------------------

File  : cl_test.c

Author: Stephan Schulz

Contents
 
  Main program for the CLIB test programm

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Nov 28 00:27:40 MET 1997

-----------------------------------------------------------------------*/

#include <clb_pdrangearrays.h>
#include <clb_intmap.h>
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
   CLState_p state;
   int i,key;
   PDRangeArr_p arr1, arr2;
   IntMap_p map;
   PStack_p stack;

   assert(argv[0]);
   InitIO(argv[0]);

   state = process_options(argc, argv);
   
   arr1 = PDRangeArrAlloc(4, 4);
   arr2 = PDRangeArrAlloc(4, GROW_EXPONENTIAL);
   map  = IntMapAlloc();

   stack = PStackAlloc();
   for(i=0; i<2000; i++)
   {
      key = drand48()*128-64;
      PDRangeArrAssignInt(arr1, key, key);
      PDRangeArrAssignInt(arr2, key, key);
      IntMapAssign(map, key, (void*)(intptr_t)key);
      PStackPushInt(stack, key);
   }
   while(!PStackEmpty(stack))
   {
      key = PStackPopInt(stack);
      printf("arr1[%d]=%ld\n", key, PDRangeArrElementInt(arr1,key));
      assert(key==PDRangeArrElementInt(arr1,key));
      printf("arr2[%d]=%ld\n", key, PDRangeArrElementInt(arr2,key));
      assert(key==PDRangeArrElementInt(arr2,key));
      printf("map[%d]=%ld\n", key, (long)IntMapGetVal(map,key));
      
   }
   PStackFree(stack);
  
   IntMapFree(map);
   PDRangeArrFree(arr2);
   PDRangeArrFree(arr1);
     

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
   PrintOptions(stdout, opts, "Options\n\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


