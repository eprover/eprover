/*-----------------------------------------------------------------------

File  : ccl_clausepos_tree.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Associate clauses with a number of compact positions in clauses.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun  6 13:25:19 CEST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCL_CLAUSEPOS_TREE

#define CCL_CLAUSEPOS_TREE

#include <ccl_clausecpos.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Associate a clause with a number of positions (all with the
 * property that c|p = t for the query term t */

typedef struct clause_tpos_cell
{
   Clause_p  clause;
   NumTree_p pos;
}ClauseTPosCell, *ClauseTPos_p;


typedef PObjTree_p ClauseTPosTree_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define ClauseTPosCellAlloc() (ClauseTPosCell*)SizeMalloc(sizeof(ClauseTPosCell))
#define ClauseTPosCellFree(junk) SizeFree(junk, sizeof(ClauseTPosCell))

ClauseTPos_p ClauseTPosAlloc(Clause_p clause);
void         ClauseTPosFree(ClauseTPos_p soc);
void         ClauseTPosTreeFree(ClauseTPosTree_p tree);

int CmpClauseTPosCells(const void *soc1, const void *soc2);

void ClauseTPosTreeTreeFreeWrapper(void *junk);

void ClauseTPosTreeInsertPos(ClauseTPosTree_p *tree , Clause_p clause,
                             CompactPos pos);
void ClauseTPosTreeDeletePos(ClauseTPosTree_p *tree , Clause_p clause,
                             CompactPos pos);
void ClauseTPosTreeDeleteClause(ClauseTPosTree_p *tree, Clause_p clause);

void ClauseTPosTreePrint(FILE* out, ClauseTPos_p tree);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





