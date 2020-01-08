/*-----------------------------------------------------------------------

File  : ccl_clauseset_indexes.c

Author: Constantin Ruhdorfer

Contents

  Functions for indexing watchlist clauses.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include <ccl_clauseset_indexes.h>

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexesAlloc()
//
//   Allocate an empty ClausesetIndexes.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
ClausesetIndexes_p ClausesetIndexesAlloc() 
{
   FPIndexFunction indexfunction;
   ClausesetIndexes_p clauseset_indexes = ClausesetIndexesAllocRaw();
   clauseset_indexes->fvindex = NULL;
   // demod_index = NULL;

   // indexfunction = GetFPIndexFunction();
   // clauseset_indexes->unitclasue_index = FPIndexAlloc(indexfunction, ,);
   return clauseset_indexes;
}

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexesFree()
//
//   Delete all index datastructures.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void ClausesetIndexesFree(ClausesetIndexes_p clauseset_indexes)
{
   assert(clauseset_indexes);

   if (clauseset_indexes->fvindex) 
   {
      FVIndexFree(clauseset_indexes->fvindex->index);
   }
   // if (demod_index) 
   // {
   //    PDTreeFree(demod_index);
   // }
   if (clauseset_indexes->unitclasue_index)
   {
      FPIndexFree(clauseset_indexes->unitclasue_index);
   }

   ClausesetIndexesFreeRaw(clauseset_indexes);
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
void ClausesetIndexInsertNewClause(ClausesetIndexes_p clauseset_indexes, 
                                   FVPackedClause_p newclause)
{
   if(ClauseIsUnit(newclause->clause))
   {
      ClausesetIndexesUCIndexededInsert(clauseset_indexes, newclause->clause);
      // ClausesetIndexesPDTIndexedInsert(clauseset_indexes, newclause);
   } else {
      FVIndexInsert(clauseset_indexes->fvindex, newclause);
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
Clause_p ClausesetIndexExtractEntry(ClausesetIndexes_p clauseset_indexes, 
                                Clause_p junk)
{
   if(ClauseIsUnit(junk))
   {
      UnitclauseIndexDeleteClause(clauseset_indexes->unitclasue_index, 
                                  junk);
      // ClausesetIndexesPDTDelteClause(clauseset_indexes, junk);
   } else {
      FVIndexDelete(clauseset_indexes->fvindex, junk);
      ClauseDelProp(junk, CPIsSIndexed);
   }

   return junk;
}

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexesUCIndexededInsert()
//
//   Inserts a clause into the watchlists unit clause index.
//   The index takes care of non-orientable clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void ClausesetIndexesUCIndexededInsert(ClausesetIndexes_p clauseset_indexes, 
                                       Clause_p newclause)
{
   assert(clauseset_indexes->unitclasue_index);
   UnitclauseIndexInsertClause(clauseset_indexes->unitclasue_index, newclause);
}

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexesPDTIndexedInsert()
//
//   Insert a demodulator into the watchlists demodulator index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
// void ClausesetIndexesPDTIndexedInsert(ClausesetIndexes_p clauseset_indexes, Clause_p newclause) 
// {

//    ClausePos_p pos;

//    assert(demod_index);

//    pos          = ClausePosCellAlloc();
//    pos->clause  = newclause;
//    pos->literal = newclause->literals;
//    pos->side    = LeftSide;
//    pos->pos     = NULL;
//    PDTreeInsert(demod_index, pos);

//    if(!EqnIsOriented(newclause->literals))
//    {
//       pos          = ClausePosCellAlloc();
//       pos->clause  = newclause;
//       pos->literal = newclause->literals;
//       pos->side    = RightSide;
//       pos->pos     = NULL;
//       PDTreeInsert(demod_index, pos);
//    }

// }

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexesPDTDelteClause()
//
//   Deletes an clause from the demod_index.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/
// void ClausesetIndexesPDTDelteClause(ClausesetIndexes_p clauseset_indexes, 
//                                     Clause_p junk)
// {
//    assert(ClauseIsUnit(junk));
//    PDTreeDelete(demod_index, junk->literals->lterm, 
//                 junk);
//    if(!EqnIsOriented(junk->literals))
//    {
//       PDTreeDelete(demod_index, junk->literals->rterm, 
//                    junk);
//    }
// } 

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexesAllocRaw()
//
//   Allocates an empty ClausesetIndexes struct.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
ClausesetIndexes_p ClausesetIndexesAllocRaw() 
{
   return (ClausesetIndexes*)SizeMalloc(sizeof(ClausesetIndexes));
}

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexesFreeRaw()
//
//   Frees an ClausesetIndex struct after all the indexes were freed.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/
void ClausesetIndexesFreeRaw(ClausesetIndexes_p clauseset_indexes) 
{
   SizeFree(clauseset_indexes, sizeof(clauseset_indexes));
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/