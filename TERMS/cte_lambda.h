/*-----------------------------------------------------------------------

  File  : cte_lambda.h

  Author: Petar Vukmirovic

  Contents

  Functions that implement main operations of lambda calculus

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Mar 24 15:54:00 CET 2021

-----------------------------------------------------------------------*/

#ifndef CTE_LAMBDA

#define CTE_LAMBDA

#include <ccl_tformulae.h>
#include <ccl_pdtrees.h>
#include <cte_termbanks.h>
#include <cte_subst.h>



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

TFormula_p NamedLambdaSNF(TB_p terms, TFormula_p t);
TFormula_p LambdaToForall(TB_p terms, TFormula_p t);
TFormula_p LiftLambdas(TB_p terms, TFormula_p t, PStack_p definitions, 
                       PDTree_p liftings);


/*-----------------------------------------------------------------------
//
// Function: ApplyTerms()
//
//   Fills var_stack with abstracted variables and returns the body of lambda
//   For example, given ^[X]:(^[Y]:s), varstack = [Y, X] and s is returned
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p ApplyTerms(TB_p terms, Term_p head, PStack_p args)
{
   Term_p res = head;
   long   len = PStackGetSP(args);
   if(!PStackEmpty(args))
   {
      if(TermIsFreeVar(head) || TermIsLambda(head))
      {
         res = TermTopAlloc(SIG_PHONY_APP_CODE, len+1);
         res->args[0] = head;
         for(long i=1; i<=len; i++)
         {
            res->args[i] = PStackElementP(args, i-1);
         }
      }
      else
      {
         res = TermTopAlloc(head->f_code, head->arity + len);
         for(int i=0; i<head->arity; i++)
         {
            res->args[i] = head->args[i];
         }
         for(int i=0; i < len; i++)
         {
            res->args[head->arity + i] = PStackElementP(args, i);
         }
      }
   }
   assert(PStackEmpty(args) || res->type == NULL); // type will be inferred and checked
   return PStackEmpty(args) ? res : TBTermTopInsert(terms, res);
}


/*-----------------------------------------------------------------------
//
// Function: AbstractVars()
//
//   Abstract var_prefix over matrix. Variable at the top of the stack
//   is the first one to abstract. Does not change the stack.
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p AbstractVars(TB_p terms, Term_p matrix, PStack_p var_prefix)
{
   int num_vars = PStackGetSP(var_prefix);
   Term_p res = matrix;
   while(num_vars)
   {
      Term_p lambda = TermTopAlloc(SIG_NAMED_LAMBDA_CODE, 2);
      lambda->args[0] = PStackElementP(var_prefix, --num_vars);
      lambda->args[1] = res;
      res = TBTermTopInsert(terms, lambda);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: UnfoldLambda()
//
//   Fills var_stack with abstracted variables and returns the body of lambda
//   For example, given ^[X]:(^[Y]:s), varstack = [Y, X] and s is returned
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p UnfoldLambda(Term_p lambda, PStack_p var_stack)
{
   while(TermIsLambda(lambda))
   {
      PStackPushP(var_stack, lambda->args[0]);
      lambda = lambda->args[1];
   }
   return lambda;
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
