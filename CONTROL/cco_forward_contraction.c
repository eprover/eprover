/*-----------------------------------------------------------------------

File  : cco_forward_contraction.c

Author: Stephan Schulz

Contents
 
  Functions that apply the processed clause sets to simplify or
  eleminate a potential new clause. Extracted from
  cco_proofproc.[ch]. 

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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
//   Apply all forward-contracting inferences to clause. Return false
//   it becomes trivial, return true otherwise. Does not delete
//   clause. Subsumed and trivial clauses are counted in the cells
//   pointed to by the 3rd and 4th argument. Provide dummies to avoid
//   this. 
//
// Global Variables: -
//
// Side Effects    : Marks maximals terms and literals in the clause.
//
/----------------------------------------------------------------------*/

static bool forward_contract_keep(ProofState_p state, ProofControl_p
				  control, Clause_p clause, ulong_c*
				  subsumed_count, ulong_c* trivial_count,
				  bool non_unit_subsumption,
				  RewriteLevel level)
{
   assert(clause);
   assert(state);

   ForwardModifyClause(state, control, clause, level);

   if(ClauseIsEmpty(clause))
   {
      return true;
   }
   
   if(control->ac_handling_active && ClauseIsACRedundant(clause))
   {
      if(!ClauseIsUnit(clause)||
	 (control->ac_handling==ACDiscardAll)||
	 ((control->ac_handling==ACKeepOrientable)&&
	  !EqnIsOriented(clause->literals)))
      {
	 (*trivial_count)++;
	 return false;
      }
      ClauseSetProp(clause, CPNoGeneration);
   }

   if(ClauseIsTautology(state->tmp_terms, clause))
      /* if(ClauseIsTrivial(clause)) */
   {
      (*trivial_count)++;
      return false;
   }
   assert(!ClauseIsTrivial(clause));
         
   if(ClauseLiteralNumber(clause)>1)
   {
      clause->weight = ClauseStandardWeight(clause);
   }
      
   if((clause->pos_lit_no &&        
       UnitClauseSetSubsumesClause(state->processed_pos_eqns,
				   clause)) ||
      (clause->neg_lit_no &&
       UnitClauseSetSubsumesClause(state->processed_neg_units,
				   clause)) ||
      (ClauseLiteralNumber(clause)>1 &&
       non_unit_subsumption &&
       ClauseSetSubsumesClause(state->processed_non_units, clause)))      
   {
      DEBUGMARK(PP_LOWDETAILS, "Clause subsumed!\n");
      (*subsumed_count)++;
      return false;
   }
   ClauseDelProp(clause, CPIsOriented);
   DoLiteralSelection(control, clause);
   if(!ClauseQueryProp(clause, CPIsOriented))
   {
      ClauseMarkMaximalLiterals(control->ocb, clause);
   }
   return true;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ForwardModifyClause()
//
//   Apply all modifying forward-inferences to clause.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void ForwardModifyClause(ProofState_p state, ProofControl_p control,
			 Clause_p clause, RewriteLevel level)
{
   int removed_lits;

   ClauseComputeLINormalform(control->ocb,
			     state->terms, clause,
			     state->demods, level,
			     control->prefer_general);

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
      
   /* Still forward simplification... */   
   if(clause->neg_lit_no)
   {
      ClausePositiveSimplifyReflect(state->processed_pos_eqns, clause);
   }
   if(clause->pos_lit_no)
   {
      ClauseNegativeSimplifyReflect(state->processed_neg_units, clause);
   }  
}


/*-----------------------------------------------------------------------
//
// Function: ForwardContractClause()
//
//   Apply all forward-contracting inferences to clause. Return false
//   and delete the clause if it becomes trivial, return true
//   otherwise. 
//
// Global Variables: -
//
// Side Effects    : Marks maximals terms and literals in the clause.
//
/----------------------------------------------------------------------*/

bool ForwardContractClause(ProofState_p state, ProofControl_p control,
			   Clause_p clause, bool non_unit_subsumption,
			   RewriteLevel level)
{
   bool res;

   DEBUGMARK(PP_HIGHDETAILS, "ForwardContractClause...\n");
   assert(clause);
   assert(state);
   
   res = forward_contract_keep(state, control, clause,
			       &(state->proc_forward_subsumed_count),
			       &(state->proc_trivial_count),
			       non_unit_subsumption, level);
   
   if(!res)
   {
      ClauseFree(clause);
   }
   DEBUGMARK(PP_HIGHDETAILS, "...ForwardContractClause\n");
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ForwardContractSet()
//
//   Apply the forward-contracting inferences to all clauses in
//   set. Delete redundant clauses. Return empty clause (if found), 
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
			    ulong_c* count_eleminated)
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
			       count_eleminated, count_eleminated,
			       non_unit_subsumption, level))
      {
	 if(ClauseIsEmpty(handle))
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
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseSetReweight(HCB_p heuristic, ClauseSet_p set)
{
   Clause_p    handle;
   ClauseSet_p tmp_set;

   assert(heuristic);
   assert(set);
   assert(!set->demod_index);
   

   ClauseSetRemoveEvaluations(set);
   tmp_set = ClauseSetAlloc();
   
   while(!ClauseSetEmpty(set))
   {
      ClauseSetInsert(tmp_set, ClauseSetExtractFirst(set));
   }
   while(!ClauseSetEmpty(tmp_set))
   {
      handle = ClauseSetExtractFirst(tmp_set);
      HCBClauseEvaluate(heuristic, handle);
      ClauseSetInsert(set, handle);
   }
   ClauseSetFree(tmp_set);
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
				    level, ulong_c* count_eleminated)
{
   Clause_p    handle;
   
   assert(state);
   assert(set);
   assert(!set->demod_index);
   
   handle = ForwardContractSet(state, control, set,
			       non_unit_subsumption, level,
			       count_eleminated);
   
   if(handle)
   {
      return handle;
   }
   ClauseSetReweight(control->hcb, set);

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
// Side Effects    : Changes set and *count_eleminated
//
/----------------------------------------------------------------------*/

void ClauseSetFilterReweigth(ProofControl_p
			     control, ClauseSet_p set,
			     ulong_c* count_eleminated)
{
   *count_eleminated += ClauseSetFilterTrivial(set);
   ClauseSetReweight(control->hcb, set);
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
				  &(state->proc_trivial_count));
	    break;
      case 'N':
	    handle = 
	       ForwardContractSet(state, control,
				  state->unprocessed,
				  true, NoRewrite,
				  &(state->proc_trivial_count));
	    break;
      case 'r':
	    handle = 
	       ForwardContractSet(state, control,
				  state->unprocessed,
				  false, RuleRewrite,
				  &(state->proc_trivial_count));
	    break;
      case 'R':
	    handle = 
	       ForwardContractSet(state, control,
				  state->unprocessed,
				  true, RuleRewrite,
				  &(state->proc_trivial_count));
	    break;
      case 'f':
	    handle = 
	       ForwardContractSet(state, control,
				  state->unprocessed,
				  false, FullRewrite,
				  &(state->proc_trivial_count));
	    break;
      case 'F':
	    handle = 
	       ForwardContractSet(state, control,
				  state->unprocessed,
				  true, FullRewrite,
				  &(state->proc_trivial_count));
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


