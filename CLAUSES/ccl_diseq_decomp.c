/*-----------------------------------------------------------------------

  File  : ccl_diseq_decomp.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Implement the disequality decomposition inference.

  f(s1,...,sn)!=f(t1,...,tn) | R
  -------------------------------
  s1!=t1 | ... | sn_tn | R

  This wraps the actual inference into one small function.

  Copyright 2024 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created:

  -----------------------------------------------------------------------*/

#include "ccl_diseq_decomp.h"



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
// Function: ClauseDisEqDecomposition()
//
//   Perform the disequality decomposition of clause as litpos.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseDisEqDecomposition(TB_p terms, Clause_p clause,
                                  CompactPos litpos)
{
   Clause_p res = NULL;
   Eqn_p lits, newlits = NULL, inflit, eqn;
   CompactPos tmppos = litpos;
   int i;

   inflit = ClauseCPosSplit(clause, &tmppos);
   assert(inflit);
   assert(tmppos==0);
   assert(inflit->lterm->f_code == inflit->rterm->f_code);
   assert(inflit->lterm->arity == inflit->rterm->arity);

   lits = EqnListCopyOptExcept(clause->literals, inflit);
   for(i=0; i<inflit->lterm->arity; i++)
   {
      eqn = EqnAlloc(inflit->lterm->args[i],
                     inflit->rterm->args[i], terms,
                     false);
      eqn->next = newlits;
      newlits = eqn;
   }
   EqnListAppend(&lits, newlits);

   EqnListRemoveResolved(&lits);
   EqnListRemoveDuplicates(lits);
   res = ClauseAlloc(lits);

   ClausePushDerivation(res, DCDisEqDecompose, clause, NULL);
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
