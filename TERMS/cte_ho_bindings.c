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

#define CONSTRAINT_STATE(c) ((c) & 3)
#define CONSTRAINT_COUNTER(c) ((c) >> 2) // c must be unisigned!!!
#define BUILD_CONSTR(c, s) (((c)<<2)|s)

#define IMIT_MASK (63U)
#define PROJ_MASK (IMIT_MASK << 6)
#define IDENT_MASK (PROJ_MASK << 6)
#define ELIM_MASK (IDENT_MASK << 6)

#define GET_IMIT(c) ( (c) & IMIT_MASK )
#define GET_PROJ(c) ( ((c) & PROJ_MASK) >> 6 )
#define GET_IDENT(c) ( ((c) & IDENT_MASK) >> 12 )
#define GET_ELIM(c) ( ((c) & ELIM_MASK) >> 18 )


#define INC_IMIT(c) ( (GET_IMIT(c)+1) | (~IMIT_MASK & c) )
#define INC_PROJ(c) ( ((GET_PROJ(c)+1) << 6) | (~PROJ_MASK & c) )
#define INC_IDENT(c) ( ((GET_IDENT(c)+1) << 12) | (~IDENT_MASK & c) )
#define INC_ELIM(c) ( ((GET_ELIM(c)+1) << 18) | (~ELIM_MASK & c) )


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
   if(TermIsPhonyApp(rhs) || TermIsDBVar(rhs))
   {
      assert(!TermIsLambda(rhs->args[0]));
      // it must be app free or bound variable
      res = NULL; 
   }
   else
   {
      Type_p var_ty = GetFVarHead(flex)->type;
      PStack_p db_vars = PStackAlloc();
      if(TypeIsArrow(var_ty))
      {
         for(int i=0; i<var_ty->arity-1; i++)
         {
            Term_p db_var =
               RequestDBVar(bank->db_vars, var_ty->args[i], var_ty->arity-i-2); 
            PStackPushP(db_vars, db_var);
         }
      }

      Type_p rigid_ty = SigGetType(bank->sig, rhs->f_code);
      Term_p matrix;
      if(TypeIsArrow(rigid_ty))
      {
         matrix = TermTopAlloc(rhs->f_code, rigid_ty->arity-1);
         for(int i=0; i<rigid_ty->arity-1; i++)
         {
            Term_p fvar = FreshVarWArgs(bank, db_vars, rigid_ty->args[i]);
            matrix->args[i] = fvar;
         }
      }
      else
      {
         matrix = TermConstCellAlloc(rhs->f_code);
         matrix->type = rigid_ty;
      }
      matrix = TBTermTopInsert(bank, matrix);
      res = CloseWithTypePrefix(bank, var_ty->args, TypeGetMaxArity(var_ty), matrix);

      PStackFree(db_vars);
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
   assert(idx <= flex->arity - 1);
   Type_p var_ty = GetFVarHead(flex)->type;
   // optimization for not calling WHNF
   Term_p arg =
      TermIsTopLevelFreeVar(rhs) ?
         flex->args[idx+1] : WHNF_step(bank, flex->args[idx+1]);
   assert(GetRetType(var_ty) == GetRetType(arg->type));

   if(!TermIsTopLevelFreeVar(arg) && !TermIsTopLevelFreeVar(rhs))
   {
      if(TermIsTopLevelDBVar(arg) && TermIsTopLevelDBVar(rhs))
      {
         Term_p db_arg = TermIsPhonyApp(arg) ? arg->args[0] : arg;
         Term_p db_rhs = TermIsPhonyApp(rhs) ? rhs->args[0] : rhs;
         if(db_arg != db_rhs)
         {
            return NULL;
         }
      }
      else if(!TermIsTopLevelAnyVar(arg) && !TermIsTopLevelAnyVar(rhs) &&
              !TermIsLambda(arg) && !TermIsLambda(rhs))
      {
         if(arg->f_code != rhs->f_code)
         {
            return NULL;
         }
      }
   }
   // up this point NULL will be returned if it was determined
   // that projection will result in failure

   Term_p matrix;
   if(TypeIsArrow(arg->type))
   {
      PStack_p db_vars = PStackAlloc();
      for(int i=0; i<var_ty->arity-1; i++)
      {
         Term_p db_var =
            RequestDBVar(bank->db_vars, var_ty->args[i], var_ty->arity-i-2); 
         PStackPushP(db_vars, db_var);
      }

      matrix = TermTopAlloc(SIG_PHONY_APP_CODE, arg->type->arity);
      matrix->args[0] = PStackElementP(db_vars, idx);
#ifndef NDEBUG
      matrix->type = GetRetType(matrix->args[0]->type);
#endif
      for(int i=1; i<arg->type->arity; i++)
      {
         matrix->args[i] = FreshVarWArgs(bank, db_vars, arg->type->args[i-1]);
      }
      matrix = TBTermTopInsert(bank, matrix);
      PStackFree(db_vars);
   }
   else
   {
      matrix = TBRequestDBVar(bank, arg->type, TypeGetMaxArity(var_ty)-idx-1);
   }
   return CloseWithTypePrefix(bank, var_ty->args, var_ty->arity-1, matrix);
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
   for(int i=1; i<flex->arity; i++)
   {
      if(i-1 != idx)
      {
         Term_p dbv = TBRequestDBVar(bank, flex->args[i]->type, flex->arity-i-1);
         PStackPushP(db_vars, dbv);
      }
   }

   Term_p res = FreshVarWArgs(bank, db_vars, flex->type);
   for(int i=flex->arity-1; i>=1; i--)
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

Term_p build_ident(TB_p bank, Term_p lhs, Term_p rhs)
{
   return NULL;
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

Term_p build_trivial_ident(TB_p bank, Term_p lhs, Term_p rhs)
{
   return NULL;
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

ConstraintTag_t ComputeNextBinding(Term_p flex, Term_p rhs, 
                                   ConstraintTag_t state, Limits_t* applied_bs,
                                   TB_p bank, Subst_p subst,
                                   HeuristicParms_p parms, bool* succ)
{
   assert(TermIsTopLevelFreeVar(flex));
   ConstraintTag_t cnt = CONSTRAINT_COUNTER(state);
   ConstraintTag_t is_solved = CONSTRAINT_STATE(state);
   ConstraintTag_t res = 0;
   PStackPointer orig_subst = PStackGetSP(subst);

   if(is_solved != DECOMPOSED_VAR)
   {
      const int num_args_l = MAX(flex->arity-1, 0);
      const int num_args_r = TermIsTopLevelFreeVar(rhs) ? MAX(rhs->arity-1, 0) : 0;
      const int limit = 1 + 2*num_args_l + 2*num_args_r + 1;
                  // 1 for imitation
                  // 2*arguments for projection and eliminations
                  // 1 for identification

      while(res == 0 && cnt < limit)
      {
         if(cnt == 0)
         {
            cnt++;
            if(!TermIsAppliedFreeVar(rhs) &&
               GET_IMIT(*applied_bs) < parms->imit_limit)
            {
               Term_p target = build_imitation(bank, flex, rhs);
               if(target)
               {
                  // imitation building can fail if head is DB var
                  res = BUILD_CONSTR(cnt, state);
                  SubstAddBinding(subst, flex, target);
                  *applied_bs = INC_IMIT(*applied_bs);
               }
            }
         }
         else if((num_args_l != 0 || num_args_r != 0) 
                  && cnt <= num_args_l + num_args_r)
         {
            // sometimes we need to apply projection on both left
            // and right side
            bool left_side = num_args_l != 0 && cnt <= num_args_l;
            Term_p arg = 
               left_side ? flex->args[cnt] : rhs->args[cnt-num_args_l]; 
            Term_p hd_var = GetFVarHead(left_side ? flex : rhs);
            if(GetRetType(hd_var->type) == GetRetType(arg->type) &&
               (GET_PROJ(*applied_bs) < parms->func_proj_limit
               || !TypeIsArrow(arg->type)))
            {
               int offset = left_side ? 1 : num_args_l + 1;
               if(!left_side)
               {
                  SWAP(flex, rhs);
               }
               Term_p target = build_projection(bank, flex, rhs, cnt-offset);
               if(target)
               {
                  // building projection can fail if it is determined
                  // that it would lead to unsolvable problem
                  res = BUILD_CONSTR(cnt+1, state);
                  SubstAddBinding(subst, flex, target);
                  *applied_bs = INC_PROJ(*applied_bs);
               }
            }
            cnt++;
         }
         else if((num_args_l != 0 || num_args_r != 0) && 
                 cnt <= 2*(num_args_l+num_args_r))
         {
            // elimination -- currently computing only linear
            // applied variable so we do not subtract 1
            cnt++;
            if(GET_ELIM(*applied_bs) < parms->elim_limit)
            {
               bool left_side = num_args_l != 0 && cnt <= 2*num_args_l + num_args_r;
               if(!left_side)
               {
                  flex = rhs;
               }
               int offset = cnt - (left_side ? 1 : 2)*num_args_l - num_args_r;
               Term_p target = 
                  build_elim(bank, flex, cnt-offset);
               res = BUILD_CONSTR(cnt, state);
               SubstAddBinding(subst, GetFVarHead(flex), target);
               *applied_bs = INC_ELIM(*applied_bs);
            }
            else 
            {
               // skipping other arguments
               cnt = 2*(num_args_l+num_args_r)+1;
            }
         }
         else if(cnt == 2*(num_args_l+num_args_r)+1 && TermIsTopLevelFreeVar(rhs))
         {
            // identification
            cnt++;
            Term_p target = 
               (GET_IDENT(*applied_bs) < parms->ident_limit ? build_ident : build_trivial_ident)
               (bank, flex, rhs);
            res = BUILD_CONSTR(cnt, DECOMPOSED_VAR);
            *applied_bs = INC_IDENT(*applied_bs);
            SubstAddBinding(subst, flex, target);
         }
      }
   }
   *succ = PStackGetSP(subst) != orig_subst;
   return res;
}
