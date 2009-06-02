/*-----------------------------------------------------------------------

File  : ccl_findex.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Implementation of

  Copyright 2009 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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

FIndex_p FIndexAlloc()
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
   PStack_p subterms = PStackAlloc();
   long i;
   Term_p term;   

   /* The following assumes TPOpFlag to be unset in all subterms of
      clause, and will set it. */
   ClauseCollectSubterms(clause, subterms);
   
   for(i=0; i<PStackGetSP(subterms); i++)
   {
      term = PStackElementP(subterms, i);
      if(!TermIsVar(term))
      {
         findex_add_instance(index, term->f_code, clause);
      }
   }
   /* Reset TPpFlag */
   TermStackDelProps(subterms, TPOpFlag);
   PStackFree(subterms);
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


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


