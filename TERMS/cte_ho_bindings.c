/*-----------------------------------------------------------------------

File  : cte_ho_bindings.c

Author: Petar Vukmirovic.

Contents

  Implementation of the module which creates higher-order variable
  bindings.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> ma 25 okt 2021 10:35:21 CEST
    New

-----------------------------------------------------------------------*/

#include "cte_ho_bindings.h"
#include "cte_pattern_match_mgu.h"
#include "cte_lambda.h"

#define IMIT_MASK (63U)
#define PROJ_MASK (IMIT_MASK << 6)
#define IDENT_MASK (PROJ_MASK << 6)
#define ELIM_MASK (IDENT_MASK << 6)

#define GET_IMIT(c) ((c)&IMIT_MASK)
#define GET_PROJ(c) (((c)&PROJ_MASK) >> 6)
#define GET_IDENT(c) (((c)&IDENT_MASK) >> 12)
#define GET_ELIM(c) (((c)&ELIM_MASK) >> 18)

#define INC_IMIT(c) ((GET_IMIT(c) + 1) | (~IMIT_MASK & c))
#define INC_PROJ(c) (((GET_PROJ(c) + 1) << 6) | (~PROJ_MASK & c))
#define INC_IDENT(c) (((GET_IDENT(c) + 1) << 12) | (~IDENT_MASK & c))
#define INC_ELIM(c) (((GET_ELIM(c) + 1) << 18) | (~ELIM_MASK & c))

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
// Function: build_imitation()
//
//   Builds imitation binding if rhs has a constant as the head.
//   Otherwise returns NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p build_imitation(TB_p bank, Term_p flex, Term_p rhs)
{
   Term_p res;
   if (TermIsPhonyApp(rhs) || TermIsFreeVar(rhs) || TermIsDBVar(rhs))
   {
      assert(!TermIsPhonyApp(rhs) || !TermIsLambda(rhs->args[0]));
      // it must be app free or bound variable
      res = NULL;
   }
   else
   {
      Type_p rigid_ty = SigGetType(bank->sig, rhs->f_code);
      if (rigid_ty)
      {
         Type_p var_ty = GetFVarHead(flex)->type;
         PStack_p db_vars = PStackAlloc();
         if (TypeIsArrow(var_ty))
         {
            for (int i = 0; i < var_ty->arity - 1; i++)
            {
               Term_p db_var =
                   TBRequestDBVar(bank, var_ty->args[i], var_ty->arity - i - 2);
               PStackPushP(db_vars, db_var);
            }
         }

         Term_p matrix;
         if (TypeIsArrow(rigid_ty))
         {
            matrix = TermTopAlloc(rhs->f_code, rigid_ty->arity - 1);
            for (int i = 0; i < rigid_ty->arity - 1; i++)
            {
               matrix->args[i] = FreshVarWArgs(bank, db_vars, rigid_ty->args[i]);
            }
         }
         else
         {
            matrix = TermConstCellAlloc(rhs->f_code);
         }
#ifndef NDEBUG
         matrix->type = GetRetType(rigid_ty);
#endif
         matrix = TBTermTopInsert(bank, matrix);
         res = CloseWithTypePrefix(bank, var_ty->args, TypeGetMaxArity(var_ty), matrix);

         PStackFree(db_vars);
      }
      else
      {
         // in some cases there are symbols that have polymorphic types --
         // their imitation is not yet implemented.
         res = NULL;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: build_projection()
//
//   Projects onto argument idx if return type of variable at the head
//   of flex returns the same type as the argument. Otherwise returns NULL.
//   Inside the code idx is increased because flex is applied using
//   PHONY_APP_CODE.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p build_projection(TB_p bank, Term_p flex, Term_p rhs, int idx)
{
   assert(TermIsAppliedFreeVar(flex));
   assert(!TermIsLambda(rhs));
   assert(idx < flex->arity - 1);
   Type_p var_ty = GetFVarHead(flex)->type;
   // optimization for not calling WHNF
   Term_p arg =
       TermIsTopLevelFreeVar(rhs) ? flex->args[idx + 1] : WHNF_step(bank, flex->args[idx + 1]);
   assert(GetRetType(var_ty) == GetRetType(arg->type));
   if (!TermIsTopLevelFreeVar(arg) && !TermIsTopLevelFreeVar(rhs))
   {
      if (TermIsTopLevelDBVar(arg) && TermIsTopLevelDBVar(rhs))
      {
         Term_p db_arg = TermIsPhonyApp(arg) ? arg->args[0] : arg;
         Term_p db_rhs = TermIsPhonyApp(rhs) ? rhs->args[0] : rhs;
         if (db_arg != db_rhs)
         {
            return NULL;
         }
      }
      else if (!TermIsTopLevelDBVar(arg) && !TermIsTopLevelDBVar(rhs) &&
               !TermIsLambda(arg) && !TermIsLambda(rhs))
      {
         if (arg->f_code != rhs->f_code)
         {
            return NULL;
         }
      }
      else if (!TermIsLambda(arg) && !TermIsLambda(rhs) &&
               !EQUIV(TermIsTopLevelDBVar(arg), TermIsTopLevelDBVar(rhs)))
      {
         return NULL;
      }
   }
   // up this point NULL will be returned if it was determined
   // that projection will result in failure

   Term_p matrix;
   if (TypeIsArrow(arg->type))
   {
      PStack_p db_vars = PStackAlloc();
      for (int i = 0; i < var_ty->arity - 1; i++)
      {
         Term_p db_var = TBRequestDBVar(bank, var_ty->args[i], var_ty->arity - i - 2);
         PStackPushP(db_vars, db_var);
      }

      matrix = TermTopAlloc(SIG_PHONY_APP_CODE, arg->type->arity);
      matrix->args[0] = PStackElementP(db_vars, idx);
#ifndef NDEBUG
      matrix->type = GetRetType(matrix->args[0]->type);
#endif
      for (int i = 1; i < arg->type->arity; i++)
      {
         matrix->args[i] = FreshVarWArgs(bank, db_vars, arg->type->args[i - 1]);
      }
      matrix = TBTermTopInsert(bank, matrix);
      PStackFree(db_vars);
   }
   else
   {
      matrix = TBRequestDBVar(bank, arg->type, TypeGetMaxArity(var_ty) - idx - 1);
   }
   return CloseWithTypePrefix(bank, var_ty->args, var_ty->arity - 1, matrix);
}

/*-----------------------------------------------------------------------
//
// Function: build_elim()
//
//   Eliminates argument idx. Always succeeds.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p build_elim(TB_p bank, Term_p flex, int idx)
{
   assert(TermIsAppliedFreeVar(flex));
   PStack_p db_vars = PStackAlloc();
   for (int i = 1; i < flex->arity; i++)
   {
      if (i - 1 != idx)
      {
         Term_p dbv = TBRequestDBVar(bank, flex->args[i]->type, flex->arity - i - 1);
         PStackPushP(db_vars, dbv);
      }
   }

   Term_p res = FreshVarWArgs(bank, db_vars, flex->type);
   for (int i = flex->arity - 1; i >= 1; i--)
   {
      res = CloseWithDBVar(bank, flex->args[i]->type, res);
   }
   PStackFree(db_vars);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: build_ident()
//
//   Builds identification binding. Must be called with both lhs and 
//   rhs top-level free variables. Then it returns.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool build_ident(TB_p bank, Term_p lhs, Term_p rhs,
                 Term_p *l_target, Term_p *r_target)
{
   bool res = false;
   if (TermIsTopLevelFreeVar(rhs))
   {
      Type_p l_ty = GetFVarHead(lhs)->type;
      Type_p r_ty = GetFVarHead(rhs)->type;
      assert(GetRetType(l_ty) == GetRetType(r_ty));
      const int l_tylen = TypeGetMaxArity(l_ty);
      const int r_tylen = TypeGetMaxArity(r_ty);
      Type_p arg_types[l_tylen + r_tylen];
      memcpy(arg_types, l_ty->args, l_tylen * sizeof(Type_p));
      memcpy(arg_types + l_tylen, r_ty->args, r_tylen * sizeof(Type_p));
      Type_p matrix_var_ty =
          TypeBankInsertTypeShared(bank->sig->type_bank,
                                   ArrowTypeFlattened(arg_types, l_tylen + r_tylen, GetRetType(l_ty)));
      Term_p matrix_var = VarBankGetFreshVar(bank->vars, matrix_var_ty);
      matrix_var = TBInsert(bank, matrix_var, DEREF_NEVER);

      PStack_p l_dbvars = PStackAlloc();
      PStack_p r_dbvars = PStackAlloc();

      PStack_p to_apply_l = PStackAlloc();
      PStack_p to_apply_r = PStackAlloc();

      for (int i = 0; i < l_tylen; i++)
      {
         Term_p dbv = TBRequestDBVar(bank, l_ty->args[i], l_tylen - i - 1);
         PStackPushP(l_dbvars, dbv);
         PStackPushP(to_apply_l, dbv);
      }
      for (int i = 0; i < r_tylen; i++)
      {
         Term_p dbv = TBRequestDBVar(bank, r_ty->args[i], r_tylen - i - 1);
         PStackPushP(r_dbvars, dbv);
         PStackPushP(to_apply_l, FreshVarWArgs(bank, l_dbvars, r_ty->args[i]));
      }
      for (int i = 0; i < l_tylen; i++)
      {
         PStackPushP(to_apply_r, FreshVarWArgs(bank, r_dbvars, l_ty->args[i]));
      }
      PStackPushStack(to_apply_r, r_dbvars);

      Term_p matrix_l = ApplyTerms(bank, matrix_var, to_apply_l);
      Term_p matrix_r = ApplyTerms(bank, matrix_var, to_apply_r);

      *l_target =
          CloseWithTypePrefix(bank, l_ty->args, TypeGetMaxArity(l_ty), matrix_l);
      *r_target =
          CloseWithTypePrefix(bank, r_ty->args, TypeGetMaxArity(r_ty), matrix_r);

      PStackFree(to_apply_r);
      PStackFree(to_apply_l);
      PStackFree(l_dbvars);
      PStackFree(r_dbvars);

      res = true;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: build_trivial_ident()
//
//   Builds trivial identification binding. Must be called with both lhs and 
//   rhs top-level free variables. Then it returns.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool build_trivial_ident(TB_p bank, Term_p lhs, Term_p rhs,
                         Term_p *l_target, Term_p *r_target)
{
   assert(TermIsTopLevelFreeVar(lhs));
   bool res = false;
   if (TermIsTopLevelFreeVar(rhs))
   {
      Type_p l_ty = GetFVarHead(lhs)->type;
      Type_p r_ty = GetFVarHead(rhs)->type;
      assert(GetRetType(l_ty) == GetRetType(r_ty));

      Term_p matrix =
          TBInsert(bank, VarBankGetFreshVar(bank->vars, GetRetType(l_ty)), DEREF_NEVER);

      *l_target =
          CloseWithTypePrefix(bank, l_ty->args,
                              TypeIsArrow(l_ty) ? l_ty->arity - 1 : 0, matrix);
      *r_target =
          CloseWithTypePrefix(bank, r_ty->args,
                              TypeIsArrow(r_ty) ? r_ty->arity - 1 : 0, matrix);
      res = true;
   }
   return res;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: SubstComputeFixpointMgu()
//
//   Assuming that flex is an (applied) variable and rhs an arbitrary term
//   which are normalized and to which substitution is applied generate
//   the next binding in an attempt to solve the problem flex =?= rhs. 
//   What the next binding is is determined by the value of 'state'.
//   The last two bits of 'state' have special meaning (is the variable
//   pair already processed) and the remaining bits determine how far
//   in the enumeration of bindings we are. 'applied_bs' counts how
//   many bindings of a certain kind are applied. It is a value that
//   is inspected through bit masks that give value of particular bindings.
//   Sets succ to true if substitution was changed
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

StateTag_t ComputeNextBinding(Term_p flex, Term_p rhs,
                              StateTag_t state, Limits_t *applied_bs,
                              TB_p bank, Subst_p subst,
                              HeuristicParms_p parms, bool *succ)
{
   assert(TermIsTopLevelFreeVar(flex));
   StateTag_t cnt = CONSTRAINT_COUNTER(state);
   StateTag_t end_state = CONSTRAINT_STATE(state);
   StateTag_t res = 0;
   PStackPointer orig_subst = PStackGetSP(subst);

   if (end_state != DECOMPOSED_VAR)
   {
      const int num_args_l = MAX(flex->arity - 1, 0);
      const int num_args_r = TermIsTopLevelFreeVar(rhs) ? MAX(rhs->arity - 1, 0) : 0;
      const int limit = 1 + 2 * num_args_l + 2 * num_args_r + 1;
      // 1 for imitation
      // 2*arguments for projection and eliminations
      // 1 for identification

      Term_p target = NULL;
      while (!target && cnt < limit)
      {
         if (cnt == 0)
         {
            cnt++;
            if (!TermIsTopLevelFreeVar(rhs) &&
                GET_IMIT(*applied_bs) < parms->imit_limit)
            {
               target = build_imitation(bank, flex, rhs);
               if (target)
               {
                  // imitation building can fail if head is DB var
                  SubstAddBinding(subst, GetFVarHead(flex), target);
                  if(!TermIsGround(target))
                  {
                     *applied_bs = INC_IMIT(*applied_bs);
                  }
               }
            }
         }
         else if ((num_args_l != 0 || num_args_r != 0) && cnt <= num_args_l + num_args_r)
         {
            // sometimes we need to apply projection on both left
            // and right side
            bool left_side = num_args_l != 0 && cnt <= num_args_l;
            Term_p arg =
                left_side ? flex->args[cnt] : rhs->args[cnt - num_args_l];
            Term_p hd_var = GetFVarHead(left_side ? flex : rhs);
            if (GetRetType(hd_var->type) == GetRetType(arg->type) &&
                (GET_PROJ(*applied_bs) < parms->func_proj_limit || !TypeIsArrow(arg->type)))
            {
               int offset = left_side ? 1 : num_args_l + 1;
               if (!left_side)
               {
                  SWAP(flex, rhs);
               }
               target = build_projection(bank, flex, rhs, cnt - offset);
               if (target)
               {
                  // building projection can fail if it is determined
                  // that it would lead to unsolvable problem
                  SubstAddBinding(subst, GetFVarHead(flex), target);
                  *applied_bs = INC_PROJ(*applied_bs);
               }
            }
            cnt++;
         }
         else if ((num_args_l != 0 || num_args_r != 0) &&
                  cnt <= 2 * (num_args_l + num_args_r))
         {
            // elimination -- currently computing only linear
            // applied variable so we do not subtract 1
            if (GET_ELIM(*applied_bs) < parms->elim_limit)
            {
               bool left_side = num_args_l != 0 && cnt <= 2 * num_args_l + num_args_r;
               if (!left_side)
               {
                  flex = rhs;
               }
               int offset = (left_side ? 1 : 2) * num_args_l + num_args_r + 1;
               target = build_elim(bank, flex, cnt - offset);
               cnt++;
               SubstAddBinding(subst, GetFVarHead(flex), target);
               *applied_bs = INC_ELIM(*applied_bs);
            }
            else
            {
               // skipping other arguments
               cnt = 2 * (num_args_l + num_args_r) + 1;
            }
         }
         else if ((cnt == 2 * (num_args_l + num_args_r) + 1) && TermIsTopLevelFreeVar(rhs) && (GetFVarHead(flex) != GetFVarHead(rhs)))
         {
            // identification
            cnt++;
            Term_p l_target, r_target;
            bool succ =
                (GET_IDENT(*applied_bs) < parms->ident_limit ? build_ident : build_trivial_ident)(bank, flex, rhs, &l_target, &r_target);

            end_state = DECOMPOSED_VAR;
            if (succ)
            {
               *applied_bs = INC_IDENT(*applied_bs);
               target = l_target; // to break;
               SubstAddBinding(subst, GetFVarHead(flex), l_target);
               SubstAddBinding(subst, GetFVarHead(rhs), r_target);
            }
         }
         else
         {
            cnt++;
         }
      }
   }
   res = BUILD_CONSTR(cnt, end_state);
   *succ = PStackGetSP(subst) != orig_subst;
   return res;
}
