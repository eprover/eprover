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

   handle->termL = NULL;
   handle->right = PTreeCellAllocEmpty();

   return handle;
}

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
   PTreeFree(junk->right);
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

   return PCmp(c1->termL, c2->termL);
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
UnitClauseIndexCell_p UnitclauseInsert(PObjTree_p *root, Term_p lterm)
{
   UnitClauseIndexCell_p old = UnitClauseIndexAlloc();
   UnitClauseIndexCell_p new = UnitClauseIndexAlloc();
   new->termL = lterm;
   
   old = PTreeObjStore(root, new, CmpUnitClauseIndexCells);
   if (old)
   {
      UnitClauseIndexCellFree(new);
      new = old;
   }
   return new;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDeleteClause()
//
//   Deletes the rterm to the lterm.
//   And if not orientable the other way around.
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
      Eqn_p handle = clause->literals;
      bool existed;

      existed = UnitclauseIndexDeleteRightTerm(index, clause->literals->lterm, 
                                               clause->literals->rterm);
      
      if(!EqnIsOriented(handle) && existed)
      {
         return UnitclauseIndexDeleteRightTerm(index, clause->literals->rterm, 
                                               clause->literals->lterm);
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
      Eqn_p handle = clause->literals;
      bool isNew;
      
      isNew = UnitclauseIndexInsert(index, clause->literals->lterm, 
                                    clause->literals->rterm);

      if(!EqnIsOriented(handle) && isNew)
      {
         return UnitclauseIndexInsert(index, clause->literals->lterm, 
                                    clause->literals->rterm);
      }
      
      return isNew;
   }
   // Clause was not unit.
   return false;
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
bool UnitclauseIndexInsert(UnitclauseIndex_p index, Term_p lterm, 
                           Term_p rterm) 
{
   FPTree_p              fp_node;
   UnitClauseIndexCell_p unitclause_node;
   PTree_p               *root;

   fp_node               = FPIndexInsert(index, lterm);
   unitclause_node       = UnitclauseInsert((void*)&(fp_node->payload), lterm);
   root                  = &unitclause_node->right;

   return PTreeStore(root, rterm);
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDeleteTerm()
//
//   Delete an indexing of rterm via lterm.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
void UnitclauseIndexDeleteTerm(PObjTree_p *root, Term_p lterm)
{
   UnitClauseIndexCell_p old, knode = UnitClauseIndexAlloc();
   knode->termL = lterm;
   old = PTreeObjExtractObject(root, knode, CmpUnitClauseIndexCells);

   UnitClauseIndexCellFree(old);
   UnitClauseIndexCellFree(knode);
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDeleteCellTerm()
//
//   Delete an indexing of rterm via lterm.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexDeleteCellTerm(PObjTree_p *root, Term_p lterm,
                                    Term_p rterm)
{
   UnitClauseIndexCell_p old, knode = UnitClauseIndexAlloc();
   PObjTree_p oldnode;
   bool res = false;
   knode->termL = lterm;

   oldnode = PTreeObjFind(root, knode, CmpUnitClauseIndexCells); 
   if(oldnode)
   {
      old = oldnode->key;
      PTreeDeleteEntry(&(old->right), rterm);
      if(old->right == NULL)
      {
         UnitclauseIndexDeleteTerm(root, lterm);
      }
   }

   UnitClauseIndexCellFree(knode);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexDeleteRightTerm()
//
//   Delete a given right side of a unit clause from the index. 
//   Return true if the clause existed, false otherwise.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexDeleteRightTerm(UnitclauseIndex_p index, Term_p lterm,
                                    Term_p rterm)
{
   FPTree_p fp_node;
   bool     res;

   fp_node = FPIndexFind(index, lterm);
   if(!fp_node)
   {
      return false;
   }

   res = UnitclauseIndexDeleteCellTerm((void*)&(fp_node->payload), lterm, rterm);
   
   if (fp_node->payload == NULL)
   {
      FPIndexDelete(index, lterm);
   }

   return res;
}

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
   PObjTreeFree(junk, UnitClauseIndexCellFree);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
