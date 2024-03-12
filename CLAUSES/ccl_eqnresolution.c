/*-----------------------------------------------------------------------

File  : ccl_eqnresolution.c

Author: Stephan Schulz

Contents

   Routines for performing (ordered) equality resolution.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jun  5 18:36:46 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "ccl_eqnresolution.h"
#include <cte_ho_csu.h>



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool EqResOnMaximalLiteralsOnly = true;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: build_resolvent()
//
//   Actually builds eq resolvent
//
// Global Variables: -
//
// Side Effects    : Creates clause
//
/----------------------------------------------------------------------*/

Clause_p build_resolvent(ClausePos_p pos, Subst_p subst, VarBank_p freshvars)
{
   NormSubstEqnListExcept(pos->clause->literals, pos->literal,
            subst, freshvars);
   Eqn_p new_literals = EqnListCopyOptExcept(pos->clause->literals,
                                             pos->literal);
   EqnListLambdaNormalize(new_literals);
   EqnListRemoveResolved(&new_literals);
   EqnListRemoveDuplicates(new_literals);
   return ClauseAlloc(new_literals);
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ComputeEqRes()
//
//   Given a clause and a position, try to perform equality
//   resolution and return the resulting clause.
//   If res_cls is NULL, then it assumes that you want to enumerate
//   only single clause which is returned! Else, it returns NULL
//   but fills res_cls with all clauses
//
// Global Variables: -
//
// Side Effects    : Creates clause
//
/----------------------------------------------------------------------*/

Clause_p ComputeEqRes(TB_p bank, ClausePos_p pos, VarBank_p freshvars,
                      bool* subst_is_ho, PStack_p res_cls)
{
   Subst_p  subst;
   Clause_p res = NULL;


   assert(!EqnIsPositive(pos->literal));
   subst = SubstAlloc();
   VarBankResetVCounts(freshvars);
   if(res_cls)
   {
      CSUIterator_p unif_iter =
         CSUIterInit(pos->literal->lterm, pos->literal->rterm, subst, bank);
      while(NextCSUElement(unif_iter))
      {
         *subst_is_ho = *subst_is_ho || SubstHasHOBinding(subst);
         Clause_p cl = build_resolvent(pos, subst, freshvars);
         PStackPushP(res_cls, cl);
         if(ClauseIsEmpty(cl))
         {
            break; // if empty clause is found do not enumerate unifiers anymore.
         }
      }
      CSUIterDestroy(unif_iter);
   }
   else
   {
      if(SubstMguComplete(pos->literal->lterm, pos->literal->rterm, subst))
      {
         *subst_is_ho = SubstHasHOBinding(subst);
         res = build_resolvent(pos, subst, freshvars);
      }
   }

   SubstDelete(subst);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosFirstEqResLiteral()
//
//   Find the first negative maximal literal in clause and return it.
//
// Global Variables: -
//
// Side Effects    : Changes pos.
//
/----------------------------------------------------------------------*/

Eqn_p ClausePosFirstEqResLiteral(Clause_p clause, ClausePos_p pos)
{
   pos->clause = clause;
   pos->literal = clause->literals;
   PStackReset(pos->pos);

   while(pos->literal)
   {
      if(!EqnIsPositive(pos->literal) &&
    (!EqResOnMaximalLiteralsOnly || EqnIsMaximal(pos->literal)))
      {
    break;
      }
      pos->literal = pos->literal->next;
   }
   return pos->literal;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosNextEqResLiteral()
//
//   Find the next negative maximal literal in clause and return it.
//
// Global Variables: -
//
// Side Effects    : Changes pos.
//
/----------------------------------------------------------------------*/

Eqn_p ClausePosNextEqResLiteral(ClausePos_p pos)
{
   pos->literal = pos->literal->next;

   while(pos->literal)
   {
      if(!EqnIsPositive(pos->literal) &&
    (!EqResOnMaximalLiteralsOnly || EqnIsMaximal(pos->literal)))
      {
    break;
      }
      pos->literal = pos->literal->next;
   }
   return pos->literal;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
