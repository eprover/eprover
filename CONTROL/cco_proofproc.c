/*-----------------------------------------------------------------------

File  : cco_proofproc.c

Author: Stephan Schulz

Contents
 
  Functions realizing the proof procedure.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 11:47:44 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "cco_proofproc.h"



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
// Function: check_ac_status()
//
//   Check if the AC theory has been extended by the currently
//   processed clause, and act accordingly.
//
// Global Variables: -
//
// Side Effects    : Changes AC status in signature
//
/----------------------------------------------------------------------*/

static void check_ac_status(ProofState_p state, ProofControl_p
			     control, Clause_p clause)
{
   if(control->ac_handling!=NoACHandling)
   {
      bool res;
      res = ClauseScanAC(state->signature, clause);
      if(res && !control->ac_handling_active)
      {
	 control->ac_handling_active = true;
	 if(OutputLevel)
	 {      
	    SigPrintACStatus(GlobalOut, state->signature);
	    fprintf(GlobalOut, "# AC handling enabled dynamically\n");
	 }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: remove_subsumed()
//
//   Remove all clauses subsumed by subsumer from set, kill their
//   children. Return number of removed clauses.
//
// Global Variables: -
//
// Side Effects    : Changes set, memory operations.
//
/----------------------------------------------------------------------*/

static long remove_subsumed(FVPackedClause_p subsumer, ClauseSet_p set)
{
   Clause_p handle;
   long     res;
   PStack_p stack = PStackAlloc();
            
   res = ClauseSetFindFVSubsumedClauses(set, subsumer, stack);
   
   while(!PStackEmpty(stack))
   {
      DEBUGMARK(PP_HIGHDETAILS, "*\n");
      handle = PStackPopP(stack);
      if(ClauseQueryProp(handle, CPWatchOnly))
      {
	 DocClauseQuote(GlobalOut, OutputLevel, 6, handle,
			"subsumed_watchlist", subsumer->clause);
      }
      else
      {
	 DocClauseQuote(GlobalOut, OutputLevel, 6, handle,
			"subsumed", subsumer->clause);
      }
      ClauseKillChildren(handle);
      ClauseSetDeleteEntry(handle);  
   }
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: eleminate_backward_rewritten_clauses()
//
//   Remove all processed clauses rewritable with clause and put
//   them into state->tmp_store.
//
// Global Variables: -
//
// Side Effects    : Changes clause sets
//
/----------------------------------------------------------------------*/

static bool
eleminate_backward_rewritten_clauses(ProofState_p
				     state, ProofControl_p control,
				     Clause_p clause, SysDate *date)
{
   long old_lit_count = state->tmp_store->literals,
      old_clause_count= state->tmp_store->members;
   bool min_rw = false;

   if(ClauseIsDemodulator(clause))
   {
      SysDateInc(date);
      min_rw = RemoveRewritableClauses(control->ocb,
				       state->processed_pos_rules,
				       state->tmp_store,
				       clause, *date)
	 ||min_rw;
      min_rw = RemoveRewritableClauses(control->ocb,
				       state->processed_pos_eqns,
				       state->tmp_store,
				       clause, *date)
	 ||min_rw;
      min_rw = RemoveRewritableClauses(control->ocb, 
				       state->processed_neg_units,
				       state->tmp_store,
				       clause, *date)
	 ||min_rw;
      min_rw = RemoveRewritableClauses(control->ocb, 
				       state->processed_non_units,
				       state->tmp_store,
				       clause, *date)
	 ||min_rw;
      state->backward_rewritten_lit_count+=
	 (state->tmp_store->literals-old_lit_count);
      state->backward_rewritten_count+=
	 (state->tmp_store->members-old_clause_count);
   }
   /*printf("# Removed %ld clauses\n",
     (state->tmp_store->members-old_clause_count));*/
   return min_rw;
}
   

/*-----------------------------------------------------------------------
//
// Function: eleminate_backward_subsumed_clauses()
//
//   Eleminate subsumed processed clauses, return number of clauses
//   deleted. 
//
// Global Variables: -
//
// Side Effects    : Changes clause sets.
//
/----------------------------------------------------------------------*/

static long eleminate_backward_subsumed_clauses(ProofState_p state,
						FVPackedClause_p pclause)
{
   long res = 0;

   if(ClauseLiteralNumber(pclause->clause) == 1)
   {
      if(pclause->clause->pos_lit_no)
      {
	 /* res += remove_subsumed(clause,
	    state->processed_pos_rules); Should be impossible! */
	 res += remove_subsumed(pclause, state->processed_pos_eqns);
	 res += remove_subsumed(pclause, state->processed_non_units);
      }
      else
      {
	 res += remove_subsumed(pclause, state->processed_neg_units);
	 res += remove_subsumed(pclause, state->processed_non_units);
      }
   }
   else
   {
      res += remove_subsumed(pclause, state->processed_non_units);
   }
   state->backward_subsumed_count+=res;
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: eleminate_unit_simplified_clauses()
//
//   Perform unit-back-simplification on the proof state.
//
// Global Variables: -
//
// Side Effects    : Potentially changes and moves clauses.
//
/----------------------------------------------------------------------*/

static void eleminate_unit_simplified_clauses(ProofState_p state,
					      Clause_p clause)
{
   if(ClauseIsRWRule(clause)||!ClauseIsUnit(clause))
   {
      return;
   }
   ClauseSetUnitSimplify(state->processed_non_units, clause,
			 state->tmp_store);
   if(ClauseIsPositive(clause))
   {
      ClauseSetUnitSimplify(state->processed_neg_units, clause,
			    state->tmp_store);
   }
   else
   {
      ClauseSetUnitSimplify(state->processed_pos_rules, clause,
			    state->tmp_store);
      ClauseSetUnitSimplify(state->processed_pos_eqns, clause,
			    state->tmp_store);
   }
}

/*-----------------------------------------------------------------------
//
// Function: eleminate_context_sr_clauses()
//
//   If required by control, remove all
//   backward-contextual-simplify-reflectable clauses.
//
// Global Variables: -
//
// Side Effects    : Moves clauses from state->processed_non_units 
//                   to state->tmp_store
//
/----------------------------------------------------------------------*/

static long eleminate_context_sr_clauses(ProofState_p state, 
					 ProofControl_p control, 
					 Clause_p clause)
{
   if(!control->backward_context_sr)
   {
      return 0;
   }
   return RemoveContextualSRClauses(state->processed_non_units,
				    state->tmp_store, clause);
}

/*-----------------------------------------------------------------------
//
// Function: check_watchlist()
//
//   Check if a clause subsumes one or more watchlist clauses, if yes,
//   set appropriate property in clause and remove subsumed clauses.
//
// Global Variables: -
//
// Side Effects    : As decribed.
//
/----------------------------------------------------------------------*/

void check_watchlist(ClauseSet_p watchlist, Clause_p clause)
{
   FVPackedClause_p pclause = FVIndexPackClause(clause, watchlist->fvindex);
   long removed;
   
   clause->weight = ClauseStandardWeight(clause);
   if((removed = remove_subsumed(pclause, watchlist)))
   {
      ClauseSetProp(clause, CPSubsumesWatch);
      if(OutputLevel == 1)
      {
	 fprintf(GlobalOut,"# Watchlist reduced by %ld clause%s\n",
		 removed,removed==1?"":"s");
      }
   }
   FVUnpackClause(pclause);   
}



/*-----------------------------------------------------------------------
//
// Function: generate_new_clauses()
//
//   Apply the generating inferences to the proof state, putting new
//   clauses into state->tmp_store.
//
// Global Variables: -
//
// Side Effects    : Changes proof state as described.
//
/----------------------------------------------------------------------*/

static void generate_new_clauses(ProofState_p state, ProofControl_p
				 control, Clause_p clause, Clause_p tmp_copy)
{
   state->factor_count+=
      ComputeAllEqualityFactors(state->terms, control->ocb, clause,
				state->tmp_store, state->freshvars);
   state->resolv_count+=
      ComputeAllEqnResolvents(state->terms, clause, state->tmp_store,
			      state->freshvars);
   state->paramod_count+=
      ComputeAllParamodulants(state->terms, control->ocb,
			      tmp_copy, clause,
			      state->processed_pos_rules,
			      state->tmp_store, state->freshvars);
   state->paramod_count+=
      ComputeAllParamodulants(state->terms, control->ocb,
			      tmp_copy, clause,
			      state->processed_pos_eqns,
			      state->tmp_store, state->freshvars); 
   if(!ClauseIsNegative(clause))
   { /* We don't need to try to overlap purely negative clauses! */
      state->paramod_count+=
	 ComputeAllParamodulants(state->terms, control->ocb,
				 tmp_copy, clause,
				 state->processed_neg_units,
				 state->tmp_store, state->freshvars);
   }
   state->paramod_count+=
      ComputeAllParamodulants(state->terms, control->ocb,
			      tmp_copy, clause,
			      state->processed_non_units, 
			      state->tmp_store, state->freshvars);
}

/*-----------------------------------------------------------------------
//
// Function: insert_new_clauses()
//
//   Rewrite clauses in state->tmp_store, remove superfluous literals,
//   insert them into state->unprocessed. If an empty clause is
//   detected, return it, otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : As described.
//
/----------------------------------------------------------------------*/

static Clause_p insert_new_clauses(ProofState_p state, ProofControl_p control)
{
   Clause_p handle;
   long     clause_count;

   state->generated_count+=state->tmp_store->members;	 
   state->generated_lit_count+=state->tmp_store->literals;
   while((handle = ClauseSetExtractFirst(state->tmp_store)))
   {
      ForwardModifyClause(state, control, handle, 
			  control->forward_context_sr_aggressive||
			  (control->backward_context_sr&&
			   ClauseQueryProp(handle,CPIsProcessed)),
			  control->forward_demod);
      if(ClauseIsTrivial(handle)||
	 (control->unproc_simplify&&
	  !ClauseSimplifyWithUnitSet(handle, state->unprocessed,
				     control->unproc_simplify)))
      {
	 assert(!handle->children);
	 ClauseDetachParents(handle);
	 ClauseFree(handle);
	 continue;
      }
      if(state->watchlist)
      {
	 check_watchlist(state->watchlist, handle);
      }
      if(ClauseIsEmpty(handle))
      {
	 return handle;	 
      }
      if(control->er_aggressive &&
	 control->er_varlit_destructive &&
	 (clause_count = ClauseERNormalizeVar(state->terms,
					      handle,
					      state->tmp_store,
					      state->freshvars,
					      control->er_strong_destructive)))
      {
	 state->other_redundant_count += clause_count;
	 state->resolv_count += clause_count;
	 state->generated_count += clause_count;
	 continue;
      }
      if(control->split_aggressive &&
	 (clause_count = ControlledClauseSplit(handle,
					       state->tmp_store,
					       control->split_clauses,
					       control->split_method)))
      {
	 state->generated_count += clause_count;
	 continue;
      }
      state->non_trivial_generated_count++;
      ClauseDelProp(handle, CPIsOriented);
      if(!control->select_on_proc_only)
      {
	 DoLiteralSelection(control, handle); 
      }
      else
      {
	 EqnListDelProp(handle->literals, EPIsSelected);
      }
      handle->create_date = state->proc_non_trivial_count;
      HCBClauseEvaluate(control->hcb, handle);
      ClauseDelProp(handle, CPIsOriented);
      DocClauseQuoteDefault(6, handle, "eval");
      if(control->unproc_simplify && ClauseIsUnit(handle))
      {
	 ClauseSetPDTIndexedInsert(state->unprocessed, handle);	    
      }
      else
      {
	 ClauseSetInsert(state->unprocessed, handle);
      }
   }
   return NULL;
}

#ifdef PRINT_SHARING

/*-----------------------------------------------------------------------
//
// Function: print_sharing_factor()
//
//   Determine the sharing factor and print it. Potentially expensive,
//   only useful for manual analysis.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

static void print_sharing_factor(ProofState_p state)
{
   long shared, unshared;
   shared = TBTermNodes(state->terms);
   unshared = ClauseSetGetTermNodes(state->tmp_store)+
      ClauseSetGetTermNodes(state->processed_pos_rules)+
      ClauseSetGetTermNodes(state->processed_pos_eqns)+
      ClauseSetGetTermNodes(state->processed_neg_units)+
      ClauseSetGetTermNodes(state->processed_non_units)+
      ClauseSetGetTermNodes(state->unprocessed);
   
   fprintf(GlobalOut, "\n#        GClauses   NClauses    NShared  "
	   "NUnshared    TShared  TUnshared TSharinF   \n");
   fprintf(GlobalOut, "# grep %10ld %10ld %10ld %10ld %10ld %10ld %10.3f\n",
	   state->generated_count - state->backward_rewritten_count,
	   state->tmp_store->members,
	   ClauseSetGetSharedTermNodes(state->tmp_store),
	   ClauseSetGetTermNodes(state->tmp_store),
	   shared,
	   unshared,
	   (float)unshared/shared);
}
#endif


#ifdef PRINT_RW_STATE

/*-----------------------------------------------------------------------
//
// Function: print_rw_state()
//
//   Print the system (R,E,NEW), e.g. the two types of demodulators
//   and the newly generated clauses.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void print_rw_state(ProofState_p state)
{
   char prefix[20];

   sprintf(prefix, "RWD%09ld_R: ",state->proc_non_trivial_count);
   ClauseSetPrintPrefix(GlobalOut,prefix,state->processed_pos_rules);
   sprintf(prefix, "RWD%09ld_E: ",state->proc_non_trivial_count);
   ClauseSetPrintPrefix(GlobalOut,prefix,state->processed_pos_eqns);
   sprintf(prefix, "RWD%09ld_N: ",state->proc_non_trivial_count);
   ClauseSetPrintPrefix(GlobalOut,prefix,state->tmp_store);
}

#endif


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: ProofControlInit()
//
//   Initialize a proof control cell for a given proof state (with at
//   least axioms and signature) and a set of parameters
//   describing the ordering and heuristics.
//
// Global Variables: -
//
// Side Effects    : Sets specs.
//
/----------------------------------------------------------------------*/

void ProofControlInit(ProofState_p state,ProofControl_p control,
		      HeuristicParms_p params)
{
   PStackPointer sp;
   Scanner_p in;

   assert(control && control->wfcbs);
   assert(state && state->axioms && state->signature);
   assert(params && params->wfcb_definitions &&
	  params->hcb_definitions);
   assert(!control->ocb);
   assert(!control->hcb);
   
   SpecFeaturesCompute(&(control->problem_specs),
		       state->axioms,state->signature);

   control->ocb = TOSelectOrdering(state, params,
				   &(control->problem_specs));

   in = CreateScanner(StreamTypeInternalString,
		      DefaultWeightFunctions,
		      true, NULL);
   WeightFunDefListParse(control->wfcbs, in, control->ocb, state);
   DestroyScanner(in);
   for(sp = 0; sp < PStackGetSP(params->wfcb_definitions); sp++)
   {
      in = CreateScanner(StreamTypeOptionString,
			 PStackElementP(params->wfcb_definitions, sp) ,
			 true, NULL);
      WeightFunDefListParse(control->wfcbs, in, control->ocb, state);
      DestroyScanner(in);
   }
   in = CreateScanner(StreamTypeInternalString,
		      DefaultHeuristics,
		      true, NULL);
   HeuristicDefListParse(control->hcbs, in, control->wfcbs,
			 control->ocb, state); 
   DestroyScanner(in);
   for(sp = 0; sp < PStackGetSP(params->hcb_definitions); sp++)
   {
      in = CreateScanner(StreamTypeOptionString,
			 PStackElementP(params->hcb_definitions, sp) ,
			 true, NULL);
      HeuristicDefListParse(control->hcbs, in, control->wfcbs,
			    control->ocb, state); 
      DestroyScanner(in);
   }
   control->forward_demod       = params->forward_demod;
   control->filter_limit        = params->filter_limit;
   control->filter_copies_limit = params->filter_copies_limit;
   control->reweight_limit      = params->reweight_limit;
   control->delete_bad_limit    = params->delete_bad_limit;
   control->ac_handling         = params->ac_handling;
   control->ac_res_aggressive   = params->ac_res_aggressive;
   control->forward_context_sr            = params->forward_context_sr;
   control->forward_context_sr_aggressive = params->forward_context_sr_aggressive;
   control->backward_context_sr           = params->backward_context_sr;

   control->er_varlit_destructive = params->er_varlit_destructive;
   control->er_strong_destructive = params->er_strong_destructive;
   control->er_aggressive         = params->er_aggressive;
   control->prefer_initial_clauses = params->prefer_initial_clauses;
   control->select_on_proc_only = params->select_on_proc_only;      
   control->inherit_paramod_lit = params->inherit_paramod_lit;      
   control->selection_strategy  = params->selection_strategy;      
   control->pos_lit_sel_min     = params->pos_lit_sel_min;      
   control->pos_lit_sel_max     = params->pos_lit_sel_max;      
   control->neg_lit_sel_min     = params->neg_lit_sel_min;      
   control->neg_lit_sel_max     = params->neg_lit_sel_max;      
   control->all_lit_sel_min     = params->all_lit_sel_min;      
   control->all_lit_sel_max     = params->all_lit_sel_max;      
   control->weight_sel_min      = params->weight_sel_min;      
   control->split_clauses       = params->split_clauses;      
   control->split_method        = params->split_method;      
   control->split_aggressive    = params->split_aggressive;      
   control->unproc_simplify     = params->unproc_simplify;
   control->hcb = GetHeuristic(params->heuristic_name,
			       state,
			       control, 
			       params);
}



/*-----------------------------------------------------------------------
//
// Function: ProofStateInit()
//
//   Given a proof state with axioms and a heuristic parameter
//   description, initialize the ProofStateCell, i.e. generate the
//   HCB, the ordering, and evaluate the axioms and put them in the
//   unprocessed list.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void ProofStateInit(ProofState_p state, ProofControl_p control,
		    HeuristicParms_p h_parms, FVIndexParms_p fvi_parms)
{
   Clause_p handle, new;
   HCB_p    tmphcb;
   PStack_p traverse;
   Eval_p   cell;

   OUTPRINT(1, "# Initializing proof state\n");
   
   DEBUGMARK(PP_LOWDETAILS,"ProofStateInit()...starting to copy\n");
   
   assert(ClauseSetEmpty(state->processed_pos_rules));
   assert(ClauseSetEmpty(state->processed_pos_eqns));
   assert(ClauseSetEmpty(state->processed_neg_units));
   assert(ClauseSetEmpty(state->processed_non_units));

   tmphcb = GetHeuristic("Uniq", state, control, h_parms);
   ClauseSetReweight(tmphcb, state->axioms);
   
   traverse =
      EvalTreeTraverseInit(PDArrayElementP(state->axioms->eval_indices,0));
   
   while((cell = EvalTreeTraverseNext(traverse)))
   {
      handle = cell->object;
      new = ClauseCopy(handle, state->terms);
      ClauseSetProp(new, CPInitial);
      HCBClauseEvaluate(control->hcb, new);
      DocClauseQuoteDefault(6, new, "eval");
      if(control->prefer_initial_clauses)
      {
	 EvalListSetPriority(new->evaluations, PrioBest);
      }
      ClauseSetInsert(state->unprocessed, new);
   }
   ClauseSetMarkSOS(state->unprocessed, h_parms->use_tptp_sos);
   EvalTreeTraverseExit(traverse);
   if(control->ac_handling!=NoACHandling)
   {
      if(OutputLevel)
      {
	 fprintf(GlobalOut, "# Scanning for AC axioms\n");
      }
      control->ac_handling_active = ClauseSetScanAC(state->signature,
						    state->unprocessed);
      if(OutputLevel)
      {      
	 SigPrintACStatus(GlobalOut, state->signature);
	 if(control->ac_handling_active)
	 {
	    fprintf(GlobalOut, "# AC handling enabled\n");
	 }
      }
   }
   state->original_symbols = state->signature->f_count;
   if(!control->split_clauses)
   {
      fvi_parms->symbol_slack = 0;
   }
   if(fvi_parms->features != FVINoFeatures)
   {
      long symbols = MIN(state->original_symbols+fvi_parms->symbol_slack, 
			 fvi_parms->max_symbols);     
      PermVector_p perm = PermVectorCompute(state->axioms,		    
					    fvi_parms,
					    symbols);  
      state->processed_non_units->fvindex =
	 FVIAnchorAlloc(symbols, fvi_parms->features, perm);
      state->processed_pos_rules->fvindex =
	 FVIAnchorAlloc(symbols, fvi_parms->features, PermVectorCopy(perm));
      state->processed_pos_eqns->fvindex =
	 FVIAnchorAlloc(symbols, fvi_parms->features, PermVectorCopy(perm));
      state->processed_neg_units->fvindex =
	 FVIAnchorAlloc(symbols, fvi_parms->features,
			PermVectorCopy(perm));
      if(state->watchlist)
      {
	 state->watchlist->fvindex = 
	    FVIAnchorAlloc(symbols, fvi_parms->features,
			   PermVectorCopy(perm));
	 ClauseSetFVIndexify(state->watchlist);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ProcessClause()
//
//   Select an unprocessed clause, process it. Return pointer to empty
//   clause if it can be derived, NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : Everything ;-)
//
/----------------------------------------------------------------------*/

Clause_p ProcessClause(ProofState_p state, ProofControl_p control)
{
   Clause_p         clause, tmp_copy, empty;
   FVPackedClause_p pclause;
   SysDate          clausedate;
   long             clause_count;

   DEBUGMARK(PP_LOWDETAILS, "ProcessClause...\n");
   clause = control->hcb->hcb_select(control->hcb,
				     state->unprocessed);   
   if(OutputLevel==1)
   {
      putc('#', GlobalOut);
   }   
   assert(clause);
   
   state->processed_count++;
   
   ClauseSetExtractEntry(clause);
   ClauseSetProp(clause, CPIsProcessed);
   ClauseDetachParents(clause);
   ClauseRemoveEvaluations(clause);
   
   pclause = ForwardContractClause(state, control, clause, true, 
				   control->forward_context_sr,
				   FullRewrite);
   if(!pclause)
   {
      return NULL;
   }
   check_ac_status(state, control, pclause->clause);

   if(OutputLevel)
   {
      if(OutputLevel == 1)
      {
	 putc('\n', GlobalOut);
	 putc('#', GlobalOut);
	 ClausePrint(GlobalOut, pclause->clause, true);
	 putc('\n', GlobalOut);
      }
      DocClauseQuoteDefault(6, pclause->clause, "new_given");
   }

   state->proc_non_trivial_count++;

   if(control->er_varlit_destructive &&
      (clause_count = ClauseERNormalizeVar(state->terms,
					   pclause->clause,
					   state->tmp_store,
					   state->freshvars,
					   control->er_strong_destructive)))
   {
      state->other_redundant_count += clause_count;
      state->resolv_count += clause_count;
      pclause->clause = NULL;
   }
   else if(ControlledClauseSplit(pclause->clause, state->tmp_store,
				 control->split_clauses,
				 control->split_method))
   {
      pclause->clause = NULL;
   }
   
   if(!pclause->clause) 
   {  /* ...then it has been destroyed by one of the above methods */
      if((empty = insert_new_clauses(state, control)))
      {
	 return empty;
      }
      return FVUnpackClause(pclause);
   }
   if(state->watchlist)
   {
      check_watchlist(state->watchlist, pclause->clause);
   }
    
   if(ClauseIsEmpty(pclause->clause))
   {
      return FVUnpackClause(pclause);
   }

   /* ClauseCanonize(clause); */

   /* Now on to backward simplification. */   
   clausedate = ClauseSetListGetMaxDate(state->demods, FullRewrite);
   
   eleminate_backward_rewritten_clauses(state, control, pclause->clause, &clausedate);   
   eleminate_backward_subsumed_clauses(state, pclause);
   eleminate_unit_simplified_clauses(state, pclause->clause);
   eleminate_context_sr_clauses(state, control, pclause->clause);

   clause = pclause->clause;
   tmp_copy = ClauseCopy(clause, state->tmp_terms);      
   tmp_copy->ident = clause->ident;

   ClauseMarkRestrictedTerms(clause);
   
   clause->date = clausedate;
   if(ClauseIsDemodulator(clause))
   {
      assert(clause->neg_lit_no == 0);
      if(EqnIsOriented(clause->literals))
      {
	 state->processed_pos_rules->date = clausedate;
	 ClauseSetIndexedInsert(state->processed_pos_rules, pclause);
      }
      else
      {
	 state->processed_pos_eqns->date = clausedate;
	 ClauseSetIndexedInsert(state->processed_pos_eqns, pclause);
      }
   }
   else if(ClauseLiteralNumber(clause) == 1)
   {
      assert(clause->neg_lit_no == 1);
      ClauseSetIndexedInsert(state->processed_neg_units, pclause);
   }
   else
   {
      ClauseSetIndexedInsert(state->processed_non_units, pclause);
   }
   FVUnpackClause(pclause);
   ENSURE_NULL(pclause);

   if(control->selection_strategy != SelectNoGeneration)
   {
      generate_new_clauses(state, control, clause, tmp_copy);
   }
   ClauseFree(tmp_copy);
   if(TermCellStoreNodes(&(state->tmp_terms->term_store))>TMPBANK_GC_LIMIT)
   {
      TBGCSweep(state->tmp_terms);
   }
   
#ifdef PRINT_SHARING
   print_sharing_factor(state);
#endif
#ifdef PRINT_RW_STATE
   print_rw_state(state);
#endif

   if((empty = insert_new_clauses(state, control)))
   {
      return empty;
   }  
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function:  Saturate()
//
//   Process clauses until either the empty clause has been derived, a
//   specified number of clauses has been processed, or the clause set
//   is saturated. Return emoty clause (if found) or NULL. 
//
// Global Variables: -
//
// Side Effects    : Modifies state.
//
/----------------------------------------------------------------------*/

Clause_p Saturate(ProofState_p state, ProofControl_p control, long
		  step_limit, long proc_limit, long unproc_limit, long
		  total_limit)
{
   Clause_p unsatisfiable = NULL;
   long count = 0;
   long current_storage = 0;
   long filter_base   = ProofStateStorage(state);
   long reweight_base = state->unprocessed->members;
   long filter_copies_base = ProofStateStorage(state);
   long tmp /* , storage = 0*/;

   while(!TimeIsUp &&
	 state->unprocessed->members && 
	 step_limit>count && 
	 proc_limit>(state->processed_pos_rules->members +
		     state->processed_pos_eqns->members +
		     state->processed_neg_units->members +
		     state->processed_non_units->members) &&
	 unproc_limit>state->unprocessed->members &&
	 total_limit>(state->processed_pos_rules->members +
		      state->processed_pos_eqns->members +
		      state->processed_neg_units->members +
		      state->processed_non_units->members+
		      state->unprocessed->members)&&
	 (!state->watchlist||!ClauseSetEmpty(state->watchlist)))
   {     
      DEBUGMARK(PP_LOWDETAILS,"Saturate loop entry.\n");

      count++;
      unsatisfiable = ProcessClause(state, control);
      if(unsatisfiable)
      {
	 break;
      }
      /* printf("*** Total: %ld \n"
	     "*** Proc. Rules   : %8ld,  %8ld Eqns, %8ld Bytes %ld index cells\n"
	     "*** Proc. Eqns    : %8ld,  %8ld Eqns, %8ld Bytes %ld index cells\n"
	     "*** Proc. NegUnits: %8ld,  %8ld Eqns, %8ld Bytes %ld index cells\n"
	     "*** Proc. NonUnits: %8ld,  %8ld Eqns, %8ld Bytes\n"
	     "*** Unprocessed   : %8ld,  %8ld Eqns, %8ld Bytes %ld index cells\n"
	     "*** Term cells    : %8ld, %8ld Bytes\n",
	     ProofStateStorage(state),

	     state->processed_pos_rules->members,
	     state->processed_pos_rules->literals,
	     ClauseSetStorage(state->processed_pos_rules),
	     state->processed_pos_rules->demod_index->node_count,

	     state->processed_pos_eqns->members,
	     state->processed_pos_eqns->literals,
	     ClauseSetStorage(state->processed_pos_eqns),
	     state->processed_pos_eqns->demod_index->node_count,

	     state->processed_neg_units->members,
	     state->processed_neg_units->literals,
	     ClauseSetStorage(state->processed_neg_units),
	     state->processed_neg_units->demod_index->node_count,

	     state->processed_non_units->members,
	     state->processed_non_units->literals,
	     ClauseSetStorage(state->processed_non_units),

	     state->unprocessed-> members,
	     state->unprocessed-> literals,
	     ClauseSetStorage(state->unprocessed),
	     state->unprocessed->demod_index->node_count,

	     
	     TBTermNodes(state->terms),
	     TBStorage(state->terms)); */
      current_storage = ProofStateStorage(state);
      filter_copies_base = MIN(filter_copies_base,current_storage); 
      if((current_storage - filter_copies_base) >
	 control->filter_copies_limit)
      {
	 tmp = ClauseSetDeleteCopies(state->unprocessed);
	 if(OutputLevel)
	 {
	    fprintf(GlobalOut, 
		    "# Deleted %ld clause copies (remaining: %ld)\n",
		    tmp, state->unprocessed->members);
	 }
	 state->other_redundant_count += tmp;
	 current_storage  = ProofStateStorage(state);
	 filter_copies_base = current_storage;
      }
      filter_base = MIN(filter_base,current_storage); 
      if((current_storage - filter_base) > control->filter_limit)
      {
	 tmp = state->unprocessed->members;
	 unsatisfiable = 
	    ForwardContractSet(state, control,
			       state->unprocessed, false, FullRewrite,
			       &(state->other_redundant_count));
	 
	 if(OutputLevel)
	 {
	    fprintf(GlobalOut, 
		    "# Special forward-contraction deletes %ld clauses"
		    "(remaining: %ld) \n",
		    tmp - state->unprocessed->members,
		    state->unprocessed->members);
	 }	 
	 current_storage  = ProofStateStorage(state);
	 filter_base = current_storage;
	 if(unsatisfiable)
	 {
	    break;
	 }	 
      } 
      reweight_base = MIN(state->unprocessed->members, reweight_base);
      if((state->unprocessed->members - reweight_base) 
	 > control->reweight_limit)
      {
	 OUTPRINT(1, "# Reweighting unprocessed clauses...\n");
	 ClauseSetReweight(control->hcb,  state->unprocessed);
	 reweight_base = state->unprocessed->members;
      }
      tmp = LONG_MAX;
      
      if(current_storage > control->delete_bad_limit)
      {	 
	 tmp = HCBClauseSetDeleteBadClauses(control->hcb,
					    state->unprocessed, 
					    state->unprocessed->members/2);
	 state->non_redundant_deleted += tmp;
	 if(OutputLevel)
	 {
	    fprintf(GlobalOut, 
		    "# Deleted %ld bad clauses (prover may be"
		    " incomplete now)\n", tmp);
	 }
	 state->state_is_complete = false;
	 ProofStateGCMarkTerms(state); 
	 ProofStateGCSweepTerms(state);
	 current_storage = ProofStateStorage(state);
	 filter_base = MIN(filter_base, current_storage);
	 filter_copies_base = MIN(filter_copies_base, current_storage);
      }
   }
   return unsatisfiable;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/



