/*-----------------------------------------------------------------------

File  : clb_pstacks.c

Author: Stephan Schulz

Contents
 
  Stacks for (long) integers and pointers.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


