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
// Function: apply_terms()
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

static inline Term_p apply_terms(TB_p terms, Term_p head, PStack_p args)
{
   Term_p res;
   long   len = PStackGetSP(args);
   if(TermIsVar(head) || TermIsLambda(head))
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
   assert(res->type == NULL); // type will be inferred and checked
   return TBTermTopInsert(terms, res);
}


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
   Term_p res;
   Subst_p subst = SubstAlloc();
   int     i = 1;

   while(TermIsLambda(body) && i < t->arity)
   {
      // cancelling old bindings -- for name clashes (e.g. ^[X,X]:...)
      body->args[0]->binding = NULL; 
      SubstAddBinding(subst, body->args[0], t->args[i++]);
      body = body->args[1];
   }

   res = TBInsertInstantiated(TermGetBank(t), body);

   if(i != t->arity)
   {
      PStack_p args = PStackAlloc();
      for(; i<t->arity; i++)
      {
         PStackPushP(args, t->args[i]);
      }
      res = apply_terms(TermGetBank(res), res, args);
      PStackFree(args);
   }

   SubstDelete(subst);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: unfold_lambda()
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

static inline Term_p unfold_lambda(Term_p lambda, PStack_p var_stack)
{
   while(TermIsLambda(lambda))
   {
      PStackPushP(var_stack, lambda->args[0]);
      lambda = lambda->args[1];
   }
   return lambda;
}

/*-----------------------------------------------------------------------
//
// Function: lambda_eq_to_forall()
//
//   If the term is an equation between terms where at least one is a lambda,
//   then turn it into equation of non-lambdas
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Term_p lambda_eq_to_forall(TB_p terms, Term_p t)
{
   Sig_p sig = terms->sig;
   if((t->f_code == sig->eqn_code
       || t->f_code == sig->neqn_code) 
      && t->arity == 2)
   {
      if(TermIsLambda(t->args[0]) || TermIsLambda(t->args[1]))
      {
         PStack_p lhs_vars = PStackAlloc();
         PStack_p rhs_vars = PStackAlloc();
         UNUSED(unfold_lambda(t->args[0], lhs_vars));
         UNUSED(unfold_lambda(t->args[1], rhs_vars));
         assert(!PStackEmpty(lhs_vars) || !PStackEmpty(rhs_vars));

         PStack_p more_vars = PStackGetSP(lhs_vars) > PStackGetSP(rhs_vars) ?
                              lhs_vars : rhs_vars;
         Term_p lhs = NamedLambdaSNF(apply_terms(terms, t->args[0], more_vars));
         Term_p rhs = NamedLambdaSNF(apply_terms(terms, t->args[1], more_vars));
         if(lhs->type == sig->type_bank->bool_type)
         {
            t = TFormulaFCodeAlloc(terms,
                                   t->f_code == sig->eqn_code 
                                        ? sig->equiv_code : sig->xor_code,
                                   lhs, rhs);
         }
         else
         {
            t = TFormulaFCodeAlloc(terms, t->f_code, lhs, rhs);
         }

         bool universal = t->f_code == sig->eqn_code;
         while(!PStackEmpty(more_vars))
         {
            t = TFormulaAddQuantor(terms, t, universal, PStackPopP(more_vars));
         }

         PStackFree(lhs_vars);
         PStackFree(rhs_vars);
      }
   }
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
         t = do_named_snf(reduce_head(t));
      }
      else
      {
         Term_p new_t = TermTopCopyWithoutArgs(t);
#ifndef NDEBUG
         bool diff=false;
#endif
         for(int i=0; i<t->arity; i++)
         {
            new_t->args[i] = do_named_snf(t->args[i]);
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

TFormula_p NamedLambdaSNF(TFormula_p t)
{
   // Term_p orig = t;
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
   // fprintf(stderr, "snf(");
   // TermPrintDbgHO(stderr, orig, TermGetBank(t)->sig, DEREF_NEVER);
   // fprintf(stderr, ") = ");
   // TermPrintDbgHO(stderr, t, TermGetBank(t)->sig, DEREF_NEVER);
   // fprintf(stderr, "\n");
   return t;
}

/*-----------------------------------------------------------------------
//
// Function: Lambda2Forall()
//
//   Turns equation (^[X]:s)=t into ![X]:(s = (t @ X))
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p LambdaToForall(TB_p terms, TFormula_p t)
{
   return TermMap(terms, t, lambda_eq_to_forall);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
