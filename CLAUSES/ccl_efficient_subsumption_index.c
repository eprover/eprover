/*-----------------------------------------------------------------------

File  : ccl_esindex.c

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
// Function: ESIndexInsert()
//
//   Inserts a clause into the watchlists indexes.
//   This function determines the appropiate indexes for the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void ESIndexInsert(ESIndex_p index, FVPackedClause_p newclause)
{
   if(index->unitclasue_index && ClauseIsUnit(newclause->clause))
   {
      UCIndexInsertClause(index->unitclasue_index, newclause->clause);
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
// Function: ESIndexAlloc()
//
//   Allocate an empty ESIndex.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
ESIndex_p ESIndexAlloc(FVCollect_p cspec, PermVector_p perm)
{
   ESIndex_p handle = ESIndexAllocRaw();
   handle->fvindex                    = FVIAnchorAlloc(cspec, perm);
   handle->unitclasue_index           = NULL;

   handle->wl_constants_abstraction   = false;
   handle->wl_skolemsym_abstraction   = false;
   handle->wl_abstraction_symbols     = NULL;
   
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: ESIndexFree()
//
//   Delete all index datastructures.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/
void ESIndexFree(ESIndex_p junk)
{
   if (junk->fvindex) 
   {
      FVIAnchorFree(junk->fvindex);
   }
   if (junk->unitclasue_index)
   {
      FPIndexFree(junk->unitclasue_index);
   }

   if(junk->wl_constants_abstraction || junk->wl_skolemsym_abstraction)
   {
      PDArrayFree(junk->wl_abstraction_symbols);
   }

   ESIndexFreeRaw(junk);
}

/*-----------------------------------------------------------------------
//
// Function: ESIndexUCIndexInit()
//
//   Initialize the unit clause index as required by the parameters.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
void ESIndexUCIndexInit(ESIndex_p index, Sig_p sig, 
                        char* unitclause_index_type)
{
   FPIndexFunction indexfun;
   indexfun = GetFPIndexFunction(unitclause_index_type);
   if(indexfun)
   {
      index->unitclasue_index = FPIndexAlloc(indexfun, sig, 
                                             UCIndexFreeWrapper);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ESIndexInsertClause()
//
//   Inserts a clause into the efficient subsumption index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void ESIndexInsertClause(ESIndex_p index, 
                                           Clause_p clause)
{
   FVPackedClause_p pclause = FVIndexPackClause(clause, index->fvindex);
   assert(clause->weight == ClauseStandardWeight(clause));
   ESIndexInsert(index, pclause);
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
Clause_p ClausesetIndexDeleteEntry(ESIndex_p index, 
                                   Clause_p junk)
{
   if(index->unitclasue_index && ClauseIsUnit(junk))
   {
      UCIndexDeleteClause(index->unitclasue_index, 
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