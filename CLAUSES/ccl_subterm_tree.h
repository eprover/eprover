/*-----------------------------------------------------------------------

File  : ccl_subterm_tree.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  A simple  mapping from terms to clauses in which this term
  appears as priviledged (rewriting rstricted) or unpriviledged term. 

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Aug  5 17:25:30 EDT 2009
    New

-----------------------------------------------------------------------*/

#ifndef CCL_SUBTERM_TREE

#define CCL_SUBTERM_TREE

#include <ccl_clauses.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Cell for recording all occurances of a subterm.*/

typedef struct subterm_occ_cell
{
   Term_p  term;
   PTree_p rw_rest; /* Of clauses in which the subterm appears in a
                       privileged position with restricted rewriting
                       */ 
   PTree_p rw_full; /* Of clauses in which it appeats unrestricted */ 
}SubtermOccCell, *SubtermOcc_p;

typedef PTree_p SubtermTree_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define SubtermOccCellAlloc() (SubtermOccCell*)SizeMalloc(sizeof(SubtermOccCell))
#define SubtermOccCellFree(junk) SizeFree(junk, sizeof(SubtermOccCell))

SubtermOcc_p SubtermOccAlloc(Term_p term);
void         SubtermOccFree(SubtermOcc_p soc);

int CmpSubtermCells(const void *soc1, const void *soc2);

void         SubtermTreeFree(SubtermTree_p root);
void         SubtermTreeFreeWrapper(void *junk);

SubtermOcc_p SubtermTreeInsertTerm(SubtermTree_p *root, Term_p term);

bool         SubtermTreeInsertTermOcc(SubtermTree_p *root, Term_p term, 
                                      Clause_p clause, bool restricted);  
void         SubtermTreeDeleteTerm(SubtermTree_p *root, Term_p term);
bool         SubtermTreeDeleteTermOcc(SubtermTree_p *root, Term_p term, 
                                      Clause_p clause, bool restricted);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





