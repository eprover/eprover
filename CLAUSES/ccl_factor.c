/*-----------------------------------------------------------------------

File  : ccl_factor.c

Author: Stephan Schulz

Contents

  Functions for ordered and equality factorisation.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun May 31 19:50:22 MET DST 1998
    New
<2> Tue Oct 13 15:31:57 MET DST 1998
    Added Equality Factoring

-----------------------------------------------------------------------*/

#include "ccl_factor.h"
#include <cte_ho_csu.h>



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: find_next_potential_eq_factor_partner()
//
//   Given two positions, set pos2->literal to the next positive
//   literal (at or including pos2->literal) distinct from
//   pos1->literal.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Eqn_p find_next_potential_eq_factor_partner(ClausePos_p pos1,
                   ClausePos_p pos2)
{
   Eqn_p lit;

   lit = ClausePosFindPosLiteral(pos2, false);
   if(lit==pos1->literal)
   {
      pos2->literal = pos2->literal->next;
      lit =  ClausePosFindPosLiteral(pos2, false);
   }
   return lit;
}

/*-----------------------------------------------------------------------
//
// Function: find_first_eq_factor_partner()
//
//   Given the maximal positive literal described in pos1, set pos2 to
//   the first potential partner for an equality factoring
//   inference. Return the selected literal, or NULL if no exists.
//
// Global Variables: -
//
// Side Effects    : Sets pos2
//
/----------------------------------------------------------------------*/

