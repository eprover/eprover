/*-----------------------------------------------------------------------

File  : clb_dstacks.h

Author: Stephan Schulz

Contents

  Soemwhat efficient unlimited growth stacks for doubles.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun  7 12:21:16 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CLB_DSTACKS

#define CLB_DSTACKS

#include <clb_memory.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Double stack data type */

typedef long DStackPointer;

typedef struct dstackcell
{
   long          size;    /* ...of allocated memory */
   DStackPointer current; /* First unused address, 0 for empty stack */
   double        *stack;  /* Stack area */
}DStackCell, *DStack_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DSTACK_DEFAULT_SIZE 32 /* Stacks grow exponentially (and never
              shrink unless explicitly freed) -
              take care */


#define DStackCellAlloc() (DStackCell*)SizeMalloc(sizeof(DStackCell))
#define DStackCellFree(junk)         SizeFree(junk, sizeof(DStackCell))

DStack_p DStackAlloc(void);
void     DStackFree(DStack_p junk);
#define  DStackEmpty(stack) ((stack)->current == 0)
void     DStackReset(DStack_p stack);

void     DStackPush(DStack_p stack, double val);

#define  DStackGetSP(stack) ((stack)->current)

double   DStackPop(DStack_p stack);

double   DStackTop(DStack_p stack);
double   DStackBelowTop(DStack_p stack);

double   DStackElement(DStack_p stack, DStackPointer pos);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





