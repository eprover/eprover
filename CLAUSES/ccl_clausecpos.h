/*-----------------------------------------------------------------------

  File  : ccl_clausecpos.h

  Author: Stephan Schulz

  Contents

  Positions of subterms in clauses (and in equations) using compact
  (i.e. single integer) positions.

  Copyright 2010-2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Feb 18 01:31:48 EET 2010

-----------------------------------------------------------------------*/

#ifndef CCL_CLAUSECPOS

#define CCL_CLAUSECPOS

#include <ccl_clausepos.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef long CompactPos;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


CompactPos  PackTermPos(TermPos_p pos);
CompactPos  PackClausePos(ClausePos_p pos);

void        UnpackTermPos(TermPos_p pos, Term_p t, CompactPos cpos);
void        UnpackClausePosInto(CompactPos cpos, Clause_p clause,
                          ClausePos_p pos);
ClausePos_p UnpackClausePos(CompactPos cpos, Clause_p clause);

Term_p      ClauseCPosGetSubterm(Clause_p clause, CompactPos cpos);

Eqn_p       ClauseCPosFirstLit(Clause_p clause, CompactPos *cpos);
Eqn_p       ClauseCPosNextLit(Eqn_p lit, CompactPos *cpos);
Eqn_p       ClauseCPosSplit(Clause_p clause, CompactPos *cpos);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
