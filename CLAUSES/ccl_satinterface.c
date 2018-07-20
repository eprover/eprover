/*-----------------------------------------------------------------------

  File  : ccl_satinterface.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Functions for efficient conversion of the proof state to
  propositional clauses and submission to a SAT solver.


  Copyright 2017 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Sat Sep 16 16:52:43 CEST 2017

  -----------------------------------------------------------------------*/

#include "ccl_satinterface.h"
#include <cte_idx_fp.h>



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

#define PICOSAT_BUFSIZE 200


char* GroundingStratNames[] =
{
   "NoGrounding",
   "PseudoVar",
   "FirstConst",
   "ConjMinMinFreq",
   "ConjMaxMinFreq",
   "ConjMinMaxFreq",
   "ConjMaxMaxFreq",
   "GlobalMax",
   "GlobalMin",
   NULL
};

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: sat_encode_lit()
//
//   If literal is a predicate atom, it returns the corresponding term.
//   If literal is an equational atom s=t or s!=t, it returns =(s,t) 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Term_p sat_encode_lit(Eqn_p lit)
{
   Term_p s = lit->lterm;
   Term_p t = lit->rterm;
   Term_p res = s;

   if(EqnIsEquLit(lit))
   {   
      res = EqnTermsTBTermEncode(lit->bank, s, t, true, 
                                 PGreater(s, t) ? PENormal : PEReverse);
      
   }
   
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ground()
//
//   Bind remaining variables to a constant. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void sat_ground_term(Term_p t, Subst_p subst, TB_p bank)
{
   PStack_p subterms = PStackAlloc();
   PStack_p empty = PStackAlloc();
   PStackPushP(subterms, t);

   while(!PStackEmpty(subterms))
   {
      t = TermDerefAlways(PStackPopP(subterms));
      if(TermIsVar(t))
      {
         Term_p constant = TBGetFirstConstTerm(bank, t->type);
         if(!constant)
         {
            constant = TBAllocNewSkolem(bank, empty, t->type);
            assert(TBGetFirstConstTerm(bank, t->type));
         }
         SubstAddBinding(subst, t, constant);
      }
      for(int i=0; i<t->arity; i++)
      {
         PStackPushP(subterms, t->args[i]);
      }
   }

   PStackFree(subterms);
   PStackFree(empty);
}

/*-----------------------------------------------------------------------
//
// Function: sat_translate_literal()
//
//    Translate a full E literal into a propositional literal.
//
// Global Variables: -
//
// Side Effects    : May alloc and/or updare renumber index, memory
//                   operations, inserts terms into eqn->bank.
//
/----------------------------------------------------------------------*/

int sat_translate_literal(Eqn_p eqn, SatClauseSet_p set)
{
   int  atom = 0;
   long lit_code;
   Term_p lterm, rterm, lit_term;

   assert(eqn);
   assert(set);


   if(EqnIsEquLit(eqn))
   {
      lterm = TBInsertInstantiated(eqn->bank, eqn->lterm);
      rterm = TBInsertInstantiated(eqn->bank, eqn->rterm);
      // We normalize literals to avoid spurious failures
      if(PGreater(lterm, rterm))
      {
         lit_term = EqnTermsTBTermEncode(eqn->bank, lterm, rterm, true, PENormal);
      }
      else
      {
         lit_term = EqnTermsTBTermEncode(eqn->bank, lterm, rterm, true, PEReverse);
      }
   }
   else
   {
      lit_term = TBInsertInstantiated(eqn->bank, eqn->lterm);
   }
   lit_code = lit_term->entry_no;

   if(!set->renumber_index)
   {
      set->renumber_index = PDRangeArrAlloc(lit_code, 0);
   }
   atom = PDRangeArrElementInt(set->renumber_index, lit_code);
   if(!atom)
   {
      atom = ++set->max_lit;
      PDRangeArrAssignInt(set->renumber_index, lit_code, atom);
   }
   if(EqnIsPositive(eqn))
   {
      return atom;
   }
   return -atom;
}

/*-----------------------------------------------------------------------
//
// Function: litstate_add_satclause()
//
//   Add the literals of a clause to the literal state array (bit 0
//   indicates presence of positive instances of the atom, bit 1
//   represents pesence of negative instances).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void litstate_add_satclause(int* state, SatClause_p clause)
{
   int i, lit;
   assert(state);
   assert(clause);

   for(i=0; i<clause->lit_no; i++)
   {
      lit = clause->literals[i];
      if(lit > 0)
      {
         state[lit] |= 1;
      }
      else
      {
         state[-lit] |= 2;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: litstate_check_pure()
//
//   Given a SatClause and a literal state array, check if any of the
//   literals in the clause is pure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool litstate_check_pure(int* state, SatClause_p clause)
{
   int i, lit;
   assert(state);
   assert(clause);

   for(i = 0; i<clause->lit_no; i++)
   {
      lit = clause->literals[i];
      if(lit > 0)
      {
         assert(state[lit]);
         if(state[lit]!=3)
         {
            return true;
         }
      }
      else
      {
         assert(state[-lit]);
         if(state[-lit]!=3)
         {
            return true;
         }
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: prefer_conj_min_max_freq()
//
//   Prefer conjecture symbols, among those the ones rarest in
//   conjectures, and among those the ones most frequent overall,
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_conj_min_max_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   if(conj_dist_array[f1] && !conj_dist_array[f2])
   {
      return true;
   }
   if(!conj_dist_array[f1] && conj_dist_array[f2])
   {
      return false;
   }
   return (conj_dist_array[f1] < conj_dist_array[f2])||
      ((conj_dist_array[f1] = conj_dist_array[f2]) &&
       (dist_array[f1] > dist_array[f2]));
}


/*-----------------------------------------------------------------------
//
// Function: prefer_conj_max_max_freq()
//
//   Prefer symbols based on lexicographic comparision of conjecture
//   count, total count.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_conj_max_max_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   return (conj_dist_array[f1] > conj_dist_array[f2])||
      ((conj_dist_array[f1] = conj_dist_array[f2]) &&
       (dist_array[f1] > dist_array[f2]));
}


/*-----------------------------------------------------------------------
//
// Function: prefer_conj_min_min_freq()
//
//   Prefer conjecture symbols, among those rare conjecture symbols,
//   and among those overall rare symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_conj_min_min_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   if(conj_dist_array[f1] && !conj_dist_array[f2])
   {
      return true;
   }
   if(!conj_dist_array[f1] && conj_dist_array[f2])
   {
      return false;
   }
   return (conj_dist_array[f1] < conj_dist_array[f2])||
      ((conj_dist_array[f1] = conj_dist_array[f2]) &&
       (dist_array[f1] < dist_array[f2]));
}


/*-----------------------------------------------------------------------
//
// Function: prefer_conj_max_min_freq()
//
//   Prefer symbols based on lexicographic comparision of conjecture
//   count, -total count.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_conj_max_min_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   return (conj_dist_array[f1] > conj_dist_array[f2])||
      ((conj_dist_array[f1] = conj_dist_array[f2]) &&
       (dist_array[f1] < dist_array[f2]));
}


/*-----------------------------------------------------------------------
//
// Function: prefer_global_max_freq()
//
//   Prefer most frequent symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_global_max_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   return (dist_array[f1] > dist_array[f2]);
}


/*-----------------------------------------------------------------------
//
// Function: prefer_global_min_freq()
//
//   Prefer least frequent symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_global_min_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   return (dist_array[f1] < dist_array[f2]);
}


/*-----------------------------------------------------------------------
//
// Function: sat_clause_not_pure()
//
//   Does the SAT clause have no pure literals?
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool sat_clause_not_pure(SatClause_p cl)
{
   return !cl->has_pure_lit;
}

/*-----------------------------------------------------------------------
//
// Function: export_to_solver()
//
//   Adds the clauses that satisfy filter to the solver state. filter
//   can be NULL in which case all the clauses are added. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void export_to_solver(SatSolver_p solver, SatClauseSet_p set, SatClauseFilter filter)
{
   PStackPointer i;
   SatClause_p clause;

   assert(set);

   PStackReset(set->exported);
   for(i=0; i<PStackGetSP(set->set); i++)
   {
      clause = PStackElementP(set->set, i);
      if(filter == NULL || filter(clause))
      {
         picosat_add_lits(solver, clause->literals);
         PStackPushP(set->exported, clause);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ground_gen_instances()
//
//   Ground the clauses in the set and add clauses that encode the
//   implication constraints to generlaized_terms.
//   Remembers which term it generalized, so that the same 
//   (generalization,instance) pairs are not considered again.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ground_gen_instances(ClauseSet_p set, ProofState_p state, 
                          Subst_p ground_subst, PStack_p generalized_terms)
{
   assert(state->instance_encoding_remaining > 0);
   FPIndex_p from_idx     = state->gindices.pm_from_index;
   int*      gen_remains  = &(state->instance_encoding_remaining);
   long      added        = 0;

   int       eval_idx     = 0;
   Eval_p    root = PDArrayElementP(set->eval_indices,eval_idx);
   PStack_p  eval_iter = EvalTreeTraverseInit(root, eval_idx);

   while((root = EvalTreeTraverseNext(eval_iter, eval_idx)))
   {
      Clause_p cl = root->object;
      for(Eqn_p lit = cl->literals; lit && *gen_remains; lit = lit->next)
      {
         // all already generalized terms are marked with a flag
         if(!EqnIsEquLit(lit) && !TermCellQueryProp(lit->lterm, TPSpecialFlag)
            && EqnIsNegative(lit))
         {
            PStack_p candidates = PStackAlloc();
            Term_p   query_term = lit->lterm;
            TB_p     bank       = lit->bank;

            FPIndexFindUnifiable(from_idx, query_term, candidates);

            while(!PStackEmpty(candidates) && *gen_remains)
            {
               PStack_p iter = PTreeTraverseInit(PStackPopP(candidates));
               SubtermTree_p cell = NULL;

               while((cell = PTreeTraverseNext(iter)) && *gen_remains)
               {
                  Term_p candidate = ((SubtermOcc_p)cell->key)->term;           

                  if(candidate != query_term &&
                     (!UnifFailed(SubstMguPossiblyPartial(candidate, 
                                                          query_term, 
                                                          ground_subst, bank))))
                  {
                     assert(TermCellQueryProp(candidate, TPPredPos));

                     (*gen_remains)--;
                     added++;
                     TermCellSetProp(query_term, TPSpecialFlag);
                     PStackPushP(generalized_terms, query_term);
                  }
               }
               PTreeTraverseExit(iter);
            }
            PStackFree(candidates);
         }
      }
   }
   EvalTreeTraverseExit(eval_iter);
   return added;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: SatClauseAlloc()
//
//   Allocate an empty, unlinked propositional clause with space for a
//   given number of literals. Allocates space for lit_no+1 literals,
//   where the last literal is 0 (to support efficient integration with
//   PicoSAT). Note that other literals are not initialized (not even to 0).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

SatClause_p SatClauseAlloc(int lit_no)
{
   SatClause_p handle = SatClauseCellAlloc();

   handle->has_pure_lit = false;
   handle->lit_no       = lit_no;
   // allocating space for terminating zero
   handle->literals     = SizeMalloc((handle->lit_no+1)*sizeof(int));
   handle->literals[handle->lit_no] = 0;
   handle->source       = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseBinary()
//
//   Allocate an binary clause with literals lit1 and lit2.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

SatClause_p SatClauseBinary(int lit1, int lit2)
{
   SatClause_p res = SatClauseAlloc(2);
   res->literals[0] = lit1;
   res->literals[1] = lit2;
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseFree()
//
//    Free the SatClause.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void SatClauseFree(SatClause_p junk)
{
   assert(junk);

   SizeFree(junk->literals, (junk->lit_no+1)*sizeof(int));
   SatClauseCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseSetAlloc()
//
//   Allocate a SatClauseSet. This is much less flexible than full
//   clause sets (clauses can only be added), and also carries some
//   admin information for the translation from normal clauses to
//   propositional clauses.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

SatClauseSet_p SatClauseSetAlloc(void)
{
   SatClauseSet_p set = SatClauseSetCellAlloc();

   set->max_lit = 0;
   set->renumber_index = NULL; // We create this lazily when we know
                               // the first index!
   set->set      = PStackAlloc();
   set->exported = PStackAlloc();
   set->core_size = 0;
   set->set_size_limit = -1;
   return set;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseSetFree()
//
//   Free a SatClauseSet (including the SatClauses).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SatClauseSetFree(SatClauseSet_p junk)
{
   assert(junk);
   SatClause_p clause;

   if(junk->renumber_index)
   {
      PDRangeArrFree(junk->renumber_index);
   }
   while(!PStackEmpty(junk->set))
   {
      clause = PStackPopP(junk->set);
      SatClauseFree(clause);
   }
   PStackFree(junk->set);
   PStackFree(junk->exported);
   SatClauseSetCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: SatClauseCreateAndStore()
//
//    Encode the instantiated clause as a SatClause, store it in set,
//    and return it. If encode_ins points to value > 0 it will encode
//    instance constraints [P(x) => P(sigma(x)], where sigma is an arbitrary
//    substitution and P predicate symbol] that are easy to compute (found 
//    in from_idx fp index). Makes the value encode_gens points to smaller. 
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

SatClause_p SatClauseCreateAndStore(Clause_p clause, SatClauseSet_p set)
{
   int i;
   Eqn_p lit;
   SatClause_p handle;

   assert(clause);
   assert(set);

   if(set->set_size_limit != -1 && 
         PStackGetSP(set->set) >= set->set_size_limit)
   {
      return NULL;
   }

   /*fprintf(stderr,"# PGClause (%d): ", ClauseQueryProp(clause, CPIsSatConstraint));
   ClausePrint(stderr, clause, true);
   fprintf(stderr,"\n=>");
   EqnListPrintDeref(stderr, clause->literals, "|", DEREF_ONCE);
   fprintf(stderr,"\n");*/

   handle = SatClauseAlloc(ClauseLiteralNumber(clause));
   handle->source = ClauseQueryProp(clause, CPIsSatConstraint) ? NULL : clause;
   for(i=0, lit=clause->literals;
       lit;
       i++, lit=lit->next)
   {
      assert(i<handle->lit_no);
      handle->literals[i] = sat_translate_literal(lit, set);
   }

   //SatClausePrint(stderr, handle);

   PStackPushP(set->set, handle);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: SatClausePrint()
//
//   Print a sat clause in DIMACS format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SatClausePrint(FILE* out, SatClause_p satclause)
{
   int i;

   assert(satclause);

   for(i=0; i<satclause->lit_no; i++)
   {
      fprintf(out, "%d ", satclause->literals[i]);
   }
   fprintf(out, "0\n");
}


/*-----------------------------------------------------------------------
//
// Function: SatClauseSetPrint()
//
//   Print a SatClauseSet.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SatClauseSetPrint(FILE* out, SatClauseSet_p set)
{
   PStackPointer i;

   assert(set);

   fprintf(out, "p cnf %d %ld\n", set->max_lit, PStackGetSP(set->set));

   for(i=0; i<PStackGetSP(set->set); i++)
   {
      SatClausePrint(out, PStackElementP(set->set, i));
   }
   //printf("0\n");
}


/*-----------------------------------------------------------------------
//
// Function: SatClauseSetExportToSolver()
//
//   Exports all clauses to solver.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SatClauseSetExportToSolver(SatSolver_p solver, SatClauseSet_p set)
{
   export_to_solver(solver, set, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: SatClauseSetExportToSolverNonPure()
//
//   Exports non-pure clauses to solver.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SatClauseSetExportToSolverNonPure(SatSolver_p solver, SatClauseSet_p set)
{
   export_to_solver(solver, set, sat_clause_not_pure);
}


/*-----------------------------------------------------------------------
//
// Function: SatClauseSetImportClauseSet()
//
//   Import all (instanciated) clauses from set into satset. Return
//   number of clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long SatClauseSetImportClauseSet(SatClauseSet_p satset, ClauseSet_p set)
{
   Clause_p handle;
   assert(satset);
   assert(set);
   int added = 0;

   for(handle = set->anchor->succ;
       handle != set->anchor;
       handle = handle->succ, added++)
   {
      if(!SatClauseCreateAndStore(handle, satset))
      {
         break;
      }
   }

   return added;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseSetImportGenInstances()
//
//   Import all clauses from proofstate into satset. Encodes constraints
//   between generalizations and instances. Creates at most
//   state->instance_gen_limit clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long SatClauseSetImportGenInstances(SatClauseSet_p satset, ProofState_p state)
{
   long res = 0;
   PStack_p generalized_terms = PStackAlloc();
   Subst_p  grounding         = SubstAlloc();
   //This can be fine-tuned -- we might not want to include equations
   ClauseSet_p all_sets[] = 
         { 
            state->unprocessed, state->processed_non_units, 
            state->processed_neg_units, state->processed_pos_eqns,
            state->processed_pos_rules
         };
   long added = true; // was any constraint added?
   int  passes = 0;

   while(state->instance_encoding_remaining && added)
   {
      assert(SubstIsEmpty(grounding));
      added = 0;
      
      for(int i=0; i<sizeof(all_sets)/sizeof(ClauseSet_p) 
                   && state->instance_encoding_remaining; i++)
      {
         added += ground_gen_instances(all_sets[i], state, 
                                       grounding, generalized_terms);
      }

      Subst_p remaining_vars = 
         SubstGroundVarBankFirstConst(state->terms, false);

      if(added)
      {
         for(int i=0; i<sizeof(all_sets)/sizeof(ClauseSet_p) && added; i++)
         {
            res += SatClauseSetImportClauseSet(satset, all_sets[i]);
            if(SatClauseSetLimitReached(satset))
            {
               state->instance_encoding_remaining = 0;
               break;
            }
         }
      }
      
      
      SubstBacktrack(grounding);
      SubstDelete(remaining_vars);
      passes++;
   }

   //fprintf(stderr, "# made %d passes\n", passes);
   
   SubstFree(grounding);
   while(!PStackEmpty(generalized_terms))
   {
      Term_p generalized_term = PStackPopP(generalized_terms);
      TermCellDelProp(generalized_term, TPSpecialFlag);
   }
   PStackFree(generalized_terms);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseSetImportGenInstances()
//
//   Import all clauses from proofstate into satset. Encodes constraints
//   between generalizations and instances. Creates at most
//   state->instance_gen_limit clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

const int CLAUSE_SIZE_LIMIT = 4;
void add_to_inst(PStack_p ig_clauses, Clause_p handle, ClausePos_p cl_pos, 
                          FPIndex_p positive_atom_idx, int* added_from_set)
{
   if(ClauseLiteralNumber(handle) <= CLAUSE_SIZE_LIMIT)
   {
      PStackPushP(ig_clauses, handle);
      (*added_from_set)++;
      cl_pos->clause = handle;
      //SubtermIndexInsertClause(positive_atom_idx, handle);
      for(Eqn_p eq = handle->literals; eq; eq = eq->next)
      {
         if(EqnIsPositive(eq))
         {
            cl_pos->literal = eq;
            OverlapIndexInsertPos(positive_atom_idx, handle, 
                                 PackClausePos(cl_pos), sat_encode_lit(eq));
         }
      }
   }
}

long SatClauseSetImportInstGen(SatClauseSet_p satset, ProofState_p state)
{
   const int CREATED_MAX       = 15000000;
   int SINGLE_SET_MAX    = state->instance_encoding_limit;
   satset->set_size_limit = CREATED_MAX;
   bool reverse = state->inst_gen_reverse;
   PStack_p (*iter_init_fun)(Eval_p, int) = 
      reverse ? EvalTreeTraverseRevInit : EvalTreeTraverseInit;
   Eval_p   (*iter_next_fun)(PStack_p, int) = 
      reverse ? EvalTreeTraverseRevNext : EvalTreeTraverseNext;
   //This can be fine-tuned -- we might not want to include equations
   ClauseSet_p all_sets[] = 
         { 
            state->unprocessed, state->processed_non_units, 
            state->processed_neg_units, state->processed_pos_eqns,
            state->processed_pos_rules
         };
   FPIndex_p positive_atom_idx = FPIndexAlloc(IndexFP7Create, state->signature, 
                                              SubtermOLTreeFreeWrapper);
   
   PStack_p ig_clauses = PStackAlloc();
   ClausePos_p cl_pos  = ClausePosAlloc();
   for(int i=0; i<sizeof(all_sets)/sizeof(ClauseSet_p); i++)
   {
      ClauseSet_p set = all_sets[i];
      Eval_p      root = PDArrayElementP(set->eval_indices,0);
      int         added_from_set = 0;
      if(root)
      {
         PStack_p eval_iter = iter_init_fun(root, 0);
         while((root = iter_next_fun(eval_iter, 0)) && added_from_set <= SINGLE_SET_MAX)
         {
            add_to_inst(ig_clauses, root->object, cl_pos, positive_atom_idx, &added_from_set);  
         }
         EvalTreeTraverseExit(eval_iter);
      }
      else
      {
         for(Clause_p handle = set->anchor->succ; handle!=set->anchor
                      && added_from_set <= SINGLE_SET_MAX; handle = handle->succ)
         {
            add_to_inst(ig_clauses, handle, cl_pos, positive_atom_idx, &added_from_set);
         }
      }
      
   }
   ClausePosFree(cl_pos);

   Subst_p     ground_subst = SubstAlloc();

   fprintf(stdout, "# Found %ld good clauses.\n", PStackGetSP(ig_clauses));
   while(!PStackEmpty(ig_clauses))
   {
      Clause_p orig_cl = PStackPopP(ig_clauses);
      Clause_p cl =  ClauseCopyDisjoint(orig_cl);
      for(Eqn_p lit = cl->literals; lit; lit = lit->next)
      {
         if(!EqnIsPositive(lit))
         {
            PStack_p candidates = PStackAlloc();
            Term_p   query_term = sat_encode_lit(lit);
            TB_p     bank       = lit->bank;

            FPIndexFindUnifiable(positive_atom_idx, query_term, candidates);

            while(!PStackEmpty(candidates))
            {
               PStack_p iter = PTreeTraverseInit(PStackPopP(candidates));
               SubtermTree_p cell = NULL;

               while((cell = PTreeTraverseNext(iter)))
               {
                  Term_p candidate = ((SubtermOcc_p)cell->key)->term;
                  PObjTree_p clauses = ((SubtermOcc_p)cell->key)->pl.pos.clauses;           

                  if(candidate != query_term &&
                     (!UnifFailed(SubstMguPossiblyPartial(candidate, 
                                                          query_term, 
                                                          ground_subst, bank))))
                  {                     
                     sat_ground_term(query_term, ground_subst, bank);
                     sat_ground_term(candidate, ground_subst, bank);

                     PTree_p node;
                     PStack_p cl_iter = PTreeTraverseInit(clauses);

                     while((node = PTreeTraverseNext(cl_iter)))
                     {
                        ClauseTPos_p pos = node->key;
                        SatClauseCreateAndStore(pos->clause, satset);
                     }
                     PTreeTraverseExit(cl_iter);
                     SatClause_p res = SatClauseCreateAndStore(cl, satset);              
                     if(!res)
                     {
                        break;
                     }
                     
                     res->source = orig_cl;
                     SubstBacktrack(ground_subst);
                  }
               }
               PTreeTraverseExit(iter);
            }
            PStackFree(candidates);
         }
      }
      ClauseFree(cl);
   }
   PStackFree(ig_clauses);   
   SubstDelete(ground_subst);
   return SatClauseSetCardinality(satset);
}



/*-----------------------------------------------------------------------
//
// Function: SubstPseudoGroundVarBank()
//
//   Create a substitution binding all variables of a given sort to
//   the smallest (first) variable of that sort (to be interpreted as
//   an anonymous constant - this can be seen as a complete
//   (pseudo-)grounding of all terms, literals, and clauses using this
//   variable bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Subst_p SubstPseudoGroundVarBank(VarBank_p vars)
{
   Subst_p subst = SubstAlloc();
   VarBankStack_p varstack;
   long i, j, size;
   Term_p current, norm;

   assert(vars);

   for (i=0; i < PDArraySize(vars->varstacks); i++)
   {
      varstack = PDArrayElementP(vars->varstacks, i);
      // printf("# varstack: %p\n", varstack);
      if(varstack)
      {
         size = PStackGetSP(varstack);
         // printf("# varstack size: %ld\n", size);
         if(size)
         {
            norm = PStackElementP(varstack,0);
            // printf("# varstack[1]: %p\n", norm);
            for(j=0; j< size; j++)
            {
               current = PStackElementP(varstack,j);
               // printf("# varstack[%ld]: %p\n", j, current);
               if(current && !current->binding)
               {
                  SubstAddBinding(subst, current, norm);
               }
            }
         }
      }
   }
   return subst;
}


/*-----------------------------------------------------------------------
//
// Function: SubstGroundVarBankFirstConst()
//
//   Create a substitution binding each variable to the first constant
//   of the proper sort.
//
// Global Variables: 0
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Subst_p SubstGroundVarBankFirstConst(TB_p terms, bool norm_const)
{
   Subst_p subst = SubstAlloc();
   VarBank_p vars = terms->vars;
   VarBankStack_p varstack;
   long i, j, size;
   Term_p current, norm, backup;

   assert(vars);

   for (i=0; i < PDArraySize(vars->varstacks); i++)
   {
      varstack = PDArrayElementP(vars->varstacks, i);
      //printf("# varstack: %p\n", varstack);
      if (varstack)
      {
         size = PStackGetSP(varstack);
         //printf("# varstack size: %ld\n", size);
         if(size)
         {
            backup = PStackElementP(varstack,0);
            assert(backup->type->type_uid == i);

            norm = TBGetFirstConstTerm(terms, backup->type);
            if(!norm)
            {
               norm = backup;
            }
            else if(norm_const)
            {
               norm = TermFollowRWChain(norm);
            }
            // printf("# varstack[1]: %p\n", norm);
            for(j=0; j< size; j++)
            {
               current = PStackElementP(varstack,j);
               // printf("# varstack[%ld]: %p\n", j, current);
               if(current && !current->binding)
               {
                  SubstAddBinding(subst, current, norm);
               }
            }
         }
      }
   }
   return subst;
}


/*-----------------------------------------------------------------------
//
// Function: SubstGroundFreqBased()
//
//   Generate a grounding substitution using occurance-count based
//   preference functions.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Subst_p SubstGroundFreqBased(TB_p terms, ClauseSet_p clauses,
                             FunConstCmpFunType is_better, bool norm_const)
{
   Subst_p subst = SubstAlloc();
   VarBank_p vars = terms->vars;
   VarBankStack_p varstack;
   long sort, j, size, dist_arr_size;
   Term_p current, norm, backup;
   long *conj_dist_array, *dist_array;

   assert(vars);

   dist_arr_size = (terms->sig->f_count+1) * sizeof(long);
   conj_dist_array = SizeMalloc(dist_arr_size);
   dist_array      = SizeMalloc(dist_arr_size);
   memset(conj_dist_array, 0, dist_arr_size);
   memset(dist_array, 0, dist_arr_size);

   ClauseSetAddSymbolDistribution(clauses, dist_array);
   ClauseSetAddConjSymbolDistribution(clauses, conj_dist_array);

   for (sort=0; sort < PDArraySize(vars->varstacks); sort++)
   {
      varstack = PDArrayElementP(vars->varstacks, sort);
      //printf("# varstack: %p\n", varstack);
      if (varstack)
      {
         size = PStackGetSP(varstack);
         //printf("# varstack size: %ld\n", size);
         if(size)
         {
            backup = PStackElementP(varstack,0);
            assert(backup->type->type_uid == sort);

            norm = TBGetFreqConstTerm(terms, backup->type, 
                                      conj_dist_array, dist_array, is_better);
            if(!norm)
            {
               norm = backup;
            }
            else if(norm_const)
            {
               norm = TermFollowRWChain(norm);
            }
            // printf("# varstack[1]: %p\n", norm);
            for(j=0; j< size; j++)
            {
               current = PStackElementP(varstack,j);
               // printf("# varstack[%ld]: %p\n", j, current);
               if(current)
               {
                  SubstAddBinding(subst, current, norm);
               }
            }
         }
      }
   }
   SizeFree(conj_dist_array, dist_arr_size);
   SizeFree(dist_array, dist_arr_size);

   return subst;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseSetImportProofState()
//
//   Import the all pseudo-grounded clauses in the proof state into
//   satset.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long SatClauseSetImportProofState(SatClauseSet_p satset, ProofState_p state,
                                  GroundingStrategy strat, bool norm_const)
{
   long    res = 0;
   Subst_p pseudogroundsubst = NULL;
   assert(satset);
   assert(state);

   if(state->instance_encoding_limit != -1)
   {
      strat = GMGenInstances; // make sure strategy is forced
   }

   //printf("# SatClauseSetImportProofState()\n");

   switch(strat)
   {
   case GMPseudoVar:
         pseudogroundsubst = SubstPseudoGroundVarBank(state->terms->vars);
         break;
   case GMFirstConst:
         pseudogroundsubst = SubstGroundVarBankFirstConst(state->terms,
                                                          norm_const);
         break;
   case GMConjMinMinFreq:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_conj_min_min_freq,
                                                  norm_const);
         break;
   case GMConjMaxMinFreq:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_conj_max_min_freq,
                                                  norm_const);
         break;
   case GMConjMinMaxFreq:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_conj_min_max_freq,
                                                  norm_const);
         break;
   case GMConjMaxMaxFreq:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_conj_max_max_freq,
                                                  norm_const);
         break;
   case GMGlobalMax:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_global_max_freq,
                                                  norm_const);
         break;
   case GMGlobalMin:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_global_min_freq,
                                                  norm_const);
         break;
   case GMGenInstances:
         // before generating instances -- try grounding by the same
         // constant first
         pseudogroundsubst = SubstGroundVarBankFirstConst(state->terms,
                                                          norm_const);
         break;
   default:
         assert(false && "Unimplemented grounding strategy");
         break;
   }
   // printf("# Pseudogrounded()\n");


   if(strat == GMGenInstances)
   {
      SubstBacktrack(pseudogroundsubst);
      res = SatClauseSetImportInstGen(satset, state);
   }
   else
   {
      res += SatClauseSetImportClauseSet(satset, state->processed_pos_rules);
      res += SatClauseSetImportClauseSet(satset, state->processed_pos_eqns);
      res += SatClauseSetImportClauseSet(satset, state->processed_neg_units);
      res += SatClauseSetImportClauseSet(satset, state->processed_non_units);
      res += SatClauseSetImportClauseSet(satset, state->unprocessed);
   }   
   
   fprintf(stdout, "# SATCheck is checking %ld(%d) translated clauses.\n", 
                   res, satset->max_lit);
   SubstDelete(pseudogroundsubst);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: SatClauseSetMarkPure()
//
//   Mark all clauses in satset that have pure literals.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long SatClauseSetMarkPure(SatClauseSet_p satset)
{
   long pure_clauses = 0;
   int *litstate;
   PStackPointer i;
   SatClause_p clause;

   assert(satset);

   litstate = SizeMalloc(sizeof(int)*(satset->max_lit+1));
   memset(litstate, 0, sizeof(int)*(satset->max_lit+1));


   for(i=0; i< PStackGetSP(satset->set); i++)
   {
      clause = PStackElementP(satset->set, i);
      litstate_add_satclause(litstate, clause);
      clause->has_pure_lit = false;
   }

   for(i=0; i< PStackGetSP(satset->set); i++)
   {
      clause = PStackElementP(satset->set, i);
      if(litstate_check_pure(litstate, clause))
      {
         clause->has_pure_lit = true;
         pure_clauses++;
      }
   }
   SizeFree(litstate, sizeof(int)*(satset->max_lit+1));

   return pure_clauses;
}


/*-----------------------------------------------------------------------
//
// Function: sat_extract_core()
//
//   Extracts the original clauses pointing to the unsatisfiable core 
//   and pushes them onto core.
//
// Global Variables: -
//
// Side Effects    : Reads input
//
/----------------------------------------------------------------------*/

long sat_extract_core(SatClauseSet_p satset, PStack_p core, SatSolver_p solver)
{
   SatClause_p satclause;
   long        nr_exported = PStackGetSP(satset->exported);
   long        res = 0;

   for(long id=0; id<nr_exported; id++)
   {
      if(picosat_coreclause(solver, id))
      {
         if(id < PStackGetSP(satset->exported))
         {
            res++;
            satclause = PStackElementP(satset->exported, id);
            if(satclause->source)
            {
               PStackPushP(core, satclause->source);
            }
            else
            {
               fprintf(stderr, "# warning: generated using instance generation: ");            
            }
            SatClausePrint(stderr, satclause);  
            
         }
         else
         {
            Error("PicoSat returns impossible clause number",
                  INTERFACE_ERROR);
         }
      }
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseSetCheckUnsat()
//
//   Check the satset for unsatisfiability. Return the empty clause if
//   unsat can be shown, NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : Runs external SAT solver, file operations, ...
//
/----------------------------------------------------------------------*/

ProverResult SatClauseSetCheckUnsat(SatClauseSet_p satset, Clause_p *empty, 
                                    SatSolver_p solver)
{
   ProverResult res;
   Clause_p     parent;

   SatClauseSetMarkPure(satset);
   SatClauseSetExportToSolverNonPure(solver, satset);

   res = SAT_TO_E_RESULT(picosat_sat(solver, -1));

   if(res == PRUnsatisfiable)
   {
      PStack_p unsat_core = PStackAlloc();
      fprintf(GlobalOut, "# SatCheck found unsatisfiable ground set\n");
      *empty = EmptyClauseAlloc();
      sat_extract_core(satset, unsat_core, solver);
      satset->core_size = PStackGetSP(unsat_core);
      parent = PStackPopP(unsat_core);
      ClausePushDerivation(*empty, DCSatGen, parent, NULL);
      while(!PStackEmpty(unsat_core))
      {
         parent = PStackPopP(unsat_core);
         ClausePushDerivation(*empty, DCCnfAddArg, parent, NULL);
      }
      PStackFree(unsat_core);
   }
   else
   {
      assert(res == PRSatisfiable);
   }

   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
