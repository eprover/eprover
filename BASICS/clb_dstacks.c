/*-----------------------------------------------------------------------

File  : clb_dstacks.c

Author: Stephan Schulz

Contents

  Stacks for (long) integers and pointers.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun  7 12:28:24 MET DST 1998
    News

-----------------------------------------------------------------------*/

#include <clb_dstacks.h>


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: push()
//
//   Implement push operation for DStacks. If the stack area needs to
//   grow, Realloc is emulated in terms of
//   SizeMalloc()/SizeFree(). This is because stacks are allocated and
//   deallocated a lot, and usually in the same sizes, so it pays of
//   to optimize this behaviour.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void push(DStack_p stack, double val)
{
   double *tmp;
   long   old_size;

   if(stack->current == stack->size)
   {
      /* Emulate Realloc-Functionality for use of SizeMalloc() */

      old_size = stack->size;
      stack->size = stack->size*2;
      tmp = SizeMalloc(stack->size * sizeof(double));
      memcpy(tmp, stack->stack, old_size*sizeof(double));
      SizeFree(stack->stack, old_size * sizeof(double));
      stack->stack = tmp;
   }
   stack->stack[stack->current] = val;
   stack->current++;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: DStackAlloc()
//
//   Allocate an empty stack.
//
// Global Variables: -
//
// Side Effects    : Memory oprations
//
/----------------------------------------------------------------------*/

DStack_p DStackAlloc(void)
{
   DStack_p handle;

   handle = DStackCellAlloc();
   handle->size = DSTACK_DEFAULT_SIZE;
   handle->current = 0;
   handle->stack = SizeMalloc(handle->size * sizeof(double));

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: DStackFree()
//
//   Free a stack.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void DStackFree(DStack_p junk)
{
   assert(junk);
   assert(junk->stack);

   SizeFree(junk->stack, junk->size * sizeof(double));
   DStackCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: DStackReset()
//
//   Reset a DStack to empty state.
//
// Global Variables: -
//
// Side Effects    : Changes stackpointer.
//
/----------------------------------------------------------------------*/

void DStackReset(DStack_p stack)
{
   stack->current = 0;
}

/*-----------------------------------------------------------------------
//
// Function: DStackPush()
//
//   Push a double onto the stack
//
// Global Variables: -
//
// Side Effects    : By push()
//
/----------------------------------------------------------------------*/

void DStackPush(DStack_p stack, double val)
{
   push(stack, val);
}

/*-----------------------------------------------------------------------
//
// Function: DStackPop()
//
//   Implement pop operation for non-empty DStacks.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

double DStackPop(DStack_p stack)
{
   assert(stack->current);

   stack->current--;
   return stack->stack[stack->current];
}


/*-----------------------------------------------------------------------
//
// Function: DStackTop()
//
//   Implement Top operation for non-empty DStacks.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double DStackTop(DStack_p stack)
{
   assert(stack->current);

   return stack->stack[stack->current-1];
}


/*-----------------------------------------------------------------------
//
// Function: DStackBelowTop()
//
//   Return second item on the stack (asserts that stack has >=2
//   elements).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double DStackBelowTop(DStack_p stack)
{
   assert(stack->current>=2);

   return stack->stack[stack->current-2];
}


/*-----------------------------------------------------------------------
//
// Function: DStackElement()
//
//   Return element at position pos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double DStackElement(DStack_p stack, DStackPointer pos)
{
   assert(pos<stack->current);
   assert(pos>=0);

   return stack->stack[pos];
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


