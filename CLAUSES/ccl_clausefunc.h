/*-----------------------------------------------------------------------

  File  : ccl_clausefunc.h

  Author: Stephan Schulz

  Contents

  Clause and formula functions that need to know about sets (and
  similar stuff (ccl_clauses.c is to big anyways).

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Tue Aug  7 00:02:44 CEST 2001

  -----------------------------------------------------------------------*/

#ifndef CCL_CLAUSEFUNC

#define CCL_CLAUSEFUNC

#include <ccl_clausesets.h>
#include <ccl_formula_wrapper.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void ClauseRemoveLiteralRef(Clause_p clause, Eqn_p *lit);
void ClauseRemoveLiteral(Clause_p clause, Eqn_p lit);
void ClauseFlipLiteralSign(Clause_p clause, Eqn_p lit);
int  ClauseRemoveSuperfluousLiterals(Clause_p clause);
long ClauseSetRemoveSuperfluousLiterals(ClauseSet_p set);
void ClauseSetCanonize(ClauseSet_p set);
int  ClauseRemoveACResolved(Clause_p clause);
bool ClauseUnitSimplifyTest(Clause_p clause, Clause_p simplifier);
int  ClauseCanonCompareRef(const void *clause1ref, const void* clause2ref);

Clause_p ClauseArchive(ClauseSet_p archive, Clause_p clause);
Clause_p ClauseArchiveCopy(ClauseSet_p archive, Clause_p clause);
void     ClauseSetArchiveCopy(ClauseSet_p archive, ClauseSet_p set);
bool     ClauseIsOrphaned(Clause_p clause);
long     ClauseSetDeleteOrphans(ClauseSet_p set);
bool     ClauseEliminateNakedBooleanVariables(Clause_p clause);
Clause_p ClauseRecognizeInjectivity(TB_p terms, Clause_p clause);
long ClauseSetReplaceInjectivityDefs(ClauseSet_p set, ClauseSet_p archive, TB_p terms);


void PStackClausePrint(FILE* out, PStack_p stack, char* extra);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
