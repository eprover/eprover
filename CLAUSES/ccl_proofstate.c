/*-----------------------------------------------------------------------

File  : ccl_proofstate.c

Author: Stephan Schulz

Contents
 
  Basic functions for proof state objects.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 11:14:02 MET DST 1998
    New
<2> Wed Oct 14 22:46:13 MET DST 1998
    Extracted from CONTROL/cco_proofstate.c

-----------------------------------------------------------------------*/

#include "ccl_proofstate.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* UseInlinedWatchList = WATCHLIST_INLINE_STRING;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ProofStateAlloc()
//
//   Return an empty, initialized proof state. The argument is:
//   free_symb_prop: Which sub-properties of FPDistinctProp should be
//                   ignored (i.e. which classes with distinct object 
//                   syntax  should be treated as plain free
//                   symbols). Use FPIgnoreProps for default
//                   behaviour, FPDistinctProp for fully free
//                   (conventional) semantics.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

ProofState_p ProofStateAlloc(FunctionProperties free_symb_prop)
{
   ProofState_p handle = ProofStateCellAlloc();

   handle->signature            = SigAlloc();
   SigInsertInternalCodes(handle->signature);
   handle->original_symbols     = 0;
   handle->original_terms       = TBAlloc(handle->signature);
   handle->terms                = TBAlloc(handle->signature);
   handle->tmp_terms            = TBAlloc(handle->signature);
   handle->freshvars            = VarBankAlloc();
   handle->f_axioms             = FormulaSetAlloc();
   handle->f_ax_archive         = FormulaSetAlloc();
   handle->ax_archive           = ClauseSetAlloc();
   handle->axioms               = ClauseSetAlloc();
   handle->processed_pos_rules  = ClauseSetAlloc();
   handle->processed_pos_eqns   = ClauseSetAlloc();
   handle->processed_neg_units  = ClauseSetAlloc();
   handle->processed_non_units  = ClauseSetAlloc();
   handle->unprocessed          = ClauseSetAlloc();
   handle->tmp_store            = ClauseSetAlloc();
   handle->archive              = ClauseSetAlloc();
   handle->f_archive            = FormulaSetAlloc();
   handle->extract_roots        = PStackAlloc();
   GlobalIndicesNull(&(handle->gindices));
   handle->fvi_initialized     = false;
   handle->fvi_cspec           = NULL;
   handle->processed_pos_rules->demod_index = PDTreeAlloc();
   handle->processed_pos_eqns->demod_index  = PDTreeAlloc();
   handle->processed_neg_units->demod_index = PDTreeAlloc();
   // handle->unprocessed->demod_index         = PDTreeAlloc();
   handle->demods[0]            = handle->processed_pos_rules;
   handle->demods[1]            = handle->processed_pos_eqns;
   handle->demods[2]            = NULL;
   handle->watchlist            = NULL;
   GlobalIndicesNull(&(handle->wlindices));
   handle->state_is_complete    = true;
   handle->definition_store     = DefStoreAlloc(handle->terms);
   handle->def_store_cspec      = NULL;
   
   handle->gc_original_terms    = GCAdminAlloc(handle->original_terms);
   GCRegisterFormulaSet(handle->gc_original_terms, handle->f_axioms);
   GCRegisterFormulaSet(handle->gc_original_terms, handle->f_ax_archive);
   GCRegisterClauseSet(handle->gc_original_terms, handle->axioms);
   GCRegisterClauseSet(handle->gc_original_terms, handle->ax_archive);
   handle->gc_terms             = GCAdminAlloc(handle->terms);
   GCRegisterClauseSet(handle->gc_terms, handle->processed_pos_rules);
   GCRegisterClauseSet(handle->gc_terms, handle->processed_pos_eqns);
   GCRegisterClauseSet(handle->gc_terms, handle->processed_neg_units);
   GCRegisterClauseSet(handle->gc_terms, handle->processed_non_units);
   GCRegisterClauseSet(handle->gc_terms, handle->unprocessed);
   GCRegisterClauseSet(handle->gc_terms, handle->tmp_store);
   GCRegisterClauseSet(handle->gc_terms, handle->archive);
   GCRegisterClauseSet(handle->gc_terms, handle->definition_store->def_clauses);
   GCRegisterFormulaSet(handle->gc_terms, handle->definition_store->def_archive);
   GCRegisterFormulaSet(handle->gc_terms, handle->f_archive);

   handle->status_reported              = false;
   handle->answer_count                 = 0;
   
   handle->processed_count              = 0;
   handle->proc_trivial_count           = 0; 
   handle->proc_forward_subsumed_count  = 0; 
   handle->proc_non_trivial_count       = 0;
   handle->other_redundant_count        = 0;
   handle->non_redundant_deleted        = 0;
   handle->backward_subsumed_count      = 0;
   handle->backward_rewritten_count     = 0;
   handle->backward_rewritten_lit_count = 0;
   handle->generated_count              = 0;
   handle->generated_lit_count          = 0;
   handle->non_trivial_generated_count  = 0;
   handle->context_sr_count   = 0; 
   handle->paramod_count      = 0;
   handle->factor_count       = 0;
   handle->resolv_count       = 0;

   handle->signature->distinct_props = 
      handle->signature->distinct_props&(~free_symb_prop);
   
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ProofStateInitWatchlist()
//
//   Initialize the watchlist, either by parsing it from the provided
//   file, or by collecting all clauses of type CPTypeWatchClause from
//   state->axioms. 
//
// Global Variables: -
//
// Side Effects    : Changes state->axioms, IO, memory ops.
//
/----------------------------------------------------------------------*/

void ProofStateInitWatchlist(ProofState_p state, char* watchlist_filename,
                             IOFormat parse_format)
{
   Scanner_p in;
   
   if(watchlist_filename)
   {
      state->watchlist = ClauseSetAlloc();
      GCRegisterClauseSet(state->gc_original_terms, state->watchlist);

      if(watchlist_filename != UseInlinedWatchList)
      {
         in = CreateScanner(StreamTypeFile, watchlist_filename, true, NULL);
         ScannerSetFormat(in, parse_format);
         ClauseSetParseList(in, state->watchlist,
                            state->original_terms);
         CheckInpTok(in, NoToken);
         DestroyScanner(in);
      }
      else
      {
         PStack_p stack = PStackAlloc();
         Clause_p handle;
         
         for(handle =  state->axioms->anchor->succ; 
             handle!= state->axioms->anchor;
             handle = handle->succ)
         {
            if(ClauseQueryTPTPType(handle)==CPTypeWatchClause)
            {
               PStackPushP(stack, handle);
            }
         }
         while(!PStackEmpty(stack))
         {
            handle = PStackPopP(stack);
            ClauseSetExtractEntry(handle);
            ClauseSetInsert(state->watchlist, handle);
         }        
         PStackFree(stack);
      }       
      ClauseSetSetProp(state->watchlist, CPWatchOnly);
      GlobalIndicesInsertClauseSet(&(state->wlindices),state->watchlist);
      ClauseSetDocInital(GlobalOut, OutputLevel, state->watchlist);
      ClauseSetSortLiterals(state->watchlist, EqnSubsumeInverseCompareRef);
   } 
}



/*-----------------------------------------------------------------------
//
// Function: ProofStateResetClauseSets()
//
//   Empty _all_ clause and formula sets in proof state. Keep the
//   signature and term bank. If term_gc is true, perform a garbage
//   collection of term cells. 
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void ProofStateResetClauseSets(ProofState_p state, bool term_gc)
{
   ClauseSetFreeClauses(state->axioms);
   FormulaSetFreeFormulas(state->f_axioms);
   FormulaSetFreeFormulas(state->f_ax_archive);
   ClauseSetFreeClauses(state->processed_pos_rules);
   ClauseSetFreeClauses(state->processed_pos_eqns);
   ClauseSetFreeClauses(state->processed_neg_units);
   ClauseSetFreeClauses(state->processed_non_units);
   ClauseSetFreeClauses(state->unprocessed);
   ClauseSetFreeClauses(state->tmp_store);
   ClauseSetFreeClauses(state->archive);
   ClauseSetFreeClauses(state->ax_archive);
   FormulaSetFreeFormulas(state->f_ax_archive);
   GlobalIndicesReset(&(state->gindices));
   if(state->watchlist)
   {
      ClauseSetFreeClauses(state->watchlist);
      GlobalIndicesReset(&(state->wlindices));
   }
   if(term_gc)
   {
      GCCollect(state->gc_terms);
      GCCollect(state->gc_original_terms);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ProofStateFree()
//
//   Free a ProofStateCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ProofStateFree(ProofState_p junk)
{
   assert(junk);
   ClauseSetFree(junk->axioms);
   FormulaSetFree(junk->f_axioms);
   FormulaSetFree(junk->f_ax_archive);
   ClauseSetFree(junk->processed_pos_rules);
   ClauseSetFree(junk->processed_pos_eqns);
   ClauseSetFree(junk->processed_neg_units);
   ClauseSetFree(junk->processed_non_units);
   ClauseSetFree(junk->unprocessed);
   ClauseSetFree(junk->tmp_store);
   ClauseSetFree(junk->archive);   
   ClauseSetFree(junk->ax_archive);   
   FormulaSetFree(junk->f_archive);
   PStackFree(junk->extract_roots);
   GlobalIndicesFreeIndices(&(junk->gindices));
   GCAdminFree(junk->gc_terms);
   GCAdminFree(junk->gc_original_terms);
   if(junk->watchlist)
   {
      ClauseSetFree(junk->watchlist);
   }
   GlobalIndicesFreeIndices(&(junk->wlindices));

   DefStoreFree(junk->definition_store);
   if(junk->fvi_cspec)
   {
      FVCollectFree(junk->fvi_cspec);
   }
   if(junk->def_store_cspec)
   {
      FVCollectFree(junk->def_store_cspec);
   }

   junk->original_terms->sig = NULL;
   junk->terms->sig = NULL;
   junk->tmp_terms->sig = NULL;
   SigFree(junk->signature);   
   TBFree(junk->original_terms);
   TBFree(junk->terms);
   TBFree(junk->tmp_terms);
   VarBankFree(junk->freshvars);

   ProofStateCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ProofStateStatisticsPrint()
//
//   Print the statistics of the proof state.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ProofStateStatisticsPrint(FILE* out, ProofState_p state)
{
   fprintf(out, "# Initial clauses in saturation        : %ld\n",
	   state->axioms->members);    
   fprintf(out, "# Processed clauses                    : %ld\n",
	   state->processed_count);
   fprintf(out, "# ...of these trivial                  : %ld\n",
	   state->proc_trivial_count);
   fprintf(out, "# ...subsumed                          : %ld\n",
	   state->proc_forward_subsumed_count);
   fprintf(out, "# ...remaining for further processing  : %ld\n",
	   state->proc_non_trivial_count);
   fprintf(out, "# Other redundant clauses eliminated   : %ld\n",
	   state->other_redundant_count);
   fprintf(out, "# Clauses deleted for lack of memory   : %ld\n",
	   state->non_redundant_deleted);
   fprintf(out, "# Backward-subsumed                    : %ld\n",
	   state->backward_subsumed_count);
   fprintf(out, "# Backward-rewritten                   : %ld\n",
	   state->backward_rewritten_count);
   fprintf(out, "# Generated clauses                    : %ld\n",
	   state->generated_count - state->backward_rewritten_count);   
   fprintf(out, "# ...of the previous two non-trivial   : %ld\n",
	   state->non_trivial_generated_count);   
   fprintf(out, "# Contextual simplify-reflections      : %ld\n",
	   state->context_sr_count);
   fprintf(out, "# Paramodulations                      : %ld\n",
	   state->paramod_count);   
   fprintf(out, "# Factorizations                       : %ld\n",
	   state->factor_count);    
   fprintf(out, "# Equation resolutions                 : %ld\n",
	   state->resolv_count);
   fprintf(out, 
	   "# Current number of processed clauses  : %ld\n"
	   "#    Positive orientable unit clauses  : %ld\n"
	   "#    Positive unorientable unit clauses: %ld\n"
	   "#    Negative unit clauses             : %ld\n"
	   "#    Non-unit-clauses                  : %ld\n",
	   state->processed_pos_rules->members+
	   state->processed_pos_eqns->members+
	   state->processed_neg_units->members+
	   state->processed_non_units->members,
	   state->processed_pos_rules->members,
	   state->processed_pos_eqns->members,
	   state->processed_neg_units->members,
	   state->processed_non_units->members);
   fprintf(out, 
	   "# Current number of unprocessed clauses: %ld\n",
	   state->unprocessed->members);
   fprintf(out, 
	   "# ...number of literals in the above   : %ld\n",
	   state->unprocessed->literals);
   if(TBPrintDetails)
   {
      fprintf(out, 
	      "# Total literals in generated clauses  : %ld\n",
	      state->generated_lit_count -
	      state->backward_rewritten_lit_count);      
      fprintf(out,
	      "# Shared term nodes                    : %ld\n"
	      "# ...corresponding unshared nodes      : %ld\n",
	      TBTermNodes(state->terms),
	      ClauseSetGetTermNodes(state->tmp_store)+
	      ClauseSetGetTermNodes(state->processed_pos_rules)+
	      ClauseSetGetTermNodes(state->processed_pos_eqns)+
	      ClauseSetGetTermNodes(state->processed_neg_units)+
	      ClauseSetGetTermNodes(state->processed_non_units)+
	      ClauseSetGetTermNodes(state->unprocessed));
      fprintf(out,
	      "# Shared rewrite steps                 : %lu\n",
	      state->terms->rewrite_steps);
      fprintf(out,
	      "# Match attempts with oriented units   : %lu\n"
	      "# Match attempts with unoriented units : %lu\n",
	      state->processed_pos_rules->demod_index->match_count,
	      state->processed_pos_eqns->demod_index->match_count);	      
#ifdef MEASURE_EXPENSIVE
      fprintf(out,
	      "# Oriented PDT nodes visited           : %lu\n"
	      "# Unoriented PDT nodes visited         : %lu\n",
	      state->processed_pos_rules->demod_index->visited_count,
	      state->processed_pos_eqns->demod_index->visited_count);	      
#endif
   }
   /* TermCellStorePrintDistrib(out, &(state->terms->term_store)); */
   /* TBPrintTermsFlat=false;
      TBPrintInternalInfo=true;				
      TBPrintBankInOrder(stdout,state->terms);*/
}

