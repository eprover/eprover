/*-----------------------------------------------------------------------

File  : ccl_proofstate.c

Author: Stephan Schulz

Contents
 
  Basic functions for proof state objects.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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
//   Return an empty, initialized proof state.
//
// Global Variables: -
//
// Side Effects    : memory operations
//
/----------------------------------------------------------------------*/

ProofState_p ProofStateAlloc(void)
{
   ProofState_p handle = ProofStateCellAlloc();

   handle->signature           = SigAlloc();
   handle->original_symbols    = 0;
   handle->original_terms      = TBAlloc(TPIgnoreProps, handle->signature);
   handle->terms               = TBAlloc(TPRestricted, handle->signature);
   handle->tmp_terms           = TBAlloc(TPIgnoreProps, handle->signature);
   handle->freshvars           = VarBankAlloc();
   handle->f_axioms            = FormulaSetAlloc();
   handle->axioms              = ClauseSetAlloc();
   handle->processed_pos_rules = ClauseSetAlloc();
   handle->processed_pos_eqns  = ClauseSetAlloc();
   handle->processed_neg_units = ClauseSetAlloc();
   handle->processed_non_units = ClauseSetAlloc();
   handle->unprocessed         = ClauseSetAlloc();
   handle->tmp_store           = ClauseSetAlloc();
   handle->fvi_initialized     = false;
   handle->processed_pos_rules->demod_index = PDTreeAlloc();
   handle->processed_pos_eqns->demod_index  = PDTreeAlloc();
   handle->processed_neg_units->demod_index = PDTreeAlloc();
   handle->unprocessed->demod_index         = PDTreeAlloc();
   handle->demods[0]           = handle->processed_pos_rules;
   handle->demods[1]           = handle->processed_pos_eqns;
   handle->demods[2]           = NULL;
   handle->watchlist          = NULL;
   handle->state_is_complete   = true;
   handle->processed_count     = 0;
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

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ProofStateResetClauseSets()
//
//   Empty _all_ clause and formula sets in proof state. Keep the
//   signature and term bank. If term_gc is true, performa a garbage
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
   ClauseSetFreeClauses(state->processed_pos_rules);
   ClauseSetFreeClauses(state->processed_pos_eqns);
   ClauseSetFreeClauses(state->processed_neg_units);
   ClauseSetFreeClauses(state->processed_non_units);
   ClauseSetFreeClauses(state->unprocessed);
   ClauseSetFreeClauses(state->tmp_store);
   if(state->watchlist)
   {
      ClauseSetFreeClauses(state->watchlist);
   }
   if(term_gc)
   {
      ProofStateGCMarkTerms(state);
      ProofStateGCSweepTerms(state);
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
   ClauseSetFree(junk->processed_pos_rules);
   ClauseSetFree(junk->processed_pos_eqns);
   ClauseSetFree(junk->processed_neg_units);
   ClauseSetFree(junk->processed_non_units);
   ClauseSetFree(junk->unprocessed);
   ClauseSetFree(junk->tmp_store);
   if(junk->watchlist)
   {
      ClauseSetFree(junk->watchlist);
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
// Function: ProofStateGCMarkTerms()
//
//   Mark all terms still used to represent the actual proof
//   state. Does not mark axiom terms and temporary terms.
//
// Global Variables: -
//
// Side Effects    : Marks terms
//
/----------------------------------------------------------------------*/

void ProofStateGCMarkTerms(ProofState_p state)
{
   ClauseSetGCMarkTerms(state->processed_pos_rules);
   ClauseSetGCMarkTerms(state->processed_pos_eqns);
   ClauseSetGCMarkTerms(state->processed_neg_units);
   ClauseSetGCMarkTerms(state->processed_non_units);
   ClauseSetGCMarkTerms(state->unprocessed);
   if(state->watchlist)
   {
      ClauseSetGCMarkTerms(state->watchlist);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ProofStateGCSweepTerms()
//
//   Collect unmarked term cells from the proof state term bank (not
//   from the temporary term bank or the axiom term bank). Returns
//   number of term cells collected.
//
// Global Variables: -
//
// Side Effects    : Collects memory (Surprise!)
//
/----------------------------------------------------------------------*/

long ProofStateGCSweepTerms(ProofState_p state)
{
   return TBGCSweep(state->terms);
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
   fprintf(out, "# Other redundant clauses eleminated   : %ld\n",
	   state->other_redundant_count);
   fprintf(out, "# Clauses deleted for lack of memory   : %ld\n",
	   state->non_redundant_deleted);
   fprintf(out, "# Backward-subsumed                    : %ld\n",
	   state->backward_subsumed_count);
   fprintf(out, "# Backward-rewritten                   : %ld\n",
	   state->backward_rewritten_count);
   fprintf(out, "# Contextual simplify-reflections      : %ld\n",
	   state->context_sr_count);
   fprintf(out, "# Generated clauses                    : %ld\n",
	   state->generated_count - state->backward_rewritten_count);   
   fprintf(out, "# ...of the previous two non-trivial   : %ld\n",
	   state->non_trivial_generated_count);   
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
