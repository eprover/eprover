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
// Function: try_condensation()
//
//   Try to condense literals l1 and l2 in clause. If successful,
//   modify clause and return true, otherwise return false.
//
// Global Variables: -
//
// Side Effects    : Modification of clause
//
/----------------------------------------------------------------------*/

static bool try_condensation(Clause_p clause, Eqn_p l1, Eqn_p l2, bool swap)
{
   Subst_p  subst = SubstAlloc();
   Eqn_p    newlits;
   Clause_p cand;
   bool     res = false;

   if(LiteralUnifyOneWay(l1, l2, subst, false))
   {
      newlits = EqnListCopyExcept(clause->literals,l2, l1->bank);
      SubstBacktrack(subst);
      EqnListRemoveDuplicates(newlits);
      EqnListRemoveResolved(&newlits);
      cand = ClauseAlloc(newlits);
      cand->weight = ClauseStandardWeight(cand);
      ClauseSubsumeOrderSortLits(cand);
      if(ClauseSubsumesClause(cand, clause))
      {
         EqnListFree(clause->literals);
         clause->literals = cand->literals;
         ClauseRecomputeLitCounts(clause);
         clause->weight = ClauseStandardWeight(clause);
         cand->literals = NULL;
         ClauseFree(cand);

         res = true;
      }
      else
      {
         ClauseFree(cand);
      }
   }
   SubstDelete(subst);

   return res;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: CondenseOnce()
//
//   Try to condense clause. If successful, simplify the clause, and
//   return true. If not, the clause is unchanged and false is
//   returned.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool CondenseOnce(Clause_p clause)
{
   Eqn_p    l1, l2;

   assert(ClauseIsSubsumeOrdered(clause));

   for(l1=clause->literals; l1; l1=l1->next)
   {
      assert(l1);
      for(l2=l1->next; l2; l2=l2->next)
      {
         if(try_condensation(clause, l1, l2, false))
         {
            return true;
         }
         if((!EqnIsOriented(l1))||(!EqnIsOriented(l2)))
         {
            if(try_condensation(clause, l1, l2, true))
            {
               return true;
            }
         }
      }
   }
   return false;
}



/*-----------------------------------------------------------------------
//
// Function: Condense()
//
//   Condense a clause as much as possible. Return true if the clause
//   was changed, false otherwise.
//
// Global Variables:
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool Condense(Clause_p clause)
{
   bool res = false;

   CondensationAttempts++;

   if((clause->pos_lit_no > 1) || (clause->neg_lit_no >1))
   {
      clause->weight = ClauseStandardWeight(clause);
      ClauseSubsumeOrderSortLits(clause);
      while(CondenseOnce(clause))
      {
         res = true;
      }
      if(res)
      {
         CondensationSuccesses++;
         DocClauseModificationDefault(clause, inf_condense, NULL);
         ClausePushDerivation(clause, DCCondense, NULL, NULL);
      }
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
