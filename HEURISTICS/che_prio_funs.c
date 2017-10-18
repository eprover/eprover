/*-----------------------------------------------------------------------

File  : che_prio_funs.c

Author: Stephan Schulz

Contents

  Functions for computing priorities. To add a new one, add the
  external name to PrioFunNames and the name of the function to
  prio_fun_array.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Dec  6 17:12:29 MET 1998
    New

-----------------------------------------------------------------------*/

#include "che_prio_funs.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* PrioFunNames[]=
{
   "PreferGroundGoals",
   "PreferUnitGroundGoals",
   "PreferGround",
   "PreferNonGround",
   "PreferProcessed",
   "PreferNew",
   "PreferGoals",
   "PreferNonGoals",
   "PreferMixed",
   "PreferPositive",
   "PreferNegative",
   "PreferUnits",
   "PreferNonEqUnits",
   "PreferDemods",
   "PreferNonUnits",
   "ConstPrio",
   "ByLiteralNumber",
   "ByDerivationDepth",
   "ByDerivationSize",
   "ByNegLitDist",
   "ByGoalDifficulty",
   "SimulateSOS",
   "DeferSOS",
   "PreferHorn",
   "PreferNonHorn",
   "PreferUnitAndNonEq",
   "DeferNonUnitMaxEq",
   "ByCreationDate",
   "PreferWatchlist",
   "DeferWatchlist",
   NULL
};

