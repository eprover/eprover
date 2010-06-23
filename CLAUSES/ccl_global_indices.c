/*-----------------------------------------------------------------------

File  : ccl_global_indices.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Code handling several simple indices.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri May  7 21:19:48 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_global_indices.h"



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
// Function: GlobalIndicesNull()
//
//   Set the global indices to NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void GlobalIndicesNull(GlobalIndices_p indices)
{
   indices->bw_rw_index   = NULL;
   indices->pm_into_index = NULL;
   indices->pm_from_index = NULL;
}


/*-----------------------------------------------------------------------
//
// Function: GlobalIndicesInit()
//
//   Initialize the global indices as required by the parameters.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GlobalIndicesInit(GlobalIndices_p indices, 
                       bool use_bw_rw_index,
                       bool use_pm_index)
{
   if(use_bw_rw_index)
   {
      indices->bw_rw_index = FPIndexAlloc(IndexFP7Create, SubtermBWTreeFreeWrapper);
   }
   if(use_pm_index)
   {
      indices->pm_into_index = FPIndexAlloc(IndexFP7Create, SubtermOLTreeFreeWrapper);
      indices->pm_from_index = FPIndexAlloc(IndexFP7Create, SubtermOLTreeFreeWrapper);
   }
}


/*-----------------------------------------------------------------------
//
// Function: GlobalIndicesFreeIndices()
//
//   Free the existing indices.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GlobalIndicesFreeIndices(GlobalIndices_p indices)
{
   if(indices->bw_rw_index)
   {
      FPIndexFree(indices->bw_rw_index);          
      indices->bw_rw_index = NULL;
   }
   if(indices->pm_into_index)
   {
      FPIndexFree(indices->pm_into_index);          
      indices->pm_into_index = NULL;
   }
   if(indices->pm_from_index)
   {
      FPIndexFree(indices->pm_from_index);          
      indices->pm_from_index = NULL;
   }  
}


/*-----------------------------------------------------------------------
//
// Function: GlobalIndicesReset()
//
//   Reset all exisiting indices.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GlobalIndicesReset(GlobalIndices_p indices)
{
   bool 
      use_bw_rw_index = false, 
      use_pm_index = false;

   use_bw_rw_index   = indices->bw_rw_index!=NULL;
   use_pm_index      = indices->pm_into_index!=NULL;

   GlobalIndicesInit(indices, 
                     use_bw_rw_index,
                     use_pm_index);
}


/*-----------------------------------------------------------------------
//
// Function: GlobalIndicesInsertClause()
//
//   Add a clause to all exisiting global indices.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GlobalIndicesInsertClause(GlobalIndices_p indices, Clause_p clause)
{
   assert(!ClauseQueryProp(clause, CPIsGlobalIndexed));

   ClauseSetProp(clause, CPIsGlobalIndexed);

   if(indices->bw_rw_index)
   {
      SubtermIndexInsertClause(indices->bw_rw_index, clause);
   }
   if(indices->pm_into_index)
   {
      OverlapIndexInsertIntoClause(indices->pm_into_index, clause);
   }
   if(indices->pm_from_index)
   {
      OverlapIndexInsertFromClause(indices->pm_from_index, clause);
   }
}



/*-----------------------------------------------------------------------
//
// Function: GlobalIndicesDeleteClause()
//
//   Remove a clause from all exisiting global indices.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GlobalIndicesDeleteClause(GlobalIndices_p indices, Clause_p clause)
{
   assert(ClauseQueryProp(clause, CPIsGlobalIndexed));

   ClauseDelProp(clause, CPIsGlobalIndexed);

   if(indices->bw_rw_index)
   {
      SubtermIndexDeleteClause(indices->bw_rw_index, clause);
   }
   if(indices->pm_into_index)
   {
      OverlapIndexDeleteIntoClause(indices->pm_into_index, clause);
   }
   if(indices->pm_from_index)
   {
      OverlapIndexDeleteFromClause(indices->pm_from_index, clause);
   }
}


/*-----------------------------------------------------------------------
//
// Function: GlobalIndicesInsertClauseSet()
//
//   Insert all clause in set into the indices.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GlobalIndicesInsertClauseSet(GlobalIndices_p indices, 
                                  ClauseSet_p set)
{
   Clause_p handle;

   if(!indices->bw_rw_index)
   {
      return;
   }
   for(handle=set->anchor->succ; handle!=set->anchor; handle=handle->succ)
   {
      GlobalIndicesInsertClause(indices, handle);
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


