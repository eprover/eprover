/*-----------------------------------------------------------------------

File  : ccl_condensation.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Implementation of the condensation rule.

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1>     New

-----------------------------------------------------------------------*/

#include "ccl_condensation.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long CondensationAttempts = 0;
long CondensationSuccesses = 0;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: lit_matches()
//
//   Try to extend subst to a match from pattern onto target (with
//   the sides of target swapped if swap is true). Return true on
//   success. On failure, subst is unchanged.
//
// Global Variables: -
//
// Side Effects    : Extends subst
//
/----------------------------------------------------------------------*/

static bool lit_matches(Eqn_p pattern, Eqn_p target, bool swap, Subst_p subst)
{
   PStackPointer state = PStackGetSP(subst);
   Term_p lterm = swap? target->rterm : target->lterm;
   Term_p rterm = swap? target->lterm : target->rterm;

   if(SubstMatchComplete(pattern->lterm, lterm, subst) &&
      SubstMatchComplete(pattern->rterm, rterm, subst))
   {
      return true;
   }
   SubstBacktrackToPos(subst, state);
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: lit_matches_some()
//
//   Return true if pattern matches some literal of targets other
//   than except. Equations are matched in both orientations.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lit_matches_some(Eqn_p pattern, Eqn_p targets, Eqn_p except)
{
   Subst_p subst = SubstAlloc();
   bool    res = false;
   Eqn_p   target;

   for(target = targets; target && !res; target = target->next)
   {
      if((target == except) ||
         !PropsAreEquiv(pattern, target, EPIsPositive|EPIsEquLiteral))
      {
         continue;
      }
      res = lit_matches(pattern, target, false, subst) ||
         (EqnIsEquLit(pattern) && lit_matches(pattern, target, true, subst));
      SubstBacktrack(subst);
   }
   SubstDelete(subst);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: lit_list_set_subsumes()
//
//   Return true if subst can be extended to a substitution that maps
//   every literal of patterns onto some literal of targets other than
//   except. Several patterns may be mapped onto the same target (set
//   subsumption, not multiset subsumption). Equations are matched in
//   both orientations. On failure, subst is unchanged. On success,
//   the caller has to backtrack subst.
//
// Global Variables: -
//
// Side Effects    : Extends subst
//
/----------------------------------------------------------------------*/

static bool lit_list_set_subsumes(Eqn_p patterns, Eqn_p targets,
                                  Eqn_p except, Subst_p subst)
{
   Eqn_p         target;
   PStackPointer state = PStackGetSP(subst);

   if(!patterns)
   {
      return true;
   }
   for(target = targets; target; target = target->next)
   {
      if((target == except) ||
         !PropsAreEquiv(patterns, target, EPIsPositive|EPIsEquLiteral))
      {
         continue;
      }
      if(lit_matches(patterns, target, false, subst))
      {
         if(lit_list_set_subsumes(patterns->next, targets, except, subst))
         {
            return true;
         }
         SubstBacktrackToPos(subst, state);
      }
      if(EqnIsEquLit(patterns) && lit_matches(patterns, target, true, subst))
      {
         if(lit_list_set_subsumes(patterns->next, targets, except, subst))
         {
            return true;
         }
         SubstBacktrackToPos(subst, state);
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: lit_is_condensable()
//
//   Return true if lit can be deleted from clause by condensation,
//   i.e. if there is a substitution sigma with clause*sigma a subset
//   of clause\{lit}. As a cheap necessary condition, lit has to
//   match another literal of the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lit_is_condensable(Clause_p clause, Eqn_p lit)
{
   Subst_p subst;
   bool    res;

   if(!lit_matches_some(lit, clause->literals, lit))
   {
      return false;
   }
   subst = SubstAlloc();
   res = lit_list_set_subsumes(clause->literals, clause->literals, lit, subst);
   SubstDelete(subst);

   return res;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: Condense()
//
//   Condense a clause as much as possible. Return true if the clause
//   was changed, false otherwise. A full proof object keeps the clause
//   before the step in step_archive.
//
//   A literal L of C can be deleted if C*sigma is a subset of C\{L}
//   for some sigma. C is condensed if no literal can be deleted, and
//   one pass over the literals suffices: if L cannot be deleted from
//   C, it cannot be deleted from any C' obtained from C by deleting
//   literals this way either (C'*sigma subset of C'\{L} implies
//   C*rho*sigma subset of C\{L}, where C*rho is a subset of C').
//   This follows the implementation in SPASS.
//
// Global Variables: CondensationAttempts, CondensationSuccesses
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

bool Condense(Clause_p clause, ClauseSet_p step_archive)
{
   bool  res = false;
   EqnRef lit;

   CondensationAttempts++;

   if((clause->pos_lit_no > 1) || (clause->neg_lit_no >1))
   {
      Clause_p before = ClauseArchiveStepBegin(step_archive, clause);
      /* Heavier literals first, so that failing matches fail early. */
      ClauseSubsumeOrderSortLits(clause);
      lit = &(clause->literals);
      while(*lit)
      {
         if(lit_is_condensable(clause, *lit))
         {
            EqnListDeleteElement(lit);
            res = true;
         }
         else
         {
            lit = &((*lit)->next);
         }
      }
      if(res)
      {
         ClauseRecomputeLitCounts(clause);
         clause->weight = ClauseStandardWeight(clause);
      }
      ClauseArchiveStepEnd(step_archive, clause, before, res);
      if(res)
      {
         CondensationSuccesses++;
         DocClauseModificationDefault(clause, inf_condense, NULL);
         ClausePushDerivation(clause, DCCondense);
      }
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
