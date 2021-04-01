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
#include <ccl_formula_wrapper.h>
#include <ccl_inferencedoc.h>
#include <ccl_derivation.h>

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
      res = ApplyTerms(TermGetBank(res), res, args);
      PStackFree(args);
   }

   SubstDelete(subst);
   return res;
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
      if((TermIsLambda(t->args[0]) || TermIsLambda(t->args[1])))
      {
         PStack_p lhs_vars = PStackAlloc();
         PStack_p rhs_vars = PStackAlloc();
         UNUSED(UnfoldLambda(t->args[0], lhs_vars));
         UNUSED(UnfoldLambda(t->args[1], rhs_vars));
         assert(!PStackEmpty(lhs_vars) || !PStackEmpty(rhs_vars));

         PStack_p more_vars = PStackGetSP(lhs_vars) > PStackGetSP(rhs_vars) ?
                              lhs_vars : rhs_vars;
         Term_p lhs = NamedLambdaSNF(terms, ApplyTerms(terms, t->args[0], more_vars));
         Term_p rhs = NamedLambdaSNF(terms, ApplyTerms(terms, t->args[1], more_vars));
         if(lhs->type == sig->type_bank->bool_type)
         {
            
            t = TFormulaFCodeAlloc(terms,
                                   t->f_code == sig->eqn_code 
                                        ? sig->equiv_code : sig->xor_code,
                                   EncodePredicateAsEqn(terms, lhs), 
                                   EncodePredicateAsEqn(terms, rhs));
         }
         else
         {
            t = TFormulaFCodeAlloc(terms, t->f_code, lhs, rhs);
         }

         bool universal = t->f_code == sig->eqn_code 
                           || t->f_code == sig->equiv_code;
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


/*-----------------------------------------------------------------------
//
// Function: test_leaks()
//
//   Check if substitution applied to the equation leaked some of the bound
//   variables and if the bound variables are only renamed by substitution.
//   Assumes the argument is the definition of the form 
//   \xyz. body = def X' Y' Z' where x,y,z are bound and X' Y' Z' are free
//   for body.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool test_leaks(Term_p target, Eqn_p def)
{
   Term_p head   = def->rterm;
   Term_p lambda = def->lterm;
   bool   ans    = true;

   PStack_p bound = PStackAlloc();
   UNUSED(UnfoldLambda(lambda, bound));

   //assert bound variables are only renamed in the matcher
   for(PStackPointer i=0; ans && i<PStackGetSP(bound); i++)
   {
      Term_p binding = ((Term_p)PStackElementP(bound, i))->binding;
      if(binding && (!TermIsVar(binding) || TermCellQueryProp(binding, TPIsSpecialVar)))
      {
         ans = false;
      }
      else if(binding)
      {
         TermCellSetProp(binding, TPIsSpecialVar);
      }
   }

   // if everything is OK, delete properties and empty the stack
   while(!PStackEmpty(bound))
   {
      Term_p binding = ((Term_p) PStackPopP(bound))->binding;
      if(binding)
      {
         TermCellDelProp(binding, TPIsSpecialVar);
      }
   }

   // check if the bound variables of the target appear in
   // substition(free variables) -- i.e., if they leaked
   UNUSED(UnfoldLambda(target, bound));
   
   for(int i=0; ans && i<head->arity; i++)
   {
      Term_p binding = head->args[i]->binding;
      for(PStackPointer j=0; binding && ans && j < PStackGetSP(bound); j++)
      {
         ans = ans && 
               !TermIsSubterm(binding, PStackElementP(bound, j), DEREF_NEVER);
      }
   }

   PStackFree(bound);
   return ans;
}


/*-----------------------------------------------------------------------
//
// Function: find_generalization()
//
//   Check if there is already a name for lambda term query. If so,
//   return the defining formula and store the name in *name.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFormula_p find_generalization(PDTree_p liftings, Term_p query, TermRef name)
{
   MatchRes_p mi;
   Subst_p    subst = SubstAlloc();
   WFormula_p res = NULL;

   PDTreeSearchInit(liftings, query, PDTREE_IGNORE_NF_DATE, false);
   while(!res && (mi = PDTreeFindNextDemodulator(liftings, subst)))
   {
      if(mi->remaining_args == 0 && test_leaks(query, mi->pos->literal))
      {
         *name = TBInsertInstantiated(liftings->bank, mi->pos->literal->rterm);
         res = mi->pos->data;
      }
      MatchResFree(mi);
   }
   PDTreeSearchExit(liftings);
   SubstDelete(subst);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: store_lifting()
//
//   Check if there is already a name for lambda term query. If so,
//   return the defining formula and store the name in *name.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void store_lifting(PDTree_p liftings, Term_p def_head, Term_p body, WFormula_p def)
{
   Eqn_p eqn = EqnAlloc(body, def_head, liftings->bank, true);
   ClausePos_p pos = ClausePosCellAlloc();
   pos->literal = eqn;
   pos->data = def;
   pos->side = LeftSide;
   pos->pos = NULL;
   pos->clause = NULL;
   PDTreeInsert(liftings, pos);
}


/*-----------------------------------------------------------------------
//
// Function: lift_lambda()
//
//   Convert lambda term: ^[...bound vars...]:s[...free vars...]
//   into a definiton f ..free vars.. ..bound vars.. = s
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p lift_lambda(TB_p terms, PStack_p bound_vars, Term_p body, 
                   PStack_p definitions, PDTree_p liftings)
{
   Term_p res;
   WFormula_p generalization = 
      find_generalization(liftings,
                          AbstractVars(terms, body, bound_vars),
                          &res);
   if(generalization)
   {
      PStackPushP(definitions, generalization);
   }
   else
   {
      PTree_p free_vars_tree = NULL;
      PStack_p free_vars = PStackAlloc();
      PStack_p all_vars = PStackAlloc();
      TFormulaCollectFreeVars(terms, body, &free_vars_tree);

      for(int i=0; i<PStackGetSP(bound_vars); i++)
      {
         PTreeDeleteEntry(&free_vars_tree, PStackElementP(bound_vars, i));
      }

      PTreeToPStack(free_vars, free_vars_tree);
      PTreeFree(free_vars_tree);
      for(int i=0; i<PStackGetSP(free_vars); i++)
      {
         PStackPushP(all_vars, PStackElementP(free_vars, i));
      }
      for(int i=0; i<PStackGetSP(bound_vars); i++)
      {
         PStackPushP(all_vars, PStackElementP(bound_vars, i));
      }

      Term_p def_head =  TermAllocNewSkolem(terms->sig, all_vars, body->type);
      def_head = TBTermTopInsert(terms, def_head);
      res = TermTopAlloc(def_head->f_code, PStackGetSP(free_vars));
      for(int i=0; i < PStackGetSP(free_vars); i++)
      {
         res->args[i] = PStackElementP(free_vars, i);
      }
      res = TBTermTopInsert(terms, res);
      assert(def_head->type == body->type);

      TFormula_p def_f;
      if(body->type == terms->sig->type_bank->bool_type)
      {
         def_f = TFormulaFCodeAlloc(terms, terms->sig->equiv_code, 
                                 EncodePredicateAsEqn(terms, def_head),
                                 EncodePredicateAsEqn(terms, body));
      }
      else
      {
         def_f = TFormulaFCodeAlloc(terms, terms->sig->eqn_code, def_head, body);
      }
      
      for(PStackPointer i=0; i<PStackGetSP(all_vars); i++)
      {
         def_f = TFormulaAddQuantor(terms, def_f, true,
                                    PStackElementP(all_vars, i));
      }

      WFormula_p def = WTFormulaAlloc(terms, def_f);
      DocFormulaCreationDefault(def, inf_fof_intro_def, NULL, NULL);
      WFormulaPushDerivation(def, DCIntroDef, NULL, NULL);
      store_lifting(liftings, res, AbstractVars(terms, body, bound_vars), def);

      PStackPushP(definitions, def);

      PStackFree(free_vars);
      PStackFree(all_vars);
   }
   return res;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: NamedLambdaSNF()
//
//   Computes strong normal form for the lambda term in named notation.
//   Not using TermMap for efficiency.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p NamedLambdaSNF(TB_p bank, TFormula_p t)
{
   // Term_p orig = t;
   VarBankSetVCountsToUsed(bank->vars);
   t = do_named_snf(t);
   // undoing the encoding of literals under lambdas
   if(t->f_code == bank->sig->eqn_code &&
      t->args[1] == bank->true_term &&
      t->args[0] != bank->true_term &&
      SigIsLogicalSymbol(bank->sig, t->args[0]->f_code))
   {
      t = t->args[0];  
   }
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


/*-----------------------------------------------------------------------
//
// Function: LiftLambdas()
//
//   Turns equation (^[X]:s)=t into ![X]:(s = (t @ X))
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TFormula_p LiftLambdas(TB_p terms, TFormula_p t, PStack_p definitions, PDTree_p liftings)
{
   Term_p res;
   PStack_p vars = NULL;
   bool changed = false;
   if(TermIsLambda(t))
   {
      vars = PStackAlloc();
      t = UnfoldLambda(t, vars);
   }

   res = TermTopCopyWithoutArgs(t);
   for(int i=0; i<t->arity; i++)
   {
      res->args[i] = LiftLambdas(terms, t->args[i], definitions, liftings);
      changed = changed || (res->args[i] != t->args[i]);
   }

   if(changed)
   {
      res = TBTermTopInsert(terms, res);
   }
   else
   {
      TermTopFree(res);
      res = t;
   }

   
   if(vars)
   {
      res = lift_lambda(terms, vars, res, definitions, liftings);
      PStackFree(vars);
   }

   return res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
