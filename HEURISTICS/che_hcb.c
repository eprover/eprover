/*-----------------------------------------------------------------------

  File  : che_hcb.c

  Author: Stephan Schulz

  Contents

  Functions for the administration of HCBs.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Fri Oct 16 14:52:53 MET DST 1998

  -----------------------------------------------------------------------*/

#include "che_hcb.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

PERF_CTR_DEFINE(ClauseEvalTimer);


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: get_next_clause()
//
//   Return the next clause from the selected EvalTreeTraverse-Stack,
//   or NULL if the stack is empty.
//
// Global Variables: -
//
// Side Effects    : Changes stack
//
/----------------------------------------------------------------------*/

static Clause_p get_next_clause(PStack_p *stacks, int pos)
{
   Eval_p current;

   current = EvalTreeTraverseNext(stacks[pos], pos);
   if(current)
   {
      return current->object;
   }
   return NULL;
}


void default_exit_fun(void* data)
{
   return;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsInitialize()
//
//   Initialize a heuristic parameters cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void HeuristicParmsInitialize(HeuristicParms_p handle)
{
   handle->heuristic_name                = HCB_DEFAULT_HEURISTIC;
   handle->prefer_initial_clauses        = false;

   OrderParmsInitialize(&(handle->order_params));

   handle->selection_strategy            = SelectNoLiterals;
   handle->pos_lit_sel_min               = 0;
   handle->pos_lit_sel_max               = LONG_MAX;
   handle->neg_lit_sel_min               = 0;
   handle->neg_lit_sel_max               = LONG_MAX;
   handle->all_lit_sel_min               = 0;
   handle->all_lit_sel_max               = LONG_MAX;
   handle->weight_sel_min                = 0;
   handle->select_on_proc_only           = false;
   handle->inherit_paramod_lit           = false;
   handle->inherit_goal_pm_lit           = false;
   handle->inherit_conj_pm_lit           = false;

   handle->enable_eq_factoring           = true;
   handle->enable_neg_unit_paramod       = true;
   handle->enable_given_forward_simpl    = true;
   handle->pm_type                       = ParamodPlain;

   handle->ac_handling                   = ACDiscardAll;
   handle->ac_res_aggressive             = true;

   handle->forward_context_sr            = false;
   handle->forward_context_sr_aggressive = false;
   handle->backward_context_sr           = false;

   handle->forward_demod                 = FullRewrite;
   handle->prefer_general                = false;

   handle->er_varlit_destructive         = false;
   handle->er_strong_destructive         = false;
   handle->er_aggressive                 = false;

   handle->condensing                    = false;
   handle->condensing_aggressive         = false;

   handle->split_clauses                 = SplitNone;
   handle->split_method                  = SplitGroundNone;
   handle->split_aggressive              = false;
   handle->split_fresh_defs              = true;

   strcpy(handle->rw_bw_index_type, DEFAULT_RW_BW_INDEX_NAME);
   strcpy(handle->pm_from_index_type, DEFAULT_PM_FROM_INDEX_NAME);
   strcpy(handle->pm_into_index_type, DEFAULT_PM_INTO_INDEX_NAME);
   strcpy(handle->watchlist_unit_clause_index_type, DEFAULT_WATCHLIST_UNIT_CLAUSE_INDEX_NAME);

   handle->wl_abstract_constant_sym      = false;
   handle->wl_abstract_skolem_sym        = false;

   handle->sat_check_grounding           = GMNoGrounding;
   handle->sat_check_step_limit          = LONG_MAX;
   handle->sat_check_size_limit          = LONG_MAX;
   handle->sat_check_ttinsert_limit      = LONG_MAX;
   handle->sat_check_normconst           = false;
   handle->sat_check_normalize           = false;
   handle->sat_check_decision_limit      = 10000;
   handle->filter_orphans_limit          = DEFAULT_FILTER_ORPHANS_LIMIT;
   handle->forward_contract_limit        = DEFAULT_FORWARD_CONTRACT_LIMIT;
   handle->delete_bad_limit              = DEFAULT_DELETE_BAD_LIMIT;
   handle->mem_limit                     = 0;
   handle->watchlist_simplify            = true;
   handle->watchlist_is_static           = false;
   handle->use_tptp_sos                  = false;
   handle->presat_interreduction         = false;
   handle->detsort_bw_rw                 = false;
   handle->detsort_tmpset                = false;
}


/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsAlloc()
//
//   Allocate a cell for parameters, with initialized empty stacks.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

HeuristicParms_p HeuristicParmsAlloc(void)
{
   HeuristicParms_p handle = HeuristicParmsCellAlloc();

   HeuristicParmsInitialize(handle);
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsFree()
//
//   Free a parameter cell.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void HeuristicParmsFree(HeuristicParms_p junk)
{
   assert(junk);

   junk->heuristic_name = NULL;
   /* PStackFree(junk->wfcb_definitions);
      PStackFree(junk->hcb_definitions);*/

   HeuristicParmsCellFree(junk);
}



/*-----------------------------------------------------------------------
//
// Function: HCBAlloc()
//
//   Return an empty, initialized HCB.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

HCB_p HCBAlloc(void)
{
   HCB_p handle = HCBCellAlloc();

   handle->wfcb_list     = PDArrayAlloc(4,4);
   handle->wfcb_no       = 0;
   handle->current_eval  = 0;
   handle->select_switch = PDArrayAlloc(4,4);
   handle->select_count  = 0;
   handle->hcb_select    = HCBStandardClauseSelect;
   handle->hcb_exit      = default_exit_fun;
   handle->data          = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: HCBFree()
//
//   Free a heuristics control block.
//
// Global Variables: -
//
// Side Effects    : Memory operation, releases WFCB's
//
/----------------------------------------------------------------------*/

void HCBFree(HCB_p junk)
{
   assert(junk);

   /* WFCB's are _not_ freed, as they come from a WFCBAdmin-Block
      anyways! */
   PDArrayFree(junk->wfcb_list);
   PDArrayFree(junk->select_switch);
   if(junk->data)
   {
      junk->hcb_exit(junk->data);
   }
   HCBCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: HCBAddWFCB()
//
//   Add a WFCB with to the HCB, adjust selection function. Return
//   number of weight functions in HCB.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long HCBAddWFCB(HCB_p hcb, WFCB_p wfcb, long steps)
{
   assert(hcb);
   assert(wfcb);
   assert(steps >0);

   if(hcb->wfcb_no)
   {
      steps+= PDArrayElementInt(hcb->select_switch, hcb->wfcb_no-1);
   }
   PDArrayAssignP(hcb->wfcb_list, hcb->wfcb_no, wfcb);
   PDArrayAssignInt(hcb->select_switch, hcb->wfcb_no, steps);
   hcb->wfcb_no++;

   hcb->hcb_select = (hcb->wfcb_no != 1) ?
      HCBStandardClauseSelect : HCBSingleWeightClauseSelect;

   return hcb->wfcb_no;
}



/*-----------------------------------------------------------------------
//
// Function: HCBClauseEvaluate()
//
//   Giben a HCB-Block, add evaluations to the given clause.
//
// Global Variables: -
//
// Side Effects    : Memory, adds evaluations, by eval functions
//
/----------------------------------------------------------------------*/

void HCBClauseEvaluate(HCB_p hcb, Clause_p clause)
{
   long i;
   bool empty;

   PERF_CTR_ENTRY(ClauseEvalTimer);
   assert(clause->evaluations == NULL);
   ClauseAddEvalCell(clause, EvalsAlloc(hcb->wfcb_no));

   empty = ClauseIsSemFalse(clause);
   for(i=0; i< hcb->wfcb_no; i++)
   {
      ClauseAddEvaluation(PDArrayElementP(hcb->wfcb_list, i), clause, i, empty);
   }
   PERF_CTR_EXIT(ClauseEvalTimer);
}

/*-----------------------------------------------------------------------
//
// Function: HCBStandardClauseSelect()
//
//   Select a clause from set, based on the evaluations and the data
//   in hcb.
//
// Global Variables: -
//
// Side Effects    : Modifies HCB data
//
/----------------------------------------------------------------------*/

Clause_p HCBStandardClauseSelect(HCB_p hcb, ClauseSet_p set)
{
   Clause_p clause;

   clause = ClauseSetFindBest(set, hcb->current_eval);
   while(clause && ClauseIsOrphaned(clause))
   {
      ClauseSetExtractEntry(clause);
      ClauseFree(clause);
      clause = ClauseSetFindBest(set, hcb->current_eval);
   }
   hcb->select_count++;
   while(hcb->select_count ==
         PDArrayElementInt(hcb->select_switch,hcb->current_eval))
   {
      hcb->current_eval++;
   }
   if(hcb->current_eval == hcb->wfcb_no)
   {
      hcb->select_count = 0;
      hcb->current_eval = 0;
   }
   return clause;
}


/*-----------------------------------------------------------------------
//
// Function: HCBSingleWeightClauseSelect()
//
//   Select a clause from the set based on the first weight.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Clause_p HCBSingleWeightClauseSelect(HCB_p hcb, ClauseSet_p set)
{
   Clause_p clause;

   clause = ClauseSetFindBest(set, hcb->current_eval);
   while(clause && ClauseIsOrphaned(clause))
   {
      ClauseSetExtractEntry(clause);
      ClauseFree(clause);
      clause = ClauseSetFindBest(set, hcb->current_eval);
   }
   return clause;
}


/*-----------------------------------------------------------------------
//
// Function: HCBClauseSetDelProp()
//
//   Delete the property prop from the first number clauses in set
//   that would be picked according to hcb. Note that this is _not_
//   reliable, as in real processing, clauses that would have been
//   picked may vanish due to missing parents. It should be a fairly
//   good approximation, though.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes flags.
//
/----------------------------------------------------------------------*/

long HCBClauseSetDelProp(HCB_p hcb, ClauseSet_p set, long number,
                         FormulaProperties prop)
{
   int i, j;
   long prop_cleared = 0;
   PStack_p *stacks = SizeMalloc(hcb->wfcb_no*sizeof(PStack_p));
   Clause_p clause;

   for(i=0; i< hcb->wfcb_no; i++)
   {
      stacks[i]=
         EvalTreeTraverseInit(PDArrayElementP(set->eval_indices, i),i);
   }
   while(number)
   {
      for(i=0; i < hcb->wfcb_no; i++)
      {
         for(j=0; j < PDArrayElementInt(hcb->select_switch, j); j++)
         {
            while((clause =
                   get_next_clause(stacks,i)))
            {
               if(ClauseQueryProp(clause, prop))
               {
                  ClauseDelProp(clause, prop);
                  prop_cleared++;
                  break;
               }
            }
            number--; /* We did our best - this is an easy catch for
                         the stupid case number > set->members */
            if(!number)
            {
               break;
            }
         }
         if(!number)
         {
            break;
         }
      }
   }
   for(i=0; i< hcb->wfcb_no; i++)
   {
      EvalTreeTraverseExit(stacks[i]);
   }
   SizeFree(stacks, hcb->wfcb_no*sizeof(PStack_p));

   return prop_cleared;
}


/*-----------------------------------------------------------------------
//
// Function: HCBClauseSetDeleteBadClauses()
//
//   Delete all but the best number clauses from the set.
//
// Global Variables: -
//
// Side Effects    : Changes set.
//
/----------------------------------------------------------------------*/

long HCBClauseSetDeleteBadClauses(HCB_p hcb, ClauseSet_p set, long
                                  number)
{
   long res;

   ClauseSetSetProp(set, CPDeleteClause);
   HCBClauseSetDelProp(hcb, set, number, CPDeleteClause);
   res = ClauseSetDeleteMarkedEntries(set);

   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
