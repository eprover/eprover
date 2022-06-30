/*-----------------------------------------------------------------------

File  : ccl_subterm_tree.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  A simple  mapping from terms to clauses in which this term
  appears as priviledged (rewriting rstricted) or unpriviledged term.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Aug  5 17:25:30 EDT 2009
    New

-----------------------------------------------------------------------*/

#ifndef CCL_SUBTERM_TREE

#define CCL_SUBTERM_TREE

#include <ccl_clauses.h>
#include <ccl_clausepos_tree.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Payload for backwards-rewriting index */

typedef struct bw_rw_payload
{
   PTree_p rw_rest; /* Of clauses in which the subterm appears in a
                       privileged position with restricted rewriting
                       */
   PTree_p rw_full; /* Of clauses in which it appeats unrestricted */
}BWRWPayload;


/* Payload for overlaps (paramodulation) */

typedef struct overlap_payload
{
   PObjTree_p clauses;
}OverlapPayload;


/* Cell for recording all occurrences of a subterm.*/

typedef struct subterm_occ_cell
{
   Term_p       term;
   union
   {
      BWRWPayload    occs;
      OverlapPayload pos;
   }pl;
}SubtermOccCell, *SubtermOcc_p;

typedef PObjTree_p SubtermTree_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define SubtermOccCellAlloc() (SubtermOccCell*)SizeMalloc(sizeof(SubtermOccCell))
#define SubtermOccCellFree(junk) SizeFree(junk, sizeof(SubtermOccCell))

SubtermOcc_p SubtermOccAlloc(Term_p term);
void         SubtermOccFree(SubtermOcc_p soc);
void         SubtermPosFree(SubtermOcc_p soc);

int CmpSubtermCells(const void *soc1, const void *soc2);

void         SubtermBWTreeFree(SubtermTree_p root);
void         SubtermBWTreeFreeWrapper(void *junk);
void         SubtermOLTreeFree(SubtermTree_p root);
void         SubtermOLTreeFreeWrapper(void *junk);

SubtermOcc_p SubtermTreeInsertTerm(SubtermTree_p *root, Term_p term);
SubtermOcc_p SubtermTreeFindTerm(SubtermTree_p *root, Term_p term);
void         SubtermTreeDeleteTerm(SubtermTree_p *root, Term_p term);

bool         SubtermTreeInsertTermOcc(SubtermTree_p *root, Term_p term,
                                      Clause_p clause, bool restricted);
bool         SubtermTreeDeleteTermOcc(SubtermTree_p *root, Term_p term,
                                      Clause_p clause, bool restricted);

void SubtermTreePrint(FILE* out, SubtermTree_p root, Sig_p sig);

void SubtermTreePrintDot(FILE* out, SubtermTree_p root, Sig_p sig);
void SubtermTreePrintDummy(FILE* out, SubtermTree_p root, Sig_p sig);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





