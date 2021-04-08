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
   new_clause->proof_depth = orig_clause->proof_depth+1;
   new_clause->proof_size  = orig_clause->proof_size+1;
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
// Function: build_eq_des_res()
//
//   Given an offending equivalence lit build flattened clause
//   using given signs for the left and right hand side of the equivalence
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p build_eq_des_res(Eqn_p lit, bool sign_lterm, bool sign_rterm, Clause_p cl)
{
   TB_p terms = lit->bank;
   Eqn_p rest = EqnListCopyExcept(cl->literals, lit, terms);

   assert(lit->lterm != terms->true_term);
   assert(lit->rterm != terms->true_term);

   Eqn_p l_lit = EqnAlloc(lit->lterm, terms->true_term, terms, sign_lterm);
   Eqn_p r_lit = EqnAlloc(lit->rterm, terms->true_term, terms, sign_rterm);
   l_lit->next = r_lit;

   EqnListAppend(&l_lit, rest);
   return ClauseAlloc(l_lit);
}

/*-----------------------------------------------------------------------
//
// Function: do_equiv_destruct()
//
//   Goes through clauses and finds a boolean literal that is not of the
//   form s (!=) $true and distributes the literal over the rest of
//   the clause
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool do_equiv_destruct(Clause_p cl, PStack_p tasks)
{
   bool found_lit = false;
   for(Eqn_p lit=cl->literals; !found_lit && lit; lit=lit->next)
   {
      TB_p terms = lit->bank;
      if(TypeIsBool(lit->lterm->type) && 
         lit->rterm != terms->true_term &&
         (TermIsVar(lit->lterm) ||
          !SigQueryProp(terms->sig, lit->lterm->f_code, FPFOFOp)) &&
         (TermIsVar(lit->rterm) ||
          !SigQueryProp(terms->sig, lit->rterm->f_code, FPFOFOp)))
      {
         Clause_p child1, child2;
         if(EqnIsPositive(lit))
         {
            child1 = build_eq_des_res(lit, false, true, cl);
            child2 = build_eq_des_res(lit, true, false, cl);
         }
         else
         {
            child1 = build_eq_des_res(lit, false, false, cl);
            child2 = build_eq_des_res(lit, true, true, cl);
         }

         found_lit = true;
         PStackPushP(tasks, child1);
         PStackPushP(tasks, child2);
      }
   }
   return found_lit;
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
   if(t->type != s->type || t == s)
   {
      return false;
   }

   PStackPointer begin = PStackGetSP(diss_stack);

   PStack_p tasks = PStackAlloc();
   PStackPushP(tasks, t);
   PStackPushP(tasks, s);

   bool exists_elig = false;
   
   while(!PStackEmpty(tasks))
   {
      s = PStackPopP(tasks);
      t = PStackPopP(tasks);

      if(s!=t)
      {
         if(!TermIsTopLevelVar(s) && !TermIsTopLevelVar(t) &&
            s->f_code == t->f_code)
         {
            assert(s->arity == t->arity);
            for(int i=0; i < t->arity; i++)
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
                           && !SigQueryFuncProp(sig, s->f_code, FPFOFOp)
                           && !SigQueryFuncProp(sig, t->f_code, FPFOFOp));
         }
      }
   }

   if(!exists_elig)
   {
      while(PStackGetSP(diss_stack) != begin)
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
   if(find_disagreements(terms->sig, lhs, rhs, disagreements))
   {
      Eqn_p condition = NULL;
      while(!PStackEmpty(disagreements))
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
   if(find_disagreements(terms->sig, from_t, into_t, disagreements))
   {
      Subst_p subst = SubstAlloc();
      VarBankResetVCounts(freshvars);
      NormSubstEqnList(from_pos->clause->literals, subst, freshvars);
      NormSubstEqnList(into_pos->clause->literals, subst, freshvars);

      Eqn_p condition = NULL;
      while(!PStackEmpty(disagreements))
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
      
      Clause_p res = ClauseAlloc(condition);
      res->proof_size  = into_pos->clause->proof_size+from_pos->clause->proof_size+1;
      res->proof_depth = MAX(into_pos->clause->proof_depth, from_pos->clause->proof_depth)+1;

      ClauseSetProp(res, (ClauseGiveProps(into_pos->clause, CPIsSOS)|
                          ClauseGiveProps(from_pos->clause, CPIsSOS)));
      if(!into_pos->clause->derivation)
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

   while(!PStackEmpty(from_pos_stack))
   {
      CompactPos cpos_from = PStackPopInt(from_pos_stack);
      UnpackClausePosInto(cpos_from, renamed_cl, from_pos);

      FunCode fc = PStackPopInt(from_pos_stack);
      ClauseTPosTree_p into_partners = IntMapGetVal(into_idx, fc);


      PStack_p iter = PTreeTraverseInit(into_partners);
      PTree_p node = NULL;
      while((node = PTreeTraverseNext(iter)))
      {
         ClauseTPos_p cl_cpos = node->key;
         PStack_p niter = NumTreeTraverseInit(cl_cpos->pos);
         NumTree_p node;
         while((node = NumTreeTraverseNext(niter)))
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

   while(!PStackEmpty(into_pos_stack))
   {
      CompactPos cpos_into = PStackPopInt(into_pos_stack);
      UnpackClausePosInto(cpos_into, renamed_cl, into_pos);

      FunCode fc = PStackPopInt(into_pos_stack);
      assert(fc > state->signature->internal_symbols);
      ClauseTPosTree_p from_partners = IntMapGetVal(from_idx, fc);


      PStack_p iter = PTreeTraverseInit(from_partners);
      PTree_p node = NULL;
      while((node = PTreeTraverseNext(iter)))
      {
         ClauseTPos_p cl_cpos = node->key;
         PStack_p niter = NumTreeTraverseInit(cl_cpos->pos);
         NumTree_p node;
         while((node = NumTreeTraverseNext(niter)))
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
// Function: term_apply_arg()
//
//   Applies one term to the other. Performs rudimentary typechecking.
//   Term is UNSHARED.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p term_apply_arg(TypeBank_p tb, Term_p s, Term_p arg) 
{
   assert(TypeIsArrow(s->type));
   assert(s->type->args[0] == arg->type);
   
   Term_p s_arg = NULL;
   if (UNLIKELY(!TermIsVar(s)))
   {
      s_arg = TermTopAlloc(s->f_code, s->arity+1);
      for(int i=0; i<s->arity; i++)
      {
         s_arg->args[i] = s->args[i]; 
      }
      s_arg->args[s->arity] = arg;
   }
   else
   {
      s_arg = TermTopAlloc(SIG_PHONY_APP_CODE, 2);
      s_arg->args[0] = s;
      s_arg->args[1] = arg;
   }
   
   s_arg->type = TypeBankInsertTypeShared(tb, TypeDropFirstArg(s->type));
   
   return s_arg;
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
   Type_p* args = TypeArgArrayAlloc(needed_args + 2);
   args[0] = s->args[s->arity-1]->type;
   if (UNLIKELY(TypeIsArrow(s->type)))
   {
      for(int i=0; i<s->type->arity; i++)
      {
         args[i+1] = s->type->args[i];
      }
   }
   else
   {
      args[1] = s->type;
   }
   
   Type_p res_type = TypeBankInsertTypeShared(tb, AllocArrowType(needed_args+2, args));

   if (TermIsAppliedVar(s) && s->arity==2)
   {
      Term_p t = s->args[0];
      assert(t->type == res_type);
      assert(TermIsVar(t));
      return t;
   }
   else 
   {
      Term_p t = TermTopAlloc(s->f_code, s->arity-1);
      t->type = res_type;
      for(int i=0; i<s->arity-1; i++)
      {
         t->args[i] = s->args[i];
      }
      return t;
   }
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
   for(Eqn_p lit = clause->literals; lit; lit = lit->next)
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
         Term_p* vars = TermArgTmpArrayAlloc(num_vars);
         Type_p* vars_types = TypeArgArrayAlloc(num_vars);
         for(int i=0; i<num_vars; i++)
         {
            vars[i] = PStackElementP(free_vars_stack, i);
            vars_types[i] = vars[i]->type;
         }

         Term_p new_lhs = lit->lterm, new_rhs = lit->rterm;
         for(int i=0; i<needed_args; i++)
         {
            Term_p skolem = 
               TermTopAlloc(
                  SigGetNewTypedSkolem(state->signature, vars_types,
                                       num_vars, lit_type->args[i]),
                  num_vars);
            for(int j=0; j<num_vars; j++)
            {
               skolem->args[j] = vars[j];
            }
            skolem->type = lit_type->args[i];
            skolem = TBTermTopInsert(state->terms, skolem);
            new_lhs = TBTermTopInsert(state->terms,
                                      term_apply_arg(state->signature->type_bank, new_lhs, skolem));
            new_rhs = TBTermTopInsert(state->terms,
                                      term_apply_arg(state->signature->type_bank, new_rhs, skolem));

            Eqn_p new_lit = EqnAlloc(new_lhs, new_rhs, state->terms, false);
            Eqn_p new_literals = EqnListCopyExcept(clause->literals, lit, state->terms);
            EqnListInsertFirst(&new_literals, new_lit);

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
   for(Eqn_p lit=clause->literals; lit; lit=lit->next)
   {
      bool lit_filter = 
         control->heuristic_parms.pos_ext == AllLits ||
         (control->heuristic_parms.pos_ext == MaxLits && EqnIsStrictlyMaximal(lit));
      if (EqnIsPositive(lit) && EqnIsEquLit(lit) && lit_filter)
      {
         Term_p lhs = lit->lterm, rhs = lit->rterm;

         while(lhs->arity && rhs->arity &&
               lhs->args[lhs->arity-1] == rhs->args[rhs->arity-1] &&
               TermIsVar(lhs->args[lhs->arity-1]))
         {
            Term_p var =  lhs->args[lhs->arity-1];

            bool occurs = false;
            /* Checking if var appears in any of the arguments */
            for(int i=0; !occurs && i<lhs->arity-1; i++)
            {
               occurs = TermHasFCode(lhs->args[i], var->f_code); 
            }
            for(int i=0; !occurs && i<rhs->arity-1; i++)
            {
               occurs = TermHasFCode(rhs->args[i], var->f_code); 
            }
            /* Checking if var appears in other literals */
            for(Eqn_p iter = clause->literals; !occurs && iter; iter = iter->next)
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

               if (!TermIsVar(lhs))
               {
                  lhs = TBTermTopInsert(state->terms, lhs);
               }
               if (!TermIsVar(rhs))
               {
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
      fprintf(stderr, "inj_def(");
      ClausePrint(stderr, clause, true);
      fprintf(stderr, ") = \n ");
      ClausePrint(stderr, res, true);
      fprintf(stderr, "\n");

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
      for(Eqn_p lit=cl->literals; lit; lit=lit->next)
      {
         if(EqnIsNegative(lit) && EqnIsEquLit(lit) &&
            !TypeIsArrow(lit->lterm) &&
            lit->lterm->f_code == lit->rterm->f_code &&
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
// Function: DestructEquivalences()
//
//   Performs dynamic clausfication of equivalences.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool DestructEquivalences(Clause_p cl, ClauseSet_p store, ClauseSet_p archive)
{
   PStack_p tasks = PStackAlloc();
   PStackPushP(tasks, cl);
   bool destructed_one = false;

   while(!PStackEmpty(tasks))
   {
      Clause_p task = PStackPopP(tasks);
      if(!do_equiv_destruct(task, tasks))
      {
         // clause reached a fixed point
         if(task != cl)
         {
            store_result(task, cl, store, DCDynamicCNF);
            destructed_one = true;
         }
      }
      else if(task != cl)
      {
         // removing an intermediary clause created above
         ClauseFree(task);
      }
   }
   
   PStackFree(tasks);
   if(destructed_one)
   {
      ClauseSetInsert(archive, cl);
   }
   return destructed_one;
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

   for(Eqn_p lit = cl->literals; is_resolvable && lit; lit = lit->next)
   {
      if(EqnIsEquLit(lit))
      {
         is_resolvable = is_resolvable 
                         && EqnIsNegative(lit)
                         && TermIsTopLevelVar(lit->lterm)
                         && TermIsTopLevelVar(lit->rterm);
         if(is_resolvable && TypeIsPredicate(lit->lterm->type))
         {
            Term_p lvar = (TermIsVar(lit->lterm) ? lit->lterm : lit->lterm->args[0]);
            Term_p rvar = (TermIsVar(lit->rterm) ? lit->rterm : lit->rterm->args[0]);
            int* prev_l = IntMapGetVal(ids_to_sign, lvar->f_code);
            int* prev_r = IntMapGetVal(ids_to_sign, rvar->f_code);
            if(prev_l || prev_r)
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
         if(!TermIsTopLevelVar(lit->lterm))
         {
            is_resolvable = false;
         }
         else
         {
            Term_p var = (TermIsVar(lit->lterm) ? lit->lterm : lit->lterm->args[0]);
            int* prev_val = IntMapGetVal(ids_to_sign, var->f_code);
            if(!prev_val)
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

   if(is_resolvable)
   {
      EqnListFree(cl->literals);
      cl->literals = NULL;
      ClauseRecomputeLitCounts(cl);
      ClausePushDerivation(cl, DCCondense, NULL, NULL);
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
      if (control->heuristic_parms.neg_ext != NoLits)
      {
         ComputeNegExt(state,control,orig_clause);
      }
      if (control->heuristic_parms.neg_ext != NoLits)
      {
         ComputePosExt(state,control,orig_clause);
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
   }
}
