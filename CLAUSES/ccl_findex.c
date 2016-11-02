/*-----------------------------------------------------------------------

File  : ccl_findex.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Implementation of function symbol indexing.

  Copyright 2009 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1>     New

-----------------------------------------------------------------------*/

#include "ccl_findex.h"



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
// Function: findex_add_instance()
//
//   Add an instance (of clause or formula) into index with function
//   symbol i.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void findex_add_instance(FIndex_p index, FunCode i, void* inst)
{
   void *tmp;

   tmp = &(PDArrayElementP(index->index, i));
   PTreeStore(tmp, inst);
}

/*-----------------------------------------------------------------------
//
// Function: findex_remove_instance()
//
//   Add an instance (of clause or formula) from index with function
//   symbol i.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void findex_remove_instance(FIndex_p index, FunCode i, void* inst)
{
   void *tmp;

   tmp = &(PDArrayElementP(index->index, i));
   PTreeDeleteEntry(tmp, inst);
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FIndexAlloc()
//
//   Allocate an empty FIndex.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FIndex_p FIndexAlloc(void)
{
   FIndex_p handle = FIndexCellAlloc();

   handle->index = PDArrayAlloc(100,0);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FIndexFree()
//
//   Free an FIndex.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FIndexFree(FIndex_p junk)
{
   long i;
   PTree_p tmp;

   for(i = 0; i<junk->index->size; i++)
   {
      tmp = PDArrayElementP(junk->index, i);
      PTreeFree(tmp);
   }
   PDArrayFree(junk->index);
   FIndexCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FIndexAddClause()
//
//   Add a clause to the FIndex.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FIndexAddClause(FIndex_p index, Clause_p clause)
{
   PStack_p f_codes = PStackAlloc();
   long i;
   FunCode f;

   ClauseReturnFCodes(clause, f_codes);

   for(i=0; i<PStackGetSP(f_codes); i++)
   {
      f = PStackElementInt(f_codes, i);
      findex_add_instance(index, f, clause);
   }
   PStackFree(f_codes);
}

/*-----------------------------------------------------------------------
//
// Function: FIndexRemoveClause()
//
//   Remove a clause from the FIndex.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FIndexRemoveClause(FIndex_p index, Clause_p clause)
{
   PStack_p f_codes = PStackAlloc();
   long i;
   FunCode f;

   ClauseReturnFCodes(clause, f_codes);

   for(i=0; i<PStackGetSP(f_codes); i++)
   {
      f = PStackElementInt(f_codes, i);
      findex_remove_instance(index, f, clause);
   }
   PStackFree(f_codes);
}



/*-----------------------------------------------------------------------
//
// Function: FIndexAddClauseSet()
//
//   Build a FIndex from clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FIndexAddClauseSet(FIndex_p index, ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ;
       handle!=set->anchor;
       handle = handle->succ)
   {
      FIndexAddClause(index, handle);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FIndexAddPLClause()
//
//   Add PListCell containing a clause as payload to the index.
//
// Global Variables: -
//
// Side Effects    : As FIndexAddClause()
//
/----------------------------------------------------------------------*/

void FIndexAddPLClause(FIndex_p index, PList_p lclause)
{
   PStack_p f_codes = PStackAlloc();
   long i;
   FunCode f;

   ClauseReturnFCodes(lclause->key.p_val, f_codes);

   for(i=0; i<PStackGetSP(f_codes); i++)
   {
      f = PStackElementInt(f_codes, i);
      findex_add_instance(index, f, lclause);
   }
   PStackFree(f_codes);

}



/*-----------------------------------------------------------------------
//
// Function: FIndexRemovePLClause()
//
//   Remove a PListCell conaining a clause from the FIndex.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FIndexRemovePLClause(FIndex_p index,  PList_p lclause)
{
   PStack_p f_codes = PStackAlloc();
   long i;
   FunCode f;

   ClauseReturnFCodes(lclause->key.p_val, f_codes);

   for(i=0; i<PStackGetSP(f_codes); i++)
   {
      f = PStackElementInt(f_codes, i);
      findex_remove_instance(index, f, lclause);
   }
   PStackFree(f_codes);
}



/*-----------------------------------------------------------------------
//
// Function: FIndexAddPLClauseSet()
//
//   Add all the clauses in a PList to the index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FIndexAddPLClauseSet(FIndex_p index, PList_p set)
{
   PList_p handle;

   for(handle = set->succ;
       handle!=set;
       handle = handle->succ)
   {
      FIndexAddPLClause(index, handle);
   }
}




/*-----------------------------------------------------------------------
//
// Function: FIndexAddPLFormula()
//
//   Add PListCell containing a formula as payload to the index.
//
// Global Variables: -
//
// Side Effects    : As FIndexAddFormula()
//
/----------------------------------------------------------------------*/

void FIndexAddPLFormula(FIndex_p index, PList_p lformula)
{
   PStack_p f_codes = PStackAlloc();
   long i;
   FunCode f;

   WFormulaReturnFCodes(lformula->key.p_val, f_codes);

   for(i=0; i<PStackGetSP(f_codes); i++)
   {
      f = PStackElementInt(f_codes, i);
      findex_add_instance(index, f, lformula);
   }
   PStackFree(f_codes);

}



/*-----------------------------------------------------------------------
//
// Function: FIndexRemovePLFormula()
//
//   Remove a PListCell conaining a formula from the FIndex.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FIndexRemovePLFormula(FIndex_p index,  PList_p lformula)
{
   PStack_p f_codes = PStackAlloc();
   long i;
   FunCode f;

   WFormulaReturnFCodes(lformula->key.p_val, f_codes);

   for(i=0; i<PStackGetSP(f_codes); i++)
   {
      f = PStackElementInt(f_codes, i);
      findex_remove_instance(index, f, lformula);
   }
   PStackFree(f_codes);
}



/*-----------------------------------------------------------------------
//
// Function: FIndexAddPLFormulaSet()
//
//   Add all the formulas in a PList to the index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FIndexAddPLFormulaSet(FIndex_p index, PList_p set)
{
   PList_p handle;

   for(handle = set->succ;
       handle!=set;
       handle = handle->succ)
   {
      FIndexAddPLFormula(index, handle);
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


