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
      s_arg = TermTopAlloc(SIG_APP_VAR_CODE, 2);
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

/*-----------------------------------------------------------------------
//
// Function: unif_all_pairs()
//
//   Assuming that stack contains [s1, t1, s2, t2, ..., sn, tn]
//   computes simultaneous unifier of s1 =?= t1, ..., sn =?= tn
//   and stores it in subst.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool unif_all_pairs(PStack_p pairs, Subst_p subst)
{
   assert(PStackGetSP(pairs) % 2 == 0);
   PStackPointer pos = PStackGetSP(subst);
   bool unifies = true;

   while(unifies && !PStackEmpty(pairs))
   {
      Term_p s = PStackPopP(pairs);
      Term_p t = PStackPopP(pairs);

      unifies = SubstMguComplete(s, t, subst);
   }

   if (!unifies)
   {
      SubstBacktrackToPos(subst, pos);
   }
   return unifies;
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
         Term_p* vars = TermArgArrayAlloc(num_vars);
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
         TermArgArrayFree(vars, num_vars);
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
         (control->heuristic_parms.pos_ext == MaxLits && EqnIsMaximal(lit));
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
// Function: RecognizeInjectivity()
//
//   Create a clause that postulates existence of an inverse function
//   for a given expression. In other words:
//
//           f s1 ... X ... sn != f t1 ... Y ... tn \/ X = Y
//    --------------------------------------------------------------
//            inv_f_i(f sigma(s1) ... X ... sigma(sn)) = X
//
//   where sigma is the **simultaneous** unifier of 
//   s1 =?= t1 ... sn =?= tn, and X,Y do not appear in any of the
//   s1, ..., sn, t1, ..., tn.
//   If sigma was a variable renaming, then resulting clause is tagged with
//   CPPureInjectivity. If inference was unsucessful, NULL is returned.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p RecognizeInjectivity(ProofState_p state, ProofControl_p control, Clause_p clause)
{
   assert(clause);
   Clause_p res = NULL;

   if(clause->pos_lit_no == 1 && clause->neg_lit_no == 1)
   {
      Eqn_p pos_lit = EqnIsPositive(clause->literals) ? 
                        clause->literals : clause->literals->next;
      Eqn_p neg_lit = EqnIsNegative(clause->literals) ? 
                        clause->literals : clause->literals->next;
      
      if (EqnIsEquLit(pos_lit) && EqnIsEquLit(neg_lit) &&
          TermIsVar(pos_lit->lterm) && TermIsVar(pos_lit->rterm) && 
          pos_lit->lterm != pos_lit->rterm &&
          !TermIsTopLevelVar(neg_lit->lterm) && !TermIsTopLevelVar(neg_lit->rterm)
          && neg_lit->lterm->f_code == neg_lit->rterm->f_code)
      {
         assert(neg_lit->lterm->arity == neg_lit->rterm->arity);

         int idx_var_occ = -1; // index where X or Y appears in f s1 ... X ... sn
         for (int i=0; idx_var_occ == -1 && i<neg_lit->lterm->arity; i++)
         {
            if (neg_lit->lterm->args[i] == pos_lit->lterm && 
                neg_lit->rterm->args[i] == pos_lit->rterm)
            {
               idx_var_occ = i;
            } 
            else if (neg_lit->lterm->args[i] == pos_lit->rterm &&
                     neg_lit->rterm->args[i] == pos_lit->lterm)
            {
               idx_var_occ = i;
            }
         }
         if (idx_var_occ != -1)
         {
            // collect pairs s_i =?= t_i on the stack.
            PStack_p arg_pairs = PStackAlloc();
            Subst_p subst = SubstAlloc();
            bool no_occurs = true; 

            for(int i=0; no_occurs && i<neg_lit->lterm->arity; i++)
            {
               if (i!=idx_var_occ)
               {
                  no_occurs = 
                     !TermHasFCode(neg_lit->lterm->args[i], pos_lit->lterm->f_code) &&
                     !TermHasFCode(neg_lit->rterm->args[i], pos_lit->lterm->f_code) &&
                     !TermHasFCode(neg_lit->lterm->args[i], pos_lit->rterm->f_code) &&
                     !TermHasFCode(neg_lit->rterm->args[i], pos_lit->rterm->f_code);
                  if (no_occurs)
                  {
                     PStackPushP(arg_pairs, neg_lit->lterm->args[i]);
                     PStackPushP(arg_pairs, neg_lit->rterm->args[i]);
                  }
               }
            }

            if (no_occurs && unif_all_pairs(arg_pairs, subst))
            {
               // substitution did not bind X or Y
               assert(!pos_lit->lterm->binding);
               assert(!pos_lit->rterm->binding);

               // f (sigma s_1) ... X ... (sigma s_n)
               Term_p inverse_arg = TBInsert(state->terms, neg_lit->lterm, DEREF_ALWAYS);
               // X 
               Term_p inverse_var = neg_lit->lterm->args[idx_var_occ];
               assert(inverse_var == inverse_arg->args[idx_var_occ]);

               Type_p args[1] = {neg_lit->lterm->type};

               FunCode new_inv_skolem_sym = 
                  SigGetNewTypedSkolem(state->signature, args, 1, pos_lit->lterm->type);
               Term_p inv_skolem_term = TermTopAlloc(new_inv_skolem_sym, 1);
               inv_skolem_term->args[0] = inverse_arg;
               inv_skolem_term->type = pos_lit->lterm->type;
               inv_skolem_term = TBTermTopInsert(state->terms, inv_skolem_term);

               Eqn_p eqn = EqnAlloc(inv_skolem_term, inverse_var, state->terms, true);
               res = ClauseAlloc(eqn);
               set_proof_object(res, clause, DCInvRec);
            }

            PStackFree(arg_pairs);
            SubstDelete(subst);
         }
      }
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ComputeHOInferences()
//
//   Computes all registered HO inferences. 
//   Currently only NegExt and PosExt are registered.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ComputeHOInferences(ProofState_p state, ProofControl_p control, Clause_p clause)
{
   if (problemType == PROBLEM_HO)
   {
      if (control->heuristic_parms.neg_ext != NoLits)
      {
         ComputeNegExt(state,control,clause);
      }
      if (control->heuristic_parms.neg_ext != NoLits)
      {
         ComputePosExt(state,control,clause);
      }
      if (control->heuristic_parms.inverse_recognition)
      {
         // todo : add inverse recognition
      }
   }
}