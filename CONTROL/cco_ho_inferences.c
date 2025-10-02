/*-----------------------------------------------------------------------

File  : cco_ho_inferences.c

Author: Petar Vukmirovic

Contents

  Functions that implement higher-order inferences that are non-essential
  to superposition.

  Copyright 2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include "cco_ho_inferences.h"
#include <cte_lambda.h>
#include <ccl_tcnf.h>

typedef struct ptr_pair_
{
   void* x;
   void* y;
} PtrPair;

typedef PtrPair* PtrPair_p;

#define PtrPairAlloc() (SizeMalloc(sizeof(PtrPair)))
#define PtrPairFree(junk) SizeFree((junk), sizeof(PtrPair))


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

void set_proof_object(Clause_p new_clause, Clause_p orig_clause, Clause_p parent2,
                      DerivationCode dc, int depth_incr);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: mk_ptr_pair()
//
//   Create a pointer pair on the heap
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline PtrPair_p mk_ptr_pair(void* x, void* y)
{
   PtrPair_p pair = PtrPairAlloc();
   pair->x = x;
   pair->y = y;
   return pair;
}

/*-----------------------------------------------------------------------
//
// Function: instantiate_w_abstractions()
//
//   Find abstraction for the variable var in orig_cl and store
//   the resulting clause in res
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void instantiate_w_abstractions(Term_p var, Clause_p orig_cl, PObjMap_p* store,
                                PStack_p res)
{
   PStack_p hits = PObjMapFind(store, var->type, PCmpFun);

   for(PStackPointer i=0; hits && i<PStackGetSP(hits); i++)
   {
      PtrPair_p target_cl = PStackElementP(hits, i);
      Term_p target = target_cl->x;
      Clause_p other_cl = target_cl->y;

      assert(!var->binding);
      assert(var->type == target->type);

      var->binding = target;
      Eqn_p res_lits = EqnListCopyOpt(orig_cl->literals);
      EqnListLambdaNormalize(res_lits);
      EqnListRemoveResolved(&res_lits);
      EqnListRemoveDuplicates(res_lits);
      Clause_p res_cl = ClauseAlloc(res_lits);
      NormalizeEquations(res_cl);
      set_proof_object(res_cl, orig_cl, other_cl, DCTrigger, 1);
      BooleanSimplification(res_cl);
      PStackPushP(res, res_cl);
      var->binding = NULL;
   }
}

/*-----------------------------------------------------------------------
//
// Function: do_abstract()
//
//   Replace arg with DB variable 0 (appropriately shifted) in t
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p do_abstract(Term_p t, Term_p arg, TB_p bank, int depth, Subst_p refresh)
{
   Term_p res;
   if(t == arg)
   {
      res = TBRequestDBVar(bank, arg->type, depth);
   }
   else if(TermIsLambda(t))
   {
      Term_p new_matrix = do_abstract(t->args[1], arg, bank,
                                       depth+1, refresh);
      if(new_matrix != t->args[1])
      {
         res = CloseWithDBVar(bank, t->args[0]->type, new_matrix);
      }
      else
      {
         res = t;
      }
   }
   else if(TermIsFreeVar(t))
   {
      if(t->binding)
      {
         res = t->binding;
      }
      else
      {
         Term_p fvar = VarBankGetFreshVar(bank->vars, t->type);
         SubstAddBinding(refresh, t, fvar);
         res = fvar;
      }
   }
   else if(t->arity == 0)
   {
      res = t;
   }
   else
   {
      Term_p new = TermTopCopyWithoutArgs(t);
      bool changed = false;
      for(int i=0; i<t->arity; i++)
      {
         new->args[i] = do_abstract(t->args[i], arg, bank, depth, refresh);
         changed = changed || new->args[i] != t->args[i];
      }

      if(changed)
      {
         res = TBTermTopInsert(bank, new);
      }
      else
      {
         TermTopFree(new);
         res = t;
      }
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: abstract_arg()
//
//   Construct an abrastraction %x. lhs[x] = rhs[x] where
//   lhs[x] is lhs in which arg is replaced by x (similarly rhs[x]).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p abstract_arg(Term_p lhs, Term_p rhs, Term_p arg, TB_p bank, bool sign)
{
   Subst_p refresher = SubstAlloc();
   Term_p lhs_abs = do_abstract(lhs, arg, bank, 0, refresher);
   Term_p rhs_abs = do_abstract(rhs, arg, bank, 0, refresher);

   Term_p matrix = EqnTermsTBTermEncode(bank, lhs_abs, rhs_abs, sign, PENormal);
   SubstDelete(refresher);
   return CloseWithDBVar(bank, arg->type, matrix);
}

/*-----------------------------------------------------------------------
//
// Function: store_abstraction()
//
//   Adds the calculated abstraction to the store
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void add_abs_to_store(PObjMap_p* store, Term_p abstraction, Clause_p cl)
{
   PStack_p* res = (PStack_p*)
      PObjMapGetRef(store, abstraction->type, PCmpFun, NULL);
   if(!*res)
   {
      *res = PStackAlloc();
   }
   bool found = false;
   for(PStackPointer i=0; !found && i < PStackGetSP(*res); i++)
   {
      PtrPair_p pair = PStackElementP(*res, i);
      found = pair->x == abstraction;
   }
   if (!found)
   {
      PtrPair_p pair = mk_ptr_pair(abstraction, cl);
      PStackPushP(*res, pair);
   }
}


/*-----------------------------------------------------------------------
//
// Function: store_abstraction_form()
//
//   If the formula is of the shape Q1[X]. Q2[Y].... Qn[Z]: f
//   where Qi is a quantifier, store abstraction for every quantifier.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void store_abstraction_form(WFormula_p wform, ClauseSet_p archive, PObjMap_p* store)
{
   TB_p bank = wform->terms;
   Sig_p sig = bank->sig;

   if(TFormulaIsQuantifiedNL(sig, wform->tformula) &&
      wform->tformula->arity == 2)
   {
      Term_p encoded = PostCNFEncodeFormulas(bank, wform->tformula);
      Eqn_p lit = EqnAlloc(encoded, bank->true_term, bank, true);
      Clause_p cl = ClauseAlloc(lit);
      ClausePushDerivation(cl, DCFofQuote, wform, NULL);
      ClauseSetInsert(archive, cl);

      Term_p quantified = encoded;
      while(TFormulaIsQuantifiedNL(sig, quantified) && quantified->arity == 1)
      {
         Term_p lambda = quantified->args[0];
         add_abs_to_store(store, lambda, cl);
         Term_p fvar = VarBankGetFreshVar(bank->vars, lambda->args[0]->type);
         quantified =
            WHNF_step(bank,
              TBTermTopInsert(bank, TermApplyArg(sig->type_bank, lambda, fvar)));
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: store_abstraction_cl()
//
//   Stores the computed inference with the given derivation code
//   in the temporary store for the newly infered clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void store_abstraction_cl(Clause_p cl, PObjMap_p* store)
{
   assert(ClauseLiteralNumber(cl) == 1);
   Eqn_p lit = cl->literals;
   Sig_p sig = lit->bank->sig;

   if(lit->lterm->f_code > sig->internal_symbols)
   {
      Term_p terms[2] = {lit->lterm, lit->rterm};

      for(int term_i=0; term_i<2; term_i++)
      {
         Term_p t = terms[term_i];
         Term_p other = terms[1-term_i];
         if(t->f_code == other->f_code)
         {
            for(int arg_i=0; arg_i<lit->lterm->arity; arg_i++)
            {
               if(TermIsDBClosed(t->args[arg_i]) &&
                  TermIsSubterm(other, t->args[arg_i], DEREF_NEVER))
               {
                  Term_p abstraction = abstract_arg(t, other, t->args[arg_i],
                                                   lit->bank, !EqnIsPositive(lit));
                  add_abs_to_store(store, abstraction, cl);
               }
            }
         }
         else
         {
            assert(TermIsDBClosed(t) && TermIsDBClosed(other));
            if(TermIsSubterm(other, t, DEREF_NEVER))
            {
               Term_p abstraction = abstract_arg(t, other, t,
                                                 lit->bank, !EqnIsPositive(lit));
               add_abs_to_store(store, abstraction, cl);
            }
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: set_proof_object()
//
//   Stores the computed inference with the given derivation code
//   in the temporary store for the newly infered clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define PROOF_DEPTH(c) ((c) ? ((c)->proof_depth) : 0)
#define PROOF_SIZE(c) ((c) ? ((c)->proof_size) : 0)

void set_proof_object(Clause_p new_clause, Clause_p orig_clause, Clause_p parent2,
                      DerivationCode dc, int depth_incr)
{
   new_clause->proof_depth =
      MAX(PROOF_DEPTH(orig_clause), PROOF_DEPTH(parent2)) + depth_incr;
   new_clause->proof_size =
      PROOF_SIZE(orig_clause) + PROOF_SIZE(parent2) + 1;
   ClauseSetTPTPType(new_clause, ClauseQueryTPTPType(orig_clause));
   ClauseSetProp(new_clause, ClauseGiveProps(orig_clause, CPIsSOS));
   // TODO: Clause documentation is not implemented at the moment.
   // DocClauseCreationDefault(clause, inf_efactor, clause, NULL);
   ClausePushDerivation(new_clause, dc, orig_clause, parent2);
}

/*-----------------------------------------------------------------------
//
// Function: store_result()
//
//   Stores the computed inference with the given derivation code
//   in the temporary store for the newly infered clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void store_result(Clause_p new_clause, Clause_p orig_clause, Clause_p parent2,
                  ClauseSet_p store, DerivationCode dc, int depth_incr)
{
   set_proof_object(new_clause, orig_clause, parent2, dc, depth_incr);
   ClauseSetInsert(store, new_clause);
}

/*-----------------------------------------------------------------------
//
// Function: fresh_pattern_w_ty()
//
//   Given an applied variable s, create a fresh variable applied to bound
//   variables representing arguments of s, whose return type is ty
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p fresh_pattern_w_ty(TB_p bank, Term_p t, Type_p ty)
{
   assert(TermIsAppliedFreeVar(t));
   Type_p arg_tys[t->arity-1];
   Type_p var_ty = NULL;
   Term_p res = NULL;
   for(int i=1; i<t->arity; i++)
   {
      arg_tys[i-1] = t->args[i]->type;
   }
   var_ty = TypeBankInsertTypeShared(bank->sig->type_bank,
               ArrowTypeFlattened(arg_tys, t->arity-1, ty));
   Term_p fresh_var = TBInsert(bank, VarBankGetFreshVar(bank->vars, var_ty), DEREF_NEVER);
   Term_p applied = TermTopCopyWithoutArgs(t);
#ifdef NDEBUG
   applied->type = ty;
#else
   applied->type = NULL;
#endif
   applied->args[0] = fresh_var;

   for(int i=1; i<t->arity; i++)
   {
      applied->args[i] =
         TBRequestDBVar(bank, t->args[i]->type, t->arity-i-1);
   }

   res = TBTermTopInsert(bank, applied);
   assert(res->type == ty);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: fresh_pattern()
//
//   Like fresh_pattern_w_ty but copies the return type from t
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p fresh_pattern(TB_p bank, Term_p t)
{
   return fresh_pattern_w_ty(bank, t, t->type);
}

/*-----------------------------------------------------------------------
//
// Function: close_for_appvar()
//
//   Analyze the arguments of applied variable and generate a lambda
//   prefix for the matrix that corresponds to each argument of the lambda
//   var
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p close_for_appvar(TB_p bank, Term_p appvar, Term_p matrix)
{
   Type_p ty_pref[appvar->arity-1];
   for(int i=1; i<appvar->arity; i++)
   {
      ty_pref[i-1] = appvar->args[i]->type;
   }
   return CloseWithTypePrefix(bank, ty_pref, appvar->arity-1, matrix);
}

/*-----------------------------------------------------------------------
//
// Function: apply_pattern_vars()
//
//   Apply fresh variables (applied to bound ones that correspond to arguments
//   of appvar) to head.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p apply_pattern_vars(TB_p bank, Term_p head, Term_p appvar)
{
   assert(TermIsAppliedFreeVar(appvar));
   Term_p res;
   if(!TypeIsArrow(head->type))
   {
      res = head;
   }
   else
   {
      Type_p arg_tys[appvar->arity-1];
      for(int i=1; i<appvar->arity; i++)
      {
         arg_tys[i-1] = appvar->args[i]->type;
      }

      PStack_p db_args = PStackAlloc();
      for(int i=1; i < appvar->arity; i++)
      {
         PStackPushP(db_args,
            TBRequestDBVar(bank, appvar->args[i]->type, appvar->arity-i-1));
      }

      PStack_p hd_args = PStackAlloc();
      for(int i=0; i<head->type->arity-1; i++)
      {
         Type_p v_ty =
            TypeBankInsertTypeShared(bank->sig->type_bank,
               ArrowTypeFlattened(arg_tys, appvar->arity-1, head->type->args[i]));
         Term_p fvar = TBInsert(bank, VarBankGetFreshVar(bank->vars, v_ty), DEREF_NEVER);

         PStackPushP(hd_args, ApplyTerms(bank, fvar, db_args));
      }

      res = ApplyTerms(bank, head, hd_args);

      PStackFree(db_args);
      PStackFree(hd_args);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: mk_prim_enum_inst()
//
//   Create an instance of clause and set the proof object
//   for primitive enumeration.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void mk_prim_enum_inst(ClauseSet_p store, Clause_p cl, Term_p var, Term_p target)
{
   assert(!var->binding);
   assert(var->type == target->type);

   var->binding = target;

   Eqn_p res_lits = EqnListCopyOpt(cl->literals);
   EqnListLambdaNormalize(res_lits);
   EqnListRemoveResolved(&res_lits);
   EqnListRemoveDuplicates(res_lits);
   Clause_p res = ClauseAlloc(res_lits);
   NormalizeEquations(res);
   store_result(res, cl, NULL, store, DCPrimEnum, 1);
   BooleanSimplification(res);

   var->binding = NULL;
}

/*-----------------------------------------------------------------------
//
// Function: remove_constant_args()
//
//   For each variable in var_occs, mark the indexes of arguments that
//   always occur with the same value.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void remove_constant_args(PObjMap_p* var_occs, PObjMap_p* var_removed_args)
{
   PStack_p iter = PStackAlloc();
   PObjMapTraverseInit(*var_occs, iter);
   Term_p var;
   PStack_p occs;

   while((occs = PObjMapTraverseNext(iter, (void**)&var)))
   {
      PStack_p already_removed =
         PObjMapFind(var_removed_args, var, PCmpFun);
      int num_args = TypeGetMaxArity(var->type);
      assert(!PStackEmpty(occs));
      Term_p* first_occ = PStackElementP(occs, 0);
      for(int arg_idx=0; first_occ[arg_idx] && arg_idx<num_args; arg_idx++)
      {
         bool is_removable =
            TermIsDBClosed(first_occ[arg_idx]) &&
            !PStackFindInt(already_removed, arg_idx);

         for(long occ_idx=1; is_removable && occ_idx<PStackGetSP(occs); occ_idx++)
         {
            Term_p* next_occ = PStackElementP(occs, occ_idx);
            is_removable = next_occ[arg_idx] &&
                           next_occ[arg_idx] == first_occ[arg_idx];
         }

         if(is_removable)
         {
            PStackPushInt(already_removed, arg_idx);
         }
      }
   }

   PStackFree(iter);
}

/*-----------------------------------------------------------------------
//
// Function: remove_repeated_args()
//
//   For each variable in var_occs remove the argument with index i
//   if there is another argument with index j such that for each occurence
//   arugments at i and j are the same.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void remove_repeated_args(PObjMap_p* var_occs, PObjMap_p* var_removed_args)
{
   PStack_p iter = PStackAlloc();
   PObjMapTraverseInit(*var_occs, iter);
   Term_p var;
   PStack_p occs;

   while((occs = PObjMapTraverseNext(iter, (void**)&var)))
   {
      PStack_p already_removed =
         PObjMapFind(var_removed_args, var, PCmpFun);
      int num_args = TypeGetMaxArity(var->type);
      assert(!PStackEmpty(occs));

      Term_p* first_occ = PStackElementP(occs, 0);
      for(int arg_i=0; first_occ[arg_i] && arg_i<num_args; arg_i++)
      {
         bool is_removable=false;
         for(int arg_j=arg_i+1;
             !is_removable && first_occ[arg_j] && arg_j<num_args;
             arg_j++)
         {
            is_removable = !PStackFindInt(already_removed, arg_i) &&
                           !PStackFindInt(already_removed, arg_j) &&
                           first_occ[arg_i] == first_occ[arg_j];
            if(is_removable)
            {
               // testing if we can remove i-th argument
               for(PStackPointer occ_idx=1;
                   is_removable && occ_idx<PStackGetSP(occs);
                   occ_idx++)
               {
                  Term_p* next_occ = PStackElementP(occs, occ_idx);
                  is_removable = next_occ[arg_i] && next_occ[arg_j] &&
                                 next_occ[arg_i] == next_occ[arg_j];
               }
            }
         }

         if(is_removable)
         {
            PStackPushInt(already_removed, arg_i);
         }
      }
   }
   PStackFree(iter);
}

/*-----------------------------------------------------------------------
//
// Function: compute_removal_subst()
//
//   Based on data in var_removed_args (containing indexes of arguments to be
//   removed), create a substitution removing all the arguments. Returns
//   true if at least one argument is removed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool compute_removal_subst(PObjMap_p* var_removed_args, Subst_p subst,
                           TB_p bank)
{
   PStack_p iter = PStackAlloc();
   PObjMapTraverseInit(*var_removed_args, iter);
   Term_p var;
   PStack_p occs;
   bool removed = false;

   while((occs = PObjMapTraverseNext(iter, (void**)&var)))
   {
      if(!PStackEmpty(occs))
      {
         assert(TypeIsArrow(var->type));
         int max_args = TypeGetMaxArity(var->type);
         PStack_p new_db_vars = PStackAlloc();
         for(int i=0; i<max_args; i++)
         {
            if(!PStackFindInt(occs, i))
            {
               PStackPushP(new_db_vars,
                  TBRequestDBVar(bank, var->type->args[i], max_args-i-1));
            }
         }

         Type_p arg_tys[PStackGetSP(new_db_vars)];
         for(PStackPointer i=0; i<PStackGetSP(new_db_vars); i++)
         {
            arg_tys[i] = ((Term_p)PStackElementP(new_db_vars,i))->type;
         }
         Type_p ty =
            TypeBankInsertTypeShared(bank->sig->type_bank,
               ArrowTypeFlattened(arg_tys, PStackGetSP(new_db_vars),
                                  var->type->args[var->type->arity-1]));
         Term_p fresh_var = TBInsert(bank, VarBankGetFreshVar(bank->vars, ty), DEREF_NEVER);
         Term_p matrix = ApplyTerms(bank, fresh_var, new_db_vars);
         Term_p closed =
            CloseWithTypePrefix(bank, var->type->args, var->type->arity-1, matrix);
         SubstAddBinding(subst, var, closed);
         removed = true;
         PStackFree(new_db_vars);
      }
   }
   PStackFree(iter);
   return removed;
}

/*-----------------------------------------------------------------------
//
// Function: find_disagreements()
//
//   Stores the computed inference with the given derivation code
//   in the temporary store for the newly infered clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int prim_enum_var(ClauseSet_p store, Clause_p cl, PrimEnumMode mode, Term_p app_var)
{
   assert(TermIsAppliedFreeVar(app_var));
   assert(TypeIsBool(app_var->type));
   int generated_cls = 0;
   TB_p bank = cl->literals->bank;
   Sig_p sig = bank->sig;

   if(mode == NegMode || mode == FullMode)
   {
      Term_p neg_matrix =
         TFormulaFCodeAlloc(bank, sig->not_code,
                            fresh_pattern(bank, app_var), NULL);
      mk_prim_enum_inst(store, cl, app_var->args[0],
                        close_for_appvar(bank, app_var, neg_matrix));
      generated_cls++;
   }
   if(mode == AndMode || mode == FullMode)
   {
      Term_p and_matrix =
         TFormulaFCodeAlloc(bank, sig->and_code,
                            fresh_pattern(bank, app_var),
                            fresh_pattern(bank, app_var));
      mk_prim_enum_inst(store, cl, app_var->args[0],
                        close_for_appvar(bank, app_var, and_matrix));
      generated_cls++;
   }
   if(mode == OrMode || mode == FullMode)
   {
      Term_p or_matrix =
         TFormulaFCodeAlloc(bank, sig->or_code,
                            fresh_pattern(bank, app_var),
                            fresh_pattern(bank, app_var));
      mk_prim_enum_inst(store, cl, app_var->args[0],
                        close_for_appvar(bank, app_var, or_matrix));
      generated_cls++;
   }
   if(mode == EqMode || mode == FullMode)
   {
      PTree_p ret_types = NULL;
      for(int i=1; i<app_var->arity; i++)
      {
         PTreeStore(&ret_types, app_var->args[i]->type);
      }

      PStack_p tys = PTreeTraverseInit(ret_types);
      PTree_p next = NULL;
      while((next=PTreeTraverseNext(tys)))
      {
         Type_p ret_ty = next->key;
         Term_p eq_matrix =
            TFormulaFCodeAlloc(bank,
                               TypeIsBool(ret_ty) ? sig->equiv_code : sig->eqn_code,
                               fresh_pattern_w_ty(bank, app_var, ret_ty),
                               fresh_pattern_w_ty(bank, app_var, ret_ty));
         mk_prim_enum_inst(store, cl, app_var->args[0],
                           close_for_appvar(bank, app_var, eq_matrix));
         generated_cls++;
      }
      PTreeTraverseExit(tys);
      PTreeFree(ret_types);


   }
   mk_prim_enum_inst(store, cl, app_var->args[0],
                      close_for_appvar(bank, app_var, bank->true_term));
   mk_prim_enum_inst(store, cl, app_var->args[0],
                      close_for_appvar(bank, app_var, bank->false_term));
   generated_cls += 2;
   if(mode == PragmaticMode)
   {
      for(int i=1; i < app_var->arity; i++)
      {
         for(int j=i+1; j < app_var->arity; j++)
         {
            if(app_var->args[i]->type == app_var->args[j]->type)
            {
               Type_p ty = app_var->args[i]->type;
               Term_p db_i = TBRequestDBVar(bank, ty, app_var->arity-i-1);
               Term_p db_j = TBRequestDBVar(bank, ty, app_var->arity-j-1);
               FunCode pos_code = TypeIsBool(ty) ? sig->equiv_code : sig->eqn_code;
               FunCode neg_code = TypeIsBool(ty) ? sig->xor_code : sig->neqn_code;
               Term_p eq_matrix = TFormulaFCodeAlloc(bank, pos_code, db_i, db_j);
               Term_p neq_matrix = TFormulaFCodeAlloc(bank, neg_code, db_i, db_j);
               mk_prim_enum_inst(store, cl, app_var->args[0],
                                 close_for_appvar(bank, app_var, eq_matrix));
               mk_prim_enum_inst(store, cl, app_var->args[0],
                                 close_for_appvar(bank, app_var, neq_matrix));
               generated_cls += 2;

               if(TypeIsPredicate(ty))
               {
                  Term_p proj_i = apply_pattern_vars(bank, db_i, app_var);
                  Term_p proj_j = apply_pattern_vars(bank, db_j, app_var);
                  Term_p and_matrix =
                     TFormulaFCodeAlloc(bank, sig->and_code, proj_i, proj_j);
                  Term_p or_matrix =
                     TFormulaFCodeAlloc(bank, sig->or_code, proj_i, proj_j);
                  mk_prim_enum_inst(store, cl, app_var->args[0],
                                    close_for_appvar(bank, app_var, and_matrix));
                  mk_prim_enum_inst(store, cl, app_var->args[0],
                                    close_for_appvar(bank, app_var, or_matrix));
                  generated_cls += 2;
               }
            }
         }
      }
   }
   if(mode == LogSymbolMode || mode == PragmaticMode)
   {
      Type_p var_ty = app_var->args[0]->type;
      if(TypeIsArrow(var_ty))
      {
         if(var_ty->arity == 2 &&
            TypeIsBool(var_ty->args[0]) && TypeIsBool(var_ty->args[1]))
         {
            Term_p not_matrix = TermTopAlloc(sig->not_code, 0);
            not_matrix->type = var_ty;
            mk_prim_enum_inst(store, cl, app_var->args[0],
                              TBTermTopInsert(bank, not_matrix));
            generated_cls++;
         }
         else if(var_ty->arity == 3 &&
                 TypeIsBool(var_ty->args[0]) && TypeIsBool(var_ty->args[1])
                 && TypeIsBool(var_ty->args[2]))
         {
            Term_p and_matrix = TermTopAlloc(sig->and_code, 0);
            and_matrix->type = var_ty;
            Term_p or_matrix = TermTopAlloc(sig->or_code, 0);
            or_matrix->type = var_ty;
            mk_prim_enum_inst(store, cl, app_var->args[0],
                              TBTermTopInsert(bank, and_matrix));
            mk_prim_enum_inst(store, cl, app_var->args[0],
                              TBTermTopInsert(bank, or_matrix));
            generated_cls += 2;
         }

         if(var_ty->arity == 3 &&
            var_ty->args[0] == var_ty->args[1] && TypeIsBool(var_ty->args[2]))
         {
            FunCode pos_code =
               TypeIsBool(var_ty->args[0]) ? sig->equiv_code : sig->eqn_code;
            Term_p eqn_matrix = TermTopAlloc(pos_code, 0);
            eqn_matrix->type = var_ty;
            FunCode neg_code =
               TypeIsBool(var_ty->args[0]) ? sig->xor_code : sig->neqn_code;
            Term_p neqn_matrix = TermTopAlloc(neg_code, 0);
            neqn_matrix->type = var_ty;
            mk_prim_enum_inst(store, cl, app_var->args[0],
                              TBTermTopInsert(bank, eqn_matrix));
            mk_prim_enum_inst(store, cl, app_var->args[0],
                              TBTermTopInsert(bank, neqn_matrix));
            generated_cls += 2;
         }
         if(var_ty->arity == 2 &&
            TypeIsBool(var_ty->args[1]) &&
            TypeIsArrow(var_ty->args[0]) &&
            var_ty->args[0]->arity == 2 &&
            TypeIsBool(var_ty->args[0]->args[1]))
         {
            Term_p db_var = TBRequestDBVar(bank, var_ty->args[0], 0);
            Term_p all_matrix = TermTopAlloc(sig->qall_code, 1);
            all_matrix->args[0] = db_var;
            all_matrix->type = sig->type_bank->bool_type;
            all_matrix = TBTermTopInsert(bank, all_matrix);
            Term_p ex_matrix = TermTopAlloc(sig->qex_code, 1);
            ex_matrix->args[0] = db_var;
            ex_matrix->type = sig->type_bank->bool_type;
            ex_matrix = TBTermTopInsert(bank, ex_matrix);
            mk_prim_enum_inst(store, cl, app_var->args[0],
                              CloseWithDBVar(bank, var_ty->args[0], all_matrix));
            mk_prim_enum_inst(store, cl, app_var->args[0],
                              CloseWithDBVar(bank, var_ty->args[0], ex_matrix));
            generated_cls += 2;
         }
      }
   }
   return generated_cls;
}


/*-----------------------------------------------------------------------
//
// Function: find_disagreements()
//
//   Stores the computed inference with the given derivation code
//   in the temporary store for the newly infered clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool find_disagreements(Sig_p sig, Term_p t, Term_p s, PStack_p diss_stack)
{
   if (t->type != s->type || t == s)
   {
      return false;
   }

   PStackPointer begin = PStackGetSP(diss_stack);

   PStack_p tasks = PStackAlloc();
   PStackPushP(tasks, t);
   PStackPushP(tasks, s);
   bool exists_elig = false;

   while (!PStackEmpty(tasks))
   {
      s = PStackPopP(tasks);
      t = PStackPopP(tasks);

      if (s != t)
      {
         if (!TermIsPhonyApp(s) && !TermIsPhonyApp(t) &&
             !TermIsLambda(s) && !TermIsLambda(t) &&
             s->f_code == t->f_code &&
             (!SigIsPolymorphic(sig, s->f_code) || s->arity == 0 ||
               s->args[0]->type == t->args[0]->type))
         {
            assert(s->arity == t->arity);
            for (int i = 0; i < t->arity; i++)
            {
               assert(t->args[i]->type == s->args[i]->type);
               PStackPushP(tasks, t->args[i]);
               PStackPushP(tasks, s->args[i]);
            }
         }
         else
         {
            PStackPushP(diss_stack, t);
            PStackPushP(diss_stack, s);
            exists_elig = exists_elig ||
                          (TYPE_EXT_ELIGIBLE(s->type)
                           && !TermIsFreeVar(s) && !TermIsFreeVar(t));
         }
      }
   }

   if (!exists_elig)
   {
      while (PStackGetSP(diss_stack) != begin)
      {
         PStackDiscardTop(diss_stack);
      }
   }
   PStackFree(tasks);
   return exists_elig;
}

/*-----------------------------------------------------------------------
//
// Function: advance_eq_fact_pos()
//
//   Given an *initialized* clause position pos, find the next one which
//   can take part in ExtEqFact inference
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool advance_eq_fact_pos(ClausePos_p pos)
{
   bool advanced = false;
   while(pos->literal && !advanced)
   {
      if(EqnIsPositive(pos->literal))
      {
         if(pos->side == NoSide && TermHasExtEligSubterm(pos->literal->lterm))
         {
            pos->side = LeftSide;
            advanced = true;
         }
         else if(pos->side == LeftSide && TermHasExtEligSubterm(pos->literal->rterm))
         {
            pos->side = RightSide;
            advanced = true;
         }
      }

      if(!advanced)
      {
         pos->literal = pos->literal->next;
         pos->side = NoSide;
      }
   }
   return advanced;
}

/*-----------------------------------------------------------------------
//
// Function: do_ext_eq_fact()
//
//   Given an *initialized* clause position pos, find the next one which
//   can take part in ExtEqFact inference
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void do_ext_eq_fact(ClausePos_p main_pos, ClausePos_p partner_pos,
                    ClauseSet_p cl_store)
{
   assert(main_pos->clause == partner_pos->clause);
   assert(main_pos->literal);
   assert(partner_pos->literal);
   assert(main_pos->literal != partner_pos->literal);

   Term_p main_term  = ClausePosGetSide(main_pos);
   Term_p partner_term = ClausePosGetSide(partner_pos);
   TB_p bank = main_pos->literal->bank;

   PStack_p disagreements = PStackAlloc();
   if(find_disagreements(bank->sig, main_term, partner_term, disagreements))
   {
      Term_p main_other_term = ClausePosGetOtherSide(main_pos);
      Term_p partner_other_term = ClausePosGetOtherSide(partner_pos);
      Eqn_p new_cl_lits = EqnAlloc(main_other_term, partner_other_term,
                                   bank, false);
      while (!PStackEmpty(disagreements))
      {
         Eqn_p cond = EqnAlloc(PStackPopP(disagreements), PStackPopP(disagreements),
                               bank, false);
         cond->next = new_cl_lits;
         new_cl_lits = cond;
      }
      Eqn_p old_cl_lits =
        EqnListCopyOptExcept(main_pos->clause->literals, main_pos->literal);
      EqnListAppend(&new_cl_lits, old_cl_lits);
      EqnListRemoveResolved(&new_cl_lits);
      EqnListRemoveDuplicates(new_cl_lits);
      EqnListLambdaNormalize(new_cl_lits);
      Clause_p res = ClauseAlloc(new_cl_lits);
      res->proof_size = main_pos->clause->proof_size + 1;
      res->proof_depth = main_pos->clause->proof_depth + 1;
      ClauseSetProp(res, (ClauseGiveProps(main_pos->clause, CPIsSOS)));
      ClausePushDerivation(res, DCExtEqFact, main_pos->clause, NULL);
      ClauseSetInsert(cl_store, res);
   }

   PStackFree(disagreements);
}

/*-----------------------------------------------------------------------
//
// Function: do_ext_sup()
//
//   Performs ExtEqRes inference.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void do_ext_eqres(Clause_p cl, Eqn_p lit, ClauseSet_p store)
{
   PStack_p disagreements = PStackAlloc();
   Term_p lhs = lit->lterm, rhs = lit->rterm;
   TB_p terms = lit->bank;
   if (find_disagreements(terms->sig, lhs, rhs, disagreements))
   {
      Eqn_p condition = NULL;
      while (!PStackEmpty(disagreements))
      {
         Eqn_p cond = EqnAlloc(PStackPopP(disagreements),
                               PStackPopP(disagreements), terms, false);
         cond->next = condition;
         condition = cond;
      }
      Eqn_p rest = EqnListCopyOptExcept(cl->literals, lit);
      EqnListAppend(&condition, rest);
      EqnListRemoveResolved(&condition);
      EqnListRemoveDuplicates(condition);
      EqnListLambdaNormalize(condition);
      Clause_p res = ClauseAlloc(condition);
      store_result(res, cl, NULL, store, DCExtEqRes, 1);
   }

   PStackFree(disagreements);
}

/*-----------------------------------------------------------------------
//
// Function: do_ext_sup()
//
//   Performs ExtSup inference.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void do_ext_sup(ClausePos_p from_pos, ClausePos_p into_pos, ClauseSet_p store,
                TB_p terms, VarBank_p freshvars, Clause_p orig_cl)
{
   PStack_p disagreements = PStackAlloc();
   Term_p from_t = ClausePosGetSubterm(from_pos);
   Term_p into_t = ClausePosGetSubterm(into_pos);
   // avoiding  ext sup from positive literal into positive top position
   if (!(EqnIsPositive(from_pos->literal) && EqnIsPositive(into_pos->literal)
         && PStackEmpty(into_pos->pos)
         && (ClausePosGetOtherSide(from_pos) == ClausePosGetOtherSide(into_pos) ||
             // can be done in both directions
             from_pos->clause < into_pos->clause)) &&
       find_disagreements(terms->sig, from_t, into_t, disagreements))
   {
      Subst_p subst = SubstAlloc();
      VarBankResetVCounts(freshvars);
      NormSubstEqnList(from_pos->clause->literals, subst, freshvars);
      NormSubstEqnList(into_pos->clause->literals, subst, freshvars);

      Eqn_p condition = NULL;
      while (!PStackEmpty(disagreements))
      {
         Term_p lhs = TBInsertInstantiated(terms, PStackPopP(disagreements));
         Term_p rhs = TBInsertInstantiated(terms, PStackPopP(disagreements));
         assert(lhs->type == rhs->type);
         Eqn_p cond = EqnAlloc(lhs, rhs, terms, false);
         cond->next = condition;
         condition = cond;
      }

      Term_p from_rhs = ClausePosGetOtherSide(from_pos);
      Term_p into_rhs = ClausePosGetOtherSide(into_pos);
      Term_p new_lhs = TBTermPosReplace(terms, from_rhs, into_pos->pos,
                                        DEREF_ALWAYS, 0,
                                        ClausePosGetSubterm(into_pos));

      Term_p new_rhs = TBInsertOpt(terms, into_rhs, DEREF_ALWAYS);

      Eqn_p into_copy = EqnListCopyOptExcept(into_pos->clause->literals, into_pos->literal);
      Eqn_p from_copy = EqnListCopyOptExcept(from_pos->clause->literals, from_pos->literal);
      EqnListAppend(&condition, into_copy);
      EqnListAppend(&condition, from_copy);
      Eqn_p new_lit = EqnAlloc(new_lhs, new_rhs, terms, EqnIsPositive(into_pos->literal));
      EqnListAppend(&condition, new_lit);
      EqnListRemoveResolved(&condition);
      EqnListRemoveDuplicates(condition);

      EqnListLambdaNormalize(condition);
      Clause_p res = ClauseAlloc(condition);
      res->proof_size = into_pos->clause->proof_size + from_pos->clause->proof_size + 1;
      res->proof_depth = MAX(into_pos->clause->proof_depth, from_pos->clause->proof_depth) + 1;

      ClauseSetProp(res, (ClauseGiveProps(into_pos->clause, CPIsSOS) |
                          ClauseGiveProps(from_pos->clause, CPIsSOS)));
      if (!into_pos->clause->derivation)
      {
         assert(from_pos->clause->derivation);
         ClausePushDerivation(res, DCExtSup, orig_cl, from_pos->clause);
      }
      else
      {
         assert(!from_pos->clause->derivation);
         ClausePushDerivation(res, DCExtSup, into_pos->clause, orig_cl);
      }
      ClauseSetInsert(store, res);

      SubstDelete(subst);
   }
   PStackFree(disagreements);
}

/*-----------------------------------------------------------------------
//
// Function: do_ext_sup_from()
//
//   Performs ExtSup inferences with the given clause used as 'from' partner
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void do_ext_sup_from(Clause_p renamed_cl, Clause_p orig_cl, ProofState_p state)
{
   PStack_p from_pos_stack = PStackAlloc();
   CollectExtSupFromPos(renamed_cl, from_pos_stack);
   ClausePos_p from_pos = ClausePosAlloc(), into_pos = ClausePosAlloc();
#ifdef ENABLE_LFHO
   ExtIndex_p into_idx = state->gindices.ext_sup_into_index;
#else
   ExtIndex_p into_idx = NULL;
#endif

   while (!PStackEmpty(from_pos_stack))
   {
      CompactPos cpos_from = PStackPopInt(from_pos_stack);
      UnpackClausePosInto(cpos_from, renamed_cl, from_pos);

      FunCode fc = PStackPopInt(from_pos_stack);
      ClauseTPosTree_p into_partners = IntMapGetVal(into_idx, fc);

      PStack_p iter = PTreeTraverseInit(into_partners);
      PTree_p node = NULL;
      while ((node = PTreeTraverseNext(iter)))
      {
         ClauseTPos_p cl_cpos = node->key;
         PStack_p niter = NumTreeTraverseInit(cl_cpos->pos);
         NumTree_p node;
         while ((node = NumTreeTraverseNext(niter)))
         {
            UnpackClausePosInto(node->key, cl_cpos->clause, into_pos);
            do_ext_sup(from_pos, into_pos, state->tmp_store,
                       state->terms, state->freshvars, orig_cl);
         }
         NumXTreeTraverseExit(niter);
      }
      PTreeTraverseExit(iter);
   }

   ClausePosFree(from_pos);
   ClausePosFree(into_pos);
   PStackFree(from_pos_stack);
}

/*-----------------------------------------------------------------------
//
// Function: do_ext_sup_into()
//
//   Performs ExtSup inferences with the given clause used as 'intos' partner
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void do_ext_sup_into(Clause_p renamed_cl, Clause_p orig_cl, ProofState_p state)
{
   PStack_p into_pos_stack = PStackAlloc();
   CollectExtSupIntoPos(renamed_cl, into_pos_stack);
   ClausePos_p from_pos = ClausePosAlloc(), into_pos = ClausePosAlloc();
#ifdef ENABLE_LFHO
   ExtIndex_p from_idx = state->gindices.ext_sup_from_index;
#else
   ExtIndex_p from_idx = NULL;
#endif

   while (!PStackEmpty(into_pos_stack))
   {
      CompactPos cpos_into = PStackPopInt(into_pos_stack);
      UnpackClausePosInto(cpos_into, renamed_cl, into_pos);

      FunCode fc = PStackPopInt(into_pos_stack);
      // assert(fc > state->signature->internal_symbols);
      ClauseTPosTree_p from_partners = IntMapGetVal(from_idx, fc);

      PStack_p iter = PTreeTraverseInit(from_partners);
      PTree_p node = NULL;
      while ((node = PTreeTraverseNext(iter)))
      {
         ClauseTPos_p cl_cpos = node->key;
         PStack_p niter = NumTreeTraverseInit(cl_cpos->pos);
         NumTree_p node;
         while ((node = NumTreeTraverseNext(niter)))
         {

            UnpackClausePosInto(node->key, cl_cpos->clause, from_pos);
            do_ext_sup(from_pos, into_pos, state->tmp_store,
                       state->terms, state->freshvars, orig_cl);
         }
         NumXTreeTraverseExit(niter);
      }
      PTreeTraverseExit(iter);
   }

   ClausePosFree(from_pos);
   ClausePosFree(into_pos);
   PStackFree(into_pos_stack);
}

/*-----------------------------------------------------------------------
//
// Function: find_choice_triggers()
//
//  Find subterms of t that are of the form ch(s) where ch is in choice_syms.
//  Store the subterm on stack triggers.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void find_choice_triggers(IntMap_p choice_syms, PStack_p triggers, Term_p t)
{
   if(!TermIsDBVar(t) && !TermIsLambda(t))
   {
      if(t->arity == 1 && IntMapGetVal(choice_syms, t->f_code) &&
         !TermIsFreeVar(t->args[0]))
      {
         PStackPushP(triggers, t);
      }
      else if(TermIsAppliedFreeVar(t) && t->arity == 2)
      {
         Term_p var = t->args[0];
         Type_p ty = var->type;
         if(TypeIsArrow(ty) && ty->arity>=2 &&
            (TypeIsArrow(ty->args[0]) && ty->args[0]->arity == 2 &&
             TypeIsPredicate(ty->args[0])))
         {
            // remaining checks if var type is (A -> o) -> A
            bool typechecks = true;
            Type_p a_type = ty->args[0]->args[0];
            if(TypeIsArrow(a_type) && a_type->arity == ty->arity-1)
            {
               for(int i=0; typechecks && i< a_type->arity; i++)
               {
                  typechecks = typechecks && a_type->args[i] == ty->args[i+1];
               }
            }
            else if (!TypeIsArrow(a_type) && ty->arity == 2)
            {
               typechecks = a_type == ty->args[1];
            }
            else
            {
               typechecks = false;
            }

            if(typechecks)
            {
               PStackPushP(triggers, t);
            }
         }
      }
      else if(!TermIsFreeVar(t) && t->arity)
      {
         for(int i=0; i<t->arity; i++)
         {
            find_choice_triggers(choice_syms, triggers, t->args[i]);
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: do_mk_choice_inst()
//
//  Given a term whose head is a defined choice symbol, instantiate
//  the corresponding choice axiom with the argument of the choice term.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void do_mk_choice_inst(ClauseSet_p store, IntMap_p choice_syms, Clause_p cl,
                       FunCode choice_code, Term_p trigger)
{
   assert(TypeIsArrow(trigger->type));
   assert(TypeIsPredicate(trigger->type));
   assert(IntMapGetVal(choice_syms, choice_code));

   Clause_p choice_def = IntMapGetVal(choice_syms, choice_code);
#ifndef NDEBUG
   PTree_p cl_vars = NULL, trig_vars = NULL;
   ClauseCollectVariables(choice_def, &cl_vars);
   TFormulaCollectFreeVars(choice_def->literals->bank, trigger, &trig_vars);
   PTree_p intersection = PTreeIntersection(cl_vars, trig_vars);
   assert(intersection == NULL);
   PTreeFree(cl_vars);
   PTreeFree(trig_vars);
   PTreeFree(intersection);
#endif
   Eqn_p neg_lit =
      EqnIsNegative(choice_def->literals)
         ? choice_def->literals : choice_def->literals->next;
   assert(TermIsAppliedFreeVar(neg_lit->lterm));

   Term_p var = neg_lit->lterm->args[0];
   assert(!var->binding);
   var->binding = trigger;

   Eqn_p res_lits = EqnListCopyOpt(choice_def->literals);
   EqnListLambdaNormalize(res_lits);
   EqnListRemoveResolved(&res_lits);
   EqnListRemoveDuplicates(res_lits);
   Clause_p res = ClauseAlloc(res_lits);
   NormalizeEquations(res);
   store_result(res, cl, choice_def, store, DCChoiceInst, 1);
   BooleanSimplification(res);

   var->binding = NULL;
}

/*-----------------------------------------------------------------------
//
// Function: inst_choice()
//
//  Instantiate choice axiom for choice code with trigger and its negation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int inst_choice(ClauseSet_p store, IntMap_p choice_syms, Clause_p cl,
                 FunCode choice_code, Term_p trigger)
{
   assert(cl->literals);
   TB_p bank = cl->literals->bank;
   do_mk_choice_inst(store, choice_syms, cl, choice_code, trigger);

   Term_p neg_trigger =
      TermTopCopy(LambdaEtaExpandDBTopLevel(bank, trigger));
   assert(TermIsLambda(neg_trigger));
   assert(TypeIsBool(neg_trigger->args[1]->type));
   neg_trigger->args[1] =
      TFormulaFCodeAlloc(bank, bank->sig->not_code, neg_trigger->args[1], NULL);
   do_mk_choice_inst(store, choice_syms, cl,
                     choice_code, TBTermTopInsert(bank, neg_trigger));
   return 2;
}

/*-----------------------------------------------------------------------
//
// Function: mk_new_choice()
//
//  Given a type ty create a new clause representing choice axiom.
//  Store the new clause in choice syms map and in the archive.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode mk_new_choice(TB_p bank, ClauseSet_p archive, IntMap_p choice_syms, Type_p ty)
{
   assert(TypeIsArrow(ty));
   assert(ty->arity >= 2);
   Type_p a_to_o = ty->args[0]; // ty is of the form (a -> o) -> a
   PStack_p vars = PStackAlloc();
   Term_p ch_const = TermAllocNewSkolem(bank->sig, vars, ty);
   ch_const = TBTermTopInsert(bank, ch_const);
   PStackFree(vars);

   Term_p p_var = VarBankGetFreshVar(bank->vars, a_to_o);

   Term_p ch_p =
      TBTermTopInsert(bank, TermApplyArg(bank->sig->type_bank, ch_const, p_var));
   Term_p p_ch_p =
      TBTermTopInsert(bank, TermApplyArg(bank->sig->type_bank, p_var, ch_p));

   assert(TypeIsArrow(a_to_o));
   assert(a_to_o->arity == 2);
   Type_p a = ty->args[0]->args[0];

   Term_p x_var = VarBankGetFreshVar(bank->vars, a);
   Term_p p_x =
      TBTermTopInsert(bank, TermApplyArg(bank->sig->type_bank, p_var, x_var));

   Eqn_p not_p_x_lit = EqnAlloc(p_x, bank->true_term, bank, false);
   Eqn_p p_ch_p_lit = EqnAlloc(p_ch_p, bank->true_term, bank, true);

   not_p_x_lit->next = p_ch_p_lit;
   Clause_p res = ClauseAlloc(not_p_x_lit);
   // TODO: Clause documentation is not implemented at the moment.
   // DocClauseCreationDefault(clause, inf_efactor, clause, NULL);
   ClausePushDerivation(res, DCChoiceAx, NULL, NULL);
   ClauseSetInsert(archive, res);

   assert(!IntMapGetVal(choice_syms, ch_const->f_code));

   IntMapAssign(choice_syms, ch_const->f_code, res);

   return ch_const->f_code;
}

/*-----------------------------------------------------------------------
//
// Function: term_drop_last_arg()
//
//   Removes the last argument of a term. Assumes there is at least
//   one argument.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p term_drop_last_arg(TypeBank_p tb, Term_p s)
{
   assert(s->arity);

   int needed_args = TypeGetMaxArity(s->type);
   Type_p *args = TypeArgArrayAlloc(needed_args + 2);
   args[0] = s->args[s->arity - 1]->type;
   if (UNLIKELY(TypeIsArrow(s->type)))
   {
      for (int i = 0; i < s->type->arity; i++)
      {
         args[i + 1] = s->type->args[i];
      }
   }
   else
   {
      args[1] = s->type;
   }

   Type_p res_type = TypeBankInsertTypeShared(tb, AllocArrowType(needed_args + 2, args));

   if (TermIsPhonyApp(s) && s->arity == 2)
   {
      Term_p t = s->args[0];
      assert(t->type == res_type);
      // assert(TermIsFreeVar(t));
      return t;
   }
   else
   {
      Term_p t = TermTopAlloc(s->f_code, s->arity - 1);
      t->type = res_type;
      for (int i = 0; i < s->arity - 1; i++)
      {
         t->args[i] = s->args[i];
      }
      return t;
   }
}

/*-----------------------------------------------------------------------
//
// Function: mk_leibniz_instance()
//
//   Bind variable to binding and add the corresponding instance to
//   the proof state.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void mk_leibniz_instance(ClauseSet_p store, Clause_p cl,
                         Eqn_p lit, Term_p var, Term_p binding)
{
   assert(!var->binding);

   var->binding = binding;

   Eqn_p res_lits = EqnListCopyOptExcept(cl->literals, lit);
   EqnListLambdaNormalize(res_lits);
   EqnListRemoveResolved(&res_lits);
   EqnListRemoveDuplicates(res_lits);
   Clause_p res = ClauseAlloc(res_lits);
   NormalizeEquations(res);
   store_result(res, cl, NULL, store, DCLeibnizElim, 1);

   var->binding = NULL;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ComputeNegExt()
//
//   Computes all possible NegExt inferences with the given clause.
//   NegExt is described by
//
//                         s != t  \/ C
//    -----------------------------------------------------------
//      s (sk (free_vars(s,t))) != t (sk (free_vars(s,t))) \/ C
//
//   where s != t is a maximal literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ComputeNegExt(ProofState_p state, ProofControl_p control, Clause_p clause)
{
   for (Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      Type_p lit_type = lit->lterm->type;
      int needed_args = TypeGetMaxArity(lit_type);
      bool lit_filter =
          control->heuristic_parms.neg_ext == AllLits ||
          (control->heuristic_parms.neg_ext == MaxLits && EqnIsMaximal(lit));

      if (EqnIsNegative(lit) && lit_filter && needed_args > 0)
      {
         PTree_p free_var_tree = NULL;
         UNUSED(EqnCollectVariables(lit, &free_var_tree));

         PStack_p free_vars_stack = PStackAlloc();
         PTreeToPStack(free_vars_stack, free_var_tree);

         int num_vars = PStackGetSP(free_vars_stack);
         Term_p *vars = TermArgTmpArrayAlloc(num_vars);
         Type_p *vars_types = TypeArgArrayAlloc(num_vars);
         for (int i = 0; i < num_vars; i++)
         {
            vars[i] = PStackElementP(free_vars_stack, i);
            vars_types[i] = vars[i]->type;
         }

         Term_p new_lhs = lit->lterm, new_rhs = lit->rterm;
         for (int i = 0; i < needed_args; i++)
         {
            Term_p skolem =
                TermTopAlloc(
                    SigGetNewTypedSkolem(state->signature, vars_types,
                                         num_vars, lit_type->args[i]),
                    num_vars);
            for (int j = 0; j < num_vars; j++)
            {
               skolem->args[j] = vars[j];
            }
            skolem->type = lit_type->args[i];
            skolem = TBTermTopInsert(state->terms, skolem);
            new_lhs = TBTermTopInsert(state->terms,
                                      TermApplyArg(state->signature->type_bank, new_lhs, skolem));
            new_rhs = TBTermTopInsert(state->terms,
                                      TermApplyArg(state->signature->type_bank, new_rhs, skolem));

            Eqn_p new_lit = EqnAlloc(TBInsertNoProps(state->terms, new_lhs, DEREF_ALWAYS),
                                     TBInsertNoProps(state->terms, new_rhs, DEREF_ALWAYS),
                                     state->terms, false);
            Eqn_p new_literals = EqnListCopyExcept(clause->literals, lit, state->terms);
            EqnListInsertFirst(&new_literals, new_lit);

            EqnListLambdaNormalize(new_literals);
            Clause_p new_clause = ClauseAlloc(new_literals);
            state->neg_ext_count++;
            store_result(new_clause, clause, NULL, state->tmp_store, DCNegExt, 0);
         }

         TypeArgArrayFree(vars_types, num_vars);
         TermArgTmpArrayFree(vars, num_vars);
         PStackFree(free_vars_stack);
         PTreeFree(free_var_tree);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: ComputeArgCong()
//
//   Computes all possible ArgCong inferences with the given clause.
//   ArgCong is described by
//
//                         s = t  \/ C
//    -----------------------------------------------------------
//               s FRESH_VAR = t FRESH_VAR \/ C
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ComputeArgCong(ProofState_p state, ProofControl_p control, Clause_p clause)
{
   TB_p bank = state->terms;
   VarBank_p varbank = bank->vars;
   for (Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      Type_p lit_type = lit->lterm->type;
      int needed_args = TypeGetMaxArity(lit_type);
      bool lit_filter =
          EqnIsPositive(lit) && needed_args > 0 &&
          (control->heuristic_parms.arg_cong == AllLits ||
           (control->heuristic_parms.arg_cong == MaxLits && EqnIsMaximal(lit)));

      if (lit_filter)
      {
         PStack_p fresh_vars = PStackAlloc();
         Term_p lhs = lit->lterm, rhs = lit->rterm;
         for (int i = 0; i < needed_args; i++)
         {
            Term_p fvar =
               TBInsert(bank, VarBankGetFreshVar(varbank, lhs->type->args[i]), DEREF_NEVER);
            PStackPushP(fresh_vars, fvar);

            Term_p new_lhs = ApplyTerms(bank, lhs, fresh_vars),
                   new_rhs = ApplyTerms(bank, rhs, fresh_vars);
            assert(new_lhs->type == new_rhs->type);
            Eqn_p new_lit = EqnAlloc(new_lhs, new_rhs, bank, true);
            Eqn_p new_literals = EqnListCopyExcept(clause->literals, lit, bank);
            EqnListInsertFirst(&new_literals, new_lit);

            EqnListLambdaNormalize(new_literals);
            Clause_p new_clause = ClauseAlloc(new_literals);
            store_result(new_clause, clause, NULL, state->tmp_store, DCArgCong, 0);
         }
         PStackFree(fresh_vars);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: ComputePosExt()
//
//   Computes all possible PosExt inferences with the given clause.
//   PosExt is described by
//
//      s X = t X  \/ C
//    ---------------------
//        s = t  \/ C
//
//   where s = t is a maximal literal and X does not appear in
//   s and t and C.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ComputePosExt(ProofState_p state, ProofControl_p control, Clause_p clause)
{
   TypeBank_p tb = state->type_bank;
   for (Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      bool lit_filter =
          control->heuristic_parms.pos_ext == AllLits ||
          (control->heuristic_parms.pos_ext == MaxLits && EqnIsStrictlyMaximal(lit));
      if (EqnIsPositive(lit) && EqnIsEquLit(lit) && lit_filter)
      {
         Term_p lhs = lit->lterm, rhs = lit->rterm;

         while (lhs->arity && rhs->arity &&
                !TermIsLambda(lhs) && !TermIsLambda(rhs) &&
                lhs->args[lhs->arity - 1] == rhs->args[rhs->arity - 1] &&
                TermIsFreeVar(lhs->args[lhs->arity - 1]))
         {
            Term_p var = lhs->args[lhs->arity - 1];

            bool occurs = false;
            /* Checking if var appears in any of the arguments */
            for (int i = 0; !occurs && i < lhs->arity - 1; i++)
            {
               occurs = TermHasFCode(lhs->args[i], var->f_code);
            }
            for (int i = 0; !occurs && i < rhs->arity - 1; i++)
            {
               occurs = TermHasFCode(rhs->args[i], var->f_code);
            }
            /* Checking if var appears in other literals */
            for (Eqn_p iter = clause->literals; !occurs && iter; iter = iter->next)
            {
               occurs = iter != lit &&
                        (TermHasFCode(iter->lterm, var->f_code) ||
                         TermHasFCode(iter->rterm, var->f_code));
            }

            if (!occurs)
            {
               // if occurs checked passed, we can construct PosExt inference result
               lhs = term_drop_last_arg(tb, lhs);
               rhs = term_drop_last_arg(tb, rhs);

               if (!TermIsFreeVar(lhs))
               {
                  lhs = TBTermTopInsert(state->terms, lhs);
               }
               if (!TermIsFreeVar(rhs))
               {
                  // DBG_PRINT(stderr, "inserting: ", TermPrintDbg(stderr, rhs, state->terms->sig, DEREF_NEVER), ".\n");
                  rhs = TBTermTopInsert(state->terms, rhs);
               }

               Eqn_p new_lit = EqnAlloc(lhs, rhs, state->terms, true);
               Eqn_p new_literals = EqnListCopyExcept(clause->literals, lit, state->terms);
               EqnListInsertFirst(&new_literals, new_lit);

               Clause_p new_clause = ClauseAlloc(new_literals);
               store_result(new_clause, clause, NULL, state->tmp_store, DCPosExt, 0);
            }
            else
            {
               break;
            }
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: InferInjectiveDefinition()
//
//   If clause postulates injectivity of some symbol
//   add the definition of inverse to the proof state.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void InferInjectiveDefinition(ProofState_p state, ProofControl_p control, Clause_p clause)
{
   Clause_p res = ClauseRecognizeInjectivity(state->terms, clause);
   if (res)
   {
      assert(ClauseIsUnit(res));
      ClauseSetInsert(state->tmp_store, res);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ComputeExtSup()
//
//   Computes abstracting variant of superposition rule.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void ComputeExtSup(ProofState_p state, ProofControl_p control,
                   Clause_p renamed_cl, Clause_p orig_clause)
{
   if (orig_clause->proof_depth <= control->heuristic_parms.ext_rules_max_depth)
   {
      do_ext_sup_from(renamed_cl, orig_clause, state);
      do_ext_sup_into(renamed_cl, orig_clause, state);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ComputeExtEqRes()
//
//   Computes abstracting variant of equality resolution.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ComputeExtEqRes(ProofState_p state, ProofControl_p control, Clause_p cl)
{
   if (cl->proof_depth <= control->heuristic_parms.ext_rules_max_depth)
   {
      for (Eqn_p lit = cl->literals; lit; lit = lit->next)
      {
         if (EqnIsNegative(lit) && EqnIsEquLit(lit) &&
             !TypeIsArrow(lit->lterm) &&
             lit->lterm->f_code == lit->rterm->f_code &&
             !TermIsPhonyApp(lit->lterm) &&
             !TermIsDBVar(lit->lterm) && !TermIsDBVar(lit->rterm) &&
             TermHasExtEligSubterm(lit->lterm) &&
             TermHasExtEligSubterm(lit->rterm))
         {
            do_ext_eqres(cl, lit, state->tmp_store);
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: ComputeExtEqFact()
//
//   Computes abstracting variant of equality factoring.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ComputeExtEqFact(ProofState_p state, ProofControl_p control, Clause_p cl)
{
   if (cl->proof_depth <= control->heuristic_parms.ext_rules_max_depth)
   {
      ClausePos_p pos1 = ClausePosAlloc();
      ClausePos_p pos2 = ClausePosAlloc();
      pos1->clause = pos2->clause = cl;
      pos1->literal = cl->literals;
      pos1->side = NoSide;

      while(advance_eq_fact_pos(pos1))
      {
         pos2->literal = pos1->literal->next;
         pos2->side = NoSide;
         while(advance_eq_fact_pos(pos2))
         {
            do_ext_eq_fact(pos1, pos2, state->tmp_store);
         }
      }

      ClausePosFree(pos1);
      ClausePosFree(pos2);
   }
}

/*-----------------------------------------------------------------------
//
// Function: NormalizeEquations()
//
//   Lifts nested equalities to the literal equality level, and removes
//   nested $nots.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
bool NormalizeEquations(Clause_p cl)
{
   bool normalized = false;
   for (Eqn_p lit = cl->literals; lit; lit = lit->next)
   {
      TB_p bank = lit->bank;
      Sig_p sig = bank->sig;
      if(lit->lterm == bank->true_term && lit->rterm != bank->true_term)
      {
         SWAP(lit->lterm, lit->rterm);
         EqnDelProp(lit, EPIsEquLiteral);
         EqnDelProp(lit, EPMaxIsUpToDate);
         EqnDelProp(lit, EPIsOriented);
         normalized=true;
      }

      if (lit->rterm == bank->true_term &&
          (lit->lterm->f_code == sig->eqn_code ||
           lit->lterm->f_code == sig->neqn_code ||
           lit->lterm->f_code == sig->not_code))
      {
         bool negate = false;
         normalized = true;
         Term_p lterm = lit->lterm;
         while (lterm->f_code == bank->sig->not_code)
         {
            assert(lterm->arity == 1);
            negate = !negate;
            lterm = lterm->args[0];
         }

         if (lterm->f_code == bank->sig->eqn_code ||
             lterm->f_code == bank->sig->neqn_code)
         {
            lit->lterm = lterm->args[0];
            lit->rterm = lterm->args[1];
            if (lterm->f_code == bank->sig->neqn_code)
            {
               negate = !negate;
            }
         }
         else
         {
            lit->lterm = lterm;
         }

         if(lit->lterm == bank->false_term)
         {
            lit->lterm = bank->true_term;
            negate = !negate;
         }
         if(lit->rterm == bank->false_term)
         {
            lit->rterm = bank->true_term;
            negate = !negate;
         }
         if(lit->lterm == bank->true_term)
         {
            SWAP(lit->lterm, lit->rterm);
         }

         if(lit->rterm != bank->true_term)
         {
            EqnSetProp(lit, EPIsEquLiteral);
         }
         else
         {
            EqnDelProp(lit, EPIsEquLiteral);
         }

         if (negate)
         {
            EqnFlipProp(lit, EPIsPositive);
         }
         EqnDelProp(lit, EPMaxIsUpToDate);
         EqnDelProp(lit, EPIsOriented);
      }
   }

   if (normalized)
   {
      ClauseRecomputeLitCounts(cl);
      ClauseRemoveSuperfluousLiterals(cl);
      ClausePushDerivation(cl, DCNormalize, NULL, NULL);
   }

   return normalized;
}

/*-----------------------------------------------------------------------
//
// Function: ImmediateClausification()
//
//   Performs dynamic clausfication of equivalences.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
#define DEFAULT_RENAMING_LIMIT 24
bool ImmediateClausification(Clause_p cl, ClauseSet_p store, ClauseSet_p archive,
                             VarBank_p fresh_vars, bool fool_unroll)
{
   bool clausified = false;
   for (Eqn_p lit = cl->literals; !clausified && lit; lit = lit->next)
   {
      if (EqnIsClausifiable(lit))
      {
         TB_p bank = lit->bank;

         VarBankSetVCountsToUsed(bank->vars);

         WFormula_p wrapped = WFormulaOfClause(cl, bank);

         FormulaSet_p work_set = FormulaSetAlloc();
         FormulaSetInsert(work_set, wrapped);

         ClauseSet_p res_set = ClauseSetAlloc();
         FormulaSet_p archive = FormulaSetAlloc();

         if(fool_unroll)
         {
            WFormulaSetUnrollFOOL(work_set, archive, bank);
         }
         FormulaSetSimplify(work_set, bank, false);
         //printf(COMCHAR" TFormulaSetIntroduceDefs called from ImmediateClausification\n");
         TFormulaSetIntroduceDefs(work_set, archive, bank, DEFAULT_RENAMING_LIMIT);
         //printf(COMCHAR" TFormulaSetIntroduceDefs call from ImmediateClausification end\n");

         while (!FormulaSetEmpty(work_set))
         {
            WFormula_p handle = FormulaSetExtractFirst(work_set);
            DBGTermCheckUnownedSubterm(stdout,
                                       handle->tformula,
                                       "ImmediateClausification");
            WFormulaCNF2(handle, res_set, bank, fresh_vars, 100, fool_unroll);
            // low miniscope limit for efficiency
            WFormulaFree(handle);
         }

         FormulaSetFree(work_set);
         FormulaSetFree(archive);
         while (!ClauseSetEmpty(res_set))
         {
            Clause_p res = ClauseSetExtractFirst(res_set);
            EqnListMapTerms(res->literals, (TermMapper_p)LambdaNormalizeDB,
                            res->literals ? res->literals->bank : NULL);
            // DBG_PRINT(stderr, " > ", ClausePrintDBG(stderr, res), ".\n");
            PStackReset(res->derivation);
            store_result(res, cl, NULL, store, DCDynamicCNF, 0);
         }

         clausified = true;
         ClauseSetFree(res_set);
      }
   }

   if (clausified)
   {
      ClauseSetInsert(archive, cl);
   }

   return clausified;
}

/*-----------------------------------------------------------------------
//
// Function: EliminateLeibnizEquality()
//
//   Find a subclause of C of the form X sn | ~X tn and generate two
//   series of instances C{X |-> %xn. x_i != s_i} and
//   C{X |-> %xn. x_i = t_i}.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long EliminateLeibnizEquality(ClauseSet_p store, Clause_p cl, int limit)
{
   if(cl->proof_depth > limit)
   {
      return 0;
   }

   IntMap_p pos_vars = IntMapAlloc();
   IntMap_p neg_vars = IntMapAlloc();
   long num_eliminations = 0;


   for(Eqn_p lit = cl->literals; lit; lit = lit->next)
   {
      if(!EqnIsEquLit(lit) && TermIsAppliedFreeVar(lit->lterm))
      {
         assert(lit->rterm == lit->bank->true_term);
         IntMapAssign((EqnIsPositive(lit) ? pos_vars : neg_vars),
                      lit->lterm->args[0]->f_code, cl);
      }
   }

   for(Eqn_p lit = cl->literals; lit; lit = lit->next)
   {
      if(!EqnIsEquLit(lit) && TermIsAppliedFreeVar(lit->lterm))
      {
         bool found_opposite =
            IntMapGetVal(EqnIsPositive(lit) ? neg_vars : pos_vars,
                         lit->lterm->args[0]->f_code);
         if(found_opposite)
         {
            Sig_p sig = lit->bank->sig;

            Term_p var = lit->lterm->args[0];
            Term_p lhs = lit->lterm;
            assert(TermIsAppliedFreeVar(lhs));
            for(int i=1; i<lhs->arity; i++)
            {
               if(!OccurCheck(lhs->args[i], var))
               {
                  Term_p matrix =
                     TFormulaFCodeAlloc(
                        lit->bank,
                        EqnIsPositive(lit) ? sig->neqn_code : sig->eqn_code,
                        TBRequestDBVar(lit->bank, lhs->args[i]->type, lhs->arity-i-1),
                        lhs->args[i]);
                  Term_p res = matrix;
                  for(int i=lhs->arity-1; i>=1; i--)
                  {
                     res = CloseWithDBVar(lit->bank, lhs->args[i]->type, res);
                  }

                  assert(var->type == res->type);
                  mk_leibniz_instance(store, cl, lit, var, res);
                  num_eliminations++;
               }
            }
         }
      }
   }

   IntMapFree(pos_vars);
   IntMapFree(neg_vars);
   return num_eliminations;
}

/*-----------------------------------------------------------------------
//
// Function: PrimitiveEnumeration()
//
//   Instantiate clauses with primitive substitutions -- imitations
//   of logical symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PrimitiveEnumeration(ClauseSet_p store, Clause_p cl, PrimEnumMode mode, int limit)
{
   if(cl->proof_depth > limit)
   {
      return 0;
   }

   long new_cls = 0;
   IntMap_p processed_vars = IntMapAlloc();

   for(Eqn_p lit = cl->literals; lit; lit = lit->next)
   {
      if(TypeIsBool(lit->lterm->type))
      {
         if(TermIsAppliedFreeVar(lit->lterm)
            && !IntMapGetVal(processed_vars, lit->lterm->args[0]->f_code))
         {
            new_cls += prim_enum_var(store, cl, mode, lit->lterm);
            IntMapAssign(processed_vars, lit->lterm->args[0]->f_code, cl);
         }

         if(TermIsAppliedFreeVar(lit->rterm)
            && !IntMapGetVal(processed_vars, lit->rterm->args[0]->f_code))
         {
            new_cls += prim_enum_var(store, cl, mode, lit->rterm);
            IntMapAssign(processed_vars, lit->rterm->args[0]->f_code, cl);
         }
      }
   }

   IntMapFree(processed_vars);
   return new_cls;
}


/*-----------------------------------------------------------------------
//
// Function: BooleanSimplification()
//
//   Performs boolean simplification and returns true if formula becomes
//   redundant.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool BooleanSimplification(Clause_p cl)
{
   bool changed = false;
   bool is_tautology = false;
   for(Eqn_p lit = cl->literals; !is_tautology && lit; lit = lit->next)
   {
      Term_p old_lterm = lit->lterm, old_rterm = lit->rterm;
      EqnMap(lit, (TermMapper_p)TFormulaSimplifyDecoded, lit->bank);
      if(old_lterm != lit->lterm || old_rterm != lit->rterm)
      {
         changed = true;
      }
      if(EqnIsTrue(lit))
      {
         is_tautology = true;
      }
   }

   if(changed)
   {
      ClauseRecomputeLitCounts(cl);
      ClauseRemoveSuperfluousLiterals(cl);
      ClausePushDerivation(cl, DCNormalize, NULL, NULL);
   }

   return is_tautology;
}

/*-----------------------------------------------------------------------
//
// Function: ResolveFlexClause()
//
//   If a clause contains only negative disequations of the form
//   X @ s_n != Y @ t_n, derive the empty clause
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ResolveFlexClause(Clause_p cl)
{
   int pos = 1, neg = 0, in_eq = -1;
   IntMap_p ids_to_sign = IntMapAlloc();
   bool is_resolvable = true;

   for (Eqn_p lit = cl->literals; is_resolvable && lit; lit = lit->next)
   {
      if (EqnIsEquLit(lit))
      {
         is_resolvable = is_resolvable && EqnIsNegative(lit) && TermIsTopLevelFreeVar(lit->lterm) && TermIsTopLevelFreeVar(lit->rterm);
         if (is_resolvable && TypeIsPredicate(lit->lterm->type))
         {
            Term_p lvar = (TermIsFreeVar(lit->lterm) ? lit->lterm : lit->lterm->args[0]);
            Term_p rvar = (TermIsFreeVar(lit->rterm) ? lit->rterm : lit->rterm->args[0]);
            int *prev_l = IntMapGetVal(ids_to_sign, lvar->f_code);
            int *prev_r = IntMapGetVal(ids_to_sign, rvar->f_code);
            if (prev_l || prev_r)
            {
               // if variable occurrs both in predicate and eq lit --> cannot resolve
               is_resolvable = false;
            }
            else
            {
               *IntMapGetRef(ids_to_sign, lvar->f_code) = &in_eq;
               *IntMapGetRef(ids_to_sign, rvar->f_code) = &in_eq;
            }
         }
      }
      else
      {
         assert(lit->lterm != lit->bank->true_term);
         if (!TermIsTopLevelFreeVar(lit->lterm))
         {
            is_resolvable = false;
         }
         else
         {
            Term_p var = (TermIsFreeVar(lit->lterm) ? lit->lterm : lit->lterm->args[0]);
            int *prev_val = IntMapGetVal(ids_to_sign, var->f_code);
            if (!prev_val)
            {
               *IntMapGetRef(ids_to_sign, var->f_code) =
                   EqnIsPositive(lit) ? &pos : &neg;
            }
            else
            {
               is_resolvable = is_resolvable && (*prev_val == (EqnIsPositive(lit) ? 1 : 0));
            }
         }
      }
   }

   if (is_resolvable)
   {
      EqnListFree(cl->literals);
      cl->literals = NULL;
      ClauseRecomputeLitCounts(cl);
      ClausePushDerivation(cl, DCFlexResolve, NULL, NULL);
   }

   IntMapFree(ids_to_sign);
   return is_resolvable;
}

/*-----------------------------------------------------------------------
//
// Function: ClausePruneArgs
//
//   Recognize all choice axioms in set, insert them in the choice_syms,
//   and move the clauses to archive.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
#define TermArgAlloc(n) (SizeMalloc((n)*sizeof(Term_p)))
#define TermArgFree(arg, n) SizeFree(arg, (n)*sizeof(Term_p))

void free_var_occs(void* k, void* v)
{
   Term_p var = k;
   PStack_p occs = v;
   int size = TypeGetMaxArity(var->type);
#ifdef USE_SYSTEM_MEM
   UNUSED(size); // stiffling warnings;
#endif
   while(!PStackEmpty(occs))
   {
      Term_p* args = PStackPopP(occs);
      TermArgFree(args, size);
   }
   PStackFree(v);
}

void free_removed_args(void* k, void* v)
{
   PStackFree(v);
}

void ClausePruneArgs(Clause_p cl)
{
   if(ClauseIsEmpty(cl))
   {
      return;
   }

   PObjMap_p var_occs = NULL;
   PObjMap_p var_removed_args = NULL;
   PStack_p trav_stack = PStackAlloc();
   for(Eqn_p lit = cl->literals; lit; lit = lit->next)
   {
      PStackPushP(trav_stack, lit->lterm);
      PStackPushP(trav_stack, lit->rterm);

      while(!PStackEmpty(trav_stack))
      {
         Term_p t = PStackPopP(trav_stack);
         if(TermIsAppliedFreeVar(t) ||
            (TermIsFreeVar(t) && TypeIsArrow(t->type)))
         {
            Term_p var = TermIsAppliedFreeVar(t) ? t->args[0] : t;
            int max_args = TypeGetMaxArity(var->type);
            Term_p* args = TermArgAlloc(max_args);
            assert(!TermIsFreeVar(t) || t->arity == 0);
            for(int i=1; i<t->arity; i++)
            {
               args[i-1] = t->args[i];
            }
            for(int i=MAX(t->arity-1,0); i<max_args; i++)
            {
               args[i] = NULL;
            }
            PStack_p* occurrences = (PStack_p*)PObjMapGetRef(&var_occs, var, PCmpFun, NULL);
            if(!(*occurrences))
            {
               *occurrences = PStackAlloc();
               PStack_p* removed = (PStack_p*)PObjMapGetRef(&var_removed_args, var, PCmpFun, NULL);
               assert(!(*removed));
               *removed = PStackAlloc();
            }
            PStackPushP(*occurrences, args);
         }

         for(int i=(TermIsPhonyApp(t) ? 1 : 0); i<t->arity; i++)
         {
            if(!TermIsGround(t->args[i]))
            {
               PStackPushP(trav_stack, t->args[i]);
            }
         }
      }
   }

   remove_constant_args(&var_occs, &var_removed_args);
   remove_repeated_args(&var_occs, &var_removed_args);

   Subst_p subst = SubstAlloc();
   if(compute_removal_subst(&var_removed_args, subst, cl->literals->bank))
   {
      EqnListMapTerms(cl->literals, (TermMapper_p)TBInsertInstantiated, cl->literals->bank);
      EqnListLambdaNormalize(cl->literals);
      EqnListRemoveResolved(&cl->literals);
      EqnListRemoveDuplicates(cl->literals);
      ClausePushDerivation(cl, DCPruneArg, NULL, NULL);
   }
   SubstDelete(subst);

   PObjMapFreeWDeleter(var_occs, free_var_occs);
   PObjMapFreeWDeleter(var_removed_args, free_removed_args);
   PStackFree(trav_stack);
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetRecognizeChoice
//
//   Recognize all choice axioms in set, insert them in the choice_syms,
//   and move the clauses to archive.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
void ClauseSetRecognizeChoice(IntMap_p choice_syms,
                              ClauseSet_p set,
                              ClauseSet_p archive)
{
   for(Clause_p handle = set->anchor->succ; handle!=set->anchor; handle = handle->succ)
   {
      ClauseRecognizeChoice(choice_syms, handle);
   }
}

/*-----------------------------------------------------------------------
//
// Function: InstantiateChoiceClauses()
//
//   Scan the clause for term of the form (f t) where f is a defined choice
//   symbol and instantiate the saved choice axioms with t and negation thereof
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define FAIL_ON(cond) if (cond) return false

int InstantiateChoiceClauses(ClauseSet_p store, ClauseSet_p archive,
                             IntMap_p choice_syms, Clause_p renamed_cl,
                             Clause_p orig_cl, int limit)
{
   if(orig_cl->proof_depth > limit)
   {
      return 0;
   }

   PStack_p triggers = PStackAlloc();
   int new_cls = 0;

   for(Eqn_p lit = renamed_cl->literals; lit; lit = lit->next)
   {
      assert(PStackEmpty(triggers));
      find_choice_triggers(choice_syms, triggers, lit->lterm);
      find_choice_triggers(choice_syms, triggers, lit->rterm);
      while(!PStackEmpty(triggers))
      {
         Term_p trigger = PStackPopP(triggers);
         if(!TermIsAppliedFreeVar(trigger))
         {
            FunCode choice_code = trigger->f_code;
            trigger = trigger->args[0];

            new_cls += inst_choice(store, choice_syms, orig_cl, choice_code, trigger);
         }
         else
         {
            assert(trigger->arity == 2);
            Term_p var = trigger->args[0];
            trigger = trigger->args[1];

            IntMapIter_p iter = IntMapIterAlloc(choice_syms, 0, LONG_MAX);
            PStack_p choice_codes = PStackAlloc();
            FunCode ch_code = 0;
            while(IntMapIterNext(iter, &ch_code))
            {
               Sig_p sig = lit->bank->sig;
               if(var->type == SigGetType(sig, ch_code))
               {
                  PStackPushInt(choice_codes, ch_code);
               }
            }
            IntMapIterFree(iter);

            if(PStackEmpty(choice_codes))
            {
               PStackPushInt(choice_codes,
                             mk_new_choice(lit->bank, archive, choice_syms, var->type));
            }

            while(!PStackEmpty(choice_codes))
            {
               new_cls += inst_choice(store, choice_syms, orig_cl,
                                      PStackPopInt(choice_codes), trigger);
            }
            PStackFree(choice_codes);
         }
      }
   }
   PStackFree(triggers);
   return new_cls;
}

/*-----------------------------------------------------------------------
//
// Function: PreinstantiateInduction()
//
//   Compute all induction triggers from the original clause set and
//   instantiate clauses that have variables of the correct type.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void del_node(void* key, void* val)
{
   PStack_p triggers = (PStack_p) val;
   while(!PStackEmpty(triggers))
   {
      PtrPair_p pair = PStackPopP(triggers);
      PtrPairFree(pair);
   }
   PStackFree(triggers);
}

void PreinstantiateInduction(FormulaSet_p forms, ClauseSet_p cls, ClauseSet_p archive, TB_p bank)
{
   VarBankSetVCountsToUsed(bank->vars);
   PObjMap_p terms_by_type = NULL;
   for(WFormula_p handle = forms->anchor->succ; handle != forms->anchor;
       handle = handle->succ)
   {
      if(FormulaQueryType(handle) == CPTypeConjecture)
      {
         store_abstraction_form(handle, archive, &terms_by_type);
      }
   }
   for(Clause_p handle = cls->anchor->succ; handle != cls->anchor;
       handle = handle->succ)
   {
      if(ClauseIsConjecture(handle) && ClauseLiteralNumber(handle) == 1)
      {
         store_abstraction_cl(handle, &terms_by_type);
      }
   }

   PStack_p res = PStackAlloc();

   for(Clause_p handle = cls->anchor->succ; handle != cls->anchor;
       handle = handle->succ)
   {
      PTree_p vars = NULL;
      ClauseCollectVariables(handle, &vars);

      PStack_p iter = PTreeTraverseInit(vars);
      PTree_p node = NULL;
      while((node = PTreeTraverseNext(iter)))
      {
         instantiate_w_abstractions(node->key, handle, &terms_by_type, res);
      }

      PTreeFree(vars);
      PStackFree(iter);
   }

   while(!PStackEmpty(res))
   {
      ClauseSetInsert(cls, PStackPopP(res));
   }

   PStackFree(res);
   PObjMapFreeWDeleter(terms_by_type, del_node);
}

/*-----------------------------------------------------------------------
//
// Function: ComputeHOInferences()
//
//   Computes all registered HO inferences.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ComputeHOInferences(ProofState_p state, ProofControl_p control,
                         Clause_p renamed_cl, Clause_p orig_clause)
{
   if (problemType == PROBLEM_HO)
   {
      if (control->heuristic_parms.arg_cong != NoLits)
      {
         ComputeArgCong(state, control, orig_clause);
      }
      if (control->heuristic_parms.neg_ext != NoLits)
      {
         ComputeNegExt(state, control, orig_clause);
      }
      if (control->heuristic_parms.neg_ext != NoLits)
      {
         ComputePosExt(state, control, orig_clause);
      }
      if (control->heuristic_parms.inverse_recognition)
      {
         InferInjectiveDefinition(state, control, orig_clause);
      }
      if (control->heuristic_parms.ext_rules_max_depth >= 0)
      {
         ComputeExtSup(state, control, renamed_cl, orig_clause);
         ComputeExtEqRes(state, control, orig_clause);
         ComputeExtEqFact(state, control, orig_clause);
      }
      if (control->heuristic_parms.elim_leibniz_max_depth >= 0)
      {
         EliminateLeibnizEquality(state->tmp_store, orig_clause,
                                  control->heuristic_parms.elim_leibniz_max_depth);
      }
      if (control->heuristic_parms.prim_enum_max_depth >= 0)
      {
         PrimitiveEnumeration(state->tmp_store, orig_clause,
                              control->heuristic_parms.prim_enum_mode,
                              control->heuristic_parms.prim_enum_max_depth);
      }
      if (control->heuristic_parms.inst_choice_max_depth >=0)
      {
         InstantiateChoiceClauses(state->tmp_store, state->archive, state->choice_opcodes,
                                  renamed_cl, orig_clause,
                                  control->heuristic_parms.inst_choice_max_depth);
      }
   }
}
