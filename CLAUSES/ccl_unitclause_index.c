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

UnitClauseIndexCell_p UnitClauseIndexAlloc(Term_p termL) 
{
   UnitClauseIndexCell_p handle = UnitClauseIndexCellAlloc();

   handle->termL = termL;
   handle->right = NULL;

   return handle;
}

void UnitClauseIndexFree(UnitClauseIndexCell_p junk) 
{
   PTreeFree(junk->termL);
   UnitClauseIndexCellFree(junk);
}

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
//   ...
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
UnitClauseIndexCell_p UnitclauseInsert(PObjTree_p *root, Term_p lterm) {

   UnitClauseIndexCell_p old, new = UnitClauseIndexAlloc(lterm);

   old = PTreeObjStore(root, lterm, CmpUnitClauseIndexCells);
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
// PObjecttree einfach anstelle von Subtermtree hier nutzen.
//
/----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: UnitclauseIndexInsert()
//
//   Inserts a unit clause into the Index. Return
//   true if it was new, false if it already existed.
//   Is a wrapper for UnitClauseIndexInsert.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
bool UnitclauseIndexInsertClause(UnitclauseIndex_p index, Clause_p clause)
{
   assert(ClauseIsUnit(clause));
   return UnitclauseIndexInsert(index, clause->literals->lterm, 
                                clause->literals->rterm);
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
   PObjTree_p            cell;
   FPTree_p              fp_node;
   UnitClauseIndexCell_p unitclause_node;
   PObjTree_p            root;

   fp_node         = FPIndexInsert(index, lterm);
   unitclause_node = UnitclauseInsert((void*)&(fp_node->payload), lterm);
   root            = unitclause_node->right;

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
void UnitclauseIndexDeleteTerm(PObjTree_p root, Term_p lterm)
{
   UnitClauseIndexCell_p old, knode = UnitClauseIndexAlloc(lterm);
   old = PTreeObjExtractEntry(root, knode, CmpUnitClauseIndexCells);

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
bool UnitclauseIndexDeleteCellTerm(PObjTree_p root, Term_p lterm,
                                    Term_p rterm)
{
   UnitClauseIndexCell_p old, knode = UnitClauseIndexAlloc(lterm);
   PObjTree_p oldnode;
   bool res = false;
   PObjTree_p cell;

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


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
