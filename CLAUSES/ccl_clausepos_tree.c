/*-----------------------------------------------------------------------

File  : ccl_clausepos_tree.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  A tree-based mapping mapping clauses to sets of compact positions.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 14 09:27:17 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_clausepos_tree.h"



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
// Function: clause_tpos_free_wrapper()
//
//   Wrapper of type ObjFreeFun.
//
// Global Variables: -
//
// Side Effects    : Via ClauseTPosFree()
//
/----------------------------------------------------------------------*/

static void clause_tpos_free_wrapper(void *junk)
{
   ClauseTPosFree(junk);
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClauseTPosAlloc()
//
//   Allocate a ClauseTPosCell for clause clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

ClauseTPos_p ClauseTPosAlloc(Clause_p clause)
{
   ClauseTPos_p handle = ClauseTPosCellAlloc();

   handle->clause = clause;
   handle->pos    = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseTPosFree()
//
//   Free a ClauseTPosCell, including the position tree, but not the
//   clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseTPosFree(ClauseTPos_p soc)
{
   NumTreeFree(soc->pos);
   ClauseTPosCellFree(soc);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseTPosTreeFree()
//
//   Free a ClauseTPOS-Tree.
//
// Global Variables: -
//
// Side Effects    :  Memory operations
//
/----------------------------------------------------------------------*/

void ClauseTPosTreeFree(ClauseTPosTree_p tree)
{
   PObjTreeFree(tree, clause_tpos_free_wrapper);
}



/*-----------------------------------------------------------------------
//
// Function: CmpClauseTPosCells()
//
//   Compare two ClauseTPos cells via their clausepointers.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int CmpClauseTPosCells(const void *soc1, const void *soc2)
{
   const ClauseTPos_p c1 = (const ClauseTPos_p) soc1;
   const ClauseTPos_p c2 = (const ClauseTPos_p) soc2;

   return PCmp(c1->clause, c2->clause);
}



/*-----------------------------------------------------------------------
//
// Function: ClauseTPosTreeFreeWrapper()
//
//   Free a subterm tree, with proper signature for FPIndexFree().
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseTPosTreeFreeWrapper(void *junk)
{
   ClauseTPosTreeFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseTPosTreeInsertPos()
//
//   Add a clause->pos association to the tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseTPosTreeInsertPos(ClauseTPosTree_p *tree, Clause_p clause,
                             CompactPos pos)
{
   ClauseTPos_p old, newnode = ClauseTPosAlloc(clause);
   IntOrP dummy;

   dummy.i_val = 0;
   old = PTreeObjStore(tree, newnode, CmpClauseTPosCells);

   if(old)
   {
      ClauseTPosFree(newnode);
      newnode = old;
   }
   NumTreeStore(&(newnode->pos), pos, dummy, dummy);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseTPosTreeDeletePos()
//
//   Delete a clause->pos association (and the clause, if there is no
//   remaining position).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseTPosTreeDeletePos(ClauseTPosTree_p *tree , Clause_p clause,
                             CompactPos pos)
{
   ClauseTPos_p found, key = ClauseTPosAlloc(clause);
   PObjTree_p cell;

   cell = PTreeObjFind(tree, key, CmpClauseTPosCells);
   ClauseTPosFree(key);

   if(cell)
   {
      found = cell->key;
      NumTreeDeleteEntry(&(found->pos), pos);
      if(!found->pos)
      {
         found = PTreeObjExtractObject(tree, found,
                                       CmpClauseTPosCells);
         ClauseTPosFree(found);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClauseTPosTreeDeleteClause()
//
//   Delete all associations clause->pos for any pos from the tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseTPosTreeDeleteClause(ClauseTPosTree_p *tree, Clause_p clause)
{
   ClauseTPos_p found, key = ClauseTPosAlloc(clause);

   found = PTreeObjExtractObject(tree, key, CmpClauseTPosCells);
   ClauseTPosFree(key);

   if(found)
   {
      ClauseTPosFree(found);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseTPosTreePrint()
//
//   Print a ClauseTposTree (mostly for debuging).
//
// Global Variables: -
//
// Side Effects    : Output, temp memory operations
//
/----------------------------------------------------------------------*/

void ClauseTPosTreePrint(FILE* out, ClauseTPos_p tree)
{
   fprintf(out, "OLs: ");
   ClausePrint(out, tree->clause, true);
   fprintf(out, "\nocc: ");
   NumTreeDebugPrint(out, tree->pos, true);
   fprintf(out, "\n");
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


