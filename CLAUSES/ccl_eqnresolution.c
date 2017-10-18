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



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ComputeEqRes()
//
//   Given a clause and a position, try to perform equality
//   resolution and return the resulting clause.
//
// Global Variables: -
//
// Side Effects    : Creates clause
//
/----------------------------------------------------------------------*/

Clause_p ComputeEqRes(TB_p bank, ClausePos_p pos, VarBank_p freshvars)
{
   Subst_p  subst;
   Clause_p new_clause = NULL;
   Eqn_p    new_literals;
   bool     unifies;

   assert(!EqnIsPositive(pos->literal));
   subst = SubstAlloc();
   VarBankResetVCount(freshvars);

   unifies = SubstComputeMgu(pos->literal->lterm, pos->literal->rterm,
              subst);
   if(unifies)
   {
      NormSubstEqnListExcept(pos->clause->literals, pos->literal,
              subst, freshvars);
      new_literals = EqnListCopyOptExcept(pos->clause->literals,
                                          pos->literal);
      EqnListRemoveResolved(&new_literals);
      EqnListRemoveDuplicates(new_literals);
      new_clause = ClauseAlloc(new_literals);
   }
   SubstDelete(subst);

   return new_clause;
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


