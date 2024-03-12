/*-----------------------------------------------------------------------

  File  : cco_diseq_decomp.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Code to control the computation of disequality decomposition. The
  disequality decomposition inference is

  f(s1,...,sn)!=f(t1,...,tn) | R
  -------------------------------
  s1!=t1 | ... | sn_tn | R

  Copyright 2024 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created:

  -----------------------------------------------------------------------*/

#include "cco_diseq_decomp.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


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
// Function: ComputeDisEqDecompositions()
//
//   Compute all disequality decompositions compatiple with the
//   parameters, and add them to store. Returns the number of clauses
//   created.
//
// Global Variables: -
//
// Side Effects    : Creates new clause, memory operations
//
/----------------------------------------------------------------------*/

long ComputeDisEqDecompositions(TB_p terms, Clause_p clause,
                                ClauseSet_p store,
                                long diseq_decomposition,
                                long diseq_decomp_maxarity)
{
   long count = 0;
   CompactPos litpos;
   Eqn_p lit;
   Clause_p new_clause;

   if(ClauseLiteralNumber(clause) <= diseq_decomposition)
   {
      lit = ClauseCPosFirstLit(clause, &litpos);
      while(lit)
      {
         if(EqnIsEquLit(lit) && EqnIsNegative(lit) &&
            lit->lterm->f_code == lit->rterm->f_code &&
            lit->lterm->arity <= diseq_decomp_maxarity &&
            lit->lterm->arity)
         {
            new_clause = ClauseDisEqDecomposition(terms, clause, litpos);
            if(new_clause)
            {
               ClauseSetInsert(store, new_clause);
               count++;
            }
         }
         lit = ClauseCPosNextLit(lit, &litpos);
      }
   }
   return count;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
