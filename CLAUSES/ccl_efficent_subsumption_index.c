/*-----------------------------------------------------------------------

File  : ccl_efficent_subsumption_index.c

Author: Constantin Ruhdorfer

Contents

  Interface for indexing clauses for subsumption.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include <ccl_efficent_subsumption_index.h>

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
void EfficentSubsumptionIndexInsert(EfficentSubsumptionIndex_p index, 
                                    FVPackedClause_p newclause)
{
   if(index->unitclasue_index && ClauseIsUnit(newclause->clause))
   {
      // TODO: Maybe index unitclauses into both?
      // Notice that this question is seperate from checking both indexes.
      UnitclauseIndexInsertClause(index->unitclasue_index, newclause->clause);
   } 
   if (index->fvindex) 
   {
      FVIndexInsert(index->fvindex, newclause);
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: EfficentSubsumptionIndexAlloc()
//
//   Allocate an empty EfficentSubsumptionIndex.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
EfficentSubsumptionIndex_p EfficentSubsumptionIndexAlloc(FVCollect_p cspec, 
                                                         PermVector_p perm)
{
   EfficentSubsumptionIndex_p handle = EfficentSubsumptionIndexAllocRaw();
   handle->fvindex                   = FVIAnchorAlloc(cspec, PermVectorCopy(perm));
   handle->unitclasue_index          = NULL;
   // handle->sig                       = NULL;
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: EfficentSubsumptionIndexFree()
//
//   Delete all index datastructures.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/
void EfficentSubsumptionIndexFree(EfficentSubsumptionIndex_p index)
{
   if (index->fvindex) 
   {
      FVIAnchorFree(index->fvindex);
      index->fvindex = NULL;
   }
   if (index->unitclasue_index)
   {
      FPIndexFree(index->unitclasue_index);
      // index->unitclasue_index = NULL;
      // index->sig              = NULL;
   }
   EfficentSubsumptionIndexFreeRaw(index);
}

/*-----------------------------------------------------------------------
//
// Function: EfficentSubsumptionIndexUnitClauseIndexInit()
//
//   Initialize the unit clause index as required by the parameters.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
void EfficentSubsumptionIndexUnitClauseIndexInit(EfficentSubsumptionIndex_p index,
                                                 Sig_p sig, 
                                                 char* unitclause_index_type)
{
   FPIndexFunction indexfun;
   // index->sig              = sig;
   indexfun                = GetFPIndexFunction(unitclause_index_type);
   assert(indexfun);
   // strcpy(index->unitclause_index_type, unitclause_index_type);
   index->unitclasue_index = FPIndexAlloc(indexfun, sig, UnitclauseIndexFreeWrapper);
}

/*-----------------------------------------------------------------------
//
// Function: EfficentSubsumptionIndexInsertClause()
//
//   Inserts a clause into the efficent subsumption index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void EfficentSubsumptionIndexInsertClause(EfficentSubsumptionIndex_p index, 
                                          Clause_p clause)
{ 
   FVPackedClause_p pclause = FVIndexPackClause(clause, index->fvindex);
   assert(clause->weight == ClauseStandardWeight(clause));
   EfficentSubsumptionIndexInsert(index, pclause);
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
Clause_p ClausesetIndexDeleteEntry(EfficentSubsumptionIndex_p index, 
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