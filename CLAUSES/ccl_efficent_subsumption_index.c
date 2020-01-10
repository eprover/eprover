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
   handle->sig                       = NULL;
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
      index->unitclasue_index = NULL;
      index->sig              = NULL;
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
   index->sig              = sig;
   indexfun                = GetFPIndexFunction(unitclause_index_type);
   assert(indexfun);
   strcpy(index->unitclause_index_type, unitclause_index_type);
   index->unitclasue_index = FPIndexAlloc(indexfun, sig, UnitclauseIndexFreeWrapper);
}

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
void EfficentSubsumptionIndexInsertClause(EfficentSubsumptionIndex_p index, 
                                          FVPackedClause_p newclause)
{
   // TODO: ClauseSetProp(newclause->clause, CPIsSIndexed);
   if(index->unitclasue_index && ClauseIsUnit(newclause->clause))
   {
      // TODO: Maybe index unitclauses into both?
      UnitclauseIndexInsertClause(index->unitclasue_index, newclause->clause);
   } else {
      FVIndexInsert(index->fvindex, newclause);
   }
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
   // TODO: ClauseDelProp(junk, CPIsSIndexed);
   if(index->unitclasue_index && ClauseIsUnit(junk))
   {
      UnitclauseIndexDeleteClause(index->unitclasue_index, 
                                  junk);
   } else {
      FVIndexDelete(index->fvindex, junk);
   }
   return junk;
}