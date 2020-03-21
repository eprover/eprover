/*-----------------------------------------------------------------------

File  : ccl_efficient_subsumption_index.c

Author: Constantin Ruhdorfer

Contents

  Interface for indexing clauses for subsumption.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include <ccl_efficient_subsumption_index.h>

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: EfficentSubsumptionIndexInsertClause()
//
//   Inserts a clause into the watchlists indexes.
//   This function determines the appropiate indexes for the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void EfficientSubsumptionIndexInsert(EfficientSubsumptionIndex_p index, 
                                    FVPackedClause_p newclause)
{
   if(index->unitclasue_index && ClauseIsUnit(newclause->clause))
   {
      UnitclauseIndexInsertClause(index->unitclasue_index, newclause->clause);
   } 
   else if (index->fvindex) 
   {
      FVIndexInsert(index->fvindex, newclause);
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: EfficientSubsumptionIndexAlloc()
//
//   Allocate an empty EfficientSubsumptionIndex.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
EfficientSubsumptionIndex_p EfficientSubsumptionIndexAlloc(FVCollect_p cspec, 
                                                         PermVector_p perm)
{
   EfficientSubsumptionIndex_p handle = EfficientSubsumptionIndexAllocRaw();
   handle->fvindex                   = FVIAnchorAlloc(cspec, perm);
   handle->unitclasue_index          = NULL;
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: EfficientSubsumptionIndexFree()
//
//   Delete all index datastructures.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/
void EfficientSubsumptionIndexFree(EfficientSubsumptionIndex_p index)
{
   if (index->fvindex) 
   {
      FVIAnchorFree(index->fvindex);
   }
   if (index->unitclasue_index)
   {
      FPIndexFree(index->unitclasue_index);
   }
   EfficientSubsumptionIndexFreeRaw(index);
}

/*-----------------------------------------------------------------------
//
// Function: EfficientSubsumptionIndexUnitClauseIndexInit()
//
//   Initialize the unit clause index as required by the parameters.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
void EfficientSubsumptionIndexUnitClauseIndexInit(EfficientSubsumptionIndex_p index,
                                                 Sig_p sig, 
                                                 char* unitclause_index_type)
{
   FPIndexFunction indexfun;
   indexfun = GetFPIndexFunction(unitclause_index_type);
   if(indexfun)
   {
      index->unitclasue_index = FPIndexAlloc(indexfun, sig, UnitclauseIndexFreeWrapper);
   }
}

/*-----------------------------------------------------------------------
//
// Function: EfficientSubsumptionIndexInsertClause()
//
//   Inserts a clause into the efficient subsumption index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void EfficientSubsumptionIndexInsertClause(EfficientSubsumptionIndex_p index, 
                                          Clause_p clause)
{
   FVPackedClause_p pclause = FVIndexPackClause(clause, index->fvindex);
   assert(clause->weight == ClauseStandardWeight(clause));
   EfficientSubsumptionIndexInsert(index, pclause);
   FVUnpackClause(pclause);
}

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexDeleteEntry()
//
//   Deletes a clause from the watchlists indexes inserted by 
//   ClausesetIndexInsertNewClause; E.g. This function determines
//   the appropiate indexes for the clause.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/
Clause_p ClausesetIndexDeleteEntry(EfficientSubsumptionIndex_p index, 
                                   Clause_p junk)
{
   if(index->unitclasue_index && ClauseIsUnit(junk))
   {
      UnitclauseIndexDeleteClause(index->unitclasue_index, 
                                  junk);
   }
   if (index->fvindex) 
   {
      FVIndexDelete(index->fvindex, junk);
   }
   return junk;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/