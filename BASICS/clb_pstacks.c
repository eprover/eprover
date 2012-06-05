/*-----------------------------------------------------------------------

File  : clb_pstacks.c

Author: Stephan Schulz

Contents
 
  Stacks for (long) integers and pointers.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Dec  3 17:43:15 MET 1997

-----------------------------------------------------------------------*/

#include <clb_pstacks.h>


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/* Most things are now defined as inline stuff.... */



/*-----------------------------------------------------------------------
//
// Function: PStackTopAddr()
//
//   Return address of top element on the stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IntOrP* PStackTopAddr(PStack_p stack)
{
   assert(stack->current);
   
   return &(stack->stack[stack->current-1]);
}



/*-----------------------------------------------------------------------
//
// Function: PStackComputeAverage()
//
//   Given a stack of integers, compute the arithmetic mean (returned)
//   and the standard deviation (stored in *deviation) of the
//   integers. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double PStackComputeAverage(PStack_p stack, double *deviation)
{
   PStackPointer i;
   long count = 0;
   double sum = 0, average = 0, variance = 0;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      sum+=PStackElementInt(stack,i);
      count++;
   }
   if(count)
   {
      average = sum / (double)count;
   }
   else
   {
      average = 0;
   }

   if(count)
   {
      for(i=0; i<PStackGetSP(stack); i++)
      {
	 variance+= (PStackElementInt(stack,i)-average)
	 *(PStackElementInt(stack,i)-average);
      }
      variance = variance / (double)count;
   }
   else
   {
      variance = 0;
   }
   *deviation = sqrt(variance);

   return average;
}



/*-----------------------------------------------------------------------
//
// Function: PStackSort()
//
//   Sort the elements of the PStack using qsort.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackSort(PStack_p stack, ComparisonFunctionType cmpfun)
{
   qsort(stack->stack, stack->current, sizeof(IntOrP), cmpfun);
}

/*-----------------------------------------------------------------------
//
// Function: PStackMerge()
//
//   Merge two sorted stacks onto a third. Discards duplicates.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void PStackMerge(PStack_p st1, PStack_p st2, PStack_p res, 
                     ComparisonFunctionType cmpfun)
{
   IntOrP tmp;
   int cmpres;

   while(!PStackEmpty(st1) || !PStackEmpty(st2))
   {
      if(PStackEmpty(st1))
      {
         tmp = PStackPop(st2);
      }
      else if(PStackEmpty(st2))
      {
         tmp = PStackPop(st1);
      }
      else
      {
         cmpres = cmpfun(PStackTopAddr(st1), PStackTopAddr(st2));
         if(cmpres < 0)
         {
            tmp = PStackPop(st1);
         }
         else
         {
            tmp = PStackPop(st2);
         }
      }
      if(PStackEmpty(res) || (cmpfun(PStackTopAddr(res), &tmp)!=0))
      {
         push(res, tmp);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: PStackPushStack()
//
//   Push all elements from source onto target.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackPushStack(PStack_p target, PStack_p source)
{
   PStackPointer i;

   for(i=0; i<PStackGetSP(source); i++)
   {
      push(target, PStackElement(source,i));
   }
}


/*-----------------------------------------------------------------------
//
// Function: PStackPrintInt()
//
//   Print a stack (interpreted as (long) integers) using the format
//   given. 
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PStackPrintInt(FILE* out, char* format, PStack_p stack)
{
   PStackPointer i;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      fprintf(out, format, PStackElementInt(stack, i));
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


