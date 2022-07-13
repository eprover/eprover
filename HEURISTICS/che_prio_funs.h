/*-----------------------------------------------------------------------

File  : che_prio_funs.h

Author: Stephan Schulz

Contents

  Functions dealing with priorities for clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Dec  5 16:45:41 MET 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_PRIO_FUNS

#define CHE_PRIO_FUNS

#include <clb_simple_stuff.h>
#include <ccl_clauses.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef EvalPriority (*ClausePrioFun)(Clause_p clause);


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char* PrioFunNames[];

ClausePrioFun GetPrioFun(char* name);
ClausePrioFun ParsePrioFun(Scanner_p in);

EvalPriority PrioFunPreferGroundGoals(Clause_p clause);
EvalPriority PrioFunPreferUnitGroundGoals(Clause_p clause);
EvalPriority PrioFunPreferGround(Clause_p clause);
EvalPriority PrioFunPreferNonGround(Clause_p clause);
EvalPriority PrioFunPreferProcessed(Clause_p clause);
EvalPriority PrioFunPreferNew(Clause_p clause);
EvalPriority PrioFunPreferGoals(Clause_p clause);
EvalPriority PrioFunPreferNonGoals(Clause_p clause);
EvalPriority PrioFunPreferMixed(Clause_p clause);
EvalPriority PrioFunPreferPositive(Clause_p clause);
EvalPriority PrioFunPreferNegative(Clause_p clause);
EvalPriority PrioFunPreferUnits(Clause_p clause);
EvalPriority PrioFunPreferNonEqUnits(Clause_p clause);
EvalPriority PrioFunPreferDemods(Clause_p clause);
EvalPriority PrioFunPreferNonUnits(Clause_p clause);
EvalPriority PrioFunConstPrio(Clause_p clause);
EvalPriority PrioFunByLiteralNumber(Clause_p clause);
EvalPriority PrioFunByDerivationDepth(Clause_p clause);
EvalPriority PrioFunByDerivationSize(Clause_p clause);
EvalPriority PrioFunByNegLitDist(Clause_p clause);
EvalPriority PrioFunGoalDifficulty(Clause_p clause);
EvalPriority PrioFunSimulateSOS(Clause_p clause);
EvalPriority PrioFunDeferSOS(Clause_p clause);
EvalPriority PrioFunPreferHorn(Clause_p clause);
EvalPriority PrioFunPreferNonHorn(Clause_p clause);
EvalPriority PrioFunPreferUnitAndNonEq(Clause_p clause);
EvalPriority PrioFunDeferNonUnitMaxPosEq(Clause_p clause);
EvalPriority PrioFunByCreationDate(Clause_p clause);
EvalPriority PrioFunByPosLitNo(Clause_p clause);
EvalPriority PrioFunByHornDist(Clause_p clause);
EvalPriority PrioFunPreferWatchlist(Clause_p clause);
EvalPriority PrioFunDeferWatchlist(Clause_p clause);


EvalPriority PrioFunByAppVarNum(Clause_p clause);
EvalPriority PrioFunPreferAppVar(Clause_p clause);
EvalPriority PrioFunPreferNonAppVar(Clause_p clause);
EvalPriority PrioFunPreferHOSteps(Clause_p clause);
EvalPriority PrioFunPreferLambdas(Clause_p clause);
EvalPriority PrioFunDeferLambdas(Clause_p clause);
EvalPriority PrioFunPreferFormulas(Clause_p clause);
EvalPriority PrioFunDeferFormulas(Clause_p clause);
EvalPriority PrioFunPreferEasyHO(Clause_p clause);
EvalPriority PrioFunPreferFO(Clause_p clause);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
