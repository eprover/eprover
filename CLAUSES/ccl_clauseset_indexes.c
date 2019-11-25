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
ClausesetIndexes_p ClausesetIndexesAlloc() {
   ClausesetIndexes_p clauseset_indexes = ClausesetIndexesAllocRaw();
   clauseset_indexes->fvindex = NULL;
   clauseset_indexes->demod_index = NULL;
   return clauseset_indexes;
}

/*-----------------------------------------------------------------------
//
// Function: ClausesetIndexesFree()
//
//   Delete all index datastriuctures.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void ClausesetIndexesFree(ClausesetIndexes_p clauseset_indexes)
{
   assert(clauseset_indexes);

   if (clauseset_indexes->fvindex) {
      FVIndexFree(clauseset_indexes->fvindex->index);
   }
   if (clauseset_indexes->demod_index) {
      PDTreeFree(clauseset_indexes->demod_index);
   }
   

   // TODO: Do I need to free the struct itself?
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
void ClausesetIndexesPDTIndexedInsert(ClausesetIndexes_p clauseset_indexes, Clause_p newclause) {

   ClausePos_p pos;

   assert(clauseset_indexes->demod_index);

   pos          = ClausePosCellAlloc();
   pos->clause  = newclause;
   pos->literal = newclause->literals;
   pos->side    = LeftSide;
   pos->pos     = NULL;
   PDTreeInsert(clauseset_indexes->demod_index, pos);

   if(!EqnIsOriented(newclause->literals))
   {
      pos          = ClausePosCellAlloc();
      pos->clause  = newclause;
      pos->literal = newclause->literals;
      pos->side    = RightSide;
      pos->pos     = NULL;
      PDTreeInsert(clauseset_indexes->demod_index, pos);
   }

}

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

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
ClausesetIndexes_p ClausesetIndexesAllocRaw() {
   return (ClausesetIndexes*)SizeMalloc(sizeof(ClausesetIndexes));
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/