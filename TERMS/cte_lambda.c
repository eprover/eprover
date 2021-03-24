/*-----------------------------------------------------------------------

  File  : cte_lambda.c

  Author: Petar Vukmirovic

  Contents

  Functions that implement main operations of lambda calculus

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Mar 24 15:56:00 CET 2021

-----------------------------------------------------------------------*/


#include "cte_lambda.h"
#include "cte_termbanks.h"
#include "cte_subst.h"

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
// Function: reduce_head()
//
//   Reduces all heading reducible lambda binders
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p reduce_head (Term_p t)
{
   assert(TermIsPhonyApp(t) && TermIsLambda(t->args[0]));

   Term_p body = t->args[0];
   Subst_p subst = SubstAlloc();
   int     i = 1;

   while(TermIsLambda(body) && i < t->arity)
   {
      // cancelling old bindings -- for name clashes (e.g. ^[X,X]:...)
      body->args[0]->binding = NULL; 
      SubstAddBinding(subst, body->args[0], t->args[i++]);
      body = body->args[1];
   }

   t = TBInsertInstantiated(TermGetBank(t), body);

   SubstDelete(subst);
   return t;
}

/*-----------------------------------------------------------------------
//
// Function: do_named_snf()
//
//   Do the actual work in SNF
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Term_p do_named_snf(Term_p t)
{
   if(TermIsBetaReducible(t))
   {
      if(TermIsPhonyApp(t) && TermIsLambda(t->args[0]))
      {
         t = NamedLambdaSNF(reduce_head(t));
      }
      else
      {
         Term_p new_t = TermTopCopyWithoutArgs(t);
#ifndef NDEBUG
         bool diff=false;
#endif
         for(int i=0; i<t->arity; i++)
         {
            new_t->args[i] = NamedLambdaSNF(t->args[i]);
#ifndef NDEBUG
            diff = diff || (new_t->args[i] != t->args[i]);
#endif
         }
         assert(diff);
         t = TBTermTopInsert(TermGetBank(t), new_t);
      }
   }
   return t;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: NamedLambdaSNF()
//
//   Computes strong normal form for the lambda term in named notation
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p NamedLambdaSNF(Term_p t)
{
   t = do_named_snf(t);
   TB_p bank = TermGetBank(t);
   // undoing the encoding of literals under lambdas
   if(t->f_code == bank->sig->eqn_code &&
      t->args[1] == bank->true_term &&
      t->args[0] != bank->true_term &&
      SigIsLogicalSymbol(bank->sig, t->args[0]->f_code))
   {
      t = t->args[0];  
   }
   return t;}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
