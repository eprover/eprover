/*-----------------------------------------------------------------------

  File  : ccl_subsumption.h

  Author: Stephan Schulz

  Contents

  Functions for subsumption testing -> test a clause against a (unit)
  clauseset, test a clause set against a (unit) clause.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Sat Jul  5 02:28:25 MET DST 1997

  -----------------------------------------------------------------------*/

#ifndef CCL_SUBSUPTION

#define CCL_SUBSUPTION

#include <clb_os_wrapper.h>
#include <ccl_unit_simplify.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

PERF_CTR_DECL(SetSubsumeTimer);
PERF_CTR_DECL(SubsumeTimer);

/* A subsumption test (multiset or plain subsumption) */
typedef bool (*ClauseSubsumesFun)(Clause_p subsumer, Clause_p sub_candidate);

extern bool StrongUnitForwardSubsumption;
extern long ClauseClauseSubsumptionCalls;
extern long ClauseClauseSubsumptionCallsRec;
extern long ClauseClauseSubsumptionSuccesses;
extern long UnitClauseClauseSubsumptionCalls;


bool     LiteralSubsumesClause(Eqn_p literal, Clause_p clause);
bool     UnitClauseSubsumesClause(Clause_p unit, Clause_p clause);
Clause_p UnitClauseSetSubsumesClause(ClauseSet_p set, Clause_p
                                     clause);
bool     ClausePositiveSimplifyReflect(ClauseSet_p set, Clause_p clause,
                                       ClauseSet_p step_archive);
bool     ClauseNegativeSimplifyReflect(ClauseSet_p set, Clause_p clause,
                                       ClauseSet_p step_archive);
Clause_p ClauseSetFindUnitSubsumedClause(ClauseSet_p set, Clause_p
                                         set_position, Clause_p
                                         subsumer);

bool     ClauseMultiSetSubsumesClause(Clause_p subsumer,
                                      Clause_p sub_candidate);
bool     ClausePlainSubsumesClause(Clause_p subsumer,
                                   Clause_p sub_candidate);
bool     ClausePlainSubsumesClauseExcept(Clause_p subsumer,
                                         Clause_p sub_candidate,
                                         Eqn_p except);

Clause_p ClauseSetMultiSetSubsumesClause(ClauseSet_p set,
                                         Clause_p sub_candidate);
Clause_p ClauseSetPlainSubsumesClause(ClauseSet_p set,
                                      Clause_p sub_candidate);

Clause_p ClauseSetFindMultiSetSubsumedClause(ClauseSet_p set,
                                             Clause_p set_position,
                                             Clause_p subsumer);

long     ClauseSetFindMultiSetSubsumedClauses(ClauseSet_p set,
                                              Clause_p subsumer,
                                              PStack_p res);
long     ClauseSetFindPlainSubsumedClauses(ClauseSet_p set,
                                           Clause_p subsumer,
                                           PStack_p res);

Clause_p ClauseSetFindFirstMultiSetSubsumedClause(ClauseSet_p set,
                                                  Clause_p subsumer);
Clause_p ClauseSetFindFirstPlainSubsumedClause(ClauseSet_p set,
                                               Clause_p subsumer);

Clause_p ClauseSetFindVariantClause(ClauseSet_p set,
                                    Clause_p clause);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
