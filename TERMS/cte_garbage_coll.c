/*-----------------------------------------------------------------------

  File  : cte_garbage_coll.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Code for simplifying term cell garbage collection.

  Copyright 2010, 2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jan 29 02:59:56 CET 2022 (from ccl-version)

-----------------------------------------------------------------------*/

#include "cte_garbage_coll.h"



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
//   Allocate an initialized GCAdminCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

GCAdmin_p GCAdminAlloc()
{
   GCAdmin_p handle = GCAdminCellAlloc();

   handle->clause_sets  = NULL;
   handle->formula_sets = NULL;

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

void GCRegisterFormulaSet(GCAdmin_p gc, void* set)
{
   assert(gc);
   assert(set);
   //printf(COMCHAR" GCRegisterFormulaSet(%p, %p)\n", gc, set);
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

void GCRegisterClauseSet(GCAdmin_p gc, void* set)
{
   assert(gc);
   assert(set);
   //printf(COMCHAR" GCRegisterClauseSet(%p, %p)\n", gc, set);
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

void GCDeregisterFormulaSet(GCAdmin_p gc, void* set)
{
   assert(gc);
   assert(set);
   //printf(COMCHAR" GCDeregisterFormulaSet(%p, %p)\n", gc, set);
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

void GCDeregisterClauseSet(GCAdmin_p gc, void* set)
{
   assert(gc);
   assert(set);

   //printf(COMCHAR" GCDeregisterClauseSet(%p, %p)\n", gc, set);
   PTreeDeleteEntry(&(gc->clause_sets), set);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