static ClausePrioFun prio_fun_array[]=
{
   PrioFunPreferGroundGoals,
   PrioFunPreferUnitGroundGoals,
   PrioFunPreferGround,
   PrioFunPreferNonGround,
   PrioFunPreferProcessed,
   PrioFunPreferNew,
   PrioFunPreferGoals,
   PrioFunPreferNonGoals,
   PrioFunPreferMixed,
   PrioFunPreferPositive,
   PrioFunPreferNegative,
   PrioFunPreferUnits,
   PrioFunPreferNonEqUnits,
   PrioFunPreferDemods,
   PrioFunPreferNonUnits,
   PrioFunConstPrio,
   PrioFunByLiteralNumber,
   PrioFunByDerivationDepth,
   PrioFunByDerivationSize,
   PrioFunByNegLitDist,
   PrioFunGoalDifficulty,
   PrioFunSimulateSOS,
   PrioFunDeferSOS,
   PrioFunPreferHorn,
   PrioFunPreferNonHorn,
   PrioFunPreferUnitAndNonEq,
   PrioFunDeferNonUnitMaxPosEq,
   PrioFunByCreationDate,
   PrioFunPreferWatchlist,
   PrioFunDeferWatchlist,
   NULL
};


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: GetPrioFun()
//
//   Given an external name, return a priority function or NULL if the
//   name does not match any known function.
//
// Global Variables: PrioFunNames, prio_fun_array
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ClausePrioFun GetPrioFun(char* name)
{
   int index;

   assert(name);

   index = StringIndex(name, PrioFunNames);

   if(index>=0)
   {
      return prio_fun_array[index];
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: ParsePrioFun()
//
//   Parse a priority function and return it.
//
// Global Variables: -
//
// Side Effects    : May terminate via ScannerError()
//
/----------------------------------------------------------------------*/

ClausePrioFun ParsePrioFun(Scanner_p in)
{
   ClausePrioFun prio_fun;

   CheckInpTok(in, Name);

   prio_fun = GetPrioFun(DStrView(AktToken(in)->literal));

   if(!prio_fun)
   {
      AktTokenError(in, "Not a valid priority-function", false);
   }
   NextToken(in);

   return prio_fun;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferGroundGoals()
//
//   Return PrioPrefer for ground goals, PrioNormal for all other
//   clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferGroundGoals(Clause_p clause)
{
   assert(clause);

   if(ClauseIsGoal(clause)&&ClauseIsGround(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferUnitGroundGoals()
//
//   Return PrioPrefer for unit ground goals, PrioNormal for all other
//   clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferUnitGroundGoals(Clause_p clause)
{
   assert(clause);

   if(ClauseIsUnit(clause)&&ClauseIsGoal(clause)&&ClauseIsGround(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferGround()
//
//   Return PrioPrefer for ground clauses, PrioNormal for all other
//   clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferGround(Clause_p clause)
{
   assert(clause);

   if(ClauseIsGround(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferNonGround()
//
//   Return PrioPrefer for non-ground clauses, PrioNormal for all
//   other clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferNonGround(Clause_p clause)
{
   assert(clause);

   if(!ClauseIsGround(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferProcessed()
//
//   Return PrioPrefer for clauses already procesed and eliminated by
//   backwards-contraction.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferProcessed(Clause_p clause)
{
   assert(clause);

   if(ClauseQueryProp(clause, CPIsProcessed))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferNew()
//
//   Return PrioPrefer for new clauses, PrioNormal for all others. See
//   PrioFunPreferProcessed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferNew(Clause_p clause)
{
   assert(clause);

   if(!ClauseQueryProp(clause, CPIsProcessed))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferGoals()
//
//   Return PrioPrefer for goals, PrioNormal for all other
//   clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferGoals(Clause_p clause)
{
   assert(clause);

   if(ClauseIsGoal(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}



/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferNonGoals()
//
//   Return PrioPrefer for non-goals, PrioNormal for all other
//   clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferNonGoals(Clause_p clause)
{
   assert(clause);

   if(!ClauseIsGoal(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferMixed()
//
//   Return PrioPrefer for clauses that have both positive and
//   negative literals (or neither), PrioNormal for all other clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferMixed(Clause_p clause)
{
   assert(clause);

   if(EQUIV(clause->pos_lit_no, clause->neg_lit_no))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}



/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferPositive()
//
//   Return PrioPrefer for clauses that have no negative literals,
//   PrioNormal for all other clauses. The empty clause is both
//   negative and positive.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferPositive(Clause_p clause)
{
   assert(clause);

   if(ClauseIsPositive(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}



/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferNegative()
//
//   Return PrioPrefer for clauses that have no positive
//   literals, PrioNormal for all other clauses.  At the moment, this
//   is mostly equivalent to PrioFunPreferGoals, but in the medium
//   term I want to decouple the notion of goal (user-specified) from
//   that of negative clause. The empty clause is both negative and
//   positive.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferNegative(Clause_p clause)
{
   assert(clause);

   if(ClauseIsNegative(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}





/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferUnits()
//
//   Return PrioPrefer for unit-clauses, PrioNormal for all other
//   clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferUnits(Clause_p clause)
{
   assert(clause);

   if(ClauseIsUnit(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}

/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferNonEqUnits()
//
//   Return PrioPrefer for non-equational unit-clauses, PrioNormal for
//   all other clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferNonEqUnits(Clause_p clause)
{
   assert(clause);

   if(ClauseIsUnit(clause)&&!ClauseIsEquational(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferDemods()
//
//   Return PrioPrefer for positive equational unit-clauses,
//   PrioNormal for all other clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferDemods(Clause_p clause)
{
   assert(clause);

   if(ClauseIsUnit(clause)&&
      ClauseIsEquational(clause)&&
      ClauseIsPositive(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferNonUnits()
//
//   Return PrioPrefer for non-unit-clauses, PrioNormal for all other
//   clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferNonUnits(Clause_p clause)
{
   assert(clause);

   if(!ClauseIsUnit(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}



/*-----------------------------------------------------------------------
//
// Function: PrioFunConstPrio()
//
//   Return PrioNormal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunConstPrio(Clause_p clause)
{
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunByLiteralNumber()
//
//   Return number of literals in the clause as a priority.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


EvalPriority PrioFunByLiteralNumber(Clause_p clause)
{
   return ClauseLiteralNumber(clause);
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunByDerivationDepth()
//
//   Return the derivation depth of the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


EvalPriority PrioFunByDerivationDepth(Clause_p clause)
{
   return clause->proof_depth;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunByDerivationSize()
//
//   Return the derivation size of the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


EvalPriority PrioFunByDerivationSize(Clause_p clause)
{
   return clause->proof_size;
}

/*-----------------------------------------------------------------------
//
// Function: PrioFunByNegLitDist()
//
//   Give a priority based on the number of negative (ground)
//   literals: A negative-non-ground literal adds 3, a negative ground
//   literal adds 1. Clauses with non-negative literals get a fixed
//   priority.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunByNegLitDist(Clause_p clause)
{
   Eqn_p handle;
   EvalPriority res = PrioNormal;

   for(handle=clause->literals; handle; handle=handle->next)
   {
      if(EqnIsPositive(handle))
      {
    res=400;
    break;
      }
      else
      {
    res+= EqnIsGround(handle)?1:3;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunGoalDifficulty()
//
//   Give a priorty based on how simple a goal seems to be:
//   Unit-Ground, Unit, Ground, General
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunGoalDifficulty(Clause_p clause)
{
   if(!ClauseIsGoal(clause))
   {
      return PrioNormal;
   }
   if(ClauseIsUnit(clause))
   {
      if(ClauseIsGround(clause))
      {
    return PrioPrefer;
      }
      else
      {
    return PrioPrefer+1;
      }
   }
   else if(ClauseIsGround(clause))
   {
      return PrioPrefer+2;
   }
   return PrioPrefer+3;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunSimulateSOS()
//
//   Give priority PrioNormal to SOS clauses and
//   initial clauses, and PrioDefer otherwise. Note that CPInitial is
//   intentional and correkt ;-)
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunSimulateSOS(Clause_p clause)
{
   if(ClauseIsAnyPropSet(clause, CPInitial|CPIsSOS))
   {
      return PrioNormal;
   }
   return PrioDefer;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunDeferSOS()
//
//   Give priority to non-SOS and non-initial clauses.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunDeferSOS(Clause_p clause)
{
   if(ClauseIsAnyPropSet(clause, CPInitial|CPIsSOS))
   {
      return PrioDefer;
   }
   return PrioNormal;
}



/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferHorn()
//
//   Return PrioPrefer for Horn clauses, PrioNormal for all other
//   clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferHorn(Clause_p clause)
{
   assert(clause);

   if(ClauseIsHorn(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferNonHorn()
//
//   Return PrioPrefer for Non-Horn clauses, PrioNormal for all other
//   clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferNonHorn(Clause_p clause)
{
   assert(clause);

   if(!ClauseIsHorn(clause))
   {
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferUnitAndNonEq()
//
//   Return PrioPrefer for units and all non-equational clauses,
//   PrioNormal for all other clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferUnitAndNonEq(Clause_p clause)
{
   assert(clause);

   if(ClauseIsUnit(clause))
   {
      return PrioPrefer;
   }
   if(ClauseIsEquational(clause))
   {
      return PrioNormal;
   }
   return PrioPrefer;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunDeferNonUnitMaxPosEq()
//
//   Return PrioPrefer for units and clauses without maximal positive
//   equational literal, PrioNormal otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunDeferNonUnitMaxPosEq(Clause_p clause)
{
   assert(clause);

   if(ClauseIsUnit(clause))
   {
      return PrioPrefer;
   }
   if(ClauseHasMaxPosEqLit(clause))
   {
      return PrioNormal;
   }
   return PrioPrefer;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunByCreationDate()
//
//   Return the creation date of the clause. This allows us to combine
//   a better FIFO with any other heuristic to sort clauses
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunByCreationDate(Clause_p clause)
{
   assert(clause);

   return clause->create_date;
}

/*-----------------------------------------------------------------------
//
// Function: PrioFunPreferWatchlist()
//
//   Prefer clauses that have subsumed a watchlist clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunPreferWatchlist(Clause_p clause)
{
   assert(clause);

   if(ClauseQueryProp(clause, CPSubsumesWatch))
   {
      //printf("# Found: ");
      //ClausePrint(stdout, clause, true);
      //printf("\n");
      return PrioPrefer;
   }
   return PrioNormal;
}


/*-----------------------------------------------------------------------
//
// Function: PrioFunDeferWatchlist()
//
//   Defer clauses that have subsumed a watchlist clause (probably
//   useful only for symmetry reasons).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EvalPriority PrioFunDeferWatchlist(Clause_p clause)
{
   assert(clause);

   if(ClauseQueryProp(clause, CPSubsumesWatch))
   {
      return PrioDefer;
   }
   return PrioNormal;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


