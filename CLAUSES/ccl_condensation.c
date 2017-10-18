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
   Eqn_p    l1, l2, newlits;
   Subst_p  subst = SubstAlloc();
   Clause_p cand;
   bool     swap;

   assert(ClauseIsSubsumeOrdered(clause));

   for(l1=clause->literals; l1; l1=l1->next)
   {
      assert(l1);
      for(l2=l1->next; l2; l2=l2->next)
      {
         for(swap = false; !swap; swap = true)
         {
            if(LiteralUnifyOneWay(l1, l2, subst, swap))
            {
               newlits = EqnListCopyExcept(clause->literals,l2, l1->bank);
               SubstBacktrack(subst);
               EqnListRemoveDuplicates(newlits);
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
                  SubstFree(subst);

                  return true;
               }
               else
               {
                  ClauseFree(cand);
               }
            }
         }
      }
   }
   SubstFree(subst);
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