Eqn_p find_first_eq_factor_partner(ClausePos_p pos1, ClausePos_p pos2)
{
   Eqn_p lit;

   assert(pos1);
   assert(pos2);
   assert(pos1->clause);
   assert(pos1->literal);
   assert(EqnIsPositive(pos1->literal));
   assert(EqnIsMaximal(pos1->literal));

   pos2->clause = pos1->clause;
   pos2->literal = pos1->clause->literals;
   pos2->side = LeftSide;
   PStackReset(pos2->pos);
   lit = find_next_potential_eq_factor_partner(pos1, pos2);
   return lit;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ComputeOrderedFactor()
//
//   Given two positions in a clause, try to compute the ordered
//   factor. Return it, if it exists, otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : Creates clause, temporary bindings (will be
//                   backtracked).
//
/----------------------------------------------------------------------*/

Clause_p ComputeOrderedFactor(TB_p bank, OCB_p ocb, ClausePos_p pos1,
               ClausePos_p pos2, VarBank_p freshvars)
{
   Subst_p  subst;
   Clause_p new_clause = NULL;
   Eqn_p    new_literals;
   bool     unifies;

   assert(pos1->clause == pos2->clause);
   assert(pos1->literal != pos2->literal);
   assert(pos1->side == LeftSide);

   subst = SubstAlloc();
   VarBankResetVCounts(freshvars);

   if(pos2->side == RightSide)
   {
      EqnSwapSidesSimple(pos2->literal);
   }
   unifies = EqnUnifyDirected(pos1->literal, pos2->literal, subst);
   if(pos2->side == RightSide)
   {
      EqnSwapSidesSimple(pos2->literal);
   }
   if(unifies)
   {
      if(EqnListEqnIsMaximal(ocb, pos1->clause->literals,
              pos1->literal))
      {
    NormSubstEqnListExcept(pos1->clause->literals, pos2->literal,
            subst, freshvars);
    new_literals = EqnListCopyOptExcept(pos1->clause->literals,
                                             pos2->literal);
    EqnListRemoveResolved(&new_literals);
    EqnListRemoveDuplicates(new_literals);
    new_clause = ClauseAlloc(new_literals);
      }
   }
   SubstDelete(subst);
   return new_clause;
}



/*-----------------------------------------------------------------------
//
// Function: ClausePosFirstOrderedFactorLiterals()
//
//   Given a clause, compute the first pair of literals were an
//   ordered factor might be computed. See
//   ClausePosNextFactorLiterals(). This works by setting an
//   impossible initial state and searching for the next valid one...
//
// Global Variables: -
//
// Side Effects    : Changes pos1, pos2
//
/----------------------------------------------------------------------*/

Eqn_p ClausePosFirstOrderedFactorLiterals(Clause_p clause, ClausePos_p
                 pos1, ClausePos_p pos2)
{
   Eqn_p lit;

   pos1->clause = clause;
   pos1->literal = clause->literals;
   pos1->side = LeftSide;
   PStackReset(pos1->pos);
   lit = ClausePosFindMaxLiteral(pos1, true);

   if(lit)
   {
      pos2->clause = clause;
      pos2->literal = pos1->literal;
      pos2->side = RightSide;
      PStackReset(pos2->pos);
      return ClausePosNextOrderedFactorLiterals(pos1, pos2);
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function:  ClausePosNextOrderedFactorLiterals()
//
//   Given a clause and two positions, set these position to
//   the next valid combination for an ordered factor
//   inference. Return the second literal, or NULL if no position pair
//   exists. pos2->side is used to indicate wether the unification
//   should take place as is or with one equation swapped.
//
//
// Global Variables: -
//
// Side Effects    : Changes pos1, pos2
//
/----------------------------------------------------------------------*/

Eqn_p ClausePosNextOrderedFactorLiterals(ClausePos_p pos1, ClausePos_p
                pos2)
{
   Eqn_p lit;

   if(pos2->side == LeftSide &&
      (!EqnIsOriented(pos2->literal) ||
       !EqnIsOriented(pos1->literal)))
   {
      pos2->side = RightSide;
      return pos2->literal;
   }
   pos2->side = LeftSide;
   pos2->literal = pos2->literal->next;
   lit = ClausePosFindMaxLiteral(pos2, true);

   while(!lit)
   {
      pos1->literal = pos1->literal->next;
      lit = ClausePosFindMaxLiteral(pos1, true);
      if(!lit)
      {
    break;
      }
      pos2->literal = pos1->literal->next;
      lit = ClausePosFindMaxLiteral(pos2, true);
   }
   return lit;
}


/*-----------------------------------------------------------------------
//
// Function: ComputeEqualityFactor()
//
//   Given two positions in a clause, try to compute the equality
//   factor. Return it, if it exists, otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : Creates clause, temporary bindings (will be
//                   backtracked).
//
/----------------------------------------------------------------------*/

void ComputeEqualityFactor(TB_p bank, OCB_p ocb, ClausePos_p pos1,
                           ClausePos_p pos2, VarBank_p freshvars,
                           bool* subst_is_ho, PStack_p res_clauses)
{
   Term_p  max_term, with_term, min_term, new_lside, new_rside;
   Eqn_p   new_condition, new_literals;
   Subst_p subst = SubstAlloc();

   assert(EqnIsPositive(pos1->literal));
   assert(EqnIsMaximal(pos1->literal));
   assert(EqnIsPositive(pos2->literal));
   assert(!EqnIsOriented(pos1->literal) || (pos1->side == LeftSide));

   max_term  = ClausePosGetSide(pos1);
   with_term = ClausePosGetSide(pos2);

   if((!TermIsFreeVar(max_term)||EqnIsEquLit(pos2->literal))&&
      (!TermIsFreeVar(with_term)||EqnIsEquLit(pos1->literal)))
   {
      CSUIterator_p unif_iter = CSUIterInit(max_term, with_term, subst, bank);
      while(NextCSUElement(unif_iter))
      {
         min_term = ClausePosGetOtherSide(pos1);
         if(!TOGreater(ocb, min_term, max_term, DEREF_ALWAYS, DEREF_ALWAYS)
            &&
            EqnListEqnIsMaximal(ocb, pos1->clause->literals,
                     pos1->literal))
         {
            *subst_is_ho = SubstHasHOBinding(subst);
            NormSubstEqnListExcept(pos1->clause->literals, pos2->literal,
                                   subst, freshvars);
            new_lside = TBInsertNoProps(bank, min_term, DEREF_ALWAYS);
            new_rside = TBInsertNoProps(bank,
                        ClausePosGetOtherSide(pos2),
                        DEREF_ALWAYS);
            new_condition = EqnAlloc(new_lside, new_rside, bank, false);
            new_literals = EqnListCopyOptExcept(pos1->clause->literals,
                        pos1->literal);
            EqnListInsertFirst(&new_literals, new_condition);
            EqnListLambdaNormalize(new_literals);
            EqnListRemoveResolved(&new_literals);
            EqnListRemoveDuplicates(new_literals);
            PStackPushP(res_clauses, ClauseAlloc(new_literals));
         }
      }
      CSUIterDestroy(unif_iter);
   }
   SubstDelete(subst);
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosFirstEqualityFactorSides()
//
//   Given a clause and two uninialized positions, set the positions
//   to the first potiental pair of sides for an equality factoring
//   inference. Return the second literal, or NULL if no legal pair
//   exists.
//
// Global Variables: -
//
// Side Effects    : Changes pos1, pos2
//
/----------------------------------------------------------------------*/

Eqn_p ClausePosFirstEqualityFactorSides(Clause_p clause, ClausePos_p
               pos1, ClausePos_p pos2)
{
   Term_p side;
   Eqn_p  lit = NULL;

   assert(clause);
   assert(pos1);
   assert(pos2);

   pos1->clause = clause;
   pos1->literal = clause->literals;
   side = ClausePosFindFirstMaximalSide(pos1, true);

   if(side)
   {
      lit = find_first_eq_factor_partner(pos1, pos2);
   }
   return lit;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosNextEqualityFactorSides()
//
//   Given a pair of positions pos1, pos2, compute the next potential
//   positions for a equality factoring inference.
//
// Global Variables: -
//
// Side Effects    : Changes pos1, pos2
//
/----------------------------------------------------------------------*/

Eqn_p ClausePosNextEqualityFactorSides(ClausePos_p pos1, ClausePos_p
                   pos2)
{
   Eqn_p  lit;
   Term_p side;

   if(pos2->side == LeftSide)
   {
      pos2->side = RightSide;
      return pos2->literal;
   }
   pos2->side = LeftSide;
   pos2->literal = pos2->literal->next;
   lit = find_next_potential_eq_factor_partner(pos1, pos2);
   if(!lit)
   {
      side = ClausePosFindNextMaximalSide(pos1, true);
      if(side)
      {
    lit = find_first_eq_factor_partner(pos1, pos2);
      }
   }
   return lit;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
