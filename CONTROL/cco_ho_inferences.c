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

void store_result(Clause_p new_clause, Clause_p orig_clause, 
                  ClauseSet_p store, DerivationCode dc)
{
   new_clause->proof_depth = orig_clause->proof_depth+1;
   new_clause->proof_size  = orig_clause->proof_size+1;
   ClauseSetTPTPType(new_clause, ClauseQueryTPTPType(orig_clause));
   ClauseSetProp(new_clause, ClauseGiveProps(orig_clause, CPIsSOS));
   // TODO: Clause documentation is not implemented at the moment.
   // DocClauseCreationDefault(clause, inf_efactor, clause, NULL);
   ClausePushDerivation(new_clause, dc, orig_clause, NULL);
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
   
   Term_p s_arg = TermTopAlloc(s->f_code, s->arity+1);
   s_arg->type = TypeBankInsertTypeShared(tb, TypeDropFirstArg(s->type));
   for(int i=0; i<s->arity; i++)
   {
      s_arg->args[i] = s->args[i]; 
   }
   s_arg->args[s->arity] = arg;
   return s_arg;
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

      if (EqnIsNegative(lit) && EqnIsMaximal(lit) && needed_args > 0)
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
            EqnListInsertFirst(&new_lit, new_literals);

            Clause_p new_clause = ClauseAlloc(new_literals);
            state->neg_ext_count++;
            store_result(new_clause, clause, state->tmp_store, DCNegExt);
         }

         TermArgArrayFree(vars_types, num_vars);
         TermArgArrayFree(vars, num_vars);
         PStackFree(free_vars_stack);
         PTreeFree(free_var_tree);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: ComputeHOInferences()
//
//   Computes all registered HO inferences. 
//   Currently only NegExt is registered.
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
      ComputeNegExt(state,control,clause);
   }
}