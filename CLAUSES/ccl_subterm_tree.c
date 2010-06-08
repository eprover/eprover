/*-----------------------------------------------------------------------

File  : ccl_subterm_tree.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  A tree-based mapping mapping subterms to occurances in clauses.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 14 09:27:17 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_subterm_tree.h"



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
// Function: SubtermOccAlloc()
//
//   Allocate an initialized Subterm-Occurance-Cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

SubtermOcc_p SubtermOccAlloc(Term_p term)
{
   SubtermOcc_p handle = SubtermOccCellAlloc();

   handle->term    = term;
   handle->pl.occs.rw_rest = NULL;
   handle->pl.occs.rw_full = NULL;

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
   PTreeFree(soc->pl.occs.rw_rest);
   PTreeFree(soc->pl.occs.rw_full);
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

void SubtermTreeFree(SubtermTree_p root)
{
   PObjTreeFree(root, subterm_occ_free_wrapper);
}


/*-----------------------------------------------------------------------
//
// Function: SubtermTreeFreeWrapper()
//
//   Free a subterm tree, with proper signature for FPIndexFree().
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermTreeFreeWrapper(void *junk)
{
   SubtermTreeFree(junk);
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

SubtermOcc_p SubtermTreeInsertTerm(SubtermTree_p *root, Term_p term)
{
   SubtermOcc_p old, newnode = SubtermOccAlloc(term);

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

bool SubtermTreeInsertTermOcc(SubtermTree_p *root, Term_p term, 
                              Clause_p clause, bool restricted)   
{
   SubtermOcc_p handle = SubtermTreeInsertTerm(root, term);

   if(restricted)
   {
      return PTreeStore(&(handle->pl.occs.rw_rest), clause);
   }
   return PTreeStore(&(handle->pl.occs.rw_full), clause);
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

void SubtermTreeDeleteTerm(SubtermTree_p *root, Term_p term)
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

bool SubtermTreeDeleteTermOcc(SubtermTree_p *root, Term_p term, 
                              Clause_p clause, bool restricted)
{
   SubtermOcc_p old, knode = SubtermOccAlloc(term);
   SubtermTree_p oldnode;
   bool res = false;

   oldnode = PTreeObjFind(root, knode, CmpSubtermCells);
   if(oldnode)
   {
      old = oldnode->key;
      if(restricted)
      {
         res = PTreeDeleteEntry(&(old->pl.occs.rw_rest), clause);
      }
      else
      {
         res = PTreeDeleteEntry(&(old->pl.occs.rw_full), clause);
      }
      if((old->pl.occs.rw_rest == NULL) && (old->pl.occs.rw_full == NULL))
      {
         SubtermTreeDeleteTerm(root, term);
      }
   }
   SubtermOccFree(knode);   

   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


