/*-----------------------------------------------------------------------

File  : cte_pattern_match_mgu.c

Author: Stephan Schulz

Contents

  Implementation of simple, non-indexed 1-1 match and unification
  routines on shared terms (and unshared terms with shared
  variables).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 11 16:17:33 MET 1998
    New

-----------------------------------------------------------------------*/

#include "cte_pattern_match_mgu.h"
#include <cte_lambda.h>
#include <clb_plocalstacks.h>

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

#define UNIF_FAIL(res) ((res) = NOT_UNIFIABLE); break
#define IS_RIGID(t) ((t)->f_code > 0 || !TermIsTopLevelFreeVar(t))
#define NUM_ACTUAL_ARGS(t) (TermIsAppliedFreeVar(t) ? (t)->arity-1 : (t)->arity)

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: db_var_map()
//
//   For each DB var which is the argument of a free variable
//   create a corresponding DB var which denotes which argument of
//   the free variable DB var corresponds to. For example:
//     X 0 5 1 2 --> { 0 -> DB(3), 5 -> DB(2), 1 -> DB(1), 2 -> DB(0) } 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IntMap_p db_var_map(TB_p bank, Term_p s)
{
   assert(TermIsTopLevelFreeVar(s));
   IntMap_p db_map = IntMapAlloc();

   long num_args = NUM_ACTUAL_ARGS(s);
   for(long i=1; i<s->arity; i++) // works regardless s is X or X s1 ... sn
   {
      Term_p arg = s->args[i];
      assert(TermIsDBVar(arg));
      IntMapAssign(db_map, arg->f_code, 
                   TBRequestDBVar(bank, arg->type, num_args-i));
   }


   return db_map;
}

