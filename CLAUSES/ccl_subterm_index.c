/*-----------------------------------------------------------------------

File  : ccl_subterm_index.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  An index mapping subterms to occurances in clauses.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 14 09:27:17 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_subterm_index.h"



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
// Function: term_collect_idx_subterms()
//
//   
//   Collect all non-variable subterms in term either into rest or
//   full (rest for "restricted rewriting" terms, full for the "full
//   rewriting" terms).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static long term_collect_idx_subterms(Term_p term, PTree_p *rest,
                                      PTree_p *full, bool restricted) 
{
   long res = 1;
   int i;

   if(TermIsVar(term))
   {
      return 0;
   }
   if(restricted)
   {
      PTreeStore(rest, term);
   }
   else
   {
      PTreeStore(full, term);
   }
   for(i=0; i<term->arity; i++)
   {
      res += term_collect_idx_subterms(term->args[i], rest, full, false);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_collect_idx_subterms()
//
//   Collect all non-variable subterms in eqn either into rest or
//   full (rest for "restricted rewriting" terms, full for the "full
//   rewriting" terms).
//
// Global Variables: -
//
// Side Effects    : Via term_collect_idx_subterms()
//
/----------------------------------------------------------------------*/

static long eqn_collect_idx_subterms(Eqn_p eqn, PTree_p *rest, PTree_p *full)
{
   long res = 0;
   bool restricted_rw = EqnIsMaximal(eqn) && EqnIsPositive(eqn) && EqnIsOriented(eqn);

   res += term_collect_idx_subterms(eqn->lterm, rest, full, restricted_rw);
   res += term_collect_idx_subterms(eqn->lterm, rest, full, false);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: subterm_occ_free_wrapper()
//
//   Wrapper of type ObjFreeFun.
//
// Global Variables: -
//
// Side Effects    : Via SubtermOccFree()
//
/----------------------------------------------------------------------*/

static void subterm_occ_free_wrapper(void *junk)
{
   SubtermOccFree(junk);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: SubtermOccCellAlloc()
//
//   Allocate an initialized Subterm-Occurance-Cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

SubtermOcc_p SubtermOcclAlloc(Term_p term)
{
   SubtermOcc_p handle = SubtermOccCellAlloc();

   handle->term    = term;
   handle->rw_rest = NULL;
   handle->rw_full = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: SubtermOccFree()
//
//   Free a Subtemerm-Occurance-Cell
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermOccFree(SubtermOcc_p soc)
{
   PTreeFree(soc->rw_rest);
   PTreeFree(soc->rw_full);
   SubtermOccCellFree(soc);
}


/*-----------------------------------------------------------------------
//
// Function: CmpSubtermCells()
//
//   Compare two SubtermOccurance cells via their term entry_no. This
//   is a synthetic but machine-independent measure useful primarily
//   for indexing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int CmpSubtermCells(const void *soc1, const void *soc2)
{
   const SubtermOcc_p s1 = (const SubtermOcc_p) soc1;
   const SubtermOcc_p s2 = (const SubtermOcc_p) soc2;
   
   if(s1->term->entry_no > s2->term->entry_no)
   {
      return 1;
   }
   if(s1->term->entry_no < s2->term->entry_no)
   {
      return -1;
   }
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: SubtermTreeFree()
//
//   Free a subterm tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermTreeFree(PTree_p root)
{
   PObjTreeFree(root, subterm_occ_free_wrapper);
}

/*-----------------------------------------------------------------------
//
// Function: SubtermTreeInsertTerm()
//
//   Return the SubtermOccNode corresponding to term, creating it if it
//   does not exist.
//
// Global Variables: -
//
// Side Effects    : Memory  operations
//
/----------------------------------------------------------------------*/

SubtermOcc_p SubtermTreeInsertTerm(PTree_p *root, Term_p term)
{
   SubtermOcc_p old, newnode = SubtermOcclAlloc(term);

   old = PTreeObjStore(root, newnode, CmpSubtermCells);
   if(old)
   {
      SubtermOccFree(newnode);
      newnode = old;
   }
   return newnode;
}


/*-----------------------------------------------------------------------
//
// Function: SubtermTreeInsertTermOcc() 
//
//   Insert a term occurance into the Subterm tree. Return false if an
//   entry already exists, true otherwise. 
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool SubtermTreeInsertTermOcc(PTree_p *root, Term_p term, 
                              Clause_p clause, bool restricted)   
{
   SubtermOcc_p handle = SubtermTreeInsertTerm(root, term);

   if(restricted)
   {
      return PTreeStore(&(handle->rw_rest), clause);
   }
   return PTreeStore(&(handle->rw_full), clause);
}


/*-----------------------------------------------------------------------
//
// Function: SubtermTreeDeleteTerm()
//
//   Delete the SubtermOccNode corresponding to term,
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermTreeDeleteTerm(PTree_p *root, Term_p term)
{
   SubtermOcc_p old, knode = SubtermOccAlloc(term);

   old = PTreeObjExtractObject(root, knode, CmpSubtermCells);
   SubtermOccFree(old);
   SubtermOccFree(knode);
}



/*-----------------------------------------------------------------------
//
// Function: SubtermTreeDeleteTermOcc()
//
//   Delete an indexing of clause via term.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void SubtermTreeDeleteTermOcc(PTree_p *root, Term_p term, 
                              Clause_p clause, bool restricted)
{
   SubtermOcc_p old, knode = SubtermOcclAlloc(term);
   PTree_p oldnode;

   oldnode = PTreeObjFind(root, knode, CmpSubtermCells);
   if(oldnode)
   {
      old = oldnode->key;
      if(restricted)
      {
         PTreeDeleteEntry(&(old->rw_rest), clause);
      }
      else
      {
         PTreeDeleteEntry(&(old->rw_full), clause);
      }
   }
   SubtermOccFree(knode);   
}










/*-----------------------------------------------------------------------
//
// Function: ClauseCollectIdxSubterms()
//
//   Collect all non-variable subterms in clause either into rest or
//   full (rest for "restricted rewriting" terms, full for the "full
//   rewriting" terms).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseCollectIdxSubterms(Clause_p clause, 
                              PTree_p *rest, 
                              PTree_p *full)
{
   long res = 0;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      res += eqn_collect_idx_subterms(handle, rest, full);
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


