/*-----------------------------------------------------------------------

File  : cte_match_mgu_1-1.c

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

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

#define UNIF_FAIL(res) ((res) = NOT_UNIFIABLE); break
#define IS_RIGID(t) ((t)->f_code > 0 || !TermIsTopLevelFreeVar(t))
#define NUM_ACTUAL_ARGS(t) (TermIsAppliedFreeVar(t) ? (t)->arity-1 : 0)

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/
static inline Term_p get_fvar_head(Term_p t);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: whnf_deref()
//
//   Dereference and beta-normalize term only until the head of the
//   term becomes known.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p whnf_deref(TB_p bank, Term_p t)
{
   Term_p res;
   t = TermDerefAlways(t);

   if(TermIsPhonyApp(t) && TermIsLambda(t->args[0]))
   {
      res = whnf_deref(bank, WHNF_step(bank, t));
   }
   else if (TermIsLambda(t))
   {
      PStack_p dbvars = PStackAlloc();
      Term_p matrix = UnfoldLambda(t, dbvars);
      Term_p new_matrix = whnf_deref(bank, matrix);
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
      PStackFree(dbvars);
   }
   else
   {
      res = t;
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: fresh_var_with_args()
//
//   Make fresh variable applied to args with the appropriate return type.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p fresh_var_with_args(TB_p bank, PStack_p args, Type_p ret_ty)
{
   Type_p arg_tys[PStackGetSP(args)];
   for(long i=0; i < PStackGetSP(args); i++)
   {
      arg_tys[i] = ((Term_p) PStackElementP(args, i))->type;
   }
   Type_p var_ty = 
      TypeBankInsertTypeShared(bank->sig->type_bank,
         ArrowTypeFlattened(arg_tys, PStackGetSP(args), ret_ty));
   Term_p head = VarBankGetFreshVar(bank->vars, var_ty);
   return ApplyTerms(bank, head, args);   
}

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
                   RequestDBVar(bank->db_vars, arg->type, num_args-i));
   }


   return db_map;
}

/*-----------------------------------------------------------------------
//
// Function: normalize_free_var()
//
//   Tries to normalize applied variable so that all of its arguments
//   are 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p normalize_free_var(TB_p bank, Term_p s)
{
   if(TermIsFreeVar(s))
   {
      return s;
   }

   assert(TermIsAppliedFreeVar(s));
   
   s = LambdaEtaReduceDB(bank, s);
   bool all_dbs = true;
   for(long i=1; i<s->arity && all_dbs; i++)
   {
      all_dbs = TermIsDBVar(s->args[i]);
   }

   if(all_dbs && TermArrayNoDuplicates(s->args, s->arity))
   {  
      return s;
   }
   else
   {
      return NULL;
   }
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
   t = whnf_deref(bank, t);
   Term_p res;
   if(TermIsFreeVar(t))
   {
      if(t == s_var)
      {
         *unif_res = NOT_UNIFIABLE;
         res = NULL;
      }
      else
      {
         res = s_var;
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
            res = RequestDBVar(bank->db_vars, replacement->type,
                               replacement->f_code + depth);
         }
      }
   }
   else if (TermIsLambda(t))
   {
      PStack_p dbvars = PStackAlloc();
      
      Term_p matrix = UnfoldLambda(t, dbvars);
      Term_p new_matrix = 
         solve_flex_rigid(bank, s_var, db_map, matrix, subst, depth + PStackGetSP(dbvars), unif_res);
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
   else if (TermIsFreeVar(t))
   {
      t = normalize_free_var(bank, t);
      if(!t)
      {
         *unif_res = NOT_IN_FRAGMENT;
         res = NULL;
      }
      else if (get_fvar_head(t) == s_var)
      {
         *unif_res = NOT_UNIFIABLE;
         res = NULL;
      }
      else
      {
         long num_args = NUM_ACTUAL_ARGS(t);

         PStack_p t_dbs = PStackAlloc();
         PStack_p s_dbs = PStackAlloc();

         for(long i=1; t->arity; i++) // works if t is naked as well
         {
            assert(TermIsDBVar(t->args[i]));
            Term_p arg = t->args[i];
            if(arg->f_code < depth)
            {
               PStackPushP(t_dbs, 
                           RequestDBVar(bank->db_vars, arg->type, num_args-i));
               PStackPushP(s_dbs, 
                           RequestDBVar(bank->db_vars, arg->type, arg->f_code));
            }
            else
            {
               Term_p db_val = IntMapGetVal(db_map, arg->f_code - depth);
               if(db_val)
               {
                  PStackPushP(t_dbs, 
                        RequestDBVar(bank->db_vars, arg->type, num_args-i));
                  PStackPushP(s_dbs, 
                        RequestDBVar(bank->db_vars, db_val->type, db_val->f_code + depth));
               }
            }
         }

         Term_p t_var = get_fvar_head(t);
         Term_p t_binding_matrix =
            fresh_var_with_args(bank, t_dbs, t->type);
         Type_p t_prefix[NUM_ACTUAL_ARGS(t)];
         for(long i=1; i<t->arity; i++) // works with naked t as well
         {
            t_prefix[i-1] = t->args[i]->type;
         }
         SubstAddBinding(subst, t_var,
                         CloseWithTypePrefix(bank, t_prefix, NUM_ACTUAL_ARGS(t), t_binding_matrix));
         PStackFree(t_dbs);
         PStackFree(s_dbs);

         res = ApplyTerms(bank, get_fvar_head(t_binding_matrix), s_dbs);
      }
   }
   else
   {
      res = TermTopCopy(t);
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
   s = normalize_free_var(bank, s);
   Term_p s_var = get_fvar_head(s);
   OracleUnifResult res = UNIFIABLE;
   if(!s)
   {
      res = NOT_UNIFIABLE;
   }
   else
   {
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

   s = normalize_free_var(bank, s);
   t = normalize_free_var(bank, t);

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
      for(long i=1; t->arity; i++) // works if t is naked as well
      {
         assert(TermIsDBVar(t->args[i]));
         Term_p db_val = IntMapGetVal(db_map, t->args[i]->f_code);
         if(db_val)
         {
            PStackPushP(t_dbs, 
                        RequestDBVar(bank->db_vars, t->args[i]->type, num_args-i));
            PStackPushP(s_dbs, db_val);
         }
      }

      Term_p t_var = get_fvar_head(t);
      Term_p t_binding_matrix = 
         fresh_var_with_args(bank, t_dbs, t->type);
      Type_p t_prefix[NUM_ACTUAL_ARGS(t)];
      for(long i=1; i<t->arity; i++) // works with naked t as well
      {
         t_prefix[i-1] = t->args[i]->type;
      }
      SubstAddBinding(subst, t_var,
                      CloseWithTypePrefix(bank, t_prefix, NUM_ACTUAL_ARGS(t), t_binding_matrix));

      Term_p s_var = get_fvar_head(s);
      Term_p s_binding_matrix =
         ApplyTerms(bank, get_fvar_head(t_binding_matrix), s_dbs);
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
   }
   else
   {
      s = normalize_free_var(bank, s);
      t = normalize_free_var(bank, t);
      if(!s || !t)
      {
         res = NOT_IN_FRAGMENT;
      }
      else
      {
         Term_p var = get_fvar_head(s);
         assert(var == get_fvar_head(t));
         assert(TypeIsArrow(var->type));
         long max_args = TypeGetMaxArity(var->type);
         assert(s->arity == t->arity);
         PStack_p db_args = PStackAlloc();

         for(long i=0; i<s->arity; i++)
         {
            if(s->args[i] == t->args[i])
            {
               PStackPushP(db_args, 
                           RequestDBVar(bank->db_vars, s->args[i]->type, max_args-i-1));
            }
         }

         Term_p matrix = fresh_var_with_args(bank, db_args, GetRetType(var->type));
         SubstAddBinding(subst, var, 
                         CloseWithTypePrefix(bank, var->type->args, var->arity-1, matrix));
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
// Function: get_fvar_head()
//
//   If a term is (possibly applied) free variable, get the term
//   which represents this free variable.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p get_fvar_head(Term_p t)
{
   assert(TermIsTopLevelFreeVar(t));
   if(TermIsAppliedFreeVar(t))
   {
      return t->args[0];
   }
   else
   {
      return t;
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

Term_p eta_expand_otf(TB_p bank, Term_p t1, Term_p t2)
{
   assert(TermIsLambda(t1));
   assert(!TermIsLambda(t2));

   PStack_p dbvars = PStackAlloc();
   UNUSED(UnfoldLambda(t1, dbvars));

   long pref_len = PStackGetSP(dbvars);
   for(long i=0; i<pref_len; i++)
   {
      Type_p dbvar_ty = ((Term_p)PStackElementP(dbvars, i))->type; 
      PStackAssignP(dbvars, i, RequestDBVar(bank->db_vars, dbvar_ty, pref_len-i-1));
   }

   Term_p res = ApplyTerms(bank, t2, dbvars);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: prune_lambda_prefix()
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

bool prune_lambda_prefix(TB_p bank, Term_p *t1_ref, Term_p *t2_ref)
{
   Term_p t1 = *t1_ref;
   Term_p t2 = *t2_ref;
   bool pruned = false;
   
   while(TermIsLambda(t1) && TermIsLambda(t2))
   {
      t1 = t1->args[1];
      t2 = t2->args[1];
      pruned = true;
   }

   
   if(TermIsLambda(t1) || TermIsLambda(t2))
   {
      pruned = true;
      if(TermIsLambda(t1))
      {
         t2 = eta_expand_otf(bank, t1, t2);
      }
      else
      {
         assert(TermIsLambda(t2));
         t1 = eta_expand_otf(bank, t2, t1);
      }
   }

   *t1_ref = t1;
   *t2_ref = t2;

   return pruned;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

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
   if(t1->type != t2->type)
   {
      return NOT_UNIFIABLE;
   }

   PStackPointer backtrack = PStackGetSP(subst); /* For backtracking */

   OracleUnifResult res = UNIFIABLE;
   PQueue_p jobs = PQueueAlloc();

   PQueueStoreP(jobs, t1);
   PQueueStoreP(jobs, t2);
   TB_p bank = TermGetBank(t1);
   assert(bank);
   assert(bank == TermGetBank(t2));

   VarBankSetVCountsToUsed(bank->vars);
   while(!PQueueEmpty(jobs) && res == UNIFIABLE)
   {
      t2 = whnf_deref(bank, PQueueGetLastP(jobs));
      t1 = whnf_deref(bank, PQueueGetLastP(jobs));
      prune_lambda_prefix(bank, &t1, &t2);

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
            if(get_fvar_head(t1) == get_fvar_head(t2))
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
      else if (t1->f_code == t2->f_code)
      {
         assert(t1->arity == t2->arity);
         schedule_jobs(jobs, t1->args, t2->args, t1->arity);
      }
   }

   PQueueFree(jobs);
   if(res != UNIFIABLE)
   {
      SubstBacktrackToPos(subst, backtrack);
   }
   return res;
}
