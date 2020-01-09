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
EfficentSubsumptionIndex_p EfficentSubsumptionIndexAlloc() 
{
   EfficentSubsumptionIndex_p handle = EfficentSubsumptionIndexAllocRaw();
   handle->fvindex                   = NULL;
   handle->unitclasue_index          = NULL;
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

void EfficentSubsumptionIndexFree(EfficentSubsumptionIndex_p efficent_subsumption_index)
{
   assert(efficent_subsumption_index);

   if (efficent_subsumption_index->fvindex) 
   {
      FVIndexFree(efficent_subsumption_index->fvindex->index);
   }
   if (efficent_subsumption_index->unitclasue_index)
   {
      FPIndexFree(efficent_subsumption_index->unitclasue_index);
   }

   EfficentSubsumptionIndexFreeRaw(efficent_subsumption_index);
}

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexInsertNewClause()
//
//   Inserts a clause into the watchlists indexes.
//   This function determines the appropiate indexes for the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void ClausesetIndexInsertNewClause(EfficentSubsumptionIndex_p efficent_subsumption_index, 
                                   FVPackedClause_p newclause)
{
   if(ClauseIsUnit(newclause->clause))
   {
      EfficentSubsumptionIndexUCIndexededInsert(efficent_subsumption_index, newclause->clause);
   } else {
      FVIndexInsert(efficent_subsumption_index->fvindex, newclause);
      ClauseSetProp(newclause->clause, CPIsSIndexed);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexExtractEntry()
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
Clause_p ClausesetIndexExtractEntry(EfficentSubsumptionIndex_p efficent_subsumption_index, 
                                Clause_p junk)
{
   if(ClauseIsUnit(junk))
   {
      UnitclauseIndexDeleteClause(efficent_subsumption_index->unitclasue_index, 
                                  junk);
   } else {
      FVIndexDelete(efficent_subsumption_index->fvindex, junk);
      ClauseDelProp(junk, CPIsSIndexed);
   }

   return junk;
}

/*-----------------------------------------------------------------------
//
// Function: EfficentSubsumptionIndexUCIndexededInsert()
//
//   Inserts a clause into the watchlists unit clause index.
//   The index takes care of non-orientable clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void EfficentSubsumptionIndexUCIndexededInsert(EfficentSubsumptionIndex_p efficent_subsumption_index, 
                                       Clause_p newclause)
{
   assert(efficent_subsumption_index->unitclasue_index);
   UnitclauseIndexInsertClause(efficent_subsumption_index->unitclasue_index, newclause);
}

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/