/*-----------------------------------------------------------------------
//
// Function: solve_flex_rigid()
//
//   Solve flex rigid
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
Term_p solve_flex_rigid(TB_p bank, Term_p s_var, IntMap_p db_map, Term_p t, 
                        Subst_p subst, long depth, OracleUnifResult* unif_res)
{
   t = WHNF_deref(t);
   Term_p res;
   if(TermIsFreeVar(t))
   {
      assert(!(t->binding));
      if(t == s_var)
      {
         *unif_res = NOT_UNIFIABLE;
         res = NULL;
      }
      else
      {
         res = t;
      }
   }
   else if (TermIsDBVar(t))
   {
      if(t->f_code < depth)
      {
         res = t;
      }
      else
      {
         Term_p replacement = IntMapGetVal(db_map, t->f_code - depth);
         if(!replacement)
         {
            *unif_res = NOT_UNIFIABLE;
            res = NULL;            
         }
         else
         {
            res = TBRequestDBVar(bank, replacement->type,
                                 replacement->f_code + depth);
         }
      }
   }
   else if (TermIsLambda(t))
   {
      PStack_p dbvars = PStackAlloc();
      
      Term_p matrix = UnfoldLambda(t, dbvars);
      Term_p new_matrix = 
         solve_flex_rigid(bank, s_var, db_map, matrix, 
                          subst, depth + PStackGetSP(dbvars), unif_res);
      if(*unif_res == UNIFIABLE)
      {
         if(matrix == new_matrix)
         {
            res = t;
         }
         else
         {
            res = new_matrix;
            while(!PStackEmpty(dbvars))
            {
               res = CloseWithDBVar(bank, ((Term_p)PStackPopP(dbvars))->type, res);
            }
         }
      }
      else
      {
         res = NULL;
      }

      PStackFree(dbvars);
   }
   else if (TermIsAppliedFreeVar(t))
   {
      t = NormalizePatternAppVar(bank, t);
      if(!t)
      {
         *unif_res = NOT_IN_FRAGMENT;
         res = NULL;
      }
      else if (GetFVarHead(t) == s_var)
      {
         *unif_res = NOT_UNIFIABLE;
         res = NULL;
      }
      else
      {
         long num_args = NUM_ACTUAL_ARGS(t);

         PStack_p t_dbs = PStackAlloc();
         PStack_p s_dbs = PStackAlloc();

         for(long i=1; i < t->arity; i++) // works if t is naked as well
         {
            assert(TermIsDBVar(t->args[i]));
            Term_p arg = t->args[i];
            if(arg->f_code < depth)
            {
               PStackPushP(t_dbs, 
                           TBRequestDBVar(bank, arg->type, num_args-i));
               PStackPushP(s_dbs, 
                           TBRequestDBVar(bank, arg->type, arg->f_code));
            }
            else
            {
               Term_p db_val = IntMapGetVal(db_map, arg->f_code - depth);
               if(db_val)
               {
                  PStackPushP(t_dbs, 
                        TBRequestDBVar(bank, arg->type, num_args-i));
                  PStackPushP(s_dbs, 
                        TBRequestDBVar(bank, db_val->type, db_val->f_code + depth));
               }
            }
         }

         Term_p t_var = GetFVarHead(t);
         Term_p t_binding_matrix =
            FreshVarWArgs(bank, t_dbs, t->type);
         Type_p t_prefix[NUM_ACTUAL_ARGS(t)];
         for(long i=1; i<t->arity; i++) // works with naked t as well
         {
            t_prefix[i-1] = t->args[i]->type;
         }
         SubstAddBinding(subst, t_var,
                         CloseWithTypePrefix(bank, t_prefix, NUM_ACTUAL_ARGS(t), t_binding_matrix));
         res = ApplyTerms(bank, GetFVarHead(t_binding_matrix), s_dbs);
         
         PStackFree(t_dbs);
         PStackFree(s_dbs);
      }
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(long i=0; i < res->arity && *unif_res == UNIFIABLE; i++)
      {
         res->args[i] = solve_flex_rigid(bank, s_var, db_map, t->args[i], 
                                         subst, depth, unif_res);
         changed = changed || t->args[i] != res->args[i];
      }

      if(*unif_res == UNIFIABLE)
      {
         if(!changed)
         {
            TermTopFree(res);
            res = t;
         }
         else
         {
            res = TBTermTopInsert(bank, res);
         }
      }
      else
      {
         TermTopFree(res);
         res = NULL;
      }
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: flex_rigid()
//
//   Solve pattern unification problem of the form X s = t, where t 
//   has a rigid head.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OracleUnifResult flex_rigid(TB_p bank, Term_p s, Term_p t, Subst_p subst)
{
   assert(TermIsTopLevelFreeVar(s));
   s = NormalizePatternAppVar(bank, s);
   OracleUnifResult res = UNIFIABLE;
   if(!s)
   {
      res = NOT_IN_FRAGMENT;
   }
   // all three functions are implemented very efficiently
   // optmization to make algorithm more like FO one
   else if(TermIsFreeVar(s) && TermIsPattern(t) && TermIsDBClosed(t))
   {
      if(OccurCheck(t, s))
      {
         res = NOT_UNIFIABLE;
      }
      else
      {
         SubstAddBinding(subst, s, t);
      }
   }
   else
   {
      Term_p s_var = GetFVarHead(s);
      IntMap_p db_map = db_var_map(bank, s);
      Term_p s_binding_matrix = solve_flex_rigid(bank, s_var, db_map, t, subst, 0, &res);
      if(res==UNIFIABLE)
      {
         assert(s_binding_matrix);
         Type_p s_prefix[NUM_ACTUAL_ARGS(s)];
         for(long i=1; i<s->arity; i++) // works for naked vars as well
         {
            s_prefix[i-1] = s->args[i]->type;
         }
         SubstAddBinding(subst, s_var,
                         CloseWithTypePrefix(bank, s_prefix, NUM_ACTUAL_ARGS(s), s_binding_matrix));
      }
      IntMapFree(db_map);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: flex_flex_diff()
//
//   Solve pattern unification problem of the form X s = X t.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OracleUnifResult flex_flex_diff(TB_p bank, Term_p s, Term_p t, Subst_p subst)
{
   OracleUnifResult res = UNIFIABLE;

   s = NormalizePatternAppVar(bank, s);
   t = NormalizePatternAppVar(bank, t);

   if(!s || !t)
   {
      res = NOT_IN_FRAGMENT;
   }
   else
   {
      IntMap_p db_map = db_var_map(bank, s);

      PStack_p t_dbs = PStackAlloc();
      PStack_p s_dbs = PStackAlloc();
      
      long num_args = NUM_ACTUAL_ARGS(t);
      for(long i=1; i < t->arity; i++) // works if t is naked as well
      {
         assert(TermIsDBVar(t->args[i]));
         Term_p db_val = IntMapGetVal(db_map, t->args[i]->f_code);
         if(db_val)
         {
            PStackPushP(t_dbs, 
                        TBRequestDBVar(bank, t->args[i]->type, num_args-i));
            PStackPushP(s_dbs, db_val);
         }
      }

      Term_p t_var = GetFVarHead(t);
      Term_p t_binding_matrix = 
         FreshVarWArgs(bank, t_dbs, t->type);
      Type_p t_prefix[NUM_ACTUAL_ARGS(t)];
      for(long i=1; i<t->arity; i++) // works with naked t as well
      {
         t_prefix[i-1] = t->args[i]->type;
      }
      SubstAddBinding(subst, t_var,
                      CloseWithTypePrefix(bank, t_prefix, NUM_ACTUAL_ARGS(t), t_binding_matrix));

      Term_p s_var = GetFVarHead(s);
      Term_p s_binding_matrix =
         ApplyTerms(bank, GetFVarHead(t_binding_matrix), s_dbs);
      Type_p s_prefix[NUM_ACTUAL_ARGS(s)];
      for(long i=1; i<s->arity; i++) // works with naked s as well
      {
         s_prefix[i-1] = s->args[i]->type;
      }
      SubstAddBinding(subst, s_var, 
                      CloseWithTypePrefix(bank, s_prefix, NUM_ACTUAL_ARGS(s), s_binding_matrix));

      IntMapFree(db_map);
      PStackFree(t_dbs);
      PStackFree(s_dbs);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: flex_flex_same()
//
//   Solve pattern unification problem of the form X s = X t.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OracleUnifResult flex_flex_same(TB_p bank, Term_p s, Term_p t, Subst_p subst)
{
   assert(TermIsTopLevelFreeVar(s));
   assert(TermIsTopLevelFreeVar(t));
   OracleUnifResult res = UNIFIABLE;

   if(TermIsFreeVar(s))
   {
      // decompose and continue, nothing to do
      assert(TermIsFreeVar(t)); // everything is eta-expanded
      assert(s == t);
   }
   else
   {
      assert(TermIsAppliedFreeVar(t));
      s = NormalizePatternAppVar(bank, s);
      t = NormalizePatternAppVar(bank, t);
      if(!s || !t)
      {
         res = NOT_IN_FRAGMENT;
      }
      else
      {
         Term_p var = GetFVarHead(s);
         assert(var == GetFVarHead(t));
         assert(TypeIsArrow(var->type));
         long max_args = TypeGetMaxArity(var->type);
         assert(s->arity == t->arity);
         PStack_p db_args = PStackAlloc();

         for(long i=1; i<s->arity; i++)
         {
            if(s->args[i] == t->args[i])
            {
               PStackPushP(db_args, 
                           TBRequestDBVar(bank, s->args[i]->type, max_args-i-1));
            }
         }

         Term_p matrix = FreshVarWArgs(bank, db_args, GetRetType(var->type));
         SubstAddBinding(subst, var, 
                         CloseWithTypePrefix(bank, var->type->args, max_args, matrix));
         PStackFree(db_args);
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: schedule_jobs()
//
//   Store the jobs represented by argument pairs to queue, prefering
//   the easier ones first.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline void schedule_jobs(PQueue_p q, Term_p* xs, Term_p* ys, long size)
{
   for(long i=0; i<size; i++)
   {
      if(IS_RIGID(xs[i]) && IS_RIGID(ys[i]))
      {
         PQueueStoreP(q, xs[i]);
         PQueueStoreP(q, ys[i]);
      }
      else
      {
         PQueueBuryP(q, xs[i]);
         PQueueBuryP(q, ys[i]);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: eta_expand_otf()
//
//   Assuming that the first arugment is a lambda and t2 is not,
//   and that the types of t1 and t2 are the same, eta-expand t2 so that
//   it has the same lambda prefix as t1 and then trim the lambda prefix
//   of t2.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void eta_expand_otf(TB_p bank, Term_p *lambda_ref, Term_p *non_lambda_ref)
{
   Term_p lambda = *lambda_ref, non_lambda = *non_lambda_ref;

   assert(TermIsLambda(lambda));
   assert(!TermIsLambda(non_lambda));

   PStack_p dbvars = PStackAlloc();
   *lambda_ref = UnfoldLambda(lambda, dbvars);

   long pref_len = PStackGetSP(dbvars);
   for(long i=0; i<pref_len; i++)
   {
      Type_p dbvar_ty = ((Term_p)PStackElementP(dbvars, i))->type; 
      PStackAssignP(dbvars, i, TBRequestDBVar(bank, dbvar_ty, pref_len-i-1));
   }

   *non_lambda_ref = ApplyTerms(bank, ShiftDB(bank, non_lambda, pref_len), dbvars);
   PStackFree(dbvars);
}

/*-----------------------------------------------------------------------
//
// Function: do_remap()
//
//   The actual driver that does the remapping.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p do_remap(TB_p bank, IntMap_p dbmap, Term_p t, OracleUnifResult* res, long depth)
{
   if(!TermHasDBSubterm(t))
   {
      return t;
   }

   if(TermIsAppliedFreeVar(t))
   {
      t = NormalizePatternAppVar(bank, t);
      if(!t)
      {
         *res = NOT_IN_FRAGMENT;
         return NULL;
      }
   }

   if(TermIsLambda(t))
   {
      PStack_p dbvars = PStackAlloc();

      Term_p matrix = UnfoldLambda(t, dbvars);
      Term_p new_matrix = do_remap(bank, dbmap, matrix, res, depth+PStackGetSP(dbvars));
      Term_p ret;
      if(matrix == new_matrix)
      {
         ret = t;
      }
      else if(new_matrix)
      {
         while(!PStackEmpty(dbvars))
         {
            new_matrix = CloseWithDBVar(bank, ((Term_p)PStackPopP(dbvars))->type,
                                        new_matrix);
         }
         ret = new_matrix;
      }
      else
      {
         assert(*res != UNIFIABLE);
         ret = NULL;
      }

      PStackFree(dbvars);
      return ret;
   }
   else if(TermIsDBVar(t))
   {
      if(t->f_code < depth)
      {
         return t;
      }
      else
      {
         Term_p replacement = IntMapGetVal(dbmap, t->f_code - depth);
         if(!replacement)
         {
            *res = NOT_UNIFIABLE;
            return NULL;            
         }
         else
         {
            return TBRequestDBVar(bank, replacement->type,
                                  replacement->f_code + depth);
         }
      }
   }
   else
   {
      Term_p copy = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(long i=0; i<copy->arity && *res == UNIFIABLE; i++)
      {
         copy->args[i] = do_remap(bank, dbmap, t->args[i], res, depth);
         changed = changed || copy->args[i] != t->args[i];
      }

      if(*res == UNIFIABLE)
      {
         if (changed)
         {
            return TBTermTopInsert(bank, copy);
         }
         else
         {
            TermTopFree(copy);
            return t;
         }
      }
      else
      {
         TermTopFree(copy);
         return NULL;
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: remap_variables()
//
//   Given a matcher applied variable remap bound variables in to_match
//   to match the ones that are arguments of the matcher. If this
//   is not possible return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p remap_variables(TB_p bank, Term_p matcher, Term_p to_match, 
                       OracleUnifResult* res)
{
   IntMap_p dbmap = db_var_map(bank, matcher);

   *res = UNIFIABLE;
   Term_p t = do_remap(bank, dbmap, to_match, res, 0);

   IntMapFree(dbmap);
   return t;
}

/*-----------------------------------------------------------------------
//
// Function: match_var()
//
//   Given an (applied) pattern variable matcher, compute the substitution
//   that binds it to to_match. If no such substitution exists,
//   or to_match is not a pattern, return the corresponding value. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OracleUnifResult match_var(TB_p bank, Subst_p subst, 
                           Term_p matcher, Term_p to_match)
{
   assert(TermIsTopLevelFreeVar(matcher));
   matcher = NormalizePatternAppVar(bank, matcher);
   if(!matcher)
   {
      return NOT_IN_FRAGMENT;
   }

   if(TermIsFreeVar(matcher))
   {
      if(TermIsDBClosed(to_match))
      {
         SubstAddBinding(subst, matcher, to_match);
         return UNIFIABLE;
      }
      else
      {
         return NOT_UNIFIABLE;
      }
   }
   else
   {
      Type_p tys[matcher->arity-1];
      for(long i=1; i<matcher->arity; i++)
      {
         tys[i-1] = matcher->args[i]->type;
      }
      OracleUnifResult res;
      Term_p binding = remap_variables(bank, matcher, to_match, &res);
      if(binding)
      {
         assert(res == UNIFIABLE);
         binding = CloseWithTypePrefix(bank, tys, matcher->arity-1, 
                                       remap_variables(bank, matcher, to_match, &res));
         assert(TermIsAppliedFreeVar(matcher));
         SubstAddBinding(subst, matcher->args[0], binding);
      }
      return res;
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PruneLambdaPrefix()
//
//   Make sure that terms are eta-expanded enough that they have the
//   lambda-prefix of the same size and then trim this prefix,
//   revealing only the bodies of the terms. References of those
//   trimmed bodies are assigned to arguments t1_ref and t2_ref.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool PruneLambdaPrefix(TB_p bank, Term_p *t1_ref, Term_p *t2_ref)
{
   Term_p t1 = *t1_ref;
   Term_p t2 = *t2_ref;
   bool pruned = false;
   
   while(TermIsLambda(t1) && TermIsLambda(t2))
   {
      assert(t1->args[0]->type == t2->args[0]->type);
      t1 = t1->args[1];
      t2 = t2->args[1];
      pruned = true;
   }

   
   if(TermIsLambda(t1) || TermIsLambda(t2))
   {
      pruned = true;
      if(TermIsLambda(t1))
      {
         eta_expand_otf(bank, &t1, &t2);
      }
      else
      {
         assert(TermIsLambda(t2));
         eta_expand_otf(bank, &t2, &t1);
      }
   }

   *t1_ref = t1;
   *t2_ref = t2;

   return pruned;
}

/*-----------------------------------------------------------------------
//
// Function: SubstComputeMguPattern()
//
//   Compute MGU of two terms which might not be patterns. If the terms
//   are not patterns, NOT_IN_FRAGMENT is returned. Otherwise, the
//   the answer to are terms unifiable is returned and subst is
//   extended in the obvious way.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OracleUnifResult SubstComputeMguPattern(Term_p t1, Term_p t2, Subst_p subst)
{
   Term_p orig_t1 = t1, orig_t2 = t2;
   UNUSED(orig_t1); UNUSED(orig_t2);
   if(t1->type != t2->type)
   {
      return NOT_UNIFIABLE;
   }

   PStackPointer backtrack = PStackGetSP(subst); /* For backtracking */

   OracleUnifResult res = UNIFIABLE;
   PQueue_p jobs = PQueueAlloc();

   PQueueStoreP(jobs, t1);
   PQueueStoreP(jobs, t2);
   TB_p bank = TermGetBank(t1) ? TermGetBank(t1) : TermGetBank(t2);
   assert(bank);

   VarBankSetVCountsToUsed(bank->vars);

   while(!PQueueEmpty(jobs) && res == UNIFIABLE)
   {
      t2 = WHNF_deref(PQueueGetLastP(jobs));
      t1 = WHNF_deref(PQueueGetLastP(jobs));
      PruneLambdaPrefix(bank, &t1, &t2);

      if(t1 == t2)
      {
         continue;
      }
      else if(TermIsGround(t1) && TermIsGround(t2))
      {
         assert(t1 != t2);
         UNIF_FAIL(res);
      }

      assert(t1->type == t2->type);

      if(TermIsTopLevelFreeVar(t2))
      {
         // making sure that if any of the terms is app var
         // t1 is for sure going to be an app var
         SWAP(t1, t2);
      }

      if(TermIsTopLevelFreeVar(t1))
      {
         if(TermIsTopLevelFreeVar(t2))
         {
            if(GetFVarHead(t1) == GetFVarHead(t2))
            {
               res = flex_flex_same(bank, t1, t2, subst);
            }
            else
            {
               res = flex_flex_diff(bank, t1, t2, subst);
            }
         }
         else
         {
            res = flex_rigid(bank, t1, t2, subst);
         }
      }
      else if (TermIsPhonyApp(t1))
      {
         // then the term must be an applied DB variable -- if 
         // the terms are unifiable then t2 must be the *same*
         // applied DB variable.
         if(TermIsPhonyApp(t2))
         {
            assert(TermIsDBVar(t1->args[0]));
            assert(TermIsDBVar(t2->args[0]));
            if(t1->args[0] == t2->args[0])
            {
               assert(t1->arity == t2->arity);
               schedule_jobs(jobs, t1->args+1, t2->args+1, t1->arity-1);
            }
            else
            {
               UNIF_FAIL(res);
            }
         }
         else
         {
            UNIF_FAIL(res);
         }
      }
      else if (TermIsDBVar(t1))
      {
         if(!TermIsDBVar(t2) || t1->f_code != t2->f_code)
         {
            UNIF_FAIL(res);
         }
      }
      else if (TermIsDBVar(t2))
      {
         assert(!TermIsPhonyApp(t1) && !TermIsDBVar(t1));
         UNIF_FAIL(res);
      }
      else if (t1->f_code == t2->f_code)
      {
         assert(t1->arity == t2->arity);
         if(SigIsPolymorphic(bank->sig, t1->f_code)
            && t1->arity != 0
            && t1->args[0]->type != t2->args[0]->type) 
         {
            // if poly symbol is constant the first type check and the
            // surrounding context guarantees type correctness
            UNIF_FAIL(res);
         }
         schedule_jobs(jobs, t1->args, t2->args, t1->arity);
      }
      else
      {
         UNIF_FAIL(res);
      }
   }

   PQueueFree(jobs);
   if(res != UNIFIABLE)
   {
      SubstBacktrackToPos(subst, backtrack);
   }
   else
   {
      assert(TermStructEqualDeref(orig_t1, orig_t2, DEREF_ALWAYS, DEREF_ALWAYS));
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SubstComputeMatchPattern()
//
//   Computes the matcher of two pattern terms.
//   NB: In HO logic, we cannot use the weight trick as substitution
//       can possibly remove some of variable arguments. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OracleUnifResult SubstComputeMatchPattern(Term_p matcher, Term_p to_match, Subst_p subst)
{
   assert(problemType == PROBLEM_HO);
   if(matcher->type != to_match->type ||
      !TermIsPattern(matcher) || !TermIsPattern(to_match))
   {
      return NOT_UNIFIABLE;
   }

   TB_p bank = TermGetBank(matcher) ? TermGetBank(matcher) : TermGetBank(to_match);
   assert(bank || TermIsAnyVar(matcher) || TermIsAnyVar(to_match));
   assert(!(TermGetBank(matcher) && TermGetBank(to_match)) 
            || TermGetBank(matcher) == TermGetBank(to_match));
     
   PStackPointer backtrack = PStackGetSP(subst); /* For backtracking */
   PLocalStackInit(jobs);

   // to make the weight stuff work!!!
   matcher = LambdaEtaExpandDB(bank, matcher);
   to_match = LambdaEtaExpandDB(bank, to_match);

   PLocalStackPush(jobs, matcher);
   PLocalStackPush(jobs, to_match);

   OracleUnifResult res = UNIFIABLE;
   
   long matcher_weight  = TermStandardWeight(matcher);
   long to_match_weight = TermStandardWeight(to_match);

   while(!PLocalStackEmpty(jobs) && res == UNIFIABLE)
   {
      to_match =  PLocalStackPop(jobs);
      matcher  =  PLocalStackPop(jobs);
      PruneLambdaPrefix(bank, &matcher, &to_match);
      if(TermIsGround(to_match) && TermIsGround(matcher))
      {
         if(LambdaNormalizeDB(bank, to_match) != LambdaNormalizeDB(bank, matcher))
         {
            UNIF_FAIL(res);
         }
      }
      if(matcher_weight > to_match_weight)
      {
         UNIF_FAIL(res);
      }

      if(TermIsTopLevelFreeVar(matcher))
      {
         matcher = NormalizePatternAppVar(bank, matcher);
         if(!matcher)
         {
            res = NOT_IN_FRAGMENT;
            break;
         }

         Term_p fvar = GetFVarHead(matcher);
         if(fvar->binding)
         {
            DerefType dummy = DEREF_ONCE;
            matcher = LambdaNormalizeDB(bank, TermDeref(matcher, &dummy));
            if(matcher != LambdaNormalizeDB(bank, to_match))
            {
               UNIF_FAIL(res);
            }
         }
         else
         {
            res = match_var(bank, subst, matcher, to_match);
         }
         matcher_weight += TermStandardWeight(to_match) - DEFAULT_VWEIGHT;

         if(matcher_weight > to_match_weight)
         {
            UNIF_FAIL(res);
         }
      }
      else if(TermIsTopLevelDBVar(matcher))
      {
         if(TermIsDBVar(matcher))
         {
            if(!TermIsDBVar(to_match) || matcher->f_code != to_match->f_code)
            {
               UNIF_FAIL(res);
            }
         }
         else
         {
            if(TermIsAppliedDBVar(to_match) && matcher->args[0] == to_match->args[0])
            {
               PLocalStackEnsureSpace(jobs, 2*(matcher->arity-1));
               for(int i=matcher->arity-1; i>=1; i--)
               {
                  PLocalStackPush(jobs, matcher->args[i]);
                  PLocalStackPush(jobs, to_match->args[i]);
               }
            }
            else
            {
               UNIF_FAIL(res);
            }
         }
         
      }
      else if(matcher->f_code == to_match->f_code)
      {
         if(SigIsPolymorphic(bank->sig, matcher->f_code)
            && matcher->arity != 0
            && matcher->args[0]->type != to_match->args[0]->type) 
         {
            UNIF_FAIL(res);
         }
         assert(matcher->arity == to_match->arity);
         PLocalStackEnsureSpace(jobs, 2*matcher->arity);
         for(int i=matcher->arity-1; i>=0; i--)
         {
            PLocalStackPush(jobs, matcher->args[i]);
            PLocalStackPush(jobs, to_match->args[i]);
         }
      }
      else
      {
         UNIF_FAIL(res);
      }
   }

   if(res != UNIFIABLE)
   {
      SubstBacktrackToPos(subst,backtrack);
   }
   PLocalStackFree(jobs);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FreshVarWArgs()
//
//   Make fresh variable applied to args with the appropriate return type.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p FreshVarWArgs(TB_p bank, PStack_p args, Type_p ret_ty)
{
   Type_p arg_tys[PStackGetSP(args)];
   for(long i=0; i < PStackGetSP(args); i++)
   {
      arg_tys[i] = ((Term_p) PStackElementP(args, i))->type;
   }
   Type_p var_ty = 
      TypeBankInsertTypeShared(bank->sig->type_bank,
         ArrowTypeFlattened(arg_tys, PStackGetSP(args), ret_ty));
   Term_p head = TBInsert(bank, VarBankGetFreshVar(bank->vars, var_ty), DEREF_NEVER);
   return PStackEmpty(args) ? TBInsert(bank, head, DEREF_NEVER) :
                              ApplyTerms(bank, head, args);
}
