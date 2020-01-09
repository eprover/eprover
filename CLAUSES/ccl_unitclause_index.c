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
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: UnitClauseIndexCellFree()
//
//   Frees a complete UnitClauseIndexCell.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
void UnitClauseIndexCellFree(UnitClauseIndexCell_p junk) 
{
   junk->clause = NULL;
   UnitClauseIndexCellFreeRaw(junk);
}

/*-----------------------------------------------------------------------
//
// Function: CmpUnitClauseIndexCells()
//
//   Compares two unitClauseIndexCells via their term pointer.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
int CmpUnitClauseIndexCells(const void* cell1, const void* cell2)
{
   const UnitClauseIndexCell_p c1 = (const UnitClauseIndexCell_p) cell1;
   const UnitClauseIndexCell_p c2 = (const UnitClauseIndexCell_p) cell2;

   return PCmp(c1->clause, c2->clause);
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseInsert()
//
//   Inserts a lterm as a UnitClauseIndexCell into the the PObjTree.
//   If it already existed it returns the old cell.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
UnitClauseIndexCell_p UnitclauseInsertCell(PObjTree_p *root, Clause_p clause)
{
   UnitClauseIndexCell_p old = UnitClauseIndexAlloc();
   UnitClauseIndexCell_p new = UnitClauseIndexAlloc();
   new->clause = clause;
   
   old = PTreeObjStore(root, new, CmpUnitClauseIndexCells);
   if (old)
   {
      UnitClauseIndexCellFree(new);
      new = old;
   }
   return new;
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexInsert()
//
//   Inserts a rterm into the Index given lterm. Return
//   true if it was new, false if it already existed.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexInsert(UnitclauseIndex_p index, Term_p indexterm, 
                           Clause_p payload) 
{
   FPTree_p                fp_node;
   UnitClauseIndexCell_p   unitclause_node;

   fp_node                 = FPIndexInsert(index, indexterm);
   unitclause_node         = UnitclauseInsertCell((void*)&(fp_node->payload), 
                                                  payload);
   return (unitclause_node == NULL);
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDeletClauseCell()
//
//   Delete an indexing of rterm via lterm.
//   Returns true if the cell existed befor, false otherwise.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexDeletClauseCell(PObjTree_p *root, Clause_p indexed)
{
   PObjTree_p oldnode;
   UnitClauseIndexCell_p knode = UnitClauseIndexAlloc();
   bool res                    = false;
   knode->clause               = indexed;

   oldnode = PTreeObjExtractEntry(root, knode, CmpUnitClauseIndexCells); 
   if(oldnode)
   {
      res = true;
   }

   UnitClauseIndexCellFree(knode);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDeleteIndexedClause()
//
//   Delete a given right side of a unit clause from the index. 
//   Return true if the clause existed, false otherwise.
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

/*-----------------------------------------------------------------------
//
// Function: UnitClauseIndexCellFreeWrapper()
//
//   Wrapper for UnitClauseIndexCellFree so that the type matches with
//   the type signature of ObjDelFun (void (*)(void *)).
//
// Global Variables: - 
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
void UnitClauseIndexCellFreeWrapper(void *junk)
{
   UnitClauseIndexCellFree(junk);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: UnitClauseIndexAlloc()
//
//   Allocates a complete UnitClauseIndexCell.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
UnitClauseIndexCell_p UnitClauseIndexAlloc() 
{
   UnitClauseIndexCell_p handle = UnitClauseIndexCellAlloc();
   handle->clause = NULL;
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDelete()
//
//   Deletes an indexed clause taking care of the index.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexDeleteClause(UnitclauseIndex_p index, Clause_p clause)
{
   if(ClauseIsUnit(clause))
   {
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
   // Clause is not unit and therefore did not exist.
   return false;
}

/*------------------------------------ -----------------------------------
//
// Function: UnitclauseIndexInsert()
//
//   Inserts a unit clause into the Index. Return
//   true if it was new, false if it already existed.
//   It also returns false if the clause was not unit!
//   Is a wrapper for UnitClauseIndexInsert.
//   If the clause is not orientable both sides are indexed.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
/e
/----------------------------------------------------------------------*/
bool UnitclauseIndexInsertClause(UnitclauseIndex_p index, Clause_p clause)
{
   if(ClauseIsUnit(clause))
   {
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
                                            true, 
                                            PEReverse);
         isNew = UnitclauseIndexInsert(index, indexedTerm, clause);
      }
      
      return isNew;
   }
   // Clause was not unit.
   return false;
}

// /*-----------------------------------------------------------------------
// //
// // Function: UnitclauseIndexDeleteTerm()
// //
// //   Delete an indexing of rterm via lterm.
// //
// // Global Variables:
// //
// // Side Effects    :
// //
// /----------------------------------------------------------------------*/
// void UnitclauseIndexDeleteTerm(PObjTree_p *root, Term_p lterm)
// {
//    UnitClauseIndexCell_p old, knode = UnitClauseIndexAlloc();
//    knode->termL = lterm;
//    old = PTreeObjExtractObject(root, knode, CmpUnitClauseIndexCells);

//    UnitClauseIndexCellFree(old);
//    UnitClauseIndexCellFree(knode);
// }

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexFreeWrapper()
//
//   Free a pointer tree in the unitclause index, 
//   with proper signature for FPIndexFree().
//
// Global Variables: - 
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
void UnitclauseIndexFreeWrapper(void *junk)
{
   PObjTreeFree(junk, UnitClauseIndexCellFreeWrapper);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
