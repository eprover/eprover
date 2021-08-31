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

void set_proof_object(Clause_p new_clause, Clause_p orig_clause,
                      DerivationCode dc)
{
   new_clause->proof_depth = orig_clause->proof_depth + 1;
   new_clause->proof_size = orig_clause->proof_size + 1;
   ClauseSetTPTPType(new_clause, ClauseQueryTPTPType(orig_clause));
   ClauseSetProp(new_clause, ClauseGiveProps(orig_clause, CPIsSOS));
   // TODO: Clause documentation is not implemented at the moment.
   // DocClauseCreationDefault(clause, inf_efactor, clause, NULL);
   ClausePushDerivation(new_clause, dc, orig_clause, NULL);
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

void store_result(Clause_p new_clause, Clause_p orig_clause,
                  ClauseSet_p store, DerivationCode dc)
{
   set_proof_object(new_clause, orig_clause, dc);
   ClauseSetInsert(store, new_clause);
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
   assert(TypeIsPredicate(app_var->args[0]->type));
   int generated_cls = 0;

   if(mode == NegMode || mode == FullMode)
   {
      // generate not
   }
   if(mode == AndMode || mode == FullMode)
   {
      // generate and
   }
   if(mode == OrMode || mode == FullMode)
   {
      // generate and
   }
   if(mode == EqMode || mode == FullMode)
   {
      // generate and
   }
   // generate true
   // generate false
   if(mode == QuantMode || mode == FullMode)
   {
      // generate quants
   }
   if(mode == PragmaticMode)
   {
      // generate various pragmatic instances
   }
   if(mode == LogSymbolMode)
   {
      // generate exact logical symbols
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
             s->f_code == t->f_code)
         {
            assert(s->arity == t->arity);
            for (int i = 0; i < t->arity; i++)
            {
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
      store_result(res, cl, store, DCExtEqRes);
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
         && ClausePosGetOtherSide(from_pos) == ClausePosGetOtherSide(into_pos)) &&
       find_disagreements(terms->sig, from_t, into_t, disagreements))
   {
      Subst_p subst = SubstAlloc();
      VarBankResetVCounts(freshvars);
      NormSubstEqnList(from_pos->clause->literals, subst, freshvars);
      NormSubstEqnList(into_pos->clause->literals, subst, freshvars);

      Eqn_p condition = NULL;
      while (!PStackEmpty(disagreements))
      {
         Eqn_p cond = EqnAlloc(TBInsertInstantiated(terms, PStackPopP(disagreements)),
                               TBInsertInstantiated(terms, PStackPopP(disagreements)),
                               terms, false);
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
   store_result(res, cl, store, DCLeibnizElim);

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
            store_result(new_clause, clause, state->tmp_store, DCNegExt);
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
            PStackPushP(fresh_vars, VarBankGetFreshVar(varbank, lhs->type->args[i]));

            Term_p new_lhs = ApplyTerms(bank, lhs, fresh_vars),
                   new_rhs = ApplyTerms(bank, rhs, fresh_vars);
            Eqn_p new_lit = EqnAlloc(new_lhs, new_rhs, bank, true);
            Eqn_p new_literals = EqnListCopyExcept(clause->literals, lit, bank);
            EqnListInsertFirst(&new_literals, new_lit);

            EqnListLambdaNormalize(new_literals);
            Clause_p new_clause = ClauseAlloc(new_literals);
            store_result(new_clause, clause, state->tmp_store, DCArgCong);
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
               store_result(new_clause, clause, state->tmp_store, DCPosExt);
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
void ComputeExtSup(ProofState_p state, ProofControl_p control,
                   Clause_p renamed_cl, Clause_p orig_clause)
{
   if (orig_clause->proof_depth <= control->heuristic_parms.ext_sup_max_depth)
   {
      do_ext_sup_from(renamed_cl, orig_clause, state);
      do_ext_sup_into(renamed_cl, orig_clause, state);
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

void ComputeExtEqRes(ProofState_p state, ProofControl_p control, Clause_p cl)
{
   if (cl->proof_depth <= control->heuristic_parms.ext_sup_max_depth)
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
// Function: InferInjectiveDefinition()
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
      if (lit->rterm == bank->true_term &&
          (lit->lterm->f_code == bank->sig->eqn_code || lit->lterm->f_code == bank->sig->neqn_code || lit->lterm->f_code == bank->sig->not_code))
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

         if(lit->rterm != bank->true_term)
         {
            EqnSetProp(lit, EPIsEquLiteral);
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
      EqnListRemoveResolved(&cl->literals);
      EqnListRemoveDuplicates(cl->literals);
      ClauseRecomputeLitCounts(cl);
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

bool ImmediateClausification(Clause_p cl, ClauseSet_p store, ClauseSet_p archive,
                             VarBank_p fresh_vars)
{
   bool clausified = false;
   for (Eqn_p lit = cl->literals; !clausified && lit; lit = lit->next)
   {
      if (EqnIsClausifiable(lit))
      {
         // DBG_PRINT(stderr, " ", EqnPrintDBG(stderr, lit), " is clausifiable.\n");
         // DBG_PRINT(stderr, "new cnf attempt: ", ClausePrintDBG(stderr, cl), ".\n");
         // DBG_PRINT(stderr, "derivation", DerivationDebugPrint(stderr, cl->derivation), ".\n");
         TB_p bank = lit->bank;

         VarBankSetVCountsToUsed(bank->vars);

         WFormula_p wrapped = WFormulaOfClause(cl, bank);
         // DBG_PRINT(stderr, "decoded: ", TermPrintDbgHO(stderr, wrapped->tformula, lit->bank->sig, DEREF_NEVER), ".\n");

         FormulaSet_p work_set = FormulaSetAlloc();
         FormulaSetInsert(work_set, wrapped);

         ClauseSet_p res_set = ClauseSetAlloc();
         FormulaSet_p archive = FormulaSetAlloc();

         // DBG_PRINT(stderr, "begin: ", FormulaSetPrint(stderr, work_set, true), ".\n");
         TFormulaSetUnrollFOOL(work_set, archive, bank);
         // DBG_PRINT(stderr, "unrolled: ", FormulaSetPrint(stderr, work_set, true), ".\n");
         FormulaSetSimplify(work_set, bank, false);
         // DBG_PRINT(stderr, "simplifed: ", FormulaSetPrint(stderr, work_set, true), ".\n");
         TFormulaSetIntroduceDefs(work_set, archive, bank);
         // DBG_PRINT(stderr, "defs: ", FormulaSetPrint(stderr, work_set, true), ".\n");

         while (!FormulaSetEmpty(work_set))
         {
            WFormula_p handle = FormulaSetExtractFirst(work_set);
            WFormulaCNF2(handle, res_set, bank, fresh_vars, 100); // low miniscope limit for efficiency
            WFormulaFree(handle);
         }

         FormulaSetFree(work_set);
         FormulaSetFree(archive);
         while (!ClauseSetEmpty(res_set))
         {
            Clause_p res = ClauseSetExtractFirst(res_set);
            // DBG_PRINT(stderr, " > ", ClausePrintDBG(stderr, res), ".\n");
            PStackReset(res->derivation);
            store_result(res, cl, store, DCDynamicCNF);
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
                        RequestDBVar(lit->bank->db_vars, lhs->args[i]->type, lhs->arity-i-1),
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
      if (control->heuristic_parms.ext_sup_max_depth >= 0)
      {
         ComputeExtSup(state, control, renamed_cl, orig_clause);
         ComputeExtEqRes(state, control, orig_clause);
      }
      if (control->heuristic_parms.elim_leibniz_max_depth >= 0)
      {
         EliminateLeibnizEquality(state->tmp_store, orig_clause,
                                  control->heuristic_parms.elim_leibniz_max_depth);
      }
   }
}