/*-----------------------------------------------------------------------
//
// Function: ProofStatePrint()
//
//   Print the clause sets of the proof state.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ProofStatePrint(FILE* out, ProofState_p state)
{
   fprintf(out, "\n# Processed positive unit clauses:\n");
   ClauseSetPrint(out, state->processed_pos_rules, true);
   ClauseSetPrint(out, state->processed_pos_eqns, true);
   fprintf(out, "\n# Processed negative unit clauses:\n");
   ClauseSetPrint(out, state->processed_neg_units, true);
   fprintf(out, "\n# Processed non-unit clauses:\n");
   ClauseSetPrint(out, state->processed_non_units, true);
   fprintf(out, "\n# Unprocessed clauses:\n");
   ClauseSetPrint(out, state->unprocessed, true);
}

/*-----------------------------------------------------------------------
//
// Function: ProofStatePropDocQuote()
//
//   Print all clauses in the main clause sets in state for which
//   props is true (if outputlevel is large enough, as defined in
//   ClauseSetPropDocQuote().
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ProofStatePropDocQuote(FILE* out, int level, 
			    ClauseProperties prop,
			    ProofState_p state, char* comment)
{
   ClauseSetPropDocQuote(GlobalOut, level, prop,
			 state->processed_pos_rules, comment);
   ClauseSetPropDocQuote(GlobalOut, level, prop,
			 state->processed_pos_eqns, comment);
   ClauseSetPropDocQuote(GlobalOut, level, prop,
			 state->processed_neg_units, comment);
   ClauseSetPropDocQuote(GlobalOut, level, prop,
			 state->processed_non_units, comment);
   ClauseSetPropDocQuote(GlobalOut, level, prop,
			 state->unprocessed, comment);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
