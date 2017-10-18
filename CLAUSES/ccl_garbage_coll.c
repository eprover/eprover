/*-----------------------------------------------------------------------

File  : ccl_garbage_coll.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for simplifying term cell garbage collection.

  Copyright 2010 by the author.
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
// Function: GCAdminAlloc()
//
//   Allocate an initialized GCAdminCell for the given termbank.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

GCAdmin_p GCAdminAlloc(TB_p bank)
{
   GCAdmin_p handle = GCAdminCellAlloc();

   assert(bank);

   handle->bank         = bank;
   handle->clause_sets  = NULL;
   handle->formula_sets = NULL;
   bank->gc             = handle;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: GCAdminFree()
//
//   Free a GCAdmin Cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GCAdminFree(GCAdmin_p junk)
{
   assert(junk);

   PTreeFree(junk->clause_sets);
   PTreeFree(junk->formula_sets);

   GCAdminCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: GCRegisterFormulaSet()
//
//   Register a formula set as containing relevant terms.
//
// Global Variables: -
//
// Side Effects    : Via PTReeStore()
//
/----------------------------------------------------------------------*/

void GCRegisterFormulaSet(GCAdmin_p gc, FormulaSet_p set)
{
   assert(gc);
   assert(set);
   PTreeStore(&(gc->formula_sets), set);
}


/*-----------------------------------------------------------------------
//
// Function: GCRegisterClauseSet()
//
//   Register a clause set as containing relevant terms.
//
// Global Variables: -
//
// Side Effects    : Via PTReeStore()
//
/----------------------------------------------------------------------*/

void GCRegisterClauseSet(GCAdmin_p gc, ClauseSet_p set)
{
   assert(gc);
   assert(set);
   PTreeStore(&(gc->clause_sets), set);
}


/*-----------------------------------------------------------------------
//
// Function: GCDeregisterFormulaSet()
//
//   Unregister a formula set as containing relevant terms.
//
// Global Variables: -
//
// Side Effects    : Via PTreeDeleteEntry()
//
/----------------------------------------------------------------------*/

void GCDeregisterFormulaSet(GCAdmin_p gc, FormulaSet_p set)
{
   assert(gc);
   assert(set);
   PTreeDeleteEntry(&(gc->formula_sets), set);
}

/*-----------------------------------------------------------------------
//
// Function: GCDeregisterClauseSet()
//
//   Unregister a clause set as containing relevant terms.
//
// Global Variables: -
//
// Side Effects    : Via PTreeDeleteEntry()
//
/----------------------------------------------------------------------*/

void GCDeregisterClauseSet(GCAdmin_p gc, ClauseSet_p set)
{
   assert(gc);
   assert(set);
   PTreeDeleteEntry(&(gc->clause_sets), set);
}

/*-----------------------------------------------------------------------
//
// Function: GCCollect()
//
//   Perform garbage collection on gc->bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long GCCollect(GCAdmin_p gc)
{
   PTree_p entry;
   PStack_p trav;

   assert(gc);
   assert(gc->bank);

   trav = PTreeTraverseInit(gc->clause_sets);
   while((entry = PTreeTraverseNext(trav)))
   {
      ClauseSetGCMarkTerms(entry->key);
   }
   PTreeTraverseExit(trav);

   trav = PTreeTraverseInit(gc->formula_sets);
   while((entry = PTreeTraverseNext(trav)))
   {
      FormulaSetGCMarkCells(entry->key);
   }
   PTreeTraverseExit(trav);

   return TBGCSweep(gc->bank);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


