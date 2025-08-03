/*-----------------------------------------------------------------------

File  : ccl_garbage_coll.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for simplifying term cell garbage collection.

  Copyright 2010-2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Mar 20 09:55:09 CET 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_garbage_coll.h"



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


/*-----------------------------------------------------------------------
//
// Function: TBGCCollect()
//
//   Perform garbage collection on bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TBGCCollect(TB_p bank)
{
   PTree_p entry;
   PStack_p trav;

   assert(bank);
   assert(bank->gc);

   //printf(COMCHAR" GCCollect(%p)\n", gc);
   trav = PTreeTraverseInit(bank->gc->clause_sets);
   while((entry = PTreeTraverseNext(trav)))
   {
      //printf(COMCHAR" Marking clause set %p\n", entry->key);
      ClauseSetGCMarkTerms(entry->key);
   }
   PTreeTraverseExit(trav);

   trav = PTreeTraverseInit(bank->gc->formula_sets);
   while((entry = PTreeTraverseNext(trav)))
   {
      //printf(COMCHAR" Marking formula set %p\n", entry->key);
      FormulaSetGCMarkCells(entry->key);
   }
   PTreeTraverseExit(trav);

   return TBGCSweep(bank);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
