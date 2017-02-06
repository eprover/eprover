/*-----------------------------------------------------------------------

File  : cco_forward_contraction.c

Author: Stephan Schulz

Contents

  Functions that apply the processed clause sets to simplify or
  eliminate a potential new clause. Extracted from
  cco_proofproc.[ch].

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Nov  9 17:46:50 MET 1998
    New

-----------------------------------------------------------------------*/

#include "cco_forward_contraction.h"



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
// Function: forward_contract_keep()
//
//   Apply all forward-contracting inferences to clause. Return NULL
//   if it becomes trivial, a FVPackedClause containing it
//   otherwise. Does not delete clause. Subsumed and trivial clauses
//   are counted in the cells pointed to by the 4th and 5th
//   argument. Provide dummies to avoid this.
//
// Global Variables: -
//
// Side Effects    : Marks maximals terms and literals in the clause.
//
/----------------------------------------------------------------------*/

static FVPackedClause_p forward_contract_keep(ProofState_p state, ProofControl_p
                     control, Clause_p clause, unsigned long*
                     subsumed_count, unsigned long* trivial_count,
                     bool non_unit_subsumption,
                     bool context_sr,
                                              bool condense,
                     RewriteLevel level)
{
   FVPackedClause_p pclause;
   Clause_p subsumer = 0;
   bool trivial;

   assert(clause);
   assert(state);

   if(control->heuristic_parms.enable_given_forward_simpl)
   {
      trivial = ForwardModifyClause(state, control, clause,
                                    context_sr, condense, level);
      if(trivial)
      {
         (*trivial_count)++;
         return NULL;
      }

      if(ClauseIsEmpty(clause))
      {
         return FVIndexPackClause(clause, NULL);
      }

      if(control->ac_handling_active && ClauseIsACRedundant(clause))
      {
         if(!ClauseIsUnit(clause)||
            (control->heuristic_parms.ac_handling==ACDiscardAll)||
            ((control->heuristic_parms.ac_handling==ACKeepOrientable)&&
             !EqnIsOriented(clause->literals)))
         {
            (*trivial_count)++;
            return NULL;
         }
         ClauseSetProp(clause, CPNoGeneration);
      }

      if(ClauseIsTautology(state->tmp_terms, clause))
      {
         (*trivial_count)++;
         return NULL;
      }
      assert(!ClauseIsTrivial(clause));

      clause->weight = ClauseStandardWeight(clause);
      pclause = FVIndexPackClause(clause, state->processed_non_units->fvindex);

      if(clause->pos_lit_no)
      {
         subsumer = UnitClauseSetSubsumesClause(state->processed_pos_eqns, clause);
      }
      if(!subsumer && clause->neg_lit_no)
      {
         subsumer = UnitClauseSetSubsumesClause(state->processed_neg_units,
                                                clause);
      }
      if(!subsumer && (ClauseLiteralNumber(clause)>1) && non_unit_subsumption)
      {
         ClauseSubsumeOrderSortLits(clause);
         subsumer = ClauseSetSubsumesFVPackedClause(state->processed_non_units, pclause);
      }
      if(subsumer)
      {
         DocClauseQuote(GlobalOut, OutputLevel, 6, pclause->clause,
                        "subsumed", subsumer);
         (*subsumed_count)++;
         FVUnpackClause(pclause);
         ENSURE_NULL(pclause);
         return NULL;
      }
   }
   else /* !control->enable_given_forward_simpl -- this is just a
         * minimal subset of what is done above*/
   {
      if(ClauseIsEmpty(clause))
      {
         return FVIndexPackClause(clause, NULL);
      }
      clause->weight = ClauseStandardWeight(clause);
      pclause = FVIndexPackClause(clause, state->processed_non_units->fvindex);
   }
   ClauseDelProp(clause, CPIsOriented);
   DoLiteralSelection(control, clause);
   ClauseCondMarkMaximalTerms(control->ocb, clause);

   return pclause;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ForwardModifyClause()
//
//   Apply all modifying forward-inferences to clause (unless it
//   becomes trivial). Return true if it does become trivial.
//
// Global Variables: -
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

bool ForwardModifyClause(ProofState_p state,
                         ProofControl_p control,
          Clause_p clause,
                         bool context_sr,
                         bool condense,
                         RewriteLevel level)
{
   int removed_lits;
   bool done = false;
   bool limited_rw;
   bool condensed;

   while(!done)
   {
      ClauseComputeLINormalform(control->ocb,
                                state->terms, clause,
                                state->demods, level,
                                control->heuristic_parms.prefer_general);

      limited_rw = ClauseQueryProp(clause, CPLimitedRW);
      removed_lits = ClauseRemoveSuperfluousLiterals(clause);
      if(removed_lits)
      {
         DocClauseModificationDefault(clause, inf_minimize, NULL);
      }

      if(control->ac_handling_active)
      {
         ClauseRemoveACResolved(clause);
      }

      /* Now we mark maximal terms... */
      ClauseOrientLiterals(control->ocb, clause);

      if(condense)
      {
         condensed = Condense(clause);
         if(condensed)
         {
            ClauseOrientLiterals(control->ocb, clause);
         }
      }


      if(ClauseIsTrivial(clause))
      {
         return true;
      }

      /* Still forward simplification... */
      if(clause->neg_lit_no)
      {
         ClausePositiveSimplifyReflect(state->processed_pos_eqns, clause);
      }
      if(clause->pos_lit_no)
      {
         ClauseNegativeSimplifyReflect(state->processed_neg_units, clause);
      }
      if(context_sr && ClauseLiteralNumber(clause) > 1)
      {
         state->context_sr_count +=
            ClauseContextualSimplifyReflect(state->processed_non_units,
                                            clause);
      }
      done = ClauseQueryProp(clause, CPLimitedRW)==limited_rw;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ForwardContractClause()
//
//   Apply all forward-contracting inferences to clause. Return NULL
//   and delete the clause if it becomes trivial, return
//   FVPackedClause otherwise.
//
// Global Variables: -
//
// Side Effects    : Marks maximals terms and literals in the clause.
//
/----------------------------------------------------------------------*/

FVPackedClause_p ForwardContractClause(ProofState_p state,
                   ProofControl_p control,
                   Clause_p clause,
                   bool non_unit_subsumption,
                   bool context_sr,
                                       bool condense,
                   RewriteLevel level)
{
   FVPackedClause_p res;

   assert(clause);
   assert(state);

   res = forward_contract_keep(state, control, clause,
                &(state->proc_forward_subsumed_count),
                &(state->proc_trivial_count),
                non_unit_subsumption, context_sr, condense,
                level);

   if(!res)
   {
      ClauseFree(clause);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ForwardContractSet()
//
//   Apply the forward-contracting inferences to all clauses in
//   set. Delete redundant clauses. If terminate_on_empty is true,
//   return empty clause (if found),
//   NULL otherwise. The empty clause will be extracted from set,
//   which may not be fully contracted in this case.
//
// Global Variables: -
//
// Side Effects    : Changes clauses and set.
//
/----------------------------------------------------------------------*/

Clause_p ForwardContractSet(ProofState_p state, ProofControl_p
             control, ClauseSet_p set, bool
             non_unit_subsumption, RewriteLevel level,
             unsigned long* count_eliminated, bool
             terminate_on_empty)
{
   Clause_p handle, next;

   assert(state);
   assert(set);
   assert(!set->demod_index);

   handle = set->anchor->succ;
   while(handle != set->anchor)
   {
      next = handle->succ;

      assert(handle);

      if(forward_contract_keep(state, control, handle,
                count_eliminated, count_eliminated,
                non_unit_subsumption, false, false, level))
      {
    if(terminate_on_empty&&ClauseIsEmpty(handle))
    {
       ClauseSetExtractEntry(handle);
       return handle;
    }
      }
      else
      {
    ClauseDetachParents(handle);
    ClauseSetDeleteEntry(handle);
      }
      handle = next;
   }
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetReweight()
//
//   Re-Evaluate all clauses in set.
//
// Args:
//   heuristic: heuristic by which clauses should be reweighted
//   set: clauses to remove and reweight
//   is_ctrl_hcb: set if the heuristic is from the proof control
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/



void ClauseSetReweight(HCB_p heuristic, ClauseSet_p set, bool is_ctrl_hcb)
{
   PQueue_p tmp_queue;

   assert(heuristic);
   assert(set);
   assert(!set->demod_index);


   ClauseSetRemoveEvaluations(set);
   tmp_queue = PQueueAlloc();

   while(!ClauseSetEmpty(set))
   {
      PQueueStoreP(tmp_queue, ClauseSetExtractFirst(set));
   }

   ClauseSetEvalInsertQueue(set, tmp_queue, heuristic, is_ctrl_hcb, 0);

   PQueueFree(tmp_queue);
}


/*-----------------------------------------------------------------------
//
// Function:  ForwardContractSetReweight()
//
//   Apply contracting inferences to all claues in set, then
//   reevaluate them. Return empty clause (if found), NULL
//   otherwise. The empty clause will be extracted from set, which may
//   not be fully contracted in this case.
//
// Global Variables: -
//
// Side Effects    :  Changes clauses and set.
//
/----------------------------------------------------------------------*/

Clause_p ForwardContractSetReweight(ProofState_p state, ProofControl_p
                control, ClauseSet_p set, bool
                non_unit_subsumption, RewriteLevel
                level, unsigned long* count_eliminated)
{
   Clause_p    handle;

   assert(state);
   assert(set);
   assert(!set->demod_index);

   handle = ForwardContractSet(state, control, set,
                non_unit_subsumption, level,
                count_eliminated, true);

   if(handle)
   {
      return handle;
   }
   ClauseSetReweight(control->hcb, set, true);

   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFilterReweigth()
//
//   Remove all trivial clauses from set and reweigth it.
//
// Global Variables: -
//
// Side Effects    : Changes set and *count_eliminated
//
/----------------------------------------------------------------------*/

void ClauseSetFilterReweigth(ProofControl_p
              control, ClauseSet_p set,
              unsigned long* count_eliminated)
{
   *count_eliminated += ClauseSetFilterTrivial(set);
   ClauseSetReweight(control->hcb, set, true);
}



/*-----------------------------------------------------------------------
//
// Function: ProofStateFilterUnprocessed()
//
//   Apply various filter operations (guided by *desc) to the
//   set of unprocessed clauses in state. Return the empty clause (and
//   stop filtering) if it was found, otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : Changes clause set.
//
/----------------------------------------------------------------------*/

Clause_p ProofStateFilterUnprocessed(ProofState_p state,
                 ProofControl_p control, char*
                 desc)
{
   Clause_p handle = NULL;

   while(*desc)
   {
      switch(*desc)
      {
      case 'u':
       state->non_redundant_deleted+=
          ClauseSetDeleteNonUnits(state->unprocessed);
       break;
      case 'c':
       state->other_redundant_count+=
          ClauseSetDeleteCopies(state->unprocessed);
       break;
      case 'n':
       handle =
          ForwardContractSet(state, control,
              state->unprocessed,
              false, NoRewrite,
              &(state->proc_trivial_count), true);
       break;
      case 'N':
       handle =
          ForwardContractSet(state, control,
              state->unprocessed,
              true, NoRewrite,
              &(state->proc_trivial_count), true);
       break;
      case 'r':
       handle =
          ForwardContractSet(state, control,
              state->unprocessed,
              false, RuleRewrite,
              &(state->proc_trivial_count), true);
       break;
      case 'R':
       handle =
          ForwardContractSet(state, control,
              state->unprocessed,
              true, RuleRewrite,
              &(state->proc_trivial_count), true);
       break;
      case 'f':
       handle =
          ForwardContractSet(state, control,
              state->unprocessed,
              false, FullRewrite,
              &(state->proc_trivial_count), true);
       break;
      case 'F':
       handle =
          ForwardContractSet(state, control,
              state->unprocessed,
              true, FullRewrite,
              &(state->proc_trivial_count), true);
       break;
      }
      if(handle)
      {
    break;
      }
      desc++;
   }
   return handle;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


