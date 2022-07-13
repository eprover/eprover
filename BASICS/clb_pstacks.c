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
#include "clb_simple_stuff.h"


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
// Function: PStackGrow()
//
//   Grow the stack area. Realloc is emulated in terms of
//   SizeMalloc()/SizeFree(). This is because stacks are allocated and
//   deallocated a lot, and usually in the same sizes, so it pays off
//   to optimize this behaviour.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void __attribute__ ((noinline)) PStackGrow(PStack_p stack)
{
      IntOrP *tmp;
      long   old_size;

      /* Emulate Realloc-Functionality for use of SizeMalloc() */
      old_size = stack->size;
      stack->size = stack->size*2;
      tmp = SizeMalloc(stack->size * sizeof(IntOrP));
      memcpy(tmp, stack->stack, old_size*sizeof(IntOrP));
      SizeFree(stack->stack, old_size * sizeof(IntOrP));
      stack->stack = tmp;
}


/*-----------------------------------------------------------------------
//
// Function: PStackDiscardElement()
//
//   Remove element number i from the stack. If it is not the top
//   element, the top element gets swapped in.
//0
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void PStackDiscardElement(PStack_p stack, PStackPointer i)
{
   assert(stack);
   assert(i < PStackGetSP(stack));
   assert(i >= 0);

   stack->current--;
   if(stack->current != i)
   {
      stack->stack[i] = stack->stack[stack->current];
   }
}



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
// Function: PStackBinSearch()
//
//   Perform a binar search on the (ordered) stack between indices
//   lower (inclusive) and upper (exclusive). Return index of key,
//   when found, or index of the next bigger element if not.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PStackPointer PStackBinSearch(PStack_p stack, void* key, PStackPointer lower,
                              PStackPointer upper, ComparisonFunctionType cmpfun)
{
   PStackPointer i;
   int cmpres;

   while(lower < upper)
   {
      //printf("Upper: %ld Lower: %ld\n", upper, lower);
      i = (lower+upper)/2;
      //printf("Pip %ld\n",i);
      cmpres = cmpfun(key, PStackElementP(stack, i));
      //printf("Pup %d\n",cmpres);
      if(cmpres < 0)
      {
         upper = i-1;
      }
      else if(cmpres > 0)
      {
         lower = i+1;
      }
      else
      {
         return i;
      }
   }
   //printf("Regular exit\n");
   return lower+1;
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

/*-----------------------------------------------------------------------
//
// Function: PStackPrintP()
//
//   Print a stack (interpreted as pointer stack) using the format
//   given.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PStackPrintP(FILE* out, char* format, PStack_p stack)
{
   PStackPointer i;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      fprintf(out, format, PStackElementP(stack, i));
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
