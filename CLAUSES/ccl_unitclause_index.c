/*-----------------------------------------------------------------------

File  : ccl_unitclause_index.c

Author: Constantin Ruhdorfer

Contents

  A simple index for unitclauses.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include <ccl_unitclause_index.h>

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: UnitclauseInsertCell()
//
//   Inserts a clause into the index by inserting it into a leaf of 
//   the FingerPrintIndex given the appropiate PTree.
//   Return false if an entry for this clause exists, true otherwise.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
bool UnitclauseInsertCell(PTree_p *root, Clause_p clause)
{
   return PTreeStore(root, clause);
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexInsert()
//
//   Inserts a clause into the index given the appropiate term 
//   representation =(lterm, rterm).
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexInsert(UnitclauseIndex_p index, Term_p indexterm, 
                           Clause_p payload) 
{
   FPTree_p fp_node = FPIndexInsert(index, indexterm);

   return UnitclauseInsertCell((void*)&(fp_node->payload), payload);
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDeletClauseCell()
//
//   Deletes an indexed clause from the leaf of the FingerPrintIndex 
//   given the appropiate PTree.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexDeletClauseCell(PTree_p *root, Clause_p indexed)
{
   return PTreeDeleteEntry(root, indexed);
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDeleteIndexedClause()
//
//   Delete a clause given the indexed term (of the shape =(lterm, rterm))
//   also deletes the indexed term from the index if the leaf becomes empty
//   aka if there is nothing left to index. 
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexDeleteIndexedClause(UnitclauseIndex_p index, 
                                        Term_p indexedterm,
                                        Clause_p indexed)
{
   FPTree_p fp_node;
   bool     res;

   fp_node = FPIndexFind(index, indexedterm);
   if(!fp_node)
   {
      return false;
   }

   res = UnitclauseIndexDeletClauseCell((void*)&(fp_node->payload), indexed);
   
   if (fp_node->payload == NULL)
   {
      FPIndexDelete(index, indexedterm);
   }

   return res;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDeleteClause()
//
//   Deletes an indexed clause taking care of the index.
//   If the clause is not orientable both sides are deleted.
//   Assumes clause is unit.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexDeleteClause(UnitclauseIndex_p index, Clause_p clause)
{
   assert(ClauseIsUnit(clause));

   Eqn_p  handle = clause->literals;
   bool   existed;
   Term_p indexedTerm;

   indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                      handle->lterm, 
                                      handle->rterm, 
                                      true, // TODO: Are you sure that this is always okay?
                                      PENormal);

   existed = UnitclauseIndexDeleteIndexedClause(index, indexedTerm, clause);
   
   if(!EqnIsOriented(handle) && existed)
   {
      indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                         handle->lterm, 
                                         handle->rterm, 
                                         true, // TODO: Are you sure that this is always okay?
                                         PEReverse);

      return UnitclauseIndexDeleteIndexedClause(index, indexedTerm, clause);
   }

   return existed;
}

/*------------------------------------ -----------------------------------
//
// Function: UnitclauseIndexInsertClause()
//
//   Inserts a unit clause into the Index. Return
//   true if it was new, false if it already existed.
//   Is a wrapper for UnitClauseIndexInsert.
//   If the clause is not orientable both sides are indexed.
//   Assumes clause is unit.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
/e
/----------------------------------------------------------------------*/
bool UnitclauseIndexInsertClause(UnitclauseIndex_p index, Clause_p clause)
{
   assert(ClauseIsUnit(clause));

   Eqn_p  handle = clause->literals;
   bool   isNew;
   Term_p indexedTerm;

   indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                      handle->lterm, 
                                      handle->rterm, 
                                      true, // TODO: Are you sure that this is always okay?
                                      PENormal);

   isNew = UnitclauseIndexInsert(index, indexedTerm, clause);

   if(!EqnIsOriented(handle) && isNew)
   {
      indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                         handle->lterm, 
                                         handle->rterm, 
                                         true, // TODO: Are you sure that this is always okay?
                                         PEReverse);

      isNew = UnitclauseIndexInsert(index, indexedTerm, clause);
   }
   return isNew;
}

/*------------------------------------ -----------------------------------
//
// Function: UnitClauseIndexFindSubsumedCandidates()
//
//   Finds clauses that are candidates to be subsumed by the given clause.
//
// Global Variables: -
//
// Side Effects    : -
/
/----------------------------------------------------------------------*/
long UnitClauseIndexFindSubsumedCandidates(UnitclauseIndex_p index, 
                                           Clause_p clause, PStack_p candidates)
{
   Term_p indexedTerm;
   Eqn_p  handle           = clause->literals;
   long   numberMatchables = 0;

   indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                      handle->lterm, 
                                      handle->rterm, 
                                      true, // TODO: Are you sure that this is always okay?
                                      PENormal);

   numberMatchables = FPIndexFindMatchable(index, indexedTerm, candidates);

   if(!EqnIsOriented(handle))
   {
      indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                         handle->lterm, 
                                         handle->rterm, 
                                         true, // TODO: Are you sure that this is always okay?
                                         PEReverse);

      numberMatchables += FPIndexFindMatchable(index, indexedTerm, candidates);
   }
   return numberMatchables;
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexFreeWrapper()
//
//   Frees the PTree assosiated with the leaf of th fp_index so 
//   that the type matches with the type signature of FPFreeTreeFun:
//
//   void (*FPTreeFreeFun)(void*)
//
// Global Variables: - 
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
void UnitclauseIndexFreeWrapper(void *junk)
{
   PTreeFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
