/*-----------------------------------------------------------------------

File  : cle_classification.c

Author: Stephan Schulz

Contents

  Functions for classifying with TSM's

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Aug 13 20:28:03 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include "cle_classification.h"


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
// Function: TSMTermClassify()
//
//   Classify a term with a TSM, i.e. return -1 if the evaluation is
//   lower than limit, +1 otherwise
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double TSMTermClassify(TSMAdmin_p admin, Term_p term, PatternSubst_p
             subst)
{
   double eval = TSMEvalTerm(admin, term, subst);

   printf("Evaluation: %7.4f ", eval);
   return TSMEvalNormalize(eval, admin->eval_limit);
}


/*-----------------------------------------------------------------------
//
// Function: TSMClassifiedTermCheck()
//
//   Classify term on the tsm and compare it with the original
//   classification. Return true if they match, false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TSMClassifiedTermCheck(TSMAdmin_p admin, FlatAnnoTerm_p term)
{
   double res = TSMTermClassify(admin, term->term, admin->subst);

   printf(" Termeval: %7.4f ", term->eval);
   return res == term->eval;
}

/*-----------------------------------------------------------------------
//
// Function: TSMClassifySet()
//
//   Classify all terms in set with the TSM, return number of
//   successes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TSMClassifySet(TSMAdmin_p admin, FlatAnnoSet_p set)
{
   NumTree_p      handle;
   PStack_p       stack;
   FlatAnnoTerm_p term;
   long           res = 0;

   stack = NumTreeTraverseInit(set->set);

   while((handle = NumTreeTraverseNext(stack)))
   {
      term = handle->val1.p_val;
      if(TSMClassifiedTermCheck(admin, term))
      {
    printf("OKOK ");
    res+=term->sources;
      }
      else
      {
    printf("FAIL ");
      }
      TermPrint(stdout, term->term, admin->index_bank->sig,
      DEREF_NEVER);
      printf("\n");
   }
   NumTreeTraverseExit(stack);
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


