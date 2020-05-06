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
bool uc_index_insert_cell(PTree_p *root, Clause_p clause)
{
   return PTreeStore(root, clause);
}

/*-----------------------------------------------------------------------
//
// Function: UCIndexInsert()
//
//   Inserts a clause into the index given the appropiate term 
//   representation =(lterm, rterm).
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
bool uc_index_insert(UCIndex_p index, Term_p indexterm, Clause_p payload) 
{
   FPTree_p fp_node = FPIndexInsert(index, indexterm);

   return uc_index_insert_cell((void*)&(fp_node->payload), payload);
}

/*-----------------------------------------------------------------------
//
// Function: UCIndexDeleteClauseCell()
//
//   Deletes an indexed clause from the leaf of the FingerPrintIndex 
//   given the appropiate PTree.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
bool uc_index_delete_clause_cell(PTree_p *root, Clause_p indexed)
{
   return PTreeDeleteEntry(root, indexed);
}

/*-----------------------------------------------------------------------
//
// Function: UCIndexDeleteIndexedClause()
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
bool uc_index_delete_indexed_clause(UCIndex_p index, Term_p indexedterm,
                                    Clause_p indexed)
{
   FPTree_p fp_node;
   bool     res;

   fp_node = FPIndexFind(index, indexedterm);
   if(!fp_node)
   {
      return false;
   }

   res = uc_index_delete_clause_cell((void*)&(fp_node->payload), indexed);
   
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
// Function: UCIndexDeleteClause()
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
bool UCIndexDeleteClause(UCIndex_p index, Clause_p clause)
{
   assert(ClauseIsUnit(clause));

   Eqn_p  handle = clause->literals;
   bool   existed;
   Term_p indexedTerm;

   indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                      handle->lterm, 
                                      handle->rterm, 
                                      EqnIsPositive(handle),
                                      PENormal);

   existed = uc_index_delete_indexed_clause(index, indexedTerm, clause);
   
   if(!EqnIsOriented(handle) && existed)
   {
      indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                         handle->lterm, 
                                         handle->rterm, 
                                         EqnIsPositive(handle),
                                         PEReverse);

      return uc_index_delete_indexed_clause(index, indexedTerm, clause);
   }

   return existed;
}

/*------------------------------------ -----------------------------------
//
// Function: UCIndexInsertClause()
//
//   Inserts a unit clause into the Index. Return
//   true if it was new, false if it already existed.
//   Is a wrapper for UCIndexInsert.
//   If the clause is not orientable both sides are indexed.
//   Assumes clause is unit.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
bool UCIndexInsertClause(UCIndex_p index, Clause_p clause)
{
   assert(ClauseIsUnit(clause));

   Eqn_p  handle = clause->literals;
   bool   isNew;
   Term_p indexedTerm;

   indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                      handle->lterm, 
                                      handle->rterm, 
                                      EqnIsPositive(handle),
                                      PENormal);

   isNew = uc_index_insert(index, indexedTerm, clause);

   if(!EqnIsOriented(handle) && isNew)
   {
      indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                         handle->lterm, 
                                         handle->rterm, 
                                         EqnIsPositive(handle),
                                         PEReverse);

      isNew = uc_index_insert(index, indexedTerm, clause);
   }
   return isNew;
}

/*------------------------------------ -----------------------------------
//
// Function: UCIndexFindSubsumedCandidates()
//
//   Finds clauses that are candidates to be subsumed by the given clause.
//
// Global Variables: -
//
// Side Effects    : -
/
/----------------------------------------------------------------------*/
long UCIndexFindSubsumedCandidates(UCIndex_p index, Clause_p clause, 
                                   PStack_p candidates)
{
   Term_p indexedTerm;
   Eqn_p  handle           = clause->literals;
   long   numberMatchables = 0;

   indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                      handle->lterm, 
                                      handle->rterm, 
                                      EqnIsPositive(handle),
                                      PENormal);

   numberMatchables = FPIndexFindMatchable(index, indexedTerm, candidates);

   if(!EqnIsOriented(handle))
   {
      indexedTerm = EqnTermsTBTermEncode(handle->bank, 
                                         handle->lterm, 
                                         handle->rterm, 
                                         EqnIsPositive(handle),
                                         PEReverse);

      numberMatchables += FPIndexFindMatchable(index, indexedTerm, candidates);
   }
   return numberMatchables;
}

/*-----------------------------------------------------------------------
//
// Function: UCIndexFreeWrapper()
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
void UCIndexFreeWrapper(void *junk)
{
   PTreeFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
