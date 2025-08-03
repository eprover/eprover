/*-----------------------------------------------------------------------

File  : ccl_global_indices.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code handling several simple indices.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri May  7 21:19:48 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_global_indices.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

PERF_CTR_DEFINE(PMIndexTimer);
PERF_CTR_DEFINE(BWRWIndexTimer);

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
   indices->pm_negp_index = NULL;
   indices->pm_from_index = NULL;
   SetExtIntoIdx(indices, NULL);
   SetExtFromIdx(indices, NULL);
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
                       Sig_p sig,
                       char* rw_bw_index_type,
                       char* pm_from_index_type,
                       char* pm_into_index_type,
                       int   ext_rules_max_depth)
{
   FPIndexFunction indexfun;

   // fprintf(GlobalOut, COMCHAR" GlobalIndicesInit(%p, <>, %s, %s, %s)\n", indices, rw_bw_index_type, pm_from_index_type, pm_into_index_type);

   indices->sig = sig;
   indexfun = GetFPIndexFunction(rw_bw_index_type);
   strcpy(indices->rw_bw_index_type, rw_bw_index_type);
   if(indexfun)
   {
      indices->bw_rw_index = FPIndexAlloc(indexfun, sig, SubtermBWTreeFreeWrapper);
   }
   indexfun = GetFPIndexFunction(pm_from_index_type);
   strcpy(indices->pm_from_index_type, pm_from_index_type);
   if(indexfun)
   {
      indices->pm_from_index = FPIndexAlloc(indexfun, sig, SubtermOLTreeFreeWrapper);
   }
   indexfun = GetFPIndexFunction(pm_into_index_type);
   strcpy(indices->pm_into_index_type, pm_into_index_type);
   if(indexfun)
   {
      indices->pm_into_index = FPIndexAlloc(indexfun, sig, SubtermOLTreeFreeWrapper);
   }
   indexfun = GetFPIndexFunction(pm_into_index_type);
   strcpy(indices->pm_negp_index_type, pm_into_index_type);
   if(indexfun)
   {
      indices->pm_negp_index = FPIndexAlloc(indexfun, sig, SubtermOLTreeFreeWrapper);
   }

   assert(problemType != PROBLEM_NOT_INIT);
   if(problemType == PROBLEM_HO)
   {
      SetExtIntoIdx(indices, ExtIdxAlloc());
      SetExtFromIdx(indices, ExtIdxAlloc());
      SetExtMaxDepth(indices, ext_rules_max_depth);
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
   if(indices->pm_from_index)
   {
      FPIndexFree(indices->pm_from_index);
      indices->pm_from_index = NULL;
   }
   if(indices->pm_into_index)
   {
      FPIndexFree(indices->pm_into_index);
      indices->pm_into_index = NULL;
   }
   if(indices->pm_negp_index)
   {
      FPIndexFree(indices->pm_negp_index);
      indices->pm_negp_index = NULL;
   }
   if(GetExtIntoIdx(indices))
   {
      ExtIndexFree(GetExtIntoIdx(indices));
      SetExtIntoIdx(indices, NULL);

      assert(GetExtFromIdx(indices));
      ExtIndexFree(GetExtFromIdx(indices));
      SetExtFromIdx(indices, NULL);
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
   GlobalIndicesFreeIndices(indices);

   GlobalIndicesInit(indices,
                     indices->sig,
                     indices->rw_bw_index_type,
                     indices->pm_from_index_type,
                     indices->pm_into_index_type,
                     GetExtMaxDepth(indices));
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

void GlobalIndicesInsertClause(GlobalIndices_p indices, Clause_p clause,
                               bool lambda_demod)
{
   assert(!ClauseQueryProp(clause, CPIsGlobalIndexed));

   ClauseSetProp(clause, CPIsGlobalIndexed);

   // printf(COMCHAR" Inserting clause %p in index %p: ", clause, indices);ClausePrint(stdout, clause, true); printf("\n");

   if(indices->bw_rw_index)
   {
      PERF_CTR_ENTRY(BWRWIndexTimer);
      SubtermIndexInsertClause(indices->bw_rw_index, clause, lambda_demod);
      PERF_CTR_EXIT(BWRWIndexTimer);
   }

   if(indices->pm_into_index)
   {
      PERF_CTR_ENTRY(PMIndexTimer);
      //OverlapIndexInsertIntoClause(indices->pm_into_index, clause);
      OverlapIndexInsertIntoClause2(indices->pm_into_index,
                                    indices->pm_negp_index,
                                    clause);
      PERF_CTR_EXIT(PMIndexTimer);
   }
   if(indices->pm_from_index)
   {
      PERF_CTR_ENTRY(PMIndexTimer);
      OverlapIndexInsertFromClause(indices->pm_from_index, clause);
      PERF_CTR_EXIT(PMIndexTimer);
   }
   if(GetExtIntoIdx(indices))
   {
      ExtIndexInsertIntoClause(GetExtIntoIdx(indices), clause, GetExtMaxDepth(indices));
      assert(GetExtFromIdx(indices));
      ExtIndexInsertFromClause(GetExtFromIdx(indices), clause, GetExtMaxDepth(indices));
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

void GlobalIndicesDeleteClause(GlobalIndices_p indices, Clause_p clause, bool lambda_demod)
{
   //printf(COMCHAR" XXX GlobalIndicesDeleteClause()... (set=%p): ", clause->set);ClausePrint(GlobalOut, clause, true);printf("\n");

   assert(ClauseQueryProp(clause, CPIsGlobalIndexed));

   ClauseDelProp(clause, CPIsGlobalIndexed);

   if(indices->bw_rw_index)
   {
      PERF_CTR_ENTRY(BWRWIndexTimer);
      SubtermIndexDeleteClause(indices->bw_rw_index, clause, lambda_demod);
      PERF_CTR_EXIT(BWRWIndexTimer);
   }

   if(indices->pm_into_index)
   {
      PERF_CTR_ENTRY(PMIndexTimer);
      // OverlapIndexDeleteIntoClause(indices->pm_into_index, clause);
      OverlapIndexDeleteIntoClause2(indices->pm_into_index,
                                    indices->pm_negp_index,
                                    clause);
      PERF_CTR_EXIT(PMIndexTimer);
   }
   if(indices->pm_from_index)
   {
      PERF_CTR_ENTRY(PMIndexTimer);
      OverlapIndexDeleteFromClause(indices->pm_from_index, clause);
      PERF_CTR_EXIT(PMIndexTimer);
   }
   if(GetExtIntoIdx(indices))
   {
      assert(GetExtFromIdx(indices));
      ExtIndexDeleteIntoClause(GetExtIntoIdx(indices), clause);
      ExtIndexDeleteFromClause(GetExtFromIdx(indices), clause);
   }
   // printf(COMCHAR" ...GlobalIndicesDeleteClause()\n");
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
                                  ClauseSet_p set,
                                  bool lambda_demod)
{
   Clause_p handle;

   // fprintf(GlobalOut, "GlobalIndicesInsertClauseSet(%p, %p)\n", indices, set);

   if(!indices->bw_rw_index)
   {
      return;
   }
   for(handle=set->anchor->succ; handle!=set->anchor; handle=handle->succ)
   {
      GlobalIndicesInsertClause(indices, handle, lambda_demod);
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
