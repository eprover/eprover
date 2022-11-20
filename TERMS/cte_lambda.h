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

typedef Term_p (*TermNormalizer)(TB_p bank, Term_p t);



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void       SetEtaNormalizer(TermNormalizer);
TermNormalizer GetEtaNormalizer();
TFormula_p NamedLambdaSNF(TB_p terms, TFormula_p t);
TFormula_p NamedToDB(TB_p bank, TFormula_p lambda);
Term_p     CloseWithDBVar(TB_p bank, Type_p ty, Term_p body);
Term_p     CloseWithTypePrefix(TB_p bank, Type_p* tys, long size, Term_p matrix);
Term_p     ShiftDB(TB_p bank, Term_p term, int shift_val);
Term_p     AbstractVars(TB_p terms, Term_p matrix, PStack_p var_prefix);
Term_p     WHNF_step(TB_p bank, Term_p t);
Term_p     WHNF_deref(Term_p t);
Term_p     BetaNormalizeDB(TB_p bank, Term_p term);
Term_p     LambdaEtaReduceDB(TB_p bank, Term_p term);
Term_p     LambdaEtaExpandDBTopLevel(TB_p bank, Term_p t);
Term_p     LambdaEtaExpandDB(TB_p bank, Term_p term);
Term_p     LambdaNormalizeDB(TB_p bank, Term_p term);
Term_p     PostCNFEncodeFormulas(TB_p bank, Term_p term);
Term_p     DecodeFormulasForCNF(TB_p bank, Term_p term);
Term_p     FlattenApps(TB_p bank, Term_p hd, Term_p* args, long num_args, Type_p res_type);

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
      if(TermIsAnyVar(head) || TermIsLambda(head))
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
// Function: UnfoldLambda()
//
//   Fills var_stack with abstracted variables and returns the body of
//   lambda. For example, given ^[X]:(^[Y]:s), varstack = [Y, X] and s
//   is returned
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
