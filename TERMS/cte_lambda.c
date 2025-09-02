/*-----------------------------------------------------------------------

  File  : cte_lambda.c

  Author: Petar Vukmirovic

  Contents

  Functions that implement main operations of lambda calculus

  Copyright 1998-2021 by the author.
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

#define DB_NOT_FOUND (-1)

static TermNormalizer eta_norm = LambdaEtaReduceDB;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

Term_p do_beta_normalize_db(TB_p bank, Term_p t);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: drop_args()
//
//   Does eta-normalization in an optimized way: it does not do one
//   lambda binder at the time (e.g. %x. (%y. g x y)  -> %x. g x -> g ), but
//   it does it in one go %xy. g x y -> g.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p drop_args(TB_p bank, Term_p t, long args_to_drop)
{
   Term_p res = NULL;
   assert(args_to_drop >= 0);
   assert(args_to_drop <= t->arity);
   assert(!TermIsPhonyApp(t) || args_to_drop <= t->arity -1);
   if(args_to_drop == 0)
   {
      res = t;
   }
   else if(TermIsPhonyApp(t) && t->arity == args_to_drop + 1)
   {
      res = t->args[0];
   }
   else
   {
      Type_p ty_args[args_to_drop]; // VLA
      for(long i=t->arity - args_to_drop; i < t->arity; i++)
      {
         ty_args[i-t->arity + args_to_drop] = t->args[i]->type;
      }

      res = TermTopAlloc(t->f_code, t->arity - args_to_drop);
      res->type = TypeBankInsertTypeShared(bank->sig->type_bank,
                                           ArrowTypeFlattened(ty_args,
                                                              args_to_drop,
                                                              t->type));
      for(long i=0; i < t->arity - args_to_drop; i++)
      {
         res->args[i] = t->args[i];
      }
      res = TBTermTopInsert(bank, res);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FlattenApps()
//
//   Apply additional arguments to hd assuming hd needs to be flattened.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p FlattenApps(TB_p bank, Term_p hd, Term_p* args, long num_args,
                    Type_p res_type)
{
   Term_p res = TermTopAlloc(hd->f_code, hd->arity + num_args);
#ifdef NDEBUG
   res->type = res_type;
#endif
   for(long i=0; i < hd->arity; i++)
   {
      res->args[i] = hd->args[i];
   }
   for(long i=0; i < num_args; i++)
   {
      res->args[hd->arity+i] = args[i];
   }

   res = TBTermTopInsert(bank, res);
   assert(res->type == res_type);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: flatten_and_make_shared()
//
//   Beta normalization and eta-reduction can result in a term that
//   has PhonyApp symbol at head and a regular symbol as its first argument.
//   This function performs the necessary flattening on the intermediary
//   term created during either normalization procedure and makes sure
//   that it is shared.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p flatten_and_make_shared(TB_p bank, Term_p t)
{
   assert(!TermIsShared(t));
   if(TermIsPhonyApp(t) &&
      !(TermIsAnyVar(t->args[0]) || TermIsLambda(t->args[0])))
   {
      Term_p junk = t;
      t = FlattenApps(bank, t->args[0], t->args+1, t->arity-1, t->type);
      TermTopFree(junk);
   }
   else
   {
      t = TBTermTopInsert(bank, t);
   }
   return t;
}

/*-----------------------------------------------------------------------
//
// Function: do_shift_db()
//
//   Performs the actual shifting.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p do_shift_db(TB_p bank, Term_p t, int shift_val, int depth)
{
   assert(shift_val != 0);
   Term_p res = NULL;
   if(TermIsDBVar(t))
   {
      if(t->f_code >= depth)
      {
         res = TBRequestDBVar(bank, t->type, t->f_code + shift_val);
      }
      else
      {
         res = t;
      }
   }
   else if (TermIsLambda(t))
   {
      assert(t->f_code == SIG_DB_LAMBDA_CODE);
      assert(t->arity == 2);

      Term_p matrix = t->args[1];
      Term_p shifted = do_shift_db(bank, matrix, shift_val, depth+1);
      if(matrix == shifted)
      {
         res = t;
      }
      else
      {
         res = CloseWithDBVar(bank, t->args[0]->type, shifted);
      }
   }
   else if (t->arity == 0 || !TermHasDBSubterm(t))
   {
      res = t; // optimization
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      bool changed = false;

      for(long i=0; i<res->arity; i++)
      {
         res->args[i] = do_shift_db(bank, t->args[i], shift_val, depth);
         changed = changed || res->args[i] != t->args[i];
      }

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
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: replace_bound_vars()
//
//   1.  For DB vars that are bound, do nothing.
//   2a. For DB vars that are loosely bound
//       with index 0 <= idx < total_bound, replace them with the
//       corresponding term (shifted for depth)
//   2b. For other losely bound variables with index idx, shift them for
//       total_bound - idx
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p replace_bound_vars(TB_p bank, Term_p t, int total_bound, int depth)
{
   assert(total_bound > 0);
   Term_p res = NULL;
   if(TermIsDBVar(t))
   {
      if(t->f_code < depth)
      {
         res = t;
      }
      else // it is losely bound variable;
      {
         if(t->f_code - depth < total_bound)
         {
            Term_p corresponding_db =
               TBRequestDBVar(bank, t->type, t->f_code - depth);
            assert(corresponding_db->binding);
            res = ShiftDB(bank, corresponding_db->binding, depth);
         }
         else
         {
            res = TBRequestDBVar(bank, t->type, t->f_code - total_bound);
         }
      }
   }
   else if(TermIsLambda(t))
   {
      assert(t->f_code == SIG_DB_LAMBDA_CODE);
      assert(TermIsDBVar(t->args[0]));

      Term_p matrix = t->args[1];
      Term_p new_matrix = replace_bound_vars(bank, matrix, total_bound, depth+1);
      if(matrix == new_matrix)
      {
         res = t;
      }
      else
      {
         res = CloseWithDBVar(bank, t->args[0]->type, new_matrix);
      }
   }
   else if (t->arity == 0 || !TermHasDBSubterm(t))
   {
      res = t; //optimization
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      bool changed = false;

      for(long i=0; i < res->arity; i++)
      {
         res->args[i] = replace_bound_vars(bank, t->args[i], total_bound, depth);
         changed = changed || res->args[i] != t->args[i];
      }

      res = changed ? flatten_and_make_shared(bank, res) :
                      (TermTopFree(res), t);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: find_min_db()
//
//   Find the loosely bound DB variable with the minimal index.
//   Return DB_NOT_FOUND if no such variable exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long find_min_db(Term_p t, long depth)
{
   long res = DB_NOT_FOUND;
   if(TermIsDBVar(t))
   {
      if(t->f_code >= depth)
      {
         res = t->f_code - depth;
      }
   }
   else if (TermIsLambda(t))
   {
      res = find_min_db(t->args[1], depth+1);
   }
   else if (TermHasDBSubterm(t))
   {
      for(long i=0; i<t->arity; i++)
      {
         long min_db = find_min_db(t->args[i], depth);
         if(min_db != DB_NOT_FOUND)
         {
            if(res == DB_NOT_FOUND)
            {
               res = min_db;
            }
            else
            {
               res = MIN(res, min_db);
            }
         }
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: reduce_eta_top_level()
//
//   Does one step of argument removal necessary for eta-reduction.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p reduce_eta_top_level(TB_p bank, Term_p t)
{
   PStack_p bound_vars = PStackAlloc();
   Term_p matrix = UnfoldLambda(t, bound_vars);
   Term_p res = t;

   if(TermIsLambda(t) &&
      matrix->arity > 0 &&
      TermIsDBVar(matrix->args[matrix->arity-1]) &&
      matrix->args[matrix->arity-1]->f_code == 0)
   {  // term is of the shape %x... . ... @ x
      long last_db = matrix->arity - 1;
      long left_limit =
         MAX(matrix->arity - PStackGetSP(bound_vars),
             TermIsPhonyApp(matrix) ? 1 : 0);
      for(; last_db >= left_limit; last_db--)
      {
         long expected_db = matrix->arity-1 - last_db;
         if(! (TermIsDBVar(matrix->args[last_db]) &&
               matrix->args[last_db]->f_code == expected_db))
         {
            break;
         }
      }
      last_db++; // last loop execution failed

      assert(last_db >=0);
      assert(last_db < matrix->arity);

      long min_db = DB_NOT_FOUND;
      for(long i=0; i<last_db; i++)
      {
         long min_db_i = find_min_db(matrix->args[i], 0);
         if(min_db_i != DB_NOT_FOUND)
         {
            if(min_db == DB_NOT_FOUND)
            {
               min_db = min_db_i;
            }
            else
            {
               min_db = MIN(min_db, min_db_i);
            }
         }
      }

      if(min_db != 0)
      {
         long to_drop = min_db == DB_NOT_FOUND ? matrix->args[last_db]->f_code + 1
                                               : MIN(min_db, matrix->args[last_db]->f_code + 1);

         res = ShiftDB(bank, drop_args(bank, matrix, to_drop), -to_drop);

         while(to_drop) // dropping leftmost binders
         {
            UNUSED(PStackPopP(bound_vars));
            to_drop--;
         }

         while(!PStackEmpty(bound_vars))
         {
            res = CloseWithDBVar(bank, ((Term_p)PStackPopP(bound_vars))->type, res);
         }
      }
   }
   PStackFree(bound_vars);
   assert(res);
   assert(res->type == t->type);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: do_eta_expand_db()
//
//   Does eta-expansion on the lambda terms in the De Bruijn notation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p do_eta_expand_db(TB_p bank, Term_p t)
{
   Term_p res;
   if(TermIsLambda(t))
   {
      if(TermHasEtaExpandableSubterm(t->args[1]))
      {
         Term_p new_matrix = do_eta_expand_db(bank, t->args[1]);
         assert(new_matrix != t->args[1]);
         res = TermTopCopy(t);
         res->args[1] = new_matrix;
         res = TBTermTopInsert(bank, res);
      }
      else
      {
         res = t;
      }
   }
   else if(t->arity == 0 || !TermHasEtaExpandableSubterm(t))
   {
      res = t; // optimization
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      if(TermIsPhonyApp(t))
      {
         res->args[0] = t->args[0];
      }
      bool changed = false;
      for(long i=TermIsPhonyApp(t)?1:0; i < res->arity; i++)
      {
         res->args[i] = do_eta_expand_db(bank, t->args[i]);
         changed = changed || res->args[i] != t->args[i];
      }

      if(changed)
      {
         res = TBTermTopInsert(bank, res);
      }
      else
      {
         TermTopFree(res);
         res = t;
      }
   }
   return LambdaEtaExpandDBTopLevel(bank, res);
}

/*-----------------------------------------------------------------------
//
// Function: do_eta_reduce_db()
//
//   Does eta-normalization in an optimized way: it does not do one
//   lambda binder at the time (e.g. %x. (%y. g x y)  -> %x. g x -> g ), but
//   it does it in one go %xy. g x y -> g.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p do_eta_reduce_db(TB_p bank, Term_p t)
{
   Term_p res;
   assert(bank);
   if(t->arity == 0 || !TermHasLambdaSubterm(t))
   {
      res = t; // optimization
   }
   else if (TermIsLambda(t))
   {
      PStack_p bvars = PStackAlloc();
      Term_p matrix = UnfoldLambda(t, bvars);
      Term_p reduced = do_eta_reduce_db(bank, matrix);
      if(matrix == reduced)
      {
         res = t;
      }
      else
      {
         res = reduced;
         while(!PStackEmpty(bvars))
         {
            res = CloseWithDBVar(bank, ((Term_p)PStackPopP(bvars))->type, res);
         }
      }
      PStackFree(bvars);

      res = reduce_eta_top_level(bank, res);
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(long i=0; i<t->arity; i++)
      {
         res->args[i] = do_eta_reduce_db(bank, t->args[i]);
         changed = changed || res->args[i] != t->args[i];
      }

      res = changed ? flatten_and_make_shared(bank, res) :
                      (TermTopFree(res), t);
   }

   if((res->f_code == bank->sig->qall_code
         || res->f_code == bank->sig->qex_code)
      && res->arity == 1
      && !TermIsLambda(res->args[0]))
   {
      Term_p copy = TermTopCopyWithoutArgs(res);
      copy->args[0] = LambdaEtaExpandDBTopLevel(bank, res->args[0]);
      res = TBTermTopInsert(bank, copy);
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: do_beta_normalize_db()
//
//   Performs the actual beta-normalization.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p do_beta_normalize_db(TB_p bank, Term_p t)
{
   Term_p res = NULL;
   DBGTermCheckUnownedSubterm(stdout, t, "UnownedBN0");

   if(TermIsPhonyApp(t) && TermIsLambda(t->args[0]))
   {
      res = WHNF_step(bank, t);
      DBGTermCheckUnownedSubterm(stdout, res, "UnownedBN1");
      if(TermIsBetaReducible(res))
      {
         res = do_beta_normalize_db(bank, res);
         DBGTermCheckUnownedSubterm(stdout, res, "UnownedBN2");
      }
   }
   else if (t->arity == 0 || !TermIsBetaReducible(t))
   {
      res = t; // optimization
      DBGTermCheckUnownedSubterm(stdout, res, "UnownedBN2.5");
   }
   else if (TermIsLambda(t))
   {
      assert(t->f_code == SIG_DB_LAMBDA_CODE);
      assert(TermIsDBVar(t->args[0]));

      Term_p matrix = t->args[1];
      Term_p reduced_matrix = do_beta_normalize_db(bank, matrix);
      DBGTermCheckUnownedSubterm(stdout, matrix, "UnownedBN3");
      if(matrix == reduced_matrix)
      {
         res = t;
         DBGTermCheckUnownedSubterm(stdout, res, "UnownedBN3.5");
      }
      else
      {
         res = CloseWithDBVar(bank, t->args[0]->type, reduced_matrix);
         DBGTermCheckUnownedSubterm(stdout, res, "UnownedBN4");
      }
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(long i=0; i < res->arity; i++)
      {
         res->args[i] = do_beta_normalize_db(bank, t->args[i]);
         changed = changed || res->args[i] != t->args[i];
      }

      if(!changed)
      {
         TermTopFree(res);
         res = t;
         DBGTermCheckUnownedSubterm(stdout, res, "UnownedBN6");
      }
      else
      {
         res = TBTermTopInsert(bank, res);
         DBGTermCheckUnownedSubterm(stdout, res, "UnownedBN7");
      }
   }
   DBGTermCheckUnownedSubterm(stdout, res, "UnownedBNX");
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: do_named_to_db()
//
//   Performs the actual conversion. Tries to reduce recursion by doing
//   multiple lambda steps at the same time. Recursion can be completely
//   elimininated using two stacks: One with pairs (term_to_process, depth)
//   and the other one which records the terms that have been decomposed.
//   However, we go for recursion it is unlikely that we will have that
//   extremely deep terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p do_named_to_db(TB_p bank, Term_p t, long depth)
{
   Term_p res = NULL;
   if(TermIsLambda(t))
   {
      assert(t->f_code == SIG_NAMED_LAMBDA_CODE);
      assert(t->arity == 2);
      assert(TermIsFreeVar(t->args[0]));

      PStack_p vars = PStackAlloc();
      PStack_p previous_bindings = PStackAlloc();

      Term_p body = UnfoldLambda(t, vars);
      for(PStackPointer i=0; i < PStackGetSP(vars); i++)
      {
         Term_p var = (Term_p)PStackElementP(vars, i);
         PStackPushP(previous_bindings, var->binding);
         var->binding = TBRequestDBVar(bank, var->type, depth++);
      }

      res = do_named_to_db(bank, body, depth);

      while(!PStackEmpty(vars))
      {
         Term_p var = (Term_p)PStackPopP(vars);
         Term_p prev_binding = (Term_p)PStackPopP(previous_bindings);
         var->binding = prev_binding;

         res = CloseWithDBVar(bank, var->type, res);
      }

      PStackFree(vars);
      PStackFree(previous_bindings);
   }
   else if(TermIsFreeVar(t))
   {
      if(t->binding && TermIsDBVar(t->binding))
      {
         assert(t->binding->type == t->type);
         res = TBRequestDBVar(bank, t->type,
                              depth - t->binding->f_code - 1);
      }
      else
      {
         res = t;
      }
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(long i = 0; i < t->arity; i++)
      {
         res->args[i] = do_named_to_db(bank, t->args[i], depth);
         changed = changed || res->args[i] != t->args[i];
      }

      if (changed)
      {
         res = TBTermTopInsert(bank, res);
      }
      else
      {
         TermTopFree(res);
         res = t;
      }
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: encode_quantifiers_as_lambdas()
//
//   Encodes (![X,Y,Z]: body) into ! @ ^[X]: (! @ ^[Y]: ( ! @ ^[Z]: (body)))
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p do_post_cnf_encode(TB_p bank, Term_p t, long depth)
{
   Term_p res;
   if(TermIsDBVar(t))
   {
      res = t;
   }
   else if(TermIsFreeVar(t))
   {
      if(t->binding && TermIsDBVar(t->binding))
      {
         assert(t->binding->type == t->type);
         res = TBRequestDBVar(bank, t->type,
                              depth - t->binding->f_code - 1);
      }
      else
      {
         res = t;
      }
   }
   else if (!TermHasBoolSubterm(t) && TermIsGround(t))
   {
      res = t; // optimization
   }
   else if(TermIsLambda(t))
   {
      assert(t->f_code == SIG_DB_LAMBDA_CODE);
      assert(t->arity == 2);
      assert(TermIsDBVar(t->args[0]));
      Term_p new_matrix = do_post_cnf_encode(bank, t->args[1], depth+1);
      if(new_matrix != t->args[1])
      {
         res = CloseWithDBVar(bank, t->args[0]->type, new_matrix);
      }
      else
      {
         res = t;
      }
   }
   else if((t->f_code == bank->sig->qall_code || t->f_code == bank->sig->qex_code) &&
           t->arity == 2)
   {
      FunCode quant = t->f_code;
      PStack_p prefix = PStackAlloc();
      Term_p matrix = t;

      while(matrix->f_code == quant)
      {
         Term_p var = matrix->args[0];
         PStackPushP(prefix, var);
         PStackPushP(prefix, var->binding);
         var->binding = TBRequestDBVar(bank, var->type, depth++);

         matrix = matrix->args[1];
      }

      matrix =
         do_post_cnf_encode(bank,
            ShiftDB(bank, matrix, PStackGetSP(prefix) / 2), depth);

      while(!PStackEmpty(prefix))
      {
         Term_p prev_binding = PStackPopP(prefix);
         Term_p var = PStackPopP(prefix);
         var->binding = prev_binding;

         Term_p quantified = TermTopAlloc(quant, 1);
         quantified->args[0] = CloseWithDBVar(bank, var->type, matrix);
#ifdef NDEBUG
         quantified->type = bank->sig->type_bank->bool_type;
#endif
         matrix = TBTermTopInsert(bank, quantified);
      }

      PStackFree(prefix);
      res = matrix;
   }
   else if (t->f_code == bank->sig->eqn_code && t->arity == 2
            && t->args[1] == bank->true_term)
   {
      res = do_post_cnf_encode(bank, t->args[0], depth);
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(long i = 0; i < t->arity; i++)
      {
         res->args[i] = do_post_cnf_encode(bank, t->args[i], depth);
         changed = changed || res->args[i] != t->args[i];
      }

      if (changed)
      {
         res = TBTermTopInsert(bank, res);
      }
      else
      {
         TermTopFree(res);
         res = t;
      }
   }
   DBGTermCheckUnownedSubterm(stdout, res, "do_post_cnf_encodeX");
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: replace_bvars()
//
//   Assuming free variables are bound to DB variables
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p replace_fvars (TB_p bank, Term_p t, long depth)
{
   if(TermIsGround(t))
   {
      return t;
   }
   else if(TermIsFreeVar(t))
   {
      if(t->binding)
      {
         return depth==0 ? t->binding :
                 TBRequestDBVar(bank, t->type, t->binding->f_code+depth);
      }
      else
      {
         return t;
      }
   }
   else if(TermIsLambda(t))
   {
      Term_p new_matrix = replace_fvars(bank, t->args[1], depth+1);
      if(new_matrix != t->args[1])
      {
         return CloseWithDBVar(bank, t->args[0]->type, new_matrix);
      }
      else
      {
         return t;
      }
   }
   else
   {
      Term_p copy = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(long i=0; i<t->arity; i++)
      {
         copy->args[i] = replace_fvars(bank, t->args[i], depth);
         changed = changed || copy->args[i] != t->args[i];
      }
      if(changed)
      {
         return TBTermTopInsert(bank, copy);
      }
      else
      {
         TermTopFree(copy);
         return t;
      }
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: CloseWithDBVar()
//
//   Given body of the lambda, create a term LAM.body where LAM is the
//   abstraction constructor for DB var of type ty.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p CloseWithDBVar(TB_p bank, Type_p ty, Term_p body)
{
   assert(TermIsShared(body));

   Term_p res = TermTopAlloc(SIG_DB_LAMBDA_CODE, 2);
   res->args[0] = TBRequestDBVar(bank, ty, 0);
   res->args[1] = body;
   res->type =
      TypeBankInsertTypeShared(bank->sig->type_bank,
                               ArrowTypeFlattened(&ty, 1, body->type));
   return TBTermTopInsert(bank, res);
}

/*-----------------------------------------------------------------------
//
// Function: CloseWithTypePrefix()
//
//   Given an array of types [t1, t2, ..., tn] create a lambda term
//   [X1:t1]: (... [Xn:tn]: (s))
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p CloseWithTypePrefix(TB_p bank, Type_p* tys, long size, Term_p matrix)
{
   Term_p res = matrix;
   for(long i = size-1; i>=0; i--)
   {
      res = CloseWithDBVar(bank, tys[i], res);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SetEtaNormalizer()
//
//   Register a function that is going to be used for eta normalization.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

inline void SetEtaNormalizer(TermNormalizer norm)
{
   eta_norm = norm;
}

/*-----------------------------------------------------------------------
//
// Function: GetEtaNormalizer()
//
//   Register a function that is going to be used for eta normalization.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

inline TermNormalizer GetEtaNormalizer()
{
   return eta_norm;
}

/*-----------------------------------------------------------------------
//
// Function: NamedToDB()
//
//   Given *closed* lambda in the named representation,
//   return the corresponding
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p NamedToDB(TB_p bank, Term_p lambda)
{
   Term_p res = BetaNormalizeDB(bank,
                                TermHasLambdaSubterm(lambda) ?
                                do_named_to_db(bank, lambda, 0) : lambda);
   DBGTermCheckUnownedSubterm(stdout, res, "NamedToDBX");
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ShiftDB()
//
//   Shifts all losely bound variables by *shift_val*
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p ShiftDB(TB_p bank, Term_p term, int shift_val)
{
   Term_p res;
   if (shift_val == 0)
   {
      res = term;
   }
   else
   {
      res = do_shift_db(bank, term, shift_val, 0);
   }
   assert(res->type == term->type);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: AbstractVars()
//
//   Abstract var_prefix over matrix. Variable at the top of the stack
//   is the first one to abstract.
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p AbstractVars(TB_p terms, Term_p matrix, PStack_p var_prefix)
{
   assert(TermIsDBClosed(matrix));
   Subst_p subst = SubstAlloc();
   for(long i=PStackGetSP(var_prefix)-1; i>=0; i--)
   {
      Term_p v = PStackElementP(var_prefix, i);
      assert(v->binding == NULL);
      SubstAddBinding(subst, v,
         TBRequestDBVar(terms, v->type, PStackGetSP(var_prefix)-i-1));
   }
   matrix = replace_fvars(terms, matrix, 0);
   for(long i=PStackGetSP(var_prefix)-1; i>=0; i--)
   {
      matrix =
         CloseWithDBVar(terms, ((Term_p)PStackElementP(var_prefix,i))->type, matrix);
   }
   SubstDelete(subst);
   assert(TermIsDBClosed(matrix));
   return matrix;
}

/*-----------------------------------------------------------------------
//
// Function: whnf_step_uncached()
//
//   Actaully compute whnf without considering cache.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p whnf_step_uncached(TB_p bank, Term_p t)
{
   Term_p  res = NULL;
   long    num_remaining = t->arity - 1;
   Term_p* remaining_args = t->args + 1;
   Term_p  matrix = t->args[0];

   assert(num_remaining);

   PStack_p to_bind_stack = PStackAlloc();
   while(TermIsLambda(matrix) && num_remaining)
   {
      assert(matrix->f_code == SIG_DB_LAMBDA_CODE);
      assert(TermIsDBVar(matrix->args[0]));
      PStackPushP(to_bind_stack, *remaining_args);
      assert((*remaining_args)->type == matrix->args[0]->type);
      remaining_args++;
      num_remaining--;
      matrix = matrix->args[1];
   }

   long total_bound = PStackGetSP(to_bind_stack);
   assert(total_bound);

   for(PStackPointer i=0; i < PStackGetSP(to_bind_stack); i++)
   {
      Term_p target = PStackElementP(to_bind_stack, i);
      Term_p db_var = TBRequestDBVar(bank, target->type, total_bound - i - 1);
      assert(db_var->binding == NULL);
      db_var->binding = target;
      PStackAssignP(to_bind_stack, i, db_var);
   }

   Term_p new_matrix = replace_bound_vars(bank, matrix, total_bound, 0);
   if (num_remaining)
   {
      PStack_p rest = PStackAlloc();
      for(long i = 0; i < num_remaining; i++)
      {
         PStackPushP(rest, remaining_args[i]);
      }
      new_matrix = ApplyTerms(bank, new_matrix, rest);
      PStackFree(rest);
   }

   while(!PStackEmpty(to_bind_stack))
   {
      ((Term_p)PStackPopP(to_bind_stack))->binding = NULL;
   }

   // res = do_beta_normalize_db(bank, new_matrix);
   res = new_matrix;

   PStackFree(to_bind_stack);
   TermSetCache(t, res);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: WHNF_step()
//
//   Given a term of the form (%XYZ. body) x1 x2 x3 x4 ...
//   Computes the term (body[X -> x1, Y -> x2; Z -> x3]) x4 ...
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p WHNF_step(TB_p bank, Term_p t)
{
   if(!TermIsPhonyApp(t) || !TermIsLambda(t->args[0]))
   {
      return t;
   }

   Term_p res = false;
   res = TermGetCache(t);
   if(!res)
   {
      res = whnf_step_uncached(bank, t);
   }
   //printf("### WHNF: ");
   //TermPrintDbg(stdout, t, bank->sig, DEREF_NEVER);
   //printf(" => ");
   //TermPrintDbg(stdout, res, bank->sig, DEREF_NEVER);
   //printf("\n");

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: WHNF_deref()
//
//   Dereference and beta-normalize term only until the head of the
//   term becomes known.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p WHNF_deref(Term_p t)
{
   Term_p res;
   t = TermDerefAlways(t);

   if(TermIsPhonyApp(t) && TermIsLambda(t->args[0]))
   {
      res = WHNF_deref(WHNF_step(TermGetBank(t), t));
   }
   else if (TermIsLambda(t))
   {
      PStack_p dbvars = PStackAlloc();
      Term_p matrix = UnfoldLambda(t, dbvars);
      Term_p new_matrix = WHNF_deref(matrix);
      if(matrix == new_matrix)
      {
         res = t;
      }
      else
      {
         res = new_matrix;
         while(!PStackEmpty(dbvars))
         {
            res = CloseWithDBVar(TermGetBank(t), ((Term_p)PStackPopP(dbvars))->type, res);
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
// Function: BetaNormalizeDB()
//
//   Normalizes de Bruijn encoded lambda terms
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p BetaNormalizeDB(TB_p bank, Term_p term)
{
   Term_p res = term;

   DBGTermCheckUnownedSubterm(stdout, term, "UnownedBNDB0");
   if(TermIsBetaReducible(term))
   {
      res = do_beta_normalize_db(bank, term);
      DBGTermCheckUnownedSubterm(stdout, res, "UnownedBNDB1");
      if(res->f_code == bank->sig->eqn_code &&
         res->arity==2 &&
         res->args[1] == bank->true_term &&
         res->args[0] != bank->true_term &&
         res->args[0]->f_code > 0 &&
         SigIsLogicalSymbol(bank->sig, res->args[0]->f_code))
      {
         res = res->args[0];
         DBGTermCheckUnownedSubterm(stdout, res, "UnownedBNDB2");
      }
   }
   assert(!TermIsBetaReducible(res));
   DBGTermCheckUnownedSubterm(stdout, res, "UnownedBNDBX");
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PostCNFEncodeFormulas()
//
//   Takes quantifiers encoded using a free variable into the ones
//   which use DB.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p PostCNFEncodeFormulas(TB_p bank, Term_p term)
{
   return LambdaNormalizeDB(bank, do_post_cnf_encode(bank, term, 0));
}

/*-----------------------------------------------------------------------
//
// Function: DecodeFormulasForCNF()
//
//   Takes formulas that are in the form for proving and decodes them
//   into the form necessary for CNF. Most importantly, atoms are encoded
//   as $eq(term, $true) and lambda-encoded quantifiers are turned into
//   variable-encoded ones.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p DecodeFormulasForCNF(TB_p bank, Term_p t)
{
   Sig_p sig = bank->sig;
   Term_p res = NULL;

   //printf("### DecodeFormulasForCNF(): ");
   //TermPrintDbg(stdout, t, bank->sig, DEREF_NEVER);
   //printf("\n");

   if((t->f_code == sig->qall_code || t->f_code == sig->qex_code) && t->arity == 1)
   {
      assert(TypeIsArrow(t->args[0]->type));
      assert(t->args[0]->type->arity == 2);
      assert(TypeIsPredicate(t->args[0]->type));
      Term_p fresh_var =
         VarBankGetFreshVar(bank->vars, t->args[0]->type->args[0]);
      Term_p new_matrix =
         WHNF_step(bank,
            TBTermTopInsert(bank,
               TermApplyArg(bank->sig->type_bank, t->args[0], fresh_var)));
      res = TFormulaQuantorAlloc(bank, t->f_code, fresh_var,
                                 DecodeFormulasForCNF(bank, new_matrix));
   }
   else if (TermIsAnyVar(t) || t->arity == 0)
   {
      res = t;
   }
   else if(TermIsLambda(t))
   {
      Term_p new_matrix = DecodeFormulasForCNF(bank, t->args[1]);
      if(t->args[1] != new_matrix)
      {
         res = CloseWithDBVar(bank, t->args[0]->type, new_matrix);
      }
      else
      {
         res = t;
      }
   }
   else
   {
      res = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(long i = 0; i < t->arity; i++)
      {
         res->args[i] = DecodeFormulasForCNF(bank, t->args[i]);
         changed = changed || res->args[i] != t->args[i];
      }

      if (changed)
      {
         res = TBTermTopInsert(bank, res);
      }
      else
      {
         TermTopFree(res);
         res = t;
      }
   }
   //printf("### End DecodeFormulasForCNF(): ");
   //TermPrintDbg(stdout, res, bank->sig, DEREF_NEVER);
   //printf("\n");

   return EncodePredicateAsEqn(bank, res);
}

/*-----------------------------------------------------------------------
//
// Function: LambdaEtaExpandDBTopLevel()
//
//   Do only one top-level step of eta expansion.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p LambdaEtaExpandDBTopLevel(TB_p bank, Term_p t)
{
   if(TypeIsArrow(t->type) && !TermIsLambda(t))
   {
      long num_args = TypeGetMaxArity(t->type);
      PStack_p db_args = PStackAlloc();
      for(long i=0; i<num_args; i++)
      {
         Term_p fresh_db = TBRequestDBVar(bank, t->type->args[i], num_args-i-1);
         PStackPushP(db_args,
                     TypeIsArrow(fresh_db->type) ?
                        do_eta_expand_db(bank, fresh_db) : fresh_db);
      }

      t = ApplyTerms(bank, ShiftDB(bank, t, num_args), db_args);
      while(!PStackEmpty(db_args))
      {
         t = CloseWithDBVar(bank, ((Term_p)PStackPopP(db_args))->type, t);
      }

      PStackFree(db_args);
   }
   return t;
}


/*-----------------------------------------------------------------------
//
// Function: LambdaEtaReduceDB()
//
//   Performs eta-reduction on DB terms
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p LambdaEtaReduceDB(TB_p bank, Term_p term)
{
   Term_p res;
   if(TermHasLambdaSubterm(term))
   {
      // sometimes bank comes from a variable and
      // that can be a problem.
      res = do_eta_reduce_db(!bank ? TermGetBank(term) : bank, term);
   }
   else
   {
      res = term;
   }
   assert(res->type == term->type);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: LambdaEtaExpandDB()
//
//   Performs eta-expansion on DB terms
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p LambdaEtaExpandDB(TB_p bank, Term_p term)
{
   if(TermHasEtaExpandableSubterm(term))
   {
      return do_eta_expand_db(bank, term);
   }
   else
   {
      return term;
   }
}

/*-----------------------------------------------------------------------
//
// Function: LambdaNormalizeDB()
//
//   Performs beta normalization, folowed by eta normalization.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p LambdaNormalizeDB(TB_p bank, Term_p term)
{
   DBGTermCheckUnownedSubterm(stdout, term, "LambdaNormalizeDB0");
   Term_p res = GetEtaNormalizer()(bank, BetaNormalizeDB(bank, term));
   DBGTermCheckUnownedSubterm(stdout, res, "LambdaNormalizeDBX");
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
