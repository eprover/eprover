/*-----------------------------------------------------------------------

File  : ccl_subterm_index.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  A simple (hashed) index from terms to clauses in which this term
  appears as priviledged (rewriting restricted) or unpriviledged term.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed May  5 10:19:14 CEST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCL_SUBTERM_INDEX

#define CCL_SUBTERM_INDEX

#include <cte_fp_index.h>
#include <ccl_subterm_tree.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef FPIndex_p SubtermIndex_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


bool SubtermIndexInsertOcc(SubtermIndex_p index, Clause_p clause,
                           Term_p term, bool restricted);
bool SubtermIndexDeleteOcc(SubtermIndex_p index, Clause_p clause,
                           Term_p term, bool restricted);


long ClauseCollectIdxSubterms(Clause_p clause,
                              PTree_p *rest,
                              PTree_p *full,
                              bool lambda_demod);

void SubtermIndexInsertClause(SubtermIndex_p index, Clause_p clause, bool lambda_demod);
void SubtermIndexDeleteClause(SubtermIndex_p index, Clause_p clause, bool lambda_demod);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





