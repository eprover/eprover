/*-----------------------------------------------------------------------

File  : clb_pstacks.h

Author: Stephan Schulz

Contents

  Soemwhat efficient unlimited growth stacks for pointers/long ints.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Dec  3 16:22:48 MET 1997
    New

-----------------------------------------------------------------------*/

#ifndef CLB_PSTACKS

#define CLB_PSTACKS

#include <clb_memory.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Generic stack data type, can take (long) integers or pointers */

typedef long   PStackPointer;

typedef struct pstackcell
{
   long          size;    /* ...of allocated memory */
   PStackPointer current; /* First unused address, 0 for empty stack */
   IntOrP        *stack;  /* Stack area */
}PStackCell, *PStack_p;


#define PSTACK_DEFAULT_SIZE 128 /* Stacks grow exponentially (and never
                                   shrink unless explicitly freed) -
                                   take care */


#define PStackCellAlloc() (PStackCell*)SizeMalloc(sizeof(PStackCell))
#define PStackCellFree(junk)         SizeFree(junk, sizeof(PStackCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define PSTACK_AVG_MEM 68
#else
#define PSTACK_AVG_MEM (MEMSIZE(PStackCell)+MEMSIZE(IntOrP)*6)
#endif

#define  PStackBaseAddress(stackarg) ((stackarg)->stack)
static inline PStack_p PStackAlloc(void);
static inline PStack_p PStackVarAlloc(long size);
static inline void     PStackFree(PStack_p junk);
static inline PStack_p PStackCopy(PStack_p stack);
#define  PStackEmpty(stack) ((stack)->current == 0)
static inline void     PStackReset(PStack_p stack);

static inline void     PStackPushInt(PStack_p stack, long val);
static inline void     PStackPushP(PStack_p stack, void* val);
#define  PStackGetSP(stack) ((stack)->current)
#define  PStackGetTopSP(stack) ((stack)->current-1)

static inline IntOrP   PStackPop(PStack_p stack);
#define  PStackPopInt(stack) (PStackPop(stack).i_val)
#define  PStackPopP(stack)   (PStackPop(stack).p_val)
static inline void     PStackDiscardTop(PStack_p stack);

static inline IntOrP   PStackTop(PStack_p stack);
#define  PStackTopInt(stack) (PStackTop(stack).i_val)
#define  PStackTopP(stack)   (PStackTop(stack).p_val)
IntOrP*  PStackTopAddr(PStack_p stack);

static inline IntOrP   PStackBelowTop(PStack_p stack);
#define  PStackBelowTopInt(stack) (PStackBelowTop(stack).i_val)
#define  PStackBelowTopP(stack)   (PStackBelowTop(stack).p_val)


static inline IntOrP   PStackElement(PStack_p stack, PStackPointer pos);
#define  PStackElementInt(stack,pos) (PStackElement(stack,pos).i_val)
#define  PStackElementP(stack,pos) (PStackElement(stack,pos).p_val)

static inline IntOrP *PStackElementRef(PStack_p stack, PStackPointer pos);

#define   PStackAssignP(stack, pos, value) \
          PStackElementRef((stack), (pos))->p_val = (value)
#define   PStackAssignInt(stack, pos, value) \
          PStackElementRef((stack), (pos))->i_val = (value)

void     PStackDiscardElement(PStack_p stack, PStackPointer i);

void     PStackSort(PStack_p stack, ComparisonFunctionType cmpfun);
PStackPointer PStackBinSearch(PStack_p stack, void* key, PStackPointer lower,
                              PStackPointer upper, ComparisonFunctionType cmpfun);

void     PStackMerge(PStack_p st1, PStack_p st2, PStack_p res,
                     ComparisonFunctionType cmpfun);

double   PStackComputeAverage(PStack_p stack, double *deviation);

void     PStackPushStack(PStack_p target, PStack_p source);

void     PStackPrintInt(FILE* out, char* format, PStack_p stack);
void     PStackPrintP(FILE* out, char* format, PStack_p stack);

void     PStackGrow(PStack_p stack);

/*---------------------------------------------------------------------*/
/*                  Implementations as inline functions                */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: push()
//
//   Implements push operation for pstacks and
//   checks and ensures there is enought space on the steck.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline void push(PStack_p stack, IntOrP val)
{
   if(UNLIKELY(stack->current == stack->size))
   {
      PStackGrow(stack);
   }
   stack->stack[stack->current] = val;
   stack->current++;
}

/*-----------------------------------------------------------------------
//
// Function: PStackAlloc()
//
//   Allocate an empty stack.
//
// Global Variables: -
//
// Side Effects    : Memory oprations
//
/----------------------------------------------------------------------*/

