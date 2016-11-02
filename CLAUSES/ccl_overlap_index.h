/*-----------------------------------------------------------------------

File  : ccl_overlap_index.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  A simple (hashed) index from terms to clause position sets
  (organized as trees of clauses with a tree of positions at which the
  term occurs. Positions are encoded in a two-level tree itself:
  Position sets are indexed by clauses.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Jun  3 11:30:36 CEST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCL_OVERLAP_INDEX

#define CCL_OVERLAP_INDEX

#include <cte_fp_index.h>
#include <ccl_subterm_tree.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef FPIndex_p OverlapIndex_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void OverlapIndexInsertPos(OverlapIndex_p index, Clause_p clause,
                           CompactPos pos, Term_p iterm);
void OverlapIndexDeletePos(OverlapIndex_p index, Clause_p clause,
                           CompactPos pos, Term_p iterm);

void OverlapIndexDeleteClauseOcc(OverlapIndex_p index,
                                 Clause_p clause, Term_p term);

long ClauseCollectIntoTerms(Clause_p clause, PTree_p *terms);
long ClauseCollectIntoTermsPos(Clause_p clause, PStack_p terms);

long ClauseCollectFromTerms(Clause_p clause, PTree_p *terms);
long ClauseCollectFromTermsPos(Clause_p clause, PStack_p terms);


void OverlapIndexInsertIntoClause(OverlapIndex_p index, Clause_p clause);
void OverlapIndexDeleteIntoClause(OverlapIndex_p index, Clause_p clause);

void OverlapIndexInsertFromClause(OverlapIndex_p index, Clause_p clause);
void OverlapIndexDeleteFromClause(OverlapIndex_p index, Clause_p clause);

void OverlapIndexFPLeafPrint(FILE* out, PStack_p stack, FPTree_p leaf);

long ClauseCollectIntoTerms2(Clause_p clause, PTree_p *terms, PTree_p *natoms);

long ClauseCollectIntoTermsPos2(Clause_p clause,
                                PStack_p terms,
                                PStack_p natoms);
void OverlapIndexInsertIntoClause2(OverlapIndex_p tindex,
                                   OverlapIndex_p naindex,
                                   Clause_p clause);
void OverlapIndexDeleteIntoClause2(OverlapIndex_p tindex,
                                   OverlapIndex_p naindex,
                                   Clause_p clause);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