static inline PStack_p PStackAlloc(void)
{
   PStack_p handle;

   handle = PStackCellAlloc();
   handle->size = PSTACK_DEFAULT_SIZE;
   handle->current = 0;
   handle->stack = SizeMalloc(PSTACK_DEFAULT_SIZE * sizeof(IntOrP));

   //printf("\nPStackAlloc: %p\n", handle);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PStackVarAlloc()
//
//   Allocate an empty stack with selectable initial size.
//
// Global Variables: -
//
// Side Effects    : Memory oprations
//
/----------------------------------------------------------------------*/

static inline PStack_p PStackVarAlloc(long size)
{
   PStack_p handle;

   handle = PStackCellAlloc();
   handle->size = size;
   handle->current = 0;
   handle->stack = SizeMalloc(handle->size * sizeof(IntOrP));

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PStackFree()
//
//   Free a stack.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline void  PStackFree(PStack_p junk)
{
   assert(junk);
   assert(junk->stack);

   SizeFree(junk->stack, junk->size * sizeof(IntOrP));
   PStackCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: PStackCopy()
//
//   Copy a PStack with contents. Use with care, as some data
//   structures may not be copyable very well (e.g. pointers to the
//   same array, registered references, ...)
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline PStack_p PStackCopy(PStack_p stack)
{
   PStack_p handle = PStackAlloc();
   PStackPointer i;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      push(handle, PStackElement(stack,i));
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PStackReset()
//
//   Reset a PStack to empty state.
//
// Global Variables: -
//
// Side Effects    : Changes stackpointer.
//
/----------------------------------------------------------------------*/

static inline void PStackReset(PStack_p stack)
{
   stack->current = 0;
}

/*-----------------------------------------------------------------------
//
// Function: PStackPushInt()
//
//   Push a (long) int onto the stack
//
// Global Variables: -
//
// Side Effects    : By push()
//
/----------------------------------------------------------------------*/

static inline void PStackPushInt(PStack_p stack, long val)
{
   IntOrP help;
   help.i_val = val;

   push(stack, help);
}

/*-----------------------------------------------------------------------
//
// Function: PStackPushP()
//
//   Push a pointer onto the stack
//
// Global Variables: -
//
// Side Effects    : by push()
//
/----------------------------------------------------------------------*/

static inline void PStackPushP(PStack_p stack, void* val)
{
   IntOrP help;
   help.p_val = val;

   push(stack, help);
}


/*-----------------------------------------------------------------------
//
// Function: PStackPop()
//
//   Implement pop operation for non-empty pstacks.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static inline IntOrP PStackPop(PStack_p stack)
{
   assert(stack->current);

   stack->current--;
   return stack->stack[stack->current];
}


/*-----------------------------------------------------------------------
//
// Function: PStackDiscardTop()
//
//   Do a PStackPop without returning result, to avoid warnings.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static inline void PStackDiscardTop(PStack_p stack)
{
   assert(stack->current);

   stack->current--;
}


/*-----------------------------------------------------------------------
//
// Function: PStackTop()
//
//   Implement Top operation for non-empty pstacks.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline IntOrP PStackTop(PStack_p stack)
{
   assert(stack->current);

   return stack->stack[stack->current-1];
}



/*-----------------------------------------------------------------------
//
// Function: PStackBelowTop()
//
//   Return second item on the stack (asserts that stack has >=2
//   elements).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline IntOrP PStackBelowTop(PStack_p stack)
{
   assert(stack->current>=2);

   return stack->stack[stack->current-2];
}


/*-----------------------------------------------------------------------
//
// Function: PStackElement()
//
//   Return element at position pos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline IntOrP PStackElement(PStack_p stack, PStackPointer pos)
{
   assert(pos<stack->current);
   assert(pos>=0);

   return stack->stack[pos];
}


/*-----------------------------------------------------------------------
//
// Function: PStackElementRef()
//
//   Return reference to element at position pos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline IntOrP *PStackElementRef(PStack_p stack, PStackPointer pos)
{
   assert(pos<stack->current);
   assert(pos>=0);

   return &(stack->stack[pos]);
}

/*-----------------------------------------------------------------------
//
// Function: PStackFindP()
//
//   Find a pointer in the stack
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline bool PStackFindP(PStack_p stack, void* p)
{
   for(PStackPointer i=0; i<PStackGetSP(stack); i++)
   {
      if(PStackElementP(stack, i) == p)
      {
         return true;
      }
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: PStackFindInt()
//
//   Find an int in the stack
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline bool PStackFindInt(PStack_p stack, long num)
{
   for(PStackPointer i=0; i<PStackGetSP(stack); i++)
   {
      if(PStackElementInt(stack, i) == num)
      {
         return true;
      }
   }
   return false;
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
